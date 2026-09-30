// Day 28 实验 1：手机遥控 + 自动避障，一个界面两种开法
//
// Day 27 的车只能人开，Day 21 的车只能自己跑。今天把两套合成一个固件：
// 同一个 /cmd 路由、同一个心跳、同一个看门狗，只是多了一个"模式"。
//
//   ┌─ 面板 ─────────────────────────────────┐
//   │ [自动避障]   ← 点击切换，板子锁存这个模式  │
//   │        ▲                             │
//   │     ◀ ■停 ▶      ← 手动时才有效        │
//   │        ▼                             │
//   └──────────────────────────────────────┘
//
// 四个要点：
// ① 两套行为不能同时抢电机。模式是板子上唯一的状态源，
//    自动时方向键一律不理（收下了也不该执行），回手动时先停车再让人开。
// ② 自动模式下看门狗不能关。这是本固件最重要的一个决定：
//    人在屋里跟着车走，Wi-Fi 一掉手机就没法让它停了。
//    所以心跳的含义从"人在打方向"扩展成**"人还在，还要这个模式"**，
//    一跳没来 → 刹车 + 退回手动。看门狗保护的是模式，不只是方向。
// ③ 测距节奏统一成 100ms。手动时它只喂面板，200ms 够；
//    自动时它要驱动决策（Day 21 定的 100ms），两边取严的那个。
// ④ 串口退成只报决策。Day 21 每 500ms 报一次距离，今天不报了——
//    面板上距离一直在跳，串口该留给"为什么转、往哪转、为什么停"。
//
// 电路沿用 Day 24 定型的车，一个元件都没加：
//
//   TB6612FNG      ESP32-S3        HC-SR04        ESP32-S3
//   ─────────      ────────        ───────        ────────
//   AIN1/PWMA/AIN2  GPIO10 / 12 / 11    Trig ────────  GPIO4
//   BIN1/PWMB/BIN2  GPIO15 / 17 / 16    Echo ────────  GPIO5

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
const int DUTY_TURN   = 120;     // 转向固定用这个，不跟滑块走（Day 27 的决定）

// ---------- 测距参数 ----------
const float CM_PER_US = 0.0343 / 2.0;
const unsigned long TIMEOUT_US = 6000;    // ≈103cm
// 取 Day 21 的 100ms 而不是 Day 27 的 200ms：面板刷新是 500ms 一次，
// 手动时用 100ms 只是多测两次，自动时却必须要这么快才能及时避障。见要点 ③
const unsigned long MEASURE_MS = 100;
// 读数下限：低于 2cm 不可信（发射脉冲还没结束就收到回波）
const float MIN_VALID_CM = 2.0;

// ---------- 看门狗 ----------
// 500ms 太紧（一次请求 + 网络抖动就可能误停），3000ms 太松（够车冲出两米）。
// 1000ms 是"松手不发 stop 也能在一米内停住"的量级。
const unsigned long CMD_TIMEOUT_MS = 1000;

// ---------- 指令状态 ----------
// 枚举名避开 Arduino 核心宏 OK / LOW / HIGH
enum Cmd { CMD_STOP, CMD_FWD, CMD_BACK, CMD_LEFT, CMD_RIGHT };
enum Mode { MODE_MANUAL, MODE_AUTO };

Cmd cmdDir = CMD_STOP;          // 上电即停车，Day 27 要点 ③
int  cmdDuty = DUTY_CRUISE;
Mode cmdMode = MODE_MANUAL;     // 上电必须是手动。自动是一次明确的点击，不是默认态
bool carMoving = false;         // 两种模式都看这一个：车现在动没动
unsigned long lastCmdMs = 0;    // 任何一次被接受的 /cmd 到达都刷新
unsigned long timeoutStops = 0; // 看门狗停车次数，给页面和日志当证据
unsigned long flashUntil = 0;   // 看门狗停车后的闪红截止时刻，0 = 不闪

// ---------- 自动避障状态机（原样搬 Day 21 实验 2）----------
// 阈值迟滞：进 15cm 退 25cm，中间 10cm 死区让车不在临界点抽搐
const float NEAR_CM = 15.0;
const float FAR_CM  = 25.0;
// 动作时长：后退拉开距离 → 转向换方向 → 强制前进一段再重新看
const unsigned long BACK_MS   = 500;
const unsigned long TURN_MS   = 400;
const unsigned long COMMIT_MS = 300;

enum State { S_CRUISE, S_BACK, S_TURN, S_COMMIT };

State autoState = S_CRUISE;
unsigned long autoStateStart = 0;
bool turnLeftNext = true;       // 每次避障换一边转，否则越转越深
float blockedCm = 0;            // 触发避障的那次读数，留给日志
unsigned long avoidRound = 0;   // 避障轮次，给面板看

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
    digitalWrite(in1, LOW);  digitalWrite(in2, LOW);
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

// ---------- 灯色 ----------
// 手动和自动共用一块 RGB，所以颜色不能撞车：
//   绿/橙两种模式都有，含义一致（前进/后退），照用
//   转向：手动左右两个方向用品红和蓝，自动那一个转向用红 —— 红色只属于自动
// 于是脱离 USB 时看灯就能分辨"现在是谁在开"，不用猜
void showStatus() {
  if (cmdMode == MODE_AUTO) {
    switch (autoState) {
      case S_CRUISE: RgbCycle::setColor(0, 255, 0);   break;  // 绿：自动直行
      case S_BACK:   RgbCycle::setColor(255, 160, 0); break;  // 橙：自动后退
      case S_TURN:   RgbCycle::setColor(255, 0, 0);   break;  // 红：自动转向
      case S_COMMIT: RgbCycle::setColor(255, 255, 0); break;  // 黄：自动定向前进
    }
    return;
  }
  if (carMoving) {
    switch (cmdDir) {
      case CMD_FWD:   RgbCycle::setColor(0, 255, 0);   break;  // 绿：前进
      case CMD_BACK:  RgbCycle::setColor(255, 160, 0); break;  // 橙：后退
      case CMD_LEFT:  RgbCycle::setColor(255, 0, 255); break;  // 品红：左转
      case CMD_RIGHT: RgbCycle::setColor(0, 128, 255); break;  // 蓝：右转
      default: break;
    }
  } else {
    RgbCycle::setColor(0, 0, 0);                            // 暗：停着
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

const char* autoName(State s) {
  switch (s) {
    case S_BACK:   return "back";
    case S_TURN:   return "turn";
    case S_COMMIT: return "commit";
    default:       return "cruise";
  }
}

// ---------- 把 cmdDir / cmdDuty 变成实际的电机动作（仅手动模式调用）----------
// 方向和速度是两个独立状态：滑块只改 cmdDuty，心跳只发 dir。
// 两者分开放，页面不必在每次心跳里重复发一遍速度。
void applyMotion() {
  switch (cmdDir) {
    case CMD_FWD:  drive( cmdDuty,  cmdDuty);     carMoving = true;  break;
    case CMD_BACK: drive(-cmdDuty, -cmdDuty);     carMoving = true;  break;
    // 原地转向：两轮等速反向，左转 = 左轮退右轮进（同 Day 21 的 drive(-x, +x)）
    case CMD_LEFT:  drive(-DUTY_TURN, DUTY_TURN); carMoving = true;  break;
    case CMD_RIGHT: drive( DUTY_TURN, -DUTY_TURN); carMoving = true; break;
    default:        brakeAll();                   carMoving = false; break;
  }
  showStatus();
}

// 唯一的收车出口：不管在哪个模式、因为什么原因，都从这停下
void stopAll() {
  cmdDir = CMD_STOP;
  brakeAll();
  carMoving = false;
  showStatus();
}

// ---------- 模式 ----------
// 模式是板子上唯一的状态源。切自动立刻开始跑（用户刚点的，不是上电默认）；
// 切手动必须先停车再交还控制权，否则自动的那一脚油门会接着手动状态续下去。
void setMode(Mode m) {
  if (m == cmdMode) return;
  cmdMode = m;
  if (m == MODE_AUTO) {
    avoidRound = 0;
    turnLeftNext = true;
    enterAuto(S_CRUISE);
    Serial.println("模式 → 自动避障");
  } else {
    stopAll();          // 含 cmdDir = CMD_STOP，方向键不会接着自动的余速跑
    Serial.println("模式 → 手动（已停车）");
  }
}

// ---------- 自动避障 ----------
void enterAuto(State s) {
  autoState = s;
  autoStateStart = millis();
  switch (s) {
    case S_CRUISE:
      // 直行速度跟滑块走：一个旋钮同时管两种模式
      drive( cmdDuty,  cmdDuty);
      break;
    case S_BACK:
      drive(-cmdDuty, -cmdDuty);
      Serial.printf("第%lu轮｜前方 %.1fcm < %.0fcm，挡住了 → 后退 %dms\n",
                    avoidRound, blockedCm, NEAR_CM, BACK_MS);
      break;
    case S_TURN:
      if (turnLeftNext) drive(-DUTY_TURN,  DUTY_TURN);
      else              drive( DUTY_TURN, -DUTY_TURN);
      Serial.printf("第%lu轮｜原地%s转 %dms（左右交替，下一轮换另一边）\n",
                    avoidRound, turnLeftNext ? "左" : "右", TURN_MS);
      break;
    case S_COMMIT:
      drive( cmdDuty,  cmdDuty);
      Serial.printf("第%lu轮｜转向已转开，强制前进 %dms 再重新测距\n",
                    avoidRound, COMMIT_MS);
      break;
  }
  // 四个状态车都在动。退出手动时由 stopAll() 负责停，这里只置位
  carMoving = true;
  showStatus();
}

void autoTick() {
  if (cmdMode != MODE_AUTO) return;    // 被看门狗或停止键踢回手动就别再动
  unsigned long now = millis();
  unsigned long inState = now - autoStateStart;

  // 除了巡航，其余状态是"定时动作"：不测距、不打断
  switch (autoState) {
    case S_BACK:
      if (inState < BACK_MS) return;
      enterAuto(S_TURN);
      return;
    case S_TURN:
      if (inState < TURN_MS) return;
      turnLeftNext = !turnLeftNext;
      enterAuto(S_COMMIT);
      return;
    case S_COMMIT:
      if (inState < COMMIT_MS) return;
      enterAuto(S_CRUISE);
      return;
    case S_CRUISE:
      break;
  }

  // 巡航状态：按 MEASURE_MS 节流测距后判断
  if (now - lastMeasureMs < MEASURE_MS) return;
  lastMeasureMs = now;
  float cm = measureCm();
  sampleCount++;
  // 死区内按上次结论走，靠迟滞自然稳住：
  //   cm < NEAR → 进避障；cm > FAR → 继续直行；中间不动
  if (cm >= MIN_VALID_CM && cm < NEAR_CM) {
    blockedCm = cm;
    avoidRound++;
    enterAuto(S_BACK);
  }
}

float measureCm() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  unsigned long us = pulseIn(ECHO_PIN, HIGH, TIMEOUT_US);
  if (us == 0) return -1.0;       // pulseIn 超时返回 0，等价于"前方很远"
  return us * CM_PER_US;
}

// ---------- 控制面板 ----------
// 沿用 Day 25 的三条：整页一次发完、静态内容放 flash、页面和数据分开。
// 比 Day 27 只多一个"自动避障"开关和一行模式显示。
static const char PAGE[] PROGMEM = R"rawliteral(
<!doctype html><html lang="zh"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1,maximum-scale=1,user-scalable=no">
<title>遥控 / 自动避障小车</title>
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
 #auto{height:52px;font-size:16px;width:100%;margin-bottom:14px;background:#ecfdf5;border-color:#a7f3d0}
 #auto.on{background:#059669;border-color:#059669;color:#fff}
 .spacer{visibility:hidden}
 .row{display:flex;align-items:center;gap:10px;font-size:13px;color:#374151}
 input[type=range]{flex:1}
 .meta{font-size:12px;color:#9ca3af;margin-top:14px;line-height:1.7}
 #state{font-weight:600}
</style></head><body>
<h1>遥控 / 自动避障小车</h1>
<div class="sub">按住方向键行驶，松手即停；点「自动避障」让它自己跑。两种模式下松手 1 秒没指令都刹车</div>

<div class="card">
  <div class="label">前方距离</div>
  <div class="val" id="cm">--<small>cm</small></div>
  <div class="bar"><i id="cmbar" style="width:0%"></i></div>
</div>

<button id="auto">自动避障</button>

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
  状态 <span id="state">stop</span> ｜ 避障 <span id="av">0</span> 次 ｜ 超时停车 <span id="tmo">0</span> 次 ｜
  信号 <span id="rssi">--</span> dBm ｜ 在线 <span id="up">--</span> s<br>
  <code id="ip">--</code> ｜ <code>http://esp32s3.local</code>
</div>

<script>
const BEAT_MS = 250;          // 心跳间隔，必须小于板子的 1000ms 超时
let holding = null;           // 当前按住的方向，null = 没按
let mode = 'manual';          // 以板子说的为准，每 500ms 由 /data 校正一次
let beat = null;

// 自动状态机那四个动作，转成面板上的中文
const ACT = { cruise:'直行', back:'后退', turn:'转向', commit:'定向' };

function send(q) {
  // 遥控指令丢了不要重试轰炸：超时看门狗会兜底，重试只会让指令堆积
  fetch('/cmd?' + q).catch(() => {});
}

// 心跳什么时候该跑：手动时"按住方向键"算有人在开，自动时"自动开着"算。
// 空闲的手动模式不发——那时车停着，发不发都不影响安全。
function needBeat() { return holding !== null || mode === 'auto'; }
function syncBeat() {
  if (needBeat() && beat === null) { sendBeat(); beat = setInterval(sendBeat, BEAT_MS); }
  else if (!needBeat() && beat !== null) { clearInterval(beat); beat = null; }
}
// 手动按住时心跳顺带重申方向；自动时人没打方向，这就是一句纯"我还在"
function sendBeat() { send(holding ? 'dir=' + holding : 'beat=1'); }

function releaseHold() {
  if (holding === null) return false;
  holding = null;
  for (const id of ['fwd', 'back', 'left', 'right']) {
    document.getElementById(id).classList.remove('on');
  }
  send('dir=stop');
  return true;
}

function endHold() {
  // 页面隐藏 / 断线 / 按停止键走这条：不光松手，还要把自动模式一起退掉
  const wasActive = releaseHold() || mode === 'auto';
  if (mode === 'auto') { mode = 'manual'; send('mode=manual'); }
  syncBeat();
  return wasActive;
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
    if (mode === 'auto') return;      // 自动时方向键不理，别和状态机抢车
    holding = id;
    send('dir=' + id);
    syncBeat();
  });
  b.addEventListener('pointerup',     () => { b.classList.remove('on'); if (mode !== 'auto') endHold(); });
  b.addEventListener('pointercancel', () => { b.classList.remove('on'); if (mode !== 'auto') endHold(); });
  b.addEventListener('lostpointercapture', () => { b.classList.remove('on'); if (mode !== 'auto') endHold(); });
  // 安卓长按按钮会弹系统菜单，弹出来时 pointerup 就没了
  b.addEventListener('contextmenu', e => e.preventDefault());
}

document.getElementById('stop').addEventListener('click', endHold);

// 模式开关：点一下进自动，再点一下回手动。先松手再切，两个模式不抢车
document.getElementById('auto').addEventListener('click', () => {
  const next = (mode === 'auto') ? 'manual' : 'auto';
  releaseHold();
  mode = next;
  send('mode=' + next);
  showMode();
  syncBeat();
});

function showMode() {
  const b = document.getElementById('auto');
  b.classList.toggle('on', mode === 'auto');
  b.textContent = (mode === 'auto') ? '自动中 · 点击退出手动' : '自动避障';
}

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
    // 模式只认板子的说法：上一轮如果被看门狗踢回手动，这里会把按钮纠正过来
    mode = d.mode;
    showMode();
    syncBeat();
    document.getElementById('state').textContent =
      d.mode === 'auto' ? '自动·' + (ACT[d.act] || d.act) : d.dir;
    document.getElementById('av').textContent = d.av;
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

// ---------- 自动模式下滑动滑块 ----------
// 不等下一次状态切换，立刻改当前这一段的 duty —— 否则在自动模式里
// 拉滑块看起来像坏了。转向那一段用固定 DUTY_TURN（Day 27 的决定），不动它
void reapplyAutoDuty() {
  switch (autoState) {
    case S_CRUISE:
    case S_COMMIT: drive( cmdDuty,  cmdDuty); break;
    case S_BACK:   drive(-cmdDuty, -cmdDuty); break;
    default: break;   // S_TURN 用 DUTY_TURN
  }
}

// ---------- 路由 ----------
void handleRoot() {
  server.send(200, "text/html; charset=utf-8", String(FPSTR(PAGE)));
}

void handleData() {
  // 比 Day 27 多 mode / act / av 三个字段：自动模式下"车在干什么"
  // 是脱离 USB 时唯一能看出它在想什么的通道
  char buf[320];
  snprintf(buf, sizeof(buf),
           "{\"cm\":%.1f,\"rssi\":%ld,\"up\":%lu,\"n\":%lu,\"ip\":\"%s\","
           "\"dir\":\"%s\",\"duty\":%d,\"tmo\":%lu,"
           "\"mode\":\"%s\",\"act\":\"%s\",\"av\":%lu}",
           lastCm,
           WiFi.RSSI(),
           millis() / 1000,
           sampleCount,
           WiFi.localIP().toString().c_str(),
           cmdName(cmdDir),
           cmdDuty,
           timeoutStops,
           cmdMode == MODE_AUTO ? "auto" : "manual",
           cmdMode == MODE_AUTO ? autoName(autoState) : cmdName(cmdDir),
           avoidRound);
  server.send(200, "application/json", buf);
}

// 只认白名单里的值，别把 server.arg() 直接拿去用。
// 今天是驱动电机，一个没见过的字符串至少该被拒掉，而不是静默忽略
void handleCmd() {
  bool recognized = false;   // 这条请求到底有没有认出点什么
  bool changed = false;

  if (server.hasArg("dir")) {
    recognized = true;
    String d = server.arg("dir");
    Cmd next = cmdDir;
    if      (d == "fwd")   next = CMD_FWD;
    else if (d == "back")  next = CMD_BACK;
    else if (d == "left")  next = CMD_LEFT;
    else if (d == "right") next = CMD_RIGHT;
    else if (d == "stop")  next = CMD_STOP;
    else {
      server.send(400, "text/plain; charset=utf-8",
                  "400 — dir 只接受 fwd/back/left/right/stop");
      return;
    }
    // 自动模式下方向一律不理。收下了但不执行：面板的心跳里还带着 dir，
    // 拒掉会让心跳失效，看门狗反而把车停住
    if (cmdMode == MODE_MANUAL && next != cmdDir) {
      cmdDir = next;
      changed = true;
      Serial.printf("指令 → %s\n", cmdName(cmdDir));
    }
  }

  if (server.hasArg("speed")) {
    recognized = true;
    int v = server.arg("speed").toInt();
    if (v < DUTY_MIN || v > DUTY_MAX) {
      char msg[80];
      snprintf(msg, sizeof(msg), "400 — speed 要在 %d~%d 之间，收到 %d",
               DUTY_MIN, DUTY_MAX, v);
      server.send(400, "text/plain; charset=utf-8", msg);
      return;
    }
    if (v != cmdDuty) {
      cmdDuty = v;
      changed = true;      // 速度变了要立刻作用，不等下一次心跳或状态切换
      Serial.printf("速度 → %d\n", cmdDuty);
    }
  }

  if (server.hasArg("mode")) {
    recognized = true;
    String m = server.arg("mode");
    if (m == "auto") {
      // setMode 自己已经动过电机了，不置 changed
      setMode(MODE_AUTO);
    } else if (m == "manual") {
      if (cmdMode == MODE_AUTO) setMode(MODE_MANUAL);
    } else {
      server.send(400, "text/plain; charset=utf-8",
                  "400 — mode 只接受 auto/manual");
      return;
    }
  }

  // 纯心跳：不带任何意义，只表示客户端还在。
  // 自动模式下人没打方向，就靠它把看门狗喂住——见要点 ②
  if (server.hasArg("beat")) {
    recognized = true;
  }

  if (!recognized) {
    // 光秃秃一个 GET /cmd 不算活着，否则任何人都能靠空请求把车续着跑
    server.send(400, "text/plain; charset=utf-8",
                "400 — 至少带一个 dir / speed / mode / beat");
    return;
  }

  // 看门狗只认"被接受的指令"。上面几个 400 分支都是提前 return 的 ——
  // 一个只会发垃圾参数的客户端不该被当成还活着，
  // 否则它能靠刷无效请求一直把车续着跑。
  lastCmdMs = millis();

  // setMode() 自己已经动过电机了，这里只补速度和方向的即时生效
  if (changed) {
    if (cmdMode == MODE_MANUAL) applyMotion();
    else reapplyAutoDuty();
  }
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

  // 上电默认态 = 手动 + 停车。先把轮子刹住再去做别的事，
  // 包括连 Wi-Fi 那几秒——那期间手机连不进来，车没有任何理由动
  cmdMode = MODE_MANUAL;
  stopAll();

  Serial.println("Day28 实验1：手机遥控 + 自动避障");
  Serial.println("上电默认手动停车；松手/断线/超时都刹车，并退回手动");
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
  //
  // 自动模式下这条更要紧（要点 ②）：Wi-Fi 一掉手机就没法叫停它了，
  // 所以超时不但停车，还顺手退回手动，等手机回来看到的是干净的停车态。
  bool ctrlAlive = (WiFi.status() == WL_CONNECTED) &&
                   (millis() - lastCmdMs <= CMD_TIMEOUT_MS);
  if (carMoving && !ctrlAlive) {
    bool wasAuto = (cmdMode == MODE_AUTO);
    // 先退模式再停车：不先退的话，stopAll() 停完，下面 autoTick() 又会立刻接着跑
    if (wasAuto) cmdMode = MODE_MANUAL;
    stopAll();
    timeoutStops++;
    if (WiFi.status() != WL_CONNECTED) {
      Serial.printf("⚠ Wi-Fi 掉线，已刹车（第 %lu 次）\n", timeoutStops);
    } else {
      Serial.printf("⚠ %lu ms 没收到指令，已刹车（第 %lu 次）\n",
                    CMD_TIMEOUT_MS, timeoutStops);
    }
    if (wasAuto) Serial.println("⚠ 自动模式已退回手动");
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

  // ---------- 两种模式 ----------
  // 自动：状态机自己管测距和电机（含转向、后退的定时动作）
  // 手动：只测距喂面板，电机完全听 /cmd 的
  //
  // 后退/转向/定向那三段不测距，所以面板上的距离是进入避障那一刻的值，
  // 会停最多 1.2 秒不刷。这不是 bug：转的时候测到的是侧面的墙，
  // 拿它判断"前方通没通"是错的（Day 21 要点 ② 的延伸）
  if (cmdMode == MODE_AUTO) {
    autoTick();
  } else if (millis() - lastMeasureMs >= MEASURE_MS) {
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
