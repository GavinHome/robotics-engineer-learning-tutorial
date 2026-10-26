/*
 * Day40 实验2 - 互补滤波 (MPU6050 / ESP32-S3)
 * ------------------------------------------------------------
 *   angle = α × (angle + gyr_y × dt) + (1 − α) × accel_angle
 *
 * 陀螺管快变(高通), 加速度计管慢变(低通), 两个频段互补:
 *   实验1 里 accel 的抖 被 (1−α) 压掉, gyro 的漂 被 α 的高通滤掉。
 *
 * α 四档一次调完: 1/2/3/4 -> 0.90 / 0.95 / 0.98 / 0.995
 * 验收: 发 z 置零后静止 60 秒, maxDev(最大偏离) < 1° 为达标。
 *
 * 标定值从 flash 读(Day39 六面标定存的, NVS 命名空间 day39):
 *   bax/bay/baz 零偏, sax/say/saz 标度, bgx/bgy/bgz 陀螺零偏
 * ⚠️ 没跑六面标定就没有标度键, 会用默认 16384, Y 轴差几个百分点。
 *
 * 硬件/约定同 Day39:
 *    VCC->3V3  SDA->GPIO41  SCL->GPIO42  AD0->GND(地址0x68)
 *    串口 115200
 *
 * 键: 1/2/3/4 切 α    z 置零+开始60秒验收统计    v 打印当前状态
 */

#include <Wire.h>
#include <Preferences.h>
#include <math.h>

// ==================== 配置区 ====================
const int      SDA_PIN    = 41;
const int      SCL_PIN    = 42;
const uint8_t  MPU        = 0x68;    // AD0接GND=0x68, 接VCC=0x69
const int      SAMPLE_HZ  = 100;
const int      PRINT_DIV  = 10;      // 10Hz 打印
const char*    NS         = "day39";
const float    DT_NOMINAL = 0.01;    // 100Hz, 只用来算"校正时间常数"的显示值
const float    ACCEPT_SEC = 60.0;    // 验收窗口
const float    ACCEPT_DEG = 1.0;     // 验收门限: 最大偏离 < 1°
const float    SETTLE_SEC = 3.0;     // 按 z 后先让板子稳这么久, 再取基准、开始计时
// 陀螺符号: 倾斜时若 fused 远离 accel 而不是跟随, 把这个改成 -1
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

// α 四档
const int   NALPHA = 4;
const float ALPHAS[NALPHA] = { 0.90, 0.95, 0.98, 0.995 };

// 标定值(从 flash 加载)
float bax = 0, bay = 0, baz = 0;
float sax = 16384, say = 16384, saz = 16384;
float bgx = 0, bgy = 0, bgz = 0;
bool  calOK   = false;
bool  scaleOK = false;

// 滤波状态
float alpha      = 0.98;   // 当前档位
float angle      = 0;      // 融合角(度)
float gyroAngle  = 0;      // 陀螺纯积分角(度), 并排显示用
float refAngle   = 0;      // 验收基准角(取稳后的那个角)
float maxDev     = 0;      // 验收窗口内最大偏离
unsigned long t0 = 0;      // z 时刻
bool  baselineSet = false; // 取稳了没有
bool  reported   = false;  // 结论打过了没有

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
// 煞有介事的假角, 不拦下来就会喂进滤波器, 把 fused 也带歪。
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
    Serial.println("⚠️ 读到全 0 帧(I2C 断了? 查杜邦线), 本帧已丢弃: 不滤波、不打印");
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

// α 的校正时间常数: 加速度计把融合角拉回真值的快慢
float tauOf(float a) {
  return a * DT_NOMINAL / (1.0 - a);
}

// ---------------- 打印 ----------------
void printLine(float accel, float gyro, float fused, float dev, float t) {
  Serial.printf("accel=%+7.2f  gyro=%+8.2f  fused=%+7.2f  a=%.3f  maxDev=%.2f  t=%3.0fs\n",
                accel, gyro, fused, alpha, dev, t);
}

void printState(float accel, float gyro, float fused) {
  float t = (millis() - t0) / 1000.0;
  Serial.println("\n================ Day40 实验2 互补滤波 · 当前状态 ================");
  Serial.printf("α = %.3f  (信陀螺 %.1f%%, 信加速度计 %.1f%%, 校正时间常数 ≈ %.2fs)\n",
                alpha, alpha * 100, (1 - alpha) * 100, tauOf(alpha));
  Serial.printf("accel(抖)    = %+7.2f\n", accel);
  Serial.printf("gyro积分(漂) = %+8.2f\n", gyro);
  Serial.printf("fused        = %+7.2f\n", fused);
  if (baselineSet) {
    float ta = t - SETTLE_SEC;
    Serial.printf("z 以来 %.1f 秒(验收 %.1f 秒), 最大偏离 %.2f° %s\n",
                  t, ta, maxDev,
                  (ta >= ACCEPT_SEC) ? ((maxDev < ACCEPT_DEG) ? "→ 达标" : "→ 未达标") : "");
  } else {
    Serial.printf("z 以来 %.1f 秒, 正在取稳(还有 %.1f 秒)\n", t, SETTLE_SEC - t);
  }
  Serial.println("=========================================================");
}

void printStartHint() {
  Serial.println("\n---------------- 操作说明 ----------------");
  Serial.println("并排看三个数: accel(抖) / gyro积分(漂) / fused(又稳又不漂)");
  Serial.println("调 α: 按 1/2/3/4 -> 0.90 / 0.95 / 0.98 / 0.995, 四档一次调完");
  Serial.printf("验收: 按 z 置零 -> 等 %.0f 秒取稳 -> 静止 %.0f 秒, maxDev < %.0f° 为达标\n",
                SETTLE_SEC, ACCEPT_SEC, ACCEPT_DEG);
  Serial.println("  每档流程: 按数字键切档 -> 按 z -> 手离开 -> 抬手约 30° 看响应");
  Serial.println("按 v 随时打印当前状态(可整段复制)。");
  Serial.println(">>> 按 z 开始 <<<\n");
}

// ---------------- 主流程 ----------------
void setup() {
  Serial.begin(115200);
  delay(300);
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(400000);

  Serial.println("\n\n===== Day40 实验2 互补滤波 =====");
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
  Serial.printf("\n当前 α = %.3f (校正时间常数 ≈ %.2fs)\n", alpha, tauOf(alpha));
  printStartHint();
  t0 = millis();
}

void loop() {
  static bool wantLine  = false;   // 打单行
  static bool wantBlock = false;   // 打完整状态块

  if (Serial.available()) {
    char c = Serial.read();
    if (c >= '1' && c <= '0' + NALPHA) {
      alpha = ALPHAS[c - '1'];
      Serial.printf("\n>>> α = %.3f (信陀螺 %.1f%%, 校正时间常数 ≈ %.2fs)\n",
                    alpha, alpha * 100, tauOf(alpha));
    } else if (c == 'z' || c == 'Z') {
      // 置零 + 开始验收。先留 SETTLE_SEC 秒让板子稳下来, 基准取稳下来的角——
      // 刚摆好/刚抬手放下就按 z, 记下的是运动中的值, maxDev 会被污染。
      // 板子微倾也没关系: 基准就是稳下来的那个角, 不是 0。
      baselineSet = false;
      maxDev   = 0;
      reported = false;
      t0       = millis();
      Serial.printf("\n>>> 已置零, 等 %.0f 秒取稳后开始 %.0f 秒验收统计\n", SETTLE_SEC, ACCEPT_SEC);
    } else if (c == 'v' || c == 'V') {
      wantBlock = true;
    }
  }

  int16_t axr, ayr, azr, gxr, gyr, gzr;
  bool ok = mpuRead(&axr, &ayr, &azr, &gxr, &gyr, &gzr);

  // dt 用 micros() 实测差值, 不写死
  static unsigned long lastUs = 0;
  unsigned long now = micros();
  if (lastUs == 0) lastUs = now;
  float dt = (now - lastUs) / 1000000.0;
  lastUs = now;

  // 坏帧直接丢: 滤波器会记住假的漂移, 打印会污染日志。dt 已先更新, 不会积出尖刺。
  if (!ok) {
    warnBadFrame();
    wantLine = false;      // 单行不打(会刷出一模一样的假值)
    wantBlock = false;
    delay(1000 / SAMPLE_HZ);
    return;
  }

  // 去偏 + 标度 -> g
  float ax = (axr - bax) / sax;
  float ay = (ayr - bay) / say;
  float az = (azr - baz) / saz;
  // 陀螺 -> °/s
  float gyrY = (gyr - bgy) / GYR_LSB_PER_DPS * GYR_SIGN;

  // 三个角
  float accelAngle = atan2(-ax, sqrt(ay * ay + az * az)) * RAD2DEG;
  gyroAngle += gyrY * dt;
  angle = alpha * (angle + gyrY * dt) + (1.0 - alpha) * accelAngle;

  // 验收统计: 先取稳, 再计时
  float t = (millis() - t0) / 1000.0;
  if (!baselineSet && t >= SETTLE_SEC) {
    refAngle    = angle;      // 基准 = 稳下来的那个角, 不是 0, 也不是运动中的值
    maxDev      = 0;
    baselineSet = true;
    Serial.printf(">>> 基准已取稳: %+.2f°, 开始 %.0f 秒验收统计\n", refAngle, ACCEPT_SEC);
  }
  if (baselineSet) {
    float dev = fabs(angle - refAngle);
    if (dev > maxDev) maxDev = dev;
  }

  // 验收窗口到, 打一次结论
  if (baselineSet && t >= SETTLE_SEC + ACCEPT_SEC && !reported) {
    reported = true;
    Serial.printf("\n>>> %.0f 秒到: 最大偏离 %.2f° → %s (门限 %.0f°)\n",
                  ACCEPT_SEC, maxDev, (maxDev < ACCEPT_DEG) ? "达标" : "未达标", ACCEPT_DEG);
    if (maxDev >= ACCEPT_DEG)
      Serial.println(">>> 不过就查: 零偏脏没脏 / dt 是不是实测 / α 是不是选大了 / 陀螺符号 / 板子是不是还在慢慢挪\n");
  }

  // 'v' 打完整状态块
  if (wantBlock) { printState(accelAngle, gyroAngle, angle); wantBlock = false; }

  static int n = 0;
  if (++n >= PRINT_DIV) { n = 0; wantLine = true; }
  if (wantLine) { printLine(accelAngle, gyroAngle, angle, maxDev, t); wantLine = false; }

  delay(1000 / SAMPLE_HZ);
}
