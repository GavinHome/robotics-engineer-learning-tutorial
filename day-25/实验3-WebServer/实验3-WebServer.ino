// Day 25 实验 3：板子自己起 Web Server，浏览器访问 IP 看实时传感器数据
//
// 这是 Day 25 里最有价值的一个实验 —— 前面两个只是"能联网"，这个是"别人能来问我"。
// 车以后脱离 USB，浏览器就是它唯一的显示器：不用装任何软件，手机连同一个 Wi-Fi
// 就能看到车在测什么。
//
// 电路（沿用 Day 21，一个元件都没加）：
//
//   HC-SR04        ESP32-S3
//   ───────        ────────
//   VCC  ────────  3V3
//   GND  ────────  GND
//   Trig ────────  GPIO4
//   Echo ────────  GPIO5
//
// 指南要求"访问 ESP32 IP 看传感器数据"——车上的传感器只有 HC-SR04 一个，
// 它就已经满足要求了，不需要再临时搭个电位器充当第二个传感器。
//
// 三个要点：
// ① loop() 里不能有 delay()。WebServer 靠 server.handleClient() 推进，
//    一次 delay(500) 就是半秒不响应，页面会转圈。用 millis() 做非阻塞节流。
// ② 别用 String 拼整个 HTML。ESP32 的堆只有几百 KB，字符串反复拼接会打洞，
//    跑几小时就 malloc 失败。静态页面放成 const char[]（在 flash 里，不占堆）。
// ③ 页面和数据分开：`/` 给 HTML，`/data` 给 JSON。浏览器每秒拉一次 /data 刷新数字，
//    不用整页重载。这也是 Day 26 Python 端要用的接口 —— 同一份 JSON，两种消费者。
//
// 📌 车上目前没有模拟量。Day 27-28 的"电池电压监测"要用 ADC，届时必须接
//    ADC1（GPIO1-10）：ESP32-S3 的 ADC2（GPIO11-20）和 Wi-Fi 共用一套硬件，
//    开了 Wi-Fi 之后 analogRead(ADC2) 会失败。这条 Day 8/12/22 都记过，
//    今天的实验里没有 ADC，所以没验成。

#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <RgbCycle.h>

const char* SSID = "你的WiFi名";
const char* PASS = "你的密码";

// ---------- 引脚 ----------
const int TRIG_PIN = 4;
const int ECHO_PIN = 5;

// ---------- 测距 ----------
const float CM_PER_US = 0.0343 / 2.0;
const unsigned long TIMEOUT_US = 6000;    // ≈103cm，避障场景够用
const unsigned long MEASURE_MS = 200;     // 两次测量间隔，手册要求 ≥60ms

float lastCm = -1.0;         // -1 = 超时/无效
unsigned long lastMeasureMs = 0;
unsigned long sampleCount = 0;

WebServer server(80);

// ---------- 静态页面放 flash ----------
// ③ PROGMEM 的 const char[] 烧在 flash 里，不占堆。
// 页面里的 %s 占位符在发送时替换，避免运行时拼字符串
static const char PAGE[] PROGMEM = R"rawliteral(
<!doctype html><html lang="zh"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>ESP32-S3 遥测</title>
<style>
 body{font-family:-apple-system,PingFang SC,sans-serif;max-width:640px;margin:24px auto;padding:0 16px;color:#111}
 h1{font-size:20px;margin:0 0 4px}
 .sub{color:#6b7280;font-size:13px;margin-bottom:20px}
 .card{border:1px solid #e5e7eb;border-radius:8px;padding:16px;margin-bottom:12px}
 .label{font-size:13px;color:#6b7280}
 .val{font-size:32px;font-weight:600;margin-top:4px}
 .val small{font-size:15px;font-weight:400;color:#6b7280;margin-left:4px}
 .bar{height:8px;background:#e5e7eb;border-radius:4px;margin-top:10px;overflow:hidden}
 .bar>i{display:block;height:100%;background:#2563eb;transition:width .2s}
 .meta{font-size:12px;color:#9ca3af;margin-top:16px;line-height:1.7}
</style></head><body>
<h1>ESP32-S3 遥测</h1>
<div class="sub">前方距离，每秒自动刷新</div>

<div class="card">
  <div class="label">前方距离</div>
  <div class="val" id="cm">--<small>cm</small></div>
  <div class="bar"><i id="cmbar" style="width:0%"></i></div>
</div>

<div class="meta">
  IP <span id="ip">--</span> ｜ 信号 <span id="rssi">--</span> dBm ｜
  在线 <span id="up">--</span> s ｜ 采样 <span id="n">--</span> 次<br>
  mDNS：<code>http://esp32s3.local</code>
</div>

<script>
// ④ 每秒拉一次 /data 只更新数字，不重载整页
async function tick(){
  try{
    const r = await fetch('/data');
    const d = await r.json();
    const cm = d.cm;
    document.getElementById('cm').innerHTML =
      cm < 0 ? '超出<small>量程</small>' : cm.toFixed(1) + '<small>cm</small>';
    // 0~100cm 映射到 0~100%
    document.getElementById('cmbar').style.width =
      cm < 0 ? '100%' : Math.min(100, cm / 100 * 100) + '%';
    document.getElementById('ip').textContent = d.ip;
    document.getElementById('rssi').textContent = d.rssi;
    document.getElementById('up').textContent = d.up;
    document.getElementById('n').textContent = d.n;
  }catch(e){
    document.getElementById('cm').innerHTML = '断线<small>重试中</small>';
  }
}
tick();
setInterval(tick, 1000);   // 1 秒。改成 100 也能跑，但会明显抬高 CPU 占用
</script></body></html>
)rawliteral";

// ---------- 测距 ----------
float measureCm() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  unsigned long us = pulseIn(ECHO_PIN, HIGH, TIMEOUT_US);
  if (us == 0) return -1.0;
  return us * CM_PER_US;
}

// ---------- 路由 ----------
void handleRoot() {
  // 整页只有一个资源，直接一次发完。真要分片就用
  // server.sendContent() 分几次发，别用 String += 拼
  server.send(200, "text/html; charset=utf-8", String(FPSTR(PAGE)));
}

void handleData() {
  // JSON 就一层，手写拼接即可，不需要 ArduinoJson
  // 用 char 缓冲而不是 String：长度已知，栈上分配完就回收
  char buf[256];
  snprintf(buf, sizeof(buf),
           "{\"cm\":%.1f,\"rssi\":%ld,\"up\":%lu,\"n\":%lu,\"ip\":\"%s\"}",
           lastCm,
           WiFi.RSSI(),
           millis() / 1000,
           sampleCount,
           WiFi.localIP().toString().c_str());
  server.send(200, "application/json", buf);
}

void handleNotFound() {
  server.send(404, "text/plain; charset=utf-8",
              "404 — 只有 / 和 /data");
}

void connectWiFi() {
  Serial.printf("连接 %s", SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASS);
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - start > 15000) {
      Serial.println("\n连不上，10 秒后重启");
      delay(10000);
      ESP.restart();
    }
    Serial.print(".");
    RgbCycle::setColor(0, 0, ((millis() / 300) % 2) ? 255 : 0);
    delay(300);
  }
}

void setup() {
  Serial.begin(115200);
  delay(200);
  RgbCycle::begin();
  RgbCycle::setColor(255, 0, 0);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  digitalWrite(TRIG_PIN, LOW);

  Serial.println("Day25 实验3：Web Server 显示传感器数据");
  connectWiFi();

  // mDNS：之后可以用 http://esp32s3.local 访问，不用记 IP。
  // 局域网里 IP 会变（DHCP 租约到期），名字不会变
  if (MDNS.begin("esp32s3")) {
    Serial.println("mDNS 已启动 → http://esp32s3.local");
  } else {
    Serial.println("mDNS 启动失败（不影响用 IP 访问）");
  }

  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.onNotFound(handleNotFound);
  server.begin();

  RgbCycle::setColor(0, 255, 0);   // 绿：服务已就绪
  Serial.println("────────────────────────────");
  Serial.printf("浏览器打开：http://%s\n", WiFi.localIP().toString().c_str());
  Serial.println("或 http://esp32s3.local");
  Serial.printf("JSON 接口：http://%s/data\n", WiFi.localIP().toString().c_str());
  Serial.println("────────────────────────────");
}

void loop() {
  // ② 这个必须每轮都调，且不能被阻塞。delay() 在这里是禁品
  server.handleClient();

  // 非阻塞节流测距
  if (millis() - lastMeasureMs >= MEASURE_MS) {
    lastMeasureMs = millis();
    lastCm = measureCm();
    sampleCount++;

    // 串口留一份，方便对着浏览器核对两个通道是不是同一份数据
    if (lastCm < 0) {
      Serial.printf("#%lu 超出量程（>%lucm）\n", sampleCount, (unsigned long)(TIMEOUT_US * CM_PER_US));
    } else {
      Serial.printf("#%lu %.1f cm\n", sampleCount, lastCm);
    }
  }

  // 灯色跟着距离走，脱离串口时它是唯一输出
  if (lastCm >= 0 && lastCm < 15.0)      RgbCycle::setColor(255, 0, 0);    // 近 = 红
  else if (lastCm >= 0 && lastCm < 40.0) RgbCycle::setColor(255, 255, 0);  // 中 = 黄
  else                                   RgbCycle::setColor(0, 255, 0);    // 远 = 绿

  // 掉线自动重连。AP 重启之后不该要人去按复位
  static unsigned long lastReconnectMs = 0;
  if (WiFi.status() != WL_CONNECTED && millis() - lastReconnectMs > 5000) {
    lastReconnectMs = millis();
    Serial.println("Wi-Fi 掉线，重连中");
    WiFi.reconnect();
  }

  delay(10);
}
