/*
  实验2：串口输入控制舵机角度
  ------------------------------------------------------------------
  在串口监视器里输入 0~180 的数字回车，舵机转到对应角度并回显。
  输入非数字或超出范围会被拒绝。

  接线同实验1：

    GPIO18  ──── 黄色（信号）
    GND     ──── 棕色（GND）
    外接 5V+ ──── 红色（+5V）

  ⚠️ Serial.parseInt() 有几个坑，见 README「串口输入的三个坑」一节。
     超时后它返回 0，而 0 是合法角度 —— 所以这里显式判断解析结果。

  配套笔记：../README.md
*/

const int SERVO_PIN = 18;
const int PWM_FREQ   = 50;      // 舵机协议固定 50Hz

// ⚠️ ESP32-S3 的 LEDC 最高只支持 14 位（SOC_LEDC_TIMER_BIT_WIDTH = 14）。
//    舵机教程常见的 16 位是初代 ESP32 的参数，照抄到 S3 上 ledcAttach()
//    直接返回 false 且不报错，表现就是"串口正常、舵机不动"。
const int PWM_RES    = 14;

const int US_PERIOD = 20000;
const int DUTY_MAX  = 16384;    // 2^14，与 PWM_RES 必须一致
const int US_MIN    = 500;      // 0.5ms  → 0°
const int US_MAX    = 2500;     // 2.5ms  → 180°
const int ANGLE_MIN = 0;
const int ANGLE_MAX = 180;

int angleToDuty(int deg) {
  if (deg < ANGLE_MIN) deg = ANGLE_MIN;
  if (deg > ANGLE_MAX) deg = ANGLE_MAX;
  float us = US_MIN + (float)(US_MAX - US_MIN) * deg / (ANGLE_MAX - ANGLE_MIN);
  return (int)(us * DUTY_MAX / (float)US_PERIOD + 0.5);
}

void applyAngle(int deg) {
  int duty = angleToDuty(deg);
  ledcWrite(SERVO_PIN, duty);
  Serial.printf("{\"angle\":%d,\"duty\":%d,\"us\":%.0f}\n",
                deg, duty, duty * US_PERIOD / (float)DUTY_MAX);
}

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("\n=== 实验2：串口控制舵机角度 ===");
  Serial.println("输入 0~180 的整数后回车，舵机转到对应角度。");

  // ledcAttach 返回 bool，不检查就静默失效：
  // 引脚被占或分辨率超限时串口照常打印，舵机却完全不动。
  // 别用 ledcRead() / ledcReadFreq() 复核，两个都会误报（详见实验1 的注释）。
  bool okAttach = ledcAttach(SERVO_PIN, PWM_FREQ, PWM_RES);
  Serial.printf("[自检] GPIO%d  attach=%s\n",
                SERVO_PIN, okAttach ? "OK" : "失败");
  if (!okAttach) Serial.println("[自检] PWM 没起来！先查这一行，别查舵机。");

  // 上电先回中位，确保舵机处在已知位置，也顺手验证一次通路。
  applyAngle(90);
}

void loop() {
  if (!Serial.available()) return;

  // parseInt 默认 1 秒超时，无输入就返回 0 —— 0 是合法角度，直接执行会把舵机打到限位。
  // 所以用 setTimeout(10) 把等待期压到 10ms，再靠范围判断挡住 0 之外的一切非数字输入。
  Serial.setTimeout(10);
  long parsed = Serial.parseInt();

  // 把这一行剩下的字符（含换行）读干净，否则残留数字会被下一次 parseInt 拼上。
  while (Serial.available()) Serial.read();

  if (parsed >= ANGLE_MIN && parsed <= ANGLE_MAX) {
    applyAngle((int)parsed);
  } else {
    Serial.printf("IGNORED: %ld 不在 %d~%d 范围内\n",
                  parsed, ANGLE_MIN, ANGLE_MAX);
  }
}
