#include <RgbCycle.h>

const int BUTTON_PIN = 1;
const int LED_PIN = 2;

void setup() {
  RgbCycle::begin();            // 彩灯：初始化

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);
  Serial.begin(115200);
}

void loop() {
  RgbCycle::update();

  int buttonState = digitalRead(BUTTON_PIN);
  if (buttonState == LOW) {
    digitalWrite(LED_PIN, HIGH);
    Serial.println("Button PRESSED");
    delay(200);  // 简单消抖
  } else {
    digitalWrite(LED_PIN, LOW);
  }
  delay(10);
}
