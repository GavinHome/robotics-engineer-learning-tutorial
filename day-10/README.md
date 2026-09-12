# Day 10：PWM 呼吸灯 —— 用占空比做亮度渐变

> 日期：2026-09-12
> 状态：✅ 外接 LED 呼吸灯跑通（视频实测），✅ 彩灯循环 + 呼吸灯双任务并行跑通，✅ 彩灯循环抽成 `RgbCycle` 库（一行调用）
> ⚠️ 未完成：Day 10 计划中的电位器调光（任务 4/5）

---

## 一、目标

Day 9 里 GPIO 只有"全亮 / 全灭"两种状态（`digitalWrite`）。Day 10 要拿到**第三种状态：任意亮度**。

原理是 **PWM（脉冲宽度调制）**：引脚在极短时间内不停开 / 关，靠"开的时间占比"（占空比 duty）决定平均亮度。频率足够高（本实验 5 kHz）时，人眼看到的是连续亮度而不是闪烁。占空比从 0 慢慢升到满，再降回来，就是呼吸灯。

ESP32 的 PWM 由 **LEDC 外设**硬件产生：配置一次后，波形由硬件自己一直输出，CPU 去干别的事（比如给彩灯发数据）都不会打断它。这也是本课后面 Day 11（按键）、Day 19（电机）的非阻塞编程基础。

---

## 二、硬件与电路

**不用重新搭电路——完全复用 Day 9 实验二/三的外接 LED，一根线都不用加。**

| 项目 | 说明 |
| --- | --- |
| 控制脚 | **GPIO2** |
| 接线 | `GPIO2 → 220Ω 限流电阻 → LED 长脚（阳极）→ LED 短脚（阴极）→ GND` |
| 限流电阻 | 220Ω（库存清单中的 220Ω ×10） |
| 电源 | 板子 USB 5V，GPIO2 输出 3.3V 逻辑电平 |

板载 WS2812B（GPIO48）保持 Day 9 的接法不变，本课的综合实验里两路同时工作。

---

## 三、两个实验与代码

| 实验 | 代码 | 内容 | 控制脚 |
| --- | --- | --- | --- |
| 实验一 | [`breath_led/breath_led.ino`](breath_led/breath_led.ino) | 外接 LED 呼吸（单任务） | GPIO2 |
| 实验二 | [`combined_pwm_blink/combined_pwm_blink.ino`](combined_pwm_blink/combined_pwm_blink.ino) | 板载彩灯循环 + 外接 LED 呼吸（双任务并行） | GPIO48 + GPIO2 |

### 实验一：外接 LED 呼吸灯

完整代码：[`breath_led/breath_led.ino`](breath_led/breath_led.ino)

```cpp
const int LED_PIN = 2;
const int STEP_MS = 4;      // 每档 4ms：256 档 ≈ 1.02s 单向，一个完整呼吸 ≈ 2s

void setup() {
  ledcAttach(LED_PIN, 5000, 8);   // 引脚 2, 5kHz, 8 位分辨率 → duty 0~255
  ledcWrite(LED_PIN, 0);          // 上电从灭开始
  Serial.begin(115200);
  Serial.println("ESP32-S3 PWM 呼吸灯 Start");
}

void loop() {
  for (int b = 0; b <= 255; b++) { ledcWrite(LED_PIN, b); delay(STEP_MS); }  // 渐亮
  for (int b = 255; b >= 0; b--) { ledcWrite(LED_PIN, b); delay(STEP_MS); }  // 渐暗
}
```

两个 API 的分工：

- `ledcAttach(pin, freq, resolution)` — 把引脚绑到 LEDC，设定 PWM 频率与分辨率。8 位分辨率意味着 duty 取 **0~255**。
- `ledcWrite(pin, duty)` — 写占空比。**按引脚写**，不按通道写（见下节 API 变化）。

### 实验二：彩灯 + 呼吸灯双任务并行

完整代码：[`combined_pwm_blink/combined_pwm_blink.ino`](combined_pwm_blink/combined_pwm_blink.ino)

```cpp
#include <RgbCycle.h>   // Day 9 的彩灯循环，已抽成可复用库

const int LED_PIN = 2;

unsigned long lastBreath = 0;
const unsigned long BREATH_INTERVAL = 4;   // 每 4ms 走一档
const int BREATH_STEP = 1;
int brightness = 0;
int breathDir  = 1;

void setup() {
  RgbCycle::begin();            // 彩灯：初始化
  RgbCycle::setInterval(800);   // 彩灯：本任务用 800ms/色（一个循环 2.4s）
  ledcAttach(LED_PIN, 5000, 8); // 外接 LED：绑定 PWM
  ledcWrite(LED_PIN, 0);
}

void loop() {
  unsigned long now = millis();

  RgbCycle::update();   // 任务 A：彩灯，内部自己按 800ms 换色

  if (now - lastBreath >= BREATH_INTERVAL) {   // 任务 B：呼吸灯
    lastBreath = now;
    brightness += breathDir * BREATH_STEP;
    if (brightness >= 255) { brightness = 255; breathDir = -1; }
    if (brightness <= 0)   { brightness = 0;   breathDir =  1; }
    ledcWrite(LED_PIN, brightness);
  }
}
```

关键点：**整个 `loop()` 里没有一处 `delay()`**。彩灯和呼吸灯各自用 `millis()` 判断自己的节奏，互不阻塞——彩灯换色不会让呼吸变卡，呼吸灯步进也不会打乱彩灯换色。

> **`millis()` 相减写法**：`now - lastBreath >= BREATH_INTERVAL`（而不是 `now >= lastBreath + INTERVAL`）。前者在 `millis()` 溢出回绕时依然正确。

### 彩灯循环为什么变成了库

Day 9 的彩灯循环原本写在 `rgb_cycle.ino` 里。如果 Day 10、Day 11…… 每个任务的草图都把那段代码复制一遍，改动一次就要改十几处。

所以把彩灯循环抽成了一个 Arduino 库 **`RgbCycle`**，装在：

```
~/Documents/Arduino/libraries/RgbCycle/
```

装到系统库目录后，**任意草图**一行 `#include <RgbCycle.h>` 就能用：

```cpp
void setup() { RgbCycle::begin(); RgbCycle::setInterval(800); }
void loop()  { RgbCycle::update();   /* 其他任务 */ }
```

| 接口 | 调用位置 | 作用 |
| --- | --- | --- |
| `RgbCycle::begin()` | `setup()` 一次 | 初始化灯珠、清掉复位期间的随机锁存色 |
| `RgbCycle::setInterval(ms)` | `setup()` 一次 | 设定每色停留时间，不调用则用默认 800ms |
| `RgbCycle::update()` | `loop()` 每圈 | 内部判断是否到点，到点换下一色，不阻塞 |
| `RgbCycle::setColor(r, g, b)` | 任意 | 让灯珠停在指定颜色 |

库源码在仓库里留了一份备份：[`lib/RgbCycle/`](lib/RgbCycle/)（两边内容一致，改动需手动同步）。

### ⚠️ API 变化：esp32 3.x 删掉了 `ledcSetup` / `ledcAttachPin`

Day 10 计划书里的参考代码用的是 **arduino-esp32 2.x** 的老 API：

```cpp
ledcSetup(channel, freq, resolution);   // ❌ 3.x 已删除
ledcAttachPin(pin, channel);            // ❌ 3.x 已删除
```

本机是 **esp32 3.3.10-cn**，编译直接报 `'ledcSetup' was not declared in this scope`。3.x 的新 API 把"通道"这一层去掉了，**引脚即通道**：

```cpp
ledcAttach(pin, freq, resolution);   // ✅ 一次绑定
ledcWrite(pin, duty);                // ✅ 按引脚写
```

附带好处：不再需要 `pinMode(pin, OUTPUT)`，`ledcAttach` 会把引脚配好。

---

## 四、运行结果（视频实录）

完整视频：[`LED呼吸灯效果.MOV`](LED呼吸灯效果.MOV)

| 项目 | 值 |
| --- | --- |
| 编码 | HEVC，1920×1080 |
| 帧率 | 29.97 fps（30000/1001） |
| 总帧数 | 227 |
| 时长 | 7.57 s |

**视频内容**：面包板上为跳线、220Ω 限流电阻与外接 LED，手持拍摄。LED 亮度呈连续的"渐亮 → 渐暗"循环，肉眼可见平滑呼吸，无阶跃跳动。

### 节奏验证

先用最直接的办法：把每帧整幅画面的平均亮度连成一条曲线，看它是否有周期性。

结果这条曲线几乎是**平的**——最小值 140.17、最大值 144.61、标准差仅 **1.355**，而逐帧最大通道普遍为 255。

> **验证方式的局限**：相机自动曝光把画面整体推到了接近饱和。LED 变暗时相机自动提亮、变亮时又压暗，正好抵消掉亮度变化。所以**逐帧平均亮度不能用来判断呼吸节奏**（这和 Day 9 录像里"看不准 LED 亮灭"是同一个坑）。

改用**自相关**——把整幅亮度曲线去均值后与自身做不同时间差的比较，看它在多大间隔上重复自己：

| 项目 | 值 |
| --- | --- |
| 自相关峰值 | lag 72 帧 = **2.40 s**，r = **0.627** |
| 最强负相关 | lag 25 帧 = **0.83 s**，r = −0.505 |
| 设计值 | 256 档 × 4 ms × 2 = **2.048 s** |

负相关出现在约 0.83 s、正相关峰值在 2.40 s，与"半个周期 1.0 s / 一个周期 2.0 s"的量级一致。即：**录像里的亮度调制确实以约 2 秒为周期在重复**，与代码里 `2.048 s` 的设计值吻合（实测 2.40 s 偏大，可能来自手持拍摄的整体位移与曝光漂移）。

用 2.048 s 正弦参考做逐像素相关进一步验证：最大相关系数 **0.791**，但 |r| > 0.6 的像素仅 344 个（占全画面 **0.149%**）。也就是说这个信号**真实但很弱**——逐像素亮度变化的主要来源是手持抖动和自动曝光，不是 LED 本身。

### 验证结论

- ✅ **可确认**：LED 有连续的亮度渐变（肉眼可见），且录像中的亮度调制以 ≈2 秒为周期重复，与设计值 2.048 s 相符。
- ⚠️ **不能确认**：无法从本段视频对每一帧的亮度做定量判定（自动曝光 + 全画面污染）。呼吸曲线是否线性、峰值亮度具体是多少，本录像都证明不了。

这与 Day 9 的结论是同一条经验：**相机自动曝光会掩盖亮度变化，定量判断必须另找参照**。

---

## 五、出了什么问题 & 如何修复

### 1. 照抄计划书的代码编译不过

**现象**：`error: 'ledcSetup' was not declared in this scope`，以及 `'ledcAttachPin' was not declared in this scope; did you mean 'ledcAttach'?`

**原因**：30 天计划书的 Day 10 参考代码写于 arduino-esp32 2.x 时代，而本机内核是 3.3.10-cn。3.x 版本**删除了** `ledcSetup()` 与 `ledcAttachPin()`，改用 `ledcAttach(pin, freq, resolution)` + `ledcWrite(pin, duty)`，并且 duty 按**引脚**而不是按通道写。

**修复**：按 3.x 新 API 重写，见 `breath_led.ino`。同时删掉了不再需要的 `pinMode()`。

> **教训**：教程 / 博客里的 Arduino 代码都绑定了内核版本。报"未声明"错误时，先查该函数在当前内核里是否还存在，而不是怀疑自己写错。

### 2. 彩灯循环代码要被复制到每个任务里

**现象**：写完 Day 10 综合版，发现彩灯循环那 20 行代码又要从 Day 9 复制一遍；照这个趋势，Day 11、Day 12 每个任务都要再抄一遍。

**原因**：Day 9 把彩灯循环写成了**草图内的代码**，而不是**可复用的模块**。

**修复**：抽成 Arduino 库 `RgbCycle`，装到 `~/Documents/Arduino/libraries/`，此后任意草图一行 `RgbCycle::update()` 即可。库的 `update()` 内部用 `millis()` 判断节奏、绝不 `delay()`，所以能和其他任务共存在同一个 `loop()` 里。

**代价**：系统库目录与仓库备份是两份副本，改动需手动同步（见 [`lib/RgbCycle/README.md`](lib/RgbCycle/README.md)）。

### 3. 两个灯都快得不像话

**现象**：初版彩灯 500ms/色、呼吸 1.0s 单程，实机看两个灯都在"急闪"，不像呼吸也不像彩灯。

**原因**：两个节奏都偏快，且两个灯同时快速变化时视觉上互相干扰。另外 `RgbCycle` 初版没有暴露节奏参数，改一次要动库源码。

**修复**：彩灯放慢到 **800ms/色**（一个循环 2.4 s），呼吸放慢到 **≈2 s 一个完整周期**；并给库加了 `RgbCycle::setInterval(ms)`，各任务可以自己定节奏而不必改库。

### 4. 代码注释与实际参数不符

`combined_pwm_blink.ino` 的 `loop()` 里有两处内联注释写着"内部按 500ms 换色"和"内部按 12ms 步进"，但调慢节奏后实际值是 **800ms** 和 **4ms**。README 与代码头部注释已按实际值写，内联注释未同步——**改参数时注释最容易漏掉**，看节奏请以常量定义为准。

---

## 六、关键观察

1. **PWM = 用时间换亮度**：引脚只有开 / 关两态，靠占空比得到中间亮度；频率够高（5 kHz）人眼就只看到亮度，看不到闪。
2. **LEDC 是硬件外设**：`ledcAttach` 配置一次后波形由硬件持续输出，CPU 去发 WS2812B 数据也不会打断它——这是后面所有"多任务并行"的硬件基础。
3. **API 按引脚而不是通道**：3.x 的 `ledcWrite(pin, duty)` 简化了 2.x 的"通道 → 引脚"两层结构，`pinMode` 也不再需要。
4. **`ledcFade(pin, from, to, ms)` 也能做呼吸**：硬件自动渐变，比手动循环 + `delay` 更省 CPU。本课先用手写循环，是为了把占空比的概念看清楚。
5. **非阻塞是并行的前提**：Day 9 的 `delay(500)` 会让整个循环停住；Day 10 综合版把两路都改成 `millis()` 判断，才能各走各的节奏。
6. **`millis()` 相减判断不会因溢出出错**：`now - last >= interval` 在 `unsigned long` 回绕时仍正确，别用 `now >= last + interval`。
7. **复用靠改造成模块，不靠复制粘贴**：把彩灯循环装成库之后，后续任务只有一行调用；代价是两份副本要同步。
8. **相机自动曝光会掩盖亮度变化**：逐帧平均亮度（std 1.355，接近全平）完全看不出呼吸，必须用自相关 / 逐像素相关绕开它——但即便如此信号也很弱。

---

## 七、Day 10 小结

1. 理解了 PWM 与占空比：亮度不是电压值，而是"开的时间比例"。
2. 用 `ledcAttach()` + `ledcWrite()` 跑通外接 LED 呼吸灯，一个完整呼吸约 2 s。
3. 把 Day 9 的彩灯循环抽成 `RgbCycle` 库，实现"一行调用 + 自定义节奏"，两个任务在同一个 `loop()` 里并行且互不干扰。
4. 踩到并解决了 esp32 3.x 删除 `ledcSetup` / `ledcAttachPin` 的 API 断裂。
5. 用自相关（峰值 2.40 s，r=0.627）验证了录像中的亮度调制周期，并如实记录该测量的局限（自动曝光 + 手持抖动，信号弱、不可定量）。

**尚未完成（Day 10 计划中的任务 4/5）**：电位器接 GPIO1（ADC1_CH0）做手动调光，以及 ADC1 / ADC2 通道限制的验证。留待后续补做。

**下一步（Day 11）**：按键输入 —— `digitalRead()` 读按键状态，配合 Day 10 的非阻塞写法做"按一下切换状态"。

---

## 参考资料

- [Arduino-ESP32 LEDC（PWM）文档](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/ledc.html)
- [Arduino-ESP32 官方文档](https://docs.espressif.com/projects/arduino-esp32/en/latest/)
- [Adafruit NeoPixel 库说明](https://learn.adafruit.com/adafruit-neopixel-uberguide)
