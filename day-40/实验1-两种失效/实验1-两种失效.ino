/*
 * Day40 实验1 - 两种失效模式 (MPU6050 / ESP32-S3)
 * ------------------------------------------------------------
 * 并排打两个俯仰角, 看它们各自的病:
 *   accel   加速度计 atan2 直算 : 静止也在抖(噪声), 但不漂
 *   gyro    陀螺仪积分         : 平滑, 但零偏积分成单向漂
 * 摆着不动看 60 秒: 一个抖、一个漂, 两条曲线分开——
 * 这就是"必须融合"的实感证据, 实验2 用互补滤波治好它。
 *
 * 标定值从 flash 读(Day39 存的, NVS 命名空间 day39):
 *   bax/bay/baz 加速度零偏   sax/say/saz 标度(没有就用默认 16384)
 *   bgx/bgy/bgz 陀螺零偏
 * ⚠️ 只跑过 Day39 实验1 没跑六面标定的话, flash 里没有标度,
 *    Y 轴会差几个百分点(Day39 实测 4.6%), 含 Y 的姿态上 accel 角会偏。
 *    先烧 Day39 实验2 跑完六面标定再玩今天。
 *
 * 硬件/约定同 Day39:
 *    VCC->3V3  SDA->GPIO41  SCL->GPIO42  AD0->GND(地址0x68)
 *    串口 115200
 *
 * 键: z = 陀螺积分重新置零(顺便重新计 60 秒)   v = 打印当前值
 */

#include <Wire.h>
#include <Preferences.h>
#include <math.h>

// ==================== 配置区 ====================
const int      SDA_PIN    = 41;
const int      SCL_PIN    = 42;
const uint8_t  MPU        = 0x68;    // AD0接GND=0x68, 接VCC=0x69
const int      SAMPLE_HZ  = 100;
const int      PRINT_DIV  = 10;      // 每10次采样打印一次 = 10Hz
const char*    NS         = "day39"; // 读 Day39 存的标定值
// 陀螺符号: 倾斜时若 gyro 角朝 accel 的反方向跑, 把这个改成 -1
const float    GYR_SIGN   = 1.0;
// ================================================

// 寄存器(同 Day39)
const uint8_t REG_PWR_MGMT_1   = 0x6B;
const uint8_t REG_ACCEL_CONFIG = 0x1C;   // ±2g
const uint8_t REG_GYRO_CONFIG  = 0x1B;   // ±250dps
const uint8_t REG_CONFIG       = 0x1A;   // 低通
const uint8_t REG_WHO_AM_I     = 0x75;
const uint8_t REG_ACCEL_OUT    = 0x3B;

const float GYR_LSB_PER_DPS = 131.0;     // ±250dps
const float RAD2DEG         = 57.29578;

// 标定值(从 flash 加载)
float bax = 0, bay = 0, baz = 0;
float sax = 16384, say = 16384, saz = 16384;
float bgx = 0, bgy = 0, bgz = 0;
bool  calOK   = false;
bool  scaleOK = false;      // flash 里有没有六面标定存的标度

float        gyroAngle = 0; // 陀螺积分角(度)
unsigned long t0       = 0; // 观察计时起点

Preferences prefs;

// ---------------- MPU6050 读写 ----------------
void mpuWrite(uint8_t reg, uint8_t val) {
  Wire.beginTransmission(MPU);
  Wire.write(reg);
  Wire.write(val);
  Wire.endTransmission();
}

uint8_t whoAmI() {
  Wire.beginTransmission(MPU);
  Wire.write(REG_WHO_AM_I);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 1, true);
  return Wire.available() ? Wire.read() : 0xFF;
}

// 返回 false = 这一帧不能用: 要么 I2C 没回够 14 字节, 要么六路 raw 全 0。
// 全 0 不是"静止", 是线松了——raw=0 经过去偏/标度照样能算出一个
// 煞有介事的假角(实测: 恰好 -6.19°), 不拦下来就会当成真数据积分+打印。
bool mpuRead(int16_t* ax, int16_t* ay, int16_t* az,
             int16_t* gx, int16_t* gy, int16_t* gz) {
  Wire.beginTransmission(MPU);
  Wire.write(REG_ACCEL_OUT);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 14, true);
  if (Wire.available() < 14) return false;
  *ax = (Wire.read() << 8) | Wire.read();
  *ay = (Wire.read() << 8) | Wire.read();
  *az = (Wire.read() << 8) | Wire.read();
  Wire.read(); Wire.read();                 // 温度, 跳过
  *gx = (Wire.read() << 8) | Wire.read();
  *gy = (Wire.read() << 8) | Wire.read();
  *gz = (Wire.read() << 8) | Wire.read();
  if (*ax == 0 && *ay == 0 && *az == 0 && *gx == 0 && *gy == 0 && *gz == 0)
    return false;
  return true;
}

// 坏帧提示限速到约 1 秒一次, 免得把串口刷爆
void warnBadFrame() {
  static unsigned long lastWarn = 0;
  unsigned long now = millis();
  if (now - lastWarn >= 1000) {
    lastWarn = now;
    Serial.println("⚠️ 读到全 0 帧(I2C 断了? 查杜邦线), 本帧已丢弃: 不积分、不打印");
  }
}

// ---------------- 读 Day39 的标定值 ----------------
void loadCalib() {
  prefs.begin(NS, true);                    // 只读
  calOK   = prefs.getBool("cal", false);
  scaleOK = prefs.isKey("sax");             // 六面标定才会写这个键
  bax = prefs.getFloat("bax", 0);  bay = prefs.getFloat("bay", 0);  baz = prefs.getFloat("baz", 0);
  sax = prefs.getFloat("sax", 16384); say = prefs.getFloat("say", 16384); saz = prefs.getFloat("saz", 16384);
  bgx = prefs.getFloat("bgx", 0);  bgy = prefs.getFloat("bgy", 0);  bgz = prefs.getFloat("bgz", 0);
  prefs.end();

  Serial.println("\n---------------- 标定值(来自 Day39, NVS=day39) ----------------");
  if (calOK) {
    Serial.printf("零偏 bias(LSB)   : bax=%+.1f bay=%+.1f baz=%+.1f\n", bax, bay, baz);
    Serial.printf("标度 scale(LSB/g): sax=%.1f say=%.1f saz=%.1f\n", sax, say, saz);
    Serial.printf("陀螺零偏(LSB)    : bgx=%+.1f bgy=%+.1f bgz=%+.1f\n", bgx, bgy, bgz);
    if (!scaleOK)
      Serial.println("⚠️ flash 里没有标度(没跑六面标定?), 先用默认 16384。");
    else
      Serial.println("标度已加载(六面标定那份)。");
  } else {
    Serial.println("⚠️ flash 里没有标定记录! 先烧 Day39 跑一遍校准再回来。");
    Serial.println("   今天先按 零偏=0/标度=16384 跑, 数字会很难看。");
  }

  // 自检: 标度三个全等 16384 = 六面标定的守卫兜底(六个姿势 max-min 全 ≤ 1000 LSB),
  // 说明标定时板子基本没换过姿势; 零偏上千 LSB 也不是正常静止该有的量级。
  bool scaleGuarded = (sax == 16384 && say == 16384 && saz == 16384);
  bool biasWild     = (fabs(bax) > 4000 || fabs(bay) > 4000 || fabs(baz) > 4000);
  if (calOK && (scaleGuarded || biasWild)) {
    Serial.println("\n⚠️⚠️ 这份标定不可信, 今天的数会全错:");
    if (scaleGuarded)
      Serial.println("  三个标度全是 16384 —— 六面标定的守卫兜底值, 说明六个姿势没真换");
    if (biasWild)
      Serial.println("  有零偏上万/上千 LSB(≈好几 g), 正常静止零偏只有几百");
    Serial.println("  → 回 Day39 重跑六面标定(六个方向真的换一遍), 再回来。\n");
  }
}

// ---------------- 打印一行当前值 ----------------
void printNow(float accel, float gyro) {
  float t = (millis() - t0) / 1000.0;
  Serial.printf("accel=%+7.2f  gyro=%+8.2f  diff=%+7.2f  t=%3.0fs\n",
                accel, gyro, accel - gyro, t);
}

void printStartHint() {
  Serial.println("\n---------------- 怎么看 ----------------");
  Serial.println("板子摆好别扶, 盯着这两列 60 秒:");
  Serial.println("  accel : 围着真值上下抖 —— 噪声, 但不跑远");
  Serial.println("  gyro  : 很平滑, 但一个劲地单方向爬 —— 零偏被积分成了漂移");
  Serial.println("  diff  : 两者的差距, 就是'必须融合'的证据");
  Serial.println("发 'z' 陀螺积分归零重新计时, 发 'v' 打印当前值。\n");
}

// ---------------- 主流程 ----------------
void setup() {
  Serial.begin(115200);
  delay(300);
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(400000);

  Serial.println("\n\n===== Day40 实验1 两种失效模式 =====");
  mpuWrite(REG_PWR_MGMT_1, 0x00);      // 唤醒
  mpuWrite(REG_ACCEL_CONFIG, 0x00);    // ±2g
  mpuWrite(REG_GYRO_CONFIG, 0x00);     // ±250dps
  mpuWrite(REG_CONFIG, 0x03);          // 低通 ~44Hz
  delay(100);

  uint8_t who = whoAmI();
  Serial.printf("WHO_AM_I(0x75)=0x%02X %s\n", who, (who == 0x68) ? "(OK)" : "(异常!检查接线/地址)");
  if (who != 0x68) {
    Serial.println("!! MPU6050 无响应, 请检查 SDA/SCL/AD0 后重启");
    while (1) delay(100);
  }

  loadCalib();
  printStartHint();
  t0 = millis();
}

void loop() {
  static bool wantPrint = false;

  if (Serial.available()) {
    char c = Serial.read();
    if (c == 'z' || c == 'Z') {
      gyroAngle = 0;
      t0 = millis();
      Serial.println("\n>>> 陀螺积分已归零, 重新计时\n");
    } else if (c == 'v' || c == 'V') {
      wantPrint = true;               // 立刻打一行, 不等下一个周期
    }
  }

  int16_t axr, ayr, azr, gxr, gyr, gzr;
  bool ok = mpuRead(&axr, &ayr, &azr, &gxr, &gyr, &gzr);

  // dt 用 micros() 实测差值, 不写死(1ms 的 millis 分辨率对 10ms 采样是 10% 量化误差)
  static unsigned long lastUs = 0;
  unsigned long now = micros();
  if (lastUs == 0) lastUs = now;
  float dt = (now - lastUs) / 1000000.0;
  lastUs = now;

  // 坏帧直接丢: 积分会记住假的漂移, 打印会污染日志, 两个都不做
  if (!ok) {
    warnBadFrame();
    wantPrint = false;
    delay(1000 / SAMPLE_HZ);
    return;
  }

  // 去偏 + 标度 -> g
  float ax = (axr - bax) / sax;
  float ay = (ayr - bay) / say;
  float az = (azr - baz) / saz;
  // 陀螺 -> °/s
  float gyrY = (gyr - bgy) / GYR_LSB_PER_DPS * GYR_SIGN;

  // 加速度计角: 重力方向和板面法线的夹角
  float accelAngle = atan2(-ax, sqrt(ay * ay + az * az)) * RAD2DEG;
  // 陀螺积分角
  gyroAngle += gyrY * dt;

  static int n = 0;
  if (++n >= PRINT_DIV) { n = 0; wantPrint = true; }
  if (wantPrint) { printNow(accelAngle, gyroAngle); wantPrint = false; }

  delay(1000 / SAMPLE_HZ);
}
