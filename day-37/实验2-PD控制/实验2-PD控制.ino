// Day 37：加入 D 项
//
// Day 36 把"线在哪"变成了连续的转向量 turn = Kp × pos，架起实测五档全在线性区、
// 无限幅。但 P 有个天生治不了的毛病：它只看**当前偏差**，不看偏差在怎么变。
// 车冲到线跟前时偏差正在飞快缩小，P 给的修正量却按"当前偏差"算，还是那么大——
// 于是冲过去；冲过去偏差反向变大，P 又给一大坨，于是摆回来。Day 35 那个
// 2 秒一周的摆动，根子在这。
//
// 今天加第二项：
//
//   turn = Kp × pos + Kd × (pos − pos_prev) / dt
//
// 第二项只看偏差的**变化率**。偏差在缩小 → 它给一个反向的量，把 P 那个
// "按当前偏差算的大修正"抵掉一部分，车就不冲过头。这就是"阻尼"。
//
// 今天要立住的四件事：
//   ① d = Kd × Δpos / dt，只对变化率起作用。**偏差不动 → d 恒等于 0**，
//      和 Kd 多大无关。架起拿黑道具静止压着就能验：Kd 从 0 加到 0.040，
//      日志里 D 那一列永远是 0。
//   ② D 单独试：Kp 归零只留 D（串口 o）。车偏出去不会自己回来——D 是阻尼
//      不是纠偏，它只能让已有的运动慢下来，不能产生指向线的运动。
//   ③ P + D 一起调，看摆振幅度的衰减（⏳ 要地面跑）。
//   ④ D 是噪声放大器。串口 n 量 AO 噪声，直接算"这一档 Kd 会把噪声放大成
//      多大的 D 输出"，和 P 项的输出放一起比。这是今天唯一能在架起做完的证据。
//
// dt 用的是真实帧间隔，不是写死的 0.1。控制周期是 100ms，所以
//   d = Kd × Δpos / 0.1 = Kd × Δpos × 10
// Δpos 的量纲和 pos 一样是 ±1000。Day 35 那个摆动（2 秒一周、±800）在
// 1/4 周期（500ms）里走完 800，摊到 100ms 一帧约 160，所以 Kd = 0.010 时
// d ≈ 16，和 Kp=0.03 在 pos=800 时的 turn=24 同一量级——这就是 Kd 的工作区间。
//
// 接线 = Day 36 一根不动：
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
//   ⚠️ TB6612 这六根线必须和 Day 21 完全一致，否则 duty 标定值全部作废。
//
// 串口命令（115200）：
//   c = 标定白底   b = 标定黑底   w = 总表   t = 量电位器阈值
//   p = 曲线模式   q = 静音（关掉每帧 raw 日志）   s = 转向方向取反
//   + / - = 巡航 duty ±5      [ / ] = 死区 ±25
//   ---- Kp（Day 36 原样）----
//   1~5 = 第 N 档 Kp          k / j = Kp 微调 ±0.001
//   ---- Kd（今天新增）----
//   6~0 = 第 N 档 Kd（0.000 / 0.005 / 0.010 / 0.020 / 0.040）
//   d / f = Kd 微调 ±0.001    o = 只留 D（Kp 临时归零，再按恢复）
//   n = 量 AO 噪声 + 算 D 项输出（今天的关键证据）
// 命令大小写都认。
// 彩灯：绿=直行、红=左弧线、蓝=右弧线、白=丢线（保持方向）、红闪=已刹停、黄=正在量电位器
//
// 持久化：标定表、方向、Kp、Kd、duty、死区全存 flash（NVS）。
// 命名空间延用 Day 35 的 "day35"——同一台车同一高度，标定表就是同一份。
// 新增 "kd" 键，Day 36 的固件没有它，读不到就用默认的 D 关。
//
// ⚠️ 换环境 / 改光照 / 动过高度 → 白底会漂，必须重按 c/b 覆盖。

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
int  dutyCruise = 115;   // 前进基准。**不随误差变化**——这就是"前进量/转向量分离"
const int DUTY_KICK   = 150;   // 起步冲击：短暂超压顶开静摩擦
const unsigned long KICK_MS = 200;
const int TRIM_RIGHT  = 0;     // 车往左偏 → 右轮慢 → 加正数补偿（两个 TT 马达不一样快）

// ---------- P 项 ----------
// Kp 用整数存：kpNum = 1000 表示 Kp = 1.000。
int  kpNum = 30;   // 默认第 3 档：Kp = 0.030
int  kpNumSaved = 30;   // 按 o 进"只留 D"之前记着这一档，再按 o 回去
const int KP_PRESET[5] = { 5, 15, 30, 60, 120 };
const char* const KP_NOTE[5] = {
  "几乎修不动，车会一直偏出去",
  "修得慢，弯道跟不上",
  "中间档，通常最接近工作点",
  "修得急，直道可能开始摆",
  "内轮频繁停转，明显来回摆"
};

// ---------- D 项 ----------
// d = Kd × (pos − pos_prev) / dt。Kd 同样用整数存：kdNum = 1000 → Kd = 1.000。
//
// 量纲推一遍（这是今天最容易配错的地方）：
//   dt 固定 0.1s（PERIOD_MS），所以 d = Kd × Δpos × 10
//   Day 35 的摆动：2 秒一周、±800。1/4 周期 500ms 走完 800，
//   摊到 100ms 一帧 Δpos ≈ 160 → Kd=0.010 时 d ≈ 16
//   对照 P：Kp=0.030、pos=800 时 turn = 24
//   → Kd 的工作区间在 0.005 ~ 0.020，超出就是噪声放大
// 默认 Kd = 0（D 关）：上电行为和 Day 36 完全一样，先复现 P 再加 D。
int  kdNum = 0;
const int KD_PRESET[5] = { 0, 5, 10, 20, 40 };
const char* const KD_NOTE[5] = {
  "D 关，退回纯 P——先复现 Day 36 再往下走",
  "轻阻尼，摆幅开始缩",
  "中间档，通常最接近工作点",
  "阻尼强，可能发僵、该拐不拐",
  "噪声放大，轮子原地抖"
};

// |pos| 小于死区算居中，P 项不给量。**死区只管 P 不管 D**——D 管的是
// "别冲过去"，正好在死区里最有用；关掉它，车照样会冲过死区再摆回来。
int  deadband = 150;
int  steerSign = -1;   // −1：pos>0（线在车的左侧）→ 左轮减速 → 左弧线。按 s 取反。
                      // 阵列镜像：W[] 假设 0 号最左，装车时 0/1 号实际在车的右侧（Day 35 台架实测）。

// ---------- 采样 ----------
const unsigned long PERIOD_MS  = 100;
const unsigned long TEACH_MS   = 20;
const int   CAL_SAMPLES = 200;
const int   CAL_GAP_MS  = 5;
// 帧间隔超过 3 个周期（刚上电、刚静音回来、刚量完噪声）就不算变化率，
// 把 posPrev 对齐到当前值。否则一个几百毫秒的大 Δpos 会让 D 炸一下。
const unsigned long DT_GAP_MS = PERIOD_MS * 3;

// ---------- 丢线 ----------
const unsigned long LOST_TIMEOUT_MS = 500;
const unsigned long RECOVER_MS      = 300;
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
long  posPrev = 0;              // 上一帧的 pos，D 项用
unsigned long prevSteerMs = 0;  // 上一帧 applySteer 的时刻，算 dt 用
int   lastTurn = 0;   // P+D 之和，日志里看实际给了多少
int   lastP    = 0;   // P 项单独的量
int   lastD    = 0;   // D 项单独的量
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
// 标定表 + 方向 + Kp + Kd + 串口调出来的 duty/死区，全部存 flash（NVS）。
// 按过 c / b / s / 1~5 / 6~0 / k / j / d / f / o / +/- / [/] 之后自动存；
// 下次上电自动恢复，车可以脱离电脑直接跑。
// 白底本身会漂（3 分钟 ~250 码），换环境、换光照、改高度之后要重按 c / b / t 覆盖旧值。
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
  calPref.putInt("kp",      kpNum);
  calPref.putInt("kps",     kpNumSaved);
  calPref.putInt("kd",      kdNum);
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
    kpNum      = calPref.getInt("kp",      kpNum);
    kpNumSaved = calPref.getInt("kps",     kpNumSaved);
    kdNum      = calPref.getInt("kd",      kdNum);
    if (kpNum == 0) kpNum = kpNumSaved;   // 上次停在"只留 D"，这次照常带 P 起来
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

// ---------------------------------------------------------------- AO → pos
// 只走 AO 这条路（DO 不参与）。loop 和噪声测量共用它，保证两处算出来的 pos
// 是同一套公式。D 项吃的是这个 pos 的帧间差，所以这条路径必须单一来源。
long computePosFromAO() {
  float wsum = 0.0f;
  float sumN = 0.0f;
  for (int i = 0; i < 5; i++) {
    int span = blackRaw[i] - whiteRaw[i];
    if (span < 1) span = 1;
    float ni = (float)(raw[i] - whiteRaw[i]) / (float)span;
    if (ni < 0.0f) ni = 0.0f;
    n[i] = ni;
    sumN += ni;
    wsum += W[i] * ni;
  }
  if (sumN < 1e-3f) sumN = 1e-3f;
  return (long)(wsum / sumN * 500.0f);
}

// ---------------------------------------------------------------- PD 控制

// turn = Kp × pos + Kd × (pos − pos_prev) / dt，左右轮 = 巡航 ∓ turn。
//
// 符号：Day 35 定下 steerSign = −1 是对的（阵列镜像，0 号在车的右侧）。
//   pos > 0（黑心压在 3/4 号一侧、车的左侧）→ turn < 0 → rl < rr → 左轮慢 → 左弧线 → 车往左去够线 ✓
//   pos < 0（黑心压在 0/1 号一侧、车的右侧）→ turn > 0 → rl > rr → 右轮慢 → 右弧线 → 车往右去够线 ✓
//   steerSign 被按反的话，同一个 pos 会给出相反的弧线，先按 s 修回来再调 Kp/Kd。
//
// 为什么 D 项也乘 steerSign：阻尼要的是"反对当前的变化"。
//   车往左够线（pos 从大正往 0 走）→ Δpos < 0 → 要一个正的 turn 把 P 那个
//   大修正抵掉一部分 → kd × 负数 × (−1) = 正数 ✓
//   冲过头（pos 从 0 往大正走）→ Δpos > 0 → kd × 正数 × (−1) = 负数，
//   叠加到 P 上让左轮更慢，把往右的趋势按住 ✓
// 所以两项共用同一个 steerSign，不用给 D 单开一个方向常数。
void applySteer(bool lost) {
  int rl, rr;
  lastP = 0;
  lastD = 0;

  if (lost) {
    // 丢线：不动方向，两轮给一样的 duty 继续走，等超时刹停（Day 34 原样）
    rl = rr = dutyCruise;
    lastTurn = 0;
    posPrev = pos;      // 丢线期间 pos 不更新，对齐 posPrev，线回来时 D 不会炸一下
  } else {
    // 死区只管 P：D 管的是"别冲过去"，在死区里最有用，不关。
    long e = (labs(pos) <= deadband) ? 0 : pos;

    unsigned long nowMs = millis();
    float dt = (nowMs - prevSteerMs) * 0.001f;
    if (prevSteerMs == 0 || dt > DT_GAP_MS * 0.001f) {
      // 第一帧，或者隔太久（刚静音回来、刚量完噪声）：不算变化率
      posPrev = pos;
      dt = PERIOD_MS * 0.001f;
    }
    prevSteerMs = nowMs;

    float kp = kpNum * 0.001f;
    float kd = kdNum * 0.001f;
    long  dpos = pos - posPrev;

    long turnP = (long)(kp * (float)e * (float)steerSign);
    long turnD = (long)(kd * (float)dpos / dt * (float)steerSign);
    long turn  = turnP + turnD;

    // 限幅：内轮最低 0，不反转。轮子朝前是这台车的物理约束（差速转向）。
    if (turn >  dutyCruise) turn =  dutyCruise;
    if (turn < -dutyCruise) turn = -dutyCruise;

    rl = dutyCruise + (int)turn;
    rr = dutyCruise - (int)turn;
    lastTurn = (int)turn;
    lastP    = (int)turnP;
    lastD    = (int)turnD;
    posPrev  = pos;
  }
  rr += TRIM_RIGHT;

  if (rl == 0 && rr == 0) { kicked = false; setWheels(0, 0); return; }
  kickIfNeeded(rl, rr);
  setWheels(rl, rr);
}

// ---------------------------------------------------------------- 命令

// Kp 的当前档位和它意味着什么。切档位时打出来，日志里能直接看出"这一档是什么现象"。
void printKp(int preset) {
  float kp = kpNum * 0.001f;
  Serial.printf("[kp] Kp = %.3f", kp);
  if (preset >= 0) Serial.printf("（第 %d 档）", preset + 1);
  Serial.printf("：pos=±800 → P=±%ld\n",
                (long)(kp * 800.0f * (steerSign > 0 ? 1 : -1)));
  if (preset >= 0) Serial.printf("[kp] 预期：%s\n", KP_NOTE[preset]);
  Serial.println("[kp] 试下一档：按 1~5；微调：k 加大 / j 减小");
}

void printKd(int preset) {
  float kd = kdNum * 0.001f;
  Serial.printf("[kd] Kd = %.3f", kd);
  if (preset >= 0) Serial.printf("（第 %d 档）", preset + 1);
  // dt = 0.1s，所以 d = Kd × Δpos × 10。拿 Day 35 摆动的 Δpos≈160 当参照。
  Serial.printf("：Δpos=±160/帧 → D=±%ld\n",
                (long)(kd * 160.0f / 0.1f * (steerSign > 0 ? 1 : -1)));
  if (kdNum == 0) Serial.println("[kd] D 关着 —— 行为和 Day 36 的纯 P 完全一样。");
  else if (preset >= 0) Serial.printf("[kd] 预期：%s\n", KD_NOTE[preset]);
  Serial.println("[kd] 试下一档：按 6 7 8 9 0；微调：d 加大 / f 减小");
}

// n 命令：量 AO 噪声，直接算出"这一档 Kd 会把噪声放大成多大的 D 输出"。
// 这是指南第 4 条要的证据——不靠嘴说"Kd 太大噪声大"，用数比。
void noiseScan() {
  if (!calibrated) {
    Serial.println("[n] 还没标定，先按 c 再按 b。");
    return;
  }

  const int N = 30;            // 30 帧 × 100ms = 3 秒
  long  pmin = 2147483647L, pmax = -2147483647L;
  int   rmin[5], rmax[5];
  long  dmax = 0;
  long  prev = 0;
  bool  first = true;

  for (int i = 0; i < 5; i++) { rmin[i] = 4095; rmax[i] = 0; }

  Serial.println();
  Serial.println("[n] 以控制周期 100ms 采 30 帧（约 3 秒）。车和线都别动。");
  Serial.println("[n] 这 3 秒电机保持当前 duty，别在这时候判断现象。");
  Serial.println("[n] 下面 30 行是纯 pos，直接喂 Python：");

  for (int k = 0; k < N; k++) {
    for (int i = 0; i < 5; i++) {
      raw[i] = analogRead(AO_PIN[i]);
      if (raw[i] < rmin[i]) rmin[i] = raw[i];
      if (raw[i] > rmax[i]) rmax[i] = raw[i];
    }
    long p = computePosFromAO();
    Serial.println(p);
    if (p < pmin) pmin = p;
    if (p > pmax) pmax = p;
    if (!first) {
      long d = labs(p - prev);
      if (d > dmax) dmax = d;
    }
    first = false;
    prev = p;
    delay(PERIOD_MS);
  }

  // 恢复被噪声测量动过的全局量，别让一次测量污染后面的控制
  posPrev = pos;
  prevSteerMs = 0;

  float kd = kdNum * 0.001f;
  float kp = kpNum * 0.001f;
  long  dOut = (long)(kd * (float)dmax / (PERIOD_MS * 0.001f) * (steerSign > 0 ? 1 : -1));
  long  pOut = (long)(kp * (float)pos * (steerSign > 0 ? 1 : -1));

  Serial.println();
  Serial.println("  路   AO 最小   AO 最大   AO 摆幅");
  Serial.println("---------------------------------------------");
  for (int i = 0; i < 5; i++) {
    Serial.printf("  %d   %7d   %7d   %7d\n", i, rmin[i], rmax[i], rmax[i] - rmin[i]);
  }
  Serial.println("---------------------------------------------");
  Serial.printf("  pos 摆幅      ：%ld（最小 %+ld，最大 %+ld）\n", pmax - pmin, pmin, pmax);
  Serial.printf("  帧间最大 |Δpos|：%ld\n", dmax);
  Serial.println();
  Serial.printf("  当前 Kp = %.3f，pos = %+ld  →  P 项输出 %+ld\n", kp, pos, pOut);
  Serial.printf("  当前 Kd = %.3f，dt = %.1fs，最大 |Δpos| = %ld\n", kd, PERIOD_MS * 0.001f, dmax);
  Serial.printf("                    →  D 项输出 %+ld\n", dOut);
  Serial.println();
  if (kdNum == 0) {
    Serial.println("  现在 Kd = 0，噪声还喂不出 D。按 7 或 8 加一档再按一次 n 对比。");
  } else if (pOut == 0) {
    Serial.println("  P 项输出 0（pos 在死区内或 Kp=0），没法比。先把 Kp 调到第 3 档。");
  } else {
    float ratio = (float)labs(dOut) / (float)labs(pOut);
    Serial.printf("  噪声喂出来的 D 是 P 的 %.2f 倍\n", ratio);
    if (ratio < 0.2f)      Serial.println("  → 这一档 Kd 安全，噪声还没被放大到能推动轮子。");
    else if (ratio < 0.5f) Serial.println("  → 临界。轮子会开始细抖，直道上能看到。");
    else                   Serial.println("  → 噪声占主导，轮子会原地抖。Kd 太大，往小调（按 f）。");
  }
  Serial.println();
}

void handleCmd(char c) {
  if (c == 'c') { calibrate(false); return; }
  if (c == 'b') { calibrate(true);  return; }
  if (c == 'w') { printTables(); return; }
  if (c == 'p') {
    plotMode = !plotMode;
    Serial.printf("[plot] %s\n", plotMode ? "开：每行 pos / 压线路数×250 / turn / D，喂 Python 画图" : "关");
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
  if (c >= '1' && c <= '5') {
    kpNum = KP_PRESET[c - '1'];
    savePrefs();
    printKp(c - '1');
    return;
  }
  if (c >= '6' && c <= '9') {
    kdNum = KD_PRESET[c - '5'];
    savePrefs();
    printKd(c - '5');
    return;
  }
  if (c == '0') {
    kdNum = KD_PRESET[0];
    savePrefs();
    printKd(0);
    return;
  }
  if (c == 'k') {
    kpNum = min(300, kpNum + 1);
    savePrefs();
    printKp(-1);
    return;
  }
  if (c == 'j') {
    kpNum = max(1, kpNum - 1);
    savePrefs();
    printKp(-1);
    return;
  }
  if (c == 'd') {
    kdNum = min(200, kdNum + 1);
    savePrefs();
    printKd(-1);
    return;
  }
  if (c == 'f') {
    kdNum = max(0, kdNum - 1);
    savePrefs();
    printKd(-1);
    return;
  }
  if (c == 'o') {
    if (kpNum == 0) {
      kpNum = kpNumSaved;
      Serial.printf("[pd] 恢复 P：Kp = %.3f\n", kpNum * 0.001f);
    } else {
      kpNumSaved = kpNum;
      kpNum = 0;
      Serial.println("[pd] 只留 D：Kp = 0。");
      Serial.println("[pd] D 是阻尼不是纠偏——车偏出去不会自己回来，只会越偏越远。");
      Serial.println("[pd] 按 o 恢复刚才那档 Kp。");
    }
    savePrefs();
    return;
  }
  if (c == 'n') { noiseScan(); return; }
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

  Serial.println("TCRT5000 五路 + TB6612 PD 控制巡线 ready.");
  Serial.println("AO：#0=GPIO6  #1=GPIO7  #2=GPIO8  #3=GPIO2  #4=GPIO3");
  Serial.println("DO：#0=GPIO1  #1=GPIO13 #2=GPIO14 #3=GPIO21 #4=GPIO47");
  Serial.println("TB6612：左轮 10/11/12  右轮 15/16/17  STBY=3V3");
  Serial.printf("duty：巡航 %d / 冲击 %d（Day 21 的 6V 标定值）\n", dutyCruise, DUTY_KICK);
  Serial.printf("控制：turn = Kp × pos + Kd × Δpos / dt\n");
  Serial.printf("      Kp = %.3f，Kd = %.3f，左右轮 = 巡航 ∓ turn\n",
                kpNum * 0.001f, kdNum * 0.001f);
  Serial.println("命令：c=标白底 b=标黑底 w=总表 t=量电位器 p=曲线 q=静音");
  Serial.println("      s=转向取反  1~5=Kp档位  6~0=Kd档位  k/j=Kp微调  d/f=Kd微调");
  Serial.println("      o=只留D（Kp归零）  n=量AO噪声算D输出  +/-=巡航duty  [/]=死区");
  Serial.println("先按 c 再按 b，然后按 t 确认五路 OK，再放到线上看方向。");
  Serial.println("方向反了就按 s —— 不用重新烧录。");
  Serial.println("今天的关键实验：按 6（Kd=0）拿黑道具静止压住 → 按 n → 按 8 → 再按 n。");
  if (loadPrefs()) {
    Serial.println("[nvs] 已恢复上次的标定 + 方向 + Kp + Kd，直接上线跑。");
    Serial.println("[nvs] 命名空间 day35，读的就是 Day 35 那一份表，不用重标。");
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
    pos = computePosFromAO();
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
    Serial.printf("%ld %d %d %d\n", pos, onCount * 250, lastTurn, lastD);
  } else if (!frameLog || now < tableQuietUntil) {
    // 静音中，或表刚打完：别被每帧行冲走
  } else {
    const char* act = "直行";
    if (state == S_STOP)    act = "刹停";
    else if (state == S_LOST) act = "丢线保持";
    else if (curL < curR - 10) act = "左弧线";
    else if (curR < curL - 10) act = "右弧线";
    const char* lostTag = lost ? (lostAllWhite ? " 全白" : " 全黑") : "";
    Serial.printf("[%d %d %d %d %d] n=%d%s | raw %4d %4d %4d %4d %4d | pos %+5ld P%+4d D%+4d turn %+4d L%3d R%3d %s",
                  onLine[0], onLine[1], onLine[2], onLine[3], onLine[4], onCount,
                  lostTag,
                  raw[0], raw[1], raw[2], raw[3], raw[4], pos,
                  lastP, lastD, lastTurn, curL, curR, act);
    // 转向量顶到限幅 = 内轮已经停转。直道上频繁出现这一行，说明 Kp 太大或巡航 duty 太小。
    if (state == S_FOLLOW && (lastTurn >= dutyCruise || lastTurn <= -dutyCruise)) {
      Serial.print("  ← turn 限幅：内轮停转");
    }
    // D 项自己就超出行程：这不是"修得猛"，是噪声被 Kd 放大了。
    if (state == S_FOLLOW && (lastD >= dutyCruise || lastD <= -dutyCruise)) {
      Serial.print("  ← D 项超限幅：噪声被放大，Kd 太大");
    }
    if (state == S_LOST) {
      Serial.printf(" 丢线 %lums（保持 %+ld %s）", now - lostSince, lastPos,
                    lastPos < 0 ? "偏左" : (lastPos > 0 ? "偏右" : "正中"));
    } else if (state == S_STOP) {
      Serial.print(" 已刹停");
    }
    Serial.println();
  }
}
