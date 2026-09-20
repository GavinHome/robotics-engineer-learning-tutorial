// Day 19 实验 1：TB6612FNG 正反转 / 刹车 / 滑行
//
// 电路（6 根公对母杜邦线，电机直接拧在 AO1/AO2 端子）：
//
//   TB6612FNG 板      ESP32-S3 / 电源
//   ────────────      ─────────────
//   VCC       ──────  3V3（逻辑电平）
//   GND       ──────  ESP32 GND 且 电池负极（共地！）
//   STBY      ──────  3V3（常高，省一个 GPIO）
//   AIN1      ──────  GPIO10
//   AIN2      ──────  GPIO11
//   PWMA      ──────  GPIO12
//   VM        ──────  电池盒正极（只装 2 节 5 号 ≈ 3V，TT 马达额定电压）
//   AO1/AO2   ──────  电机两端（不分极性，转向反了就拔下来对调）
//
// 三个要点：
// ① 电机不能由 GPIO 直驱。TT 马达启动电流几百 mA，ESP32-S3 单个 GPIO
//   只能出 40mA；而且电机是感性负载，断电瞬间的反电动势会把 GPIO 顶穿。
//   H 桥就是替 GPIO 干"大电流开关"这层活，GPIO 只负责发逻辑电平。
// ② TB6612FNG 真值表（A 通道）：
//      IN1  IN2  PWM   输出              模式
//       1    0    1    OUT1=H  OUT2=L   正转
//       0    1    1    OUT1=L  OUT2=H   反转
//       1    1    1    OUT1=L  OUT2=L   刹车（两端短接到一起）
//       0    0    x    双高阻            滑行（自由转动）
//       x    x    0    双高阻            滑行
//   IN1=IN2=1 才是刹车，IN1=IN2=0 是滑行。中文里常混着说，硬件行为完全不同：
//   刹车时电机迅速停住，滑行时靠惯性再转一会儿。
// ③ STBY 在这里接 3.3V 常高，代码完全不碰它 —— 芯片始终使能。想整块
//   驱动断电待机，才需要把 STBY 拉低。

#include <RgbCycle.h>

const int AIN1 = 10;
const int AIN2 = 11;
const int PWMA = 12;

// 枚举名避开 Arduino 核心宏 HIGH / LOW / OK
enum MotorMode { M_FWD, M_BRAKE, M_REV, M_COAST };
const char* MODE_NAME[] = { "FWD", "BRAKE", "REV", "COAST" };

// 演示序列：正转 → 刹车 → 反转 → 刹车 → 滑行，一轮看全四种行为
const MotorMode SEQ[] = { M_FWD, M_BRAKE, M_REV, M_BRAKE, M_COAST };
const int SEQ_LEN = 5;
const unsigned long HOLD_MS = 2000;   // 每种模式停留
// duty 够不够要看 VM 不看百分比：实测 3V × 180/255 ≈ 2.12V 掉在死区里，
// 电机只嗡嗡响不转（3.0V 直流直供却正常转）；220 → 2.59V 才起转。
const int DRIVE_DUTY = 220;           // 实测起转点，在 180 与 220 之间

int seqIdx = 0;
unsigned long modeStart = 0;

void showMode(MotorMode m) {
  switch (m) {
    case M_FWD:   RgbCycle::setColor(0, 255, 0);   break;  // 绿：正转
    case M_BRAKE: RgbCycle::setColor(255, 0, 0);   break;  // 红：刹车
    case M_REV:   RgbCycle::setColor(255, 160, 0); break;  // 橙：反转
    case M_COAST: RgbCycle::setColor(0, 0, 255);   break;  // 蓝：滑行
  }
}

void setMotor(MotorMode m, int duty) {
  switch (m) {
    case M_FWD:
      digitalWrite(AIN1, HIGH);
      digitalWrite(AIN2, LOW);
      ledcWrite(PWMA, duty);
      break;
    case M_REV:
      digitalWrite(AIN1, LOW);
      digitalWrite(AIN2, HIGH);
      ledcWrite(PWMA, duty);
      break;
    case M_BRAKE:
      // 两端同时拉低 = 短接制动。PWM 必须非 0，否则又退回高阻滑行。
      digitalWrite(AIN1, HIGH);
      digitalWrite(AIN2, HIGH);
      ledcWrite(PWMA, 255);
      break;
    case M_COAST:
      digitalWrite(AIN1, LOW);
      digitalWrite(AIN2, LOW);
      ledcWrite(PWMA, 0);
      break;
  }
}

void setup() {
  Serial.begin(115200);
  delay(200);

  RgbCycle::begin();
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  ledcAttach(PWMA, 5000, 8);   // 5kHz / 8 位 → duty 0~255

  setMotor(M_COAST, 0);        // 上电先滑行，别抢跑
  showMode(M_COAST);
  modeStart = millis();

  Serial.println("TB6612FNG demo: AIN1=GPIO10 AIN2=GPIO11 PWMA=GPIO12 STBY=3V3(常高)");
}

void loop() {
  if (millis() - modeStart < HOLD_MS) return;   // 非阻塞节流
  modeStart = millis();

  MotorMode m = SEQ[seqIdx];
  setMotor(m, DRIVE_DUTY);
  showMode(m);

  int duty = (m == M_BRAKE) ? 255 : ((m == M_COAST) ? 0 : DRIVE_DUTY);
  Serial.printf("{\"mode\":\"%s\",\"duty\":%d,\"ain1\":%d,\"ain2\":%d}\n",
                MODE_NAME[m], duty, digitalRead(AIN1), digitalRead(AIN2));

  seqIdx = (seqIdx + 1) % SEQ_LEN;
}
