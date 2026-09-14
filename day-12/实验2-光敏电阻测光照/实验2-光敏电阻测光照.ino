#include <RgbCycle.h>

const int ADC_PIN = 1;   // GPIO1 = ADC1_CH0

void setup() {
  RgbCycle::begin();               // 彩灯：初始化
  RgbCycle::setInterval(800);      // 彩灯：800ms/色

  Serial.begin(115200);
  analogReadResolution(12);        // 12 位 ADC
  analogSetAttenuation(ADC_11db);  // 0-3.3V 全量程
}

void loop() {
  RgbCycle::update();              // 任务 A：彩灯照常循环

  int raw = analogRead(ADC_PIN);
  float voltage = raw * 3.3 / 4095.0;
  Serial.printf("Raw: %4d  Voltage: %.2fV\n", raw, voltage);

  delay(500);
}
