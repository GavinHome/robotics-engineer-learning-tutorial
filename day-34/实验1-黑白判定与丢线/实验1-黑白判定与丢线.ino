// Day 34：黑 / 白判定与丢线处理
//
// 接线（Day 33 的五路 AO 不动，再把五路 DO 接上）：
//
//   TCRT5000        ESP32-S3
//   ─────────       ────────
//   VCC  ×5  ────── 3V3（级联：每块的 VCC 接下一块的 VCC，最后只出一根）
//   GND  ×5  ────── GND（同样级联）
//   0 号 AO ────── GPIO6  (ADC1_CH5)   ← 最左
//   1 号 AO ────── GPIO7  (ADC1_CH6)
//   2 号 AO ────── GPIO8  (ADC1_CH7)
//   3 号 AO ────── GPIO2  (ADC1_CH1)
//   4 号 AO ────── GPIO3  (ADC1_CH2)
//   0 号 DO ────── GPIO1
//   1 号 DO ────── GPIO13
//   2 号 DO ────── GPIO14
//   3 号 DO ────── GPIO21
//   4 号 DO ────── GPIO47
//
//   ⚠️ AO 的 0~4 和 DO 的 0~4 必须是同一批模块、同一个左右顺序。
//      接反一路，那一路的"黑白"就和别的路反着来，五路 DO 的组合全是乱的。
//
// DO 这五个脚是怎么挑的：
//   纯数字输入，不需要 ADC，所以 ADC 那 20 个脚不用特意留着，只躲这些：
//   0/3/45/46 strapping、22~34 在 WROOM-1 上没引出（26~32 内部 Flash、
//   33/34 Octal PSRAM）、35~37 Octal PSRAM、38 彩灯、39~42 JTAG、
//   43/44 USB 串口、48 板载彩灯、4/5 超声波、10~17 TB6612、
//   18 舵机（Day 27 云台）、8/9 MPU-6050 的 I2C、2/3/6/7/8 五路 AO。
//   数剩下正好五个：1、13、14、21、47。
//   内部上拉开着（INPUT_PULLUP）——LM393 输出是集电极开路，模块板上那颗
//   上拉万一没焊，脚就浮空了；加上内部上拉不碍事，两种板子都能读。
//
// 今天要立住的三件事：
//   ① 每个模块的二值阈值（板上那颗 104 电位器）落在哪。用 t 命令量出来，
//      不靠"转到看起来差不多"。Day 33 的逐路白底/黑底表就是它的靶子：
//      靶心 = (white_i + black_i) / 2。
//   ② 丢线条件不是"AO 加起来不够多"，而是五路 DO 的组合：
//        全白（0 路压线）—— 线跑到阵列外面了
//        全黑（5 路全压线）—— 阵列盖在一大片黑上
//      两种情况的质心都是假的：全黑时 Σ(w·n) 恰好等于 0，pos 读出 0，
//      车以为自己正压在线上。这是 Day 33 的 SUM_MIN 漏掉的情况。
//   ③ 丢线之后干什么：保持上一帧方向 + 记丢线时长，超时就停并上报，
//      不继续乱转。（Day 24 避障状态机的老办法，今天原样搬过来。）
//
// 串口命令（115200）：
//   c = 标定白底（① 区，3cm 稳住约 1 秒）—— 同时记下五路 DO 在白底上的电平
//   b = 标定黑底（② 区，稳住约 1 秒）—— 同时记下五路 DO 在黑底上的电平
//   w = 打印白底 / 黑底 / 建议阈值 / DO 极性总表
//   t = 量电位器：横扫纸，记录每路 DO 翻转那一刻的 AO，那就是阈值。按任意键收表
//   p = 切换曲线模式
//   q = 静音键：关掉每帧 raw 日志和 [teach] 翻点行。关掉后串口只剩状态变化和你
//       按出来的表，怎么按都不会滚屏，看表不用往上翻。再按一次恢复
// 命令大小写都认。t 横扫时按任意键 = 收表（q 例外，q 什么时候都能按）。
// 每打完一张表会静默 5 秒不刷每帧日志：10Hz 一秒 10 行，表打出来一秒就被冲出屏幕。
// 想彻底不滚屏就按 q。
// 彩灯：绿=线居中、红=偏左、蓝=偏右、白=丢线（保持方向）、红闪=丢线超时已停车、
//       黄=正在量电位器

#include <RgbCycle.h>

// ---------- 引脚 ----------
const int AO_PIN[5]     = { 6, 7, 8, 2, 3 };
const char* const AO_NAME[5] = { "6", "7", "8", "2", "3" };
const int DO_PIN[5]     = { 1, 13, 14, 21, 47 };
const char* const DO_NAME[5] = { "1", "13", "14", "21", "47" };

// 每路相对中点的权重。0 号最左，所以 w 从 −2 排到 +2。
const int W[5] = { -2, -1, 0, 1, 2 };

// ---------- 采样 ----------
const unsigned long PERIOD_MS  = 100;    // 10Hz，和 Day 33 一致，两条曲线好对比
const unsigned long TEACH_MS   = 20;     // t 命令下扫快点，别错过翻转那一刻
const int   CAL_SAMPLES = 200;
const int   CAL_GAP_MS  = 5;

// ---------- 丢线 ----------
// 连续丢线这么久就停车。500ms 的来历：车大约 300mm/s，100ms 挪 30mm；
// "线抖了一下"只会丢一两帧（100~200ms），"真冲出赛道"是永远。
// Day 35 装车量到真实速度后要回来重定这个数。
const unsigned long LOST_TIMEOUT_MS = 500;
// 从停车恢复要先连续看见线这么久，免得在边界上抖进抖出。
const unsigned long RECOVER_MS      = 300;
// 阈值离靶心（白黑中点）最多允许偏这么多：1/6 量程，也就是 n 允许偏离 0.5  ±0.167。
// 300 码那个老门槛在 2200 的量程里只占 14%，会放行 n≈0.16 这种明显偏低的值。
const int OFF_MID_DIV = 6;
// 表刚打完的这几秒别刷每帧日志。10Hz 一秒 10 行，表打出来一秒就被冲出屏幕，
// 每次看表都得在串口监视器里往上翻。安静几秒，按 w 可以重新打一张。
const unsigned long TABLE_QUIET_MS = 5000;

// ---------- 标定表 ----------
int  whiteRaw[5] = { 1600, 1600, 1600, 1600, 1600 };  // 标定前用一个中性的假值
int  blackRaw[5] = { 4095, 4095, 4095, 4095, 4095 };  // 没标过黑时按 Day 32 的天花板

// 标定时顺手记下的 DO 电平，用来定极性 + 检查电位器有没有把黑白分开
int  doAtWhite[5] = { 0, 0, 0, 0, 0 };
int  doAtBlack[5] = { 1, 1, 1, 1, 1 };
int  lineLevel[5];        // 这一路 DO 读到几算"压线"
bool doSplit[5];          // 黑白两级 DO 分没分开（分不开 = 电位器没调好）
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
bool  lostAllWhite = false;   // 丢线的原因是全白还是全黑
RunState state = S_WAIT;
unsigned long lostSince    = 0;
unsigned long recoverSince = 0;
unsigned long stopSince    = 0;
unsigned long lastReport   = 0;
bool  plotMode  = false;
bool  teachMode = false;
// 每帧那行 raw 日志的开关。它 10Hz 刷屏，表打出来一秒就被冲出屏幕，看表得上翻。
// 按 q 可以彻底关掉：关了之后只有状态变化和表会打，串口一直是安静的。
bool  frameLog  = true;

// ---------- t 命令：量电位器 ----------
// 两个方向都存一整串翻点，收表时取中位数。扫快了会有一两个翻点跑偏几百码
// （20ms 一帧，AO 在两次采样之间就穿过了阈值），只取最后一对会正好踩上离群点。
//
// 交互是手动的：按 t 开始，横扫，按任意键收表。不自动收表——DO 在阈值附近会抖，
// 20ms 一帧一秒内就能抖出四个翻点，自动收表会在人还没扫完的时候就自己收了。
const int TEACH_MAX = 16;

int  risePts[5][TEACH_MAX];   // DO 白→黑 那一刻的 AO
int  fallPts[5][TEACH_MAX];   // DO 黑→白 那一刻的 AO
int  riseCnt[5];
int  fallCnt[5];
bool prevOn[5];
unsigned long tableQuietUntil  = 0;   // 表打完后的静默截止时刻

// 插入排序后取中位数。会改动传入的数组，调用处传副本。
int median(int* v, int cnt) {
  for (int a = 1; a < cnt; a++) {
    int k = v[a], b = a - 1;
    while (b >= 0 && v[b] > k) { v[b + 1] = v[b]; b--; }
    v[b + 1] = k;
  }
  return v[cnt / 2];
}

// ---------------------------------------------------------------- 彩灯

void showState() {
  if (!calibrated) { RgbCycle::setColor(0, 255, 0); return; }         // 绿：等标定
  if (state == S_STOP) {
    bool lit = (millis() / 250) % 2 == 0;                             // 红闪：已停车
    RgbCycle::setColor(lit ? 255 : 0, 0, 0);
    return;
  }
  if (state == S_LOST) { RgbCycle::setColor(255, 255, 255); return; } // 白：丢线，保持方向
  if (pos < -100)     { RgbCycle::setColor(255, 0, 0); return; }      // 红：偏左
  if (pos >  100)     { RgbCycle::setColor(0, 0, 255); return; }      // 蓝：偏右
  RgbCycle::setColor(0, 255, 0);                                      // 绿：居中
}

// ---------------------------------------------------------------- 表

void printTables() {
  tableQuietUntil = millis() + TABLE_QUIET_MS;   // 表打完别被每帧日志冲走
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

// ---------------------------------------------------------------- 标定

// black = false 标白底，true 标黑底。AO 取 200 个样本的均值压抖动；
// DO 同时数高电平个数，过半算高——极性就从这两次记录里自己浮出来。
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

  // 极性：黑底上读到什么电平，什么电平就代表"压线"
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
}

// ---------------------------------------------------------------- t：量电位器

void teachStart() {
  teachMode = true;
  for (int i = 0; i < 5; i++) {
    riseCnt[i] = fallCnt[i] = 0;
    prevOn[i] = (digitalRead(DO_PIN[i]) == lineLevel[i]);
  }
  RgbCycle::setColor(255, 255, 0);          // 黄：正在量电位器，看灯就知道 t 按下去了
  Serial.println();
  Serial.println("[teach] 把纸从 ① 白底区慢慢横扫过 ② 黑块，再扫回来，做两个来回。");
  Serial.println("[teach] 每路 DO 翻转时我会记下那一刻的 AO —— 那就是这颗电位器的阈值。");
  Serial.println("[teach] 扫慢点：一颗管子 1 秒左右。按任意键结束并打印结果。");
  Serial.println();
}

void teachStop() {
  teachMode = false;
  tableQuietUntil = millis() + TABLE_QUIET_MS;   // 表打完别被每帧日志冲走
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

    // 两个方向的翻点合到一起取中位数。样本多，个别跑偏的翻点拉不动中位数。
    int buf[2 * TEACH_MAX];
    for (int k = 0; k < nr; k++) buf[k] = risePts[i][k];
    for (int k = 0; k < nf; k++) buf[nr + k] = fallPts[i][k];
    int thr  = median(buf, total);          // median 会排序，之后 buf[0]/buf[total-1] 就是最小最大
    int spread = buf[total - 1] - buf[0];
    snprintf(sSpread, sizeof(sSpread), "%d", spread);

    // 判定只看"离靶心多远"：已分开只保证阈值没掉出 [白, 黑] 这个宽 2000+ 码的区间，
    // 不保证它在中间。而 n≈0.1 和 n≈0.5 在 w 表里都显示"已分开"。
    char verdict[48];
    int mid    = (whiteRaw[i] + blackRaw[i]) / 2;
    int span   = blackRaw[i] - whiteRaw[i];
    int offMid = thr - mid;                 // 负 = 偏白侧（阈值偏低），正 = 偏黑侧
    int limit  = span / OFF_MID_DIV;
    int nPct   = (thr - whiteRaw[i]) * 100 / span;   // 阈值折合成量程百分比
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
}

// ---------------------------------------------------------------- setup

void setup() {
  Serial.begin(115200);

  RgbCycle::begin();
  showState();

  analogReadResolution(12);
  for (int i = 0; i < 5; i++) {
    pinMode(AO_PIN[i], INPUT);
    pinMode(DO_PIN[i], INPUT_PULLUP);
    lineLevel[i] = 1;      // 标定前先假定"DO 高 = 压线"，标完自动改
    doSplit[i]   = false;
    prevOn[i]    = false;
  }

  Serial.println("TCRT5000 五路 DO ready. AO 全在 ADC1，VCC=3V3");
  Serial.println("AO：#0=GPIO6  #1=GPIO7  #2=GPIO8  #3=GPIO2  #4=GPIO3");
  Serial.println("DO：#0=GPIO1  #1=GPIO13 #2=GPIO14 #3=GPIO21 #4=GPIO47");
  Serial.println("命令：c=标白底  b=标黑底  w=总表  t=量电位器  p=曲线模式  q=静音");
  Serial.println("先按 c 再按 b，然后按 t 量五路 DO 的实际阈值。");
  Serial.println("嫌刷屏就按 q：关掉每帧 raw 日志和 [teach] 行，只剩状态变化和表。");
  delay(200);
}

// ---------------------------------------------------------------- loop

void loop() {
  while (Serial.available() > 0) {
    char c = (char)Serial.read();
    if (c == '\n' || c == '\r' || c == ' ') continue;
    if (c >= 'A' && c <= 'Z') c += 32;   // 大小写都认：按了大写 T 不能静默没反应
    // t 横扫时任意键 = 收表；q 例外，它是全局静音键，横扫中也能按
    if (teachMode && c != 't' && c != 'q') { teachStop(); continue; }
    handleCmd(c);
  }

  static unsigned long last = 0;
  unsigned long now = millis();
  unsigned long period = teachMode ? TEACH_MS : PERIOD_MS;
  if (now - last < period) return;
  last = now;

  if (teachMode) { teachPoll(); return; }

  // 极性每帧重算一遍：五条比较，忽略不计，但按 b 之后立刻生效
  for (int i = 0; i < 5; i++) {
    doSplit[i]   = (doAtWhite[i] != doAtBlack[i]);
    lineLevel[i] = doAtBlack[i];
  }

  // 五路依次读。相邻两次读只差几十微秒，横扫的速度远低于这个，skew 可以不管。
  for (int i = 0; i < 5; i++) raw[i] = analogRead(AO_PIN[i]);

  // 二值化只认 DO。AO 仍然每帧都读，但只用来看阈值、看渐变。
  onCount = 0;
  for (int i = 0; i < 5; i++) {
    onLine[i] = (digitalRead(DO_PIN[i]) == lineLevel[i]);
    if (onLine[i]) onCount++;
  }

  // 丢线条件：全白（线跑出阵列）或全黑（阵列盖在一大片黑上）
  lostAllWhite = (onCount == 0);
  bool lost = (onCount == 0) || (onCount == 5);

  if (!lost) {
    // 跟线：位置值还是 Day 33 的 AO 加权质心。DO 只回答"线还在不在"，
    // 不回答"线在哪"——那是两种形态的分工。
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
    if (sumN < 1e-3f) sumN = 1e-3f;      // 电位器没调好时 DO 说压线而 AO 还在白底
    pos = (long)(wsum / sumN * 500.0f);
    lastPos = pos;

    if (state != S_FOLLOW) {
      if (state == S_STOP && (now - recoverSince) < RECOVER_MS) {
        // 刚从停车恢复，要连续看见线 RECOVER_MS 才认，免得在边界上抖
      } else {
        Serial.printf("[state] %s → 跟线%s\n", STATE_NAME[state],
                      state == S_WAIT ? "" : "（线回来了）");
        state = S_FOLLOW;
      }
    }
    lostSince = 0;
  } else {
    // 丢线：保持上一帧方向，只记时长，不动 pos
    if (state != S_LOST && state != S_STOP) {
      state = S_LOST;
      lostSince = now;
      Serial.printf("[state] 跟线 → 丢线（%s）保持 pos=%+ld\n",
                    lostAllWhite ? "五路全白" : "五路全黑", lastPos);
    }
    if (state == S_LOST && (now - lostSince) >= LOST_TIMEOUT_MS) {
      state = S_STOP;
      recoverSince = now;
      stopSince = now;
      Serial.printf("[state] 丢线 → 停车：连续 %lums 没看见线，停并上报。\n", LOST_TIMEOUT_MS);
      Serial.printf("[report] 最后位置 pos=%+ld，%s。等线回来或复位。\n",
                    lastPos, lastPos < 0 ? "线在左边离开的" : (lastPos > 0 ? "线在右边离开的" : "线在正中间离开的"));
    }
    if (state == S_STOP) recoverSince = now;
  }

  showState();

  // 停车之后每秒再报一次，免得串口静下来看不出车还停着
  if (state == S_STOP && (now - lastReport) >= 1000) {
    lastReport = now;
    Serial.printf("[report] 仍停车，已 %lus。最后 pos=%+ld，%s。\n",
                  (now - stopSince) / 1000, lastPos,
                  lostAllWhite ? "五路全白" : "五路全黑");
  }

  if (plotMode) {
    Serial.printf("%ld %d\n", pos, onCount * 250);
  } else if (!frameLog || now < tableQuietUntil) {
    // 每帧日志关着，或者表刚打完：静几秒让人在串口监视器里看完，别被 10Hz 冲走上翻
  } else {
    Serial.printf("[%d %d %d %d %d] n=%d%s | raw %4d %4d %4d %4d %4d | pos %+5ld %s",
                  onLine[0], onLine[1], onLine[2], onLine[3], onLine[4], onCount,
                  lost ? (lostAllWhite ? " 全白" : " 全黑") : "",
                  raw[0], raw[1], raw[2], raw[3], raw[4], pos,
                  STATE_NAME[state]);
    if (state == S_LOST) {
      Serial.printf(" 丢线 %lums（保持 %+ld %s）", now - lostSince, lastPos,
                    lastPos < 0 ? "偏左" : (lastPos > 0 ? "偏右" : "正中"));
    } else if (state == S_STOP) {
      Serial.printf(" 已停车");
    }
    Serial.println();
  }
}
