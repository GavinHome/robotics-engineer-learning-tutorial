#!/usr/bin/env python3
"""读取 ESP32-S3 串口的 JSON 遥测数据，存成 CSV。

用法：
    .venv/bin/python day-22/serial_logger.py                 # 自动找串口
    .venv/bin/python day-22/serial_logger.py /dev/cu.usbmodemXXX
    .venv/bin/python day-22/serial_logger.py --port ... --n 200   # 采 200 条就停

产出：day-22/sensor_log.csv
"""

import argparse
import csv
import glob
import json
import sys
from datetime import datetime

import serial

BAUD = 115200
FIELDS = ["timestamp", "ms", "dist_cm", "adc_raw", "voltage"]


def find_port() -> str | None:
    """macOS 下 ESP32-S3 的 USB CDC 口形如 /dev/cu.usbmodemXXX。"""
    cands = sorted(glob.glob("/dev/cu.usbmodem*") + glob.glob("/dev/cu.usbserial-*"))
    return cands[0] if cands else None


def open_serial(port: str) -> serial.Serial:
    # timeout=1：readline 最多等 1 秒，否则 Ctrl+C 要等很久才响应
    ser = serial.Serial(port, BAUD, timeout=1)
    # 打开串口会让 DTR 拉低复位一次 ESP32，等它重启完再读，
    # 否则前几行会混进 ROM 启动日志
    ser.reset_input_buffer()
    return ser


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("port", nargs="?", help="串口设备，省略则自动探测")
    ap.add_argument("--out", default="day-22/sensor_log.csv")
    ap.add_argument("--n", type=int, default=0, help="采集条数上限，0 = 不限")
    args = ap.parse_args()

    port = args.port or find_port()
    if not port:
        print("找不到串口。插上开发板，或用 ls /dev/cu.* 查名字后显式传入", file=sys.stderr)
        return 1

    try:
        ser = open_serial(port)
    except serial.SerialException as e:
        print(f"打不开 {port}：{e}", file=sys.stderr)
        return 1

    print(f"已连接 {port} @ {BAUD}，写入 {args.out}，Ctrl+C 停止")
    written = 0

    try:
        # newline="" 是 csv 模块在 Python 3 下的硬性要求，否则 Windows 会多出空行
        with open(args.out, "w", newline="", encoding="utf-8") as f, ser:
            writer = csv.DictWriter(f, fieldnames=FIELDS)
            writer.writeheader()

            while True:
                if args.n and written >= args.n:
                    break

                try:
                    raw = ser.readline()
                except serial.SerialException:
                    print("\n串口断开（板子被拔了？）", file=sys.stderr)
                    break

                if not raw:
                    continue

                # errors="replace"：波特率偶尔错半个周期会吐出非法 UTF-8，
                # 不处理的话整条循环会崩，而这只是丢一帧数据的事
                line = raw.decode("utf-8", errors="replace").strip()

                # 过滤：ESP32 上电 ROM 会先吐 "ESP-ROM:esp32s3-..." 这类非 JSON 行
                if not line.startswith("{"):
                    continue

                try:
                    data = json.loads(line)
                except json.JSONDecodeError:
                    # 半行/错位时偶发，跳过比中断整轮采集划算
                    continue

                row = {
                    "timestamp": datetime.now().isoformat(timespec="milliseconds"),
                    "ms": data.get("ms", ""),
                    # 超时用 -1，不是 0 —— 0 会被当成"贴脸"，画出来是根假尖刺
                    "dist_cm": data.get("dist_cm", ""),
                    "adc_raw": data.get("adc_raw", ""),
                    "voltage": data.get("voltage", ""),
                }
                writer.writerow(row)
                written += 1
                print(f"\r已存 {written} 条   ", end="", flush=True)

    except KeyboardInterrupt:
        print("\n停止")

    print(f"共 {written} 条 → {args.out}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
