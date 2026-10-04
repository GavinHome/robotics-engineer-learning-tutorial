/*
  Day 31 实验1：反应计时游戏——轮询版

  在 loop() 里用 digitalRead() 问按键"按了没"，问一次算一次。
  和实验2（中断版）跑同一套游戏规则，用来对比两者的检出延迟。

  规则：
    1. 绿灯亮 = 随机等待中，此时按了算"抢跑"
    2. 延时 2~5 秒后外接 LED 亮起（板载灯转蓝），计时开始
    3. 按键断开计时，串口打印 reaction = xx ms
    4. 黄灯展示 1.5 秒后自动开下一轮

  接线：
    GPIO1  → 按键一脚，按键另一脚 → GND（用内部上拉，不按=高、按下=低）
    GPIO2  → 1kΩ 电阻 → LED 长脚，LED 短脚 → GND
    GPIO3  → 悬空，只给 randomSeed 提供噪声
    GPIO48 → 板载 WS2812B（RgbCycle 库）

  ── 两种配置：整个文件只差 loop() 里那一行 delay ──
  A. 空转：loop() 不加延时，实测 ≈50万 圈/秒，一圈 ≈2 µs
     日志 ../实验1-串口打印.txt，4 轮 reaction 358 / 750 / 788 / 1802 ms
  B. 每圈 delay(50)：loop() 每圈固定睡 50ms，实测 20 圈/秒，一圈 ≈50 ms
     日志 ../实验1-串口打印-delay50.txt，21 轮 reaction 全是 50 的整数倍
  两配置唯一的影响"轮询最多要等多久才发现按键"：A 是 2 µs，B 是 50 ms。
  想切回 A，把 loop() 里那行 delay(50); 重新注释掉即可。
*/

#include <RgbCycle.h>

const int PIN_BTN  = 1;
const int PIN_LED  = 2;
const int PIN_SEED = 3;

// 枚举成员不能叫 LOW / HIGH（Arduino 核心把这两个定义成了宏）
enum St { ST_WAIT, ST_TIMING, ST_RESULT };

const int RANDOM_MIN_MS = 2000;
const int RANDOM_MAX_MS = 5000;
const unsigned long DEBOUNCE_MS = 50;     // 轮询版去抖：在 loop() 里按时间戳判
const unsigned long RESULT_MS  = 1500;

St st = ST_WAIT;
unsigned long tRound  = 0;
unsigned long tLight  = 0;
unsigned long tResult = 0;
unsigned long waitMs  = 0;

// 轮询版专有：统计 loop() 跑多快，用来量化"多久才能发现按键被按下"
unsigned long loopsThisSec = 0;
unsigned long lastReport = 0;

void startRound() {
  waitMs = random(RANDOM_MIN_MS, RANDOM_MAX_MS);
  tRound = millis();
  st = ST_WAIT;
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
  Serial.println("Day31 轮询版：等外接 LED 亮起（板载转蓝）后再按键");
  startRound();
}

void loop() {
  unsigned long now = millis();

  // 每圈固定睡 50ms，模拟"loop() 里有别的活要干"——轮询版最多要等这么久才发现按键
  delay(50);
  loopsThisSec++;
  if (now - lastReport >= 1000) {
    lastReport = now;
    Serial.printf("[轮询] loop = %lu 圈/秒\n", loopsThisSec);
    loopsThisSec = 0;
  }

  switch (st) {
    case ST_WAIT:
      if (digitalRead(PIN_BTN) == LOW) {          // 抢跑
        if (now - tRound >= 200) {                // 上电那一下的抖动不理会
          RgbCycle::setColor(255, 0, 0);
          Serial.println("抢跑！灯还没亮，本轮重新开始");
          startRound();
        }
        break;
      }
      if (now - tRound >= waitMs) {
        digitalWrite(PIN_LED, HIGH);
        tLight = now;
        st = ST_TIMING;
        RgbCycle::setColor(0, 0, 255);            // 蓝：计时中
      }
      break;

    case ST_TIMING:
      if (digitalRead(PIN_BTN) == LOW && now - tLight >= DEBOUNCE_MS) {
        unsigned long dt = now - tLight;
        digitalWrite(PIN_LED, LOW);
        RgbCycle::setColor(255, 255, 0);          // 黄：本轮结束
        Serial.printf("reaction = %lu ms\n", dt);
        st = ST_RESULT;
        tResult = now;
      }
      break;

    case ST_RESULT:
      if (now - tResult >= RESULT_MS) startRound();
      break;
  }
}
