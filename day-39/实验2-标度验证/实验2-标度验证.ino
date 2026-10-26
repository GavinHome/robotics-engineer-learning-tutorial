/*
 * Day39 实验2 - 六面自动标定法 (MPU6050 / ESP32-S3)
 * ------------------------------------------------------------
 * 独立程序。依次提示 6 个方向，每个方向：
 *    文字提示 -> 你摆好松手 -> 3秒准备 -> 8秒自动采样求均值
 *    -> 宽松粗检(只抓明显摆错) -> 自动记录 -> 下一步
 * 全程不用按键、不用盯实时数，双手摆位即可。
 *
 * 6 面法原理：让 X/Y/Z 每根轴都"朝上一次、朝下一次"，
 * 用每轴的 (max+min)/2 求零偏、(max-min)/2 求标度。
 * 比 3 步法(平放/竖放/倒放)多标了两根水平轴，合成重力才能到 1.000。
 *
 * 硬件/约定(与实验1一致)：
 *    VCC->3V3  SDA->GPIO41  SCL->GPIO42  AD0->GND(地址0x68)
 *    串口 115200
 *
 * NVS(命名空间 day39，键名与实验1/主程序兼容)：
 *    bax/bay/baz   加速度零偏(LSB)
 *    sax/say/saz   加速度标度(LSB/g)   <- 本实验新增
 *    bgx/bgy/bgz   陀螺零偏(LSB)
 *    cal           是否已标定(bool)
 *    q0..q5/qmask  6个姿态的原始均值(本程序自用)
 */

#include <Wire.h>
#include <Preferences.h>
#include <math.h>

// ==================== 配置区 ====================
const int      SDA_PIN   = 41;
const int      SCL_PIN   = 42;
const uint8_t  MPU       = 0x68;     // AD0接GND=0x68, 接VCC=0x69
const int      SAMPLE_HZ = 100;
const int      HOLD_SEC  = 8;        // 每姿态采样秒数, 可改10
const char*    NS        = "day39";  // NVS命名空间
// ================================================

// 寄存器
const uint8_t REG_PWR_MGMT_1   = 0x6B;
const uint8_t REG_ACCEL_CONFIG = 0x1C;   // ±2g
const uint8_t REG_GYRO_CONFIG  = 0x1B;   // ±250dps
const uint8_t REG_CONFIG       = 0x1A;   // 低通
const uint8_t REG_WHO_AM_I     = 0x75;
const uint8_t REG_ACCEL_OUT    = 0x3B;

// 6 个姿态：纯物理描述，不用记 X/Y
const int   NPOSE = 6;
const char* POSE_NAME[NPOSE] = {
  "平放    芯片丝印朝上, 放桌面",
  "倒放    翻面, 芯片朝下贴桌",
  "竖-A    沿短边立起, 引脚(VCC)那头朝上",
  "竖-B    竖-A翻过来, 引脚(VCC)那头朝下",
  "竖-C    沿长边立起: 有排针(8个引脚)那条长边着桌, 排针那头朝下",
  "竖-D    竖-C翻过来: 换成没排针那条长边着桌, 排针那头朝上"
};

// 标定结果
float biasAx = 0, biasAy = 0, biasAz = 0;      // LSB
float scaleAx = 16384, scaleAy = 16384, scaleAz = 16384;  // LSB/g
float biasGx = 0, biasGy = 0, biasGz = 0;      // LSB
bool  calOK  = false;

float poseRaw[NPOSE][3];   // 6姿态原始均值(LSB)

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

void mpuRead(int16_t* ax, int16_t* ay, int16_t* az,
             int16_t* gx, int16_t* gy, int16_t* gz) {
  Wire.beginTransmission(MPU);
  Wire.write(REG_ACCEL_OUT);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 14, true);
  *ax = (Wire.read() << 8) | Wire.read();
  *ay = (Wire.read() << 8) | Wire.read();
  *az = (Wire.read() << 8) | Wire.read();
  Wire.read(); Wire.read();                 // 温度, 跳过
  *gx = (Wire.read() << 8) | Wire.read();
  *gy = (Wire.read() << 8) | Wire.read();
  *gz = (Wire.read() << 8) | Wire.read();
}

// ---------------- 采一个姿态(自动流程) ----------------
// 返回 true=合格, false=明显摆错需重做
bool doPose(int idx) {
  Serial.println("\n========================================");
  Serial.printf("  第 %d / %d 步\n", idx + 1, NPOSE);
  Serial.printf("  摆成: %s\n", POSE_NAME[idx]);
  Serial.println("  摆好后松手别扶, 3秒后自动采样");
  Serial.println("----------------------------------------");
  for (int i = 3; i > 0; i--) {
    Serial.printf("  准备 %d...\n", i);
    delay(1000);
  }

  Serial.println("  采样中, 保持不动...");
  double sa[3] = {0, 0, 0}, sg[3] = {0, 0, 0};
  int n = 0, total = HOLD_SEC * SAMPLE_HZ;
  for (int t = 0; t < total; t++) {
    int16_t ax, ay, az, gx, gy, gz;
    mpuRead(&ax, &ay, &az, &gx, &gy, &gz);
    sa[0] += ax; sa[1] += ay; sa[2] += az;
    sg[0] += gx; sg[1] += gy; sg[2] += gz;
    n++;
    if (t % SAMPLE_HZ == 0)
      Serial.printf("   剩余 %d 秒\n", (total - t) / SAMPLE_HZ);
    delay(1000 / SAMPLE_HZ);
  }

  poseRaw[idx][0] = sa[0] / n;
  poseRaw[idx][1] = sa[1] / n;
  poseRaw[idx][2] = sa[2] / n;

  // 陀螺全程静止, 累加(最后统一求平均)
  static double gsum[3] = {0, 0, 0};
  static long   gcnt = 0;
  if (idx == 0) { gsum[0] = gsum[1] = gsum[2] = 0; gcnt = 0; }
  gsum[0] += sg[0]; gsum[1] += sg[1]; gsum[2] += sg[2]; gcnt += n;
  biasGx = gsum[0] / gcnt; biasGy = gsum[1] / gcnt; biasGz = gsum[2] / gcnt;

  Serial.printf("  记录 raw(LSB): ax=%.0f  ay=%.0f  az=%.0f\n",
                poseRaw[idx][0], poseRaw[idx][1], poseRaw[idx][2]);

  // 六路 raw 全 0 不是"静止", 是 I2C 一帧都没读回来。
  // 这种数据存进 flash 会把旧值覆盖成空的, 所以直接判本轮作废、不保存。
  if (fabs(poseRaw[idx][0]) < 500 && fabs(poseRaw[idx][1]) < 500 && fabs(poseRaw[idx][2]) < 500) {
    Serial.println("\n  ⚠️⚠️ 这个姿态六路 raw 全是 0 —— I2C 没读回来(查杜邦线插没插紧)");
    Serial.println("  ⚠️ 本轮标定作废, 不会写入 flash(旧值保持原样)。");
    Serial.println("  ⚠️ 查好线后重新上电, 再发 's' 重标。\n");
    return false;
  }

  // 只提示, 绝不拦截: 无论如何都记录并继续下一步
  int maxA = 0;
  float maxV = fabs(poseRaw[idx][0]);
  for (int a = 1; a < 3; a++)
    if (fabs(poseRaw[idx][a]) > maxV) { maxV = fabs(poseRaw[idx][a]); maxA = a; }
  float o1 = fabs(poseRaw[idx][(maxA + 1) % 3]);
  float o2 = fabs(poseRaw[idx][(maxA + 2) % 3]);
  if (maxV < 10000 || o1 > 6000 || o2 > 6000) {
    Serial.println("  (提示)这个方向读数不太典型, 已记录, 继续。");
    Serial.println("  (提示)若摆错了: 标完发 'v' 看表2, 或发 's' 整体重标。");
  } else {
    Serial.println("  OK, 下一步");
  }
  return true;
}

// ---------------- 计算零偏/标度 ----------------
void computeCalib() {
  const float* p[3] = { nullptr, nullptr, nullptr };
  float col[3][NPOSE];
  for (int i = 0; i < NPOSE; i++)
    for (int a = 0; a < 3; a++)
      col[a][i] = poseRaw[i][a];

  float* bias[3]  = { &biasAx,  &biasAy,  &biasAz  };
  float* scale[3] = { &scaleAx, &scaleAy, &scaleAz };
  for (int a = 0; a < 3; a++) {
    float mx = col[a][0], mn = col[a][0];
    for (int i = 1; i < NPOSE; i++) {
      if (col[a][i] > mx) mx = col[a][i];
      if (col[a][i] < mn) mn = col[a][i];
    }
    *bias[a]  = (mx + mn) / 2.0;
    *scale[a] = (fabs(mx - mn) > 1000) ? (mx - mn) / 2.0 : 16384.0;
  }
}

// ---------------- 存取 NVS ----------------
void saveCalib() {
  prefs.begin(NS, false);
  prefs.putFloat("bax", biasAx);  prefs.putFloat("bay", biasAy);  prefs.putFloat("baz", biasAz);
  prefs.putFloat("sax", scaleAx); prefs.putFloat("say", scaleAy); prefs.putFloat("saz", scaleAz);
  prefs.putFloat("bgx", biasGx);  prefs.putFloat("bgy", biasGy);  prefs.putFloat("bgz", biasGz);
  prefs.putBool ("cal", true);
  for (int i = 0; i < NPOSE; i++) {
    char key[4]; snprintf(key, sizeof(key), "q%d", i);
    prefs.putBytes(key, poseRaw[i], sizeof(poseRaw[i]));
  }
  prefs.putInt("qmask", (1 << NPOSE) - 1);
  prefs.end();
  Serial.println("已存入 flash (NVS: " + String(NS) + ")");
}

bool loadCalib() {
  prefs.begin(NS, true);
  calOK = prefs.getBool("cal", false);
  if (!calOK) { prefs.end(); return false; }
  biasAx  = prefs.getFloat("bax", 0);  biasAy  = prefs.getFloat("bay", 0);  biasAz  = prefs.getFloat("baz", 0);
  scaleAx = prefs.getFloat("sax", 16384); scaleAy = prefs.getFloat("say", 16384); scaleAz = prefs.getFloat("saz", 16384);
  biasGx  = prefs.getFloat("bgx", 0);  biasGy  = prefs.getFloat("bgy", 0);  biasGz  = prefs.getFloat("bgz", 0);
  prefs.end();
  return true;
}

// ---------------- 汇总表 ----------------
void printSummary() {
  Serial.println("\n================ Day39 实验2 六面标定 · 结果 ================");
  Serial.println("标定参数(已存 flash, NVS=day39):");
  Serial.printf("  零偏 bias(LSB)   : bax=%+.1f  bay=%+.1f  baz=%+.1f\n", biasAx, biasAy, biasAz);
  Serial.printf("  标度 scale(LSB/g): sax=%.1f  say=%.1f  saz=%.1f\n", scaleAx, scaleAy, scaleAz);
  Serial.printf("  陀螺零偏(LSB)    : bgx=%+.1f  bgy=%+.1f  bgz=%+.1f\n", biasGx, biasGy, biasGz);
  Serial.printf("  已校准: %s\n", calOK ? "是" : "否");

  Serial.println("\n表1 六面标度验证(去偏+标度后, 期望 合成重力=1.000)");
  Serial.println("| 姿态 | ax | ay | az | 合成重力 | 判定 |");
  Serial.println("|---|---|---|---|---|---|");
  for (int i = 0; i < NPOSE; i++) {
    float ax = (poseRaw[i][0] - biasAx) / scaleAx;
    float ay = (poseRaw[i][1] - biasAy) / scaleAy;
    float az = (poseRaw[i][2] - biasAz) / scaleAz;
    float g  = sqrt(ax * ax + ay * ay + az * az);
    const char* judge = (g > 0.97 && g < 1.03) ? "OK" : "X";
    Serial.printf("| %s | %+.3f | %+.3f | %+.3f | %.3f | %s |\n",
                  POSE_NAME[i], ax, ay, az, g, judge);
  }

  Serial.println("\n表2 六姿态原始均值(LSB) —— 核对每根轴都朝上/朝下过一次、摆得干不干净");
  Serial.println("| 姿态 | ax | ay | az |");
  Serial.println("|---|---|---|---|");
  for (int i = 0; i < NPOSE; i++) {
    Serial.printf("| %s | %+.0f | %+.0f | %+.0f |\n",
                  POSE_NAME[i], poseRaw[i][0], poseRaw[i][1], poseRaw[i][2]);
  }
  Serial.println("=========================================================");
  Serial.println("复制整段发回给 AI 核对标定是否正确。");
}

// ---------------- 主流程 ----------------
void runCalibration() {
  Serial.println("\n开始六面标定: 按提示摆6个方向, 每个保持不动");
  for (int i = 0; i < NPOSE; i++) {
    if (!doPose(i)) {
      Serial.println("\n!! 标定中止: 有姿态一帧都没采到, flash 里的旧值保持原样, 没被覆盖。");
      Serial.println("!! 查好杜邦线, 重新上电, 再发 's' 重标。\n");
      return;
    }
  }
  computeCalib();
  printSummary();
  saveCalib();
}

void printStartHint() {
  Serial.println("\n---------------- 操作说明 ----------------");
  Serial.println("依次摆 6 个方向, 每个保持 8 秒:");
  for (int i = 0; i < NPOSE; i++)
    Serial.printf("  %d. %s\n", i + 1, POSE_NAME[i]);
  Serial.println("建议先备好: 清空桌面、找个直角盒子靠竖放。");
  Serial.println("每个方向会有 3 秒摆位倒计时, 不用急。");
  Serial.println(">>> 准备好后发送 's' 开始标定 <<<");
  Serial.println("标定完发送 'v' 打出结果表, 整段复制发回给 AI 核对。");
  Serial.println("实时显示中(用当前标定值)...\n");
}

void printDone() {
  Serial.println("\n---------------- 六面标定完成 ----------------");
  Serial.println("6 个方向全部采完, 结果表见上。");
  Serial.println("实时显示中(用当前标定值)...");
  Serial.println("发 'v' 再打印结果表, 发 's' 重新标定。\n");
}

void setup() {
  Serial.begin(115200);
  delay(300);
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(400000);   // 400kHz, 同实验1

  Serial.println("\n\n===== Day39 实验2 六面自动标定 =====");
  mpuWrite(REG_PWR_MGMT_1, 0x00);      // 唤醒
  mpuWrite(REG_ACCEL_CONFIG, 0x00);    // ±2g
  mpuWrite(REG_GYRO_CONFIG, 0x00);     // ±250dps
  mpuWrite(REG_CONFIG, 0x03);          // 低通 ~44Hz
  delay(100);

  uint8_t who = whoAmI();
  Serial.printf("I2C SDA=GPIO%d SCL=GPIO%d 400kHz 地址 0x%02X\n", SDA_PIN, SCL_PIN, MPU);
  Serial.printf("WHO_AM_I(0x75)=0x%02X %s\n", who, (who == 0x68) ? "(OK)" : "(异常!检查接线/地址)");
  if (who != 0x68) {
    Serial.println("!! MPU6050 无响应, 请检查 SDA/SCL/AD0 后重启");
    while (1) delay(100);
  }

  if (loadCalib()) {
    Serial.println("\n已从 flash 加载标定值:");
    calOK = true;
    printSummary();
  } else {
    Serial.println("\n尚未标定(flash 无数据)。");
  }

  printStartHint();
}

void loop() {
  if (Serial.available()) {
    char c = Serial.read();
    if (c == 'v' || c == 'V') {
      printSummary();
    } else if (c == 's' || c == 'S' || c == 'c' || c == 'C') {
      Serial.println("\n>>> 收到开始指令, 进入六面标定\n");
      runCalibration();
      printDone();
    }
  }

  int16_t ax, ay, az, gx, gy, gz;
  mpuRead(&ax, &ay, &az, &gx, &gy, &gz);
  float fx = (ax - biasAx) / scaleAx;
  float fy = (ay - biasAy) / scaleAy;
  float fz = (az - biasAz) / scaleAz;
  float g  = sqrt(fx * fx + fy * fy + fz * fz);

  Serial.printf("ax=%+.3f ay=%+.3f az=%+.3f  |g|=%.3f  g=(%+d,%+d,%+d)\n",
                fx, fy, fz, g,
                gx - (int)biasGx, gy - (int)biasGy, gz - (int)biasGz);
  delay(100);
}
