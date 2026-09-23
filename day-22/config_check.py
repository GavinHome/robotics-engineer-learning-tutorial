#!/usr/bin/env python3
"""解析 config.json 并做合法性校验。

用法：
    .venv/bin/python day-22/config_check.py
    .venv/bin/python day-22/config_check.py day-22/其他配置.json

要点：json.load 只保证"语法合法"，不保证"值合理"。
duty 写成 999、enter_cm 比 exit_cm 大这类错误照样能解析成功，
所以读完必须自己校验，否则错误会一路带到电机上才发现。
"""

import json
import sys
from pathlib import Path


def load_config(path: Path) -> dict:
    """读 JSON 配置。出错时把路径带上，方便定位是哪个文件坏了。"""
    try:
        with open(path, encoding="utf-8") as f:
            return json.load(f)
    except FileNotFoundError:
        raise SystemExit(f"配置文件不存在：{path}")
    except json.JSONDecodeError as e:
        # e 自带行号列号，比只说"解析失败"有用得多
        raise SystemExit(f"{path} 不是合法 JSON：第 {e.lineno} 行第 {e.colno} 列 {e.msg}")


def check(cfg: dict) -> list[str]:
    """返回问题清单。空列表 = 全部通过。

    刻意收集所有问题后一次性返回，而不是遇到第一个就抛异常——
    配置往往不止一处错，一次全报出来省得多跑几遍。
    """
    problems: list[str] = []

    car = cfg.get("car")
    if not isinstance(car, dict):
        problems.append("缺少 car 段")
        car = {}
    else:
        for key in ("duty_cruise", "duty_turn"):
            v = car.get(key)
            if not isinstance(v, int):
                problems.append(f"car.{key} 必须是整数，现在是 {v!r}")
            elif not 0 <= v <= 255:
                problems.append(f"car.{key}={v} 超出 0~255（PWM 8 位分辨率）")
        if abs(car.get("trim_right", 0)) > 255:
            problems.append(f"car.trim_right={car['trim_right']} 绝对值超过 255")

    avoid = cfg.get("avoid")
    if not isinstance(avoid, dict):
        problems.append("缺少 avoid 段")
        avoid = {}
    else:
        enter, exit_cm = avoid.get("enter_cm"), avoid.get("exit_cm")
        if not isinstance(enter, (int, float)) or not isinstance(exit_cm, (int, float)):
            problems.append("avoid.enter_cm / exit_cm 必须是数字")
        elif enter >= exit_cm:
            problems.append(
                f"迟滞方向反了：enter_cm({enter}) 必须小于 exit_cm({exit_cm})，"
                "否则车会在阈值附近反复触发又立刻解除，来回抽搐"
            )

    pins = cfg.get("pins")
    if isinstance(pins, dict):
        # ESP32-S3-WROOM-1 未引出的脚：26-32 内部 Flash，33/34 Octal PSRAM
        forbidden = set(range(22, 35)) | {43, 44}
        used: dict[int, str] = {}
        for name, pin in pins.items():
            if not isinstance(pin, int):
                problems.append(f"pins.{name} 必须是整数，现在是 {pin!r}")
                continue
            if pin in forbidden:
                problems.append(f"pins.{name}=GPIO{pin} 在本板上未引出或占用")
            if pin in used:
                problems.append(f"GPIO{pin} 被 {used[pin]} 和 {name} 重复占用")
            used[pin] = name
    else:
        problems.append("缺少 pins 段")

    return problems


def main() -> int:
    path = Path(sys.argv[1] if len(sys.argv) > 1 else "day-22/config.json")
    cfg = load_config(path)

    print(f"读取 {path}")
    print(f"  duty_cruise = {cfg.get('car', {}).get('duty_cruise')}")
    print(f"  避障阈值    = 进 {cfg.get('avoid', {}).get('enter_cm')}cm / "
          f"出 {cfg.get('avoid', {}).get('exit_cm')}cm")
    print(f"  串口        = {cfg.get('serial', {}).get('baud')} baud，"
          f"采样周期 {cfg.get('serial', {}).get('sample_ms')}ms")

    problems = check(cfg)
    if problems:
        print(f"\n发现 {len(problems)} 个问题：")
        for p in problems:
            print(f"  - {p}")
        return 1

    print("\n配置校验通过")
    return 0


if __name__ == "__main__":
    sys.exit(main())
