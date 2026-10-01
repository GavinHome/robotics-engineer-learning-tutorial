/*
 * Day 9 — 外接 LED 经典 Blink（第二份：独立版）
 * 板卡：ESP32-S3-WROOM-1 N16R8
 *
 * 电路：GPIO2 → 220Ω 限流电阻 → LED（长脚）→ LED（短脚）→ GND
 *
 * 这是 Arduino 官方 Blink 示例在本板上的直接移植：
 * digitalWrite(HIGH) → GPIO2 输出 3.3V → 电流经电阻流过 LED → 亮
 * digitalWrite(LOW)  → GPIO2 输出 0V  → LED 两端无压降         → 灭
 */

const int EXT_LED_PIN = 2;   // 外接 LED 控制脚

void setup() {
  pinMode(EXT_LED_PIN, OUTPUT);
  digitalWrite(EXT_LED_PIN, LOW);   // 上电先灭
  Serial.begin(115200);
  Serial.println("ESP32-S3 外接 LED Blink Start");
}

void loop() {
  digitalWrite(EXT_LED_PIN, HIGH);
  Serial.println("LED ON");
  delay(500);

  digitalWrite(EXT_LED_PIN, LOW);
  Serial.println("LED OFF");
  delay(500);
}
