// Day 10 综合版：板载彩灯循环 + 外接 LED 呼吸，两路各走各的节奏
//
// 电路 = Day 9 实验三原样（不用加线）：
//   GPIO48 → 板载 WS2812B（板内走线）
//   GPIO2  → 220Ω → LED 长脚(阳极) → LED 短脚(阴极) → GND
//
// 依赖：RgbCycle 库（已装在 ~/Documents/Arduino/libraries/RgbCycle/）
//       —— 把 Day 9 的彩灯循环封装成模块，一行 update() 即可复用。
//
// 关键：整个 loop() 里没有一处 delay()，两路节奏互不干扰。

#include <RgbCycle.h>

const int LED_PIN = 2;

// ---- 呼吸灯自己的节奏 ----
// 每 4ms 走一档，duty 步进 1：256 档 × 4ms ≈ 1.02s 单向，一个完整呼吸 ≈ 2s
unsigned long lastBreath = 0;
const unsigned long BREATH_INTERVAL = 4;
const int BREATH_STEP = 1;
int brightness = 0;
int breathDir  = 1;      // +1 渐亮, -1 渐暗

void setup() {
  RgbCycle::begin();            // 彩灯：初始化（内含 setBrightness + show 熄灭）
  RgbCycle::setInterval(800);   // 彩灯：本任务用 800ms/色（一个循环 2.4s）

  ledcAttach(LED_PIN, 5000, 8); // 外接 LED：绑定 PWM
  ledcWrite(LED_PIN, 0);        // 上电从灭开始
}

void loop() {
  unsigned long now = millis();

  // ---- 任务 A：板载彩灯（非阻塞，内部按 500ms 换色）----
  RgbCycle::update();

  // ---- 任务 B：外接 LED 呼吸（非阻塞，内部按 12ms 步进）----
  if (now - lastBreath >= BREATH_INTERVAL) {
    lastBreath = now;

    brightness += breathDir * BREATH_STEP;
    if (brightness >= 255) { brightness = 255; breathDir = -1; }
    if (brightness <= 0)   { brightness = 0;   breathDir =  1; }

    ledcWrite(LED_PIN, brightness);
  }
}
