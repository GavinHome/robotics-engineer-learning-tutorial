/*
 * Day 9 — 板载 WS2812B 彩灯循环（绿 → 蓝 → 红）
 * 板卡：ESP32-S3-WROOM-1 N16R8（44 pin Type-C）
 * 依赖：Adafruit NeoPixel（Arduino IDE 库管理器安装）
 *
 * 说明：板载灯珠不是普通 LED，不能 digitalWrite 直接驱动，
 *       它是一颗单总线可编程的 WS2812B，控制脚接 GPIO48。
 */

#include <Adafruit_NeoPixel.h>

#define RGB_PIN   48   // 板载 WS2812B 控制脚
#define RGB_COUNT 1    // 只有一颗灯珠

// NEO_GRB + NEO_KHZ800 是 WS2812B 的标准时序组合
Adafruit_NeoPixel rgb(RGB_COUNT, RGB_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  rgb.begin();
  rgb.setBrightness(50);  // 0-255，先调低，避免刺眼
  rgb.show();             // 上电先熄灭
}

void loop() {
  rgb.setPixelColor(0, rgb.Color(0, 255, 0));   // 绿
  rgb.show();
  delay(500);

  rgb.setPixelColor(0, rgb.Color(0, 0, 255));   // 蓝
  rgb.show();
  delay(500);

  rgb.setPixelColor(0, rgb.Color(255, 0, 0));   // 红
  rgb.show();
  delay(500);
}
