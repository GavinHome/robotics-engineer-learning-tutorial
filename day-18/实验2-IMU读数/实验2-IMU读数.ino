// Day 18 实验 2：MPU-6050 读数 + 倾斜角
//
// 电路（与实验 1 相同，4 根母对母杜邦线，零额外元件）：
//
//   MPU-6050         ESP32-S3
//   ────────         ────────
//   VCC      ──────  3V3
//   GND      ──────  GND（必须共地）
//   SDA      ──────  GPIO8
//   SCL      ──────  GPIO9
//
// 三个要点：
// ① I2C 是「地址 + 寄存器」的访问方式：主机先发设备地址（MPU-6050 = 0x68），
//   再发寄存器地址和读写方向。同一条总线上可以挂多个不同地址的设备。
// ② 加速度计静止时测的是重力矢量。把板子放平，重力全落在 Z 轴 → az ≈ 9.8。
//   倾斜后重力在 X/Y 上出现分量，用 atan2 就能反推出倾角。
// ③ 陀螺仪测角速度（rad/s），积分才是角度。它不受震动影响，但会漂移，
//   静止一会儿角度就跑偏；加速度计反过来——长期准、动态抖。
//   真正的姿态解算要用互补滤波 / 卡尔曼把两者融合，超出本课范围。

#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <RgbCycle.h>
#include <Wire.h>

const int SDA_PIN = 8;
const int SCL_PIN = 9;

Adafruit_MPU6050 mpu;

const unsigned long SAMPLE_MS = 500;   // 采样周期
unsigned long lastSample = 0;

// 倾斜档位：不能叫 OK / LOW / HIGH —— Arduino 核心已把它们定义成宏，会冲突。
enum TiltZone { T_LEVEL, T_TILT, T_STEEP, T_FAIL };
TiltZone lastZone = T_FAIL;

void showZone(TiltZone z) {
  switch (z) {
    case T_LEVEL: RgbCycle::setColor(0, 255, 0);   break;  // 绿：放置平稳
    case T_TILT:  RgbCycle::setColor(255, 160, 0); break;  // 橙：有倾角
    case T_STEEP: RgbCycle::setColor(255, 0, 0);   break;  // 红：陡
    case T_FAIL:  RgbCycle::setColor(0, 0, 255);   break;  // 蓝：传感器异常
  }
}

// 给定倾斜幅度，返回档位 + 文字标签
TiltZone classify(float tiltDeg, const char*& label) {
  if (tiltDeg < 10.0)      { label = "LEVEL"; return T_LEVEL; }
  else if (tiltDeg < 45.0) { label = "TILT";  return T_TILT;  }
  else                     { label = "STEEP"; return T_STEEP; }
}

void setup() {
  Serial.begin(115200);
  delay(200);

  RgbCycle::begin();
  showZone(T_FAIL);

  Wire.begin(SDA_PIN, SCL_PIN);

  // mpu.begin() 内部会读 WHO_AM_I 寄存器（值应为 0x68）来确认芯片在线。
  // 第二个参数传 &Wire，让它用我们上面指定好引脚的那条总线。
  if (!mpu.begin(0x68, &Wire)) {
    Serial.println("MPU6050 not found at 0x68. Check wiring / run the scanner sketch.");
    while (1) delay(100);   // 停在这里，灯保持蓝色
  }

  // 量程：越小分辨率越高，但容易打满。8G / 500°/s 是通用折中。
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  // 片内低通滤波，滤掉机械抖动。21Hz 对应 ~5ms 延迟，够用。
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  Serial.println("MPU6050 ready. SDA=GPIO" + String(SDA_PIN) +
                 " SCL=GPIO" + String(SCL_PIN));
  Serial.println("acc units: m/s^2 | gyro units: rad/s | angle units: deg");
}

void loop() {
  if (millis() - lastSample < SAMPLE_MS) return;   // 非阻塞节流
  lastSample = millis();

  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // ---- 由重力矢量反推倾角 ----
  // roll  绕 X 轴翻滚：用 Y、Z 两个分量求反正切
  // pitch 绕 Y 轴俯仰：用 X 与 YZ 合矢量求反正切（分母取模长才对）
  // atan2 而非 atan：atan2 能覆盖 ±180° 全象限，atan 只有 ±90°。
  float roll  = atan2(a.acceleration.y, a.acceleration.z) * 180.0 / M_PI;
  float pitch = atan2(-a.acceleration.x,
                      sqrt(a.acceleration.y * a.acceleration.y +
                           a.acceleration.z * a.acceleration.z)) * 180.0 / M_PI;

  const char* label;
  TiltZone z = classify(fabs(roll) + fabs(pitch), label);
  if (z != lastZone) {        // 只在档位变化时刷新灯，避免每帧重复写灯珠
    showZone(z);
    lastZone = z;
  }

  // 一行 JSON，延续 Day 13/14/17 的格式，方便 Python 直接解析
  Serial.printf("{\"ax\":%.2f,\"ay\":%.2f,\"az\":%.2f,"
                "\"gx\":%.3f,\"gy\":%.3f,\"gz\":%.3f,"
                "\"roll\":%.1f,\"pitch\":%.1f,\"zone\":\"%s\"}\n",
                a.acceleration.x, a.acceleration.y, a.acceleration.z,
                g.gyro.x, g.gyro.y, g.gyro.z,
                roll, pitch, label);
}
