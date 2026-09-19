// Day 17：HC-SR04 超声波测距
//
// 电路（宽电压 3-5.5V 版 HC-SR04）：
//
//   HC-SR04          ESP32-S3
//   ───────          ────────
//   VCC      ──────  3V3（板载 LDO 输出，有驱动能力；板上两个 3V3 任选其一）
//   GND      ──────  GND（必须共地）
//   Trig     ──────  GPIO4（输出：发 10μs 高脉冲）
//   Echo     ──────  GPIO5（输入：回响高电平，宽度 ∝ 距离）
//
// ⚠️ 接线顺序：先接 GND，最后接 VCC。拆线反过来。
//
// ⚠️ 为什么 VCC 接 3V3 而不是 5V（2026-09-19 修订，详见 day-17/README.md §三）：
//    ① 板上 `5Vin` 是电源【输入】脚，不是稳压输出，没有驱动能力
//       （Day 14 已踩坑：空载量到 4.4V，接 11kΩ 负载后塌到 0）
//    ② 宽电压版 Echo 高电平跟随 VCC → 3V3 供电时 Echo 就是 3.3V，
//       可直连 GPIO5，连 1kΩ+2kΩ 分压都省了
//    代价是发射功率略低。2026-09-19 实测 3V3 供电可读到 173cm（见 README §六）。
//
// 时序（一个测量周期）：
//
//   Trig  ──────┐          ┌────────────────────────
//               │  10μs    │
//               └──────────┘
//                          模块内部发出 8 个 40kHz 脉冲
//   Echo  ──────────────┐                    ┌──────
//                       │  宽度 = 往返时间    │
//                       └────────────────────┘
//                       ←────── duration ────→
//
//   距离 cm = duration(μs) / 58
//           = duration(μs) × 0.0343 / 2      （0.0343 cm/μs 是声速）

#include <RgbCycle.h>

// ---------- 引脚 ----------
// 没沿用指南的 GPIO5/18：4/5 在排针上相邻好走线，也避开 Day 18 打算用的 GPIO8/9（I2C）。
// 若你插线时想换脚，只改这两行即可。
const int TRIG_PIN = 4;
const int ECHO_PIN = 5;

// ---------- 物理常量 ----------
// 声速 343 m/s = 0.0343 cm/μs。除以 2 是因为脉冲走了个来回。
const float CM_PER_US = 0.0343 / 2.0;   // 0.01715 cm/μs

// 手册标称 2~400cm。400cm 往返 ≈ 23.3ms，留余量取 30ms 超时。
const unsigned long TIMEOUT_US = 30000;

// 两次测量之间的间隔：手册建议 ≥60ms，避免上一次回波干扰下一次。
const unsigned long CYCLE_MS = 100;

// 模块标称下限 2cm，低于此值读数不可信（发射脉冲还没结束就收到回波）。
const float MIN_VALID_CM = 2.0;
const float MAX_VALID_CM = 400.0;

// ---------- 距离→灯色 ----------
// 沿用 Day 14 的"状态指示"思路：灯不再是心跳，而是距离档位。
// 注意：不能叫 OK / LOW / HIGH —— Arduino 核心已把它们定义成宏，会冲突。
enum DistZone { D_NEAR, D_MID, D_FAR, D_INVALID };
DistZone lastZone = D_INVALID;

void showZone(DistZone z) {
  switch (z) {
    case D_NEAR:    RgbCycle::setColor(255, 0, 0);   break;  // 红：很近
    case D_MID:     RgbCycle::setColor(255, 160, 0); break;  // 橙：中距
    case D_FAR:     RgbCycle::setColor(0, 255, 0);   break;  // 绿：远/通畅
    case D_INVALID: RgbCycle::setColor(0, 0, 255);   break;  // 蓝：超时/超量程
  }
}

// ---------- 一次测量 ----------
// 返回微秒数；0 表示超时（pulseIn 超时返回 0）。
unsigned long measureOnce() {
  // 先拉低 2μs，确保 Trig 上是一个干净的上升沿
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);          // 手册要求 ≥10μs
  digitalWrite(TRIG_PIN, LOW);

  // pulseIn 阻塞等待高电平，返回其持续微秒数；超时返回 0。
  return pulseIn(ECHO_PIN, HIGH, TIMEOUT_US);
}

void setup() {
  Serial.begin(115200);

  RgbCycle::begin();      // 只初始化灯珠，不调 update()（那会循环变色，覆盖档位色）
  showZone(D_INVALID);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  digitalWrite(TRIG_PIN, LOW);

  Serial.println("HC-SR04 ready. Trig=GPIO" + String(TRIG_PIN) +
                 " Echo=GPIO" + String(ECHO_PIN));
  delay(200);
}

void loop() {
  unsigned long duration = measureOnce();

  float cm = 0.0;
  DistZone z;
  const char* label;

  if (duration == 0) {                       // 超时：前方无障碍或超出 400cm
    z = D_INVALID; label = "TIMEOUT";
  } else {
    cm = duration * CM_PER_US;
    if (cm < MIN_VALID_CM)      { z = D_NEAR;    label = "TOO_CLOSE"; }
    else if (cm <= 30.0)        { z = D_NEAR;    label = "NEAR";      }
    else if (cm <= 150.0)       { z = D_MID;     label = "MID";       }
    else if (cm <= MAX_VALID_CM){ z = D_FAR;     label = "FAR";       }
    else                        { z = D_INVALID; label = "OUT_OF_RANGE"; }
  }

  if (z != lastZone) {          // 只在档位变化时刷新灯，避免每帧重复写灯珠
    showZone(z);
    lastZone = z;
  }

  // 输出一行 JSON（延续 Day 13/14 的格式，方便 Python 直接解析）
  Serial.printf("{\"us\": %lu, \"cm\": %.1f, \"zone\": \"%s\"}\n",
                duration, cm, label);

  delay(CYCLE_MS);
}
