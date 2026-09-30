// Day 25 实验 2：HTTP 客户端 —— 板子主动去互联网上要一个页面
//
// 指南要求请求 http://httpbin.org/get。这个服务会把"它看到的请求"原样返回成 JSON，
// 所以拿到的内容正好可以用来核对：我发出去的到底是什么。
//
// 三个要点：
// ① 先手写一遍裸 HTTP，再用库。指南给的是 WiFiClient + println 手搓请求行，
//    这样能看清 HTTP 就是"几行文本 + 一个空行"；但真要长期用得上 HTTPS，
//    裸写就写不出来了（TLS 握手不是几行字符串）。所以两个都留着，
//    前面理解协议，后面是实际能用的版本。
// ② HTTP/1.1 必须带 Host 头。同一台服务器上挂着几百个站点，靠 Host 区分。
//    少了它，httpbin.org 会返回 400 或者直接不响应。
// ③ 读响应别用 while(client.connected()) 死等。服务端不主动断开的话会一直卡着。
//    加超时：N 毫秒内没有新字节就认为收完了。

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <RgbCycle.h>

const char* SSID = "你的WiFi名";
const char* PASS = "你的密码";

// httpbin.org 的主机名。HTTPS 走 443，HTTP 走 80
const char* HOST = "httpbin.org";
const char* PATH = "/get";

// 收响应的耐心上限：服务端处理慢或者网络拥塞时，等 5 秒还没有新数据就撤
const unsigned long READ_TIMEOUT_MS = 5000;

// 两次请求之间等 15 秒，别把人家的免费服务当压测目标
const unsigned long REQUEST_INTERVAL_MS = 15000;

unsigned long lastRequestMs = 0;
int requestCount = 0;

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
  RgbCycle::setColor(0, 255, 0);
  Serial.printf("\n已连接，IP %s\n", WiFi.localIP().toString().c_str());
}

// ---------- 方案 A：裸 WiFiClient，把 HTTP 报文一个字节一个字节打出来 ----------
//
// 这一段存在的意义是看清楚 HTTP 请求到底长什么样。真正在项目里请用方案 B。
void rawHttpGet() {
  Serial.println("\n═══ 方案 A：裸 WiFiClient 手搓 80 端口 ═══");

  WiFiClient client;
  if (!client.connect(HOST, 80)) {
    Serial.println("TCP 连接失败（DNS 没解析出来？还是 80 端口被拦？）");
    return;
  }
  Serial.println("TCP 已建立");

  // 注意每行结尾是 \r\n，不是 \n。HTTP 规定如此，写 \n 有些服务器也认，
  // 但那是对方的容错，不是规范
  client.printf("GET %s HTTP/1.1\r\n", PATH);
  client.printf("Host: %s\r\n", HOST);          // ② 少了这行就是 400
  client.print("User-Agent: ESP32-S3/1.0\r\n");
  client.print("Connection: close\r\n");        // 让服务端发完就断，省得我们猜长度
  client.print("\r\n");                         // 空行 = 请求头结束

  // 收响应。③ 用"最后一次收到数据的时间"做超时，而不是 connected()
  unsigned long lastByteMs = millis();
  int bytes = 0;
  while (client.connected() || client.available()) {
    if (client.available()) {
      String line = client.readStringUntil('\n');
      line.trim();
      bytes += line.length() + 1;
      Serial.printf("  │ %s\n", line.c_str());
      lastByteMs = millis();
    } else if (millis() - lastByteMs > READ_TIMEOUT_MS) {
      Serial.println("  （超时，认为收完了）");
      break;
    } else {
      delay(10);
    }
  }
  client.stop();
  Serial.printf("共 %d 字节，连接已关闭\n", bytes);
}

// ---------- 方案 B：HTTPClient + WiFiClientSecure，实际项目用这个 ----------
//
// 差别不只是"多了个 S"：
//   - HTTPS 要 TLS 握手，证书验证要烧进根证书（下面用 setInsecure() 跳过，
//     代价是不防中间人；正式项目请用 WiFiClientSecure::setCACert() 指定根证书）
//   - HTTPClient 帮我们处理重定向、chunked 编码、Content-Length，
//     这些裸写都要自己做
void secureHttpGet() {
  Serial.println("\n═══ 方案 B：HTTPClient + TLS（443 端口）═══");

  WiFiClientSecure secureClient;
  // 跳过证书校验。图省事，但在不可信网络里等于把门敞开
  secureClient.setInsecure();

  HTTPClient http;
  // begin() 里给完整 URL，库自己判断 http / https
  if (!http.begin(secureClient, String("https://") + HOST + PATH)) {
    Serial.println("http.begin 失败");
    return;
  }

  // 超时按毫秒设，不设就是默认 5 秒，弱网下经常不够
  http.setTimeout(8000);
  http.addHeader("User-Agent", "ESP32-S3/1.0");

  int code = http.GET();
  Serial.printf("HTTP 状态码: %d\n", code);

  if (code > 0) {
    String payload = http.getString();
    Serial.printf("响应体 %d 字节：\n", payload.length());
    Serial.println(payload);

    // 从 JSON 里抠出 origin —— 这是路由器给这辆车分配的公网 IP。
    // 不用 ArduinoJson 也能做：JSON 就一层，indexOf + 切片足够
    int k = payload.indexOf("\"origin\"");
    if (k >= 0) {
      int colon = payload.indexOf(':', k);
      int q1 = payload.indexOf('"', colon);
      int q2 = payload.indexOf('"', q1 + 1);
      if (colon > 0 && q1 > 0 && q2 > q1) {
        Serial.printf("→ 这条请求在公网上看到的来源是：%s\n",
                      payload.substring(q1 + 1, q2).c_str());
      }
    }
  } else {
    // 负数错误码。HTTPClient 用 -1 表示连接不上，-2 表示超时（各版本略有差异）
    Serial.printf("请求失败，错误码 %d（负数 = 连不上/超时/DNS 失败）\n", code);
  }

  http.end();   // 必须调，否则连接池里的 socket 不释放，几次之后就没得用了
  Serial.printf("当前剩余堆内存: %u 字节\n", ESP.getFreeHeap());
}

void setup() {
  Serial.begin(115200);
  delay(200);
  RgbCycle::begin();
  RgbCycle::setColor(255, 0, 0);

  Serial.println("Day25 实验2：HTTP 客户端");
  connectWiFi();

  // 先跑裸的，看清楚报文；再跑 TLS 的，看清楚实际用法
  rawHttpGet();
  secureHttpGet();
}

void loop() {
  // 每 15 秒重发一次 HTTPS 请求，观察连接是否稳定、堆内存有没有泄漏
  if (millis() - lastRequestMs > REQUEST_INTERVAL_MS) {
    lastRequestMs = millis();
    requestCount++;
    Serial.printf("\n──── 第 %d 次轮询（已运行 %lus）────\n", requestCount, millis() / 1000);
    secureHttpGet();
  }
  delay(100);
}
