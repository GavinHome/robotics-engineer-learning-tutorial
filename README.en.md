# Robotics Engineer Learning Tutorial

A hands-on, month-by-month robotics engineering curriculum. Starting from zero electronics experience, progressing to ROS2, control theory, and job-ready robotics skills in 6 months.

## What's Inside

| Folder | Description |
|--------|-------------|
| [`教程/`](./%E6%95%99%E7%A8%8B/) | Original 1–6 month article-style learning plans |
| [`进度/`](./%E8%BF%9B%E5%BA%A6/) | Day-by-day practical extension of Month 1 (30 days) + terminology glossary |

This repo doubles as a public learning journal. Every experiment, circuit, and robot project is documented with photos, schematics, code, and a dedicated "What Went Wrong & How I Fixed It" section.

---

## 6-Month Roadmap

| Month | Focus |
|-------|-------|
| 1 | Electronics fundamentals, breadboarding, soldering, ESP32-S3, sensors, DC motors |
| 2 | Microcontrollers, motor drivers, servo/stepper control, IMU, ultrasonic ranging |
| 3 | Mechanical design, CAD, 3D printing, laser cutting, chassis fabrication |
| 4 | ROS2 simulation, Gazebo, Nav2, enterprise dev practices |
| 5 | Control theory, PID tuning, Kalman filters, math foundations |
| 6 | LeRobot, RL for robotics, specialization tracks, portfolio & job hunt |

---

## Month 1 at a Glance

- **Week 1** — Circuit theory & simulation (Falstad, Tinkercad, All About Circuits)
- **Week 2** — ESP32-S3 GPIO, PWM, ADC, serial communication
- **Week 3** — Soldering, HC-SR04 ultrasonic, MPU-6050 IMU, DC motors + TB6612 driver
- **Week 4** — Python scripts, Git/GitHub workflow, Wi-Fi remote control car capstone

Budget: **$0–$90** depending on tier (simulator-only → full ESP32 starter kit).

---

## Day 1 — Ohm's Law & Voltage Divider

### Goal
Understand voltage, current, and resistance. Build and simulate your first circuits. Verify Ohm's Law with real measurements.

### Screenshots
- **Falstad simulation — single resistor (10 kΩ @ 5 V):** [`day-01/1.1 第一个仿真电路.png`](day-01/1.1%20第一个仿真电路.png)

### Formulas

**1. Ohm's Law**
```
V = I × R
```
- `V` = voltage in volts (V)
- `I` = current in amperes (A)
- `R` = resistance in ohms (Ω)

> From my Falstad simulation: 5 V across 10 kΩ → I = 5 V / 10000 Ω = **0.5 mA (500 µA)**

**2. Voltage Divider**
```
Vout = Vin × R2 / (R1 + R2)
```
- `R1` = top resistor (closest to Vin)
- `R2` = bottom resistor (closest to GND)

> Example: Vin = 5 V, R1 = 1 kΩ, R2 = 2 kΩ → Vout = 5 × 2 / 3 = **3.33 V**

**3. Power Dissipation**
```
P = V × I = I² × R = V² / R
```
- `P` = power in watts (W)
- Most through-hole resistors are rated 1/4 W (0.25 W)

> From my Falstad simulation: 5 V across 10 kΩ → P = 25 / 10000 = **2.5 mW** (well within 1/4 W rating)

**4. Series / Parallel Resistance**
```
Series:     R_eq = R1 + R2 + ...
Parallel:   1 / R_eq = 1/R1 + 1/R2  →  R_eq = (R1 × R2) / (R1 + R2)
```

**5. Kirchhoff's Current Law (KCL)**
```
Σ I_in = Σ I_out   at any node
```

**6. RC Time Constant**
```
τ = R × C
```
- After ~5τ, a capacitor is >99% charged/discharged
- `R` in ohms, `C` in farads → `τ` in seconds

### Quick Reference

| Symbol | Meaning | Unit |
|--------|---------|------|
| `V` | Voltage | Volt (V) |
| `I` | Current | Ampere (A) |
| `R` | Resistance | Ohm (Ω) |
| `P` | Power | Watt (W) |
| `C` | Capacitance | Farad (F) |
| `τ` | Time constant | Second (s) |

### Files in This Repo

```
robotics-engineer-learning-tutorial/
├── README.md                    ← you are here
├── docs/                        ← ESP32-S3 board reference (schematic + pinout)
├── day-01/                      ← today's work (screenshots + notes)
└── ...
```

---

## Learning Journal Policy

Every day's work lives in a dated folder and includes:

1. **What I built** (photos, GIFs, schematics)
2. **Code** (well-commented, compiles/runs without errors)
3. **Test results** (measured values, oscilloscope captures, serial logs)
4. **What went wrong & how I fixed it** — the most important section
5. **References** (datasheets, tutorial links, forum threads that helped)

---

## Hardware

See [`教程/第1月-电子学与工作台.md`](./%E6%95%99%E7%A8%8B/%E7%AC%AC1%E6%9C%88-%E7%94%B5%E5%AD%90%E5%AD%A6%E4%B8%8E%E5%B7%A5%E4%BD%9C%E5%8F%B0.md) and [`进度/第1月-30天逐日指南.md`](./%E8%BF%9B%E5%BA%A6/%E7%AC%AC1%E6%9C%88-30%E5%A4%A9%E9%80%90%E6%97%A5%E6%8C%87%E5%8D%97.md) for full BOM and budget tiers.

---

## Resources

- Falstad Circuit Simulator: https://www.falstad.com/circuit/
- Tinkercad Circuits: https://www.tinkercad.com/circuits
- All About Circuits Textbook: https://www.allaboutcircuits.com/textbook/
- ESP32-S3 Docs: https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/

---

## License

This learning journal is personal and for educational purposes. Third-party resources belong to their respective owners.
