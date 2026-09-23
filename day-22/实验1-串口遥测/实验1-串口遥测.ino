// Day 22 实验 1：向 Python 喂数据的遥测草图
//
// 目的：给 Day 22 的 Python 脚本（存 CSV）和 Day 26 的实时绘图提供真实数据源。
//
// 输出格式是本节的重点。前 21 天串口打的中文人类可读日志（如"前方 25.9cm，通畅"）
// 对机器来说是字符串，Python 得写正则去抠数字，脆弱又难维护。
// 改成 JSON 一行一条：Python 端 json.loads() 直接成字典，字段名自解释。
//
// 每行形如：
//   {"ms":12345,"dist_cm":25.9,"adc_raw":2048,"voltage":1.65}
//
// 关键取舍：
// ① 只发 JSON，不发任何说明文字。Python 端靠"行首是 {" 来区分数据行与噪声行，
//    上电时 ESP32 ROM 会先吐一串启动日志（"ESP-ROM:esp32s3-..."），正是靠这个过滤掉的。
// ② dist 超时给 -1，不是 0。0 会被当成"贴脸"，画进图里是一根扎到 0 的假尖刺。
// ③ 用固定周期（100ms）而不是越快越好：串口 115200 波特率下每行 50 字节，
//    10Hz 只占带宽的 0.4%，但超过 50Hz 就开始丢字符，JSON 会解析失败。
//
// 电路：
//   HC-SR04  TRIG=GPIO4  ECHO=GPIO5（沿用 Day 21）
//   电位器/光敏  中间抽头 → GPIO1（ADC1_CH0，Wi-Fi 不冲突）
//   板载 RGB    GPIO48 — 本实验做心跳灯，update() 每圈调用

#include <RgbCycle.h>

const int TRIG_PIN = 4;
const int ECHO_PIN = 5;
const int ADC_PIN  = 1;      // ADC1_CH0。Day 27 电池监测也走 ADC1，ADC2 与 Wi-Fi 冲突

const unsigned long TIMEOUT_US  = 30000;   // 30ms ≈ 5m 量程上限
const float         CM_PER_US   = 0.0343 / 2;
const unsigned long SAMPLE_MS   = 100;     // 10Hz
const int           ADC_MAX     = 4095;    // ESP32-S3 12 位 ADC
const float         VREF        = 3.3;

unsigned long lastSample = 0;

float measureCm() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  unsigned long us = pulseIn(ECHO_PIN, HIGH, TIMEOUT_US);
  if (us == 0) return -1.0;     // 超时/超出量程：-1，不是 0
  return us * CM_PER_US;
}

void setup() {
  Serial.begin(115200);
  delay(200);

  RgbCycle::begin();            // 心跳灯：begin() + loop 里 update()

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  digitalWrite(TRIG_PIN, LOW);

  analogReadResolution(12);     // 默认就是 12 位，显式写清楚，换板子不会踩坑
  analogSetAttenuation(ADC_11db);  // 量程拉到约 0~3.3V，否则超过 1.1V 就顶到 4095

  // 注意：这里不打印任何人类可读的横幅。
  // Python 端只认 { 开头的行，多打一行中文就要多写一个过滤分支。
}

void loop() {
  RgbCycle::update();           // 心跳灯，不用 delay，不拖慢采样

  if (millis() - lastSample < SAMPLE_MS) return;
  lastSample = millis();

  float cm  = measureCm();
  int   raw = analogRead(ADC_PIN);
  float v   = raw * VREF / ADC_MAX;

  // 一行 JSON，字段顺序固定，Python 端 csv.DictWriter 直接用
  Serial.printf("{\"ms\":%lu,\"dist_cm\":%.1f,\"adc_raw\":%d,\"voltage\":%.2f}\n",
                millis(), cm, raw, v);
}
