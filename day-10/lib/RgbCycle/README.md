# RgbCycle（仓库备份）

这里是 Arduino 库 `RgbCycle` 的源码备份，实际安装位置：

```
~/Documents/Arduino/libraries/RgbCycle/
```

之所以两者都留：装到系统库目录才能让**任意草图**（不止本仓库）一行 `#include <RgbCycle.h>` 调用；仓库里留一份是为了跟着 git 走、换电脑不丢。

## 文件

| 文件 | 说明 |
| --- | --- |
| `RgbCycle.h` | 模块本体，非阻塞彩灯循环 |
| `library.properties` | Arduino 库元数据 |

## 用法

```cpp
#include <RgbCycle.h>

void setup() { RgbCycle::begin(); }

void loop() {
  RgbCycle::update();   // 彩灯自己跑自己的节奏，不阻塞
  // 其他任务写在这里
}
```

## 接口

| 函数 | 调用位置 | 作用 |
| --- | --- | --- |
| `RgbCycle::begin()` | `setup()` 一次 | 初始化灯珠、清掉复位期间的随机锁存色 |
| `RgbCycle::update()` | `loop()` 每圈 | 判断是否到 500ms，到点换下一色 |
| `RgbCycle::setColor(r, g, b)` | 任意 | 让灯珠停在指定颜色 |

## 维护提醒

改动源码时，**两边都要同步**：改完 `~/Documents/Arduino/libraries/RgbCycle/` 后，把文件复制回这里，或反过来。更新步骤：

```bash
cp ~/Documents/Arduino/libraries/RgbCycle/RgbCycle.h \
   ~/Documents/Arduino/libraries/RgbCycle/library.properties \
   机器人工程师学习计划/robotics-engineer-learning-tutorial/day-10/lib/RgbCycle/
```

> 这个"两处副本要手动同步"的代价，是换来"任意草图可调用"的必然结果。如果哪天觉得麻烦，可以改成把库只放在各任务草图文件夹里（Arduino 会把它显示成标签页），代价则是每个任务要复制一份。
