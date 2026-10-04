/*
  Day 31 实验2：反应计时游戏——中断版

  按键用 attachInterrupt() 挂在 FALLING 边沿，按下瞬间立刻进 ISR。
  ISR 里只做三件事：打个时间戳、置个标志、分清是不是抢跑；
  串口打印、判断、改灯全在 loop() 里做。

  游戏规则、接线与实验1（轮询版）完全一致，方便直接对比。
*/

#include <RgbCycle.h>

const int PIN_BTN  = 1;
const int PIN_LED  = 2;
const int PIN_SEED = 3;

// 中断事件码。不能叫 LOW / HIGH —— Arduino 核心把这两个定义成了宏，编译必炸
enum Evt { EVT_NONE = 0, EVT_HIT, EVT_FALSE_START };
enum St  { ST_WAIT, ST_TIMING, ST_RESULT };

const int RANDOM_MIN_MS = 2000;
const int RANDOM_MAX_MS = 5000;
const unsigned long DEBOUNCE_MS = 50;     // ISR 内按时间戳去抖，不阻塞
const unsigned long RESULT_MS  = 1500;

// ISR 与 loop() 之间只靠这几个变量通信，凡是两边都要访问的必须 volatile
volatile Evt evt = EVT_NONE;
volatile unsigned long tPress   = 0;
volatile unsigned long lastIsr  = 0;
volatile bool timing = false;

St st = ST_WAIT;
unsigned long tRound  = 0;
unsigned long tLight  = 0;
unsigned long tResult = 0;
unsigned long waitMs  = 0;

// ESP32 的中断处理函数必须放在 IRAM 里（Flash 缓存被禁用时也能执行）
void IRAM_ATTR onPress() {
  unsigned long t = millis();
  if (t - lastIsr < DEBOUNCE_MS) return;   // 按键抖动：50ms 内的重复沿直接忽略
  lastIsr = t;
  if (timing) { tPress = t; evt = EVT_HIT; }
  else        { evt = EVT_FALSE_START; }
}

void startRound() {
  waitMs = random(RANDOM_MIN_MS, RANDOM_MAX_MS);
  tRound = millis();
  st = ST_WAIT;
  timing = false;
  digitalWrite(PIN_LED, LOW);
  RgbCycle::setColor(0, 255, 0);      // 绿：随机等待中，别按
}

void setup() {
  Serial.begin(115200);
  pinMode(PIN_BTN, INPUT_PULLUP);
  pinMode(PIN_LED, OUTPUT);
  randomSeed(analogRead(PIN_SEED));   // 悬空脚的噪声播种；若每轮随机序列一样，用手碰一下 GPIO3
  RgbCycle::begin();
  RgbCycle::setColor(0, 0, 0);
  attachInterrupt(digitalPinToInterrupt(PIN_BTN), onPress, FALLING);
  Serial.println("Day31 中断版：等外接 LED 亮起（板载转蓝）后再按键");
  startRound();
}

void loop() {
  unsigned long now = millis();

  // 1) 消费中断事件：ISR 不干活，重活都在这儿
  if (evt != EVT_NONE) {
    Evt e = evt;
    evt = EVT_NONE;                   // 读走就清，清标志这件事只能 loop() 做
    if (e == EVT_FALSE_START && st == ST_WAIT) {
      if (now - tRound >= 200) {      // 上电那一下的抖动不理会
        RgbCycle::setColor(255, 0, 0);
        Serial.println("抢跑！灯还没亮，本轮重新开始");
        startRound();
      }
    }
    if (e == EVT_HIT && st == ST_TIMING) {
      unsigned long dt = tPress - tLight;
      digitalWrite(PIN_LED, LOW);
      RgbCycle::setColor(255, 255, 0);   // 黄：本轮结束
      Serial.printf("reaction = %lu ms\n", dt);
      timing = false;
      st = ST_RESULT;
      tResult = now;
    }
  }

  // 2) 状态机
  switch (st) {
    case ST_WAIT:
      if (now - tRound >= waitMs) {
        digitalWrite(PIN_LED, HIGH);
        tLight = now;
        timing = true;
        st = ST_TIMING;
        RgbCycle::setColor(0, 0, 255);   // 蓝：计时中
      }
      break;
    case ST_TIMING:
      break;                          // 什么都不问，等中断来
    case ST_RESULT:
      if (now - tResult >= RESULT_MS) startRound();
      break;
  }
}
