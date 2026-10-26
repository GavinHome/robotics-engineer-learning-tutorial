// Day 39：MPU-6050 裸读 + 校准
//
// 自平衡车要 MPU-6050 干的只有一件事：给角度环一个**不漂移的俯仰角**。
// 今天给这件事打地基——不装库、照数据手册把原始量读出来，再把零偏校掉。
// 明天（Day 40）在同样的原始量上做互补滤波，最后才是平衡车的角度环。
//
// 今天要立住的四件事：
//   ① 裸读 I2C：不引入 Adafruit_MPU6050，用 Wire 直接读寄存器。
//      里程碑"不看教程、按数据手册连接并读取 I2C 传感器"说的是这件事。
//      三个必须查的寄存器：WHO_AM_I(0x75)=0x68 验地址、
//      PWR_MGMT_1(0x6B) 写 0 唤醒、ACCEL_CONFIG(0x1C) 选 ±2g。
//   ② 换算：16384 LSB/g（±2g）、131 LSB/(°/s)（±250°/s）。
//      光会读 16 位原值不算懂，能说出"这一位等于多少 g"才算。
//   ③ 零偏：静止采 2000 次求均值。不校的话平放不读 0，
//      自平衡车会**永久偏向一侧**——静差就是这么来的。
//   ④ 标度：竖放 / 倒放读数应分别是 ±90°。这个验的是灵敏度，
//      不是零位——两个是不同的错，分开验。
//
// ⚠️ 为什么今天不做"俯仰角"：atan2 那一步是 Day 40 的活。
//    今天只保证 raw 和 g 是对的，一层一层往上加，错了好定位。
//
// 接线（沿用 Day 18 那套，一根不动）：
//
//   MPU-6050         ESP32-S3
//   ────────         ────────
//   VCC      ──────  3V3（模块自带稳压，别把 5V 接到 SDA/SCL）
//   GND      ──────  GND（必须共地，否则总线永远读不到 ACK）
//   SDA      ──────  GPIO41
//   SCL      ──────  GPIO42
//   INT      ──────  悬空（今天不用中断，轮询就够）
//   AD0      ──────  GND（决定 I2C 地址 = 0x68；接 VCC 是 0x69）
//
//   ⚠️ 接线顺序：先 GND，最后 VCC。拆线反过来。
//   ⚠️ 别用 GPIO 0/3/45/46 做 I2C——那是 strapping 引脚，
//      上电瞬间要采样电平决定启动模式，外接模块会干扰。
//   ⚠️ 别让电机线贴 I2C 线。自平衡车上 MPU 是最大受害者，
//      底盘会当共模天线把 PWM 噪声灌进来——今天不装车，先养好习惯。
//
// 串口命令（115200）：
//   a = 加速度计原始量 + g（每秒一帧，默认就是这个）
//   g = 陀螺仪原始量 + °/s
//   t = 温度
//   r = 六轴一起打（每帧 6 个量，给 telemetry_plot.py 用）
//   k = 开始零偏校准（按了之后手别碰板子，采 2000 次，约 3 秒）
//   w = 显示当前零偏 / 标度常量
//   1 = 记「平放」姿态（采 100 帧求均值，存 flash）
//   2 = 记「竖放」姿态
//   3 = 记「倒放」姿态
//   v / p = 汇总：零偏 + 两张 markdown 表，整段复制粘贴用
//   0 = 停打（静音，只看命令回显）
//  命令大小写都认。
//
// 彩灯：蓝=正在初始化 / 绿=正常打印 / 黄=正在采零偏 / 红=WHO_AM_I 对不上
//
// 持久化：零偏 + 标度存 flash（NVS，命名空间 "day39"）。
// Day 40 的固件会读同一份——所以校准只做一次，不是每天重做。

#include <RgbCycle.h>
#include <Wire.h>
#include <Preferences.h>

// ---------- I2C ----------
const int SDA_PIN = 41;
const int SCL_PIN = 42;
const uint8_t MPU = 0x68;          // AD0 接 GND 时的地址；接 VCC 是 0x69

// ---------- 寄存器地址（查数据手册得来，不是抄来的）----------
const uint8_t REG_WHO_AM_I    = 0x75;   // 应回 0x68
const uint8_t REG_PWR_MGMT_1  = 0x6B;   // 写 0 唤醒；默认在睡眠
const uint8_t REG_ACCEL_CONFIG = 0x1C;  // bit4:3 = 量程：0=±2g, 1=±4g, 2=±8g, 3=±16g
const uint8_t REG_GYRO_CONFIG  = 0x1B;  // bit4:3 = 量程：0=±250, 1=±500, 2=±1000, 3=±2000 °/s
const uint8_t REG_ACCEL_XOUT_H = 0x3B;  // 加速度数据从这里连续 6 字节（X/Y/Z 高低）
const uint8_t REG_GYRO_XOUT_H  = 0x43;  // 陀螺仪同理
const uint8_t REG_TEMP_OUT_H   = 0x41;  // 温度 2 字节

// ---------- 灵敏度（选定量程下每一位代表的物理量）----------
// 这两个数决定了"1 个 LSB 有多大"，也决定了标度验算的期望值。
// 数据手册给的是"满量程 / 65536"，别用 32768 除——那是 ±1g 的误算法。
const float ACCEL_LSB_PER_G = 16384.0f;   // ±2g  →  1 LSB = 1/16384 g
const float GYRO_LSB_PER_DPS = 131.0f;    // ±250°/s → 1 LSB = 1/131 °/s

// ---------- 零偏（按 k 校准得到，存 NVS）----------
// 单位就是"原始 LSB"，不是 g/°/s。因为板子不动时读数本身就在零附近抖，
// 减掉的是均值，剩下的才是信号。
float biasAx = 0, biasAy = 0, biasAz = 0;
float biasGx = 0, biasGy = 0, biasGz = 0;
bool  calibrated = false;    // false 时只打原始值，不应用零偏——看得见校准前后的差别

// ---------- 采样 ----------
const unsigned long PERIOD_MS = 100;    // 10Hz，和 Day 40 的控制周期同频
const int CAL_SAMPLES = 2000;           // 静止 2000 次 ≈ 3.3 秒，均值才稳

// 一次 burst 读 6 个轴，比读 6 次快一个数量级——
// 加速度和陀螺仪要在**同一时刻**采样，否则做融合时会有一个时间错位。
// 这个习惯今天就要立：一次传输读完 14 字节（加速度 6 + 温度 2 + 陀螺仪 6）。
struct Sample {
  int16_t ax, ay, az;
  int16_t gx, gy, gz;
  int16_t temp;
};

// ---------- 打印模式 ----------
enum Mode { MODE_ACCEL, MODE_GYRO, MODE_TEMP, MODE_RAW6, MODE_SILENT };
Mode mode = MODE_ACCEL;

Preferences prefs;

// ---------- 姿态记录（标度验证用） ----------
// 摆一个姿态按一个数字键（1 平放 / 2 竖放 / 3 倒放），固件采 100 帧求均值存下来。
// 三个都记完按 v（或 p），一次把零偏 + 两张 markdown 表全打出来，整段复制粘贴即可。
// 为什么记 100 帧而不是打一帧：串口刷得快，单帧噪声 ±0.01g，抄的时候根本不知道该抄哪一行。
// 姿态原始均值也存 flash（和零偏同一个命名空间）——只存内存的话，清一次控制台或复位就全没了，
// 而零偏还在，汇总表会打出"未记"，看着像固件坏了。
struct PoseRec {
  bool  valid;
  float rawAx, rawAy, rawAz;   // 原始均值（LSB）——"不校准"那一栏、以及存 flash 的都是这个
};
PoseRec poses[3];
const char* POSE_NAME[3]    = { "平放", "竖放", "倒放" };
const char* POSE_EXPECT[3]  = { "az=+1.000", "ay=±1.000", "az=−1.000" };
const char* NVS_POSE_KEY[3] = { "p0", "p1", "p2" };

// ---------- 底层：读写寄存器 ----------
// endTransmission(false) 的 false = 不释放总线（发 restart 而不是 stop），
// 后面紧跟 requestFrom 才是完整的一次"读"。
// 这一点写错的表现是 WHO_AM_I 恒回 0xFF 或 0，而且错得很稳定——最坑的一种。

uint8_t readReg(uint8_t reg) {
  Wire.beginTransmission(MPU);
  Wire.write(reg);
  Wire.endTransmission(false);          // 不释放总线
  Wire.requestFrom(MPU, (uint8_t)1);
  return Wire.available() ? (uint8_t)Wire.read() : 0xFF;
}

void writeReg(uint8_t reg, uint8_t val) {
  Wire.beginTransmission(MPU);
  Wire.write(reg);
  Wire.write(val);
  Wire.endTransmission();               // 写操作正常释放
}

// 一次传输读完 14 字节：加速度 6 + 温度 2 + 陀螺仪 6。
Sample readAll() {
  Sample s = { 0, 0, 0, 0, 0, 0, 0 };
  Wire.beginTransmission(MPU);
  Wire.write(REG_ACCEL_XOUT_H);         // 从加速度起头，一次拉 14 字节
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, (uint8_t)14);
  if (Wire.available() < 14) return s;  // 读不满就当这一帧作废，别用半包数据
  s.ax  = (int16_t)(Wire.read() << 8 | Wire.read());
  s.ay  = (int16_t)(Wire.read() << 8 | Wire.read());
  s.az  = (int16_t)(Wire.read() << 8 | Wire.read());
  s.temp = (int16_t)(Wire.read() << 8 | Wire.read());
  s.gx  = (int16_t)(Wire.read() << 8 | Wire.read());
  s.gy  = (int16_t)(Wire.read() << 8 | Wire.read());
  s.gz  = (int16_t)(Wire.read() << 8 | Wire.read());
  return s;
}

// ---------- 零偏校准 ----------
// 只采"静止"态。采的时候手别碰板子、桌上别有震动源（风扇、手机震动）。
// 2000 次是经验值：少于 500 均值不稳，多于 5000 收益递减还占时间。
void calibrate() {
  RgbCycle::setColor(255, 200, 0);      // 黄：正在采，别碰
  Serial.println("\n[cal] 静止采 2000 次，手别碰板子…");
  delay(500);                            // 给手离开的时间

  double sax = 0, say = 0, saz = 0, sgx = 0, sgy = 0, sgz = 0;
  int good = 0;
  for (int i = 0; i < CAL_SAMPLES; i++) {
    Sample s = readAll();
    if (s.ax == 0 && s.ay == 0 && s.az == 0) continue;   // 读失败的帧别混进均值
    sax += s.ax; say += s.ay; saz += s.az;
    sgx += s.gx; sgy += s.gy; sgz += s.gz;
    good++;
    delay(1);
  }
  if (good < CAL_SAMPLES / 2) {
    Serial.printf("[cal] 有效帧只有 %d/%d，I2C 可能有问题，放弃保存\n", good, CAL_SAMPLES);
    RgbCycle::setColor(255, 0, 0);
    delay(1000);
    return;
  }

  // ax/ay 平放时受力为 0，零偏 = 实测均值。
  // az 不一样：平放时重力全压在它上面，实测的是 1g + 零偏。
  // 要减掉的只是零偏那一小截，所以减 16384（1g）。
  // 不减的话去偏后 az ≈ 0，而 §六 要求平放 az = +1.000——Day 40 的 atan2 也指望它是 1g。
  biasAx = sax / good; biasAy = say / good; biasAz = saz / good - ACCEL_LSB_PER_G;
  biasGx = sgx / good; biasGy = sgy / good; biasGz = sgz / good;
  calibrated = true;

  Serial.printf("[cal] 有效 %d 帧，零偏（原始 LSB）：\n", good);
  Serial.printf("  accel  ax=%+.1f ay=%+.1f az=%+.1f（az 已扣掉 1g，平放时它上面压着 1.000g）\n", biasAx, biasAy, biasAz);
  Serial.printf("  gyro   gx=%+.1f gy=%+.1f gz=%+.1f\n", biasGx, biasGy, biasGz);
  Serial.printf("  平放去偏后 az = %+.3f g（应接近 +1.000，这就是标度的第一个参照）\n",
                (float)((readAll().az - biasAz) / ACCEL_LSB_PER_G));

  // 校准时的自检。板子平放且不动时：ax/ay 零偏 < 500 LSB（0.03g）、
  // az 零偏 = 16384×(cosθ−1)，歪 10° 才有 −249，所以 < 500；
  // 陀螺静止时零偏 < 300 LSB（2.3°/s）。超了就是校准姿态歪了或在动，
  // 这份零偏是脏的——存进去会让之后每一个读数都错，不如重按一次 k。
  bool dirty = fabsf(biasAx) > 500 || fabsf(biasAy) > 500 || fabsf(biasAz) > 500 ||
               fabsf(biasGx) > 300 || fabsf(biasGy) > 300 || fabsf(biasGz) > 300;
  if (dirty) {
    Serial.println("[cal] ⚠️ 这次校准可疑：板子当时没放平，或者采的 3.3 秒里动了。");
    Serial.println("      ax/ay 零偏 > 500 说明歪了 >3°，陀螺零偏 > 300 说明在转。");
    Serial.println("      这份零偏是脏的——放平、别碰、桌上别有震动，重按一次 k 覆盖它。");
  }

  prefs.begin("day39", false);
  prefs.putFloat("bax", biasAx); prefs.putFloat("bay", biasAy); prefs.putFloat("baz", biasAz);
  prefs.putFloat("bgx", biasGx); prefs.putFloat("bgy", biasGy); prefs.putFloat("bgz", biasGz);
  prefs.putBool("cal", true);
  prefs.end();
  Serial.println("[cal] 已存 flash，重启不丢；Day 40 的固件直接读这份");
  RgbCycle::setColor(0, 255, 0);
}

// ---------- 打印 ----------
void printSample(const Sample& s) {
  // 打印格式刻意和 Day 37 的帧日志同构：时间戳打头，量纲写清楚，等号对齐。
  // telemetry_plot.py 是按这个格式解析的，改列名要先改脚本。
  switch (mode) {
    case MODE_ACCEL:
      Serial.printf("[t=%6.2fs] ax=%+7.3f ay=%+7.3f az=%+7.3f g   raw %6d %6d %6d\n",
                    millis() / 1000.0f,
                    (s.ax - biasAx) / ACCEL_LSB_PER_G,
                    (s.ay - biasAy) / ACCEL_LSB_PER_G,
                    (s.az - biasAz) / ACCEL_LSB_PER_G,
                    s.ax, s.ay, s.az);
      break;
    case MODE_GYRO:
      Serial.printf("[t=%6.2fs] gx=%+8.2f gy=%+8.2f gz=%+8.2f °/s   raw %6d %6d %6d\n",
                    millis() / 1000.0f,
                    (s.gx - biasGx) / GYRO_LSB_PER_DPS,
                    (s.gy - biasGy) / GYRO_LSB_PER_DPS,
                    (s.gz - biasGz) / GYRO_LSB_PER_DPS,
                    s.gx, s.gy, s.gz);
      break;
    case MODE_TEMP:
      // 数据手册公式：T = raw/340 + 36.53。没有零偏可校——温度不准不影响角度，
      // 打出来只是用来判断"芯片是不是被电机烤了"。
      Serial.printf("[t=%6.2fs] temp raw %5d → %+.2f °C\n",
                    millis() / 1000.0f, s.temp, s.temp / 340.0f + 36.53f);
      break;
    case MODE_RAW6:
      // 给绘图脚本的紧凑格式：一个时间戳 + 6 个量，空格分隔。
      Serial.printf("%.2f %.4f %.4f %.4f %.3f %.3f %.3f\n",
                    millis() / 1000.0f,
                    (s.ax - biasAx) / ACCEL_LSB_PER_G,
                    (s.ay - biasAy) / ACCEL_LSB_PER_G,
                    (s.az - biasAz) / ACCEL_LSB_PER_G,
                    (s.gx - biasGx) / GYRO_LSB_PER_DPS,
                    (s.gy - biasGy) / GYRO_LSB_PER_DPS,
                    (s.gz - biasGz) / GYRO_LSB_PER_DPS);
      break;
    case MODE_SILENT:
      break;
  }
}

void showConstants() {
  Serial.println("\n[w] 当前常量：");
  Serial.printf("  量程     accel ±2g (16384 LSB/g)   gyro ±250°/s (131 LSB/(°/s))\n");
  Serial.printf("  零偏     ax=%+.1f ay=%+.1f az=%+.1f | gx=%+.1f gy=%+.1f gz=%+.1f\n",
                biasAx, biasAy, biasAz, biasGx, biasGy, biasGz);
  Serial.printf("  已校准   %s\n", calibrated ? "是" : "否（现在打的是原始值，没减零偏）");
  Serial.println("  标度验证：摆姿态按 1（平放）/ 2（竖放）/ 3（倒放），按 v 或 p 出汇总表");
}

// ---------- 姿态记录（标度验证用） ----------
// 去偏后的三个分量 + 合成重力。原始值存 flash，去偏在用的时候才算——
// 这样以后重按 k 换了零偏，汇总表会自动跟着新零偏走，不会新旧对不上。
void poseDebiased(const PoseRec& p, float& ax, float& ay, float& az, float& mag) {
  ax = (p.rawAx - biasAx) / ACCEL_LSB_PER_G;
  ay = (p.rawAy - biasAy) / ACCEL_LSB_PER_G;
  az = (p.rawAz - biasAz) / ACCEL_LSB_PER_G;
  mag = sqrtf(ax * ax + ay * ay + az * az);
}

void savePoses() {
  prefs.begin("day39", false);
  int mask = 0;
  for (int i = 0; i < 3; i++) {
    if (!poses[i].valid) continue;
    float raw[3] = { poses[i].rawAx, poses[i].rawAy, poses[i].rawAz };
    prefs.putBytes(NVS_POSE_KEY[i], raw, sizeof(raw));
    mask |= (1 << i);
  }
  prefs.putInt("pmask", mask);
  prefs.end();
}

void loadPoses() {
  prefs.begin("day39", true);
  int mask = prefs.getInt("pmask", 0);
  for (int i = 0; i < 3; i++) {
    if (!(mask & (1 << i))) continue;
    float raw[3];
    if (prefs.getBytes(NVS_POSE_KEY[i], raw, sizeof(raw)) != sizeof(raw)) continue;
    poses[i].valid = true;
    poses[i].rawAx = raw[0]; poses[i].rawAy = raw[1]; poses[i].rawAz = raw[2];
  }
  prefs.end();
}

void recordPose(int i) {
  RgbCycle::setColor(255, 200, 0);      // 黄：正在采，别碰
  Serial.printf("\n[%d] 记「%s」（期望 %s）：摆好别动，采 100 帧求均值…\n",
                i + 1, POSE_NAME[i], POSE_EXPECT[i]);
  delay(400);                            // 给手离开的时间

  double sax = 0, say = 0, saz = 0;
  int good = 0;
  for (int n = 0; n < 100; n++) {
    Sample s = readAll();
    if (s.ax == 0 && s.ay == 0 && s.az == 0) continue;
    sax += s.ax; say += s.ay; saz += s.az;
    good++;
    delay(5);
  }
  if (good < 50) {
    Serial.printf("[%d] 有效帧只有 %d/100，放弃\n", i + 1, good);
    if (good == 0) {
      // 100 帧全是 0 = requestFrom 一字节都没拿到，I2C 总线断了。
      // 这不是"测不出来"，是根本没通——宁可报错，也不能把 100 个 0 平均成一份"数据"。
      Serial.println("      100 帧全 0 → I2C 没通（读寄存器拿不到 14 字节）。查线：");
      Serial.println("        · SDA(GPIO41) / SCL(GPIO42) 杜邦线没插紧，或只插进去一半");
      Serial.println("        · GND 没共地（没有回流路径，总线永远读不到 ACK）");
      Serial.println("        · VCC 松了 / 模块没上电");
      Serial.println("      先按 a 看流：raw 全是 0 就是线的问题，和姿态、和面包板都没关系。");
    }
    RgbCycle::setColor(255, 0, 0);
    return;
  }

  poses[i].valid = true;
  poses[i].rawAx = sax / good; poses[i].rawAy = say / good; poses[i].rawAz = saz / good;
  savePoses();

  float ax, ay, az, mag;
  poseDebiased(poses[i], ax, ay, az, mag);
  Serial.printf("  已记（%d 帧）去偏后：ax=%+.3f ay=%+.3f az=%+.3f  合成重力=%.3f\n",
                good, ax, ay, az, mag);
  Serial.printf("  原始 LSB 均值：%+.0f / %+.0f / %+.0f（校准前的那一栏）\n",
                poses[i].rawAx, poses[i].rawAy, poses[i].rawAz);
  int done = 0;
  for (int k = 0; k < 3; k++) if (poses[k].valid) done++;
  Serial.printf("  已记 %d/3 个姿态。三个都记完按 v 出汇总表\n", done);
  RgbCycle::setColor(0, 255, 0);
}

void dumpReport() {
  Serial.println("\n================ Day 39 实验2 汇总 ================");
  Serial.println("零偏（flash，NVS 命名空间 day39）：");
  Serial.printf("  accel  ax=%+.1f ay=%+.1f az=%+.1f\n", biasAx, biasAy, biasAz);
  Serial.printf("  gyro   gx=%+.1f gy=%+.1f gz=%+.1f\n", biasGx, biasGy, biasGz);
  Serial.printf("  已校准：%s\n", calibrated ? "是" : "否（现在打的是原始值，没减零偏）");

  Serial.println("\n—— 表1 标度验证（每格 100 帧均值，期望：合成重力 = 1.000）——");
  Serial.println("| 姿态 | ax | ay | az | 合成重力 | 判定 |");
  Serial.println("|---|---|---|---|---|---|");
  for (int i = 0; i < 3; i++) {
    if (!poses[i].valid) {
      Serial.printf("| %s | | | | | 未记，按 %d |\n", POSE_NAME[i], i + 1);
      continue;
    }
    float ax, ay, az, mag;
    poseDebiased(poses[i], ax, ay, az, mag);
    float err = mag - 1.0f;
    Serial.printf("| %s | %+.3f | %+.3f | %+.3f | %.3f | %s %s |\n",
                  POSE_NAME[i], ax, ay, az, mag,
                  fabsf(err) <= 0.02f ? "✓" : "✗",
                  fabsf(err) <= 0.02f ? "应 = 1.000" : "合成重力不对");
  }

  Serial.println("\n—— 表2 不校准 vs 校准（同一姿态平放）——");
  Serial.println("| | 校准前 ax/ay/az | 校准后 ax/ay/az |");
  Serial.println("|---|---|---|");
  if (poses[0].valid) {
    float ax, ay, az, mag;
    poseDebiased(poses[0], ax, ay, az, mag);
    Serial.printf("| 平放 | %+.3f / %+.3f / %+.3f（raw %+.0f / %+.0f / %+.0f） | %+.3f / %+.3f / %+.3f |\n",
                  poses[0].rawAx / ACCEL_LSB_PER_G, poses[0].rawAy / ACCEL_LSB_PER_G,
                  poses[0].rawAz / ACCEL_LSB_PER_G,
                  poses[0].rawAx, poses[0].rawAy, poses[0].rawAz,
                  ax, ay, az);
  } else {
    Serial.println("| 平放 | （未记，按 1） | |");
  }
  Serial.println("====================================================");
}

// ---------- 串口 ----------
void handleCmd(char c) {
  switch (c) {
    case 'a': case 'A': mode = MODE_ACCEL; Serial.println("[mode] 加速度计"); break;
    case 'g': case 'G': mode = MODE_GYRO;  Serial.println("[mode] 陀螺仪"); break;
    case 't': case 'T': mode = MODE_TEMP;  Serial.println("[mode] 温度"); break;
    case 'r': case 'R': mode = MODE_RAW6;  Serial.println("[mode] 六轴紧凑（给绘图）"); break;
    case '0':           mode = MODE_SILENT; Serial.println("[mode] 静音"); break;
    case 'k': case 'K': calibrate(); break;
    case 'w': case 'W': showConstants(); break;
    case 'v': case 'V': dumpReport(); break;
    case 'p': case 'P': dumpReport(); break;
    case '1': recordPose(0); break;
    case '2': recordPose(1); break;
    case '3': recordPose(2); break;
    default:
      if (c == '\n' || c == '\r') break;   // 串口监视器发命令自带换行，别当命令
      Serial.printf("[?] 未知命令 '%c'。a/g/t/r/k/w/v/p/1/2/3/0\n", c);
  }
}

// ---------- 主循环 ----------
void setup() {
  Serial.begin(115200);
  delay(200);                            // 给串口监视器连上的时间

  RgbCycle::begin();
  RgbCycle::setColor(0, 0, 255);         // 蓝：正在初始化

  // ESP32-S3 没有固定 I2C 引脚，必须显式指定；默认那套（GPIO21/22 之类）不是本板的。
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(400000);                 // 400kHz 快速模式；100kHz 也能跑，慢一倍

  Serial.println("\n=== Day 39：MPU-6050 裸读 + 校准 ===");
  Serial.printf("I2C  SDA=GPIO%d SCL=GPIO%d 400kHz  地址 0x%02X\n", SDA_PIN, SCL_PIN, MPU);

  uint8_t who = readReg(REG_WHO_AM_I);
  Serial.printf("WHO_AM_I(0x75) = 0x%02X（期望 0x68）", who);
  if (who != 0x68) {
    Serial.println("  ✗ 对不上！");
    Serial.println("  排查顺序：① AD0 是不是悬空了 ② VCC/GND 共地没 ③ 线插到底没");
    Serial.println("  （Day 18 踩过：扫描器能扫到、库初始化却报 not found，根因是母头没插紧）");
    Serial.println("  先跑 Day 18 的 I2C Scanner 确认总线，再回来。");
    RgbCycle::setColor(255, 0, 0);
    while (true) delay(100);             //  halt：地址错了一切后续都没意义
  }
  Serial.println("  ✓");

  writeReg(REG_PWR_MGMT_1, 0x00);        // 退出睡眠。不写这个寄存器所有读数恒 0
  delay(100);                            // 等内部时钟稳定，手册要求
  writeReg(REG_ACCEL_CONFIG, 0x00);      // ±2g
  writeReg(REG_GYRO_CONFIG, 0x00);       // ±250°/s
  delay(50);

  // 回读确认量程真的写进去了。写寄存器不报错，但读出来不是 0 就说明写失败。
  Serial.printf("ACCEL_CONFIG(0x1C) = 0x%02X（期望 0x00 = ±2g）\n", readReg(REG_ACCEL_CONFIG));
  Serial.printf("GYRO_CONFIG(0x1B)  = 0x%02X（期望 0x00 = ±250°/s）\n", readReg(REG_GYRO_CONFIG));

  // 读上次存的零偏。没有（第一次上电）就 calibrated = false，
  // 打出来的数带着原始零偏——这正好是要看的"不校准会长什么样"。
  prefs.begin("day39", true);
  if (prefs.getBool("cal", false)) {
    biasAx = prefs.getFloat("bax", 0); biasAy = prefs.getFloat("bay", 0); biasAz = prefs.getFloat("baz", 0);
    biasGx = prefs.getFloat("bgx", 0); biasGy = prefs.getFloat("bgy", 0); biasGz = prefs.getFloat("bgz", 0);
    calibrated = true;
    Serial.println("\n[nvs] 已恢复上次的零偏（平放不动时各轴应接近 0）");
  } else {
    Serial.println("\n[nvs] 没有零偏记录。板子平放别动，按 k 校一个。");
    Serial.println("      现在打的是原始值——故意让你看不校准是什么样。");
  }
  prefs.end();

  loadPoses();                             // 姿态原始均值也在 flash 里，重启接着用
  int npose = 0;
  for (int i = 0; i < 3; i++) if (poses[i].valid) npose++;
  if (npose > 0) Serial.printf("[nvs] 已恢复 %d/3 个姿态记录\n", npose);

  Serial.println("\n命令：a 加速度 / g 陀螺仪 / t 温度 / r 六轴（给绘图）/ 0 静音");
  Serial.println("      k 校零偏 / w 看常量 / 1 平放 2 竖放 3 倒放 / v 出汇总表");
  Serial.println("\n--- 开始打（10Hz）---\n");
  RgbCycle::setColor(0, 255, 0);         // 绿：正常
}

void loop() {
  while (Serial.available()) handleCmd((char)Serial.read());

  static unsigned long last = 0;
  unsigned long now = millis();
  if (now - last < PERIOD_MS) return;
  last = now;

  Sample s = readAll();
  printSample(s);
}
