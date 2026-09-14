# Day 12：ADC 与传感器读数

> 日期：2026-09-14
> 状态：✅ 已上机实测（电位器 raw=0→4095，对应电压 0V→3.3V；raw=2048 ≈ 1.65V）
>
> 接线：**GPIO1（ADC1_CH0）→ 电位器中间脚**，两侧脚分别接 3V3 和 GND
> 代码约定：沿用 Day 10 的并行 loop（`RgbCycle::update()` + ADC 读取）

---

## 一、目标

Day 11 读的是**数字信号**（HIGH/LOW）。Day 12 反过来——读**模拟信号**：用 ADC（Analog-to-Digital Converter）把连续电压变成数字值。

核心知识点：
- **12 位 ADC**：ESP32-S3 的 ADC 是 12 位，输出 0–4095
- **电压计算**：`V = raw × 3.3 / 4095`
- **ADC1 vs ADC2**：GPIO1–10 属于 ADC1（推荐），GPIO11–20 属于 ADC2（与 WiFi 冲突）
- **attenuation**：`ADC_11db` 让 ADC 能测 0–3.3V 全量程
- **Serial Plotter**：串口绘图器可视化实时数据

---

## 二、硬件与电路

| 器件 | 接线 | 说明 |
| --- | --- | --- |
| 电位器 | **3V3 → 左侧脚**，**GND → 右侧脚**，**GPIO1 → 中间脚** | 三脚电位器做分压器，中间脚输出 0–3.3V |
| 外接 LED | GPIO2 → 2kΩ → LED → GND | 复用 Day 11 外接 LED |
| 板载彩灯 | GPIO48 | 保持 Day 9 接法，`RgbCycle::update()` 照常运行 |

电位器原理：左右两侧固定接 3V3 和 GND，旋转中间脚改变分压比：
- 逆时针到底：中间脚 ≈ GND → 0V
- 顺时针到底：中间脚 ≈ 3V3 → 3.3V
- 中间位置：约 1.65V

---

## 三、代码

完整代码：[`实验1-电位器测电压.ino`](./实验1-电位器测电压/实验1-电位器测电压.ino)

```cpp
#include <RgbCycle.h>

const int ADC_PIN = 1;   // GPIO1 = ADC1_CH0

void setup() {
  RgbCycle::begin();               // 彩灯：初始化
  RgbCycle::setInterval(800);      // 彩灯：800ms/色

  Serial.begin(115200);
  analogReadResolution(12);        // 设置 ADC 为 12 位（0-4095）
  analogSetAttenuation(ADC_11db);  // 设置衰减，可测 0-3.3V
}

void loop() {
  RgbCycle::update();              // 任务 A：彩灯照常循环

  int raw = analogRead(ADC_PIN);
  float voltage = raw * 3.3 / 4095.0;
  Serial.printf("Raw: %4d  Voltage: %.2fV\n", raw, voltage);

  delay(500);
}
```

---

## 四、`analogRead()` 详解

| 函数 | 作用 |
| --- | --- |
| `analogReadResolution(12)` | 设置 ADC 精度为 12 位，返回范围 0–4095 |
| `analogSetAttenuation(ADC_11db)` | 设置输入衰减，让 ADC 能测量 0–3.3V（默认只能测约 0–1.1V） |
| `analogRead(pin)` | 读取指定 ADC 引脚的原始值（0–4095） |

### ADC1 vs ADC2

| ADC 组 | 可用引脚 | 说明 |
| --- | --- | --- |
| **ADC1** | GPIO1–10 | 稳定，推荐初学使用 |
| **ADC2** | GPIO11–20 | 与 WiFi 共用，WiFi 启用时读数不准，初学阶段**不要用** |

---

## 五、电压计算

ESP32-S3 的 ADC 输出是**数字量**（0–4095），需要转换成实际电压：

```
电压(V) = 原始值 × 参考电压 / 最大值
电压(V) = raw × 3.3 / 4095
```

实测验证：
- raw = 0 → 0.00V（电位器逆时针到底）
- raw = 2048 → 1.65V（电位器中间位置）
- raw = 4095 → 3.30V（电位器顺时针到底）

---

## 六、串口绘图器（Serial Plotter）

打开 Arduino IDE → 工具 → 串口绘图器，可以看到电压曲线随电位器旋转实时变化。

---

## 七、出了什么问题 & 如何修复

- **读数始终为 0 或 4095**：电位器接错（中间脚没接 GPIO），或接线松动。修复：检查三线是否接对。
- **读数跳变很大**：ADC 噪声大，未加衰减。修复：确认 `analogSetAttenuation(ADC_11db)` 已调用。
- **WiFi 启用后 ADC 不准**：用了 ADC2 引脚。修复：改用 ADC1（GPIO1–10）。

---

## 八、下一步

- 实验 2：光敏电阻（LDR）分压电路，读取光照强度
- 进阶：多次采样取平均，降低 ADC 噪声
