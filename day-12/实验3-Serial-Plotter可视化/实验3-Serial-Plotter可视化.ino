#include <RgbCycle.h>

const int ADC_PIN = 1;   // GPIO1 = ADC1_CH0

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
  Serial.println(voltage);
  delay(500);
}
