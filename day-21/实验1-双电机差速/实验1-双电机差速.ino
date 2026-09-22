// Day 21 实验 1：双电机差速运动
//
// 把 Day 19 的 A 通道复制成 A+B 两路，用左右轮速差实现转向。
//
// 电路（TB6612FNG 两路全用上，STBY 仍接 3V3 常高）：
//
//   TB6612FNG        ESP32-S3 / 电源
//   ─────────        ───────────────
//   VCC      ──────  3V3（逻辑）
//   GND      ──────  ESP32 GND 且 电池负极（共地！）
//   STBY     ──────  3V3（常高）
//   AIN1     ──────  GPIO10      左电机
//   AIN2     ──────  GPIO11
//   PWMA     ──────  GPIO12
//   AO1      ──────  左电机 +（正极）
//   AO2      ──────  左电机 −（负极）
//   BIN1     ──────  GPIO15      右电机
//   BIN2     ──────  GPIO16
//   PWMB     ──────  GPIO17
//   BO1      ──────  右电机 +（正极）
//   BO2      ──────  右电机 −（负极）
//   正负极接反不会烧，只是转向反过来 —— 前进时轮子倒着转就对调这两根
//   VM       ──────  电池盒正极（只装 2 节 5 号 ≈ 3V）
//
// 三个要点：
// ① 转向不靠"转向机构"，靠左右轮速差。两轮同速 = 直行；两轮反向 = 原地打转；
//    两轮同向不同速 = 走弧线，速差越大弯越急。
// ② duty 不能照抄指南的 150/200。Day 19 实测：3V 供电下 duty 180（≈2.12V）
//    掉在死区里只嗡嗡响不转，220（≈2.59V）才起转。判据是平均电压，不是占空比百分比。
//    现在用 6V 供电，占空比砍半，平均电压仍是 2.7V。
// ③ Core 3.x 的 ledcAttach(pin, ...) 按引脚分配通道，两个电机各占一路，
//    不会互相覆盖。指南那个 Motor 类把 _channel 硬编码成 0，两个实例会抢
//    同一路 LEDC —— 旧 API 时代真实的坑，新 API 天然没有。

#include <RgbCycle.h>

// ---------- 引脚 ----------
// 左 = A 通道，右 = B 通道。10/11/12 沿用 Day 19；15/16/17 与它们同侧排针，线短也够得到。
// 都避开 strapping（0/3/45/46）、Flash/PSRAM（26~37）、USB（19/20）。
const int AIN1 = 10, AIN2 = 11, PWMA = 12;   // 左轮
const int BIN1 = 15, BIN2 = 16, PWMB = 17;   // 右轮

// ---------- 驱动参数 ----------
// 6V（4 节 5 号）供电。马达额定 3V，靠占空比把平均电压压回去：
//   6V × 115/255 = 2.7V，与 3V × 230/255 完全一样。
// 换 6V 不是为更快，是为更低内阻 —— 两个电机同时启动的峰值电流，2 节 5 号供不出来。
const int DUTY_CRUISE = 115;    // 直行（平均 2.7V）
const int DUTY_TURN   = 120;    // 原地转向（两轮反向，互相较劲，负载最大）
const int DUTY_SLOW   = 75;     // 弧线内侧轮（平均 1.8V）
const int PWM_FREQ    = 20000;  // 20kHz，超出人耳范围，消除 PWM 啸叫
const int PWM_RES     = 8;      // duty 0~255

// 两个 TT 马达存在个体差异：同样的 duty 转速不同，直行会跑偏。
// 实测车往哪边偏，就把另一边的 duty 加几个点补偿。0 = 未校准。
const int TRIM_RIGHT = 0;

// ---------- 运动模式 ----------
// 枚举名避开 Arduino 核心宏 OK / LOW / HIGH
enum Move {
  M_FORWARD, M_BACK, M_PIVOT_L, M_PIVOT_R,
  M_ARC_L, M_ARC_R, M_BRAKE, M_COAST
};
// 每个动作的中文说明 + 灯色，串口逐行打印，便于对照观察
const char* MOVE_DESC[] = {
  "前进  两轮都向前",
  "后退  两轮都向后",
  "原地左转  左轮后退+右轮前进（车体原地打转）",
  "原地右转  左轮前进+右轮后退（车体原地打转）",
  "左弧线  左轮75慢+右轮115快（弯向左）",
  "右弧线  左轮115快+右轮75慢（弯向右）",
  "刹车  两轮短接，立刻停住",
  "滑行  两轮断电，靠惯性慢停"
};
const char* MOVE_LED[] = {
  "绿", "橙", "青", "紫", "黄", "黄", "红", "蓝"
};

// 演示序列：每个动作 6 秒，观察完再进下一个
const Move SEQ[] = {
  M_FORWARD, M_BACK, M_PIVOT_L, M_PIVOT_R,
  M_ARC_L,   M_ARC_R, M_BRAKE,  M_COAST
};
const int SEQ_LEN = sizeof(SEQ) / sizeof(SEQ[0]);
const unsigned long HOLD_MS = 6000;   // 每个动作统一 6 秒

// 记录当前动作实际发给左右轮的 duty，供串口打印核对
int lastLeft = 0, lastRight = 0;

// ---------- 单个电机 ----------
// speed: -255 ~ +255。0 = 滑行（双高阻），不是刹车。
void driveMotor(int in1, int in2, int pwmPin, int speed) {
  if (speed > 0) {
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
  } else if (speed < 0) {
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
  } else {
    // 滑行：两端都断开，靠惯性停。ledcWrite 给 0，输出高阻。
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
    ledcWrite(pwmPin, 0);
    return;
  }
  ledcWrite(pwmPin, abs(speed));
}

// 刹车：两端同时拉到同一电平（短接）+ PWM 非 0。
// ⚠️ PWM=0 时输出是高阻，与 IN 无关 —— "IN1=IN2=1 但 PWM=0" 依然是滑行。
void brakeMotor(int in1, int in2, int pwmPin) {
  digitalWrite(in1, HIGH);
  digitalWrite(in2, HIGH);
  ledcWrite(pwmPin, 255);
}

void setMove(Move m) {
  // 灯色 = 当前运动，先定灯再定轮
  switch (m) {
    case M_FORWARD: RgbCycle::setColor(0, 255, 0);   break;  // 绿：前进
    case M_BACK:    RgbCycle::setColor(255, 160, 0); break;  // 橙：后退
    case M_PIVOT_L: RgbCycle::setColor(0, 255, 255); break;  // 青：原地左转
    case M_PIVOT_R: RgbCycle::setColor(255, 0, 255); break;  // 紫：原地右转
    case M_ARC_L:
    case M_ARC_R:   RgbCycle::setColor(255, 255, 0); break;  // 黄：弧线
    case M_BRAKE:   RgbCycle::setColor(255, 0, 0);   break;  // 红：刹车
    case M_COAST:   RgbCycle::setColor(0, 0, 255);   break;  // 蓝：滑行
  }

  if (m == M_BRAKE) {   // 两端短接 + PWM 非 0，两个电机各来一次
    brakeMotor(AIN1, AIN2, PWMA);
    brakeMotor(BIN1, BIN2, PWMB);
    lastLeft = 255; lastRight = 255;   // 刹车档，仅表示"短接"
    return;
  }
  if (m == M_COAST) {   // 双高阻，靠惯性停
    driveMotor(AIN1, AIN2, PWMA, 0);
    driveMotor(BIN1, BIN2, PWMB, 0);
    lastLeft = 0; lastRight = 0;
    return;
  }

  int rl = 0, rr = 0;   // 左轮 / 右轮 speed
  switch (m) {
    case M_FORWARD: rl =  DUTY_CRUISE; rr =  DUTY_CRUISE; break;
    case M_BACK:    rl = -DUTY_CRUISE; rr = -DUTY_CRUISE; break;
    // 原地转向：两轮等速反向，车体绕中心转，转弯半径 ≈ 0
    case M_PIVOT_L: rl = -DUTY_TURN;   rr =  DUTY_TURN;   break;
    case M_PIVOT_R: rl =  DUTY_TURN;   rr = -DUTY_TURN;   break;
    // 差速弧线：外侧轮全速、内侧轮降速，车体绕一个远处圆心走弧
    case M_ARC_L:   rl =  DUTY_SLOW;   rr =  DUTY_CRUISE; break;
    case M_ARC_R:   rl =  DUTY_CRUISE; rr =  DUTY_SLOW;   break;
    default: break;
  }

  // 右轮补偿：车往左偏说明右轮慢，加正补偿
  rr += TRIM_RIGHT;
  if (rr >  255) rr =  255;
  if (rr < -255) rr = -255;

  lastLeft = rl; lastRight = rr;
  driveMotor(AIN1, AIN2, PWMA, rl);
  driveMotor(BIN1, BIN2, PWMB, rr);
}

int seqIdx = 0;
unsigned long modeStart = 0;

void setup() {
  Serial.begin(115200);
  delay(200);

  RgbCycle::begin();

  pinMode(AIN1, OUTPUT); pinMode(AIN2, OUTPUT);
  pinMode(BIN1, OUTPUT); pinMode(BIN2, OUTPUT);
  // 两路各占一个 LEDC 通道，互不干扰
  ledcAttach(PWMA, PWM_FREQ, PWM_RES);
  ledcAttach(PWMB, PWM_FREQ, PWM_RES);

  Serial.println("Day21 实验1：双电机差速");
  Serial.printf("左轮 AIN1=GPIO%d AIN2=GPIO%d PWMA=GPIO%d\n", AIN1, AIN2, PWMA);
  Serial.printf("右轮 BIN1=GPIO%d BIN2=GPIO%d PWMB=GPIO%d\n", BIN1, BIN2, PWMB);
  Serial.printf("duty 直行=%d 转向=%d 弧线慢轮=%d 右轮补偿=%d\n",
                DUTY_CRUISE, DUTY_TURN, DUTY_SLOW, TRIM_RIGHT);
  Serial.println("每个动作 6 秒，灯色写在每行末尾");

  setMove(M_COAST);          // 上电先滑行一下，别抢跑
  delay(300);

  setMove(SEQ[0]);
  printMove(0);
  modeStart = millis();
}

// 动作"开始"的那一刻就打印，与灯色、轮子状态同一时刻。
// 若在动作结束时才打印，串口文字会比灯慢一拍，看着像灯和动作对不上。
void printMove(int i) {
  Move m = SEQ[i];
  char buf[48];

  if (m == M_BRAKE)      snprintf(buf, sizeof(buf), "两轮短接制动");
  else if (m == M_COAST) snprintf(buf, sizeof(buf), "两轮断电");
  else {
    // 正 = 前进，负 = 后退；abs 太小则平均电压低于启动电压，嗡嗡响但转不动
    const char* ld = lastLeft  < 0 ? "后退" : (lastLeft  > 0 ? "前进" : "停");
    const char* rd = lastRight < 0 ? "后退" : (lastRight > 0 ? "前进" : "停");
    snprintf(buf, sizeof(buf), "左轮=%s%d 右轮=%s%d", ld, abs(lastLeft), rd, abs(lastRight));
  }

  Serial.printf("%s灯 | %s | %s\n", MOVE_LED[m], MOVE_DESC[m], buf);
}

void loop() {
  if (millis() - modeStart < HOLD_MS) return;   // 当前动作还没待够 6 秒

  seqIdx = (seqIdx + 1) % SEQ_LEN;
  setMove(SEQ[seqIdx]);
  printMove(seqIdx);
  modeStart = millis();
}
