# Day 22 — Python 基础回顾

> 硬件：ESP32-S3 开发板（N16R8）+ HC-SR04（沿用 Day 21 车上那套）+ 电位器或光敏电阻 ×1
> 核心：把串口数据从"给人看"改成"给程序看" → Python 落 CSV / 解析配置 / 批量改名
> 代码：[`实验1-串口遥测.ino`](./实验1-串口遥测/实验1-串口遥测.ino) ｜ [`serial_logger.py`](./serial_logger.py) ｜ [`log_to_csv.py`](./log_to_csv.py) ｜ [`config_check.py`](./config_check.py) ｜ [`config.json`](./config.json) ｜ [`batch_rename.py`](./batch_rename.py)
> 数据：[`sensor_log.csv`](./sensor_log.csv) ｜ [`sensor_log.json`](./sensor_log.json) ｜ [`sensor_log.png`](./sensor_log.png)

---

## 零、三个脚本分别是什么

指南 Day 22 要求写三个脚本，它们练的是**三块互不包含的 Python**，只有一个跟机器人有关：

| 脚本 | 干什么 | 练的模块 | 与项目的关系 |
|---|---|---|---|
| [`serial_logger.py`](./serial_logger.py) | 读串口 JSON → 存 CSV | `serial` / `csv` / `json` | **真实数据链路** |
| [`config_check.py`](./config_check.py) | 读 `config.json` → 检查值合不合理 | `json.load` + 自己写校验 | 半相关（校验小车参数） |
| [`batch_rename.py`](./batch_rename.py) | 批量改文件名 | `pathlib` / `argparse` / `unicodedata` | **完全无关**，纯语法练习 |

后两个不是"存完 CSV 就够"的补充，是另外两组语法练习。`batch_rename` 尤其跟机器人一点关系没有，指南列它是因为那是练路径操作的经典题。

另有一个 [`log_to_csv.py`](./log_to_csv.py)，不在指南要求里，是为了在没法烧板子的今天也能拿到真实数据（见 §四末尾）。

---

## 一、今天和前 21 天的关系

前 21 天的串口输出全是**中文日志**：`前方 25.9cm，通畅 → 直行前进`。那是写给人看的——调试时眼睛一扫就知道车在想什么。

今天要写 Python 程序去读这些数据，立刻撞上一个问题：Python 拿到的是字符串 `"前方 25.9cm，通畅 → 直行前进"`，想拿到那个 `25.9` 得写正则去抠。抠出来的数字还没有字段名，下次改一句日志文案，正则就失效了。

所以今天第一件事是**换输出格式**：ESP32 改成一行一条 JSON。

```
旧：前方 25.9cm，通畅 → 直行前进
新：{"ms":12345,"dist_cm":25.9,"adc_raw":2048,"voltage":1.65}
```

Python 端 `json.loads()` 一行变字典，字段名自解释，加字段不用改解析逻辑。代价是串口对人类不友好了——但这是给机器的数据通道，人看的调试信息仍然可以另外打，两者不冲突。

> 这个改动不是 Day 22 临时起意。Day 26 要实时绘图、Day 27-28 要 Wi-Fi 遥测，都得建立在"数据有结构"之上。

---

## 二、环境：为什么建了 .venv

`pip install pyserial` 直接失败：

```
error: externally-managed-environment
× This environment is externally managed
note: If you believe this is a mistake, please contact your Python installation
      or OS distribution provider. You can override this, at the risk of breaking
      your Python installation or OS distribution, by passing --break-system-packages.
```

这是 **PEP 668**。macOS 上 Homebrew 装的 Python 3.14 被标记为"外部管理"——系统里可能有别的程序依赖它自带的包，pip 直接往里塞东西可能把那些依赖搞坏，所以 pip 拒绝执行。

提示里给的 `--break-system-packages` 是能用的，但那是把保护罩整个拆掉。**正确做法是建虚拟环境**：

```bash
/opt/homebrew/bin/python3 -m venv .venv --system-site-packages
./.venv/bin/python -m pip install pyserial
```

`--system-site-packages` 是关键：matplotlib 是之前装在系统 Python 里的（3.11.1），加这个参数后 venv 能直接复用它，不用再下一遍。验证：

```
pyserial 3.5 | matplotlib 3.11.1
```

顺手建了 `.gitignore` 把 `.venv/` 排除——虚拟环境有几百个文件，且换台机器重建即可，不该进仓库。

---

## 三、实验 1：给 Python 喂数据的 ESP32 草图

[`实验1-串口遥测.ino`](./实验1-串口遥测/实验1-串口遥测.ino)

沿用 Day 21 车上那套 HC-SR04（Trig=GPIO4 / Echo=GPIO5），再接一路模拟量到 **GPIO1**。

| 信号 | GPIO | 说明 |
|---|---|---|
| HC-SR04 Trig | 4 | 沿用 Day 21 |
| HC-SR04 Echo | 5 | 沿用 Day 21 |
| 模拟量输入 | 1 | **ADC1_CH0** |
| 板载 RGB | 48 | 心跳灯 |

> ⚠️ **GPIO1 而不是 GPIO11。** ESP32-S3 的 ADC2（GPIO11-20）与 Wi-Fi 共用一套硬件，开着 Wi-Fi 时 `analogRead()` 会失败。Day 27 要同时跑 Wi-Fi 和电池电压监测，从今天起就一律走 ADC1（GPIO1-10），养成习惯。

三个不那么显然的决定：

**① 只发 JSON，不打任何中文横幅。**
ESP32 上电时 ROM 会先吐一串启动日志：

```
ESP-ROM:esp32s3-20210327
Build:Mar 27 2021
rst:0x1 (POWERON),boot:0x8 (SPI_FAST_FLASH_BOOT)
```

Python 靠"行首是不是 `{`"来区分数据行和噪声行。如果我为了好看多打一行 `Day22 串口遥测开始`，那行虽然也会被过滤掉（不以 `{` 开头），但同时多了一个需要记住的规则。索性一个字不多打。

**② 测距超时返回 `-1`，不是 `0`。**

```cpp
unsigned long us = pulseIn(ECHO_PIN, HIGH, TIMEOUT_US);
if (us == 0) return -1.0;     // 超时/超出量程：-1，不是 0
```

`0` 会被当作"障碍物贴脸"，画进图里是一根扎到 0 的假尖刺，看上去像车突然撞墙了。`-1` 在图上能被识别成"无读数"而断开。

**③ 采样率 10Hz，不是越快越好。**
115200 波特率理论每秒约 11520 字节，每行 JSON 约 50 字节，10Hz 只占 0.4% 带宽。超过 50Hz 就开始丢字符，JSON 会解析到一半断掉——数据画出来是随机的断线。

板载彩灯这次是**心跳灯**，`RgbCycle::begin()` + `loop()` 里 `RgbCycle::update()`，不调 `setColor()`。它内部用 `millis()` 判断，不阻塞采样。

---

## 四、脚本 1：串口数据 → CSV

[`serial_logger.py`](./serial_logger.py)

### 先弄清：Python 是怎么拿到车上数据的

**车上没有 Python。** ESP32 上跑的是 C++（Arduino 编译成机器码），Python 只跑在 Mac 上。中间就是那根 USB 线——它同时供电**和**传数据（ESP32-S3 是 USB CDC，插上后 Mac 认成一个虚拟串口）。

```
ESP32（C++）                      Mac（Python）
─────────────                    ─────────────
Serial.printf("{...}\n")   ──USB──►  ser.readline()
                                     /dev/cu.usbmodem5C831106401
```

关键点：`/dev/cu.usbmodem5C831106401` **是一个文件**（Unix 里一切皆文件）。Python 读它跟读 `data.txt` 是同一套动作：

```python
ser = serial.Serial('/dev/cu.usbmodem5C831106401', 115200)  # 相当于 open()
line = ser.readline()   # 相当于 f.readline()，拿到 b'{"ms":100,...}\n'
```

**不是手动复制，是实时的**：ESP32 每 100ms 发一行，Python 那边立刻读到。波特率 115200 只是告诉两边每秒传多少位，设成一样才能对上。

整条链路：

| 环节 | 谁做 | 动作 |
|---|---|---|
| 产生 | ESP32 上的 C++ | `Serial.printf("{...}\n")` 每 100ms |
| 传输 | USB 线 | 虚拟串口 `/dev/cu.usbmodemXXX` |
| 接收 | Mac 上的 Python | `ser.readline()` → 字节串 |
| 解码 | Python | `.decode('utf-8')` → 字符串 |
| 解析 | Python | `json.loads()` → 字典 |
| 落盘 | Python | `csv.writer` → `sensor_log.csv` |

用假串口实测时撞到两个真实现象：

**① 第一条数据会丢。** 打开串口的瞬间 DTR 电平变化**把 ESP32 复位了一次**，复位前缓冲区里的数据是陈旧的、可能半截的。所以脚本里有：

```python
ser.reset_input_buffer()   # 打开后先清空，等板子重启完再读
```

**② 没数据时 `readline()` 返回 `b''` 而不是卡住。** 那是 `timeout=1` 生效——一秒没等到就返回空字节。不给 timeout 就会永远等下去，Ctrl+C 都不响应。

> ⚠️ **Arduino IDE 的串口监视器和 Python 不能同时占这个口。** 跑脚本前要关掉监视器，否则报 `could not open port: Resource busy`。

```bash
./.venv/bin/python day-22/serial_logger.py                    # 自动探测串口
./.venv/bin/python day-22/serial_logger.py --n 200            # 采 200 条自动停
```

产出 `day-22/sensor_log.csv`：

```csv
timestamp,ms,dist_cm,adc_raw,voltage
2026-09-22T18:42:07.251,100,25.9,2048,1.65
2026-09-22T18:42:07.306,200,-1.0,2100,1.69
2026-09-22T18:42:07.413,400,30.8,1995,1.61
```

几个坑：

- **`serial.Serial(port, BAUD, timeout=1)` 的 timeout 不能省。** 不给 timeout 时 `readline()` 会一直等到收到换行符，而 Ctrl+C 的 KeyboardInterrupt 要等这一次调用返回才生效——表现为按了 Ctrl+C 程序半天不退出。
- **`open(..., newline="")` 是 csv 模块的硬性要求。** 不写的话 Python 会自己做换行转换，CSV 里多出空行。
- **解码要 `errors="replace"`。** 波特率偶尔错半个周期会吐出非法 UTF-8 字节，默认的严格模式会抛异常中断整个采集循环——而这本来只是丢一帧数据的事。
- **过滤而非解析全部行。** `if not line.startswith("{"): continue` 挡掉 ROM 日志；`json.JSONDecodeError` 也要 catch，串口偶尔会读到半行。

### 无硬件也能验证

不想为了测脚本反复拔插板子，用 pty 造一个假串口灌数据：

```python
import os, pty, subprocess, threading, time
master, slave = pty.openpty()
# 往 master 写带噪声的数据流，把 os.ttyname(slave) 当串口名传给脚本
```

实测灌入"ROM 噪声 + 正常帧 + 残缺帧 + 正常帧"，脚本正确跳过噪声和残缺帧，只落了 3 条完整数据——正是上面的 CSV。

### 今天的 CSV 是怎么来的

**数据源：Day 21 已保存的串口日志 [`实验2-超声波避障-串口打印.txt`](../day-21/实验2-超声波避障-串口打印.txt)，不是本次从串口实时读取。**

车上正跑着别的程序，不方便重烧遥测草图。但落 CSV 这段逻辑没必要等——那份日志里的每一个数字都是当时车上真实测到的，不是构造出来的。于是写了 [`log_to_csv.py`](./log_to_csv.py)，把中文日志解析成和 `serial_logger.py` **字段完全一致**的 CSV：

```bash
./.venv/bin/python day-22/log_to_csv.py
./.venv/bin/python day-22/log_to_csv.py day-21/实验3-单步调试-日志打印.txt
```

产出 [`sensor_log.csv`](./sensor_log.csv) ／ [`sensor_log.json`](./sensor_log.json) ／ [`sensor_log.png`](./sensor_log.png)：

```
解析 day-21/实验2-超声波避障-串口打印.txt
  共 49 条记录，其中有效距离 14 条
  距离范围 6.1 ~ 37.2 cm
  超时(-1) 26 条，无效(-2) 9 条
```

两条正则就够了：

```python
DIST_RE       = re.compile(r"前方\s+([\d.]+)\s*cm")
OVER_RANGE_RE = re.compile(r"超出量程")     # 日志写"超出量程（>100cm）"，对应草图的 cm < 0 → -1
INVALID_RE    = re.compile(r"读数无效")     # 对应 0 <= cm < 2.0 → -2
```

有个坑差点漏掉。日志里有两种含距离的行：

```
前方 25.9cm，通畅 → 直行前进
第1轮｜前方 6.1cm < 15cm，挡住了 → 后退 500ms
```

只抓第一种的话，**小于 15cm 的近距离读数会全部漏掉**——而触发避障的那批恰恰是最有价值的一段（能看出迟滞有没有生效）。所以两种行都抓。

时间戳是还原出来的：`ms = seq * 500`，因为那份日志来自 Day 21 草图的 `LOG_MS=500`。`timestamp` 和 `adc_raw`/`voltage` 三列留空——日志里当年没打印这些，空着比编一个数诚实。

[`sensor_log.png`](./sensor_log.png) 里只有 14 个真实点，中间大片是 -1（超时）和 -2（无效）断掉的地方。这不是数据质量差，正是 HC-SR04 的真实表现：超过约 100cm 就超时，低于 2cm 读数不可信。画出来断断续续才是对的。

> 这也顺带说明了为什么 Day 21 要在 `config.json` 里留 `enter_cm=15 / exit_cm=25` 的迟滞：真实读数就是这么跳。

---

## 五、脚本 2：解析并校验 JSON 配置

[`config_check.py`](./config_check.py) + [`config.json`](./config.json)

把 Day 21 小车的参数抽成配置文件：

```json
{
  "car":   { "duty_cruise": 115, "duty_turn": 120, "trim_right": 0, "pwm_freq": 20000 },
  "avoid": { "enter_cm": 15, "exit_cm": 25, "back_ms": 500, "turn_ms": 400 },
  "pins":  { "ain1": 10, "ain2": 11, "pwma": 12, "bin1": 15, "bin2": 16, "pwmb": 17,
             "trig": 4, "echo": 5, "adc_battery": 1 },
  "serial":{ "baud": 115200, "sample_ms": 100 }
}
```

**`json.load()` 只保证语法合法，不保证值合理。** `duty_cruise` 写成 999 照样解析成功，一路带到电机上才发现不对。所以读完必须自己校验。

校验项里有两个是真正会咬人的：

```python
elif enter >= exit_cm:
    problems.append("迟滞方向反了：enter_cm 必须小于 exit_cm")
```

Day 21 的 15/25cm 迟滞就是为了不让车在阈值附近抽搐。写反了的话，车会"进入 15cm 触发后退 → 后退后距离 >15cm 立刻解除 → 又前进 → 又触发"，原地抖成一团。

```python
forbidden = set(range(22, 35)) | {43, 44}   # 本板未引出 / Flash / PSRAM / USB
```

GPIO22-34 在 ESP32-S3-WROOM-1 上未引出（26-32 内部 Flash，33/34 Octal PSRAM），43/44 是 USB 串口。配置里写错这些脚，编译照样过，上机才发现没反应。顺带查重复占用——两个功能抢同一个 GPIO 是很常见的复制粘贴事故。

另一个设计：**收集所有问题后一次性返回**，不是遇到第一个就抛异常。配置往往不止一处错，一次全报出来省得多跑几遍。

---

## 六、脚本 3：批量重命名

[`batch_rename.py`](./batch_rename.py)

```bash
./.venv/bin/python day-22/batch_rename.py day-21          # 只演练
./.venv/bin/python day-22/batch_rename.py day-21 --apply  # 真执行
```

第一版写成了"只保留 ASCII 字符"，跑演练时发现：

```
扫描 day-22：5 个文件，需重命名 1 个
  实验1-串口遥测.ino  →  1.ino
```

中文全被剥掉，只剩个 `1.ino`。**这个设计是错的。**

想清楚脚本到底该解决什么：本仓库的素材名全是中文（`实验1-双电机差速`、`超声波避障接线实物图`），删掉中文等于删掉全部信息。转拼音要引入 `pypinyin`，为一次批量改名装个词库不划算。

脚本真正擅长处理的是**空格、括号、全角数字、连续分隔符**这些在命令行和 URL 里容易出错的字符。所以改成保留中文，只做：

```python
s = unicodedata.normalize("NFKC", name)   # 全角 ＡＢＣ１２３ → 半角 ABC123
s = SPLIT_RE.sub("_", s)                  # 空格括号顿号逗号 → 下划线
s = re.sub(r"_+", "_", s).strip("_.")
```

另外两条保护：

- **默认 `--dry-run`。** 重命名不可逆，尤其这些照片是实测记录，改错名字就对不上实验。先看清单确认无误再加 `--apply`。
- **目标名冲突就跳过并报告，不覆盖。** 宁可少改一个，也不能静默吞掉一张照片。

---

## 七、踩的坑

| 现象 | 原因 | 解决 |
|---|---|---|
| `pip install` 报 externally-managed-environment | PEP 668 保护 Homebrew Python | 建 venv，不用 `--break-system-packages` |
| venv 里 import matplotlib 失败 | 默认 venv 与系统包隔离 | `venv --system-site-packages` 复用 |
| 批量改名后中文文件名变 `1.ino` | 脚本把非 ASCII 全剥了 | 保留中文，只规整分隔符与全角 |
| CSV 每行之间多一个空行 | Python 自动换行转换 | `open(..., newline="")` |
| Ctrl+C 后程序不退出 | `readline()` 无 timeout 一直阻塞 | `serial.Serial(..., timeout=1)` |
| 超时读数画出来是扎到 0 的尖刺 | 用 0 表示"无读数" | 改用 -1 |
| 解析日志时近距离读数全丢了 | 只抓了"通畅"那一种行 | 两种含距离的行都要抓，否则 <15cm 的样本全漏 |

---

## 八、编译与运行

```
实验1-串口遥测：Sketch uses 328152 bytes (25%) / Global variables 22320 bytes (6%)
```

✅ 通过（`--fqbn esp32:esp32:esp32s3`）。

```
config_check.py  → 配置校验通过（exit 0）
batch_rename.py  → 演练模式，输出改名清单（exit 0）
serial_logger.py → 假串口端到端验证，3 条数据正确落盘（exit 0）
log_to_csv.py    → 解析 Day 21 日志，49 条记录 / 14 条有效距离（exit 0）
```

⏳ `serial_logger.py` 的**实时串口**采集待上机：烧 [`实验1-串口遥测.ino`](./实验1-串口遥测/实验1-串口遥测.ino)，然后 `./.venv/bin/python day-22/serial_logger.py --n 300`。在那之前 [`sensor_log.csv`](./sensor_log.csv) 由 `log_to_csv.py` 从 Day 21 已保存日志解析而来——数字是真实测到的，但不是本次串口实时读的。

> 📌 **串口这条路是有窗口期的，而且窗口已经关了。** Day 24 接上 MP1584EN 降压模块之后车脱离 USB 独立跑（见 Day 21「成品化供电」），自由跑时没有串口；要采串口就得插 USB，插上线车又被拴住。Day 26 的"串口实时绘图"只能改成收 Wi-Fi 数据 —— 好在只需要把 `serial.Serial()` 换成 socket 读，本文件的解析与落 CSV 逻辑原样复用。

---

## 九、下一步

- **Day 23**：Git 版本控制（Learn Git Branching + 为前几周项目建仓库 + `.gitignore`）

---

### 选做 / 进阶

⏭️ 进阶：`serial_logger.py` 加 `--plot` 参数，采完直接用 matplotlib 出图——这就是 Day 26 的活，提前做了 Day 26 只剩实时刷新那一层
⏭️ 进阶：让 `config.json` 真正生效——ESP32 从串口读 JSON 并在运行时改参数，省掉反复烧录（标定 trim 时特别有用）
⏭️ 进阶：`batch_rename.py` 支持正则替换（`--sed 's/实验/ex/'`），应对更复杂的改名需求
