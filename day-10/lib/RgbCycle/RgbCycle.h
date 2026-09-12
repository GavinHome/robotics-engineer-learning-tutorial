// RgbCycle.h — 板载 WS2812B 彩灯循环（非阻塞）
//
// 从 Day 9 的 rgb_cycle.ino 抽出，改造成可在任意草图里复用的模块。
//
// 用法：
//   #include <RgbCycle.h>
//   void setup() { RgbCycle::begin(); RgbCycle::setInterval(800); }
//   void loop()  { RgbCycle::update();  /* 其他任务 */ }
//
// 注意：update() 内部用 millis() 判断节奏，绝不 delay()，
//       因此不会阻塞同一 loop() 里的其他任务。

#pragma once

#include <Adafruit_NeoPixel.h>

namespace RgbCycle {

constexpr uint8_t  PIN     = 48;    // 板载灯珠控制脚（net RGB_CTRL）
constexpr uint8_t  COUNT   = 1;     // 板上只有一颗灯珠
constexpr uint8_t  BRIGHT  = 50;    // 0-255，低亮度避免直视刺眼

// 默认每色停留 ms；可在 setup() 里用 setInterval() 按任务单独调整
constexpr unsigned long DEFAULT_INTERVAL = 500;

inline Adafruit_NeoPixel strip(COUNT, PIN, NEO_GRB + NEO_KHZ800);
inline unsigned long interval = DEFAULT_INTERVAL;
inline unsigned long lastMs   = 0;
inline uint8_t       index    = 0;

// 在 setup() 里调用一次
inline void begin() {
  strip.begin();
  strip.setBrightness(BRIGHT);   // 0-255，先调低亮度，避免直视刺眼
  strip.show();                  // 上电先熄灭，清掉复位期间锁存的随机颜色
  lastMs = millis();
}

// 调整本任务的换色节奏（每色停留 ms）。不调用则用默认 800ms。
// 例：RgbCycle::setInterval(300);  // 这个任务想快闪
inline void setInterval(unsigned long ms) {
  interval = ms;
}

// 在 loop() 里每圈调用一次；内部自己判断是否到点，不阻塞
inline void update() {
  unsigned long now = millis();
  if (now - lastMs < interval) return;   // 用相减判断，millis() 溢出回绕也正确
  lastMs = now;

  switch (index) {
    case 0: strip.setPixelColor(0, strip.Color(0, 255, 0)); break;   // 绿
    case 1: strip.setPixelColor(0, strip.Color(0, 0, 255)); break;   // 蓝
    case 2: strip.setPixelColor(0, strip.Color(255, 0, 0)); break;   // 红
  }
  strip.show();                  // setPixelColor 只写内存，show() 才真正推给灯珠

  index = (index + 1) % 3;
}

// 需要让彩灯停在某个固定颜色时调用（调用后 update() 仍会继续循环变色）
inline void setColor(uint8_t r, uint8_t g, uint8_t b) {
  strip.setPixelColor(0, strip.Color(r, g, b));
  strip.show();
}

}  // namespace RgbCycle
