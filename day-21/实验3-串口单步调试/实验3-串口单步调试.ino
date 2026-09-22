// Day 21 实验 3：串口单步调试
//
// 一个动作一个动作地测，输数字回车即可，不用等演示序列轮完一圈。
//
//   1 前进（绿灯）   2 后退（橙灯）   3 原地左转（青灯）  4 原地右转（紫灯）
//   5 左弧线（黄灯） 6 右弧线（黄灯） 7 刹车（红灯）      8 滑行（蓝灯）
//   9 右轮连测（前进 3 秒 → 后退 3 秒，左轮全程断电）
//   s 占空比扫描：两轮同时从 80 逐级升到 255，每档 1.5 秒，找起转点
//   v 切换 6V / 3V 占空比标定（默认 6V；换回 3V 盒时按一下，不用重新烧）
//
//   单轮四态（另一路全程断电，起步冲击档跑）：
//     lf 左轮前进  lb 左轮后退
//     rf 右轮前进  rb 右轮后退
//     lz 左轮刹车  lc 左轮滑行     —— 都先转 2 秒再执行，否则静止轮看不出区别
//     rz 右轮刹车  rc 右轮滑行
//   0 停下并断电（灭灯）
//
// 每个动作跑 6 秒，然后自动刹车停住，等你输下一条。
// 每条命令都会打印 6 根 GPIO 的实测电平，方便拿万用表逐根对。
//
// 标定依赖供电：默认 6V；换回 3V 盒时输 v 切过去，不用重新烧。
//
// 马达额定 3V，靠占空比把平均电压压到 3V 以内：
//   平均电压 = 电源电压 × 占空比
//   3V 供电 × 230/255 = 2.7V
//   6V 供电 × 115/255 = 2.7V   ← 平均电压相同，马达受力/发热/转速都一样
// PWM 峰值虽然是 6V，但电机是线圈，电流被电感平滑，实际跟平均值走。
//
// 电路与实验 1 完全相同：
//   左轮 AIN1=GPIO10 AIN2=GPIO11 PWMA=GPIO12   AO1=左轮+ AO2=左轮−
//   右轮 BIN1=GPIO15 BIN2=GPIO16 PWMB=GPIO17   BO1=右轮+ BO2=右轮−

#include <RgbCycle.h>

const int AIN1 = 10, AIN2 = 11, PWMA = 12;   // 左轮
const int BIN1 = 15, BIN2 = 16, PWMB = 17;   // 右轮

// ---------- 供电与占空比 ----------
// 两套值，运行时输 v 切换，不用重新烧。
//
// 3V（2 节 5 号）：电池内阻相对电压太大，两个电机同时启动会把电压拉垮，
//   结果是嗡嗡响不转，或者一个先转、另一个后转（先转的电流降了才轮到它）。
// 6V（4 节 5 号）：马达额定仍是 3V，所以占空比减半，平均电压不变；
//   但 PWM 导通瞬间的峰值电流大得多，顶得开静摩擦，两个轮子才转得起来。
//   3V 盒内阻太大，两个电机同时启动会把电压拉垮 —— 嗡嗡响不转，或一先一后。
//
// 默认 6V（现在用的就是它）。若换回 3V 盒，输 v 切过去。
bool supply6V = true;

int dutyCruise() { return supply6V ? 115 : 230; }
int dutyTurn()   { return supply6V ? 120 : 240; }
int dutySlow()   { return supply6V ?  75 : 150; }
int dutyKick()   { return supply6V ? 150 : 255; }   // 起步冲击，短暂高于额定只为顶开静摩擦
float supplyV()  { return supply6V ? 6.0 : 3.0;  }
// 20kHz 而不是 5kHz：一是超出人耳范围，消除 PWM 啸叫（之前那阵"嗡嗡响"
// 有一部分就是它，容易和"电机堵转转不动"的嗡嗡声混淆）；二是周期变短，
// 电机线圈的电流纹波更小，平均电压更接近理论值。
const int PWM_FREQ    = 20000;
const int PWM_RES     = 8;

const unsigned long RUN_MS    = 6000;   // 动作持续
const unsigned long BRAKE_MS  = 1000;   // 收尾刹车
const unsigned long KICK_MS   = 200;    // 起步冲击时长
const unsigned long SWEEP_MS  = 1500;   // 扫描每档时长

// ---------- 电机 ----------
void driveMotor(int in1, int in2, int pwmPin, int speed) {
  if (speed > 0) {
    digitalWrite(in1, HIGH); digitalWrite(in2, LOW);
  } else if (speed < 0) {
    digitalWrite(in1, LOW);  digitalWrite(in2, HIGH);
  } else {
    digitalWrite(in1, LOW);  digitalWrite(in2, LOW);
    ledcWrite(pwmPin, 0);
    return;
  }
  ledcWrite(pwmPin, abs(speed));
}

// 起步冲击：两个轮子同时先全压 KICK_MS 把静摩擦顶开，再一起降到目标占空比。
// 电机启动瞬间是堵转状态，电流最大；慢慢升会让两轮"抢电流"，
// 看起来像"先左后右""嗡嗡响不转"。
void kickStart(int left, int right) {
  int k = dutyKick();
  if (left)  driveMotor(AIN1, AIN2, PWMA, left  > 0 ?  k : -k);
  if (right) driveMotor(BIN1, BIN2, PWMB, right > 0 ?  k : -k);
  delay(KICK_MS);
  driveMotor(AIN1, AIN2, PWMA, left);
  driveMotor(BIN1, BIN2, PWMB, right);
}

// 刹车 = 两脚短接 + PWM 非 0。两个条件缺一个就退化成滑行。
void brakeMotor(int in1, int in2, int pwmPin) {
  digitalWrite(in1, HIGH); digitalWrite(in2, HIGH);
  ledcWrite(pwmPin, 255);
}

void stopAll() {
  driveMotor(AIN1, AIN2, PWMA, 0);
  driveMotor(BIN1, BIN2, PWMB, 0);
  RgbCycle::setColor(0, 0, 0);
}

// ---------- 打印 6 根 GPIO 的实测电平 ----------
// PWM 打印"写进去的值"而不是 ledcRead() 的回读值：
// Core 3.x 的 ledcRead() 在部分版本恒返回 0，会误导判断。
void printPins(const char* tag, int dutyA, int dutyB) {
  // 平均电压 = 电源电压 × 占空比，用万用表直流档量电机两端应该就是这个数
  Serial.printf("  %s → AIN1=%d AIN2=%d | BIN1=%d BIN2=%d | PWMA=%d%%(%.1fV) PWMB=%d%%(%.1fV)\n",
                tag,
                digitalRead(AIN1), digitalRead(AIN2),
                digitalRead(BIN1), digitalRead(BIN2),
                dutyA * 100 / 255, supplyV() * dutyA / 255.0,
                dutyB * 100 / 255, supplyV() * dutyB / 255.0);
}

void soloTest(int in1, int in2, int pwmPin, int o1, int o2, int opwm, const char* who);
void dutySweep();
void oneWheel(bool isLeft, char mode);

// 执行一个动作：跑 RUN_MS，然后刹车 BRAKE_MS，最后断电
void runMove(const char* name, const char* led, int left, int right) {
  Serial.printf("%s灯 | %s | 左轮=%s%d 右轮=%s%d | 跑 6 秒\n",
                led, name,
                left  < 0 ? "后退" : "前进", abs(left),
                right < 0 ? "后退" : "前进", abs(right));

  kickStart(left, right);
  printPins("输出", abs(left), abs(right));

  delay(RUN_MS);

  // 收尾刹车，灯转红
  RgbCycle::setColor(255, 0, 0);
  brakeMotor(AIN1, AIN2, PWMA);
  brakeMotor(BIN1, BIN2, PWMB);
  Serial.println("红灯 | 刹车 1 秒");
  printPins("刹车", 255, 255);

  delay(BRAKE_MS);
  stopAll();
  Serial.println("灭灯 | 已断电，等下一条命令（1-8，0=立刻断电）\n");
}

void setup() {
  Serial.begin(115200);
  delay(200);

  RgbCycle::begin();

  pinMode(AIN1, OUTPUT); pinMode(AIN2, OUTPUT);
  pinMode(BIN1, OUTPUT); pinMode(BIN2, OUTPUT);
  ledcAttach(PWMA, PWM_FREQ, PWM_RES);
  ledcAttach(PWMB, PWM_FREQ, PWM_RES);

  stopAll();

  Serial.println("\n===== Day21 串口单步调试 =====");
  Serial.println("1 前进(绿)   2 后退(橙)   3 原地左转(青)  4 原地右转(紫)");
  Serial.println("5 左弧线(黄) 6 右弧线(黄) 7 刹车(红)      8 滑行(蓝)");
  Serial.println("9 右轮连测（前进3秒→后退3秒，左轮断电）");
  Serial.println("s 占空比扫描：两轮同时 80→255，每档 1.5 秒，找起转点");
  Serial.println("单轮四态（另一路全程断电）：");
  Serial.println("  lf 左轮前进  lb 左轮后退  lz 左轮刹车  lc 左轮滑行");
  Serial.println("  rf 右轮前进  rb 右轮后退  rz 右轮刹车  rc 右轮滑行");
  Serial.printf("v 切换 6V / 3V 标定（当前 %.1fV）\n", supplyV());
  Serial.println("0 立刻断电");
  Serial.printf("左轮 AIN1=%d AIN2=%d PWMA=%d ｜ 右轮 BIN1=%d BIN2=%d PWMB=%d\n",
                AIN1, AIN2, PWMA, BIN1, BIN2, PWMB);
  Serial.println("输命令后回车");
  Serial.printf("⚠ 当前标定 %.1fV，若实际供电不是 %.1fV 请先输 v 切换 —— 标定错了会把马达超压运行\n\n",
                supplyV(), supplyV());
}

void loop() {
  if (!Serial.available()) return;

  char c = Serial.read();

  // l/r 开头的双字母命令：第二个字母可能还在路上，等最多 500ms
  char m = 0;
  if (c == 'l' || c == 'r') {
    unsigned long t0 = millis();
    while (!Serial.available() && millis() - t0 < 500) delay(10);
    m = Serial.available() ? Serial.read() : 0;
    if (m != 'f' && m != 'b' && m != 'z' && m != 'c') {
      Serial.println("第二个字母只认 f/b/z/c（前进/后退/刹车/滑行）\n");
      return;
    }
  }

  while (Serial.available()) Serial.read();         // 清掉尾巴

  // 两个字母合成一个整数，这样单字母和双字母命令能写在同一个 switch 里
  int cmd = m ? c * 256 + m : c;

  switch (cmd) {
    case 'l' * 256 + 'f': oneWheel(true,  'f'); break;   // 左轮前进
    case 'l' * 256 + 'b': oneWheel(true,  'b'); break;   // 左轮后退
    case 'l' * 256 + 'z': oneWheel(true,  'z'); break;   // 左轮刹车
    case 'l' * 256 + 'c': oneWheel(true,  'c'); break;   // 左轮滑行
    case 'r' * 256 + 'f': oneWheel(false, 'f'); break;   // 右轮前进
    case 'r' * 256 + 'b': oneWheel(false, 'b'); break;   // 右轮后退
    case 'r' * 256 + 'z': oneWheel(false, 'z'); break;   // 右轮刹车
    case 'r' * 256 + 'c': oneWheel(false, 'c'); break;   // 右轮滑行

    case 'v': // 切换 6V / 3V 占空比标定（默认 6V）
      supply6V = !supply6V;
      Serial.printf("供电标定 = %.1fV ｜ 巡航=%d 转向=%d 慢速=%d 起步冲击=%d\n\n",
                    supplyV(), dutyCruise(), dutyTurn(), dutySlow(), dutyKick());
      break;

    case '1': RgbCycle::setColor(0, 255, 0);
              runMove("前进  两轮都向前", "绿",  dutyCruise(),  dutyCruise()); break;
    case '2': RgbCycle::setColor(255, 160, 0);
              runMove("后退  两轮都向后", "橙", -dutyCruise(), -dutyCruise()); break;
    case '3': RgbCycle::setColor(0, 255, 255);
              runMove("原地左转 左退右进", "青", -dutyTurn(),    dutyTurn());   break;
    case '4': RgbCycle::setColor(255, 0, 255);
              runMove("原地右转 左进右退", "紫",  dutyTurn(),   -dutyTurn());   break;
    case '5': RgbCycle::setColor(255, 255, 0);
              runMove("左弧线  左慢右快", "黄",  dutySlow(),    dutyCruise());  break;
    case '6': RgbCycle::setColor(255, 255, 0);
              runMove("右弧线  左快右慢", "黄",  dutyCruise(),  dutySlow());    break;

    case '7': // 纯刹车：短接 3 秒，看两个轮子是不是都立刻停住
      Serial.println("红灯 | 刹车 3 秒 —— 两个轮子都应立刻停住");
      RgbCycle::setColor(255, 0, 0);
      brakeMotor(AIN1, AIN2, PWMA);
      brakeMotor(BIN1, BIN2, PWMB);
      printPins("刹车", 255, 255);
      delay(3000);
      stopAll();
      Serial.println("灭灯 | 已断电\n");
      break;

    case '8': // 滑行：断电，靠惯性慢停
      Serial.println("蓝灯 | 滑行 3 秒 —— 两个轮子都应断电自由转");
      RgbCycle::setColor(0, 0, 255);
      stopAll();
      RgbCycle::setColor(0, 0, 255);
      printPins("滑行", 0, 0);
      delay(3000);
      stopAll();
      Serial.println("灭灯 | 已断电\n");
      break;

    case '9': // 右轮连测：左轮全程断电，只看右轮能不能正反转
      soloTest(BIN1, BIN2, PWMB, AIN1, AIN2, PWMA, "右轮"); break;

    case 's': // 占空比扫描：两轮同时升档，找出到底要多大 duty 才转
      dutySweep(); break;

    case '0': stopAll();
      Serial.println("灭灯 | 已断电\n"); break;
  }
}

// 只跑一路：另一路全程断电，看这一路能不能正反转
void soloTest(int in1, int in2, int pwmPin, int o1, int o2, int opwm, const char* who) {
  Serial.printf("单测%s —— 另一路全程断电\n", who);
  driveMotor(o1, o2, opwm, 0);

  RgbCycle::setColor(0, 255, 0);
  driveMotor(in1, in2, pwmPin, dutyKick());
  Serial.printf("  %s前进 3 秒（绿）\n", who);
  delay(3000);

  RgbCycle::setColor(255, 160, 0);
  driveMotor(in1, in2, pwmPin, -dutyKick());
  Serial.printf("  %s后退 3 秒（橙）\n", who);
  delay(3000);

  stopAll();
  Serial.println("灭灯 | 已断电\n");
}

// 单轮四态：f 前进 / b 后退 / z 刹车 / c 滑行，另一路全程断电。
// 刹车和滑行要先转 2 秒再执行，否则静止的轮子根本看不出两者区别。
void oneWheel(bool isLeft, char mode) {
  int in1 = isLeft ? AIN1 : BIN1;
  int in2 = isLeft ? AIN2 : BIN2;
  int pwm = isLeft ? PWMA : PWMB;
  int o1  = isLeft ? BIN1 : AIN1;
  int o2  = isLeft ? BIN2 : AIN2;
  int opwm= isLeft ? PWMB : PWMA;
  const char* who = isLeft ? "左轮" : "右轮";

  const char* act = (mode == 'f') ? "前进" : (mode == 'b') ? "后退"
                  : (mode == 'z') ? "刹车" : "滑行";

  driveMotor(o1, o2, opwm, 0);          // 另一路断电

  int d = dutyKick();

  if (mode == 'z' || mode == 'c') {
    Serial.printf("%s | %s先前进 2 秒\n", who, who);
    RgbCycle::setColor(0, 255, 0);
    driveMotor(in1, in2, pwm, d);
    delay(2000);
  }

  if (mode == 'f') {
    RgbCycle::setColor(0, 255, 0);
    driveMotor(in1, in2, pwm, d);
    Serial.printf("%s 前进 3 秒（绿）—— 应向前转\n", who);
    printPins(who, isLeft ? d : 0, isLeft ? 0 : d);
    delay(3000);
  } else if (mode == 'b') {
    RgbCycle::setColor(255, 160, 0);
    driveMotor(in1, in2, pwm, -d);
    Serial.printf("%s 后退 3 秒（橙）—— 应向后转\n", who);
    printPins(who, isLeft ? d : 0, isLeft ? 0 : d);
    delay(3000);
  } else if (mode == 'z') {
    RgbCycle::setColor(255, 0, 0);
    brakeMotor(in1, in2, pwm);
    Serial.printf("%s 刹车（红）—— 应立刻停住\n", who);
    printPins(who, isLeft ? d : 0, isLeft ? 0 : d);
    delay(3000);
  } else {
    RgbCycle::setColor(0, 0, 255);
    driveMotor(in1, in2, pwm, 0);
    Serial.printf("%s 滑行（蓝）—— 应靠惯性慢停\n", who);
    printPins(who, 0, 0);
    delay(3000);
  }

  stopAll();
  Serial.printf("灭灯 | %s断电\n\n", who);
}

// 占空比扫描：两轮同时从 80 逐级升到 255，每档 1.5 秒。
// 看轮子从哪一档开始转 —— 那一档就是这辆车在当前供电下的起转点。
void dutySweep() {
  Serial.printf("占空比扫描（%.1fV）：两轮同时升档，每档 1.5 秒，看从哪档开始转\n", supplyV());
  for (int d = 80; d <= 255; d += 25) {
    Serial.printf("  duty=%d (%d%%)\n", d, d * 100 / 255);
    driveMotor(AIN1, AIN2, PWMA, d);
    driveMotor(BIN1, BIN2, PWMB, d);
    delay(SWEEP_MS);
  }
  stopAll();
  Serial.println("灭灯 | 扫描结束\n");
}
