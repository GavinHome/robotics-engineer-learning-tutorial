#!/usr/bin/env python3
"""批量重命名文件：把中文/空格文件名规整成统一小写下划线格式。

用法（默认只演练，不真改）：
    .venv/bin/python day-22/batch_rename.py day-21
    .venv/bin/python day-22/batch_rename.py day-21 --apply     # 真正执行
    .venv/bin/python day-22/batch_rename.py day-21 --prefix day21

设计取舍：
① 默认 --dry-run。重命名是不可逆的，尤其本仓库照片是实测记录，
   改错了名字就对不上实验。先看清单确认无误再加 --apply。
② 遇到重名冲突就跳过并报告，不覆盖。宁可少改一个，也不能静默吞掉一张照片。
③ 只动文件不动目录，且跳过 .git / .venv，避免把仓库结构搞坏。
"""

import argparse
import re
import sys
import unicodedata
from pathlib import Path

SKIP_DIRS = {".git", ".venv", ".claude", "__pycache__"}

# 连续的分隔符压成一个下划线：空格、括号、中英文标点都算
SPLIT_RE = re.compile(r"[\s\(\)\[\]{}、，,。．\.\-—_—]+")


def normalize(name: str) -> str:
    """规整文件名：全角转半角、分隔符统一成下划线、去掉首尾多余符号。

    刻意保留中文。第一反应是把中文转成拼音或干脆删掉，但本仓库的文件名
    全是中文（"实验1-双电机差速"），删掉就只剩 "1"，改名反而丢失信息；
    转拼音要额外装 pypinyin，为一次批量改名引入依赖不划算。
    保留中文，让脚本专心解决它真正擅长的问题：空格、括号、全角数字、
    连续分隔符这些在命令行和 URL 里容易出错的字符。
    """
    # NFKC：全角字母数字（ＡＢＣ１２３）折成半角，是这一步真正的作用
    s = unicodedata.normalize("NFKC", name)
    s = SPLIT_RE.sub("_", s)
    s = re.sub(r"_+", "_", s).strip("_.")
    return s.lower()


def plan(files: list[Path], prefix: str) -> list[tuple[Path, Path]]:
    """算出 (旧路径, 新路径) 列表。已有目标名的一律跳过。"""
    jobs: list[tuple[Path, Path]] = []
    taken = {p.name for p in files}

    for p in files:
        stem = normalize(p.stem)
        if not stem:
            continue   # 空壳名（全是被剥掉的符号）—— 不动它，免得把文件改没
        new_name = f"{prefix}_{stem}{p.suffix}" if prefix else f"{stem}{p.suffix}"
        if new_name == p.name:
            continue   # 已经是规范名
        if new_name in taken:
            print(f"  跳过 {p.name}：目标名 {new_name} 已存在，不覆盖", file=sys.stderr)
            continue
        taken.add(new_name)
        jobs.append((p, p.with_name(new_name)))

    return jobs


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("dir", nargs="?", default=".", help="要整理的目录")
    ap.add_argument("--prefix", default="", help="新文件名前缀，如 day21")
    ap.add_argument("--apply", action="store_true", help="真正执行（默认只演练）")
    args = ap.parse_args()

    root = Path(args.dir)
    if not root.is_dir():
        print(f"不是目录：{root}", file=sys.stderr)
        return 1

    files = [p for p in sorted(root.rglob("*"))
             if p.is_file() and not (set(p.parts) & SKIP_DIRS)]
    if not files:
        print(f"{root} 下没有文件", file=sys.stderr)
        return 1

    jobs = plan(files, args.prefix)

    # 目标名互相冲突时（不同文件算出同一个新名）也过一遍，plan 已用 taken 挡住
    print(f"扫描 {root}：{len(files)} 个文件，需重命名 {len(jobs)} 个")
    for old, new in jobs:
        print(f"  {old.name}  →  {new.name}")

    if not args.apply:
        print("\n以上是演练（--dry-run）。确认无误后加 --apply 真正执行")
        return 0

    done = 0
    for old, new in jobs:
        try:
            old.rename(new)
            done += 1
        except OSError as e:
            print(f"  失败 {old.name}：{e}", file=sys.stderr)

    print(f"\n完成 {done}/{len(jobs)}")
    return 0 if done == len(jobs) else 1


if __name__ == "__main__":
    sys.exit(main())
