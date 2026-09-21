/*
  实验1：舵机自动摆动
  ------------------------------------------------------------------
  让 SG90 舵机在 0° → 90° → 180° → 90° → 0° 之间来回摆动，
  每个位置停留，用 millis() 非阻塞调度（不用 delay）。

  接线（SG90 三条线，棕/红/橙）：

    ESP32-S3              SG90 舵机
    GPIO18  ────────────  黄色（信号）
    GND     ────────────  棕色（GND）
    外接 5V+ ────────────  红色（+5V）

    ⚠️ 红线绝不能接 ESP32 的 3V3 脚：舵机不是逻辑器件，
       堵转电流 700mA 以上，3.3V LDO 带不动，会把板子拖垮。

  脉冲协议：50Hz（周期 20ms），高电平宽度 0.5ms~2.5ms 对应 0°~180°。

  配套笔记：../README.md
*/

// 周期 20ms = 20000us，即 PWM 频率 50Hz。
// 舵机的内部控制回路需要时间采样和保持，所以是 50Hz 而不是 Day 19 的 5kHz。
const int SERVO_PIN = 18;
const int PWM_FREQ   = 50;      // 舵机协议固定 50Hz，不能改

// ⚠️ ESP32-S3 的 LEDC 最高只支持 14 位分辨率（SOC_LEDC_TIMER_BIT_WIDTH = 14）。
//    许多舵机教程写 16 位 —— 那是初代 ESP32（20 位）的参数，照抄到 S3 上
//    ledcAttach() 会直接返回 false，且不报任何错，表现就是"代码在跑、舵机不动"。
//    14 位也完全够：20000us / 16384 = 1.22us 每步，0°~180° 有 1638 个台阶 ≈ 0.11°/步。
const int PWM_RES    = 14;

const int US_PERIOD = 20000;    // 50Hz 的周期，微秒
const int DUTY_MAX  = 16384;    // 2^14，与 PWM_RES 必须一致
const int US_MIN    = 500;      // 0.5ms  → 0°
const int US_MAX    = 2500;     // 2.5ms  → 180°
const int ANGLE_MIN = 0;
const int ANGLE_MAX = 180;

// 停留时间：舵机从当前位置转到目标位置需要时间（SG90 约 0.1s 走 60°）。
// 300ms 足够 180° 全程走完，不会出现"还没到位就又发新指令"。
const unsigned long HOLD_MS = 300;

const int SEQ_LEN = 5;
const int SEQ[SEQ_LEN] = { 0, 90, 180, 90, 0 };

// 角度 → 14 位占空比
//   50Hz 下 14 位 = 16384 步，每步 20000/16384 ≈ 1.22us
//   duty = 脉冲宽度(us) × 16384 / 20000
//   0° → 500us  → 410
//   90° → 1500us → 1229
//   180° → 2500us → 2048
int angleToDuty(int deg) {
  if (deg < ANGLE_MIN) deg = ANGLE_MIN;
  if (deg > ANGLE_MAX) deg = ANGLE_MAX;
  float us = US_MIN + (float)(US_MAX - US_MIN) * deg / (ANGLE_MAX - ANGLE_MIN);
  return (int)(us * DUTY_MAX / (float)US_PERIOD + 0.5);   // +0.5 是四舍五入
}

int seqIdx = 0;
int angle  = SEQ[0];
unsigned long lastMs = 0;

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("\n=== 实验1：舵机自动摆动 ===");

  // ESP32 Arduino Core 3.x 的 PWM API：按引脚写，没有通道号。
  // 旧教程的 ledcSetup(通道, 频率, 分辨率) + ledcAttachPin(引脚, 通道) 已删除。
  //
  // ⚠️ ledcAttach / ledcWrite 都返回 bool。不检查返回值的话，引脚被别的外设
  //    占着、或 LEDC 通道分配失败时，后面全部静默失效，串口却照常打印，
  //    表现就是"代码在跑、舵机没反应"，极难排查。
  bool okAttach = ledcAttach(SERVO_PIN, PWM_FREQ, PWM_RES);
  bool okWrite  = ledcWrite(SERVO_PIN, angleToDuty(angle));

  // ⚠️ 别拿 ledcRead() / ledcReadFreq() 当判据，两个都会骗人：
  //   ledcReadFreq() 从时钟分频寄存器反推频率，50Hz 这种极低频常算出 0；
  //   ledcRead() 读的是影子寄存器，要等一个完整 PWM 周期（50Hz = 20ms）才同步，
  //   attach 后立刻读会拿到残留值 —— 实测读到 6，而真实 duty 是 410。
  //   实测这两个都误报过"失败"，而舵机其实转得好好的。
  // 唯一可靠的硬件判据就是 attach / write 的 bool 返回值：
  //   false → 引脚被占、分辨率超限（ESP32-S3 上限 14 位）、通道用尽
  Serial.printf("频率 %dHz / 分辨率 %d 位 → 每步 %.3fus\n",
                PWM_FREQ, PWM_RES, US_PERIOD / (float)DUTY_MAX);
  Serial.printf("角度 0 / 90 / 180 → duty %d / %d / %d\n",
                angleToDuty(0), angleToDuty(90), angleToDuty(180));
  Serial.printf("起点 %d°，每 %lums 走一格，序列 0-90-180-90-0\n",
                angle, HOLD_MS);

  Serial.printf("[自检] GPIO%d  attach=%s  write=%s\n",
                SERVO_PIN, okAttach ? "OK" : "失败", okWrite ? "OK" : "失败");
  if (!okAttach || !okWrite) {
    Serial.println("[自检] PWM 没起来！舵机不可能动 —— 先查这一行，别查舵机。");
  } else {
    Serial.println("[自检] PWM 已就绪。舵机再不动就是接线/供电/舵机本身的问题。");
  }
}

void loop() {
  if (millis() - lastMs < HOLD_MS) return;   // 非阻塞节流
  lastMs = millis();

  int duty = angleToDuty(angle);
  ledcWrite(SERVO_PIN, duty);
  Serial.printf("{\"angle\":%d,\"duty\":%d,\"us\":%.0f}\n",
                angle, duty, duty * US_PERIOD / (float)DUTY_MAX);

  seqIdx = (seqIdx + 1) % SEQ_LEN;
  angle  = SEQ[seqIdx];
}
