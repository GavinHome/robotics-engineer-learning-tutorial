# Day 29 — 作品集整理与 GitHub 个人主页

> 硬件：无新增（纯整理日，一天没碰烙铁）
> 核心：把 28 天的日记当成"别人会看的东西"审一遍，再给自己立一张 GitHub 门面
> 产出：[`GitHub-Profile-README.md`](./GitHub-Profile-README.md)（可直接参考/套用的主页范文）

---

## 零、指南的四件事与今天实际做的

指南 Day 29 列了四项：审查 README、建 GitHub Profile README、确保代码能编译且无警告、学高级 Markdown。四项都做了，但**审查这一项机械查出来的问题比"改改文笔"实在得多**——它揪出了 3 个真的会让访客点空、点不到东西的缺陷。

| 指南要求 | 状态 |
|---|---|
| 1. 审查本月所有项目 README | ✅ 全量审完，修掉 3 类真问题（15 处改动） |
| 2. 建 GitHub Profile README | ✅ 内容写好，合并进已有主页，已推送 |
| 3. 确保所有代码能编译且无警告 | ✅ 12 个工程全编译通过，0 error 0 warning |
| 4. 学高级 Markdown（表格/任务列表/折叠块） | ✅ 表格 1080 行早就在用；**折叠块第一次用上** |

---

## 一、审查：三个真问题

审的时候设了四条判据——**硬件清单、电路图、踩坑是否具体、有没有演示**。逐天查完，"踩坑是否具体"这条是全过的（全文搜"调试了很久"这类词，一处都没有）；但另外三条查出了下面三个问题。

**① 有一整份成果在仓库里，却没有任何一篇 README 提到它。**

`智能小车/` 这个目录：PCB 设计说明、载板设计说明、两张实物照片，git 里 tracked 得好好的，可是外层 README 的「项目结构」表里没有它，任何一天的 README 里也没有它。**从头翻到尾的人永远发现不了这四个文件存在。**

修法：外层 README 的项目结构表加一行，直接链到正反面照片。中英文两个 README 各加一处。

> 这一条比它看起来重要。写日记的人是自己最知道仓库里有什么的人，也正因如此最不可能发现"外人找不到它"。

**② 英文 README 的 Day 1 少列了三张图。**

中文 Day 1 列了四张仿真截图 + 一个 Falstad 电路模板；英文对应位置只有一张，而且那一张指向一个**不存在的文件名**（`1.1 第一个仿真电路.png`）。Day 1 没有单独的日README，全部内容只在外层 README 里——所以英文读者在这一天看到的成果就只有中文版的四分之一。

修法：照着中文补齐四张图 + Falstad 电路模板那一节，链接走百分号编码（`Ω` 是 `%CE%A9`，不是 `%C3%A9`，见下面的坑）。

**③ 有些天的截图在文件夹里躺着，但 README 里没引用。**

这是量最大的一类，10 个文件：

| 目录 | 没被引用的文件 |
|---|---|
| `day-06/` | `点亮LED-USB供电.png` |
| `day-11/` | `按下亮灯.png`、`松开灭灯.png`、`IDE.png` |
| `day-12/` | `实验1-电位器测电压.png`、`实验2-强光光敏电压.png`、`实验2-遮住光敏电压.png`、`实验2电路-光敏电阻测光照.png`、`实验3-Serial Plotter 可视化.png` |
| `day-19/` | `TB6612FNG-1.png` |

修的时候顺手发现 `day-11` 的 §六 讲运行结果一段全是文字、旁边就躺着两张对照截图没贴。这不是"改个链接"的事——**有图不引，等于没拍**。

---

## 二、怎么查的：两个能重跑的脚本

"人工看一眼"在这个规模的仓库里不够。写两段十几行的 Python，一分钟出结果：

<details>
<summary>① 断链检查：README 里引用的文件是否都真实存在</summary>

```python
import os, re, glob
from urllib.parse import unquote
pat = re.compile(r'\]\(([^)#][^)]*)\)')          # 抓 [text](target) 的 target
for md in ['README.md','README.en.md'] + sorted(glob.glob('day-*/README.md')):
    base = os.path.dirname(md) or '.'
    for m in pat.finditer(open(md, encoding='utf-8').read()):
        p = unquote(m.group(1).strip())           # 百分号编码要解码，否则中文名全判错
        if p.startswith('http'): continue
        if not os.path.exists(os.path.normpath(os.path.join(base, p))):
            print("BROKEN:", md, "->", p)
```

</details>

<details>
<summary>② 孤儿检查：文件夹里的图片有没有被任何一篇 README 引用</summary>

```python
refs = set()
for md in ['README.md','README.en.md'] + sorted(glob.glob('day-*/README.md')):
    base = os.path.dirname(md) or '.'
    for m in pat.finditer(open(md, encoding='utf-8').read()):
        p = unquote(m.group(1).strip())
        if not p.startswith('http'):
            refs.add(os.path.normpath(os.path.join(base, p)))
# 链到目录 = 引用了目录里的所有文件，否则 lib/RgbCycle/* 会被误报
files = [f for f in glob.glob('day-*/**/*', recursive=True) if os.path.isfile(f)]
for f in files:
    for r in refs:
        if f.startswith(r + os.sep):
            refs.add(f)
# 磁盘上的文件减去 refs，剩下的就是孤儿
orphan = [f for f in files if os.path.normpath(f) not in refs]
```

</details>

两个脚本合起来跑，结论是 `broken: 0` / `orphans: 0`。**这两个数才是"审完了"的证据**，比"我看了一遍"有用。

---

## 三、编译检查：14 个工程全部通过

指南要求"确保所有代码都能编译且无警告"。逐工程过一遍 `arduino-cli compile --fqbn esp32:esp32:esp32s3`：

<details>
<summary>展开完整编译清单</summary>

| 工程 | 结果 | 占用 |
|---|---|---|
| `day-28/实验1-手机遥控与自动避障` | ✅ 0 error 0 warning | 975804 B (74%) |
| `day-27/实验1-WiFi遥控` | ✅ | 971856 B (74%) |
| `day-25/实验3-WebServer` | ✅ | 958565 B (73%) |
| `day-21/实验2-超声波避障` | ✅ | 324803 B (24%) |
| `day-19/实验1-电机正反转` | ✅ | — |
| `day-20/实验1-自动摆动` | ✅ | — |
| `day-17/实验1-超声波测距` | ✅ | — |
| `day-14/实验1-数字电压表` | ✅ | — |
| `day-13/实验1-JSON串口输出` | ✅ | — |
| `day-12/实验1-电位器测电压` | ✅ | — |
| `day-11/button_led` | ✅ | — |
| `day-09/rgb_cycle` | ✅ | — |
| `day-09/external_led_blink` | ✅ | — |
| `day-09/combined_blink` | ✅ | — |

</details>

**Arduino 的规矩：草图名必须等于所在文件夹名。** `day-09` 原有三个 `.ino` 直接躺在文件夹根下（`day-09/rgb_cycle.ino`），`day-11` 有一个 `day11.ino`。这四个直接丢给 `arduino-cli` 全部报"找不到 sketch"——因为工具链是**拿文件夹名认出草图**的，`.ino` 文件名只是恰好要跟它一样。IDE 也一样打不开。

当时的绕法是复制到同名临时目录再编译，能用，但仓库里这四个文件的位置本身就是错的：换台机器 clone 下来，点也点不开。所以各自建了同名子目录：

| 原来 | 现在 |
|---|---|
| `day-09/rgb_cycle.ino` | `day-09/rgb_cycle/rgb_cycle.ino` |
| `day-09/external_led_blink.ino` | `day-09/external_led_blink/external_led_blink.ino` |
| `day-09/combined_blink.ino` | `day-09/combined_blink/combined_blink.ino` |
| `day-11/day11.ino` | `day-11/button_led/button_led.ino` |

（`day11` 这个名字没有信息量，顺手换成 `button_led`——内容是按键控 LED，和前面几个英文草图名一个风格。）

**要一次改干净**：移动文件只是第一步，两个外层 README 里 4 处指向旧路径的代码链接会同时变断链，`day-09/README.md`、`day-11/README.md` 里还有 5 处相对链接。改完必须重跑断链脚本确认 `broken: 0`，再编译四个草图确认 0 error——**结构改没改对，只看链接和编译，不看感觉**。

---

## 四、高级 Markdown：折叠块第一次派上用场

三项高级特性里有两项早就在用了，一项今天是第一次：

| 特性 | 之前的状态 | 今天 |
|---|---|---|
| 表格 | 全部 README 合计 2027 行表格，天天在用 | 无需补 |
| 任务列表 | 只在 `day-06`、`day-15`、`day-16` 出现过（都是自检表场景） | 场景本身自然，不硬塞 |
| 折叠块 `<details>` | **全仓库 0 处** | 用在"完整的编译清单"和两段脚本上 |

折叠块的正确用法不是"把长内容藏起来"，而是**让读者自己决定要不要展开**——上面的编译清单是给"我要复核"的人看的，脚本是给"我要重跑"的人看的，多数人只想要"编译过没过"这个结论。20 行的表格摊在正文里，对只想知道结论的人是噪音，对要复核的人是必需的，折叠块正好把两类人分开。

配套的一条坑：`<details>` 下面第一行**必须紧跟着 `<summary>`**，中间不能空行；而 `<summary>` 那一行结束之后**必须空一行**再开始正文内容。空行位置错了，GitHub 会把标签当普通文字直接印出来——HTML 混写时，渲染器比语法检查更挑。

---

## 五、Profile README：仓库不是空的

指南第二件事是建 GitHub 个人主页。看着是"写完一推就完事"，实际有个坑：**这个仓库可能不是空的。**

用 [github-profile-readme-generator](https://github.com/rahuldkjain/github-profile-readme-generator) 这类工具生成过主页的话，仓库里会有两个文件，而且它们是**手拉手的一对**：

| 文件 | 是什么 |
|---|---|
| `README.md` | 生成器的**产物**——统计卡片、访客计数、技能图标全在里面 |
| `data.json` | 生成器的**源配置**——网站上改它、点 Generate，才产出 README |

**直接覆盖 `README.md` 会连带踩三件事**：统计卡片和访客计数清零重来；已经展示出去的身份被换成只有新内容；`data.json` 变成孤儿——留着不再对应 README，删掉又弄坏生成器那条路。

**所以做的是合并，不是覆盖**：原有 URL 一个不动（动手前先把原有 URL 抓成集合，改完再 diff 一次，`丢失: 0` 才提交），要加的内容往后排。

**写公开教程时要分清"能迁移的教训"和"我这次的账号清单"。** 后者（有哪些卡片、改成几行、动了几个字段）只对自己有意义，写进去只是噪音；前者才是给别人看的。真正能迁的是这些：

| 问题 | 原因 | 修法 |
|---|---|---|
| 我"顺手"把一个图标换了 | 写新 README 时凭印象写了同图标的另一个 CDN 版本，原来那个用的是别的源站 | 按原文件逐字改回。**"顺手统一一下"是最容易偷偷改坏别人东西的动因** |
| 差点误判主页的名字是错的 | 三处名字对不上（README 一个、`data.json` 一个、commit 作者又一个），先入为主认定 README 那个是模板残留 | `gh api user` 查主页显示名，用事实判不用直觉。**先查事实再动手** |
| **把"精简"做成了"重写"** | 让精简某一节，结果把相邻的整段一起砍了 | 上一版找回来，只删指名的那几行。**听到"精简"先问精简谁，再动手** |
| 首页写了做不到的承诺 | 写"每个文件夹都有踩坑小节"，数了一遍才发现大半根本没有 | 承诺退回标准该在的位置：主页只写确实普遍成立的事 |
| 把"过程"写成了"项目" | 把做出小车的必经步骤（ADC → 串口 → 数据落盘）和最终成果并排罗列，读者会以为那些是终点 | 一个月一条成果物，中间步骤留在当天日志里。**计划和成果各一张表，不互相冒充** |

还有一条：**别人的文件里，看着没用的东西也得先问过再动。** 原 README 有个空的 `Connect with me:` 占位标题，我按"没用的占位"删了——那是原作者的资产，不是我的优化对象，按原位置原样放回。

**这条最值得记住**：`data.json` 和 `README.md` 在这个仓库里是**手拉手的两份东西**，但没有任何机制保证它们一致。改了 README 不同步 data.json，下次打开生成器网站点 Generate，前面所有手写内容全被冲掉。

---

## 六、踩的坑

| 问题 | 原因 | 修法 |
|---|---|---|
| 断链脚本报出 12 个"坏链" | 看仓库里有中文名就以为脚本不认，其实是忘了 URL 解码——README 里写的是百分号编码 `%E6%95%99%E7%A8%8B`，脚本拿它直接去拼路径 | 先 `unquote()` 再拼路径，12 个假阳性全消 |
| 孤儿脚本把 103 个文件都报成孤儿 | `refs` 里存的是 README 里写的相对目标（`tinkercad.png`），磁盘上却是 `day-08/tinkercad.png`，两个根本没对齐 | 存的时候就用 `os.path.join(base, target)` 归一化，从 103 个降到 10 个真孤儿 |
| 英文 README 里 `Ω` 编码写错 | 手写百分号编码时把 `Ω`（U+03A9）当成了 `é`（U+00E9） | 用 `urllib.parse.quote()` 生成，别手写——希腊字母的西里尔式长相太容易混 |
| 四个 `.ino` 编译不过 | 草图文件名 ≠ 文件夹名，散在 `day-09/`、`day-11/` 根下 | 各建同名子目录（`rgb_cycle/rgb_cycle.ino`），同时改掉两个外层 README 里 4 处代码链接，改完重跑断链脚本 + 编译验证 |

---

## 七、产出与遗留

**产出**

- [`GitHub-Profile-README.md`](./GitHub-Profile-README.md)：一份可直接参考套用的英文个人主页范文——按"已交付成果 + 后续月份计划"组织，主页只放结论，细节指回仓库
- 两个外层 README 各修 3 处：项目结构表补 `智能小车/`、英文 Day 1 补齐四张图和电路模板、重复标题去重

---

### 选做 / 进阶

⏭️ 指南 Day 27-28 的 **IMU 姿态显示**：车上没有 IMU，且两轮 + 万向轮底盘是静稳定的，姿态不进控制回路，手机上只是个显示项。等做两轮平衡车或无人机时它会从显示项变成生死线——那时换 ICM-42688-P / BMI270，别用手里这颗 MPU-6050（零偏漂移大、DMP 配置麻烦）

⏭️ 把 `/data` 的轮询并进心跳。现在心跳每 250ms 一次、拉数据每 500ms 一次，是两条独立连接，合成一个能省一半建连开销

⏭️ 给 `/data` 加掉线计数。`up` 是 `millis()/1000`，从上电起算、掉线重连不清零，"一直好着"和"断了又连回来"在面板上长得一样

⏭️ 转向时给两侧不同 duty。现在原地转向两侧都是 `DUTY_TURN`，但 Day 24 记过两个马达转速不一致，等 duty 会让原地转画弧而不是原地转

⏭️ 给 `/cmd` 加访问控制。现在同一局域网内任何人都能让车动——家里无所谓，拿到公开场合就是谁都能开
