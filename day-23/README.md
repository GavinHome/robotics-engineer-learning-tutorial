# Day 23 — Git 版本控制

> 硬件：无（纯软件日）
> 核心：不练"怎么敲 git"，练"**怎么确认改动是对的、搞砸了怎么捞回来**"——督察与救火
> 仓库：[`robotics-engineer-learning-tutorial`](https://github.com/GavinHome/robotics-engineer-learning-tutorial)

---

## 零、今天练的不是指南写的那三件事

指南 Day 23 列了 `init / add / commit / push / branch / merge / log / diff`，还要求"为前两周项目建三个公开仓库"。

对照现实：

| 指南要求 | 实际情况 |
|---|---|
| `git init` / 建仓库 | 这个仓库 2026 年就在用，164 个 commit，早已挂 `origin` |
| `add` / `commit` / `push` / `log` | 天天在用 |
| `branch` / `merge` / `diff` | **真没练过**——一直单分支 `main` 直线提交 |
| 建 3 个公开仓库 | 单仓已经包含其他项目，公开数量早就够，没必要为凑数割裂 30 天主线 |

真正缺的是另外一层：`add` / `commit` / `push` 是**执行**，而"**怎么确认改动是对的**"和"**搞砸了怎么捞回来**"是另一回事——出了事能不能自己兜住，全看这两样。这才是今天的重点。

> ⏭️ 指南要求的"拆 3 个独立仓库"跳过理由：拆出来的仓库会脱离 30 天时间线，以后改笔记要两头同步。单仓内已有其他项目，公开数量本来就够。

---

## 一、分支就是一个指针，不是一个副本

`git branch xxx` 之后看一眼 refs 文件：

```bash
$ cat .git/refs/heads/practice/day23
d94aa4096c9b801d648474e6f17d7cd6ff2f1e81
$ ls -l .git/refs/heads/practice/day23
41 bytes
```

**41 字节**——40 个十六进制字符加一个换行符。开分支不复制任何文件，只是在某个 commit 上贴了个标签。切换分支时变的是工作区内容，仓库体积几乎不变。

所以分支可以随便开。成本高的是**合并时的思考**，不是创建。

---

## 二、双点语法 `A..B`：督察的核心

分叉之后（`main` 被推进，同时分支在改别的），三个问题对应三条命令：

```bash
git log --oneline main..practice/day23     # ① 分支有、但 main 还没有的（待合入）
git log --oneline practice/day23..main     # ② main 有、但分支还没有的（我落后了）
git diff --stat main practice/day23        # ③ 两边总共动了哪些文件，各动了多少行
```

实测输出：

```
4bc81a5 docs: 分支上的改动                ← ①
9531b59 chore: main 上的另一处改动          ← ②

 .gitignore       | 1 -
 day-22/README.md | 4 ++++                      ← ③
 2 files changed, 4 insertions(+), 1 deletion(-)
```

**双点 `A..B` 读成"B 有而 A 没有"。** 顺序反了答案也反。只会 `git log` 顺着看一条线的话，遇到分叉就只能猜；有了这个语法，能立刻确认哪些改动还没合进来。

`git diff --stat` 是这套里最常用的：**只看文件名和改动量，不看内容**。一句"我改了配置"，配上这个就能看出是不是顺手动了三个不相干的文件。

---

## 三、冲突：看懂标记比会用工具重要

两边同时改 `day-22/README.md` 的末尾，`git merge` 报：

```
Auto-merging day-22/README.md
CONFLICT (content): Merge conflict in day-22/README.md
```

冲突中的文件长这样：

```
<<<<<<< HEAD
> main 备注：同一处，main 上也改了
||||||| d94aa40
=======
## 附录

（分支上追加的一段）

> 分支备注：本文件在分支上被修改
>>>>>>> practice/day23
```

四段标记：

| 标记 | 含义 |
|---|---|
| `<<<<<<< HEAD` | 冲突开始；下面是我这边（当前分支）的内容 |
| `\|\|\|\|\|\|\| d94aa40` | **分叉基点**——两边共同的祖先在这行长什么样 |
| `=======` | 分界线，上面是我，下面是对方 |
| `>>>>>>> practice/day23` | 冲突结束；上面是合进来的分支内容 |

中间那行 `|||||||` 是 **diff3 格式**，默认不开（`git config --global merge.conflictStyle diff3` 就有了）。开着的价值很大：不给你基点，你只看到"我写了 A、他写了 B"，不知道原始是 C；有了基点能判断出**对方是新增还是删掉了你的东西**——前者多半要两边都留，后者要想一下。

解决完之后 `git status --short` 的 **`UU`** 也是新知识：

```
UU day-22/README.md
```

两位状态码，左位是暂存区状态、右位是工作区。**`UU` = 两边都是 U（unmerged）**，意思是这个文件冲突还没解决。常见的第一位的其他值是 `M`（已修改已暂存）、`A`（新添加）。

> 实际处理这次冲突时选了"两边都留"——因为基点那一看是空的，说明两边都是纯新增，不存在谁覆盖谁。

---

## 四、reflog：丢了 commit 之后唯一的后悔药

最有演示价值的一段。故意做破坏性操作：

```bash
$ git reset --hard d94aa40        # 把 main 硬退回去
HEAD is now at d94aa40 docs: 更新电子维修工具采购清单及待购项目

$ git log --oneline -2            # 刚才那 4 个 commit 全不见了
d94aa40 ...
df5c31a ...
```

`git log` 已经看不见它们了。**但 reflog 记得**：

```bash
$ git reflog -8
d94aa40 HEAD@{0}: reset: moving to d94aa40
41c40c0 HEAD@{1}: commit (merge): merge: 解决冲突，两边内容都保留
aeec322 HEAD@{2}: commit: docs: main 触碰同一行
...
```

`41c40c0` 还在。捞回来就一行：

```bash
$ git reset --hard 41c40c0
```

**为什么 `reset --hard` 删不掉东西？** 因为 Git 删的只是"指针"，commit 对象还躺在 `.git/objects/` 里。`reflog` 是 HEAD 的移动流水账——它记录了每一次 checkout / commit / reset / merge 之后 HEAD 落在哪。

代价是 reflog 有过期时间：默认 **90 天**（不可达对象 `gc` 后清掉）。所以"三个月前误删的东西现在还能救"这话不成立，真出事要当天处理。

> 这也解释了为什么 `reset --hard` 属于危险操作却不致命：危险在丢**没 commit 的工作区改动**，那部分 reflog 里没有。commit 过的东西基本都捞得回来。

---

## 五、merge 还是 rebase

```
前：  main  A—B—C
            \
      dev    D—E            ← 在 B 上开的分支

merge 后：  A—B—C—M         M 有两个父节点，历史保留分叉
                  \ /
                   D—E

rebase 后： A—B—C—D'—E'     一条直线，D/E 变成新 commit
```

关键差别：**rebase 改写了 commit**。`D` 变成 `D'`，hash 不一样——图上看着是同一个改动，Git 眼里是两个不同的 commit。

由此一条硬规矩：**已经 push 出去给别人看的 commit 不要 rebase。** 本地重写了历史，别人的仓库里还留着旧 hash，下次同步会打架。

| | 保留信息 | 历史形状 |
|---|---|---|
| merge | 保留"这里曾经开过分支"这个事实 | 有分叉，如实反映并行开发 |
| rebase | 抹掉曾开过分支这件事 | 一条直线，好读 |

本项目 main 一直是单条直线 164 个 commit，说明单人单机线性推进时 merge 就够用。真要用 rebase 的场景是 PR 合入前把自己的分支拉到最新。

---

## 六、收尾：怎么确认本地和远端一致

```bash
git status                           # working tree clean
git log --oneline origin/main..main  # 空 = 没有本地独有的 commit
git log --oneline main..origin/main  # 空 = 没有落后远端
```

两个方向的 log 都为空才叫"与远端完全一致"。**只看 `git status` 是不够的**——它能告诉你工作区干净，但告诉不了你本地多推了还是少拉了。

---

## 七、今天新增的命令

| 命令 | 干什么 | 什么时候用 |
|---|---|---|
| `git log A..B` | B 有而 A 没有的 commit | 确认有哪些改动还没合进来 |
| `git diff --stat A B` | 两边改了哪些文件、改了多少 | 检查是不是顺手动了别的文件 |
| `git log --graph --oneline` | 画历史分叉图 | 看清楚分支关系 |
| `git reflog` | HEAD 的移动流水账 | commit 丢了要捞回来 |
| `git reset --hard <hash>` | 硬退到指定 commit | 既能回滚，也能配 reflog 救回 |
| `git checkout -b <name>` | 建分支并切过去 | 开新活之前 |
| `git branch -D <name>` | 强制删分支 | 分支用完不再需要时 |

外加一个配置建议：

```bash
git config --global merge.conflictStyle diff3
```

让冲突标记带上分叉基点（`|||||||` 那行）。代价是多几行噪音，收益是能判断对方是新增还是删除。

---

## 八、踩的坑

| 现象 | 原因 | 解决 |
|---|---|---|
| `git log` 看不到刚做的 commit | `reset --hard` 只移了指针，log 沿新 HEAD 走 | `git reflog` 找旧 hash，`reset --hard` 回去 |
| 冲突文件里只看到两边的内容，不知道原来是什么 | 默认冲突格式不带基点 | 开 `merge.conflictStyle diff3` |
| 以为工作区干净就等于仓库干净 | `git status` 不对比远端 | 双向 `git log main..origin/main` |

---

## 九、下一步

- **Day 24**：README 写作与项目展示（Fritzing / draw.io 画电路图 + GIF 演示）
- 📌 Day 24 降压模块已接好，车脱离 USB 独立供电（电池 6V 分两路，电机直连、开发板走降压）

---

### 选做 / 进阶

⏭️ 进阶：`gh auth login` 后把日常操作迁到 CLI——建 PR、看 CI 都不用开浏览器（Day 29 做 GitHub Profile 时会用到）
⏭️ 进阶：配 `git config --global rerere.enabled true`，让 Git 记住冲突是怎么解的，下次同样冲突自动套用
⏭️ 进阶：练 `git bisect`——传感器读数某天开始不对时，二分查找定位到出问题的那个 commit
⏭️ 进阶：给仓库加 commit 规范（Conventional Commits），这个仓库已经在用了（`feat:` / `docs:` / `chore:`）
