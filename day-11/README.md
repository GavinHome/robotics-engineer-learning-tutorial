# Day 11：数字输入与按键

> 日期：2026-09-14
> 状态：✅ 已上机实测（按键按下电压 0V，松开电压 3.3V；LED 按按键亮灭同步）
>
> 接线：**GPIO1 → 2kΩ 电阻 → 按键 → GND**，外接绿 LED 由 GPIO2 控制
> 代码约定：本课起沿用 Day 10 的并行 loop（`RgbCycle::update()` + 按键逻辑）

---

## 一、目标

Day 9/10 都是 GPIO **输出**（写电平、PWM）。Day 11 反过来——读 GPIO **输入**：用一个按键控制 LED 亮灭。

核心知识点：
- **INPUT_PULLUP**：用内部上拉电阻，省掉外部电阻
- **消抖**：按键按下瞬间触点弹跳，需延时 20ms 过滤
- **边沿检测**：区分"按下"（FALLING）和"松开"（RISING）

---

## 二、硬件与电路

| 器件 | 接线 | 说明 |
| --- | --- | --- |
| 按键 | **GPIO1 → 按键 → GND** | 直接接按键，无额外电阻（实测按键按下电压 0V，松开电压 3.3V） |
| 外接 LED | **GPIO2 → 220Ω → LED 长脚 → LED 短脚 → GND** | 复用 Day 9 的外接 LED |
| 板载彩灯 | GPIO48 | 保持 Day 9 接法，代码中 `RgbCycle::update()` 照常运行 |

万用表实测：

| 按键状态 | GPIO1 电压 | 说明 |
| --- | --- | --- |
| 松开 | 3.3V | 内部上拉电阻把引脚拉高 |
| 按下 | 0V | 按键导通，引脚被拉低到 GND |

---

## 三、代码

完整代码：[`day11.ino`](./day11.ino)

```cpp
#include <RgbCycle.h>

const int BUTTON_PIN = 1;   // GPIO1 = 按键输入
const int LED_PIN    = 2;   // GPIO2 = 外接 LED 输出

void setup() {
  RgbCycle::begin();            // 彩灯：初始化
  RgbCycle::setInterval(800);   // 彩灯：800ms/色

  pinMode(BUTTON_PIN, INPUT_PULLUP);  // 启用内部上拉，按键按下 = LOW
  pinMode(LED_PIN,    OUTPUT);         // LED 引脚设为输出
  Serial.begin(115200);
}

void loop() {
  RgbCycle::update();   // 任务 A：彩灯照常循环

  int buttonState = digitalRead(BUTTON_PIN);
  if (buttonState == LOW) {          // 按键按下（被拉低）
    digitalWrite(LED_PIN, HIGH);     // LED 亮
    Serial.println("Button PRESSED");
    delay(200);                      // 简单消抖：按住期间跳过 200ms
  } else {                           // 按键松开（被上拉回 HIGH）
    digitalWrite(LED_PIN, LOW);      // LED 灭
  }
  delay(10);
}
```

---

## 四、`pinMode()` 详解

这是新手最常问的问题：**`setup()` 里写了 `pinMode()`，为什么 `loop()` 里还要写 `digitalWrite()`？**

`pinMode()` 和 `digitalWrite()` 干的是**完全不同的事**：

| 函数 | 作用 | 类比 |
| --- | --- | --- |
| `pinMode(pin, INPUT_PULLUP)` | **配置引脚的工作模式**——"这个脚是输入还是输出？如果是输入，要不要内部上拉？" | 给门贴标签："这是入口"还是"这是出口" |
| `digitalWrite(pin, HIGH/LOW)` | **向输出引脚写入电平**——"如果是输出脚，给它 HIGH 还是 LOW？" | 按开关："开"还是"关" |
| `digitalRead(pin)` | **从输入引脚读取电平**——"这个输入脚现在是 HIGH 还是 LOW？" | 看指示灯："亮"还是"灭" |

**一句话**：`pinMode()` 只执行一次（`setup()`），决定引脚的"身份"；`digitalWrite()` / `digitalRead()` 在 `loop()` 里反复执行，实际读写电平。

### 三种模式

| 模式 | 引脚行为 | 典型用途 |
| --- | --- | --- |
| `OUTPUT` | 你可以用 `digitalWrite()` 主动驱动它为 HIGH 或 LOW | 点亮 LED、控制继电器 |
| `INPUT` | 引脚高阻抗，既不拉高也不拉低——**必须接外部上拉/下拉电阻**，否则悬空时会乱跳 | 读取传感器、按键（外部有上下拉时） |
| `INPUT_PULLUP` | 内部启用一个 ~40–50kΩ 的上拉电阻，默认把引脚拉到 HIGH；外部接地时变为 LOW | **按键/开关的标准接法**（省掉外部电阻） |

### 为什么按键用 `INPUT_PULLUP`？

```
内部上拉 40kΩ
    |
GPIO1 ----[按键]---- GND
```

- 按键**断开**时：开关断开，GPIO1 只连着内部上拉电阻，没有电流回路，引脚被拉到 3.3V → `digitalRead()` 读到 **HIGH**
- 按键**闭合**时：开关闭合，GPIO1 通过导线直接连 GND，内部上拉电阻被"短路"，引脚电压 ≈ 0V → `digitalRead()` 读到 **LOW**

这就是"**低电平有效**"——按键按下是 LOW，松开是 HIGH。代码里 `if (buttonState == LOW)` 就是"按键被按下"。

**关键点**：40kΩ 电阻在 ESP32 芯片内部，不在按键内部。按键就是一个纯导线开关，按下时两个触点直接导通（电阻约 0.1Ω），相当于用导线"跳过"了上拉电阻，把 GPIO1 直接拉到 GND，所以电压为 0V。

### 为什么不用 `INPUT` + 外部上拉？

可以，但多一个电阻就多一件事。`INPUT_PULLUP` 用芯片内部电阻，代码一行搞定，足够用于按键/开关这类慢速输入。

---

## 五、按键消抖

机械按键按下瞬间，金属触点会**快速弹跳**几次（微秒级），导致 `digitalRead()` 在一段时间内来回跳变：

```
按下 → HIGH→LOW→HIGH→LOW→HIGH → 稳定 LOW
```

软件消抖最简单：读到边沿后延时 20ms，让弹跳平息再读取。

本代码用的**超简单版**（适合入门）：

```cpp
if (buttonState == LOW) {
  digitalWrite(LED_PIN, HIGH);
  Serial.println("Button PRESSED");
  delay(200);  // 按住期间 200ms 只处理一次
}
```

进阶版（无阻塞，适合多任务）：记录上次稳定状态，只有**连续多次**读到相反值才认定真变。

---

## 六、运行结果

- **按键松开**：GPIO1 = 3.3V，LED 灭，串口无输出
- **按键按下**：GPIO1 = 0V，LED 亮，串口打印 `Button PRESSED`
- **按住不放**：每 200ms 打印一次（`delay(200)` 消抖）
- **松开后**：LED 立即灭，串口停止打印

---

## 七、出了什么问题 & 如何修复

- **引脚接错**：把按键接到了 3V3 而非 GND，导致逻辑反转（按下=HIGH）。修复：按键一端接 GND，代码用 `INPUT_PULLUP`。
- **LED 不亮**：长脚/短脚接反，或电阻值太大（1kΩ 以上 LED 会很暗）。220Ω 最保险。
- **按键乱跳**：没加消抖，串口同一按下事件打印好几行。修复：加 `delay(20)` 以上。

---

## 八、下一步

- Day 12：ADC 与传感器读数（用 GPIO1 读电位器）
- Day 19：电机驱动（用到按键的思想）
