// Day 21 实验 2：超声波避障 —— 前方障碍 < 15cm 时后退并转向
//
// 在实验 1 的双电机之上加一个 HC-SR04，形成"感知 → 决策 → 执行"闭环。
//
// 电路（实验 1 全部保留，只多 4 根线）：
//
//   HC-SR04        ESP32-S3
//   ───────        ────────
//   VCC     ──────  3V3（宽电压版；Echo 高电平跟随 VCC，3.3V 可直连 GPIO）
//   GND     ──────  GND（共地）
//   Trig    ──────  GPIO4
//   Echo    ──────  GPIO5
//
// 三个要点：
// ① 决策要有"迟滞"。单一阈值 15cm 会让车卡在临界点上反复前后抽搐：
//    退到 16cm 就前进 → 又进到 14cm → 再退。所以进来用 15cm、出去用 25cm，
//    中间 10cm 的死区让状态稳住。
// ② 避障动作不能是"停在那儿想"。转向必须持续固定时长再重新测距，
//    否则刚转一点就测一次，可能还没转开又看到障碍，原地打转。
// ③ pulseIn() 是阻塞的，最坏卡住 TIMEOUT_US。避障场景只关心 15cm 以内，
//    把超时从 Day 17 的 30ms（400cm）压到 6ms（约 100cm），最坏卡顿减到 1/5。
//    前方超过 100cm 一律按"通畅"处理 —— 反正都是前进。
// ④ 串口输出写成人能读的中文行，不写 JSON 裸数字。避障是一串连续动作，
//    光看 {"cm":12.3} 看不出"现在是第几步、为什么转、往哪边转"。
//    状态切换的那一刻就打印原因，动作和文字才是同一时刻的。

#include <RgbCycle.h>

// ---------- 引脚 ----------
const int AIN1 = 10, AIN2 = 11, PWMA = 12;   // 左轮
const int BIN1 = 15, BIN2 = 16, PWMB = 17;   // 右轮
const int TRIG_PIN = 4;
const int ECHO_PIN = 5;

// ---------- 驱动参数 ----------
const int DUTY_CRUISE = 115;    // 直行（6V × 115/255 = 2.7V 平均）
const int DUTY_TURN   = 120;    // 原地转向
const int PWM_FREQ    = 20000;  // 20kHz，消除 PWM 啸叫
const int PWM_RES     = 8;

// ---------- 测距参数 ----------
const float CM_PER_US = 0.0343 / 2.0;        // 声速 343m/s，来回 ÷2
// 只关心近处，超时压到 6ms ≈ 103cm。超时的结果是"通畅"，与远距结论一致。
const unsigned long TIMEOUT_US = 6000;
const unsigned long CYCLE_MS   = 100;        // 手册要求两次测量间隔 ≥60ms
const unsigned long LOG_MS     = 500;        // 通畅时最多每 500ms 打一行，别刷屏
// 读数下限：低于 2cm 不可信（发射脉冲还没结束就收到回波）
const float MIN_VALID_CM = 2.0;

// ---------- 避障阈值（迟滞） ----------
const float NEAR_CM = 15.0;     // 进入避障：小于它就算挡路
const float FAR_CM  = 25.0;     // 退出避障：大于它才算通畅
// NEAR 与 FAR 之间 10cm 是死区，见 ①

// ---------- 避障动作时长 ----------
const unsigned long BACK_MS  = 500;    // 后退拉开距离
const unsigned long TURN_MS  = 400;    // 转向换方向，必须够转开
// 转向后强制前进这段距离再重新测距，避免"刚转开又看到障碍"的原地打转
const unsigned long COMMIT_MS = 300;

// ---------- 状态机 ----------
// 枚举名避开 Arduino 核心宏 OK / LOW / HIGH
enum State { S_CRUISE, S_BACK, S_TURN, S_COMMIT };

State state = S_CRUISE;
unsigned long stateStart = 0;
// 每次避障换一边转，否则会朝同一个方向越转越深
bool turnLeftNext = true;
// 触发避障的那次读数，留给后退那行的日志用
float blockedCm = 0;
// 连续避障计数：手一直挡着就会一直触发，用它把"第几轮"打出来
int avoidRound = 0;

// ---------- 电机 ----------
void driveMotor(int in1, int in2, int pwmPin, int speed) {
  if (speed > 0) {
    digitalWrite(in1, HIGH); digitalWrite(in2, LOW);
  } else if (speed < 0) {
    digitalWrite(in1, LOW);  digitalWrite(in2, HIGH);
  } else {
    digitalWrite(in1, LOW);  digitalWrite(in2, LOW);
    ledcWrite(pwmPin, 0);       // 高阻 = 滑行
    return;
  }
  ledcWrite(pwmPin, abs(speed));
}

// 刹车 = 短接 + PWM 非 0，两个条件都要满足
void brakeMotor(int in1, int in2, int pwmPin) {
  digitalWrite(in1, HIGH); digitalWrite(in2, HIGH);
  ledcWrite(pwmPin, 255);
}

void drive(int left, int right) {
  driveMotor(AIN1, AIN2, PWMA, left);
  driveMotor(BIN1, BIN2, PWMB, right);
}

void brakeAll() {
  brakeMotor(AIN1, AIN2, PWMA);
  brakeMotor(BIN1, BIN2, PWMB);
}

void showState(State s) {
  switch (s) {
    case S_CRUISE: RgbCycle::setColor(0, 255, 0);   break;  // 绿：畅通直行
    case S_BACK:   RgbCycle::setColor(255, 160, 0); break;  // 橙：后退
    case S_TURN:   RgbCycle::setColor(255, 0, 0);   break;  // 红：转向
    case S_COMMIT: RgbCycle::setColor(255, 255, 0); break;  // 黄：强制前进
  }
}

// ---------- 测距 ----------
// 返回厘米；-1 表示超时（等价于"前方很远"）
float measureCm() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);          // 手册要求 ≥10μs
  digitalWrite(TRIG_PIN, LOW);

  unsigned long us = pulseIn(ECHO_PIN, HIGH, TIMEOUT_US);
  if (us == 0) return -1.0;       // pulseIn 超时返回 0，不是 -1
  return us * CM_PER_US;
}

void enterState(State s) {
  state = s;
  stateStart = millis();
  showState(s);

  switch (s) {
    case S_CRUISE:
      drive( DUTY_CRUISE,  DUTY_CRUISE);
      break;
    case S_BACK:
      drive(-DUTY_CRUISE, -DUTY_CRUISE);
      // 说清"为什么退"：把触发的那次读数直接打出来
      // 时长用毫秒整数打印。写成 %.0f 秒会把 500ms 四舍五入成 "0 秒"
      Serial.printf("第%d轮｜前方 %.1fcm < %.0fcm，挡住了 → 后退 %dms\n",
                    avoidRound, blockedCm, NEAR_CM, BACK_MS);
      break;
    case S_TURN:
      // 原地转向：两轮等速反向
      if (turnLeftNext) drive(-DUTY_TURN,  DUTY_TURN);
      else              drive( DUTY_TURN, -DUTY_TURN);
      // 说清"往哪转"和"为什么这次是这边"：左右交替，不让车朝一个方向越转越深
      Serial.printf("第%d轮｜原地%s转 %dms（左轮%s / 右轮%s，左右交替，下一轮换另一边）\n",
                    avoidRound,
                    turnLeftNext ? "左" : "右",
                    TURN_MS,
                    turnLeftNext ? "后退" : "前进",
                    turnLeftNext ? "前进" : "后退");
      break;
    case S_COMMIT:
      drive( DUTY_CRUISE,  DUTY_CRUISE);
      Serial.printf("第%d轮｜转向已转开，强制前进 %dms 再重新测距（避免刚转开又看到障碍）\n",
                    avoidRound, COMMIT_MS);
      break;
  }
}

void setup() {
  Serial.begin(115200);
  delay(200);

  RgbCycle::begin();

  pinMode(AIN1, OUTPUT); pinMode(AIN2, OUTPUT);
  pinMode(BIN1, OUTPUT); pinMode(BIN2, OUTPUT);
  ledcAttach(PWMA, PWM_FREQ, PWM_RES);
  ledcAttach(PWMB, PWM_FREQ, PWM_RES);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  digitalWrite(TRIG_PIN, LOW);

  Serial.println("Day21 实验2：超声波避障");
  Serial.printf("阈值 进<%.0fcm 出>%.0fcm（迟滞 %.0fcm）\n",
                NEAR_CM, FAR_CM, FAR_CM - NEAR_CM);
  Serial.println("通畅时每 0.5 秒报一次距离；挡住就打印避障的每一步和原因");

  brakeAll();
  enterState(S_CRUISE);
}

void loop() {
  unsigned long now = millis();
  unsigned long inState = now - stateStart;

  // 除了 CRUISE，其余状态都是"定时动作"，不测距、不打断
  switch (state) {
    case S_BACK:
      if (inState < BACK_MS) return;
      enterState(S_TURN);
      return;
    case S_TURN:
      if (inState < TURN_MS) return;
      turnLeftNext = !turnLeftNext;    // 下次换另一边
      enterState(S_COMMIT);
      return;
    case S_COMMIT:
      if (inState < COMMIT_MS) return;
      enterState(S_CRUISE);
      return;
    case S_CRUISE:
      break;    // 下面才测距
  }

  // CRUISE 状态：按 CYCLE_MS 节流测距
  static unsigned long lastMeasure = 0;
  if (now - lastMeasure < CYCLE_MS) return;
  lastMeasure = now;

  float cm = measureCm();

  // 读数落在死区里按"上次结论"处理，靠迟滞自然稳住：
  //   cm < NEAR → 进入避障；cm > FAR → 继续直行；中间不动
  if (cm >= MIN_VALID_CM && cm < NEAR_CM) {
    blockedCm = cm;
    avoidRound++;
    enterState(S_BACK);
    return;
  }

  // 通畅：按 LOG_MS 报一次距离，不刷屏
  static unsigned long lastLog = 0;
  if (now - lastLog < LOG_MS) return;
  lastLog = now;

  if (cm < MIN_VALID_CM) {
    // cm = -1 是超时（前方超过约 100cm），低于 2cm 是不可信读数，两者都当前方通畅
    Serial.printf("前方通畅：%s，继续直行\n",
                  cm < 0 ? "超出量程（>100cm）" : "读数无效");
  } else {
    Serial.printf("前方 %.1fcm，通畅 → 直行前进\n", cm);
  }
}
