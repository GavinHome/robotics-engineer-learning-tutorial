// Day 19 实验 2：PWM 调速（开环）
//
// 电路与实验 1 完全一致，一根线都不用加。
//
// 四个要点：
// ① PWM 调速调的是"平均电压"。duty = 180/255 ≈ 70.6%，电机两端平均电压
//    ≈ 3V × 0.706 ≈ 2.12V，转速大致跟着这个平均电压走。
// ② 频率不能随便选：几百 Hz 能听见啸叫、力矩脉动明显；太高 MOSFET 开关
//   损耗大、发热。5kHz 是安静的折中，要彻底免啸叫得上 >20kHz。
// ③ 开环调速的固有局限：同样 duty 下，电池越用越没劲、或者负载变重，
//    转速都会掉，代码并不知道。要稳必须加编码器做闭环 —— 那是后面的事。
// ④ 死区：duty 太小时电机根本不动（得先克服静摩擦）。串口一路打印 duty，
//    肉眼看电机从第几个值开始转，那就是这块板子的启动阈值。
//    死区由平均电压决定：电池 3V，同样的静摩擦需要的 duty 比 6V 时大约翻倍。

#include <RgbCycle.h>

const int AIN1 = 10;
const int AIN2 = 11;
const int PWMA = 12;

// START_DUTY 取实测起转点 220：3V 下从 120 起爬会有 1.1s 死区
// （平均电压 1.41V~2.12V < 启动阈值），电机只嗡嗡响不转，看着像坏了。
// 想看完整加速曲线就改回 120，代价是每循环多 1.1s 的死区段。
const int START_DUTY = 220;
const int STEP = 5;          // 每步加/减 5
const int STEP_MS = 40;      // 每步停留 40ms → 单向约 1.1s
const int HOLD_MS = 1000;    // 到顶后匀速 1s
const float VM_VOLT = 3.0;   // 电机电源电压（2 节 5 号），按实际电池改

enum Phase { P_UP, P_HOLD, P_DOWN };
const char* PHASE_NAME[] = { "UP", "HOLD", "DOWN" };

int duty = START_DUTY;
int printedDuty = -1;
Phase phase = P_UP;
unsigned long stepStart = 0;
unsigned long holdStart = 0;

void setup() {
  Serial.begin(115200);
  delay(200);

  RgbCycle::begin();
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  ledcAttach(PWMA, 5000, 8);   // 5kHz / 8 位 → duty 0~255

  digitalWrite(AIN1, HIGH);    // 固定正转方向
  digitalWrite(AIN2, LOW);
  ledcWrite(PWMA, duty);

  stepStart = millis();
  Serial.println("PWM ramp: AIN1=GPIO10 AIN2=GPIO11 PWMA=GPIO12");
}

void loop() {
  if (millis() - stepStart < STEP_MS) return;
  stepStart = millis();

  switch (phase) {
    case P_UP:
      duty += STEP;
      if (duty > 255) { duty = 255; phase = P_HOLD; holdStart = millis(); }
      break;
    case P_HOLD:
      if (millis() - holdStart >= HOLD_MS) phase = P_DOWN;
      break;
    case P_DOWN:
      duty -= STEP;
      if (duty < START_DUTY) duty = START_DUTY;   // 回到起点再爬，循环演示
      if (duty == START_DUTY) phase = P_UP;
      break;
  }

  ledcWrite(PWMA, duty);
  RgbCycle::setColor(duty, duty / 4, 0);   // 灯随 duty 变亮，便于对照串口

  if (duty != printedDuty) {               // 只在 duty 变化时打印，避免重复行
    printedDuty = duty;
    Serial.printf("{\"phase\":\"%s\",\"duty\":%d,\"pct\":%.1f,\"vavg\":%.2f}\n",
                  PHASE_NAME[phase], duty, duty * 100.0 / 255.0,
                  VM_VOLT * duty / 255.0);
  }
}
