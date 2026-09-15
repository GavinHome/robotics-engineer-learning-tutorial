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
// 校准源怎么选：
//   ✗ 板上 5Vin 引脚 —— 它是电源【输入】脚，不是稳压输出，没有驱动能力。
//     万用表（10MΩ 输入阻抗）能测到 4.4V，接上 11kΩ 分压负载后直接 collapses 到 0。
//   ✓ 板上 3V3 引脚 —— 板载 LDO 输出，有驱动能力。
//   ✓ 剪掉充电头的 USB 线（红=5V 黑=GND）—— 真正的 5V 电源，有驱动能力，
//     实测可用，是目前最好的第二校准点。注意黑线要和 ESP32 共地。
//
// 2026-09-15 实测（CAL_SCALE = 1.0055 上传后）：
//   板上 3V3：万用表 3.33V，本表读 3.23V，误差 -3.0%（adc 327, v_pin 0.292）
//   USB 线 5V：本表读 4.99V，但【真值没量过】，不能当基准
//   表笔悬空：0.00V（adc 0）
//
//   ⚠️ 4.99 看着很准，但什么都没证明：USB 充电器空载常输出 5.1~5.2V，
//   若那根线实际是 5.14V，则 4.99 对应的误差同样是 -3.0%，与 3V3 点完全一致。
//   只有万用表量过的点才能当基准。
//
// 改系数前务必看这四条：
// 1) 读数本身会漂 ±1.2%（当天三次测 3V3：3.24 → 3.31 → 3.23），
//    所以必须【同时测量】：万用表表笔夹在分压器 VIN 输入端（即你的表笔尖），
//    和串口读数同一时刻读。先量一次、过会儿再看串口 —— 这个错误已经栽过两次
//    （3.16V 算出 1.0538，上传后读数 3.49V，误差反而从 -2.7% 放大到 +4.8%）。
// 2) 别拿未知真值的源当基准（见上，USB 线就是例子）。
// 3) 单点乘性校准修不了零点偏移：adc 327 按理是 0.264V，v_pin 却报 0.292V，
//    差约 28mV。纯乘法消不掉它，还会把它一起放大。要修得做两点标定。
// 4) 但两点标定现在做不了：用 v_pin 0.292 / 0.451 拟合得 a=10.50、b=+0.263，
//    意味着表笔接地时应报 0.26V，而实测接地/悬空都是 adc 0 → 0.00V，直接矛盾；
//    且两点只跨输入 3.2~5.0V，外推到 0V 和 12V/24V 不成立。
//
// 结论：保持 1.0055 别再调，先去做一次同时测量。详见 校准记录.md。
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
