/*
 * Day 9 — 综合版：板载 WS2812B 彩灯循环 + 外接 LED 同步闪烁
 * 板卡：ESP32-S3-WROOM-1 N16R8
 *
 * 两路同时跑在同一个 loop() 里：
 *   1. 板载 GPIO48 彩灯：绿 → 蓝 → 红 → 全灭（WS2812B 数字通信）
 *   2. 外接 GPIO2 LED：亮 → 灭 → 亮 → 灭（digitalWrite 直驱）
 */

#include <Adafruit_NeoPixel.h>

// ---------- 板载彩灯 ----------
#define RGB_PIN   48
#define RGB_COUNT 1
Adafruit_NeoPixel rgb(RGB_COUNT, RGB_PIN, NEO_GRB + NEO_KHZ800);

// ---------- 外接 LED ----------
#define EXT_LED_PIN 2

void setup() {
  rgb.begin();
  rgb.setBrightness(50);
  rgb.show();

  pinMode(EXT_LED_PIN, OUTPUT);
  digitalWrite(EXT_LED_PIN, LOW);
}

void loop() {
  // 绿 + LED 亮
  rgb.setPixelColor(0, rgb.Color(0, 255, 0));
  rgb.show();
  digitalWrite(EXT_LED_PIN, HIGH);
  delay(500);

  // 蓝 + LED 灭
  rgb.setPixelColor(0, rgb.Color(0, 0, 255));
  rgb.show();
  digitalWrite(EXT_LED_PIN, LOW);
  delay(500);

  // 红 + LED 亮
  rgb.setPixelColor(0, rgb.Color(255, 0, 0));
  rgb.show();
  digitalWrite(EXT_LED_PIN, HIGH);
  delay(500);

  // 全灭 + LED 灭
  rgb.setPixelColor(0, rgb.Color(0, 0, 0));
  rgb.show();
  digitalWrite(EXT_LED_PIN, LOW);
  delay(500);
}
