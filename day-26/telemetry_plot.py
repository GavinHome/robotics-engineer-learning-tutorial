#!/usr/bin/env python3
"""Day 26 实验：远程拉取 ESP32-S3 的 /data，实时绘制距离曲线。

用法：
    ./.venv/bin/python day-26/telemetry_plot.py                       # 默认 esp32s3.local
    ./.venv/bin/python day-26/telemetry_plot.py --host 192.168.0.5    # mDNS 不灵时用 IP
    ./.venv/bin/python day-26/telemetry_plot.py --csv day-26/run.csv  # 同时落 CSV
    ./.venv/bin/python day-26/telemetry_plot.py --interval 1.0        # 改成 1 秒一轮

Ctrl+C 或关掉窗口即停，停时存一张 PNG。

为什么不走串口：指南 Day 26 原文写的是"打开串口、实时读取 JSON"，
但车自 Day 24 起脱离 USB 独立跑，自由跑时根本没有串口设备，
要采串口就得插线，插上线车又被拴住——实时绘图的意义正在于看它自由跑。
Day 25 的 Web Server 已经把同一份 JSON 放到了 HTTP 上，
所以这个脚本只需要 requests 拉一次 /data，固件一行都不用改。
真要挂着 USB 调试，串口打的中文行 "#699 28.0 cm" 已经够看；
机器读串口 JSON 那是 day-22/serial_logger.py 的活，不在这里重复。
"""

import argparse
import csv
import json
import math
import sys
import time
from collections import deque
from datetime import datetime

import matplotlib
import matplotlib.pyplot as plt
import requests

WINDOW_S = 60.0          # 横轴只看最近 60 秒，滚动窗口
MAX_POINTS = 2000        # 环形缓冲上限，防止跑一夜把内存吃光
OUT_OF_RANGE = -1.0      # 和固件约定一致：pulseIn 超时返回 -1
CM_FIELD = "cm"          # Day 25 /data 的字段名


def pick_cjk_font() -> None:
    """让标题里的中文能画出来。

    标题写的是中文，但 matplotlib 默认的 DejaVu Sans 没有汉字字形，
    存出来的 PNG 里会是一排豆腐块（并且刷一屏 Glyph missing 警告）。
    matplotlib 不内置 CJK 字体，只能从系统现成的里挑一个注册进去；
    挑不到就维持默认，让它自己降级。
    """
    try:
        from matplotlib import font_manager
    except ImportError:
        return
    candidates = ("PingFang SC", "Heiti SC", "Songti SC", "Arial Unicode MS",
                  "Noto Sans CJK SC", "Microsoft YaHei", "SimHei")
    installed = {f.name for f in font_manager.fontManager.ttflist}
    found = [n for n in candidates if n in installed]
    if found:
        plt.rcParams["font.sans-serif"] = found + list(plt.rcParams["font.sans-serif"])
        # unicode_minus 关掉：USI 负号在部分中文字体里也是缺的
        plt.rcParams["axes.unicode_minus"] = False


def parse_data(text: str) -> dict | None:
    """把 /data 的响应体转成记录，不是合法 JSON 就丢弃。

    板子重启、Wi-Fi 重连的瞬间可能吐半个响应体，过滤必须做。
    """
    text = text.strip()
    if not text.startswith("{"):
        return None
    try:
        d = json.loads(text)
    except json.JSONDecodeError:
        return None
    cm = d.get(CM_FIELD)
    if cm is None:
        return None
    return {"cm": float(cm), "raw": d}


def main() -> int:
    ap = argparse.ArgumentParser(description="远程实时绘制 ESP32-S3 距离曲线")
    ap.add_argument("--host", default="esp32s3.local", help="板子地址，默认 esp32s3.local")
    ap.add_argument("--interval", type=float, default=0.2,
                    help="轮询间隔秒，默认 0.2（固件每 200 ms 测一次，再快也是重复值）")
    ap.add_argument("--csv", default="", help="同时把数据落成 CSV")
    ap.add_argument("--png", default="day-26/telemetry_plot.png", help="停止时存的截图")
    args = ap.parse_args()

    host = (args.host.strip()
            .removeprefix("http://").removeprefix("https://").rstrip("/"))
    url = f"http://{host}/data"

    # 提早失败比"跑起来了但没窗口"好。.venv 的 Python 3.14 没装 tkinter，
    # TkAgg/QtAgg 都用不了；macOS 上 matplotlib 默认选的 macosx 后端是好的，
    # 所以这里只判断，不指定——指定反而会把能用的环境弄坏。
    NON_INTERACTIVE = {"agg", "pdf", "svg", "ps", "pgf", "cairo", "template"}
    if matplotlib.get_backend().lower() in NON_INTERACTIVE:
        sys.exit(f"当前 matplotlib 后端是 {matplotlib.get_backend()}（无界面）。"
                 "这个脚本要实时弹窗，SSH 或纯终端下跑不了。")

    pick_cjk_font()

    fig, ax = plt.subplots(figsize=(9, 4.5))
    (line,) = ax.plot([], [], lw=1.4, color="#2563eb")
    ax.set_xlabel("t (s)")
    ax.set_ylabel("distance (cm)")
    ax.grid(True, alpha=0.3)
    # y 轴下限钉在 0：距离不可能为负，自动缩放会把一片 -1 拉成难看的负区
    ax.set_ylim(0, 30)

    plt.ion()
    fig.show()

    xs: deque[float] = deque(maxlen=MAX_POINTS)
    ys: deque[float] = deque(maxlen=MAX_POINTS)
    t_start = time.monotonic()
    count = valid = timeouts = drops = 0
    csv_file = None
    writer = None

    if args.csv:
        # newline="" 是 csv 模块的硬性要求，否则 Windows 下会多出空行
        csv_file = open(args.csv, "w", newline="", encoding="utf-8")
        writer = csv.writer(csv_file)
        writer.writerow(["timestamp", "t_s", "cm", "rssi", "up", "n"])
        csv_file.flush()

    print(f"轮询 {url}，每 {args.interval:.2f} 秒一次（Ctrl+C 停止）")
    print("绘图窗口已弹出。")
    try:
        while True:
            t0 = time.monotonic()
            rec = None
            err = None
            try:
                r = requests.get(url, timeout=2.0)
                r.raise_for_status()
                rec = parse_data(r.text)
            except requests.RequestException as e:
                # 网络抖一下不该让整个采集中断——车还在跑，
                # 曲线断在那里，本身就是"这会儿连不上"的可视化。
                # 这里只记错误不计数：计数统一在下面 NaN 分支做一次，
                # 否则一次掉线会被算成两次
                rec = None
                err = f"{type(e).__name__}: {e}"

            t = time.monotonic() - t_start
            cm = rec["cm"] if rec else math.nan
            raw = rec["raw"] if rec else {}

            if math.isnan(cm) or cm == OUT_OF_RANGE:
                # 两种"没数"都不能当成 0 或负数画下去——0 会被读成"贴脸"，
                # -1 会在曲线底部凿一根假尖刺（Day 22 记过这个坑）。
                # 用 NaN 让线断在那里，空白本身就是信息：
                #   超时(-1) = 前方真有超声吸收面，这是有效测量结论
                #   NaN       = 根本没采到（网络失败或响应体不是 JSON）
                if cm == OUT_OF_RANGE:
                    timeouts += 1
                else:
                    drops += 1
                    if err:
                        print(f"\r#{count} {err}", end="", flush=True)
                cm_plot = math.nan
            else:
                valid += 1
                cm_plot = cm

            xs.append(t)
            ys.append(cm_plot)
            count += 1

            if writer:
                # CSV 存原始值：超时的 -1 要留着，-1 本身就是"前方有吸收面"
                # 这个测量结论；只有真没采到（NaN）才留空。
                # 图上画断线，文件里留原值——两处各取所需。
                writer.writerow([
                    datetime.now().isoformat(timespec="milliseconds"),
                    f"{t:.3f}",
                    "" if math.isnan(cm) else f"{cm:.1f}",
                    raw.get("rssi", ""),
                    raw.get("up", ""),
                    raw.get("n", ""),
                ])

            line.set_data(xs, ys)
            xmax = xs[-1] if xs else 1.0
            ax.set_xlim(max(0.0, xmax - WINDOW_S), max(WINDOW_S, xmax))

            known = [v for v in ys if not math.isnan(v)]
            if known:
                ymax = max(min(100.0, max(known) * 1.15 + 5.0), 20.0)
                ax.set_ylim(0, ymax)

            cur = "--" if math.isnan(cm_plot) else f"{cm_plot:.1f}"
            ax.set_title(
                f"{host}｜当前 {cur} cm｜有效 {valid}/{count}｜"
                f"超时 {timeouts}｜掉线 {drops}｜"
                f"窗口 {xmax - max(0.0, xmax - WINDOW_S):.0f}s",
                fontsize=11,
            )
            fig.canvas.draw_idle()
            plt.pause(0.001)

            # 用户直接关了窗口，别继续在后台跑
            if not plt.get_fignums():
                break

            if writer and count % 5 == 0:
                csv_file.flush()

            # 固件每 200 ms 才更新一次 lastCm，请求本身也要花时间，
            # 按剩余时间睡，别叠加成忙等
            spent = time.monotonic() - t0
            time.sleep(max(0.0, args.interval - spent))

    except KeyboardInterrupt:
        print("\n停止采集")
    finally:
        if csv_file:
            csv_file.close()
        try:
            fig.savefig(args.png, dpi=110, bbox_inches="tight")
            print(f"截图已存：{args.png}")
        except Exception as e:
            print(f"截图没存成：{e}", file=sys.stderr)

    print(f"共 {count} 次轮询：有效 {valid}、超时(有吸收面) {timeouts}、掉线(没采到) {drops}")
    if args.csv:
        print(f"CSV：{args.csv}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
