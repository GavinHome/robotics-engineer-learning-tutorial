// Day 35：开环巡线
//
// 今天不写控制律。只用"线在哪一边"打方向，固定 duty 差，先把"哪偏哪纠"的
// 方向验证对，再看这个开关逻辑在哪里丢线、怎么摆。P 控制是 Day 36。
//
// 接线 = Day 34 的五路 AO + 五路 DO 一根不动，再加上 Day 21 已经调好的 TB6612：
//
//   TCRT5000        ESP32-S3
//   ─────────       ────────
//   0~4 号 AO ───── GPIO6 / 7 / 8 / 2 / 3   （ADC1，Day 33）
//   0~4 号 DO ───── GPIO1 / 13 / 14 / 21 / 47 （Day 34）
//
//   TB6612          ESP32-S3
//   ──────          ────────
//   STBY  ───────── 3V3（常高，不接 GPIO 省一个脚）
//   AIN1/AIN2/PWMA  GPIO10 / 11 / 12   ← 左轮
//   BIN1/BIN2/PWMB  GPIO15 / 16 / 17   ← 右轮
//
//   ⚠️ 左轮右轮的 AO/DO 顺序不能反：W[] 按 0 号最左排，反一路 pos 的符号就反了。
//   ⚠️ TB6612 这六根线必须和 Day 21 完全一致，否则今天的 duty 标定值全部作废。
//
// 今天要立住的三件事：
//   ① 方向：pos < 0（线在左）→ 左轮减速 / pos > 0（线在右）→ 右轮减速。
//      方向做成 steerSign 一个常数，串口按 s 就能取反——不用烧录就能试另一个方向。
//   ② 固定 duty 差，不用 map() 把 pos 线性映射到 duty。轮子响应是非线性的，
//      今天故意只用三档（巡航 / 慢速 / 起步冲击），把线性假设留给 Day 36 去破。
//   ③ 丢线还是 Day 34 那套：全白或全黑 → 保持方向记时长 → 500ms → 刹车上报。
//      今天多了轮子，"停车"是真的刹住，不是只打个日志。
//
// 串口命令（115200）：
//   c = 标定白底   b = 标定黑底   w = 总表   t = 量电位器阈值
//   p = 曲线模式   q = 静音（关掉每帧 raw 日志）
//   s = 转向方向取反        + / - = 巡航 duty 加减 5
//   [ / ] = 死区加减 25
// 命令大小写都认。
// 彩灯：绿=直行、红=左弧线、蓝=右弧线、白=丢线（保持方向）、红闪=已刹停、黄=正在量电位器
//
// 持久化：标定表、转向方向、duty/死区都存 flash（NVS）。按过 c/b/s/+/-/[/] 就存，
// 下次上电自动恢复——车可以脱离电脑直接跑。白底会漂（3 分钟 ~250 码），
// 换环境、换光照、改高度之后重按 c/b/t 覆盖。

#include <RgbCycle.h>
#include <Preferences.h>

// ---------- 五路传感器 ----------
const int AO_PIN[5]     = { 6, 7, 8, 2, 3 };
const char* const AO_NAME[5] = { "6", "7", "8", "2", "3" };
const int DO_PIN[5]     = { 1, 13, 14, 21, 47 };
const char* const DO_NAME[5] = { "1", "13", "14", "21", "47" };
const int W[5] = { -2, -1, 0, 1, 2 };   // 0 号最左

// ---------- TB6612（Day 19/21 的线不动） ----------
const int AIN1 = 10, AIN2 = 11, PWMA = 12;   // 左轮
const int BIN1 = 15, BIN2 = 16, PWMB = 17;   // 右轮
const int PWM_FREQ = 20000;   // 超人耳，消啸叫；也顺带减小电流纹波
const int PWM_RES  = 8;       // 0~255

// ---------- 占空比（Day 21 在 6V 下标出来的，今天直接沿用） ----------
// 巡航这一档做成变量：串口 +/- 能运行时改，换电池、换地面不用重新烧录。
int  dutyCruise = 115;   // 巡航：留一档余量
const int DUTY_SLOW   = 75;    // 弧线内轮，故意压在起转点以下，外轮拖着走
const int DUTY_KICK   = 150;   // 起步冲击：短暂超压顶开静摩擦
const unsigned long KICK_MS = 200;
const int TRIM_RIGHT  = 0;     // 车往左偏 → 右轮慢 → 加正数补偿（两个 TT 马达不一样快）

// ---------- 开环转向 ----------
// 今天只有三档，没有比例项。dir = pos 的符号 × steerSign。
// steerSign = +1：pos<0（线在左）→ 左轮减速。按 s 取反，用来验证方向。
int  steerSign = +1;
// |pos| 小于死区算居中，两轮同速直行。死区太小会在线上抖，太大会该拐不拐。
int  deadband = 150;

// ---------- 采样 ----------
const unsigned long PERIOD_MS  = 100;
const unsigned long TEACH_MS   = 20;
const int   CAL_SAMPLES = 200;
const int   CAL_GAP_MS  = 5;

// ---------- 丢线 ----------
const unsigned long LOST_TIMEOUT_MS = 500;
const unsigned long RECOVER_MS      = 300;
// 阈值离靶心（白黑中点）最多允许偏这么多：1/6 量程。
const int OFF_MID_DIV = 6;
const unsigned long TABLE_QUIET_MS = 5000;

// ---------- 标定表 ----------
int  whiteRaw[5] = { 1600, 1600, 1600, 1600, 1600 };
int  blackRaw[5] = { 4095, 4095, 4095, 4095, 4095 };
int  doAtWhite[5] = { 0, 0, 0, 0, 0 };
int  doAtBlack[5] = { 1, 1, 1, 1, 1 };
int  lineLevel[5];
bool doSplit[5];
bool calibrated = false;

// ---------- 运行状态 ----------
enum RunState { S_WAIT, S_FOLLOW, S_LOST, S_STOP };
const char* const STATE_NAME[4] = { "等标定", "跟线", "丢线", "停车" };

int   raw[5];
float n[5];
long  pos     = 0;
long  lastPos = 0;
bool  onLine[5];
int   onCount = 0;
bool  lostAllWhite = false;
RunState state = S_WAIT;
unsigned long lostSince    = 0;
unsigned long recoverSince = 0;
unsigned long stopSince    = 0;
unsigned long lastReport   = 0;
bool  plotMode  = false;
bool  teachMode = false;
bool  frameLog  = true;

// ---------- t 命令：量电位器 ----------
const int TEACH_MAX = 16;
int  risePts[5][TEACH_MAX];
int  fallPts[5][TEACH_MAX];
int  riseCnt[5];
int  fallCnt[5];
bool prevOn[5];
unsigned long tableQuietUntil = 0;

int median(int* v, int cnt) {
  for (int a = 1; a < cnt; a++) {
    int k = v[a], b = a - 1;
    while (b >= 0 && v[b] > k) { v[b + 1] = v[b]; b--; }
    v[b + 1] = k;
  }
  return v[cnt / 2];
}

// ---------------------------------------------------------------- 电机
// speed: -255 ~ +255。0 = 滑行（双高阻），不是刹车。
void driveMotor(int in1, int in2, int pwmPin, int speed) {
  if (speed > 0) {
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
  } else if (speed < 0) {
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
  } else {
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
    ledcWrite(pwmPin, 0);
    return;
  }
  ledcWrite(pwmPin, abs(speed));
}

// 刹车：两端同时拉到同一电平（短接）+ PWM 非 0。
// ⚠️ PWM=0 时输出高阻，与 IN 无关 —— "IN1=IN2=1 但 PWM=0" 依然是滑行。
void brakeMotor(int in1, int in2, int pwmPin) {
  digitalWrite(in1, HIGH);
  digitalWrite(in2, HIGH);
  ledcWrite(pwmPin, 255);
}

int curL = 0, curR = 0;   // 当前两轮 duty，日志和起步冲击要用

void setWheels(int left, int right) {
  left  = constrain(left,  -255, 255);
  right = constrain(right, -255, 255);
  curL = left;
  curR = right;
  driveMotor(AIN1, AIN2, PWMA, left);
  driveMotor(BIN1, BIN2, PWMB, right);
}

// 起步冲击：从静止启动那一帧给一档高压，200ms 后落到目标值。
// 不加这段两个轮子容易卡在静摩擦上嗡嗡响。只在"从不动到动"时用一次，
// 不在每次修正时用——否则车会一抽一抽，今天要观察的摆动就被它盖住了。
bool kicked = false;
void kickIfNeeded(int left, int right) {
  if (kicked || (left == 0 && right == 0)) return;
  kicked = true;
  setWheels(left  > 0 ?  DUTY_KICK : -DUTY_KICK,
            right > 0 ?  DUTY_KICK : -DUTY_KICK);
  delay(KICK_MS);
}

// ---------------------------------------------------------------- 彩灯

void showLed(int r, int g, int b) { RgbCycle::setColor(r, g, b); }

void showState() {
  if (!calibrated) { showLed(0, 255, 0); return; }              // 绿：等标定
  if (teachMode)   { showLed(255, 255, 0); return; }            // 黄：量电位器
  if (state == S_STOP) {
    showLed((millis() / 250) % 2 == 0 ? 255 : 0, 0, 0);         // 红闪：已刹停
    return;
  }
  if (state == S_LOST) { showLed(255, 255, 255); return; }      // 白：丢线，保持方向
  if (curL < curR - 10) { showLed(255, 0, 0); return; }         // 红：左弧线
  if (curR < curL - 10) { showLed(0, 0, 255); return; }         // 蓝：右弧线
  showLed(0, 255, 0);                                           // 绿：直行
}

// ---------------------------------------------------------------- 表

void printTables() {
  tableQuietUntil = millis() + TABLE_QUIET_MS;
  Serial.println();
  Serial.println("  路  AO脚  DO脚    白底   黑底   量程  建议阈值   DO白  DO黑  电位器");
  Serial.println("----------------------------------------------------------------------------");
  for (int i = 0; i < 5; i++) {
    int target = (whiteRaw[i] + blackRaw[i]) / 2;
    Serial.printf("  %d   %-4s  %-4s  %5d  %5d  %5d  %6d     %d     %d   %s\n",
                  i, AO_NAME[i], DO_NAME[i],
                  whiteRaw[i], blackRaw[i], blackRaw[i] - whiteRaw[i], target,
                  doAtWhite[i], doAtBlack[i],
                  doSplit[i] ? "已分开" : "没分开!");
  }
  Serial.println("----------------------------------------------------------------------------");
  Serial.println("  建议阈值 = 白底和黑底的中点。离白、离黑各半个量程，是最稳的位置。");
  Serial.println("  DO白/DO黑 是标定时记下的电平：两者相同 = 这颗电位器没把黑白分开。");
  Serial.println();
}

// ---------------------------------------------------------------- 标定值持久化
// 标定表 + 转向方向 + 串口调出来的 duty/死区，全部存 flash（NVS）。
// 按过 c / b / s / +/- / [/] 之后自动存；下次上电自动恢复，
// 车可以脱离电脑直接跑。白底本身会漂（3 分钟 ~250 码），
// 换环境、换光照、改高度之后要重按 c / b / t 覆盖旧值。
Preferences calPref;

void savePrefs() {
  calPref.begin("day35", false);
  calPref.putBytes("white", whiteRaw,  sizeof(whiteRaw));
  calPref.putBytes("black", blackRaw,  sizeof(blackRaw));
  calPref.putBytes("doW",   doAtWhite, sizeof(doAtWhite));
  calPref.putBytes("doB",   doAtBlack, sizeof(doAtBlack));
  calPref.putBytes("lvl",   lineLevel, sizeof(lineLevel));
  calPref.putBytes("split", doSplit,   sizeof(doSplit));
  calPref.putBool("cal",    calibrated);
  calPref.putInt("steer",   steerSign);
  calPref.putInt("cruise",  dutyCruise);
  calPref.putInt("dead",    deadband);
  calPref.end();
}

bool loadPrefs() {
  calPref.begin("day35", true);
  bool ok = calPref.getBool("cal", false);
  if (ok) {
    calPref.getBytes("white", whiteRaw,  sizeof(whiteRaw));
    calPref.getBytes("black", blackRaw,  sizeof(blackRaw));
    calPref.getBytes("doW",   doAtWhite, sizeof(doAtWhite));
    calPref.getBytes("doB",   doAtBlack, sizeof(doAtBlack));
    calPref.getBytes("lvl",   lineLevel, sizeof(lineLevel));
    calPref.getBytes("split", doSplit,   sizeof(doSplit));
    steerSign  = calPref.getInt("steer",  steerSign);
    dutyCruise = calPref.getInt("cruise", dutyCruise);
    deadband   = calPref.getInt("dead",    deadband);
  }
  calPref.end();
  calibrated = ok;
  return ok;
}

// ---------------------------------------------------------------- 标定

void calibrate(bool black) {
  Serial.printf("[cal] 五路正在%s上取均值：保持 3cm 高度、别动、别挡光，约 1 秒\n",
                black ? "黑底" : "白底");
  long accAO[5] = { 0, 0, 0, 0, 0 };
  int  cntDO[5] = { 0, 0, 0, 0, 0 };
  for (int k = 0; k < CAL_SAMPLES; k++) {
    for (int i = 0; i < 5; i++) {
      accAO[i] += analogRead(AO_PIN[i]);
      cntDO[i] += digitalRead(DO_PIN[i]);
    }
    delay(CAL_GAP_MS);
  }
  Serial.println();
  for (int i = 0; i < 5; i++) {
    if (black) {
      blackRaw[i]  = (int)(accAO[i] / CAL_SAMPLES);
      doAtBlack[i] = (cntDO[i] * 2 > CAL_SAMPLES) ? 1 : 0;
    } else {
      whiteRaw[i]  = (int)(accAO[i] / CAL_SAMPLES);
      doAtWhite[i] = (cntDO[i] * 2 > CAL_SAMPLES) ? 1 : 0;
    }
    Serial.printf("  %d 号 AO GPIO%-3s %s = %4d   DO GPIO%-3s = %d\n",
                  i, AO_NAME[i], black ? "black" : "white",
                  black ? blackRaw[i] : whiteRaw[i],
                  DO_NAME[i], black ? doAtBlack[i] : doAtWhite[i]);
  }
  calibrated = true;
  Serial.println();
  printTables();

  int splitCount = 0;
  for (int i = 0; i < 5; i++) {
    doSplit[i]   = (doAtWhite[i] != doAtBlack[i]);
    lineLevel[i] = doAtBlack[i];
    if (doSplit[i]) splitCount++;
  }
  if (splitCount == 5) {
    Serial.println("[cal] 五路 DO 的黑白电平都分开了，极性已定。按 t 量电位器的实际阈值。");
  } else {
    Serial.printf("[cal] 只有 %d/5 路的 DO 把黑白分开了。没分开的那几路，\n", splitCount);
    Serial.println("[cal] 把纸放在它的黑白之间慢慢扫，同时转那颗电位器，直到串口的 [1 0 ...] 会翻。");
  }
  Serial.println();
  savePrefs();
  Serial.println("[nvs] 标定已存 flash：下次上电直接用它，不用带电脑。");
  Serial.println();
}

// ---------------------------------------------------------------- t：量电位器

void teachStart() {
  teachMode = true;
  for (int i = 0; i < 5; i++) {
    riseCnt[i] = fallCnt[i] = 0;
    prevOn[i] = (digitalRead(DO_PIN[i]) == lineLevel[i]);
  }
  showState();
  Serial.println();
  Serial.println("[teach] 把纸从 ① 白底区慢慢横扫过 ② 黑块，再扫回来，做两个来回。");
  Serial.println("[teach] 每路 DO 翻转时我会记下那一刻的 AO —— 那就是这颗电位器的阈值。");
  Serial.println("[teach] 扫慢点：一颗管子 1 秒左右。按任意键结束并打印结果。");
  Serial.println();
}

void teachStop() {
  teachMode = false;
  tableQuietUntil = millis() + TABLE_QUIET_MS;
  Serial.println();
  Serial.println("  路    阈值    靶心  离靶心     n  散布  判定");
  Serial.println("-------------------------------------------------------------");
  for (int i = 0; i < 5; i++) {
    int nr = riseCnt[i], nf = fallCnt[i], total = nr + nf;
    char sSpread[8];

    if (total == 0) {
      Serial.printf("  %d  %5s  %6s  %7s  %5s  %4s  没扫到，重扫\n",
                    i, "-", "-", "-", "-", "-");
      continue;
    }

    int buf[2 * TEACH_MAX];
    for (int k = 0; k < nr; k++) buf[k] = risePts[i][k];
    for (int k = 0; k < nf; k++) buf[nr + k] = fallPts[i][k];
    int thr  = median(buf, total);
    int spread = buf[total - 1] - buf[0];
    snprintf(sSpread, sizeof(sSpread), "%d", spread);

    char verdict[48];
    int mid    = (whiteRaw[i] + blackRaw[i]) / 2;
    int span   = blackRaw[i] - whiteRaw[i];
    int offMid = thr - mid;
    int limit  = span / OFF_MID_DIV;
    int nPct   = (thr - whiteRaw[i]) * 100 / span;
    if (!calibrated)             strcpy(verdict, "没标定，判不了");
    else if (thr <= whiteRaw[i]) strcpy(verdict, "阈值掉到白底以下，永远报黑");
    else if (thr >= blackRaw[i]) strcpy(verdict, "阈值顶到黑底以上，永远报白");
    else if (offMid < -limit)
      snprintf(verdict, sizeof(verdict), "偏低 %d 码，往上调", -offMid);
    else if (offMid >  limit)
      snprintf(verdict, sizeof(verdict), "偏高 %d 码，往下调", offMid);
    else
      snprintf(verdict, sizeof(verdict), "OK");

    Serial.printf("  %d  %5d  %6d  %+7d  0.%02d  %4s  %s\n",
                  i, thr, mid, offMid, nPct, sSpread, verdict);
  }
  Serial.println("-------------------------------------------------------------");
  Serial.println("  阈值 = DO 翻转那一刻的 AO 取中位数。n=0.50 才是正中间。");
  Serial.println("  偏低往上调、偏高往下调；散布到几百码 = 那一路扫快了，信中位数。");
  Serial.println();
  showState();
}

void teachPoll() {
  for (int i = 0; i < 5; i++) {
    int  ao = analogRead(AO_PIN[i]);
    bool on = (digitalRead(DO_PIN[i]) == lineLevel[i]);
    if (on && !prevOn[i]) {
      if (riseCnt[i] < TEACH_MAX) risePts[i][riseCnt[i]++] = ao;
      if (frameLog) Serial.printf("[teach] 路 %d  DO 白→黑 @ AO %4d\n", i, ao);
    }
    if (!on && prevOn[i]) {
      if (fallCnt[i] < TEACH_MAX) fallPts[i][fallCnt[i]++] = ao;
      if (frameLog) Serial.printf("[teach] 路 %d  DO 黑→白 @ AO %4d\n", i, ao);
    }
    prevOn[i] = on;
  }
}

// ---------------------------------------------------------------- 开环转向

// 今天全部的方向逻辑就这一个函数。三档，没有比例项。
void applySteer(bool lost) {
  int rl, rr;
  int dir = (pos < 0 ? -1 : +1) * steerSign;   // pos<0 = 线在左

  if (lost) {
    // 丢线：不动方向，两轮给一样的 duty 继续走，等超时刹停
    rl = rr = dutyCruise;
  } else if (labs(pos) <= deadband) {
    rl = rr = dutyCruise;                     // 直行
  } else if (dir < 0) {
    rl = DUTY_SLOW;  rr = dutyCruise;         // 左弧线：左轮减速
  } else {
    rl = dutyCruise; rr = DUTY_SLOW;          // 右弧线：右轮减速
  }
  rr += TRIM_RIGHT;

  if (rl == 0 && rr == 0) { kicked = false; setWheels(0, 0); return; }
  kickIfNeeded(rl, rr);
  setWheels(rl, rr);
}

// ---------------------------------------------------------------- 命令

void handleCmd(char c) {
  if (c == 'c') { calibrate(false); return; }
  if (c == 'b') { calibrate(true);  return; }
  if (c == 'w') { printTables(); return; }
  if (c == 'p') {
    plotMode = !plotMode;
    Serial.printf("[plot] %s\n", plotMode ? "开：只打 pos 和 压线路数×250" : "关");
    return;
  }
  if (c == 't') {
    if (teachMode) teachStop();
    else           teachStart();
    return;
  }
  if (c == 'q') {
    frameLog = !frameLog;
    Serial.printf("[log] 串口%s。%s\n",
                  frameLog ? "恢复" : "静音",
                  frameLog ? "每帧 raw 日志和 [teach] 行又回来了"
                           : "之后只剩状态变化和你按出来的表，不会再滚屏");
    return;
  }
  if (c == 's') {
    steerSign = -steerSign;
    savePrefs();
    Serial.printf("[steer] 方向取反：现在 pos<0 → %s\n",
                  steerSign > 0 ? "左轮减速（左弧线）" : "右轮减速（右弧线）");
    return;
  }
  if (c == '+' || c == '=') {
    dutyCruise = min(255, dutyCruise + 5);
    savePrefs();
    Serial.printf("[duty] 巡航 = %d\n", dutyCruise);
    return;
  }
  if (c == '-' || c == '_') {
    dutyCruise = max(0, dutyCruise - 5);
    savePrefs();
    Serial.printf("[duty] 巡航 = %d\n", dutyCruise);
    return;
  }
  if (c == '[' || c == '{') {
    deadband = max(0, deadband - 25);
    savePrefs();
    Serial.printf("[dead] 死区 = %d\n", deadband);
    return;
  }
  if (c == ']' || c == '}') {
    deadband = min(1000, deadband + 25);
    savePrefs();
    Serial.printf("[dead] 死区 = %d\n", deadband);
    return;
  }
}

// ---------------------------------------------------------------- setup

void setup() {
  Serial.begin(115200);

  RgbCycle::begin();

  analogReadResolution(12);
  for (int i = 0; i < 5; i++) {
    pinMode(AO_PIN[i], INPUT);
    pinMode(DO_PIN[i], INPUT_PULLUP);
    lineLevel[i] = 1;
    doSplit[i]   = false;
    prevOn[i]    = false;
  }

  pinMode(AIN1, OUTPUT); pinMode(AIN2, OUTPUT);
  pinMode(BIN1, OUTPUT); pinMode(BIN2, OUTPUT);
  ledcAttach(PWMA, PWM_FREQ, PWM_RES);
  ledcAttach(PWMB, PWM_FREQ, PWM_RES);
  setWheels(0, 0);

  showState();

  Serial.println("TCRT5000 五路 + TB6612 开环巡线 ready.");
  Serial.println("AO：#0=GPIO6  #1=GPIO7  #2=GPIO8  #3=GPIO2  #4=GPIO3");
  Serial.println("DO：#0=GPIO1  #1=GPIO13 #2=GPIO14 #3=GPIO21 #4=GPIO47");
  Serial.println("TB6612：左轮 10/11/12  右轮 15/16/17  STBY=3V3");
  Serial.printf("duty：巡航 %d / 慢速 %d / 冲击 %d（Day 21 的 6V 标定值）\n",
                dutyCruise, DUTY_SLOW, DUTY_KICK);
  Serial.println("命令：c=标白底 b=标黑底 w=总表 t=量电位器 p=曲线 q=静音");
  Serial.println("      s=转向取反  +/-=巡航duty  [/]=死区");
  Serial.println("先按 c 再按 b，然后按 t 确认五路 OK，再放到线上看方向。");
  Serial.println("方向反了就按 s —— 不用重新烧录。");
  if (loadPrefs()) {
    Serial.println("[nvs] 已恢复上次的标定 + 方向，直接上线跑。");
    Serial.println("[nvs] 换环境 / 改高度 / 白底漂了 → 重按 c/b/t；方向不对 → 按 s。");
  } else {
    Serial.println("[nvs] flash 里没有标定，先按 c 再按 b，然后 t。");
  }
  delay(200);
}

// ---------------------------------------------------------------- loop

void loop() {
  while (Serial.available() > 0) {
    char c = (char)Serial.read();
    if (c == '\n' || c == '\r' || c == ' ') continue;
    if (c >= 'A' && c <= 'Z') c += 32;
    if (teachMode && c != 't' && c != 'q') { teachStop(); continue; }
    handleCmd(c);
  }

  static unsigned long last = 0;
  unsigned long now = millis();
  unsigned long period = teachMode ? TEACH_MS : PERIOD_MS;
  if (now - last < period) return;
  last = now;

  if (teachMode) { teachPoll(); return; }

  for (int i = 0; i < 5; i++) {
    doSplit[i]   = (doAtWhite[i] != doAtBlack[i]);
    lineLevel[i] = doAtBlack[i];
  }

  for (int i = 0; i < 5; i++) raw[i] = analogRead(AO_PIN[i]);

  onCount = 0;
  for (int i = 0; i < 5; i++) {
    onLine[i] = (digitalRead(DO_PIN[i]) == lineLevel[i]);
    if (onLine[i]) onCount++;
  }

  lostAllWhite = (onCount == 0);
  bool lost = (onCount == 0) || (onCount == 5);

  if (!lost) {
    float wsum = 0.0f;
    float sumN = 0.0f;
    for (int i = 0; i < 5; i++) {
      int span = blackRaw[i] - whiteRaw[i];
      if (span < 1) span = 1;
      n[i] = (float)(raw[i] - whiteRaw[i]) / (float)span;
      if (n[i] < 0.0f) n[i] = 0.0f;
      sumN += n[i];
      wsum += W[i] * n[i];
    }
    if (sumN < 1e-3f) sumN = 1e-3f;
    pos = (long)(wsum / sumN * 500.0f);
    lastPos = pos;

    if (state != S_FOLLOW && calibrated) {
      if (state == S_STOP && (now - recoverSince) < RECOVER_MS) {
        // 刚从刹停恢复，要连续看见线 RECOVER_MS 才认
      } else {
        Serial.printf("[state] %s → 跟线%s\n", STATE_NAME[state],
                      state == S_WAIT ? "" : "（线回来了）");
        state = S_FOLLOW;
      }
    }
    lostSince = 0;
  } else {
    if (state != S_LOST && state != S_STOP && calibrated) {
      state = S_LOST;
      lostSince = now;
      Serial.printf("[state] 跟线 → 丢线（%s）保持 pos=%+ld\n",
                    lostAllWhite ? "五路全白" : "五路全黑", lastPos);
    }
    if (state == S_LOST && (now - lostSince) >= LOST_TIMEOUT_MS) {
      state = S_STOP;
      recoverSince = now;
      stopSince = now;
      Serial.printf("[state] 丢线 → 刹停：连续 %lums 没看见线，两轮短接。\n", LOST_TIMEOUT_MS);
      Serial.printf("[report] 最后位置 pos=%+ld，%s。等线回来或复位。\n",
                    lastPos, lastPos < 0 ? "线在左边离开的" : (lastPos > 0 ? "线在右边离开的" : "线在正中间离开的"));
    }
    if (state == S_STOP) recoverSince = now;
  }

  if (state == S_STOP) {
    setWheels(0, 0);
    brakeMotor(AIN1, AIN2, PWMA);
    brakeMotor(BIN1, BIN2, PWMB);
    curL = curR = 0;
    kicked = false;
  } else if (!calibrated) {
    setWheels(0, 0);          // 没标定不许动：停在 S_WAIT 等 c/b
    curL = curR = 0;
    kicked = false;
  } else {
    applySteer(lost);
  }

  showState();

  if (state == S_STOP && (now - lastReport) >= 1000) {
    lastReport = now;
    Serial.printf("[report] 仍刹停，已 %lus。最后 pos=%+ld，%s。\n",
                  (now - stopSince) / 1000, lastPos,
                  lostAllWhite ? "五路全白" : "五路全黑");
  }

  if (plotMode) {
    Serial.printf("%ld %d\n", pos, onCount * 250);
  } else if (!frameLog || now < tableQuietUntil) {
    // 静音中，或表刚打完：别被每帧行冲走
  } else {
    const char* act = "直行";
    if (state == S_STOP)    act = "刹停";
    else if (state == S_LOST) act = "丢线保持";
    else if (curL < curR - 10) act = "左弧线";
    else if (curR < curL - 10) act = "右弧线";
    Serial.printf("[%d %d %d %d %d] n=%d%s | raw %4d %4d %4d %4d %4d | pos %+5ld %s L%3d R%3d %s",
                  onLine[0], onLine[1], onLine[2], onLine[3], onLine[4], onCount,
                  lost ? (lostAllWhite ? " 全白" : " 全黑") : "",
                  raw[0], raw[1], raw[2], raw[3], raw[4], pos,
                  act, curL, curR, STATE_NAME[state]);
    if (state == S_LOST) {
      Serial.printf(" 丢线 %lums（保持 %+ld %s）", now - lostSince, lastPos,
                    lastPos < 0 ? "偏左" : (lastPos > 0 ? "偏右" : "正中"));
    } else if (state == S_STOP) {
      Serial.printf(" 已刹停");
    }
    Serial.println();
  }
}
