#include <RgbCycle.h>

const int ADC_PIN = 1;   // GPIO1 = ADC1_CH0（电位器 / 光敏电阻复用）

void setup() {
  RgbCycle::begin();
  RgbCycle::setInterval(800);

  Serial.begin(115200);
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);  // 量程 0-3.3V
}

void loop() {
  RgbCycle::update();

  int raw = analogRead(ADC_PIN);
  float voltage = raw * 3.3 / 4095.0;

  // 用 Serial.printf 输出 JSON 格式字符串，方便 Python/浏览器解析
  Serial.printf("{\"sensor\": %d, \"voltage\": %.2f}\n", raw, voltage);

  delay(1000);
}
