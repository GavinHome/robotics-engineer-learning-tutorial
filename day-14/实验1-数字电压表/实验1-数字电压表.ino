// Day 14：第二周项目 — 数字电压表
//
// 综合运用 GPIO / ADC / Serial：把 ESP32-S3 变成一个 0~36V 的直流电压表。
//
// 电路（分压比 11:1）：
//   被测电压 VIN ──[ R_HI 10kΩ ]──┬── GPIO1 (ADC1_CH0)
//                                 │
//                            [ R_LO 1kΩ ]
//                                 │
//                                GND
//
// 【本电路实际使用 10kΩ + 1kΩ】
// 逐日指南原本建议 1MΩ + 100kΩ，这里改小了。原因：分压比同样是 11:1，
// 但等效源阻抗从 91kΩ 降到 909Ω，ADC 采得准、噪声小，不需要额外并去耦电容。
//
// ⚠️ 只能测直流 0~36V。严禁测市电、严禁超过 36V，会烧毁 GPIO 甚至整块板。
// ⚠️ 接线顺序：先接 GND，最后接 VIN；拆线反过来。ESP32 要先上电。

#include <RgbCycle.h>

// ---------- 硬件参数 ----------
const int ADC_PIN = 1;        // GPIO1 = ADC1_CH0（ADC1 不受 WiFi 影响）

const float R_HI = 10.0;      // kΩ，输入侧电阻（VIN → GPIO1）
const float R_LO = 1.0;       // kΩ，下拉电阻（GPIO1 → GND）

// ---------- 校准 ----------
// 单点校准系数。用板上 3V3 引脚实测后调整：
//   CAL_SCALE = 万用表真实值 / 本表读数
//   读数偏大 → 调小；读数偏小 → 调大。
//
// 为什么不用 5V：5Vin 是电源【输入】引脚，不是稳压输出，没有驱动能力。
// 万用表（10MΩ 输入阻抗）能测到 4.4V，但接上 11kΩ 分压负载后电压直接
// collapses 到 0。3V3 是板载 LDO 的输出，有驱动能力，可用作校准基准。
//
// 2026-09-15 实测：万用表 3.33V / 本表原始读数 3.31V → CAL_SCALE = 1.0055
//                 校准后读数 3.33V，误差 ~0%
//
// 两个教训，改系数前务必看：
// 1) 必须用【当次】的读数算。原始读数会在不同次测量之间漂移
//    （本次两次测量：3.24V → 3.31V，+2.2%）。拿旧读数算出的系数会过校正：
//    曾用 3.16V 算出 1.0538，结果读数 3.49V，误差反而从 -2.7% 放大到 +4.8%。
// 2) 单点乘性校准只能修正比例误差，修不了零点偏移。
//    证据：adc 328 按理是 328/4095×3.3 = 0.264V，但 v_pin 报 0.302V，差约 38mV。
//    要连偏移一起修，得做两点标定，改成 vin = a × vPin + b（见 校准记录.md）。
const float CAL_SCALE = 1.0055;

// ---------- 采样 ----------
const int SAMPLES = 16;       // 平均次数，越大越稳但越慢

// ---------- 报警阈值 ----------
const float V_LOW_UNRELIABLE = 1.0;                          // 低于此值 ADC 低端线性差
const float V_HIGH_WARN      = 30.0;                         // 接近量程上限，提醒
const float V_RANGE_MAX      = 3.3 * (R_HI + R_LO) / R_LO;   // 量程上限 ≈ 36.3V

// ---------- 状态指示（板载 WS2812B，GPIO48）----------
// Day 14 起彩灯不再是"心跳"，改成状态指示：绿=正常 蓝=过低 红=超限
// 注意：不能叫 OK / LOW / HIGH —— Arduino 核心已把它们定义成宏，会冲突。
enum VStatus { V_OK, V_TOO_LOW, V_TOO_HIGH };
VStatus lastStatus = V_OK;

void showStatus(VStatus s) {
  switch (s) {
    case V_OK:       RgbCycle::setColor(0, 255, 0); break;   // 绿
    case V_TOO_LOW:  RgbCycle::setColor(0, 0, 255); break;   // 蓝
    case V_TOO_HIGH: RgbCycle::setColor(255, 0, 0); break;   // 红
  }
}

void setup() {
  Serial.begin(115200);

  RgbCycle::begin();      // 只初始化灯珠，不调用 update()（那会循环变色，覆盖状态色）
  showStatus(V_OK);

  analogReadResolution(12);            // 0~4095
  analogSetAttenuation(ADC_11db);      // 量程约 0~3.3V

  Serial.println("Digital Voltmeter ready. Range: 0 - " + String(V_RANGE_MAX, 1) + " V");
  delay(200);
}

void loop() {
  // 1) 多次采样取平均，抑制 ADC 噪声。
  //    analogReadMilliVolts() 会套用 eFuse 里出厂校准的参考电压，比自己乘 3.3 准得多。
  //    若你的 Arduino-ESP32 版本没有这个函数，改用：
  //      sumMv += analogRead(ADC_PIN) * 3.3 / 4095.0 * 1000.0;
  long sumRaw = 0;
  float sumMv = 0.0;
  for (int i = 0; i < SAMPLES; i++) {
    sumRaw += analogRead(ADC_PIN);
    sumMv  += analogReadMilliVolts(ADC_PIN);
    delayMicroseconds(200);
  }
  float avg  = sumRaw / (float)SAMPLES;
  float vPin = sumMv / SAMPLES / 1000.0;   // 引脚电压，单位 V

  // 2) 反推被测电压：VIN = V_PIN × (R_HI + R_LO) / R_LO
  float vin = vPin * (R_HI + R_LO) / R_LO * CAL_SCALE;

  // 3) 判断状态
  VStatus s;
  const char* label;
  if (vin >= V_HIGH_WARN)          { s = V_TOO_HIGH; label = "OVER"; }
  else if (vin < V_LOW_UNRELIABLE) { s = V_TOO_LOW;  label = "LOW";  }
  else                             { s = V_OK;       label = "OK";   }

  if (s != lastStatus) {        // 只在状态变化时刷新灯，避免每帧重复写灯珠
    showStatus(s);
    lastStatus = s;
  }

  // 4) 每秒输出一行 JSON（延续 Day 13 的格式，方便 Python 直接解析）
  Serial.printf("{\"adc\": %.0f, \"v_pin\": %.3f, \"vin\": %.2f, \"status\": \"%s\"}\n",
                avg, vPin, vin, label);

  delay(1000);
}
