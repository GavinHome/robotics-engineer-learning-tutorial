// Day 10 任务 3：PWM 呼吸灯（外接 LED，GPIO2）
//
// 电路（复用 Day 9 的外接 LED，一根线都不用加）：
//   GPIO2 → 220Ω → LED 长脚(阳极) → LED 短脚(阴极) → GND
//
// API 说明：esp32 3.x 已删除 ledcSetup() / ledcAttachPin()，
//          改用 ledcAttach(pin, freq, resolution)，ledcWrite 按【引脚】写。

const int LED_PIN = 2;

// 每档停留 4ms：256 档 × 4ms ≈ 1.02s 单向，一个完整呼吸 ≈ 2s
const int STEP_MS = 4;

void setup() {
  // 一次绑定：引脚 2, 5kHz, 8 位分辨率 → duty 范围 0~255
  ledcAttach(LED_PIN, 5000, 8);
  ledcWrite(LED_PIN, 0);          // 上电从灭开始

  Serial.begin(115200);
  Serial.println("ESP32-S3 PWM 呼吸灯 Start");
}

void loop() {
  // 渐亮：duty 0 → 255
  for (int b = 0; b <= 255; b++) {
    ledcWrite(LED_PIN, b);
    delay(STEP_MS);
  }
  // 渐暗：duty 255 → 0
  for (int b = 255; b >= 0; b--) {
    ledcWrite(LED_PIN, b);
    delay(STEP_MS);
  }
}
