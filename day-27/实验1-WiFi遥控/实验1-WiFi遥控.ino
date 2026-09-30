// Day 27 实验 1：Wi-Fi 遥控小车 —— 手机浏览器开车，板子自己看门
//
// 这一步只有一件事是新的：**服务器第一次能改物理世界的状态**。
// Day 25 的 `/data` 是只读的，读挂了顶多少看一个数；今天 `/cmd` 是写的，
// 写通道断了车还在跑，就会撞东西。所以今天一半的功夫花在"怎么让它停下来"。
//
// 电路（沿用 Day 24 定型的车，一个元件都没加）：
//
//   TB6612FNG      ESP32-S3
//   ─────────      ────────
//   AIN1/PWMA/AIN2  GPIO10 / 12 / 11   左轮
//   BIN1/PWMB/BIN2  GPIO15 / 17 / 16   右轮
//
//   HC-SR04        ESP32-S3
//   ───────        ────────
//   Trig ────────  GPIO4
//   Echo ────────  GPIO5
//
// 引脚排布是 Day 17~21 一路让出来的：4/5 超声、8/9 留 I2C、10/11/12/15/16/17 电机。
// 今天仍然不接 IMU、不接电池分压 —— 见 README「为什么省掉两样」。
//
// 三个要点：
// ① 停止不能指望浏览器。松手停靠页面发 `stop`，但页面可能崩、可能被系统挂起、
//    手机可能直接走出 Wi-Fi 范围——这些情况下没有任何 `stop` 会发出来。
//    板子必须自己有个看门狗：超过 CMD_TIMEOUT_MS 没收到指令就刹车。
// ② 看门狗要能用，按住期间就必须持续发心跳。否则用户正常按住 3 秒，
//    板子第 1 秒就判超时停车，看门狗反而成了打断驾驶的隐患。
//    心跳和兜底是同一件事的两面：发得勤，超时才只在"真出事"时触发。
// ③ 上电默认态必须是停车。板子一上电 Wi-Fi 还没连上，手机也连不进来，
//    这会儿要是默认前进，插电一瞬间车就窜出去。

#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <RgbCycle.h>

const char* SSID = "你的WiFi名";
const char* PASS = "你的密码";

// ---------- 电机引脚（Day 24 定型）----------
const int AIN1 = 10, AIN2 = 11, PWMA = 12;   // 左轮
const int BIN1 = 15, BIN2 = 16, PWMB = 17;   // 右轮

// ---------- 超声引脚 ----------
const int TRIG_PIN = 4;
const int ECHO_PIN = 5;

// ---------- 驱动参数 ----------
const int PWM_FREQ    = 20000;   // 20kHz，消除 PWM 啸叫
const int PWM_RES     = 8;
const int DUTY_MIN    = 60;      // 低于它马达只哼哼不转
const int DUTY_MAX    = 200;     // 高于它平均电压超马达额定
const int DUTY_CRUISE = 115;     // 默认速度：6V × 115/255 ≈ 2.7V 平均
const int DUTY_TURN   = 120;     // 原地转向固定用这个，不跟滑块走

// ---------- 测距参数 ----------
const float CM_PER_US = 0.0343 / 2.0;
const unsigned long TIMEOUT_US = 6000;    // ≈103cm
const unsigned long MEASURE_MS = 200;     // 手册要求 ≥60ms

// ---------- 看门狗 ----------
// 500ms 太紧（一次请求 + 网络抖动就可能误停），3000ms 太松（够车冲出两米）。
// 1000ms 是"松手不发 stop 也能在一米内停住"的量级。
const unsigned long CMD_TIMEOUT_MS = 1000;

// ---------- 指令状态 ----------
// 枚举名避开 Arduino 核心宏 OK / LOW / HIGH
enum Cmd { CMD_STOP, CMD_FWD, CMD_BACK, CMD_LEFT, CMD_RIGHT };

Cmd cmdDir = CMD_STOP;          // 上电即停车，见要点 ③
int cmdDuty = DUTY_CRUISE;
bool cmdMoving = false;
unsigned long lastCmdMs = 0;    // 任何一次被接受的 /cmd 到达都刷新
unsigned long timeoutStops = 0; // 看门狗停车次数，给页面和日志当证据
unsigned long flashUntil = 0;   // 看门狗停车后的闪红截止时刻，0 = 不闪

WebServer server(80);

// ---------- 测距 ----------
float lastCm = -1.0;
unsigned long lastMeasureMs = 0;
unsigned long sampleCount = 0;

// ---------- 电机 ----------
void driveMotor(int in1, int in2, int pwmPin, int speed) {
  if (speed > 0) {
    digitalWrite(in1, HIGH); digitalWrite(in2, LOW);
  } else if (speed < 0) {
    digitalWrite(in1, LOW);  digitalWrite(in2, HIGH);
  } else {
    digitalWrite(in1, LOW); digitalWrite(in2, LOW);
    ledcWrite(pwmPin, 0);       // 高阻 = 滑行
    return;
  }
  ledcWrite(pwmPin, abs(speed));
}

// 刹车 = 短接 + PWM 满值，两个条件都要满足
void brakeMotor(int in1, int in2, int pwmPin) {
  digitalWrite(in1, HIGH); digitalWrite(in2, HIGH);
  ledcWrite(pwmPin, 255);
}

// left / right 正负号就是前进后退，和 Day 21 实验 2 同一套约定
void drive(int left, int right) {
  driveMotor(AIN1, AIN2, PWMA, left);
  driveMotor(BIN1, BIN2, PWMB, right);
}

void brakeAll() {
  brakeMotor(AIN1, AIN2, PWMA);
  brakeMotor(BIN1, BIN2, PWMB);
}

// ---------- 把 cmdDir / cmdDuty 变成实际的电机动作 ----------
// 方向和速度是两个独立状态：滑块只改 cmdDuty，心跳只发 dir。
// 两者分开放，页面不必在每次心跳里重复发一遍速度。
void applyMotion() {
  switch (cmdDir) {
    case CMD_FWD:  drive( cmdDuty,  cmdDuty);     cmdMoving = true;  break;
    case CMD_BACK: drive(-cmdDuty, -cmdDuty);     cmdMoving = true;  break;
    // 原地转向：两轮等速反向，左转 = 左轮退右轮进（同 Day 21 的 drive(-x, +x)）
    case CMD_LEFT:  drive(-DUTY_TURN, DUTY_TURN); cmdMoving = true;  break;
    case CMD_RIGHT: drive( DUTY_TURN, -DUTY_TURN); cmdMoving = true; break;
    default:        brakeAll();                   cmdMoving = false; break;
  }
  showStatus();
}

// RGB 改用途了：Day 25 用它传距离远近，但遥控时人盯着手机屏幕，
// 灯该传的是"车现在动不动、为什么停"。脱离 USB 时它仍是唯一输出。
void showStatus() {
  if (cmdMoving) {
    switch (cmdDir) {
      case CMD_FWD:   RgbCycle::setColor(0, 255, 0);   break;  // 绿：前进
      case CMD_BACK:  RgbCycle::setColor(255, 160, 0); break;  // 橙：后退
      case CMD_LEFT:  RgbCycle::setColor(255, 0, 0);   break;  // 红：左转
      case CMD_RIGHT: RgbCycle::setColor(255, 0, 255); break;  // 品红：右转
      default: break;
    }
  } else {
    RgbCycle::setColor(0, 0, 0);                          // 暗：停着
  }
}

const char* cmdName(Cmd c) {
  switch (c) {
    case CMD_FWD:   return "fwd";
    case CMD_BACK:  return "back";
    case CMD_LEFT:  return "left";
    case CMD_RIGHT: return "right";
    default:        return "stop";
  }
}

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

// ---------- 控制面板 ----------
// 沿用 Day 25 的三条：整页一次发完、静态内容放 flash、页面和数据分开。
// 新增的是按钮怎么绑定 —— 见 PAGE 里的注释。
static const char PAGE[] PROGMEM = R"rawliteral(
<!doctype html><html lang="zh"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1,maximum-scale=1,user-scalable=no">
<title>Wi-Fi 遥控小车</title>
<style>
 body{font-family:-apple-system,PingFang SC,sans-serif;max-width:420px;margin:20px auto;padding:0 16px;color:#111}
 h1{font-size:20px;margin:0 0 4px}
 .sub{color:#6b7280;font-size:13px;margin-bottom:16px}
 .card{border:1px solid #e5e7eb;border-radius:8px;padding:14px;margin-bottom:14px}
 .label{font-size:12px;color:#6b7280}
 .val{font-size:30px;font-weight:600;margin-top:2px}
 .val small{font-size:14px;font-weight:400;color:#6b7280;margin-left:4px}
 .bar{height:8px;background:#e5e7eb;border-radius:4px;margin-top:10px;overflow:hidden}
 .bar>i{display:block;height:100%;background:#2563eb;transition:width .2s}
 .grid{display:grid;grid-template-columns:repeat(3,1fr);gap:8px;margin-bottom:14px}
 /* 手机上不设最小 64px 会经常点空；按钮要经得起拇指 */
 button{border:1px solid #d1d5db;background:#fff;border-radius:10px;height:64px;
        font-size:22px;color:#111;-webkit-user-select:none;user-select:none;
        /* 没有这条，按住按钮会触发页面滚动/双击缩放，pointerup 就丢了 */
        touch-action:none}
 button:active,button.on{background:#2563eb;color:#fff;border-color:#2563eb}
 #stop{background:#fee2e2;border-color:#fca5a5;font-size:16px}
 #stop:active{background:#dc2626;color:#fff}
 .spacer{visibility:hidden}
 .row{display:flex;align-items:center;gap:10px;font-size:13px;color:#374151}
 input[type=range]{flex:1}
 .meta{font-size:12px;color:#9ca3af;margin-top:14px;line-height:1.7}
 #state{font-weight:600}
</style></head><body>
<h1>遥控小车</h1>
<div class="sub">按住方向键行驶，松手即停；松手 1 秒内没指令，板子自己刹车</div>

<div class="card">
  <div class="label">前方距离</div>
  <div class="val" id="cm">--<small>cm</small></div>
  <div class="bar"><i id="cmbar" style="width:0%"></i></div>
</div>

<div class="grid">
  <div class="spacer"></div>
  <button id="fwd">▲</button>
  <div class="spacer"></div>
  <button id="left">◀</button>
  <button id="stop">■ 停</button>
  <button id="right">▶</button>
  <div class="spacer"></div>
  <button id="back">▼</button>
  <div class="spacer"></div>
</div>

<div class="row">
  <span>速度</span>
  <input type="range" id="speed" min="60" max="200" step="5" value="115">
  <span id="duty">115</span>
</div>

<div class="meta">
  状态 <span id="state">stop</span> ｜ 超时停车 <span id="tmo">0</span> 次 ｜
  信号 <span id="rssi">--</span> dBm ｜ 在线 <span id="up">--</span> s<br>
  <code id="ip">--</code> ｜ <code>http://esp32s3.local</code>
</div>

<script>
const BEAT_MS = 250;          // 按住时的心跳间隔，必须小于板子的 1000ms 超时
let holding = null;           // 当前按住的方向，null = 没按
let beat = null;

function send(q) {
  // 遥控指令丢了不要重试轰炸：超时看门狗会兜底，重试只会让指令堆积
  fetch('/cmd?' + q).catch(() => {});
}

function startHold(dir) {
  if (holding === dir) return;
  holding = dir;
  send('dir=' + dir);
  // 心跳是让看门狗可用的前提：不发心跳，板子 1 秒后就判超时把车停掉
  beat = setInterval(() => send('dir=' + holding), BEAT_MS);
}

function endHold() {
  if (holding === null && beat === null) return;
  holding = null;
  clearInterval(beat);
  beat = null;
  send('dir=stop');
}

// ① 用 Pointer Events，不用 onmouseup。onmouseup 是浏览器给触摸设备模拟的，
//    时机不可靠；pointerup 是统一鼠标/触摸/笔的那一个。
// ② setPointerCapture 是把"手指滑出按钮再松手"这种情况补上的关键：
//    不捕获的话 pointerup 发到按钮外面的元素上，按钮收不到，车就不停。
for (const id of ['fwd', 'back', 'left', 'right']) {
  const b = document.getElementById(id);
  b.addEventListener('pointerdown', e => {
    e.preventDefault();
    b.setPointerCapture(e.pointerId);
    b.classList.add('on');
    startHold(id);
  });
  b.addEventListener('pointerup',     () => { b.classList.remove('on'); endHold(); });
  b.addEventListener('pointercancel', () => { b.classList.remove('on'); endHold(); });
  b.addEventListener('lostpointercapture', () => { b.classList.remove('on'); endHold(); });
  // 安卓长按按钮会弹系统菜单，弹出来时 pointerup 就没了
  b.addEventListener('contextmenu', e => e.preventDefault());
}

document.getElementById('stop').addEventListener('click', endHold);

// ③ 页面被系统挂起也必须停。锁屏、切后台、来电都会让 pointerup 不再送来，
//    这时候不能只靠板子的 1 秒超时——立即停比等着看门狗反应更稳。
document.addEventListener('visibilitychange', () => {
  if (document.hidden) endHold();
});
window.addEventListener('blur', endHold);

// 速度：change 在松手时才触发，一次请求；input 事件跟着手指一路刷请求没意义
const slider = document.getElementById('speed');
slider.addEventListener('change', () => {
  send('speed=' + slider.value);
  document.getElementById('duty').textContent = slider.value;
});

async function tick() {
  try {
    const r = await fetch('/data');
    const d = await r.json();
    const cm = d.cm;
    document.getElementById('cm').innerHTML =
      cm < 0 ? '超出<small>量程</small>' : cm.toFixed(1) + '<small>cm</small>';
    document.getElementById('cmbar').style.width =
      cm < 0 ? '100%' : Math.min(100, cm) + '%';
    document.getElementById('state').textContent = d.dir;
    document.getElementById('tmo').textContent = d.tmo;
    document.getElementById('rssi').textContent = d.rssi;
    document.getElementById('up').textContent = d.up;
    document.getElementById('ip').textContent = d.ip;
  } catch (e) {
    document.getElementById('cm').innerHTML = '断线<small>重试中</small>';
    endHold();          // 拉不到数据说明连不上了，别让车继续跑
  }
}
tick();
setInterval(tick, 500);
</script></body></html>
)rawliteral";

// ---------- 路由 ----------
void handleRoot() {
  server.send(200, "text/html; charset=utf-8", String(FPSTR(PAGE)));
}

void handleData() {
  // 比 Day 25 多两个字段：dir 和 tmo。都是给"停止链路"当证据用的
  char buf[256];
  snprintf(buf, sizeof(buf),
           "{\"cm\":%.1f,\"rssi\":%ld,\"up\":%lu,\"n\":%lu,\"ip\":\"%s\","
           "\"dir\":\"%s\",\"duty\":%d,\"tmo\":%lu}",
           lastCm,
           WiFi.RSSI(),
           millis() / 1000,
           sampleCount,
           WiFi.localIP().toString().c_str(),
           cmdName(cmdDir),
           cmdDuty,
           timeoutStops);
  server.send(200, "application/json", buf);
}

// 只认白名单里的值，别把 server.arg() 直接拿去用。
// 今天是驱动电机，一个没见过的字符串至少该被拒掉，而不是静默忽略
void handleCmd() {
  String dir = server.arg("dir");
  String spd = server.arg("speed");
  bool changed = false;

  if (server.hasArg("dir")) {
    Cmd next = cmdDir;
    if      (dir == "fwd")   next = CMD_FWD;
    else if (dir == "back")  next = CMD_BACK;
    else if (dir == "left")  next = CMD_LEFT;
    else if (dir == "right") next = CMD_RIGHT;
    else if (dir == "stop")  next = CMD_STOP;
    else {
      server.send(400, "text/plain; charset=utf-8",
                  "400 — dir 只接受 fwd/back/left/right/stop");
      return;
    }
    if (next != cmdDir) {
      cmdDir = next;
      changed = true;
      Serial.printf("指令 → %s\n", cmdName(cmdDir));
    }
  }

  if (server.hasArg("speed")) {
    int v = spd.toInt();
    if (v < DUTY_MIN || v > DUTY_MAX) {
      char msg[80];
      snprintf(msg, sizeof(msg), "400 — speed 要在 %d~%d 之间，收到 %d",
               DUTY_MIN, DUTY_MAX, v);
      server.send(400, "text/plain; charset=utf-8", msg);
      return;
    }
    if (v != cmdDuty) {
      cmdDuty = v;
      changed = true;      // 速度变了要立刻作用，不等下一次心跳
      Serial.printf("速度 → %d\n", cmdDuty);
    }
  }

  // 看门狗只认"被接受的指令"。上面两个 400 分支都是提前 return 的 ——
  // 一个只会发垃圾参数的客户端不该被当成还活着，
  // 否则它能靠刷无效请求一直把车续着跑。
  lastCmdMs = millis();

  if (changed) applyMotion();
  server.send(200, "text/plain; charset=utf-8", cmdName(cmdDir));
}

void handleNotFound() {
  server.send(404, "text/plain; charset=utf-8",
              "404 — 只有 / 、 /data 、 /cmd");
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

  pinMode(AIN1, OUTPUT); pinMode(AIN2, OUTPUT);
  pinMode(BIN1, OUTPUT); pinMode(BIN2, OUTPUT);
  ledcAttach(PWMA, PWM_FREQ, PWM_RES);
  ledcAttach(PWMB, PWM_FREQ, PWM_RES);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  digitalWrite(TRIG_PIN, LOW);

  // 要点 ③：上电默认态 = 停车。先把轮子刹住再去做别的事，
  // 包括连 Wi-Fi 那几秒——那期间手机连不进来，车没有任何理由动
  brakeAll();
  cmdDir = CMD_STOP;
  cmdMoving = false;
  showStatus();

  Serial.println("Day27 实验1：Wi-Fi 遥控小车");
  Serial.println("上电默认停车；松手/断线/超时都会刹车");
  connectWiFi();

  if (MDNS.begin("esp32s3")) {
    Serial.println("mDNS 已启动 → http://esp32s3.local");
  } else {
    Serial.println("mDNS 启动失败（不影响用 IP 访问）");
  }

  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.on("/cmd", handleCmd);
  server.onNotFound(handleNotFound);
  server.begin();

  RgbCycle::setColor(0, 255, 0);
  Serial.println("────────────────────────────");
  Serial.printf("控制面板：http://%s\n", WiFi.localIP().toString().c_str());
  Serial.println("或 http://esp32s3.local（手机连同一个 Wi-Fi 打开）");
  Serial.println("────────────────────────────");
}

void loop() {
  // 这一行每轮都必须调，且不能被阻塞 —— Day 25 的结论，今天更要紧：
  // 一次 delay(500) 就是半秒不收指令，超时看门狗会误判
  server.handleClient();

  // ---------- 看门狗：今天真正新的一段 ----------
  // 触发条件有两个，都是"控制端不在了"：
  //   收不到新指令超过 CMD_TIMEOUT_MS —— 页面崩了、被系统挂起、请求丢了
  //   Wi-Fi 掉了                        —— 手机走远、AP 重启
  // 两种情况都不能让车继续跑。只在车确实在动时动作，
  // 否则静止的车会每秒刷一行日志。
  bool ctrlAlive = (WiFi.status() == WL_CONNECTED) &&
                   (millis() - lastCmdMs <= CMD_TIMEOUT_MS);
  if (cmdMoving && !ctrlAlive) {
    cmdDir = CMD_STOP;
    applyMotion();
    timeoutStops++;
    if (WiFi.status() != WL_CONNECTED) {
      Serial.printf("⚠ Wi-Fi 掉线，已刹车（第 %lu 次）\n", timeoutStops);
    } else {
      Serial.printf("⚠ %lu ms 没收到指令，已刹车（第 %lu 次）\n",
                    CMD_TIMEOUT_MS, timeoutStops);
    }
    // 闪红几下，脱离 USB 时这是唯一能看出"刚刚被看门狗停过"的信号。
    // 但这里绝不能用 delay() 闪：720 ms 不调 handleClient()，
    // 既违背 Day 25 立的规矩，又正好让看门狗自己多饿 720 ms。
    // 只记一个截止时刻，闪的动作放到 loop 末尾非阻塞地做。
    flashUntil = millis() + 720;
  }

  // 闪红提示：占用 millis() 而不是阻塞，和 loop 里其它所有事一样
  if (flashUntil != 0) {
    bool on = ((millis() / 120) % 2) == 0;
    RgbCycle::setColor(on ? 255 : 0, 0, 0);
    if (millis() >= flashUntil) {
      flashUntil = 0;
      showStatus();
    }
  }

  // 非阻塞节流测距，和 Day 25 一样
  if (millis() - lastMeasureMs >= MEASURE_MS) {
    lastMeasureMs = millis();
    lastCm = measureCm();
    sampleCount++;
  }

  // 掉线自动重连，沿用 Day 25
  static unsigned long lastReconnectMs = 0;
  if (WiFi.status() != WL_CONNECTED && millis() - lastReconnectMs > 5000) {
    lastReconnectMs = millis();
    Serial.println("Wi-Fi 掉线，重连中");
    WiFi.reconnect();
  }

  delay(10);
}
