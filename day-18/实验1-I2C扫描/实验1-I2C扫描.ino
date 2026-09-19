// Day 18 实验 1：I2C 总线扫描
//
// 接任何 I2C 模块之前先跑这个：确认线通不通、地址对不对。
// 这一步能省掉后面 90% 的排查时间。
//
// 电路（MPU-6050 模块）：
//
//   MPU-6050         ESP32-S3
//   ────────         ────────
//   VCC      ──────  3V3（模块自带稳压+LDO，不要把 5V 接到 SDA/SCL 上）
//   GND      ──────  GND（必须共地，否则没有回流路径，总线永远读不到 ACK）
//   SDA      ──────  GPIO8
//   SCL      ──────  GPIO9
//
// ⚠️ 接线顺序：先 GND，最后 VCC。拆线反过来。
//
// ⚠️ 为什么用 GPIO8/9：ESP32-S3 没有固定的 I2C 引脚，任意 GPIO 都能用，
//    靠 Wire.begin(SDA, SCL) 指定。但 0 / 3 / 45 / 46 是 strapping 引脚
//    （上电瞬间要采样电平决定启动模式），外接模块会干扰，避开。

#include <RgbCycle.h>
#include <Wire.h>

const int SDA_PIN = 8;
const int SCL_PIN = 9;

void setup() {
  Serial.begin(115200);
  delay(200);   // 给串口监视器一点时间连上

  RgbCycle::begin();
  RgbCycle::setColor(0, 0, 255);   // 蓝：扫描中

  // ESP32-S3 要显式传引脚；AVR（Uno/Nano）没有这个重载，硬件引脚是固定的。
  Wire.begin(SDA_PIN, SCL_PIN);

  Serial.println("I2C Scanner  SDA=GPIO" + String(SDA_PIN) +
                 "  SCL=GPIO" + String(SCL_PIN));
  Serial.println("Scanning 0x01 - 0x7F ...");
  delay(1000);
}

void loop() {
  int found = 0;

  // I2C 地址是 7 位，有效范围 0x08-0x77；这里扫全量程当体检。
  for (uint8_t addr = 1; addr < 0x80; addr++) {
    // beginTransmission 只把地址放到总线上，不传数据。
    // endTransmission() 返回 0 = 有设备拉低了 SDA 应答（ACK）。
    Wire.beginTransmission(addr);
    uint8_t err = Wire.endTransmission();

    if (err == 0) {
      Serial.printf("  -> found 0x%02X", addr);
      if (addr == 0x68) Serial.print("   = MPU-6050 (AD0=GND)");
      if (addr == 0x69) Serial.print("   = MPU-6050 (AD0=VCC)");
      Serial.println();
      found++;
    }
  }

  if (found == 0) {
    Serial.println("No I2C device found. Check wiring (VCC/GND/SDA/SCL).");
    RgbCycle::setColor(255, 0, 0);      // 红：没扫到
  } else {
    Serial.printf("%d device(s) found.\n", found);
    RgbCycle::setColor(0, 255, 0);      // 绿：扫到了
  }

  Serial.println();
  delay(3000);   // 每 3 秒重扫一次，方便插拔线观察
}
