// Day 10 任务二：电位器手动调光（与板载彩灯并行）
//
// 电路（在 Day 9 基础上只加一个电位器，LED 那一路不动）：
//   GPIO2  → 220Ω → LED 长脚(阳极) → LED 短脚(阴极) → GND
//   电位器  两端脚 → 3V3 / GND；中间脚(滑臂) → GPIO1（ADC1_CH0）
//
// 依赖：RgbCycle 库（~/Documents/Arduino/libraries/RgbCycle/）
// 关键：整个 loop() 里没有一处 delay()，彩灯换色与手动调光互不干扰。

#include <RgbCycle.h>

const int LED_PIN = 2;
const int POT_PIN = 1;    // GPIO1 = ADC1_CH0（ADC1 才不与 Wi-Fi 冲突）

// 每 20ms 采一次：既跟手，又不至于把串口刷爆
const unsigned long READ_INTERVAL = 20;
unsigned long lastRead = 0;

void setup() {
  RgbCycle::begin();            // 彩灯：初始化
  RgbCycle::setInterval(800);   // 彩灯：800ms/色（一个循环 2.4s）

  ledcAttach(LED_PIN, 5000, 8); // 外接 LED：绑定 PWM，8 位 → duty 0~255
  ledcWrite(LED_PIN, 0);

  Serial.begin(115200);
  Serial.println("ESP32-S3 电位器调光 Start");
}

void loop() {
  // ---- 任务 A：板载彩灯（非阻塞，内部按 800ms 换色）----
  RgbCycle::update();

  // ---- 任务 B：电位器手动调光 ----
  unsigned long now = millis();
  if (now - lastRead >= READ_INTERVAL) {
    lastRead = now;

    int potValue   = analogRead(POT_PIN);              // 0~4095（12 位 ADC）
    int brightness = map(potValue, 0, 4095, 0, 255);   // 压到 duty 的 0~255

    ledcWrite(LED_PIN, brightness);
    Serial.printf("Pot: %4d  Brightness: %3d\n", potValue, brightness);
  }
}
