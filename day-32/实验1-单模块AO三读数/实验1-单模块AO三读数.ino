// Day 32：TCRT5000 单模块 AO 三读数
//
// 接线：
//
//   TCRT5000 模块      ESP32-S3
//   ─────────────      ────────
//   VCC      ────────  3V3
//   GND      ────────  GND（必须共地）
//   AO       ────────  GPIO6（ADC1_CH5）
//   DO       ────────  今天不接
//
// ⚠️ 为什么 VCC 接 3V3 而不是 5V：这块的 AO 是接收管电压直出，幅度跟着 VCC 走。
//    5V 供电时白底的 AO 高出 ESP32-S3 的 3.3V ADC 上限，顶到满量程就分不出黑白了。
//
// ⚠️ 为什么今天只接 AO：DO 是 LM393 比较之后的二值输出，只有"过阈值 / 没过"两种状态；
//    AO 是连续的，黑 / 白 / 悬空三段读数只有它能分开。DO 留到 Day 34 定阈值时才用。
//
// 串口命令（115200）：
//   b / w / a  = 开始计"黑底 / 白底 / 悬空 5cm"这一段
//   p          = 打印三段汇总表
//   r          = 清空重记
// 顺序：先按 b 再放到黑底上、按 w 再移到白底、按 a 再抬离桌面 5cm，最后按 p。

#include <RgbCycle.h>

// ---------- 引脚 ----------
// GPIO6 = ADC1_CH5。避开 GPIO1（Day 12 电位器）、GPIO4/5（Day 17 超声波）、
// GPIO8/9（Day 18 I2C）、GPIO10~17（Day 21 TB6612 电机）。
const int AO_PIN = 6;

// 采样周期。10Hz 够看趋势，也不会让串口刷屏看不清。
const unsigned long PERIOD_MS = 100;

// ADC 换算：ESP32-S3 默认 12 位（0~4095），满量程按 3.3V 算 → 0.806 mV/LSB。
// 这里不用 analogReadMilliVolts()：它内部另做一次带校准的转换，raw 和 mv 会是两次采样，
// 两列数字对不上。自己按同一个 raw 换算，两列永远一致。
const float MV_PER_LSB = 3300.0 / 4095.0;

// ---------- 表面 ----------
// 名字避开 Arduino 核心宏（OK / LOW / HIGH）。
enum Surf { SUR_BLACK, SUR_WHITE, SUR_AIR, SUR_NONE };

const char* const SUR_NAME[4] = { "BLACK", "WHITE", "AIR", "IDLE" };

struct Stat {
  long   n      = 0;
  int    rawMin = 0;
  int    rawMax = 0;
  double rawSum = 0.0;
  float  mvMin  = 0.0f;
  float  mvMax  = 0.0f;
  double mvSum  = 0.0;
};

Stat st[4];

Surf cur = SUR_NONE;

void showSurf(Surf s) {
  switch (s) {
    case SUR_BLACK: RgbCycle::setColor(255,   0,   0); break;   // 红：计黑底
    case SUR_WHITE: RgbCycle::setColor(255, 255, 255); break;   // 白：计白底
    case SUR_AIR:   RgbCycle::setColor(  0,   0, 255); break;   // 蓝：计悬空
    default:        RgbCycle::setColor(  0, 255,   0); break;   // 绿：待命
  }
}

void resetAll() {
  for (int i = 0; i < 4; i++) st[i] = Stat();
}

void printSummary() {
  Serial.println();
  Serial.println("surface    n    raw min / mean /  max      mV min / mean /  max");
  Serial.println("---------------------------------------------------------------");
  for (int i = 0; i < 3; i++) {
    if (st[i].n == 0) {
      Serial.printf("%-6s     0    (尚未采集)\n", SUR_NAME[i]);
      continue;
    }
    Serial.printf("%-6s  %4ld    %4d / %6.1f / %4d     %6.1f / %7.1f / %6.1f\n",
                  SUR_NAME[i], st[i].n,
                  st[i].rawMin, st[i].rawSum / st[i].n, st[i].rawMax,
                  st[i].mvMin,  st[i].mvSum  / st[i].n, st[i].mvMax);
  }
  Serial.println("---------------------------------------------------------------");
  Serial.println();
}

void handleCmd(char c) {
  Surf target;
  if      (c == 'b') target = SUR_BLACK;
  else if (c == 'w') target = SUR_WHITE;
  else if (c == 'a') target = SUR_AIR;
  else if (c == 'p') { printSummary(); return; }
  else if (c == 'r') {
    resetAll();
    cur = SUR_NONE;
    showSurf(cur);
    Serial.println("[reset] 三段统计已清空");
    return;
  }
  else return;

  cur = target;
  st[cur] = Stat();          // 切段即清零，重放不用先按 r
  showSurf(cur);
  Serial.printf("[start] 正在采集 %s\n", SUR_NAME[cur]);
}

void setup() {
  Serial.begin(115200);

  RgbCycle::begin();
  showSurf(SUR_NONE);

  analogReadResolution(12);
  pinMode(AO_PIN, INPUT);

  Serial.println("TCRT5000 AO ready. VCC=3V3, AO=GPIO6");
  Serial.println("命令：b=黑底  w=白底  a=悬空5cm  p=汇总  r=清空");
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

  int raw = analogRead(AO_PIN);
  float mv = raw * MV_PER_LSB;

  if (cur != SUR_NONE) {
    Stat& s = st[cur];
    if (s.n == 0) {
      s.rawMin = s.rawMax = raw;
      s.mvMin  = s.mvMax  = mv;
    } else {
      if (raw < s.rawMin) s.rawMin = raw;
      if (raw > s.rawMax) s.rawMax = raw;
      if (mv  < s.mvMin)  s.mvMin  = mv;
      if (mv  > s.mvMax)  s.mvMax  = mv;
    }
    s.n++;
    s.rawSum += raw;
    s.mvSum  += mv;
  }

  Serial.printf("%-5s raw=%4d  mv=%6.1f\n", SUR_NAME[cur], raw, mv);
}
