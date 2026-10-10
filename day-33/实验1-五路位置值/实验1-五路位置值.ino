// Day 33：五路 AO → 一个"线在哪"的数
//
// 接线（五路全部走 ADC1，一路 ADC2 都没用上）：
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
//   DO   ×5  ────── 今天全不接（Day 34 才用）
//
//   ⚠️ 0 号 = 最左（站在桌子前方、朝模块看过去）。顺序反了，整周 pos 的符号都反着来。
//
// 脚位怎么定的：
//   ESP32-S3 上能读模拟量的只有 GPIO1~10（ADC1）和 GPIO11~20（ADC2）两段，
//   GPIO21 以上一颗 ADC 通道都没有 —— 35~42 是 JTAG / 彩灯 / PSRAM 的地盘，
//   GPIO46 也不是 ADC 脚，analogRead() 读出来全是废数。
//   按小车现有接线（见 day-24/智能小车接线图.png），已占的是：
//     GPIO4/5 = 超声波，GPIO10/11/12/15/16/17 = TB6612，GPIO48 = 板载彩灯。
//   剩下的 ADC 脚里取 6/7/8/2/3：6/7/8 是从没用过的空脚，2/3 是拆掉 Day 31
//   的按键 + LED 腾出来的。五路全在 ADC1 上。
//
// 一条要记住的：
//   GPIO8/9 是留给 MPU-6050 的 I2C 脚（第 2 周姿态融合用）。今天 IMU 没插，
//   所以 GPIO8 是空的；等第 2 周上 IMU 时，这一路要拔下来让位。
//   注意必须是"把 MPU 模块从排针上拔掉"——那两块 4.7kΩ 上拉在模块板上，
//   光是"不用它"没用，上拉会一直把 AO 读数钉在高位。
//
//   （ADC2 和 Wi-Fi 的 RF 共用模拟前端，Wi-Fi 一忙就读不出来。五路全挑 ADC1
//     就是为了躲开它：哪天把遥控（Day 27/28）和循线写进同一个固件也不用挪脚。）
//
// 位置值怎么算（Day 32 的结论直接决定了这一步）：
//   ① 逐个标定白底。Day 32 已证实黑永远顶在 4095，所以黑侧五路一模一样、没有可标的东西；
//      五路的差别全在白底（体制不同 + 高度不同）。标定只标白。
//   ② 归一化 n_i = (raw_i − white_i) / (4095 − white_i)，取值 0~1。这样五路说话一样响。
//      直接拿 raw 减白底会偏向"黑白差更大"的那一路，质心被它拽过去。
//   ③ 加权质心 pos = Σ(w_i × n_i) / Σ(n_i)，w = −2/−1/0/+1/+2，再 ×500 归一到 ±1000。
//   Σ(n) 小于 SUM_MIN 就当没看见线，pos 保持上一帧的值。
//
// 串口命令（115200）：
//   c = 标定白底（放白纸上、3cm 高度稳住再按，约 1 秒）
//   w = 打印已存的白底表
//   p = 切换"曲线模式"（只打 pos 和 sum，喂串口绘图器）
// 彩灯：绿=等标定 / 居中，白=没看见线，红=偏左，蓝=偏右。

#include <RgbCycle.h>

// ---------- 引脚 ----------
const int AO_PIN[5] = { 6, 7, 8, 2, 3 };
const char* const PIN_NAME[5] = { "6", "7", "8", "2", "3" };

// 每路相对中点的权重。0 号最左，所以 w 从 −2 排到 +2。
const int W[5] = { -2, -1, 0, 1, 2 };

// ---------- 采样 ----------
const unsigned long PERIOD_MS = 100;     // 10Hz，跟 Day 32 一致
const int   CAL_SAMPLES  = 200;          // 标定时每路取多少个样本
const int   CAL_GAP_MS   = 5;

// ---------- 归一化 ----------
// Day 32 实测：黑底三次都是 4095，顶在 3V3 轨上。所以黑侧不用标，
// 每路的量程 span_i = BLACK_RAW − white_i 直接由它自己的白底推出来。
const int BLACK_RAW = 4095;

// 看不见线的门限，单位是"等效压线传感器个数"。五路全白时 Σn≈0，
// 一路压线时大约 0.6~1.0。0.5 卡在两者中间。完整的丢线判定是 Day 34 的事。
const float SUM_MIN = 0.5f;

// ---------- 状态 ----------
int   whiteRaw[5] = { 1600, 1600, 1600, 1600, 1600 };  // 标定前用一个中性的假值
bool  calibrated  = false;

int   raw[5];
float n[5];
float sumN = 0.0f;
long  pos  = 0;      // −1000 ~ +1000
long  lastPos = 0;   // 丢线时保持上一帧
bool  lineSeen = false;
bool  plotMode = false;

void showState() {
  if (!calibrated)    { RgbCycle::setColor(  0, 255,   0); return; }  // 绿：等标定
  if (!lineSeen)      { RgbCycle::setColor(255, 255, 255); return; }  // 白：没看见线
  if (pos < -100)     { RgbCycle::setColor(255,   0,   0); return; }  // 红：偏左
  if (pos >  100)     { RgbCycle::setColor(  0,   0, 255); return; }  // 蓝：偏右
  RgbCycle::setColor(0, 255, 0);                                     // 绿：居中
}

void printWhiteTable() {
  Serial.println();
  Serial.println("  路   引脚   白底 raw   量程(4095-白底)");
  Serial.println("-------------------------------------------");
  for (int i = 0; i < 5; i++) {
    Serial.printf("  %d    GPIO%-3s  %5d      %5d\n",
                  i, PIN_NAME[i], whiteRaw[i], BLACK_RAW - whiteRaw[i]);
  }
  Serial.println("-------------------------------------------");
  Serial.println();
}

void calibrate() {
  Serial.println("[cal] 五路正在白底上取均值：保持 3cm 高度、别动、别挡光，约 1 秒");
  long acc[5] = { 0, 0, 0, 0, 0 };
  for (int k = 0; k < CAL_SAMPLES; k++) {
    for (int i = 0; i < 5; i++) acc[i] += analogRead(AO_PIN[i]);
    delay(CAL_GAP_MS);
  }
  Serial.println();
  for (int i = 0; i < 5; i++) {
    whiteRaw[i] = (int)(acc[i] / CAL_SAMPLES);
    Serial.printf("  %d 号 GPIO%-3s  white = %4d   span = %4d\n",
                  i, PIN_NAME[i], whiteRaw[i], BLACK_RAW - whiteRaw[i]);
  }
  calibrated = true;
  Serial.println("[cal] 完成。现在可以拿画了黑线的纸横扫了");
  Serial.println();
  printWhiteTable();
}

void handleCmd(char c) {
  if (c == 'c') { calibrate(); return; }
  if (c == 'w') { printWhiteTable(); return; }
  if (c == 'p') {
    plotMode = !plotMode;
    Serial.printf("[plot] %s\n", plotMode ? "开：只打 pos 和 sum" : "关");
    return;
  }
}

void setup() {
  Serial.begin(115200);

  RgbCycle::begin();
  showState();

  analogReadResolution(12);
  for (int i = 0; i < 5; i++) pinMode(AO_PIN[i], INPUT);

  Serial.println("TCRT5000 五路 ready. 全部 ADC1，VCC=3V3");
  Serial.println("脚位：#0=GPIO6  #1=GPIO7  #2=GPIO8  #3=GPIO2  #4=GPIO3");
  Serial.println("命令：c=标定白底  w=白底表  p=曲线模式");
  delay(200);
}

void loop() {
  while (Serial.available() > 0) {
    char c = (char)Serial.read();
    if (c == '\n' || c == '\r' || c == ' ') continue;
    handleCmd(c);
  }

  static unsigned long last = 0;
  unsigned long now = millis();
  if (now - last < PERIOD_MS) return;
  last = now;

  // 五路依次读。相邻两次读只差几十微秒，横扫的速度远低于这个，
  // 所以 skew 可以不管；真要同时采样得上 DMA，那是以后的事。
  for (int i = 0; i < 5; i++) raw[i] = analogRead(AO_PIN[i]);

  float wsum = 0.0f;
  sumN = 0.0f;
  for (int i = 0; i < 5; i++) {
    int span = BLACK_RAW - whiteRaw[i];
    if (span < 1) span = 1;                       // 白底顶到刻度时别除零
    n[i] = (float)(raw[i] - whiteRaw[i]) / (float)span;
    if (n[i] < 0.0f) n[i] = 0.0f;                 // 比白底还白的不算线
    sumN += n[i];
    wsum += W[i] * n[i];
  }

  lineSeen = (sumN >= SUM_MIN);
  if (lineSeen) {
    pos = (long)(wsum / sumN * 500.0f);
    lastPos = pos;
  } else {
    pos = lastPos;
  }

  showState();

  if (plotMode) {
    // 两列，让串口绘图器同时画 pos 和 Σn×200（0~1000 区间，好放在同一屏）
    Serial.printf("%ld %d\n", pos, (int)(sumN * 200.0f));
  } else {
    Serial.printf("raw %4d %4d %4d %4d %4d | n %.2f %.2f %.2f %.2f %.2f | sum %.2f | pos %+5ld %s\n",
                  raw[0], raw[1], raw[2], raw[3], raw[4],
                  n[0], n[1], n[2], n[3], n[4],
                  sumN, pos, lineSeen ? "" : "(丢线)");
  }
}
