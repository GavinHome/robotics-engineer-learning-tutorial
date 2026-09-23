#!/usr/bin/env python3
"""把 Day 21 保存的串口日志解析成 CSV。

为什么不直接烧 Day 22 的遥测草图现场采：车上正跑着别的程序，不方便重烧。
而这些日志里的每一个数字都是当时真实测到的，不是构造出来的 —— 用它做数据源，
解析链路和落 CSV 的逻辑跟读活串口完全一致。

用法：
    .venv/bin/python day-22/log_to_csv.py
    .venv/bin/python day-22/log_to_csv.py day-21/实验3-单步调试-日志打印.txt

产出：day-22/sensor_log.csv
"""

import csv
import json
import re
import sys
from pathlib import Path

DEFAULT_LOG = "day-21/实验2-超声波避障-串口打印.txt"
MIN_VALID_CM = 2.0    # 与 Day 21 草图一致：低于 2cm 视为不可信读数

# 日志里两种有效读数行：
#   前方 25.9cm，通畅 → 直行前进
#   第1轮｜前方 6.1cm < 15cm，挡住了 → 后退 500ms
# 两条都要抓：第一条是"通畅时的采样"，第二条是"触发避障的那次读数"，
# 只抓第一条的话，小于 15cm 的近距离读数会全部漏掉 —— 那恰恰是最有价值的一段。
DIST_RE = re.compile(r"前方\s+([\d.]+)\s*cm")

# "超出量程（>100cm）"= 超时（-1）；"读数无效"= 低于 2cm 的不可信读数
OVER_RANGE_RE = re.compile(r"超出量程")
INVALID_RE = re.compile(r"读数无效")

FIELDS = ["timestamp", "ms", "dist_cm", "adc_raw", "voltage"]


def parse_line(line: str, seq: int) -> dict | None:
    """把一行中文日志转成遥测记录。解析不出距离就返回 None。"""
    # 每个采样点间隔 500ms（Day 21 草图 LOG_MS=500），据此还原时间戳
    ms = seq * 500

    if OVER_RANGE_RE.search(line):
        # 超时：前方超过约 100cm。用 -1，不用 0 —— 0 会被当成"贴脸"
        return {"timestamp": "", "ms": ms, "dist_cm": -1.0,
                "adc_raw": "", "voltage": ""}
    if INVALID_RE.search(line):
        # 低于 2cm 的不可信读数，同样不该混进有效距离里
        return {"timestamp": "", "ms": ms, "dist_cm": -2.0,
                "adc_raw": "", "voltage": ""}

    m = DIST_RE.search(line)
    if not m:
        return None

    cm = float(m.group(1))
    if cm < MIN_VALID_CM:
        return None   # 与草图判断保持一致

    return {"timestamp": "", "ms": ms, "dist_cm": cm,
            "adc_raw": "", "voltage": ""}


def main() -> int:
    path = Path(sys.argv[1] if len(sys.argv) > 1 else DEFAULT_LOG)
    out = Path("day-22/sensor_log.csv")

    try:
        # errors="replace"：串口偶发非法 UTF-8，别让一帧坏数据中断整轮
        text = path.read_text(encoding="utf-8", errors="replace")
    except FileNotFoundError:
        print(f"找不到日志：{path}", file=sys.stderr)
        return 1

    rows: list[dict] = []
    for line in text.splitlines():
        # 上电 ROM 头（"ESP-ROM:esp32s3-..."）和菜单说明行都不含距离，
        # parse_line 自然返回 None —— 这里和读活串口时用同一个"抓不到就跳过"的原则
        row = parse_line(line, len(rows))
        if row:
            rows.append(row)

    if not rows:
        print(f"{path} 里没解析出距离数据", file=sys.stderr)
        return 1

    with open(out, "w", newline="", encoding="utf-8") as f:
        writer = csv.DictWriter(f, fieldnames=FIELDS)
        writer.writeheader()
        writer.writerows(rows)

    real = [r["dist_cm"] for r in rows if r["dist_cm"] > 0]
    print(f"解析 {path}")
    print(f"  共 {len(rows)} 条记录，其中有效距离 {len(real)} 条")
    if real:
        print(f"  距离范围 {min(real):.1f} ~ {max(real):.1f} cm")
    print(f"  超时(-1) {sum(1 for r in rows if r['dist_cm'] == -1)} 条，"
          f"无效(-2) {sum(1 for r in rows if r['dist_cm'] == -2)} 条")
    print(f"→ {out}")

    # 顺带演示一次 json 序列化：CSV 是给人看的表，JSON 是给程序读的结构
    json_path = out.with_suffix(".json")
    json_path.write_text(
        json.dumps([{k: v for k, v in r.items() if v != ""} for r in rows],
                   ensure_ascii=False, indent=2),
        encoding="utf-8")
    print(f"→ {json_path}")

    return 0


if __name__ == "__main__":
    sys.exit(main())
