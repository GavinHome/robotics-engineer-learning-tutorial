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

## 八、实验 2：光敏电阻（LDR）测光照

> 日期：2026-09-14
> 状态：✅ 已上机实测（强光 raw≈4095 ≈ 3.3V；遮光 raw≈0 ≈ 0V）
>
> 接线：**3V3 → 光敏电阻 → GPIO1 → 10kΩ → GND**（光敏电阻在上，10kΩ 在下作下拉）

### 电路原理

光敏电阻（LDR）的特性：

| 光照 | 电阻值 | 说明 |
| --- | --- | --- |
| 强光 | ~1–10kΩ（甚至更低） | 电阻值小 |
| 黑暗 | ~100kΩ–1MΩ | 电阻值大 |

本实验用**电压分压**电路把电阻变化转为电压变化。两个电阻串联，GPIO1 测的是下方 10kΩ 电阻上的电压：

```
3V3 ──[R_LDR]── GPIO1 ──[10kΩ]── GND
          (上拉)                    (下拉)
          ↑ 光敏电阻                 ↑ 固定电阻
```

**核心原理：串联电路中，电阻越大，它两端的电压越大。**

分压公式：`V_GPIO1 = 3.3V × (R_下拉 / (R_LDR + R_下拉))`

#### 强光时：R_LDR 很小 → GPIO1 电压很大

强光照射下，光敏电阻阻值很小（约 500Ω–1kΩ）：

```
V_GPIO1 = 3.3V × (10kΩ / (1kΩ + 10kΩ))
        = 3.3V × (10k / 11k)
        = 3.3V × 0.91
        ≈ 3.00V  →  raw ≈ 3720
```

如果光照极强，R_LDR 只有约 100Ω：

```
V_GPIO1 = 3.3V × (10k / (0.1k + 10k))
        = 3.3V × (10k / 10.1k)
        ≈ 3.27V  →  raw ≈ 4050
```

你的实测 raw ≈ 4095（≈3.30V），说明强光下 R_LDR 极小，10kΩ 几乎分到了全部 3.3V。

**关键理解：** 光敏电阻小 → 它两端的电压小（几乎 0V）→ 3.3V 几乎全部"落在"下方 10kΩ 上 → GPIO1 电压高。

#### 遮光时：R_LDR 很大 → GPIO1 电压很小

遮光（用手捂住）时，光敏电阻阻值极大（约 1MΩ）：

```
V_GPIO1 = 3.3V × (10kΩ / (1000kΩ + 10kΩ))
        = 3.3V × (10k / 1010k)
        = 3.3V × 0.0099
        ≈ 0.033V  →  raw ≈ 40 ≈ 0
```

1MΩ 远大于 10kΩ，所以几乎全部 3.3V 都"落在"光敏电阻上，下方 10kΩ 只分到约 0.033V → GPIO1 电压接近 0V。

**关键理解：** 光敏电阻大 → 它两端的电压大（几乎 3.3V）→ 下方 10kΩ 只分到一点点 → GPIO1 电压低。

#### 一句话总结

> **光敏电阻小（强光）→ 它"吃掉"的电压少 → 10kΩ 分到的高电压出现在 GPIO1 → raw 大**
> **光敏电阻大（遮光）→ 它"吃掉"的电压多 → 10kΩ 只分到一点点 → GPIO1 电压低 → raw 小**

这就是为什么强光时 raw ≈ 4095，遮光时 raw ≈ 0。

### 接线

| 器件 | 接线 | 说明 |
| --- | --- | --- |
| 光敏电阻 | 一脚接 3V3，另一脚接 GPIO1 | 上拉端 |
| 10kΩ 电阻 | GPIO1 → 10kΩ → GND | 下拉电阻，与光敏电阻组成分压 |
| 板载彩灯 | GPIO48 | 保持 Day 9 接法，`RgbCycle::update()` 照常运行 |

> 注意：本实验**复用 GPIO1**（与实验 1 电位器同一引脚，但不同电路）。实验 1 和实验 2 不需要同时运行。

### 代码

完整代码：[`实验2-光敏电阻测光照.ino`](./实验2-光敏电阻测光照/实验2-光敏电阻测光照.ino)

```cpp
#include <RgbCycle.h>

const int ADC_PIN = 1;   // GPIO1 = ADC1_CH0

void setup() {
  RgbCycle::begin();               // 彩灯：初始化
  RgbCycle::setInterval(800);      // 彩灯：800ms/色

  Serial.begin(115200);
  analogReadResolution(12);        // 12 位 ADC
  analogSetAttenuation(ADC_11db);  // 0-3.3V 全量程
}

void loop() {
  RgbCycle::update();              // 任务 A：彩灯照常循环

  int raw = analogRead(ADC_PIN);
  float voltage = raw * 3.3 / 4095.0;
  Serial.printf("Raw: %4d  Voltage: %.2fV\n", raw, voltage);

  delay(500);
}
```

### 运行结果

- **强光照射**：Raw ≈ 4095，Voltage ≈ 3.30V
- **用手遮住光敏电阻**：Raw ≈ 0，Voltage ≈ 0.00V
- **中间值**：用手半遮，raw 在几百到几千之间变化

### 出了什么问题 & 如何修复

- **读数不随光照变化**：光敏电阻接反（一端接 GND 另一端接 GPIO1，导致逻辑反转）。修复：光敏电阻一端必须接 3V3，另一端接 GPIO1。
- **读数跳变很大**：10kΩ 电阻值不标准，或光敏电阻引线过长引入噪声。修复：使用 1% 精度的电阻，引线尽量短。
- **raw 值始终接近 2048**：光敏电阻两端接反了，或分压点接错了。修复：检查电路连接。

---
## 九、实验 3：Serial Plotter 可视化

> 日期：2026-09-14
> 状态：✅ 已上机实测（串口绘图器可看到电压曲线随电位器旋转连续变化）
>
> 接线：与**实验 1 完全相同**（GPIO1 → 电位器中间脚，两侧接 3V3 和 GND）

### 目的

实验 1 和实验 2 都在串口监视器里打印数字。实验 3 换一种输出方式——只打印**单个浮点数**，让 Arduino IDE 的**串口绘图器**把电压变化画成实时曲线。

### 电路

与实验 1 完全相同：

| 器件 | 接线 |
| --- | --- |
| 电位器 | 3V3 → 左侧脚，GND → 右侧脚，GPIO1 → 中间脚 |
| 板载彩灯 | GPIO48，照常运行 |

### 代码

完整代码：[`实验3-Serial-Plotter可视化.ino`](./实验3-Serial-Plotter可视化/实验3-Serial-Plotter可视化.ino)

```cpp
#include <RgbCycle.h>

const int ADC_PIN = 1;   // GPIO1 = ADC1_CH0

void setup() {
  RgbCycle::begin();
  RgbCycle::setInterval(800);

  Serial.begin(115200);
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);  // 量程 0-3.3V
}

void loop() {
  RgbCycle::update();

  int raw = analogRead(ADC_PIN);
  float voltage = raw * 3.3 / 4095.0;
  Serial.println(voltage);       // 只打印电压值，方便绘图器读取
  delay(500);
}
```

### 与实验 1 代码的区别

| | 实验 1 | 实验 3 |
| --- | --- | --- |
| 输出格式 | `Serial.printf("Raw: %4d Voltage: %.2fV\n", raw, voltage)` | `Serial.println(voltage)` |
| 串口绘图器 | ❌ 不兼容（有文字标签） | ✅ 兼容（只有纯数字） |
| 用途 | 调试、看具体数值 | 观察电压变化趋势 |

### 运行方法

1. 上传代码，打开串口监视器确认有输出
2. 关闭串口监视器（绘图器和监视器不能同时开）
3. Arduino IDE → 工具 → **串口绘图器**
4. 旋转电位器，可以看到电压曲线实时上下移动

### 运行结果

- 电位器逆时针到底：曲线在底部（0V）
- 电位器顺时针到底：曲线在顶部（3.3V）
- 旋转过程中曲线连续平滑变化

### 出了什么问题 & 如何修复

- **绘图器只看到一条直线**：输出格式有文字标签（如 `Raw: 123 Voltage: 1.23V`），绘图器无法解析。修复：只输出纯数字，用 `Serial.println(voltage)`。
- **绘图器曲线跳变很大**：波特率不匹配，或 ADC 噪声。修复：确认 `Serial.begin(115200)` 与绘图器设置一致。
- **绘图器不显示**：串口监视器未关闭。修复：关闭监视器再打开绘图器。

---
## 十、下一步

- Day 13：PWM 进阶（舵机控制）
- 进阶：多次采样取平均，降低 ADC 噪声
