# Robotics Engineer Learning Tutorial

A hands-on, month-by-month robotics engineering curriculum. Starting from zero electronics experience, progressing to ROS2, control theory, and job-ready robotics skills in 6 months.

## What's Inside

| Folder | Description |
|--------|-------------|
| [`教程/`](./教程/) | Original 1–6 month article-style learning plans |
| [`进度/`](./进度/) | Day-by-day practical extension of Month 1 (30 days) + terminology glossary |
| [`docs/`](./docs/) | ESP32-S3 board source material (schematic + pinout) + component photos ([`元器件.jpg`](./docs/元器件.jpg)) + `小车模块分工表.md` (role of each Day 17–21 module in the finished robot) |
| [`day-01/`](./day-01/) … [`day-22/`](./day-22/) | Daily work (screenshots, circuit files, code, notes) |

> 📌 **Code convention (from Day 10)**: later experiments are written as a single `loop()` running in parallel with the onboard pixel (one `RgbCycle::update()` call plus a `millis()` test per task) — no more separate "LED-only" sketches.

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
- **Week 3** — Soldering, HC-SR04 ultrasonic, MPU-6050 IMU, DC motors + TB6612 driver, SG90 servo
- **Week 4** — Python scripts, Git/GitHub workflow, Wi-Fi remote control car capstone

Budget: **$0–$90** depending on tier (simulator-only → full ESP32 starter kit).

---

## Day 1 — Ohm's Law & Voltage Divider

### Goal
Understand voltage, current, and resistance. Build and simulate your first circuits. Verify Ohm's Law with real measurements.

### Screenshots
- **Falstad simulation — single resistor (10 kΩ @ 5 V):** [`day-01/1.1 第一个仿真电路.png`](day-01/1.1 第一个仿真电路.png)

### Measured Data

| Resistance | Current (I) | Power (P) | Verification |
|------------|-------------|-----------|--------------|
| 1 kΩ | 5 mA | 25 mW | I = 5V/1kΩ = 5mA, P = V×I = 25mW ✅ |
| 220 Ω | 22.727 mA | 113.636 mW | I = 5V/220Ω ≈ 22.7mA, P = 25/220 ≈ 113.6mW ✅ |
| 10 kΩ | 500 µA | 2.5 mW | I = 5V/10kΩ = 0.5mA, P = 25/10000 = 2.5mW ✅ |

> All measured values match Ohm's Law I = V/R and power formula P = V²/R exactly.

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
├── README.md                    ← you are here (Chinese)
├── README.en.md                 ← English version
├── 教程/                        ← original 1–6 month learning plans
├── 进度/                        ← Month 1 day-by-day guide + glossary
├── docs/                        ← ESP32-S3 board reference (schematic + pinout) + component photos (元器件.jpg) + 小车模块分工表.md
├── 元器件库存清单.md            ← parts on hand + shopping list
├── day-01/ … day-07/            ← Week 1: circuit theory & simulation
├── day-08/                      ← Day 8: ESP32-S3 board & toolchain setup
├── day-09/                      ← Day 9: first program, Blink (onboard WS2812B + external LED)
├── day-10/                      ← Day 10: PWM breathing LED (duty-cycle brightness + two parallel tasks)
├── day-11/                      ← Day 11: Digital input & button (INPUT_PULLUP + debounce)
├── day-12/                      ← Day 12: ADC & sensor reading (potentiometer + LDR + Serial Plotter)
├── day-13/                      ← Day 13: Serial communication & debugging (UART + Serial.printf + JSON output)
├── day-14/                      ← Day 14: Week 2 Project - Digital voltmeter (divider + averaging + calibration)
├── day-15/                      ← Day 15: Soldering safety & basic practice (tinning + 5-step method + cold joints + continuity)
├── day-16/                      ← Day 16: Through-hole soldering (perfboard layout + shared-hole series chain + elevated 3 mm lead bend + segment-sum self-check)
└── day-17/                      ← Day 17: HC-SR04 ultrasonic ranging (Trig/Echo timing + pulseIn + speed-of-sound conversion)
└── day-18/                      ← Day 18: MPU-6050 IMU over I2C (bus addressing + accel/gyro + tilt from gravity)
└── day-19/                      ← Day 19: DC motors with the TB6612FNG driver (H-bridge truth table + fwd/rev/brake/coast + PWM speed control)
└── day-20/                      ← Day 20: SG90 servo control (50 Hz pulse protocol + angle positioning + dedicated 5 V supply)
├── day-21/                      ← Day 21: Robot car chassis (differential steering + motion functions + ultrasonic obstacle-avoidance state machine)
└── day-22/                      ← Day 22: Python refresher (serial JSON telemetry → CSV / config validation / batch rename)
```

---

## Day 2 — Series & Parallel Circuits

### Goal
Compute equivalent resistance and understand voltage division.

### Screenshots

- **Falstad — series (1 kΩ + 2 kΩ @ 5 V):** [`day-02/1kΩ-串联-2kΩ.png`](day-02/1kΩ-串联-2kΩ.png)
- **Falstad — parallel (1 kΩ // 1 kΩ @ 5 V):** [`day-02/1kΩ-并联-1kΩ.png`](day-02/1kΩ-并联-1kΩ.png)
- **Tinkercad — series (voltmeter in parallel reads 3.33V, ammeter in series reads 1.67mA):** [`day-02/电路-串联-tinkercad.png`](day-02/电路-串联-tinkercad.png)
- **Tinkercad — parallel (voltage 5V, current 10mA, equivalent resistance 500Ω):** [`day-02/电路-并联-tinkercad.png`](day-02/电路-并联-tinkercad.png)

### Falstad Circuit Files

- **Series template:** [`day-02/电路-1kΩ-串联-2kΩ.txt`](day-02/电路-1kΩ-串联-2kΩ.txt) — 1 kΩ + 2 kΩ in series across 5V
- **Parallel template:** [`day-02/电路-1kΩ-并联-1kΩ.txt`](day-02/电路-1kΩ-并联-1kΩ.txt) — two 1 kΩ in parallel across 5V
  - Import method same as Day 1

### Measured Data

**1. Series (1 kΩ + 2 kΩ)**

| Parameter | Measured | Calculated | Verified |
|------|--------|--------|------|
| Total resistance | 3 kΩ | 1k + 2k = 3k Ω | ✅ |
| Total current | 1.667 mA | 5V / 3kΩ ≈ 1.667mA | ✅ |
| Midpoint voltage (Vout) | 3.333 V | 5 × 2k / (1k+2k) = 3.33V | ✅ |
| 1kΩ power | — | I² × R = (1.667m)² × 1k ≈ 2.78mW | — |
| 2kΩ power | — | I² × R = (1.667m)² × 2k ≈ 5.56mW | — |

> Voltage divider check: Vout = Vin × R2 / (R1 + R2) = 5 × 2000 / 3000 = **3.333 V**

**2. Parallel (1 kΩ // 1 kΩ)**

| Parameter | Measured | Calculated | Verified |
|------|--------|--------|------|
| Equivalent resistance | 500 Ω | (1k × 1k) / (1k + 1k) = 500Ω | ✅ |
| Total current | 10 mA | 5V / 500Ω = 10mA | ✅ |
| Per-branch current | 5 mA | 5V / 1kΩ = 5mA | ✅ |

> Parallel check: 1/Req = 1/R1 + 1/R2 → Req = 500 Ω, total current 10mA = 5V/500Ω ✅

### Formulas

**Voltage divider**
```
Vout = Vin × R2 / (R1 + R2)
```
- For series circuits; R2 is the lower resistor (closer to GND)

**Series equivalent resistance**
```
R_eq = R1 + R2 + ...
```

**Parallel equivalent resistance**
```
1 / R_eq = 1/R1 + 1/R2
or
R_eq = (R1 × R2) / (R1 + R2)
```

---

## Day 3 — Kirchhoff's Laws (KCL/KVL) & Power

### Goal
Understand KCL (Kirchhoff's Current Law) and KVL (Kirchhoff's Voltage Law), compute per-component power dissipation, and grasp what a resistor's power rating means.

### Screenshots

- **Tinkercad — KVL (series divider 1 kΩ + 2 kΩ @ 5 V):** [`day-03/电路A-KVL.png`](day-03/电路A-KVL.png)
- **Tinkercad — KCL (parallel split 1 kΩ // 1 kΩ @ 5 V):** [`day-03/电路B-KCL.png`](day-03/电路B-KCL.png)

### Measured Data

**1. KVL check — series (1 kΩ + 2 kΩ)**

| Parameter | Measured | Calculated | Verified |
|------|--------|--------|------|
| Total resistance | 3 kΩ | 1k + 2k = 3k Ω | ✅ |
| Total current | 1.667 mA | 5V / 3kΩ ≈ 1.667mA | ✅ |
| 1kΩ voltage (V1) | 1.667 V | I × R = 1.667m × 1k ≈ 1.667V | ✅ |
| 2kΩ voltage (V2) | 3.333 V | I × R = 1.667m × 2k ≈ 3.333V | ✅ |
| V1 + V2 | 5.000 V | 1.667 + 3.333 = 5V | ✅ |
| 1kΩ power | 2.78 mW | I² × R = (1.667m)² × 1k ≈ 2.78mW | — |
| 2kΩ power | 5.56 mW | I² × R = (1.667m)² × 2k ≈ 5.56mW | — |

> KVL check: going once around the loop, the voltage drops sum to 1.667V + 3.333V = 5V = source voltage ✅

**2. KCL check — parallel (1 kΩ // 1 kΩ)**

| Parameter | Measured | Calculated | Verified |
|------|--------|--------|------|
| Equivalent resistance | 500 Ω | (1k × 1k) / (1k + 1k) = 500Ω | ✅ |
| Total current | 10 mA | 5V / 500Ω = 10mA | ✅ |
| Branch 1 current | 5 mA | 5V / 1kΩ = 5mA | ✅ |
| Branch 2 current | 5 mA | 5V / 1kΩ = 5mA | ✅ |
| I_total = I1 + I2 | 10 mA | 5mA + 5mA = 10mA | ✅ |
| Power per branch | 25 mW | V² / R = 25 / 1k = 25mW | — |

> KCL check: total current into the node = sum of branch currents out = 10mA ✅

**3. Power comparison (understanding power ratings)**

| Resistance | Power at 5V | 1/4W rating | Safe? |
|--------|----------------|-------------|----------|
| 1 kΩ | 25 mW | 250 mW | ✅ safe (only 10%) |
| 100 Ω | 250 mW | 250 mW | ⚠️ at the limit (100%) |

> A 100Ω resistor dissipating 250mW at 5V exactly equals its 1/4W rating. In practice you want margin — pick a resistor rated for at least 2× the actual dissipation.

### Formulas

**KVL (Kirchhoff's Voltage Law)**
```
∑V = 0 (around a closed loop, voltage rises = voltage drops)
V_source = V1 + V2 + ...
```

**KCL (Kirchhoff's Current Law)**
```
∑I = 0 (at any node, current in = current out)
I_total = I1 + I2 + ...
```

**Power**
```
P = V × I = I² × R = V² / R
```

---

## Day 4 — RC Circuits & Capacitor Charging

### Goal
Understand the RC time constant, observe the exponential charge/discharge curve of a capacitor, and master τ = R × C.

### Tool Choice
- **Falstad**: its built-in oscilloscope shows the capacitor voltage exponential in real time

### Screenshots

- **Falstad — RC charging circuit (1 kΩ + 10 μF @ 5 V, scope shows the exponential curve):** [`day-04/电容电压变化.png`](day-04/电容电压变化.png)

### Falstad Circuit File

- **RC charging template:** [`day-04/电路-电容电压变化.txt`](day-04/电路-电容电压变化.txt)
  - R = 1 kΩ, C = 10 μF, supply = 5 V
  - Import: open https://www.falstad.com/circuit/ → File → Import → paste the file contents
  - Scope already configured: channel 0 = capacitor voltage, channel 3 = supply

### Measured Data (simulation)

| Parameter | Value | Unit |
|------|------|------|
| Resistance (R) | 1,000 | Ω |
| Capacitance (C) | 10 | μF |
| Supply (Vcc) | 5 | V |
| Time constant τ | 10 | ms |
| 5τ (≈99% charged) | 50 | ms |

> τ = R × C = 1,000 Ω × 10 μF = 1,000 × 10×10⁻⁶ = 0.01 s = **10 ms**
>
> After 5τ = 50 ms the capacitor reaches about 99% of the supply (Vc ≈ 5V × (1 - e⁻⁵) ≈ 4.97 V)

### Simulation Observations

- **Initial state (t = 0):** capacitor voltage = 0 V (uncharged)
- **τ = 10 ms:** ≈ 5V × (1 - e⁻¹) ≈ **3.16 V** (63.2%)
- **2τ = 20 ms:** ≈ **4.32 V** (86.5%)
- **3τ = 30 ms:** ≈ **4.75 V** (95%)
- **4τ = 40 ms:** ≈ **4.91 V** (98.2%)
- **5τ = 50 ms:** ≈ **4.97 V** (99.3%)

### Formulas

**RC charging**
```
Vc(t) = Vcc × (1 - e^(-t/τ))
```
- `Vc(t)` = capacitor voltage at time t (V)
- `Vcc` = supply voltage (V)
- `t` = time (s)
- `τ` = time constant = R × C (s)

**RC discharging**
```
Vc(t) = V0 × e^(-t/τ)
```
- `V0` = initial capacitor voltage (V)
- During discharge the capacitor releases its stored energy through the resistor

**Time constant rule**
```
τ = R × C
5τ ≈ 99% charged/discharged
```

### Key Observations

1. **Exponential rise:** capacitor voltage is not linear — it grows fast at first, then slows
2. **Physical meaning of τ:** the time for the capacitor to reach 63.2% of the way up
3. **Larger R charges slower:** R up → τ up → flatter curve
4. **Larger C charges slower:** C up → τ up → longer charge time

### R, C Combination Comparison

| Experiment | R | C | τ = R×C | Full charge (5τ) | Simulation |
|------|--------|--------|---------|---------------|----------|
| ① baseline | 1 kΩ | 10 μF | 10 ms | 50 ms | medium speed |
| ② small R | 100 Ω | 10 μF | 1 ms | 5 ms | charged in about 5 ms |
| ③ large R | 10 kΩ | 10 μF | 100 ms | 500 ms | charged in about 0.5 s |
| ④ large C | 1 kΩ | 100 μF | 100 ms | 500 ms | charged in about 0.5 s (same τ as ③) |
| ⑤ small C | 1 kΩ | 1 μF | 1 ms | 5 ms | charged in about 5 ms |

> **Key findings:**
> - τ is proportional to R: 10× R → 10× τ
> - τ is proportional to C: 10× C → 10× τ
> - Different R, C pairs with the same τ produce an identical curve shape (just a rescaled time axis)
> - **Rule of thumb: after about 5τ the capacitor is essentially fully charged (99%)**

---

## Day 5 — Diodes & the Transistor as a Switch

### Goal
Understand a diode's forward voltage drop, learn to size a current-limiting resistor, and grasp the transistor as an electronic switch.

### Tool Choice
- **Tinkercad**: lets you drop visible multimeters into the schematic so one screenshot shows every voltage, current and resistance at once

### Screenshots

- **Tinkercad — diode/LED current-limiting circuit:** [`day-05/二极管.png`](day-05/二极管.png)
- **Tinkercad — NPN transistor switch:** [`day-05/晶体管.png`](day-05/晶体管.png)

### Measured Data (Tinkercad simulation)

**1. Plain diode (1N4007)**

| Parameter | Calculated | Measured | Verified |
|------|--------|--------|------|
| Supply voltage | 5 V | 5 V | ✅ |
| Diode drop | ~0.7 V | 0.57 V | ✅ (Tinkercad's model runs low; trust bench measurements in real circuits) |
| Loop current | (5-0.7)/1000 ≈ 4.3 mA | 4.43 mA | ✅ |
| Resistor power | I²×R ≈ 19.6 mW | — | — |

> Current-limiting formula: R = (Vin - Vd) / Id = (5 - 0.57) / 0.00443 ≈ **1,000 Ω**

**2. LED (light-emitting diode)**

| Parameter | Calculated | Measured | Verified |
|------|--------|--------|------|
| Supply voltage | 5 V | 5 V | ✅ |
| LED drop | red LED ≈ 1.8-2.2 V | 1.89 V | ✅ |
| Loop current | (5-1.89)/1000 ≈ 3.1 mA | 3.11 mA | ✅ |
| Resistor power | I²×R ≈ 9.7 mW | — | — |

> Current-limiting formula: R = (Vin - Vled) / Iled = (5 - 1.89) / 0.00311 ≈ **1,000 Ω**

**3. Diode vs LED**

| Property | Plain diode | LED |
|------|-----------|-----|
| Forward drop | 0.57 V | 1.89 V |
| Emits light | No | Yes (red) |
| Typical use | Rectification, protection | Indicator, display |
| Loop current | 4.43 mA | 3.11 mA |

**4. NPN transistor switch (2N3904)**

| Parameter | Calculated | Measured | Verified |
|------|--------|--------|------|
| Base current Ib | (3.3-0.7)/1000 ≈ 2.6 mA | 2.59 mA | ✅ |
| Collector current Ic | ~3-5 mA | 3.1 mA | ✅ |
| LED voltage | ~1.8-2.2 V | 1.89 V | ✅ |
| C-E drop Vce | ~0.1-0.3 V (saturated) | 6.91 mV | ✅ |
| Current gain β | Ic/Ib ≈ 1.2 | 3.1/2.59 ≈ 1.2 | ✅ (switch mode, not amplification) |

> **Key findings:**
> - The C-E drop is only **6.91 mV**, meaning the transistor is fully saturated — effectively an ideal closed switch
> - Collector current (3.1mA) < base current (2.59mA), because here the transistor is used as a **switch**, not an amplifier
> - In switch mode β is meaningless: the transistor is either fully on (tiny C-E drop) or fully off

### Formulas

**Current-limiting resistor**
```
R = (Vin - Vd) / I
```
- `Vin` = supply voltage (V)
- `Vd` = diode/LED forward drop (V)
- `I` = desired current (A)

**Power**
```
P = I² × R
```

### Key Observations

1. **A diode's forward drop is roughly fixed**: small current changes barely move it (plain diode ~0.6-0.7V, LED ~1.8-3.3V)
2. **The series resistor protects the diode**: without it, excessive current destroys the diode
3. **An LED needs a larger limiting resistor**: its higher drop means less power in the resistor at the same current
4. **Tinkercad models differ from theory**: the plain diode measured 0.57V versus the 0.7V textbook value — perfectly acceptable for learning
5. **Transistor switching behaviour**: when on, the C-E drop is tiny (6.91mV), equivalent to an ideal closed switch
6. **Base current controls the switch**: with 2.59mA into the base the transistor conducts and the LED lights; with no base current the LED is dark
7. **A small current controls a large one**: a tiny base current (2.59mA) controls the collector loop (3.1mA), giving level shifting and switching
8. **Why a 3.3V GPIO cannot drive a 12V motor directly**:
   - ESP32-S3 GPIO sources roughly **20-40 mA** at **3.3V**
   - A 12V motor typically draws **500 mA-2A**
   - The GPIO can supply neither the current nor the voltage
   - **Solution**: GPIO → transistor base (small current control), 12V supply → collector (motor load), emitter → GND
   - The transistor acts as a "switch", using a small current to control a large one — level shifting plus power gain

---

## Day 6 — Breadboards & Multimeter Basics

### Goal
Learn to build real circuits on a breadboard, master the internal connection rules, power the breadboard by cutting a USB cable, and light a first LED.

### Parts

| Component | Spec | Qty |
|--------|------|------|
| LED | white | 1 |
| Resistor | 1 kΩ | 1 |
| Breadboard | 830-point | 1 |
| Jumpers | solderless | 2 |
| USB cable | old A-to-C/Micro, cut open for power | 1 |
| Charger | 5V USB-A output | 1 |
| Button | momentary | 1 |

### Breadboard Photo

- **Overall wiring:** [`day-06/点亮LED-充电头USB剪线供电.png`](day-06/点亮LED-充电头USB剪线供电.png)

### Power Topology: Cutting a USB Cable

```
[wall charger 5V] ── USB-A plug
                        │
                  [old USB cable, cut]
                        │
                red ──→ +5V ──→ breadboard + rail
                black ──→ GND ──→ breadboard − rail
                        │
              + rail → 1kΩ → LED → button → − rail
```

**Steps**:
1. Find an old USB charging cable (A-to-C or A-to-Micro)
2. Cut off the **phone-side end**, keeping the USB-A plug
3. Strip the jacket and find the **red (+5V)** and **black (GND)** wires; the white/green data wires are unused
4. Wire the breadboard first: red into the + rail, black into the − rail
5. Then the LED circuit: + rail → 1kΩ → LED long leg → LED short leg → button → − rail
6. **Only then** plug the USB-A end into the charger

### Measurements (2026-09-08, multimeter)

| Measurement point | Expected | Measured | Notes |
|--------|--------|--------|------|
| Supply output voltage | ≈5.0V | **5.0V** | USB standard 5V |
| + rail to − rail | ≈5.0V | **5.0V** | Breadboard power rails continuous |
| Voltage across LED | ≈1.8V | **1.8V** | Red LED forward drop (roughly fixed once conducting) |
| Loop current | ≈3.2mA | **≈3.2mA** | Meter in series |
| Voltage across resistor | ≈3.2V | **3.2V** | V = I × R = 3.2mA × 1kΩ |
| Resistor power | ≈10mW | **≈10.2mW** | Calculated: P = I² × R = (3.2mA)² × 1kΩ |

**Ohm's law check**:
```
Vtotal = Vresistor + Vled = 3.2V + 1.8V = 5.0V ✓
I = V / R = 3.2V / 1kΩ ≈ 3.2mA ✓
P = V × I = 3.2V × 3.2mA ≈ 10.2mW ✓
```

> Note: power is a calculated value (a multimeter cannot measure power directly; it is derived from voltage and current), not a direct meter reading.

### Formulas

**Current-limiting resistor**
```
R = (Vin - Vled) / Iled
```
- `Vin` = supply voltage (V)
- `Vled` = LED forward drop (V)
- `Iled` = desired LED current (A)

**Ohm's law**
```
I = V / R
```

**Power**
```
P = I² × R
```

### Key Observations

1. **Breadboard power rails run lengthwise**: everything plugged into the + rail is connected, and likewise for the − rail
2. **The long LED leg is the anode (+), the short leg the cathode (−)**; reversing it just fails to light, without damage
3. **A current-limiting resistor must be in series**: connecting 5V straight to an LED burns it out
4. **At 5V with a 1kΩ resistor the current is about 3.2mA** — safe for the LED and bright enough
5. **A power bank's Type-C port may not supply protocol-less devices**; a wall charger is more reliable
6. **MB102 power module failure signature**: green LED off = no output; swapping cables and supplies changes nothing = the module itself is faulty

### Problems Encountered

**1. MB102 power module's green LED won't light**

- Tried several USB cables and several supplies (power bank, wall charger)
- Tested the same cable on a phone and it charged fine → cable and supply are good
- Conclusion: the module itself is faulty (cold USB socket or internal break); awaiting a multimeter verdict before requesting a replacement

**2. Button that "press to stay on, press again to turn off"**

- Current state: a momentary button only holds while pressed, releasing turns it off
- Plan: use a latching tactile switch, or have ESP32 code detect the toggle

---

## Day 7 — Week 1 Review & Q&A

### Goal
Consolidate the first six days by working circuit problems by hand and checking them in Tinkercad, then tidy up a list of open questions.

### Task 1: Hand-calculated Circuit Problems

> These five cover series/parallel, voltage division, KCL, RC, and transistors — all built on the first six days.
> Assumptions: white LED forward drop ≈1.8V, NPN silicon Vbe ≈0.7V, β = 100.

**Problem 1 — Series circuit**
5V supply, 1kΩ resistor in series with a white LED (Vf≈1.8V). Find the loop current and the voltage across the resistor.

![Problem 1 - series](day-07/题目1-串联电路.svg)

**Solution:**

> 1. Let the loop current be I (unknown)
> 2. Around the loop: Vin = Vr + Vled → 5V = I × 1kΩ + 1.8V
> 3. Solve: I = (5V - 1.8V) / 1kΩ = 3.2V / 1kΩ = **3.2mA**
> 4. Resistor voltage: Vr = I × R = 3.2mA × 1kΩ = **3.2V**

**Problem 2 — Parallel circuit**
5V supply, 3 LEDs in parallel, each with a 330Ω series resistor. Find the total current (LED drop taken as 1.8V).

![Problem 2 - parallel](day-07/题目2-并联电路.svg)

**Solution:**

> 1. In parallel each branch sees the full supply: V_branch = 5V
> 2. KVL per branch: Vr + Vled = 5V → Vr = 5V - 1.8V = 3.2V
> 3. Per-branch current: I_branch = Vr / R = 3.2V / 330Ω ≈ 9.7mA
> 4. Total current (KCL): I_total = I1 + I2 + I3 = 9.7mA × 3 = **29mA**

**Problem 3 — Voltage divider**
5V supply, 1kΩ + 2kΩ in series. Find the voltage across the 2kΩ, then the actual voltage across a 10kΩ load connected in parallel with it.

![Problem 3 - voltage divider](day-07/题目3-分压电路.svg)

**Solution:**

**Part 1: unloaded divider**

1. Series current is equal:
   ```
   I = 5V / (1kΩ + 2kΩ) = 5V / 3kΩ ≈ 1.67mA
   ```
2. Voltage across the 2kΩ:
   ```
   V2 = I × 2kΩ = 1.67mA × 2kΩ ≈ 3.33V
   ```
   Or directly from the divider formula: V2 = 5V × 2kΩ / 3kΩ = 3.33V

**Part 2: with a 10kΩ load (two methods)**

**Method 1: equivalent resistance**

1. Parallel equivalent:
   ```
   Req = (R2 × Rload) / (R2 + Rload) = (2kΩ × 10kΩ) / (2kΩ + 10kΩ) ≈ 1.667kΩ
   ```
2. Total current:
   ```
   I = 5V / (1kΩ + 1.667kΩ) ≈ 1.875mA
   ```
3. Load voltage (= voltage across the equivalent resistance):
   ```
   V_load = I × Req = 1.875mA × 1.667kΩ ≈ 3.125V
   ```

**Method 2: node voltage (KCL + KVL)**

1. Let the parallel section voltage be V2 and the 1kΩ voltage be V1
2. KCL: current in = current out
   ```
   V1 / 1kΩ = V2 / 2kΩ + V2 / 10kΩ
   ```
3. KVL:
   ```
   V1 + V2 = 5V
   ```
4. Substituting and solving:
   ```
   V2 ≈ 3.125V
   ```

**Problem 4 — KCL (Kirchhoff's Current Law)**
At one node, two branch currents are known: 2mA (in) and 3mA (in). Find the third branch current (out).

![Problem 4 - KCL](day-07/题目4-KCL.svg)

**Solution:**

```
current in = current out
2mA + 3mA = 5mA
```

The third branch current is **5mA**.

**Problem 5 — RC circuit + transistor**
5V supply, 1kΩ resistor in series with a 100µF capacitor. Find the time to charge to 63.2% (τ = RC).
Then: an NPN base connects through 10kΩ to 3.3V, emitter to ground, collector to an LED + 330Ω to 5V. Determine whether the LED lights (assume β=100, Vbe≈0.7V).

![Problem 5 - RC and transistor](day-07/题目5-RC与晶体管.svg)

**Solution:**

**Part 1: RC charging time**

1. Kirchhoff's Voltage Law (KVL):
   ```
   Vin = I × R + Vc
   ```
2. Capacitor current-voltage relation:
   ```
   I = C × dVc/dt
   ```
3. Substituting into KVL gives the differential equation:
   ```
   Vin = RC × dVc/dt + Vc
   ```
4. Solve with the boundary condition Vc=0 at t=0:
   ```
   Vc(t) = Vin × (1 - e^(-t/RC))
   ```
5. Time constant:
   ```
   τ = R × C = 1kΩ × 100µF = 0.1s
   ```
6. Time to 63.2%:
   ```
   let Vc/Vin = 63.2% = 0.632
   0.632 = 1 - e^(-t/τ)
   e^(-t/τ) = 0.368
   -t/τ = ln(0.368) = -1
   t = τ = **0.1s**
   ```

**Part 2: transistor switch**

1. Base current: base to 3.3V through 10kΩ, Vbe = 0.7V
   ```
   Ib = (3.3V - Vbe) / Rb = (3.3V - 0.7V) / 10kΩ = 2.6V / 10kΩ = 0.26mA
   ```

2. Transistor current gain: Ic = β × Ib
   ```
   Ic = 100 × 0.26mA = 26mA
   ```
   This is the maximum collector current the transistor could theoretically supply

3. Actual LED branch current (limited by the load):
   - Collector loop KVL: 5V = Vf + Vce(sat) + Ic × Rc
   - LED drop Vf ≈ 1.8V, transistor saturation drop Vce(sat) ≈ 0.2V
   - Actual voltage across the resistor: Vr = 5V - 1.8V - 0.2V = 3.0V
   - Maximum LED branch current:
   ```
   ILED_max = Vr / Rc = 3.0V / 330Ω ≈ 9.09mA
   ```

4. Determine the transistor's state: compare Ic (theoretical) with ILED_max (load-limited)
   ```
   Ic = 26mA > ILED_max = 9.09mA
   ```
   The collector current demanded (9.09mA) is below what the transistor can supply (26mA)

5. Saturation check: is the base current enough to saturate?
   - Minimum base current for saturation:
   ```
   Ib_min = ILED_max / β = 9.09mA / 100 = 0.0909mA
   ```
   - Actual base current: Ib = 0.26mA
   - Verdict: Ib = 0.26mA > Ib_min = 0.0909mA ✓

6. Collector voltage transient analysis (with a differential equation):
   - Collector loop: 5V → LED → 330Ω → transistor collector → emitter → ground
   - Let the collector-to-ground voltage be Vc(t); when saturated Vce(sat) ≈ 0.2V
   - KVL around the collector loop:
   ```
   5V = Vf + I_c(t) × 330Ω + Vc(t)
   ```
   - Rearranged into a differential equation (assuming a parasitic collector capacitance Cp):
   ```
   I_c(t) = Cp × dVc/dt
   ```
   ```
   5V - Vf - 330Ω × Cp × dVc/dt - Vc(t) = 0
   ```
   - Standard form:
   ```
   dVc/dt + (1/(330Ω × Cp)) × Vc(t) = (5V - Vf)/(330Ω × Cp)
   ```
   - Boundary condition: at t = 0, Vc(0) = 5V (transistor off, collector effectively open)
   - Solving gives:
   ```
   Vc(t) = Vce(sat) + (5V - Vce(sat)) × e^(-t/τ_c)
   ```
     where τ_c = 330Ω × Cp

7. Conclusion: the transistor is in saturation, enough current flows, **the LED lights**
   - At steady state Vc ≈ Vce(sat) ≈ 0.2V
   - The transient decays exponentially toward steady state, with a time constant set by the collector parasitic capacitance

---

### Task 2: Tinkercad Automatic Street Light

**Circuit description**: a photoresistor (LDR) plus an NPN transistor controlling an LED. The LED lights when ambient light is low and goes dark when bright.

**Tinkercad wiring**:

```
5V ──[10kΩ]── B ──[LDR]── GND

5V ── LED ──[330Ω]── C
E ─────────────────── GND
```

**Wiring notes**: the 10kΩ pulls up to 5V, and node B goes to ground through the LDR. With the LDR on the bottom (to GND): in bright light the LDR resistance is small → B drops below 0.7V → transistor off → LED dark; in darkness the LDR resistance is large → B rises above 0.7V → transistor saturates → LED lights.

**Tinkercad screenshots**:
- **Dark, LED on (LDR brightness at minimum):** [`day-07/Tinkercad电路-黑暗亮灯.png`](day-07/Tinkercad电路-黑暗亮灯.png)
- **Daylight, LED off (LDR brightness at maximum):** [`day-07/Tinkercad电路-白天灯灭.png`](day-07/Tinkercad电路-白天灯灭.png)

**Measured data (Tinkercad simulation)**

| State | Vc | Vb | V_LDR | Vled | LED |
|------|-----|------|-----------|------|-----|
| Dark (on) | 37.9mV | 696mV | 696mV | 1.97V | ✓ on |
| Daylight (off) | 3.68V | 241mV | 241mV | 1.32V | ✗ off |

> Dark: Vb = 696mV > 0.7V → saturated → Vc = 37.9mV ≈ 0 → LED on
> Daylight: Vb = 241mV < 0.7V → cut off → Vc floating → LED off
> Note: the daylight Vc = 3.68V is a floating meter reading, not the real operating state

**Physical breadboard test**

Beyond the Tinkercad simulation, the same principle was built on a real 830-point breadboard (photoresistor + PN2222 transistor + LED) to verify the simulation:

- **Dark, LED on (large LDR resistance → transistor conducts):** [`day-07/面包板电路-暗光亮灯.png`](day-07/面包板电路-暗光亮灯.png)
- **Room light (marginal conduction):** [`day-07/面包板电路-白光亮灯.png`](day-07/面包板电路-白光亮灯.png)

> The hardware behaves as the simulation predicts: shading the LDR lights the LED clearly; under room light the LDR is still in the few-kΩ range, Vb lands in the 0.5-0.8V marginal band, and the LED glows faintly without fully extinguishing — consistent with Key Observation 7, showing this is inherent to an LDR divider rather than a wiring error.

**How the light-controlled lamp works (my understanding)**

The core chain: **light intensity → LDR resistance → node B voltage → transistor switch → LED on/off**.

**1. A photoresistor changes resistance with light**

Inside an LDR is a semiconductor. Incident photons excite **free electrons** (mobile carriers): more light → more carriers → lower resistance; less light → fewer carriers → higher resistance.

**2. Node B voltage comes from the 10kΩ (pull-up) + LDR (pull-down) divider**

```
5V ──[10kΩ]── B ──[LDR]── GND

Vb = 5V × R_LDR / (10kΩ + R_LDR)
```

- **Bright light → small LDR resistance** (hundreds of Ω to 1kΩ):
  ```
  Vb ≈ 5V × 0.5k / (10k + 0.5k) ≈ 0.24V   → low
  ```
- **Dark → large LDR resistance** (tens to hundreds of kΩ, e.g. 50kΩ):
  ```
  Vb = 5V × 50k / (10k + 50k) ≈ 4V        → high
  ```

The LDR is the **pull-down** at node B: the smaller its resistance, the harder B is pulled toward GND; the larger it is, the more the 10kΩ pull-up wins and raises B toward 5V. In darkness an LDR reaches tens to hundreds of kΩ, and that large resistance is exactly what lifts Vb above 0.7V.

**3. Node B voltage is the transistor's switching threshold**

B-E needs about 0.7V to turn on; node B is effectively the "switch" controlling collector current:

| Environment | Vb | Transistor | Ic | LED |
|------|-----|--------|-----|-----|
| Bright (daylight) | ≈ 0.24V, < 0.7V | cut off | ≈ 0 | **off** |
| Dark (night) | ≈ 0.7~4V, > 0.7V | saturated | ≈ 9~14mA | **on** |

- Vb < 0.7V → almost no base current injected → collector loop open → LED off
- Vb > 0.7V → base current flows → C-E conducts → `5V → 330Ω → LED → C-E → GND` completes → LED on

**4. Conclusion**

**More light → LED off; less light → LED on**: the photoresistor first turns "light" into "resistance change", the 10kΩ + LDR divider turns that into "node B voltage", and the transistor finally switches the LED loop on and off. The two breadboard readings — 241mV (off in daylight) and 696mV (on/marginal in shade) — straddle the 0.7V threshold exactly, confirming the chain.

### Formulas

**Series current limiting**
```
I = (Vin - Vf) / R
```

**Voltage divider (unloaded)**
```
Vout = Vin × R2 / (R1 + R2)
```

**Voltage divider (loaded)**
```
Vout = Vin × Rload / (R1 + Rload)   // R2 replaced by the load in parallel
```

**RC time constant**
```
τ = R × C
t_63.2% = τ
```

**Transistor switch condition**
```
Ib = (Vbase_source - Vbe) / Rb
Ic = β × Ib
LED on when: Ic > LED threshold current (about 1-2mA)
```

### Key Observations

1. **LDR placement sets the circuit's direction**: 10kΩ on top (to 5V) with the LDR below (to GND) gives "dark → on"; swapping them inverts the behaviour
2. **Base voltage is the switching threshold**: Vb ≈ 0.7V is the conduction point — measuring Vb tells you the circuit's state
3. **When saturated, Vc ≈ 0V**: a deeply saturated transistor has a very low collector voltage (around 40mV), giving the LED maximum current
4. **When cut off, Vc floats**: a meter reading Vc with the transistor off returns an indeterminate value (e.g. 3.68V), not the true potential
5. **The voltage across the LDR ≈ Vb**: its bottom end is grounded and its top end is the base, so V_LDR = Vb - GND = Vb
6. **The divider sets the switch state**: 10kΩ and the LDR form a divider whose midpoint follows the LDR — in darkness a large LDR resistance lifts B into conduction; in light a small one pulls B low and cuts off. In simulation the LDR never becomes infinitely resistive, so the base never fully floats when off
7. **LDR sensitivity is limited**: under room light a typical photoresistor is still a few kΩ, putting Vb in the 0.5-0.8V marginal band where the LED glows faintly rather than going fully dark; only strong light (a torch pointed straight at it) pushes Vb < 0.7V for a clean cutoff. This is inherent to an LDR divider — a comparator or Schmitt trigger would resolve the marginal flicker

### Open Questions

(To be worked through at the weekend.)

---

## Day 8 — Meet the ESP32-S3 & Set Up the Toolchain

### Goal

Get to know the ESP32-S3-WROOM-1 N16R8 board on hand (pinout, boot mode, the two USB-C ports), set up Arduino IDE with the esp32 core, and flash a self-check sketch to verify the hardware.

> Full write-up (board photos, pin notes, fqbn config table, self-check code and serial output, pitfalls) in [`day-08/README.md`](./day-08/README.md).

### Board

| Item | Spec |
| --- | --- |
| Module | ESP32-S3-WROOM-1 **N16R8** |
| Flash | 16 MB |
| PSRAM | 8 MB (Octal / OPI) |
| Core | Xtensa LX7 dual-core, up to 240 MHz |
| Wireless | Wi-Fi 2.4 GHz + Bluetooth 5 (LE) |
| Pins | 44-pin, Type-C |

**Don't plug into the wrong Type-C port:**

| Port | Silkscreen | Bridge chip | ESP32-S3 pins | Purpose |
| --- | --- | --- | --- | --- |
| U2 | UART | CH343P | GPIO43 (U0TXD) / GPIO44 (U0RXD) | Serial download + Serial Monitor (used here) |
| U1 | USB | none (native) | GPIO19 (D-) / GPIO20 (D+) | USB-OTG / USB-JTAG debug |

### Pin Highlights

| Category | Pins | Notes |
| --- | --- | --- |
| Digital GPIO | GPIO 0–21, 35–42, 45–48 | **33** programmable IOs total |
| UART0 | GPIO43 (TXD) / GPIO44 (RXD) | Wired to CH343P for flashing and serial printing |
| USB-OTG | GPIO19 (D-) / GPIO20 (D+) | Native USB, also usable for USB-JTAG debug |
| JTAG | GPIO39–42 | Default JTAG multiplexed pins |
| Power | 3V3 / 5V / GND | 5V accepts 4.5–5.5V; 3V3 is onboard LDO output (~1A) |
| Onboard RGB LED | GPIO48 | WS2812B, single-wire addressable (needs a third-party library; lit on Day 9) |
| Boot / Reset | GPIO0 (BOOT) / EN (reset) | Hold BOOT while powering up to enter download mode |
| ADC1 | GPIO 1–10 | Analog input, **must not exceed 3.3V** |
| ADC2 | GPIO 11–20 | ⚠️ **ADC2 is unavailable while Wi-Fi is active** |
| Touch | GPIO 1–14 | Capacitive touch channels |

### Toolchain

Board selected as **ESP32S3 Dev Module** (`esp32:esp32:esp32s3`) with **16MB Flash / OPI PSRAM / QIO / 240 MHz / 921600 / USB CDC On Boot kept Disabled**, port via the CH343P virtual serial port.

### Self-check

```cpp
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("=== ESP32-S3 N16R8 Self-check ===");
  Serial.printf("Flash size: %u MB\n", ESP.getFlashChipSize() / (1024 * 1024));
  Serial.printf("PSRAM size: %u MB\n", ESP.getPsramSize() / (1024 * 1024));
  Serial.printf("PSRAM free: %u bytes\n", ESP.getFreePsram());
  Serial.printf("CPU freq: %u MHz\n", ESP.getCpuFreqMHz());
  Serial.printf("Chip model: %s\n", ESP.getChipModel());
  Serial.printf("Core version: %s\n", ESP.getCoreVersion());
}

void loop() {
  delay(2000);
  Serial.printf("running... PSRAM free %u bytes\n", ESP.getFreePsram());
}
```

| Check | Expected | Measured | Result |
| --- | --- | --- | --- |
| Flash | 16 MB | 16 MB | ✅ |
| PSRAM | 8 MB | 8 MB | ✅ |
| PSRAM free | close to 8 MB | 8384788 bytes (≈8.0 MB) | ✅ |
| CPU freq | 240 MHz | 240 MHz | ✅ |
| Chip model | ESP32-S3 | ESP32-S3 | ✅ |
| Core version | — | 3.3.10-cn | ✅ |

> Full board photos, IDE screenshots and serial captures are in [`day-08/`](./day-08/).

### Key Observations

1. **The two Type-C ports serve completely different purposes**: only U2 (CH343P → UART0) carries serial download and the Serial Monitor; U1 requires `USB CDC On Boot` to be enabled in the IDE, otherwise the monitor stays blank
2. **`USB CDC On Boot` is the single most important setting here**: with U2 it must stay Disabled, or `Serial` gets redirected to the native USB (GPIO19/20)
3. **R8 requires OPI PSRAM**: an octal PSRAM board with QSPI selected will fail PSRAM detection
4. **`setup()` self-check lines print only once at reset**: open the Serial Monitor first, then press RST/EN — otherwise you miss the output (an actual pitfall this session)
5. **ADC2 and Wi-Fi are mutually exclusive**: GPIO 11–20 lose analog input once Wi-Fi is enabled — prefer ADC1 for sensors
6. **ADC input must not exceed 3.3V**: a divided signal has to stay within ADC range — a constraint Day 14's voltmeter will check closely
7. **The onboard WS2812B needs an extra library**: GPIO48 is confirmed from the schematic, but neither the local esp32 3.3.10-cn bundled libraries nor `~/Documents/Arduino/libraries` had a WS2812/NeoPixel driver — driving it requires installing a third-party library (e.g. Adafruit NeoPixel). Verified by actually lighting it on Day 9 (green → blue → red cycle), see [`day-09/README.md`](./day-09/README.md)

---

## Day 9 — First Program: Blink (Onboard WS2812B + External LED)

### Goal

Flash the first firmware and understand GPIO output + `delay`-based timing. This board ships no plain LED you can wire directly — instead it has an onboard addressable RGB LED, so drive that first; then add an external plain LED to walk through the classic `digitalWrite` blink; finally merge both paths into one loop. Three sketches make up Day 9's three experiments.

> Full write-up (dependency install, code, video, per-pixel three-colour verification, pitfalls) in [`day-09/README.md`](./day-09/README.md).

| Experiment | Code | Content | Pins |
| --- | --- | --- | --- |
| 1 | [`day-09/rgb_cycle.ino`](./day-09/rgb_cycle.ino) | Onboard WS2812B colour cycle | GPIO48 |
| 2 | [`day-09/external_led_blink.ino`](./day-09/external_led_blink.ino) | External plain LED blink | GPIO2 |
| 3 | [`day-09/combined_blink.ino`](./day-09/combined_blink.ino) | Colour cycle + external LED in one loop | GPIO48 + GPIO2 |

### Experiment 1: Onboard WS2812B Colour Cycle

| Item | Value |
| --- | --- |
| Part | XL-5050RGBC-WS2812B (refdes U1) |
| Type | Single-wire addressable RGB LED (driver IC + constant-current source built in) |
| Control pin | **GPIO48** (net `RGB_CTRL`) |
| Protocol | 800 kHz single-wire return-to-zero, 24 bit each (G8 R8 B8) |
| Library | Adafruit NeoPixel (third-party, install separately) |

> WS2812B is a digital part talking over 800 kHz narrow pulses, so it **cannot be driven by `digitalWrite`** — a dedicated library is required to send data with correct timing.

```cpp
#include <Adafruit_NeoPixel.h>

#define RGB_PIN   48    // onboard WS2812B control pin (net RGB_CTRL)
#define RGB_COUNT 1     // the board has exactly one pixel

Adafruit_NeoPixel rgb(RGB_COUNT, RGB_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  rgb.begin();
  rgb.setBrightness(50);   // 0-255, start low to avoid eye strain
  rgb.show();              // power up dark, clearing the random colour latched during reset
}

void loop() {
  rgb.setPixelColor(0, rgb.Color(0, 255, 0));   // green
  rgb.show();
  delay(500);

  rgb.setPixelColor(0, rgb.Color(0, 0, 255));   // blue
  rgb.show();
  delay(500);

  rgb.setPixelColor(0, rgb.Color(255, 0, 0));   // red
  rgb.show();
  delay(500);
}
```

`setPixelColor()` only writes the RAM buffer; `show()` actually transmits to the pixel — you need both.

**Result**: video [`day-09/板载彩灯红绿蓝循环.MOV`](./day-09/板载彩灯红绿蓝循环.MOV) (HEVC 1920×1080, 29.97 fps, 229 frames, 7.64 s). Verifying three screenshots by **dominant-channel pixel count**:

| Screenshot | red | green | blue | Verdict |
| --- | --- | --- | --- | --- |
| 灯珠-红色.png | **134272** | 13 | 0 | red ✅ |
| 灯珠-绿色.png | 8446 | **71787** | 0 | green ✅ |
| 灯珠-蓝色.png | 4464 | 106 | **130824** | blue ✅ |

Each dominant channel exceeds the others by 1–4 orders of magnitude; green, blue and red are all experimentally confirmed. The three phases sit at 0.20 s / 0.55 s / 1.05 s — adjacent gaps of 0.35–0.50 s, consistent with `delay(500)`.

### Experiment 2: External Plain LED Blink

The onboard pixel speaks an 800 kHz single-wire protocol and needs a library; to exercise the most primitive "direct GPIO output" usage, a plain LED is added and driven with classic `digitalWrite`.

**Wiring**: `GPIO2 → 220Ω current-limiting resistor → LED long leg (anode) → LED short leg (cathode) → GND`

| Part | Notes |
| --- | --- |
| Resistor | 220Ω (the 220Ω ×10 in the parts inventory) |
| LED | Plain through-hole LED, long leg anode / short leg cathode |
| Power | Board USB 5V, GPIO2 outputs 3.3V logic level |

```cpp
const int EXT_LED_PIN = 2;   // external LED control pin

void setup() {
  pinMode(EXT_LED_PIN, OUTPUT);
  digitalWrite(EXT_LED_PIN, LOW);   // start dark
  Serial.begin(115200);
  Serial.println("ESP32-S3 External LED Blink Start");
}

void loop() {
  digitalWrite(EXT_LED_PIN, HIGH);  // 3.3V → current through resistor and LED → on
  Serial.println("LED ON");
  delay(500);

  digitalWrite(EXT_LED_PIN, LOW);   // 0V → no drop across LED → off
  Serial.println("LED OFF");
  delay(500);
}
```

**Demo**: video [`day-09/外接LED.MOV`](./day-09/外接LED.MOV) (HEVC 1920×1080, 29.97 fps, 121 frames, 4.035 s). The breadboard shows jumpers, the 220Ω resistor and the external LED; the LED alternates roughly 500 ms on → off, matching `delay(500)`.

> This clip was recorded running **Experiment 2 (standalone)**. Experiment 3 drives GPIO2 identically (same 500 ms toggle), so it holds for the external-LED half of Experiment 3 as well.

### Experiment 3: Combined (Colour Cycle + External LED)

Experiments 1 and 2 merged into a single `loop()`: each step changes both the pixel colour and the external LED state.

**Wiring**: Experiment 1 plus Experiment 2 — GPIO48 still the onboard pixel, GPIO2 still `220Ω → LED → GND`, both sharing board GND.

Four 500 ms phases, one full cycle of **2 s**:

| Phase | GPIO48 pixel | GPIO2 external LED |
| --- | --- | --- |
| 1 | green | on |
| 2 | blue | off |
| 3 | red | on |
| 4 | all off | off |

**Key observation**: both outputs share one `loop()`, so `delay` is shared and the two phases are inherently synchronised; `rgb.show()` (single-wire timing) and `digitalWrite` (plain GPIO) coexist in the same loop.

### What Went Wrong & How I Fixed It

- **A fresh board lights up on power-up, stays green after flashing `day08.ino`, and goes dark on replug**: WS2812B has no NVM, so during reset/download GPIO48 floats and the pixel latches noise as data, producing a random colour; `day08.ino` never drove GPIO48, so the colour stuck. Fix: call `begin()` then `show()` immediately in `setup()` to blank it.
- **The "green" phase looks missing in the recording**: the camera meters on the brightest pixel, desaturating green to roughly RGB (118,162,123), below the hue threshold. Fix: switch to **dominant-channel counting** in a fixed neighbourhood of the pixel — green appears immediately (71787 vs 8446 vs 0).

---

## Day 10 — PWM Breathing LED + Potentiometer Dimming

### Goal

On Day 9 GPIO had only two states — fully on or fully off (`digitalWrite`). Day 10 adds the **third state: any brightness** — via **PWM (pulse-width modulation)**: the pin switches on and off very fast, and the "on-time fraction" (duty cycle) sets the average brightness. Sweeping duty from 0 up to full and back down gives a breathing LED; letting a knob set the duty gives potentiometer dimming.

The ESP32 generates PWM in the **LEDC peripheral**: configured once, the hardware keeps emitting the waveform on its own, so the CPU can go send WS2812B data without disturbing it. This is the hardware basis for the non-blocking style used by Day 11 (buttons) and Day 19 (motors).

> Full write-up (API breakage, code, video, period verification, pitfalls) in [`day-10/README.md`](./day-10/README.md).
>
> **Code convention (from this day on)**: later experiments are written as a single `loop()` running in parallel with the onboard pixel — no more separate "LED-only" sketches.

**Task 1 needs no new circuit** — it fully reuses Day 9's external LED. Task 2 adds only a potentiometer.

| Task | Code | Content | Pins |
| --- | --- | --- | --- |
| 1 | [`day-10/combined_pwm_blink/combined_pwm_blink.ino`](./day-10/combined_pwm_blink/combined_pwm_blink.ino) | Colour cycle + LED breathing (brightness swept by code) | GPIO48 + GPIO2 |
| 2 | [`day-10/pot_dimmer/pot_dimmer.ino`](./day-10/pot_dimmer/pot_dimmer.ino) | Colour cycle + potentiometer dimming (brightness set by the knob) | GPIO48 + GPIO2 + GPIO1 |

> There is also a minimal example, [`day-10/breath_led/breath_led.ino`](./day-10/breath_led/breath_led.ino), which strips the breathing logic out of the colour cycle so the duty cycle itself is easier to see.

### Task 1: External LED Breathing

**Wiring**: same as Day 9 — `GPIO2 → 220Ω current-limiting resistor → LED long leg (anode) → LED short leg (cathode) → GND`, not a single wire added.

```cpp
const int LED_PIN = 2;
const int STEP_MS = 4;      // 4ms per step: 256 steps ≈ 1.02s one way, ≈2s full breath

void setup() {
  ledcAttach(LED_PIN, 5000, 8);   // pin 2, 5kHz, 8-bit resolution → duty 0~255
  ledcWrite(LED_PIN, 0);          // start dark
  Serial.begin(115200);
  Serial.println("ESP32-S3 PWM breathing LED Start");
}

void loop() {
  for (int b = 0; b <= 255; b++) { ledcWrite(LED_PIN, b); delay(STEP_MS); }  // fade up
  for (int b = 255; b >= 0; b--) { ledcWrite(LED_PIN, b); delay(STEP_MS); }  // fade down
}
```

The two APIs and their roles:

- `ledcAttach(pin, freq, resolution)` — binds a pin to LEDC and sets PWM frequency and resolution. 8-bit resolution means duty spans **0–255**.
- `ledcWrite(pin, duty)` — writes the duty cycle, **keyed by pin**, not by channel.

### Task 1, Full Version: Colour Cycle + Breathing, in Parallel

```cpp
#include <RgbCycle.h>   // Day 9's colour cycle, extracted into a reusable library

const int LED_PIN = 2;

unsigned long lastBreath = 0;
const unsigned long BREATH_INTERVAL = 4;   // advance one step every 4ms
const int BREATH_STEP = 1;
int brightness = 0;
int breathDir  = 1;

void setup() {
  RgbCycle::begin();            // pixel: init
  RgbCycle::setInterval(800);   // pixel: 800ms per colour for this task (2.4s per cycle)
  ledcAttach(LED_PIN, 5000, 8); // external LED: bind PWM
  ledcWrite(LED_PIN, 0);
}

void loop() {
  unsigned long now = millis();

  RgbCycle::update();   // task A: pixel, internally switches every 800ms

  if (now - lastBreath >= BREATH_INTERVAL) {   // task B: breathing LED
    lastBreath = now;
    brightness += breathDir * BREATH_STEP;
    if (brightness >= 255) { brightness = 255; breathDir = -1; }
    if (brightness <= 0)   { brightness = 0;   breathDir =  1; }
    ledcWrite(LED_PIN, brightness);
  }
}
```

The key point: **there is not a single `delay()` anywhere in `loop()`**. Each path times itself with `millis()`, so neither blocks the other.

The `millis()` test must use the subtraction form `now - lastBreath >= BREATH_INTERVAL` (not `now >= lastBreath + INTERVAL`) — the former stays correct when `unsigned long` wraps around.

### Task 2: Potentiometer Dimming

**Circuit** (adds only a potentiometer on top of Task 1; the LED path is untouched):

| Potentiometer pin | Connects to |
| --- | --- |
| One outer pin | **3V3** |
| Other outer pin | **GND** |
| Middle pin (wiper) | **GPIO1** (= ADC1_CH0; ADC1 does not conflict with Wi-Fi) |

> ⚠️ The classic symptom of getting this wrong is a reading that is constant or jitters wildly — `analogRead()` measures the wiper voltage, so only the middle pin belongs on GPIO1.

Full code: [`day-10/pot_dimmer/pot_dimmer.ino`](./day-10/pot_dimmer/pot_dimmer.ino)

```cpp
#include <RgbCycle.h>

const int LED_PIN = 2;
const int POT_PIN = 1;                    // GPIO1 = ADC1_CH0

const unsigned long READ_INTERVAL = 20;   // sample every 20ms
unsigned long lastRead = 0;

void setup() {
  RgbCycle::begin();
  RgbCycle::setInterval(800);
  ledcAttach(LED_PIN, 5000, 8);
  ledcWrite(LED_PIN, 0);
  Serial.begin(115200);
}

void loop() {
  RgbCycle::update();   // task A: the colour cycle carries on

  unsigned long now = millis();
  if (now - lastRead >= READ_INTERVAL) {   // task B: read the potentiometer
    lastRead = now;
    int potValue   = analogRead(POT_PIN);              // 0–4095 (12-bit ADC)
    int brightness = map(potValue, 0, 4095, 0, 255);   // scale to duty 0–255
    ledcWrite(LED_PIN, brightness);
    Serial.printf("Pot: %4d  Brightness: %3d\n", potValue, brightness);
  }
}
```

Chain: `turn the knob → wiper voltage 0–3.3V → GPIO1 reads 0–4095 → map() scales to 0–255 → ledcWrite() sets the brightness`

> `map()` is a **linear** conversion, while human brightness perception is non-linear (the effect observed in Task 1). In practice the low end of the knob changes far too fast and the high end barely seems to change. This task uses the plain linear map to get the "read ADC → drive PWM" chain working; gamma correction comes later.

> Status: **✅ tested on hardware** — at three knob positions the readings match the `map()` formula exactly (see "Result" below).

### The Colour Cycle Became the `RgbCycle` Library

The first combined sketch copied Day 9's colour cycle again. At that rate every later task would copy it too, so it was extracted into an Arduino library, **`RgbCycle`**, installed at `~/Documents/Arduino/libraries/RgbCycle/`. Any sketch can now use it with a single `#include <RgbCycle.h>`:

```cpp
void setup() { RgbCycle::begin(); RgbCycle::setInterval(800); }
void loop()  { RgbCycle::update();   /* other tasks */ }
```

| API | Call site | Purpose |
| --- | --- | --- |
| `RgbCycle::begin()` | once in `setup()` | Init the pixel and clear the random colour latched during reset |
| `RgbCycle::setInterval(ms)` | once in `setup()` | Set time per colour; defaults to 800ms if not called |
| `RgbCycle::update()` | every `loop()` | Internally checks timing and advances a colour without blocking |
| `RgbCycle::setColor(r, g, b)` | anytime | Hold the pixel at a given colour |

A backup copy of the source lives in the repo: [`day-10/lib/RgbCycle/`](./day-10/lib/RgbCycle/) (two copies — changes must be synced by hand).

### Result

**Task 1 (breathing LED), video record**: [`day-10/LED呼吸灯效果.MOV`](./day-10/LED呼吸灯效果.MOV) (HEVC 1920×1080, 29.97 fps, 227 frames, 7.57 s). The breadboard shows jumpers, the 220Ω resistor and the external LED, cycling smoothly through a continuous fade-up → fade-down with no visible stepping.

Period verification used **autocorrelation**: the peak sits at lag 72 frames = **2.40 s** (r = 0.627), the strongest negative lobe at **0.83 s** (r = −0.505), the same order as the designed **2.048 s**.

> **Limitation of the measurement**: the standard deviation of the whole-frame mean luminance is only **1.355** — essentially flat, because camera auto-exposure cancels the brightness change. So **mean luminance cannot be used to judge the breathing rhythm** (the same trap as Day 9). Autocorrelation does reveal a ≈2 s period, but in the per-pixel correlation only 0.149% of pixels have |r| > 0.6 — the signal is real but weak. Conclusion: we can confirm "a continuous fade that repeats at ≈2 s"; we **cannot** make quantitative per-frame brightness claims.

**Task 2 (potentiometer dimming), serial measurements**: three knob positions, each captured as a paired "serial monitor + actual LED" photo ([`day-10/POT-137.png`](./day-10/POT-137.png) / [`POT-1684.png`](./day-10/POT-1684.png) / [`POT-4095.png`](./day-10/POT-4095.png)).

| Knob position | `analogRead()` | `pot × 255 ÷ 4095` | Measured duty | Match |
| --- | --- | --- | --- | --- |
| Turned to max | 4095 | 255.00 | 255 | ✅ |
| Somewhere mid | 1684 | 104.86 → truncated | 104 | ✅ |
| Near minimum | 137 | 8.53 → truncated | 8 | ✅ |

- **The whole chain is correct**: all three measured values match `map(pot, 0, 4095, 0, 255)` exactly. This also confirms `map()` **truncates to an integer** (no rounding) — one reason it feels imprecise at the low end.
- **The wiring is correct and the reading is stable**: with the knob untouched the reading jitters by only **±1–3 counts** (1683/1684/1682/1680), which is normal ADC noise; none of the three positions sits near a `map()` boundary, so the duty is stable and does not flicker.
- **The three brightness levels are visually distinguishable**: duty 255 is clearly brightest → 104 clearly dimmer → 8 barely visible.

> ⚠️ **The photos support only a qualitative conclusion**: camera auto-exposure inverts the ordering (the whole-frame mean luminance of the duty-8 shot is 115.78, *higher* than 100.63 for duty 255), and the three shots are handheld and unaligned (normalized cross-correlation against the duty-255 frame is only 0.155–0.172). So this log claims only "which one is brighter", never "how many times brighter".

**Pitfall: the flickering LED was the wrong sketch being flashed.** The symptom was "the POT value changes, but the LED blinks on and off". The stable serial readings above rule out the ADC side; the editor pane in `POT-1684.png` shows `combined_pwm_blink.ino` — Task 1's **breathing** sketch — open at the time. It sweeps duty from 0 to 255 and back on its own, overriding whatever the knob writes. Fix: confirm `pot_dimmer.ino` is re-uploaded. **Lesson: a functioning serial output only proves *a* program is running, not that it is the one you are looking at.** (Honest boundary: the screenshot records which file the editor had open; it cannot prove which firmware was running on the board — this is the explanation best supported by the evidence, not a proven conclusion.)

### What Went Wrong & How I Fixed It

- **The guide's reference code would not compile**: `'ledcSetup' was not declared in this scope`, `'ledcAttachPin' ... did you mean 'ledcAttach'?`. The guide was written for arduino-esp32 **2.x**, while this machine runs core **3.3.10-cn**, where both functions are gone. Fix: use `ledcAttach(pin, freq, resolution)` + `ledcWrite(pin, duty)`, writing duty by **pin** rather than channel; `pinMode()` is no longer needed.
- **The colour cycle had to be pasted into every task**: Day 9 had written it as in-sketch code rather than a module. Fix: extracted into the `RgbCycle` library (at the cost of keeping two copies in sync).
- **Both lights were absurdly fast**: the first version used 500ms per colour and a 1.0s one-way breath, so both the onboard pixel and the external LED looked like frantic blinking on the bench. Fix: slowed the pixel to 800ms per colour and the breath to ≈2 s per cycle, and added `setInterval()` to the library.
- **Comments disagree with the actual constants**: two inline comments in `combined_pwm_blink.ino` still say "500ms per colour" / "12ms per step" while the real values are 800ms / 4ms. Comments are the easiest thing to forget when tuning constants — trust the constant definitions.
- **LED flickering on and off (wrong sketch flashed)**: the stable serial readings ruled out the ADC side (the potentiometer wiring was right). The sketch open in the editor and flashed at the time was Task 1's breathing sketch, `combined_pwm_blink.ino`, whose duty sweeps 0↔255 on its own and overwrites whatever the knob writes. Fix: re-upload `pot_dimmer.ino`.

> Wiring-mistake checklist (kept for reference): wire the potentiometer alone first, print only `potValue`, and turn the knob — the value should run smoothly through 0–4095 before the LED is wired back in. The two classic mistakes are (a) putting GPIO1 on an outer pin instead of the middle wiper pin, and (b) leaving the wiper floating, in which case `analogRead()` returns noise.

### Task 2 — Conclusion (now tested)

Potentiometer dimming **is tested and working on hardware**: the chain `analogRead()` on GPIO1 → `map()` → `ledcWrite()` is correct, and the duty at all three knob positions matches the formula exactly.

> 📌 `map()` is linear while human brightness perception is not — the low end of the knob feels too fast, left to gamma correction (see the day-10 notes).

---

## Day 11 — Digital Input & Button

> Full write-up in [`day-11/README.md`](./day-11/README.md).

### Goal

Days 9 and 10 were all about GPIO **output**. Day 11 flips it around — reading GPIO **input**: using a button to control an LED.

Key concepts:
- **INPUT_PULLUP**: uses the chip's internal pull-up resistor, no external resistor needed
- **Debouncing**: the contacts bounce for microseconds when pressed, needs a 20 ms delay
- **Edge detection**: distinguish "pressed" (FALLING) from "released" (RISING)

### Wiring

| Component | Connection |
| --- | --- |
| Button | GPIO1 → button → GND |
| External LED | GPIO2 → 220Ω → LED anode → LED cathode → GND |
| Onboard pixel | GPIO48 (unchanged, `RgbCycle::update()` keeps running) |

Multimeter measurements:

| Button state | GPIO1 voltage |
| --- | --- |
| Released | 3.3V (pulled up by internal resistor) |
| Pressed | 0V (shorted to GND) |

### Code

Full code: [`day-11/day11.ino`](./day-11/day11.ino)

```cpp
#include <RgbCycle.h>

const int BUTTON_PIN = 1;   // GPIO1 = button input
const int LED_PIN    = 2;   // GPIO2 = external LED output

void setup() {
  RgbCycle::begin();            // pixel: init
  RgbCycle::setInterval(800);   // pixel: 800ms per colour

  pinMode(BUTTON_PIN, INPUT_PULLUP);  // enable internal pull-up, pressed = LOW
  pinMode(LED_PIN,    OUTPUT);         // LED pin as output
  Serial.begin(115200);
}

void loop() {
  RgbCycle::update();   // Task A: colour cycle keeps running

  int buttonState = digitalRead(BUTTON_PIN);
  if (buttonState == LOW) {          // button pressed (pulled low)
    digitalWrite(LED_PIN, HIGH);     // LED on
    Serial.println("Button PRESSED");
    delay(200);                      // simple debounce: only once per 200 ms while held
  } else {                           // button released (pulled back to HIGH)
    digitalWrite(LED_PIN, LOW);      // LED off
  }
  delay(10);
}
```

### `pinMode()` Explained

The most common beginner question: *"I wrote `pinMode()` in `setup()`, why do I still need `digitalWrite()` in `loop()`?"*

`pinMode()` and `digitalWrite()` do **completely different things**:

| Function | Purpose | Analogy |
| --- | --- | --- |
| `pinMode(pin, INPUT_PULLUP)` | **Configure the pin's role** — "is this pin an input or output?" | Label a door: "entrance" or "exit" |
| `digitalWrite(pin, HIGH/LOW)` | **Drive an output pin to a level** | Flip a switch: "on" or "off" |
| `digitalRead(pin)` | **Read the current level of an input pin** | Read a light: "lit" or "dark" |

**One line**: `pinMode()` runs once in `setup()` and sets the pin's **identity**. `digitalWrite()` / `digitalRead()` run repeatedly in `loop()` and actually **move electrons**.

### Three Modes

| Mode | Pin behaviour | Typical use |
| --- | --- | --- |
| `OUTPUT` | You can drive it HIGH or LOW with `digitalWrite()` | LEDs, relays |
| `INPUT` | High-impedance — neither pulls up nor down — **must have an external pull-up/down** | Sensors (when external pull is present) |
| `INPUT_PULLUP` | Enables an internal ~40–50 kΩ pull-up resistor; pin reads HIGH by default, LOW when grounded | **Buttons / switches (standard wiring)** |

### Why `INPUT_PULLUP` for a button?

```
Internal pull-up 40kΩ
    |
GPIO1 ----[button]---- GND
```

- Button **open**: switch is open, GPIO1 is connected only to the internal pull-up resistor, no current path, pin is pulled to 3.3V → `digitalRead()` returns **HIGH**
- Button **closed**: switch closes, GPIO1 is directly connected to GND through a near-zero-resistance contact (≈0.1Ω), the internal pull-up resistor is "shorted out", pin voltage ≈ 0V → `digitalRead()` returns **LOW**

This is "**active-low**" — pressed = LOW, released = HIGH.

**Key point**: the 40kΩ resistor is **inside the ESP32 chip**, not inside the button. The button is a pure wire switch — when pressed its two contacts touch directly, effectively "jumping over" the pull-up resistor and pulling GPIO1 straight to GND, which is why the voltage drops to 0V. The code `if (buttonState == LOW)` means "the button is being pressed".

### Button Debouncing

When a mechanical button is pressed, the metal contacts **bounce** rapidly for a few microseconds, so `digitalRead()` may oscillate:

```
press → HIGH→LOW→HIGH→LOW→HIGH → stable LOW
```

Software debouncing is simplest: after detecting an edge, wait 20 ms for the bouncing to settle before reading again.

This sketch uses a **minimal version** (good for beginners): `delay(200)` means one print per 200 ms while held.

### Results

- **Released**: GPIO1 = 3.3V, LED off, no serial output
- **Pressed**: GPIO1 = 0V, LED on, serial prints `Button PRESSED`
- **Held**: prints every 200 ms
- **Released again**: LED turns off immediately

### What Went Wrong & How I Fixed It

- **Pin wired wrong**: button on 3V3 instead of GND → logic inverted. Fix: button to GND + `INPUT_PULLUP`.
- **LED not lighting**: anode/cathode reversed, or resistor too large (>1kΩ makes it very dim). 220Ω is safest.
- **Button jitter**: no debounce, same press prints multiple lines. Fix: add `delay(20)` or better.

---

## Day 12 — ADC & Sensor Reading

> Date: 2026-09-14
> Status: ✅ Tested on hardware (potentiometer raw=0→4095, voltage 0V→3.3V; raw=2048 ≈ 1.65V)
>
> Wiring: **GPIO1 (ADC1_CH0) → potentiometer middle pin**, side pins to 3V3 and GND

### Goal

Day 11 was about **digital signals** (HIGH/LOW). Day 12 flips it around — reading **analog signals**: using ADC (Analog-to-Digital Converter) to turn continuous voltage into digital values.

Core concepts:
- **12-bit ADC**: ESP32-S3's ADC is 12-bit, output range 0–4095
- **Voltage calculation**: `V = raw × 3.3 / 4095`
- **ADC1 vs ADC2**: GPIO1–10 belong to ADC1 (recommended), GPIO11–20 belong to ADC2 (conflicts with WiFi)
- **attenuation**: `ADC_11db` lets ADC measure the full 0–3.3V range
- **Serial Plotter**: visualize real-time data in Arduino IDE

### Hardware & Circuit

| Component | Wiring | Notes |
| --- | --- | --- |
| Potentiometer | **3V3 → left pin**, **GND → right pin**, **GPIO1 → middle pin** | 3-pin potentiometer as voltage divider, middle pin outputs 0–3.3V |
| External LED | GPIO2 → 2kΩ → LED → GND | Keep Day 11 wiring, `RgbCycle::update()` runs as usual |
| Onboard RGB | GPIO48 | Keep Day 9 wiring |

Potentiometer principle: fixed side pins to 3V3 and GND, rotating middle pin changes the voltage divider ratio:
- Counter-clockwise fully: middle pin ≈ GND → 0V
- Clockwise fully: middle pin ≈ 3V3 → 3.3V
- Middle position: ~1.65V

### Code

Full code: [`day-12/实验1-电位器测电压/实验1-电位器测电压.ino`](./day-12/实验1-电位器测电压/实验1-电位器测电压.ino)

```cpp
#include <RgbCycle.h>

const int ADC_PIN = 1;   // GPIO1 = ADC1_CH0

void setup() {
  RgbCycle::begin();               // RGB: initialize
  RgbCycle::setInterval(800);      // RGB: 800ms/color

  Serial.begin(115200);
  analogReadResolution(12);        // Set ADC to 12-bit (0-4095)
  analogSetAttenuation(ADC_11db);  // Set attenuation for 0-3.3V range
}

void loop() {
  RgbCycle::update();              // Task A: RGB cycle runs in parallel

  int raw = analogRead(ADC_PIN);
  float voltage = raw * 3.3 / 4095.0;
  Serial.printf("Raw: %4d  Voltage: %.2fV\n", raw, voltage);

  delay(500);
}
```

### `analogRead()` Explained

| Function | Purpose |
| --- | --- |
| `analogReadResolution(12)` | Set ADC precision to 12-bit, range 0–4095 |
| `analogSetAttenuation(ADC_11db)` | Set input attenuation, allows measuring 0–3.3V (default only ~0–1.1V) |
| `analogRead(pin)` | Read raw ADC value from specified pin (0–4095) |

**ADC1 vs ADC2**:
- **ADC1** (GPIO1–10): stable, recommended for beginners
- **ADC2** (GPIO11–20): shares with WiFi, readings are unreliable when WiFi is enabled — **avoid in early learning**

### Voltage Calculation

The ESP32-S3 ADC outputs a **digital value** (0–4095), which must be converted to actual voltage:

```
Voltage(V) = raw × reference_voltage / max_value
Voltage(V) = raw × 3.3 / 4095
```

Measured verification:
- raw = 0 → 0.00V (potentiometer fully counter-clockwise)
- raw = 2048 → 1.65V (potentiometer at middle position)
- raw = 4095 → 3.30V (potentiometer fully clockwise)

### Serial Plotter

Open Arduino IDE → Tools → Serial Plotter to see the voltage curve change in real-time as you rotate the potentiometer.

### Experiment 2: Photoresistor (LDR) Light Sensor

> Date: 2026-09-14
> Status: ✅ Tested on hardware (strong light raw≈4095 ≈ 3.3V; covered raw≈0 ≈ 0V)
>
> Wiring: **3V3 → photoresistor → GPIO1 → 10kΩ → GND** (photoresistor on top, 10kΩ pull-down on bottom)

**Photoresistor characteristics:**

| Light condition | Resistance | Notes |
| --- | --- | --- |
| Strong light | ~1–10kΩ (or lower) | Low resistance |
| Darkness | ~100kΩ–1MΩ | High resistance |

**Voltage divider principle:**

```
3V3 ──[R_LDR]── GPIO1 ──[10kΩ]── GND
          (top)                     (bottom)
```

**Core principle: in a series circuit, the larger resistor gets the larger voltage drop.**

Formula: `V_GPIO1 = 3.3V × (R_bottom / (R_LDR + R_bottom))`

**Strong light: R_LDR is very small → GPIO1 voltage is very high**

Under strong light, the photoresistor resistance is very low (~500Ω–1kΩ):

```
V_GPIO1 = 3.3V × (10kΩ / (1kΩ + 10kΩ))
        = 3.3V × (10k / 11k)
        = 3.3V × 0.91
        ≈ 3.00V  →  raw ≈ 3720
```

If light is extremely strong, R_LDR may be only about 100Ω:

```
V_GPIO1 = 3.3V × (10k / (0.1k + 10k))
        = 3.3V × (10k / 10.1k)
        ≈ 3.27V  →  raw ≈ 4050
```

Your measured raw ≈ 4095 (≈3.30V) indicates that under strong light R_LDR is extremely small, so the 10kΩ resistor takes almost the full 3.3V.

**Key insight:** When the LDR is small, it drops almost no voltage → nearly all 3.3V falls across the bottom 10kΩ → GPIO1 voltage is high.

**Covered/dark: R_LDR is very large → GPIO1 voltage is very low**

When covered (hand over the sensor), the photoresistor resistance is very high (~1MΩ):

```
V_GPIO1 = 3.3V × (10kΩ / (1000kΩ + 10kΩ))
        = 3.3V × (10k / 1010k)
        = 3.3V × 0.0099
        ≈ 0.033V  →  raw ≈ 40 ≈ 0
```

1MΩ is much larger than 10kΩ, so almost the entire 3.3V drops across the photoresistor, and the bottom 10kΩ gets only about 0.033V → GPIO1 voltage is near 0V.

**Key insight:** When the LDR is large, it drops almost all the voltage → the bottom 10kΩ gets only a tiny fraction → GPIO1 voltage is low → raw is small.

**One-line summary:**

> **LDR small (strong light) → it "eats" little voltage → 10kΩ gets the high voltage at GPIO1 → raw is large**
> **LDR large (darkness) → it "eats" most of the voltage → 10kΩ gets almost nothing → GPIO1 voltage is low → raw is small**

**Code:** [`day-12/实验2-光敏电阻测光照/实验2-光敏电阻测光照.ino`](./day-12/实验2-光敏电阻测光照/实验2-光敏电阻测光照.ino)

### Experiment 3: Serial Plotter Visualization

> Date: 2026-09-14
> Status: ✅ Tested on hardware (Serial Plotter shows smooth voltage curve as potentiometer rotates)
>
> Wiring: **Identical to Experiment 1** (GPIO1 → potentiometer middle pin, side pins to 3V3 and GND)

**Goal:** Experiments 1 and 2 print labeled text. Experiment 3 outputs only a **single raw number** so Arduino IDE's Serial Plotter can draw a real-time voltage curve.

**Circuit:** Same as Experiment 1 (potentiometer voltage divider, GPIO1 reads the wiper).

**Code:** [`day-12/实验3-Serial-Plotter可视化/实验3-Serial-Plotter可视化.ino`](./day-12/实验3-Serial-Plotter可视化/实验3-Serial-Plotter可视化.ino)

```cpp
#include <RgbCycle.h>

const int ADC_PIN = 1;   // GPIO1 = ADC1_CH0

void setup() {
  RgbCycle::begin();
  RgbCycle::setInterval(800);

  Serial.begin(115200);
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);  // full 0-3.3V range
}

void loop() {
  RgbCycle::update();

  int raw = analogRead(ADC_PIN);
  float voltage = raw * 3.3 / 4095.0;
  Serial.println(voltage);       // only the voltage value, for the Plotter
  delay(500);
}
```

**Difference from Experiment 1 code:**

| | Experiment 1 | Experiment 3 |
| --- | --- | --- |
| Output format | `Serial.printf("Raw: %4d Voltage: %.2fV\n", raw, voltage)` | `Serial.println(voltage)` |
| Serial Plotter | ❌ incompatible (has text labels) | ✅ compatible (pure numbers only) |
| Use case | Debugging, reading exact values | Observing voltage trends |

**How to run:** Upload → close Serial Monitor → Tools → Serial Plotter → rotate the potentiometer to see the curve move.

---
## Day 13 — Serial Communication & Debugging

> Date: 2026-09-14
> Status: 🚧 In Progress
>
> Hardware: Reuse Day 12 potentiometer circuit (GPIO1 → potentiometer middle pin)
> Core: UART communication, Serial.printf() formatted output, JSON data packaging

### Goal

The first 12 days focused on **hardware control** (GPIO output/input, ADC reading). Day 13 flips it around — focusing on **communicating with the computer**: sending ESP32 data over serial for debugging and logging.

Core concepts:
- **UART communication**: TX (transmit), RX (receive), baud rate
- **Serial.printf()**: formatted output, similar to C's `printf`
- **JSON format**: pack multiple variables into structured strings for easy parsing
- **Arduino Serial Monitor**: receive and view serial data
- **Debugging techniques**: use serial output to inspect intermediate values and troubleshoot

### UART Communication Basics

UART (Universal Asynchronous Receiver/Transmitter) is the most basic serial communication protocol.

**Hardware wiring:**

| Signal | Direction | Description |
| --- | --- | --- |
| **TX** (Transmit) | ESP32 → PC | ESP32 sends data |
| **RX** (Receive) | PC → ESP32 | ESP32 receives data |
| **GND** | Common ground | Must be connected, otherwise communication fails |

ESP32-S3 development boards have a built-in USB-to-serial chip (CP2102/CH340). Connect via USB cable — no extra wiring needed.

**Baud Rate:**

| Baud Rate | Speed | Use Case |
| --- | --- | --- |
| 9600 | Slow | Debugging, long distance |
| 115200 | Fast | **ESP32 default, recommended** |
| 921600 | Very fast | Large data transfer |

Key: The serial monitor baud rate must **match** the code, otherwise you see garbled text.

### Common Serial Functions

| Function | Purpose | Example |
| --- | --- | --- |
| `Serial.begin(115200)` | Initialize serial, set baud rate | Put in `setup()` |
| `Serial.print("hello")` | Print string, no newline | Output `hellohellohello` |
| `Serial.println("hello")` | Print string with newline | Output `hello` (each line independent) |
| `Serial.printf("%d", 123)` | Formatted output | Output `123` |
| `Serial.printf("Raw: %d, V: %.2fV", raw, voltage)` | Multiple variables formatted | Output `Raw: 2048, V: 1.65V` |

### `printf` Format Specifiers

| Specifier | Type | Example | Output |
| --- | --- | --- | --- |
| `%d` | Integer | `printf("%d", 123)` | `123` |
| `%f` | Float | `printf("%f", 3.14)` | `3.140000` |
| `%.2f` | Float (2 decimal places) | `printf("%.2f", 3.14159)` | `3.14` |
| `%s` | String | `printf("%s", "hello")` | `hello` |
| `%4d` | Integer (right-aligned, 4 wide) | `printf("%4d", 42)` | `__42` |
| `\n` | Newline | `printf("line1\nline2")` | `line1`<br>`line2` |

### JSON Format Output

JSON (JavaScript Object Notation) is a lightweight data format using key-value pairs.

**Why JSON?**
- Structured, easy for programs to parse
- Human-readable and debuggable
- Cross-language (Python, JavaScript, C++ can all parse it)

**Example format:**

```json
{"sensor": 2048, "voltage": 1.65}
{"sensor": 4095, "voltage": 3.30}
{"sensor": 0, "voltage": 0.00}
```

Each record is on its own line, separated by newlines. This is called **JSON Lines** (or NDJSON), ideal for streaming data.

### Experiment 1: JSON Serial Output

> Date: 2026-09-14
> Status: 🚧 In Progress
>
> Hardware: Same as Day 12 Experiment 1 (GPIO1 → potentiometer middle pin, side pins to 3V3 and GND)

**Circuit:** Reuse Day 12 potentiometer circuit, no new hardware needed.

**Code:** [`day-13/实验1-JSON串口输出/实验1-JSON串口输出.ino`](./day-13/实验1-JSON串口输出/实验1-JSON串口输出.ino)

```cpp
#include <RgbCycle.h>

const int ADC_PIN = 1;   // GPIO1 = ADC1_CH0

void setup() {
  RgbCycle::begin();
  RgbCycle::setInterval(800);

  Serial.begin(115200);
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);  // full 0-3.3V range
}

void loop() {
  RgbCycle::update();

  int raw = analogRead(ADC_PIN);
  float voltage = raw * 3.3 / 4095.0;

  // Output JSON format string via Serial.printf, easy for Python/browser parsing
  Serial.printf("{\"sensor\": %d, \"voltage\": %.2f}\n", raw, voltage);

  delay(1000);
}
```

**Code explanation:**

```cpp
Serial.printf("{\"sensor\": %d, \"voltage\": %.2f}\n", raw, voltage);
```

| Part | Meaning |
| --- | --- |
| `"{\"sensor\": %d, \"voltage\": %.2f}\n"` | JSON format string. Note: double quotes inside JSON need `\"` escaping |
| `%d` | Placeholder for `raw` (integer) |
| `%.2f` | Placeholder for `voltage` (float with 2 decimal places) |
| `\n` | Newline, each JSON record on its own line |
| `raw, voltage` | Actual values passed to `printf` |

**Escape note:** In C strings, double quote `"` is the string boundary. To represent a double quote character inside a string, escape it with backslash: `\"`. So JSON's `{"key": value}` becomes `"{\"key\": value}"` in a C string.

**How to run:**

1. Upload code to ESP32
2. Open Arduino IDE → Tools → **Serial Monitor**
3. Confirm baud rate is set to **115200**
4. Rotate the potentiometer and observe serial output

**Expected output:**

```
{"sensor": 1024, "voltage": 0.82}
{"sensor": 2048, "voltage": 1.65}
{"sensor": 4095, "voltage": 3.30}
{"sensor": 0, "voltage": 0.00}
```

Each record is on one line, can be copy-pasted into a JSON parser for verification.

### Parsing Serial Data with Python

The JSON data from serial can be read and visualized with a Python script:

```python
import serial
import json

ser = serial.Serial('/dev/ttyUSB0', 115200)  # Linux/Mac
# ser = serial.Serial('COM3', 115200)       # Windows

while True:
    line = ser.readline().decode('utf-8').strip()
    if line:
        data = json.loads(line)
        print(f"Sensor: {data['sensor']}, Voltage: {data['voltage']}V")
```

### What Went Wrong & How to Fix

- **Garbled serial output**: baud rate mismatch. Fix: confirm `Serial.begin(115200)` matches the serial monitor setting.
- **JSON parse failure**: unescaped double quotes in the string. Fix: use `\"` instead of `"` for JSON internal quotes.
- **No output in Serial Monitor**: USB cable is power-only. Fix: use a data-capable USB cable.
- **No line separation between outputs**: missing `\n`. Fix: add `\n` at the end of `Serial.printf()`.
- **JSON format error**: missing comma after value, or keys not wrapped in double quotes. Fix: check against JSON syntax.

---
## Day 14 — Week 2 Project: Digital Voltmeter

> Date: 2026-09-15
> Status: ✅ Complete (`CAL_SCALE = 1.0055` uploaded and verified: 3V3 → 3.23V, USB 5V → 4.99V)
>
> Hardware: ESP32-S3 N16R8 + 10kΩ resistor + 1kΩ resistor + jumper wires
> Core: voltage divider, ADC conversion, multi-sample averaging, factory calibration, single-point calibration

### Goal

Turn the ESP32-S3 into a **working DC voltmeter**, tying together Day 8 (GPIO), Day 12 (ADC) and Day 13 (Serial).

Key insight: ESP32 GPIOs tolerate only 0–3.3V — connecting 5V directly will destroy the pin. **The voltage divider is not optional; it is protection.**

### Voltage Divider

```
   Voltage under test VIN
        │
    ┌───┴───┐
    │ 10kΩ  │  R_HI
    └───┬───┘
        │
        ├────────────── GPIO1 (ADC1_CH0)
        │
    ┌───┴───┐
    │  1kΩ  │  R_LO
    └───┬───┘
        │
       GND
```

**Formulas:**

```
V_PIN = VIN × R_LO / (R_HI + R_LO)     // voltage at GPIO1
VIN   = V_PIN × (R_HI + R_LO) / R_LO   // recover input voltage (this line is in the code)
```

Ratio = **11**, so `VIN = V_PIN × 11`, range = 3.3 × 11 = **36.3V**.

**Why 10kΩ + 1kΩ instead of the guide's 1MΩ + 100kΩ:**

| Option | Equivalent source impedance | Consequence |
|--------|---------------------------|-------------|
| 1MΩ + 100kΩ | ≈ 91kΩ | ADC sampling cap can't charge in time → reading 5–15% low, needs a 100nF cap to patch it |
| **10kΩ + 1kΩ** | **≈ 909Ω** | Sampling is accurate, **no capacitor needed** |

Trade-off: input impedance drops from 1.1MΩ to 11kΩ. Fine for batteries and power rails.

### Wiring

Two resistors **in series**, sharing **one hole** in the middle column — that shared node is the divider midpoint:

| Step | Action |
|------|--------|
| 1 | 10kΩ from col 10 → col 15; 1kΩ from **col 15** (shared midpoint) → col 20 |
| 2 | col 15 → **GPIO1** |
| 3 | col 20 → **GND** |
| 4 | A wire from col 10 is your **probe** — leave it disconnected for now |

⚠️ **Connect GND first, VIN last**; reverse when disconnecting. ⚠️ **Power the ESP32 first** (USB plugged in) before connecting the voltage under test.

### Code

[`day-14/实验1-数字电压表/实验1-数字电压表.ino`](./day-14/实验1-数字电压表/实验1-数字电压表.ino)

```cpp
// Multi-sample averaging. analogReadMilliVolts() applies the factory
// calibration stored in eFuse — far more accurate than multiplying by 3.3.
long sumRaw = 0;
float sumMv = 0.0;
for (int i = 0; i < SAMPLES; i++) {
  sumRaw += analogRead(ADC_PIN);
  sumMv  += analogReadMilliVolts(ADC_PIN);
  delayMicroseconds(200);
}
float avg  = sumRaw / (float)SAMPLES;
float vPin = sumMv / SAMPLES / 1000.0;

// Recover the input voltage
float vin = vPin * (R_HI + R_LO) / R_LO * CAL_SCALE;
```

### Measured Results

**Probe floating** — reads ~0, blue status LED:

- Circuit: [`day-14/表笔悬空电路.png`](./day-14/表笔悬空电路.png)
- Reading: [`day-14/表笔悬空读数.png`](./day-14/表笔悬空读数.png)

**Probe on 3V3 (before calibration)** — reads ~3.24V, green status LED:

- Circuit: [`day-14/表笔接入3V3电路.png`](./day-14/表笔接入3V3电路.png)
- Reading: [`day-14/表笔接入3V3读数.png`](./day-14/表笔接入3V3读数.png)

**Probe on 5V USB cable** — red = 5V, black = GND, black shares ground with the ESP32:

- Circuit: [`day-14/表笔接入5V的电路.png`](./day-14/表笔接入5V的电路.png)

**After calibration (`CAL_SCALE = 1.0055` uploaded)** — 3V3 reads 3.23V, USB 5V reads 4.99V:

- Reading: [`day-14/校准后的读数.png`](./day-14/校准后的读数.png)

> ⚠️ 4.99 does **not** mean the 5V point is accurate: the USB cable's true voltage was never
> measured, and chargers commonly output 5.1~5.2V unloaded. If it is actually 5.14V, then 4.99
> is also −3.0% off — exactly matching 3V3. The two points don't disagree; the 5V point
> simply proves nothing.

### Status Indicator

From Day 14 on, the onboard RGB LED stops being a "heartbeat" and becomes a **status indicator**:

| Color | Meaning |
|-------|---------|
| 🟢 Green | Normal, 1V ≤ VIN < 30V |
| 🔵 Blue | VIN < 1V, reading unreliable (poor ADC linearity at the low end) |
| 🔴 Red | VIN ≥ 30V, near/over range, dangerous |

Implementation detail: `RgbCycle::begin()` still initializes the LED, but **`RgbCycle::update()` is no longer called** (it cycles colors and would overwrite the status color). `setColor()` is called only when the status **changes**.

### Calibration Result (`CAL_SCALE = 1.0055` uploaded and verified)

| Source | Meter reading | Multimeter truth | Error |
|--------|---------------|------------------|-------|
| Onboard 3V3 | 3.23 V | 3.33 V | **−3.0%** |
| USB cable 5V | 4.99 V | not measured | unknown |
| Probe floating | 0.00 V | 0 V | — |

⚠️ **4.99 does not mean the 5V point is accurate**: the USB cable's true voltage was never
measured, and chargers commonly output 5.1~5.2 V unloaded. If it is actually 5.14 V, then 4.99
is also −3.0% off — exactly matching 3V3. The two points don't disagree; the 5V point
just proves nothing.

Pitfalls:

1. The pre-calibration reading was once mis-recorded as 3.16 V, yielding `1.0538` —
   after uploading, the meter read **3.49 V (+4.8%)**, worse than the un-calibrated −2.7%.
2. Readings drift ±1.2% (three 3V3 measurements the same day: 3.24 → 3.31 → 3.23),
   so you must **measure simultaneously**: clamp the multimeter on the divider's VIN input
   and read it at the same instant as the serial output.
3. Two-point calibration `vin = a × vPin + b` is not viable yet: the fit gives `b = +0.263 V`,
   meaning a grounded probe should read 0.26 V — contradicting the observed `adc 0 → 0.00 V`.

**Conclusion: keep `1.0055` and stop tuning it. Do one simultaneous measurement next.**

Full data and derivation in [`day-14/校准记录.md`](./day-14/校准记录.md).

### Error Sources & Handling

| Source | Magnitude | Handling |
|--------|-----------|----------|
| Vref inaccuracy | ±5% | ✅ `analogReadMilliVolts()` reads factory eFuse calibration |
| ADC noise | ±1–2 LSB | ✅ 16-sample averaging |
| High source impedance | -5–15% | ✅ Fixed at the root by using 10kΩ + 1kΩ |
| Resistor tolerance | ±10% | ⚠️ Single-point calibration |
| ADC non-linearity at both ends | low/high end | ⚠️ Avoid; code flags `LOW` |

### What Went Wrong & How to Fix

- **Reading drops to 0 when the probe touches the 5Vin pin**: 5Vin is a power **input**, not a regulated output — it has no drive capability. A multimeter (10MΩ input) reads 4.4V, but connecting an 11kΩ load collapses the voltage to 0. Fix: use 3V3 (onboard LDO output, has drive capability) as the calibration reference.
- **Floating reading drifts instead of 0**: 1kΩ not connected to GND, or the two resistors don't share a midpoint.
- **Reading jumps erratically**: the source under test doesn't share GND with the ESP32.
- **Consistent ~10% offset**: resistor tolerance — run single-point calibration.
- **Compile error naming an enum `OK`/`LOW`**: the Arduino core already defines these as macros. Rename to `V_OK`/`V_TOO_LOW`/`V_TOO_HIGH`.

---
## Day 15 — Soldering Safety & Basic Practice

> Date: 2026-09-15
> Status: ✅ **Complete** (accepted 2026-09-16: tinning + ≥5 joints on scrap PCB + continuity check + pull test all passed)
>
> Hardware: 40W soldering iron kit + DT9205A PRO multimeter + scrap PCB + pin header + DuPont wires
> Core: tip tinning → 5-step soldering → cold-joint recognition → continuity check
> Code: none (Day 15 is pure hand work; the ESP32 stays off the bench)

Full notes: [`day-15/README.md`](./day-15/README.md)

**Photos**: [`焊接导线测导通.png`](./day-15/焊接导线测导通.png), [`焊接5个焊点.png`](./day-15/焊接5个焊点.png)

### Goal

Turn "DuPont wires that fall off when you look at them" into joints that survive vibration. This is the gateway skill for Week 3 — the robot car will shake, and a cold joint means it dies halfway across the room.

Key insight: **the iron heats the pad and the lead, not the solder wire.** Melting solder on the tip and painting it on is the number one source of cold joints.

### The 5-Step Method

```
① Heat the pad AND the lead (not the solder)
      ↓
② Feed solder at the pad/lead junction (not onto the tip)
      ↓
③ Solder flows and fills the pad on its own  →  remove wire first
      ↓
④ Then remove the iron
      ↓
⑤ Hold still, let it freeze (1–2 s)
```

Three things account for 90% of beginner mistakes: heat the workpiece **before** feeding solder; feed from the **opposite** side so capillary action pulls it in; use only a **grain-of-rice** amount of solder.

**Three hard criteria for "how little is little"** (don't eyeball it): pull the wire away **within 1 second** of feeding / the solder edge **stays inside the pad** (a ring of bare copper remains visible) / it's a **small mound**, not a ball (the lead pokes out of the top).
> Exception: when soldering **stranded wire**, the solder must wick through the whole bundle — noticeably more than a grain of rice, and that's correct.

### Joint Self-Check

| Check | Good | Bad |
|-------|------|-----|
| Shape | Cone / small mound | Ball / spike with a whisker |
| Surface | **Shiny** (mirror-like) | Dull, grainy, "tofu dregs" |
| Wetting | Solder covers the whole pad | Beaded up, piled on one side |
| Amount | Grain of rice | Too much (near bridge) / too little (hole visible) |
| Strength | Can't pull it off | Falls off when pulled |

**Cold joints are the sneaky ones**: they look fine to the eye and only reveal themselves under a pull test or a continuity check.

### Objective Test: Multimeter Continuity

Switch the DT9205A PRO to continuity mode:

- Across the **two ends of the same wire** → **beeps** (conducting)
- Between this wire and an **adjacent conductor** → **silent** (insulated, no bridge)

Both must pass.

> ⚠️ **Ask one question before measuring**: were these two points **already connected**? If the board already has a copper trace joining them, the beep says nothing about your joint.
> Correct order: **measure before soldering (should be silent) → measure after (beeps)** — only the before/after contrast is evidence.

### Acceptance Measurements (2026-09-16)

| Item | Result |
|------|--------|
| Joints on scrap PCB | ✅ 5–7 separate joints; shape / wetting / amount / no bridges all pass |
| Validity of the continuity test | ✅ **The two holes were NOT connected on the bare board → connected after soldering** — rules out a pre-existing trace, proving the joint itself conducts |
| Pull test | ✅ Light pull along the wire direction — **it does not come off** |

> 📌 General methodology: **any continuity test must first ask "was it already connected?"** Without a control measurement, a beep proves nothing. Same approach applies to Day 16 desoldering and Day 21 wiring troubleshooting.

### Bonus Exercise: Deliberately Solder One Cold Joint

"Shiny" vs "dull and grainy" cannot be imagined from words. How to make a bad one: press the iron on for **only 1 second** (the workpiece never reaches temperature), or **wiggle the wire while it freezes** → the solder sits on the surface as a grey, granular, tofu-dregs mess. Shoot the good and the bad side by side for the clearest comparison.

### Deviations from the Guide

| Guide says | This README | Why |
|------------|-------------|-----|
| Practice on perfboard | Use a **scrap PCB** instead | 3 perfboards are in the mail; what you're practicing is solder volume and timing, so scrap is equivalent |
| Solder two DuPont **terminals** | Solder a **DuPont-to-DuPont splice** | No crimp terminals on hand; the splice directly produces the extension leads Day 21 needs |
| Good-vs-bad comparison photo | In progress | You need to know what good and bad look like before you can deliberately make a bad one — makes more sense at the end of the practice |

---
## Day 16 — Through-Hole Soldering (5 Resistors in Series)

> Date: 2026-09-17
> Status: 🟡 **In progress** (stage 1 accepted 2026-09-18: ① no cold joints ✅ (whole chain 4.5kΩ, segments add up) ② no shorts ✅ (joint ↔ adjacent hole = OL); desoldering technique recorded, awaiting a real job; **only ③ joint finish remains ⚠️ too little solder + it climbed the lead — touch-up optional**)
>
> Hardware: 40W soldering iron kit + DT9205A PRO multimeter + 5×7 cm double-sided perfboard + resistors 330Ω/1kΩ/2kΩ/1kΩ/220Ω
> Core: perfboard layout → shared-hole series chain → elevated 3 mm lead bend → segment-sum self-check
> Code: none (Day 16 is pure hand work; the ESP32 stays off the bench)

Full notes: [`day-16/README.md`](./day-16/README.md)

**Photos**: [`五个电阻串联焊接.png`](./day-16/五个电阻串联焊接.png) (front), [`五个电阻串联背面焊接点.png`](./day-16/五个电阻串联背面焊接点.png) (solder side)

### Goal

Take the solder-volume and timing skills practised on scrap PCB on Day 15 and apply them to a **real circuit**: solder a resistor series chain on perfboard and verify it in segments with the multimeter.

Three acceptance criteria: ① no cold joints ✅ passed ② no shorts (>1MΩ between adjacent pads) ✅ passed ③ neat joints ⚠️ flawed — too little solder, it climbed the lead.

### Perfboard ≠ Breadboard

| | Breadboard | Perfboard |
|---|-----------|-----------|
| Inside the holes | **Copper strips** — a row of 5 holes is connected | **Every hole is an isolated pad**, connected to nothing |
| Connected? | Yes, as soon as you plug in | No — nothing connects until **you solder it** |

**So a series chain on perfboard needs no jumpers**: push two component leads into **the same hole**, wrap them in solder, and they are one electrical node.

### The Shared-Hole Series Chain

```
  hole1   hole3   hole5   hole7   hole9   hole11
   ●───────●───────●───────●───────●───────●
   │ R1    │ R2    │ R3    │ R4    │ R5    │
  330Ω    1kΩ     2kΩ     1kΩ     220Ω
```

Holes 3/5/7/9 each take two leads; the solder blob is the node. Zero jumpers.

**Why different values**: the cumulative sum becomes unique — the number you measure tells you which link the chain is broken at. With five identical 1kΩ parts, reading 3kΩ only tells you "three are connected", not *which*.

### Mounting: Elevated Lead Bend (bent 3 mm from the body) + Splayed Leads

- Insert from the **component side, solder on the back**; bend each lead 90° **about 3 mm from the body** — not right against the body
- Span = body length + 6 mm ≈ **12.5 mm ≈ 5 hole pitches** → **the two holes the leads go through are far apart**, and the body sits **elevated above the board**, not flat on it
- About **3 mm** of lead sticks out the back — just enough to solder, so **no lead is wasted and you barely need to trim anything**
- On the back, splay the two leads **outward 30–45°** ("splayed feet") → the part doesn't fall out when you flip the board (with this bend they already splay outward; a light nudge is enough)
- ⚠️ **"Insert less so only 3 mm sticks out" ≠ "insert fully so 3 mm sticks out."** The former leaves the body floating a dozen mm above the board. **Insertion depth is set by where you bend the lead, not by how hard you push.**
- ❌ For contrast: the textbook "flat horizontal ∏ bend" bends 1–2 mm from the body, spans 2 hole pitches (5.08 mm), lays the body flat on the board, and leaves 3–5 mm to trim. **Not what we used here.**

### ⭐ The Segment-Sum Self-Check

| Measured across | Theoretical | Measured | Verdict |
|-----------------|-------------|----------|---------|
| Whole chain (end ↔ end) | 4.55kΩ | **4.5kΩ** | ✅ |
| R1+R2 | 1.33kΩ | **1.3kΩ** | ✅ |
| R3+R4+R5 | 3.22kΩ | **3.2kΩ** | ✅ |

```
1.33k + 3.22k = 4.55k = whole chain
1.3k  + 3.2k  = 4.5k  = measured whole chain 4.5k  ✅
```

**The two segments divide the chain with no overlap and no gap → the segment point (hole 3, the shared hole) really does conduct.** If that hole were cold, the first segment would read infinite and the second would not be 3.2k.

Measuring "the whole chain is 4.5k" only proves the chain isn't broken somewhere. Adding "the segments add up to the whole" also proves the intermediate node conducts. Far stronger than a continuity beep — a beep says "connected"; a resistance value says "connected *where*".

> 📌 An upgrade on the Day 15 methodology (*"every continuity test must first ask 'was it already connected?'"*): **don't measure one grand total — cut it into segments that cross-check each other.** Same approach for Day 21 wiring troubleshooting.

### Joint Quality

| Item | Verdict |
|------|---------|
| Conducting ✅ / no cold joints ✅ | ✅ |
| **No shorts** | ✅ On the 2MΩ range, joint ↔ adjacent empty hole = **OL** (>2MΩ); all four shared holes checked individually |
| ⚠️ **Too little solder** | The outer ring of the pad is still bare copper; solder covers only the middle (standard: covers the whole pad, edge contained within the pad) |
| ⚠️ **Elongated shape** | Solder climbed up the lead into a "pillar" instead of a "mound"; caused by feeding solder against the lead and heating too long |
| Leads not trimmed | ✅ Deliberate — desolder and reclaim all five resistors later |

To add solder: press the iron on the **pad and the base of the lead**, feed solder from the **opposite** side, and when you see it wet the whole pad on its own, remove the wire first, then the iron.

### Short Test (Criterion ②) ✅ Done

Measured point by point on the 2MΩ range — result: **OL**.

| Measured across | Should read | Measured |
|-----------------|-------------|----------|
| Joint ↔ adjacent empty hole (off-chain) | OL / >1MΩ | **OL** ✅ |
| The four shared holes (3/5/7/9) ↔ surrounding empty holes | OL / >1MΩ | **OL** ✅ |

> **OL = Over Load (over-range / open circuit).** The standard is >1MΩ; OL on the 2MΩ range means >2MΩ — far above it.
>
> ⚠️ **The same OL means opposite things on-chain vs off-chain:** OL across an **off-chain** isolated hole = ✅ no short; OL across two **on-chain** points (e.g. hole 1 ↔ hole 3, with R1 between them) = ❌ a cold/open joint — that should read about 330Ω.
>
> Don't grip the **metal** of both probes with your fingers while measuring insulation (your body resistance shunts in parallel and pulls OL down to 1.x MΩ); red probe goes in `VΩmA`, not the 10A jack.

### Desoldering Technique

Follow this when a real job comes up (wrong hole, reclaiming parts, Day 21 rework) — no separate practice session.

- One hand melts with the iron, the other sucks — **cock the plunger before heating**
- **Add solder to remove solder**: old solder has no flux left and flows poorly
- ⚠️ Never yank the lead (tears the pad off); empty the chamber while hot

### Deviations from the Guide

| Guide says | This README | Why |
|------------|-------------|-----|
| 5 resistors in series on perfboard | ✅ Kept (330/1k/2k/1k/220) | Switched to **different values** so the cumulative sum is unique and locates the break |
| ESP32-S3 header → breakout board (44 pins) | ⏭️ **Skipped** | The dev board ships with headers already soldered; re-soldering teaches nothing, and one bridge across 44 dense pins could kill the board |
| USB Type-C → custom power board | ⏭️ **Skipped** | No Type-C receptacle purchased; it's an extension. The dev board's own USB powers everything for now |
| Desoldering practice | 📌 **Technique recorded, no separate session** | Solder sucker has arrived; do it when a real job comes up (Day 21 rework). The five resistors stay on the board as a Day 16 record |

> Principle: **skip any component you can skip.** Neither skipped item blocks the Day 17–21 ultrasonic / IMU / motor wiring.

---
## Day 17 — HC-SR04 Ultrasonic Ranging

> Date: 2026-09-18
> Status: ✅ **Complete** (2026-09-19): all five LED tiers hit, open-air control passed, four points off by −0.1 to −1.0 cm
>
> Hardware: ESP32-S3 (N16R8) + HC-SR04 ultrasonic module (wide-voltage 3–5.5 V version)
> Core: timing-based communication → Trig trigger / Echo pulse → speed-of-sound conversion → timeout & range checks
> Code: [`实验1-超声波测距.ino`](./day-17/实验1-超声波测距/实验1-超声波测距.ino)

Full notes: [`day-17/README.md`](./day-17/README.md)

### Principle

Trig gets a **≥10 μs high pulse** → the module emits eight 40 kHz bursts → **Echo goes high, drops when the echo returns**. The distance is encoded in the **width** of that Echo high:

```
distance cm = duration(μs) / 58 = duration(μs) × 0.0343 / 2
```

0.0343 cm/μs is the speed of sound (343 m/s); divide by 2 because the pulse travels there and back. Easier to remember: **58 μs ≈ 1 cm**.

Range 2–400 cm, detection cone about **15°** (not a straight line — which is exactly why Day 20 puts the sensor on an SG90 to sweep left and right).

### Wiring

| HC-SR04 | ESP32-S3 |
|---------|----------|
| VCC | **3V3** (either of the two 3V3 pins) |
| GND | GND (**must be common**) |
| Trig | **GPIO4** |
| Echo | **GPIO5** (direct, no divider) |

Not the guide's GPIO5/18: 4/5 sit adjacent on the header, and it keeps clear of GPIO8/9 planned for I2C on Day 18.

**Order**: GND first → Trig/Echo → VCC last; reverse when unplugging.

### ⚠️ Why 3V3, not 5Vin / 5 V

Reading `docs/ESP32-S3-Metric.pdf` suggests the board only has `5Vin` and no 5 V. **Careful: that PDF is a mechanical drawing** — it contains only part designators (`PAU4024` / `PAJ102` / `PARGB01`) and dimensions (25.40 / 27.94 / 43.18 mm). Searching it for `5V` / `3V3` / `VIN` / `GND` gives **zero hits**, so it cannot define pin functions. The real net names live in the schematic `docs/ESP32-S3-SCH.pdf`: `VBUS` / `5V` / `3V3` / `VDD33` / `GND`.

And **the "in" in `5Vin` means input** — it is the supply entry into the board, not a regulated output. **This project already hit that trap on Day 14** (see the pitfalls note): a multimeter reads 4.4 V on 5Vin, but connecting an 11 kΩ load collapses the voltage to 0. The HC-SR04 draws tens of mA when it transmits, which would brown out the whole board.

| Pin | Nature | Can it power the HC-SR04? |
|-----|--------|---------------------------|
| **3V3** | On-board LDO **output** (~1 A) | ✅ **Use this** |
| **5Vin** | Power **input** | ❌ No drive capability, collapses |
| 5 V header | Usually tied to VBUS | ⚠️ Works, but see below |

3V3 has an extra payoff: **the wide-voltage HC-SR04's Echo high follows VCC**, so at 3V3 the Echo is 3.3 V and wires straight to GPIO5 — **no 1kΩ+2kΩ divider at all**, sidestepping the guide's "5 V into GPIO will burn it" warning entirely. If you insist on 5 V, Echo outputs 5 V and a divider becomes mandatory.

The cost is slightly lower transmit power, so the long-range limit is theoretically a bit shorter — the measurement table will show how far 3.3 V actually reaches. If it falls short, switch to the 5 V header plus a divider.

### Code highlights

```cpp
digitalWrite(TRIG_PIN, LOW);  delayMicroseconds(2);   // clean rising edge
digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10);  // datasheet: ≥10 μs
digitalWrite(TRIG_PIN, LOW);
return pulseIn(ECHO_PIN, HIGH, 30000);   // blocks for the high pulse; 0 on timeout
```

`pulseIn()` is today's new function. **It returns 0 on timeout** (not −1), so the failure test is `duration == 0`.
⚠️ It **blocks** for up to 30 ms — fine today, but on Day 21 the car has to run motors + servo + ranging at once, so it needs a non-blocking rewrite. Noted.

LED colour keeps the Day 14 status convention: red (<30 cm) / orange (30–150 cm) / green (>150 cm) / blue (timeout). Enum again avoids the `LOW`/`HIGH` macros: `D_NEAR`/`D_MID`/`D_FAR`/`D_INVALID`.

### Build

✅ Compiles (316,424 bytes / 24% flash).

### Measurements (first pass, 2026-09-19)

| # | Scene | True (ruler) | Serial cm | Error | zone / LED |
|---|-------|--------------|-----------|-------|------------|
| 1 | Hand | 6.7 | 6.5 | −0.2 | 🔴 NEAR ✅ |
| 2 | Book | 20.2 | 20.0 | −0.2 | 🔴 NEAR ✅ |
| 3 | Book | 50.1 | 50.0 | −0.1 | 🟠 MID ✅ |
| 4 | Distant target | 174.0 | 173.0 | −1.0 | 🟢 FAR ✅ |
| 5 | **Open air, no target** | — | 0 | — | 🔵 TIMEOUT ✅ |

✅ **All five LED tiers match** the code thresholds. <br>
✅ **Row 5 — the mandatory control — passed**: open air reports TIMEOUT rather than a plausible number, proving rows 1–4 measured real echoes. <br>
✅ **The 3V3 supply choice holds**: 173 cm still reads reliably, so there is no need to fall back to the "5 V + divider" plan. <br>
⚠️ **Readings skew low systematically** — all four points are negative (−0.1 to −1.0 cm), so this is not jitter. Main cause: room-temperature sound speed (~346 m/s) exceeds the 343 m/s used in the code, about −0.87% in theory; an offset in the measurement origin contributes too. Day 21 only asks "is something there?", so a sub-1 cm bias is irrelevant — **recorded, not compensated**.

### Deviations from the Guide

| Guide says | This README | Why |
|------------|-------------|-----|
| Trig→GPIO5, Echo→GPIO18 | **GPIO4 / GPIO5** instead | Adjacent on the header; keeps clear of Day 18 I2C |
| "Level shifting required" / VCC→5 V | **VCC→3V3, no divider** | 5Vin is an input with no drive capability (Day 14 trap); Echo at 3.3 V is directly safe |
| Output: measurements + timing diagram | ✅ Data filled in, diagram drawn | See the table above and [`day-17/时序图.png`](./day-17/时序图.png) |

**Timing diagram**: [`day-17/时序图.png`](./day-17/时序图.png)

---
## Day 18 — MPU-6050 IMU & I2C

> Date: 2026-09-19
> Status: ✅ **Bench-tested, both experiments pass**
>
> Hardware: ESP32-S3 (N16R8) + MPU-6050 breakout (onboard LDO + pull-ups) + 4 female-to-female jumpers
> Core idea: bus communication → addressing → register reads → accel/angular rate → recover attitude
> Code: [`I2C扫描.ino`](./day-18/实验1-I2C扫描/实验1-I2C扫描.ino) | [`IMU读数.ino`](./day-18/实验2-IMU读数/实验2-IMU读数.ino)
> Captures: [`实验1-I2C扫描.png`](./day-18/实验1-I2C扫描.png) | [`实验2-IMU读数.png`](./day-18/实验2-IMU读数.png) | [`实验2-IMU读数2.png`](./day-18/实验2-IMU读数2.png) | [`实验2-IMU读数3.png`](./day-18/实验2-IMU读数3.png)

Full notes: [`day-18/README.md`](./day-18/README.md)

### From Timing to Bus

| | Day 17 ultrasonic | Day 18 IMU |
|---|---|---|
| Medium | **Timing**: pulse width encodes distance | **Bus**: two wires converse by address + register |
| Multiple devices? | ❌ one device per line | ✅ many share SDA/SCL, told apart by address |

Two details worth keeping: ① addresses are **7 bits**, the 8th bit is read/write direction — you write `0x68` and the library does the shifting; ② every byte must be acknowledged by the receiver pulling SDA low (**ACK**), which is exactly how the scanner decides whether anyone lives at an address.

⚠️ I2C is **open-drain**: pins can only pull low, never drive high, so pull-up resistors hold the line idle-high — **no pull-ups means no ACK, ever**. The MPU-6050 breakout ships with 4.7 kΩ pull-ups, so this project needs zero extra parts. When wiring a bare chip (not a module) you must add them yourself.

### Wiring

| MPU-6050 | ESP32-S3 |
|----------|----------|
| VCC | **3V3** (breakout has its own LDO; **not 5 V** — its pull-ups would then sit above the 3.3 V IO limit) |
| GND | GND (**must be common**) |
| SDA | **GPIO8** |
| SCL | **GPIO9** |

Order: GND first → then SDA/SCL → VCC last. Reverse it when unplugging.

The ESP32-S3 has **no fixed I2C pins** — any GPIO can be routed as SDA/SCL via `Wire.begin(SDA, SCL)`. That differs completely from AVR (the Uno's A4/A5 are hard-wired). But avoid the **strapping pins 0 / 3 / 45 / 46**: their level is sampled at boot to select the startup mode, and a module hanging off them can skew it. Day 17's choice of GPIO4/5 already left today's 8/9 free.

### Why the Accelerometer Reads 9.8 at Rest

An accelerometer sitting on a desk does **not** measure gravity — it measures the table's normal force, equal to gravity and pointing up. What it actually senses is *not falling*. Flat ⇒ `az ≈ 9.8`, and that is the fastest sanity check for whether the readings are right. In free fall it reads 0.

The harder check is the **magnitude**: `|a| = √(ax²+ay²+az²)` must stay 9.81 at rest regardless of orientation. Looking only at `az` cannot tell "not level" from "wrong scale" — the magnitude splits the two cleanly.

### Recovering Tilt from Gravity

```cpp
float roll  = atan2(a.acceleration.y, a.acceleration.z) * 180.0 / M_PI;
float pitch = atan2(-a.acceleration.x,
                    sqrt(a.acceleration.y * a.acceleration.y +
                         a.acceleration.z * a.acceleration.z)) * 180.0 / M_PI;
```

Two traps: ① **use `atan2`, not `atan`** — `atan` spans only ±90° and loses the sign, `atan2` covers all four quadrants out to ±180°; ② pitch's denominator must be the **YZ vector magnitude** — the lazy `atan2(-ax, az)` goes wrong once roll gets large.

> 📌 This holds **only at rest or constant velocity**: any extra linear acceleration superposes onto gravity and the "tilt" becomes fiction. That is exactly why the gyroscope has to be fused in.

### The Two Sensors Trade Off

| | Accurate long-term | Vibration-proof | Drifts |
|---|---|---|---|
| Accel-derived angle | ✅ always points "down" | ❌ jumps when shaken | no |
| Integrated gyro | ❌ error accumulates | ✅ immune to linear accel | **yes** (`gz` is not exactly 0 even at rest) |

Neither works alone. **Complementary and Kalman filters** are the engineering answer, and both are out of scope today. Per the guide, this day uses the accelerometer alone for tilt; gyro data is read and printed to get a feel for the magnitudes.

> ⚠️ Gyro units are **rad/s, not °/s** — multiply by `180/π` for degrees per second.

### Build

✅ Both pass (`--fqbn esp32:esp32:esp32s3`): I2C scanner 339664 bytes / 25%, IMU reader 350632 bytes / 26%.

Dependency chain: `Adafruit MPU6050` (2.2.9) → `Adafruit BusIO` (1.17.4) + `Adafruit Unified Sensor` (1.1.15). Missing any one of them breaks the build.

### Measurements

**Scanner**: 7 consecutive rounds from 12:25:42 to 12:26:00, every one landing `found 0x68 = MPU-6050 (AD0=GND)` with `1 device(s) found.`, LED green.

**IMU**: `MPU6050 ready. SDA=GPIO8 SCL=GPIO9`, all three tilt zones triggered, LED followed correctly.

Ten frames averaged at rest: `ax=0.41, ay=-0.07, az=7.82`, `roll=-0.6°, pitch=-3.0°`.
Hand-tilted frames show what `atan2` buys:

| ax | ay | az | roll | pitch |
|---|---|---|---|---|
| 2.26 | **9.21** | **0.08** | **89.5°** | −13.8° |
| 1.87 | **9.45** | **−1.26** | **97.6°** | −11.1° |

Stood on edge reads 89.5°, and past 90° it still reports 97.6° — `atan` would have failed there.

### ⚠️ Sanity check failed: az = 7.82, not 9.8

```
|a| = √(0.41² + 0.07² + 7.82²) = 7.83 m/s²    vs. 9.81 reference → −20.2%
```

The vector still points almost straight along Z (roll/pitch only 3°), so this is **not** a levelling problem — tilting shrinks `az` but leaves the magnitude at 9.81. The magnitude itself is 2 m/s² short, which is a **sensitivity calibration error**, typical of a ¥13.80 breakout. Calibration factor `9.81 / 7.83 = 1.253`.

> 📌 This is exactly why the sanity check must read the **magnitude, not one axis**: `az=7.8` alone invites the wrong conclusion ("not level"); the magnitude separates "wrong direction" from "wrong scale".

### The obvious objection: `az` moves when I shake it, so how can it be wrong?

A gain error is **multiplicative** — it scales the varying part too, so it never pins a reading in place. At rest `az=7.83` means a true 9.81; the motion frame `az=10.56` means a true `10.56 / 0.798 = 13.2`, well inside what hand shaking produces.

The motion frames span `|a| = 5.63 ~ 10.85`, and the maximum **exceeds 9.81** — a broken sensor or a fabricated reading could never produce a value above gravity. Two independent facts:

```
responds correctly ✅ (follows attitude and motion, with plausible amplitudes)
absolute scale ✗     (everything sits 20.2% low)
```

Quantitative check: the two runs sat at different angles (`pitch` −3.0° and −5.5°, two hours apart), and one factor k = 0.798 explains both to two decimals:

| Run | true ax / az | × 0.798 | measured |
|---|---|---|---|
| 13:01 (pitch −3.0°) | 0.514 / 9.796 | 0.410 / 7.817 | 0.41 / 7.82 |
| 15:01 (pitch −5.5°) | 0.940 / 9.766 | 0.750 / 7.793 | 0.75 / 7.83 |

Pure gain error, not a stuck reading.

> 📌 Analogy: a ruler whose markings are printed 20% short. Measuring a table and a chair gives two different numbers, but both are 20% small. The test is to **measure something of known length**, not to check whether the number moves.

### Shaking up and down: the peaks reach 9.8, the average still does not

Vertical shaking is symmetric, so its linear acceleration averages to zero. The mean `az` over 25 frames must therefore equal the resting value for that tilt angle.

| | 值 |
|---|---|
| 平均 pitch | −14.0° |
| 平均 az | **7.73** |
| 增益模型预测（`0.798 × 9.81 × cos14°`） | **7.60** |
| 若无增益误差应为 | 9.52 |

模型与实测差 1.7%。至于那些冲到 `9.79 / 9.88 / 10.04 / 10.26` 的帧 —— 那是下落到最低点后**向上减速**的瞬间，向上加速度约 `3.3 m/s²`：

```
az = 10.26 → true Z specific force = 10.26 / 0.798 = 12.86
12.86 − 9.52 (gravity component at that tilt) = +3.34 m/s² upward
```

**The peaks come from acceleration, not from accurate static readings.** The average is the test, and the average is still 20% low.

### Gyro drift now has a number

Zero offset at rest: `gx=-0.028, gy=0.030, gz=-0.009 rad/s` (≈ −1.6 / 1.7 / −0.5 °/s). Running `gy` through the arithmetic:

```
0.030 rad/s × 30 s = 0.9 rad ≈ 52°
```

**Leave it still for 30 s and pure gyro integration wanders 52°.** "Integration drifts" stops being a slogan and becomes a number — and the reason the accelerometer has to be fused back in.

### The trap: experiment 2 said `not found` while experiment 1 found 0x68 on the same wires

The cause was a **jumper that was not seated all the way**. The part worth recording is why the scanner still passed:

| | Transactions | Effect of one failure |
|---|---|---|
| Scanner | one independent write per address | that address is missed; the next round hits it again |
| `mpu.begin()` | address probe + WHO_AM_I read, back to back | one dropped frame fails the whole `begin()` |

**"The scanner found it" does not mean the contact is reliable** — the scanner only needs occasional success, the library needs unbroken success. So reseat the wires before touching the code.

---

## Day 19 — DC Motors with the TB6612FNG Driver

> Date: 2026-09-20
> Status: ✅ **Bench-tested, both experiments pass** (coast segment and open-loop drift still pending)
>
> Hardware: ESP32-S3 (N16R8) + TB6612FNG motor driver module (headers soldered) + TT motor 1:48 (from the chassis kit, rated 3 V) + battery box (2 × AA = 3 V)
> Core idea: H-bridge truth table → fwd/rev/brake/coast → PWM speed control → dead zone and the open-loop limit
> Code: [`实验1-电机正反转.ino`](./day-19/实验1-电机正反转/实验1-电机正反转.ino) | [`实验2-PWM调速.ino`](./day-19/实验2-PWM调速/实验2-PWM调速.ino)
> Videos: [`实验1-电机正反转.MOV`](./day-19/实验1-电机正反转.MOV) | [`实验2-PWM调速.MOV`](./day-19/实验2-PWM调速.MOV)
> Captures: [`实验1-串口打印.png`](./day-19/实验1-串口打印.png) | [`实验2-串口打印.png`](./day-19/实验2-串口打印.png)

Full notes: [`day-19/README.md`](./day-19/README.md)

### Goal
Make a TT DC motor run forward, reverse, brake, and coast, then control its speed with PWM. The first day the code produces mechanical motion, and the first load a GPIO cannot drive.

### Hardware
ESP32-S3 dev board (N16R8) + TB6612FNG motor driver module (headers soldered) + TT motor 1:48 + battery box (2 × AA = 3 V, the TT motor's rated voltage). Six male-to-female jumpers plus the motor screwed straight into the AO1/AO2 terminals — zero extra components.

### Why a GPIO cannot drive a motor directly

Three reasons, each harder than the last:

1. **Not enough current.** An ESP32-S3 GPIO is good for roughly 40 mA continuous; a TT motor draws 100–200 mA free-running and over 1 A stalled. An order of magnitude short.
2. **It burns the GPIO.** A motor is an inductive load. When the current is interrupted, the back-EMF `V = −L·di/dt` spikes to several times the supply voltage and punches through the GPIO protection.
3. **No direction change.** A GPIO can only source 0 / 3.3 V. Reversing means swapping the two motor terminals — which needs two crossed switch pairs, i.e. an H-bridge.

> 📌 The division of labour is now fixed: **the GPIO only carries logic levels; the driver chip switches the current.**

### H-bridge and the TB6612FNG truth table

Four switches in an "H", the motor across the middle: S1+S4 closed runs forward, S2+S3 reverses, all open is coast, and S1+S2 closed shorts the two terminals into a brake. **Never close S1+S3 or S2+S4** — that is shoot-through, a dead short across the supply.

One TB6612FNG contains two H-bridges; this project uses channel A only:

| IN1 | IN2 | PWM | Output | Mode |
|:---:|:---:|:---:|---|---|
| 1 | 0 | 1 | OUT1=H, OUT2=L | **Forward** |
| 0 | 1 | 1 | OUT1=L, OUT2=H | **Reverse** |
| 1 | 1 | 1 | OUT1=L, OUT2=L | **Brake** |
| 0 | 0 | x | high-Z | **Coast** |
| x | x | 0 | high-Z | **Coast** |

> ⚠️ **Brake and coast are different things**: `IN1=IN2=1` is the brake, `IN1=IN2=0` is the coast. And with `PWM=0` the outputs are high-Z regardless of the inputs — so "IN1=IN2=1 with PWM=0" is still a coast. **Brake = shorted outputs plus drive capability; both conditions must hold.**

### PWM speed control and the open-loop limit

The H-bridge output is only on/off; PWM produces an **average voltage**: `duty 180/255 ≈ 70.6%` → about `3 V × 0.706 ≈ 2.12 V` across the motor. The frequency reuses Day 10's breathing-LED setting, `ledcAttach(PWMA, 5000, 8)`: 5 kHz is quiet and the switching loss is acceptable, a few hundred Hz squeals, much higher heats up.

**This project can only be open loop**: the code sets a duty and never learns the actual speed. The same duty slows down as the battery drains, under heavier load, and differs between two motors of the same type. The TT motors in the chassis kit have no encoder leads, so closed-loop speed control waits for around Day 28.

> ⚠️ **Multiply duty by VM to get the real voltage.** On a 3 V battery, `duty 180/255` is only 2.12 V average, below the TT motor's breakaway threshold — the motor buzzes but never turns. The 1~2 V a multimeter reads on AO1/AO2 is not a fault: the meter reports the average, not the instantaneous level. Measured breakaway for this motor sits between duty 180 and 220; at 220 all four modes check out, and only duty 220~255 — the last 14% — is left as a usable speed range on 3 V.

The serial port of the PWM ramp reveals two things the code hides: the `P_HOLD` phase goes silent for a full second because duty stops changing and `if (duty != printedDuty)` filters the repeats; and each `STEP_MS = 40` step actually runs about 33 ms, since `Serial.printf` itself costs a few milliseconds. Do not count on `millis()` throttling plus serial printing for precise timing.


### Three wiring rules that matter

- **Grounds must be shared**: battery negative, ESP32 GND, and module GND all tied together. Without a common ground the GPIO's "high" has no reference at the driver, which shows up as "code runs, motor does nothing".
- **VM and VCC are separate supplies**: VM takes the battery positive and the motor current (hundreds of mA to 1 A+); VCC takes the ESP32 3V3 and only a few mA of logic current. Load only 2 cells (3 V) — that is the kit TT motor's rated voltage; 4 cells at 6 V is double the rating, doubling the current and heat and cutting its life short.
- **STBY straight to 3.3 V** (always high): saves a GPIO and leaves the chip permanently enabled.

Motor AO1/AO2 are not polarised — swapping them only reverses the direction.

### Code notes

- The truth table is translated straight into a `switch`: brake sets `ledcWrite(PWMA, 255)`, coast sets `ledcWrite(PWMA, 0)`.
- `millis()` throttling instead of `delay(2000)`.
- The ramp starts at duty 220, not 0, because of the **dead zone** — below a threshold the motor does not turn at all until torque overcomes static friction. The dead zone is set by average voltage, not by the duty number: breakaway on 3 V lands at duty 220, so starting from 120 would buzz through 1.1 s of dead zone every cycle and leave only the last 14% of the duty range usable.
- ESP32 Arduino Core 3.x uses `ledcAttach(pin, freq, res)` + `ledcWrite(pin, duty)` with no channel number; the old `ledcSetup` / `ledcAttachPin` API is gone.

### Compile Results

```
Exp1 motor fwd/rev: Sketch uses 323479 bytes (24%) / Global variables 22196 bytes (6%)
Exp2 PWM ramp:      Sketch uses 323319 bytes (24%) / Global variables 22204 bytes (6%)
```

✅ Both pass (`--fqbn esp32:esp32:esp32s3`), no new libraries.

### Measured Results

**Experiment 1** ([`实验1-电机正反转.MOV`](./day-19/实验1-电机正反转.MOV) | [`实验1-串口打印.png`](./day-19/实验1-串口打印.png))

All four modes check out, on the condition `DRIVE_DUTY = 220`:

| Mode | LED | Measured |
|---|---|---|
| FWD | green | ✅ turns the same direction |
| REV | orange | ✅ turns the other way |
| BRAKE | red | ✅ stops immediately |
| COAST | blue | ⏳ awaiting hand-spin check |

One JSON line every 2 s, with `ain1` / `ain2` matching the LED colour exactly:

```
{"mode":"FWD","duty":220,"ain1":1,"ain2":0}
{"mode":"REV","duty":220,"ain1":0,"ain2":1}
{"mode":"BRAKE","duty":255,"ain1":1,"ain2":1}
{"mode":"COAST","duty":0,"ain1":0,"ain2":0}
```

**Experiment 2** ([`实验2-PWM调速.MOV`](./day-19/实验2-PWM调速.MOV) | [`实验2-串口打印.png`](./day-19/实验2-串口打印.png))

`START_DUTY = 220`, duty rises monotonically, one full cycle is about 1.55 s:

```
220 ──0.23s──> 255 ──1.09s steady──> 255 ──0.23s──> 220 ──immediately back up
```

The serial log confirms it line by line: `duty` 220→255, `pct` 86.3%→100.0%, `vavg` 2.59 V→3.00 V, strictly linear with duty, no reversal, no jitter. Noise stays at a faint 5 kHz hiss, no squeal.

**The dead zone deletes the second half of the demo**: the design ramps back to the start until the motor stops, but `P_DOWN` flips to `P_UP` the moment duty returns to 220 (= breakaway), so the motor never leaves the breakaway region and "slower and slower until it stops" never happens. That is the dead zone's second and more hidden cost.

### What Went Wrong & How I Fixed It

**① The motor buzzes but never turns — multiply duty by VM to get the real voltage**

The first run used `DRIVE_DUTY = 180`. Under the 6 V plan that is 4.24 V; after switching to a 3 V battery it drops to `3 V × 180/255 ≈ 2.12 V`, below the TT motor's geared breakaway threshold. A plain 3.0 V supply on the motor terminals turns it fine, which proves the motor is healthy — what fell short is the average voltage, not the drive chain.

At 220 (2.59 V) all four modes pass. **This motor's breakaway point on 3 V sits between duty 180 and 220.**

**② Only 1~2 V on AO1/AO2 with a multimeter is not a fault**

The meter cannot follow a 5 kHz switch, so it reports the PWM **average**, not the instantaneous level: `3 V × 180/255 ≈ 2.12 V`, right inside that 1~2 V band. The test is: 0 V means no output, 3.3 V means stuck on, and a value in between is exactly the H-bridge switching normally.

**③ Experiment 2 buzzing "forever" after upload is impossible in the code**

Duty increases by 5 every 40 ms, so it crosses breakaway within 1.1 s. The real cause was `START_DUTY = 120`, where the whole ramp starts inside the dead zone (average voltage 1.41 V~2.12 V). Watch the serial port: if `duty` climbs to 255 and it still will not turn, it is physical (loose terminal / sagging battery / mechanical load); if `duty` sits at the start, the flash was not the latest code.

---

## Day 20 — Servo Control

> Date: 2026-09-20
> Status: ✅ tested on hardware — the servo sweeps between 0° and 180°
>
> Hardware: ESP32-S3 (N16R8) + SG90 servo 180° + external 5 V supply + 3 female-to-female Dupont wires
> Core: position control → single-wire pulse protocol → duty conversion → power and shared ground
> Code: [`实验1-自动摆动.ino`](./day-20/实验1-自动摆动/实验1-自动摆动.ino) | [`实验2-串口控制角度.ino`](./day-20/实验2-串口控制角度/实验2-串口控制角度.ino)
> Bench files: [`实验1-自动摆动.MOV`](./day-20/实验1-自动摆动.MOV) | [`实验1-串口打印.png`](./day-20/实验1-串口打印.png) | [`实验1-电路图.png`](./day-20/实验1-电路图.png) | [`实验2-串口控制角度.MOV`](./day-20/实验2-串口控制角度.MOV) | [`实验2-串口输入角度.png`](./day-20/实验2-串口输入角度.png)

Full notes: [`day-20/README.md`](./day-20/README.md)

### Goal
Move an SG90 servo to a commanded angle and hold it there, then drive the target angle live from the serial port.

### How a servo differs from yesterday's TT motor

| | Day 19 TT motor | Day 20 SG90 servo |
|---|---|---|
| Controlled quantity | Speed (continuous rotation) | **Angle** (0°~180° positioning) |
| Feedback | None, open loop | Internal potentiometer senses position, **the loop is closed inside the servo** |
| Driver needed | H-bridge (needs reversing) | None, single-wire PWM |
| Wrong command | Spins too fast / stalls | Blocked-rotor heating, can burn out |
| Rated voltage | 3 V | **4.8 V** |

In one line: **a motor is "apply voltage and it turns"; a servo is "give it an angle and it stays there".** An SG90 is a motor + reduction gearbox + position potentiometer + control board, with the position loop already closed inside the case. Externally you only publish a target angle.

### Same PWM waveform, completely different meaning

The one sentence to remember today:

> **On Day 19 the pulse width encoded "average voltage = speed"; on Day 20 it encodes "absolute position = angle". The waveform looks identical; the physical meaning is unrelated.**

| | Day 19 motor | Day 20 servo |
|---|---|---|
| Frequency | 5000 Hz | **50 Hz** |
| Period | 0.2 ms | **20 ms** |
| Encoded quantity | High-time fraction | High-time **absolute width** |
| What the low time is | The other half of the duty cycle, part of the average | Pure gap, "no command" |

```
one 20 ms period ────────────────┬─────────────┬──────────
                                 │             │
                                 └─ 0.5 ms ─┘  = 0°
                                 └─ 1.5 ms ─┘  = 90°
                                 └─ 2.5 ms ─┘  = 180°
                the remaining ~17.5 ms low time is irrelevant
```

- Only the high-time width carries meaning; the rest is spacing. The servo re-reads the width on every rising edge.
- 0.5 ms~2.5 ms is the SG90's range, not what the protocol mandates. "Neutral = 1.5 ms" is the shared convention on generic servo testers.
- Frequency must be 50 Hz: the internal control loop needs to sample about every 20 ms. Too fast and it cannot keep up, too slow and it jitters while holding.
- No reversing is needed, so **no H-bridge** — yesterday's entire truth table is void, the first time the driver circuit gets thinner.

### Power: the biggest trap today

**The signal line can go straight to 3.3 V; the power line cannot.**

The SG90 high-level threshold is roughly 2.0~2.5 V, so the ESP32's 3.3 V is a reliable high level for it — the signal wire (yellow on this unit) connects directly to a GPIO with no level shifting. But the red wire must come from an external 5 V:

```
SG90 no load      ~100 mA
SG90 loaded       200~300 mA
SG90 blocked      700 mA +
ESP32 3V3 pin     keep under 500 mA, and it is shared with onboard logic
```

A servo is not a logic device, it is an electromechanical actuator. **"Logic-level compatible" is not "power compatible"** — the only criterion today's power section needs.

| Option | Approach | Verdict |
|---|---|---|
| ① ESP32 board 5V / VIN pin | Servo red wire to the board's 5V, USB powered | Saves a wire, but USB gives only 500 mA. **A blocked rotor can sag the USB port and reboot the whole ESP32**, and after reboot it blocks again — a loop |
| ② External 3~4 AA cells (4.5~6 V) + shared ground | Battery box feeds the servo on its own | ✅ recommended, servo current never touches the dev board |
| ③ The 6 V battery box | The one with lid and switch | 6 V is within the SG90's range (~3.5~6 V), it runs, but faster and hotter |

The SG90 is rated 4.8 V, so 6 V is 25% over. Fine for short debugging; use 3 AA cells (4.5 V) for the long run.

**Shared ground is the prime suspect again**: external supply negative, servo brown wire, and ESP32 GND all joined. Without it the GPIO high level has no reference inside the servo — the symptom is "code runs, serial port fine, servo does not move at all".

### Duty conversion and the 14-bit ceiling

```
period        = 1 / 50 Hz            = 20 ms = 20000 us
14-bit steps  = 2^14                = 16384
step size     = 20000 / 16384       ≈ 1.221 us
duty = pulse width (us) × 16384 / 20000
```

| Angle | Pulse width | duty |
|:---:|:---:|:---:|
| 0° | 500 us | **410** |
| 90° | 1500 us | **1229** |
| 180° | 2500 us | **2048** |

**The ESP32-S3's LEDC tops out at 14 bits** (`SOC_LEDC_TIMER_BIT_WIDTH = 14`, confirmed by forcing the value into a compile error with a `template <int N> struct Reveal;` probe). The 16 bits every servo tutorial uses is a **first-generation ESP32** number. Copy it onto an S3 and `ledcAttach` returns false at the `resolution > SOC_LEDC_TIMER_BIT_WIDTH` check, **logging nothing** — the serial port keeps printing, the servo never moves, and it looks exactly like a wiring fault.

14 bits is enough: check 8-bit (what Day 10 and Day 19 used): 256 steps, each `20000/256 = 78 us`, while the whole 0°→180° span is only 2000 us — **a single 8-bit step equals 7°**, so the servo could only jump 0, 7, 14, 21… At 14 bits a step is 1.221 us, 1638 steps over the full range ≈ **0.11°/step**, finer than the servo's own mechanical backlash.

### API version trap and the three `parseInt` pitfalls

The old `ledcSetup(0,50,16)` + `ledcAttachPin` pair is gone; Core 3.x uses `ledcAttach(SERVO_PIN, 50, 14)` + `ledcWrite(SERVO_PIN, duty)`.

**Only the bool return values can be trusted for self-checks.** `ledcRead()` and `ledcReadFreq()` both lie: `ledcReadFreq()` back-computes frequency from the clock divider register and often returns 0 at 50 Hz; `ledcRead()` reads a shadow register that only syncs once per full PWM period (20 ms), so reading right after attach returns a stale value — measured 6 while the real duty was 410, with the servo turning fine.

Entering angles over serial runs into three `Serial.parseInt()` pitfalls:

1. **The default 1 s timeout returns 0, and 0 is a legal angle** — sending nothing still drives it to a limit. `setTimeout(10)` cuts the wait to 10 ms.
2. **The previous number sticks** — `\r\n` is not a digit and gets skipped, so last round's "180" lingers in the buffer and gets concatenated into the next parse. Drain the line with `while (Serial.available()) Serial.read();` after parsing.
3. **It stops at the first number** — `abc` returns 0 (coincidentally legal), `12abc` returns 12. So a range check on the parse result alone is not enough.

### Pre-run checklist

| Symptom | Verdict |
|---|---|
| A "thunk" jump to some angle at power-up | Normal. Between power-up and the first `ledcWrite` there are no pulses, so the servo freewheels |
| Holding still, resisting your fingers | Normal. An analog servo **holds its last position** without a command; only power loss releases it |
| No heating after sitting at a fixed angle | Normal. Holding needs very little current |
| **Pin it hard and it heats up fast / burns you** | ❌ **the most common way to kill a servo**. Blocked-rotor current above 700 mA all turns into heat |
| Serial port fine, servo completely dead | Prime suspect is no shared ground, second is the red wire not on 5 V |
| Jitter, buzzing, angle never reached | Underpowered (USB 500 mA sagging) or the red wire is on 3V3 |

**An SG90 cannot tell you its real angle**: it is an analog servo and the internal potentiometer signal is never brought out. So, as on Day 19, this is open loop — the code knows "I sent 90°", not "is it really at 90°".

### On-hardware result

**Experiment 1**: serial prints `attach=OK` and duty `410 / 1229 / 2048`, matching the hand calculation; the `us` field back-computes to exactly 500 / 1500 / 2500 us. The video shows the servo sweeping between 0° and 180°.

**Experiment 2**: serial monitor at 115200 / Newline, entering `0` `180` `90` `160` `30` `0` echoes duty `410 / 2048 / 1229 / 1866 / 683` and `us` 500 / 2500 / 1500 / 2278 / 834 — every angle lands on the 14-bit formula, and the video shows the servo following each command.

When the resolution is wrong, `ledcAttach` returns false but logs nothing: the serial output keeps printing while the servo stays still, and the duty values come out at the wrong resolution — 90° is `1229` at 14 bit but `4915` at 16 bit, 180° is `2048` vs `8192`. Seeing 4915 / 8192 on the wire means the firmware on the board is not the current file.

### Build result

```
实验1-自动摆动：    Sketch uses 294486 bytes (22%) / Global variables 21596 bytes (6%)
实验2-串口控制角度： Sketch uses 294494 bytes (22%) / Global variables 21588 bytes (6%)
```

Both pass (`--fqbn esp32:esp32:esp32s3`) with no new libraries. `ESP32Servo.h` is deliberately left out — it hides the 50 Hz, the 14-bit resolution and the duty conversion, which are precisely today's material.

---

## Day 21 — Robot Car Chassis & Ultrasonic Obstacle Avoidance

> Date: 2026-09-20
> Status: ✅ verified on hardware — every motion passes at 6 V
>
> Hardware: ESP32-S3 (N16R8) + DZQJ 2WD acrylic chassis (2 × TT motor 1:48 + wheels + caster) + TB6612FNG dual H-bridge + HC-SR04 + battery box (4 × AA = 6 V)
> Core: differential steering → dual H-bridge → motion functions → sense-decide-act loop
> Code: [`实验1-双电机差速.ino`](./day-21/实验1-双电机差速/实验1-双电机差速.ino) | [`实验2-超声波避障.ino`](./day-21/实验2-超声波避障/实验2-超声波避障.ino) | [`实验3-串口单步调试.ino`](./day-21/实验3-串口单步调试/实验3-串口单步调试.ino)
> Build photo: [`智能小车实物接线.png`](./day-21/智能小车实物接线.png) | [`超声波避障接线实物图.png`](./day-21/超声波避障接线实物图.png)
> Run log: Experiment 1 [`serial`](./day-21/实验1-双电机差速-串口打印.png) | [`video`](./day-21/实验1-双电机差速.MOV) | Experiment 2 [`serial`](./day-21/实验2-超声波避障-串口打印.txt) | [`video`](./day-21/实验2-超声波避障.MOV) | Experiment 3 [`log`](./day-21/实验3-单步调试-日志打印.txt) | [`video`](./day-21/实验3-串口单步调试.MOV)

Full notes: [`day-21/README.md`](./day-21/README.md)

### Goal

Scale Day 19's single motor up to a differential-drive chassis and add an HC-SR04 — the first complete **sense → decide → act** loop in this project.

### Differential steering: a two-wheeler has no steering gear

```
Forward    left ●▶▶▶▶  right ●▶▶▶▶   same speed, same direction → straight
Pivot L    left ◀◀◀●    right ●▶▶▶▶   equal speed, opposite → turns about its centre, radius ≈ 0
Arc L      left ●▶▶     right ●▶▶▶▶   same direction, different speed → arc about a distant centre
```

Pivoting is the most useful (zero turning radius — it can turn around in a dead end), at the cost of the two wheels fighting each other and drawing more current than straight running.

### Pin map: three peripherals on one board for the first time

| Function | GPIO | From |
|---|---|---|
| HC-SR04 Trig / Echo | 4 / 5 | Day 17 |
| MPU-6050 SDA / SCL | 8 / 9 | Day 18 |
| TB6612 AIN1 / AIN2 / PWMA (left) | 10 / 11 / 12 | Day 19 |
| TB6612 BIN1 / BIN2 / PWMB (right) | **15 / 16 / 17** | new on Day 21 |
| SG90 servo signal | 18 | Day 20 |

The servo and the MPU-6050 are **left off today**: a two-wheel + caster chassis needs no attitude sensing to drive, and the servo only comes aboard with the Day 27 pan-tilt. Wire only what you use.

### Wiring

```
TB6612FNG                    ESP32-S3 / supply
─────────                    ─────────────────
VCC    ──────  3V3 (logic)
GND    ──────  ESP32 GND and battery − (common ground!)
STBY   ──────  3V3 (held high)
VM     ──────  battery + (6 V; motor current never touches the board)
AIN1/AIN2/PWMA ─── GPIO10 / 11 / 12   left wheel
AO1/AO2 ──────  left motor ± (swapped leads only reverse it, nothing burns)
BIN1/BIN2/PWMB ─── GPIO15 / 16 / 17   right wheel
BO1/BO2 ──────  right motor ±

HC-SR04                      ESP32-S3
───────                      ────────
VCC    ──────  3V3
GND    ──────  GND (common ground)
Trig   ──────  GPIO4
Echo   ──────  GPIO5
```

Three things matter: **common ground** (battery −, ESP32 GND, TB6612 GND and HC-SR04 GND all tied; miss it and the motors do not move while ranging reads 0); **VM and VCC are separate supplies** (motor current comes from the battery, not the board); **the box holds 4 cells = 6 V**, for the reason in the next section.

### Where the duty comes from: 6 V supply, half duty

A motor coil is an inductor — current is smoothed, so the motor sees the **average voltage**, not the PWM peak:

```
average voltage = supply voltage × duty
3 V × 230/255 = 2.7 V
6 V × 115/255 = 2.7 V
```

Same average voltage means identical speed, torque and heating — the 6 V peak does not burn the motor. The real reason for 6 V is **internal resistance**: a TT motor draws its stall current at start-up, and two AAs have too much resistance relative to 3 V. Two motors starting together pull the rail down, so they buzz without turning, or one turns first and the other only after the current drops. The 6 V box can supply far more peak current and breaks static friction.

Hence 115 cruising, 120 pivoting, 75 for the inner wheel on an arc, plus a 200 ms kick at 150.

The two TT motors differ, so straight running drifts; the code keeps a `TRIM_RIGHT` hand-trim constant. It is an open-loop limitation — encoder feedback is the real fix.

### Two LEDC channels don't collide

The guide's `Motor` class hard-codes `_channel = 0`, so both instances fight over one channel. Core 3.x `ledcAttach(pin, freq, res)` allocates by **pin** and picks a free channel on its own:

```cpp
ledcAttach(PWMA, 20000, 8);
ledcAttach(PWMB, 20000, 8);
```

20 kHz rather than the guide's 5 kHz: above the audible range, so no PWM whine; shorter period means smaller current ripple and an average voltage closer to theory.

📌 **Don't read back with `ledcRead()`.** Under Core 3.x it always returns 0 — braking writes 255 yet prints `PWMB=0%`, which reads like "channel B is dead". Print the value you wrote.

### Experiment 1: differential drive

Eight motions cycle automatically, 6 s each, with the LED colour matching the current motion:

| Motion | left / right duty | LED |
|---|---|---|
| forward / reverse | +115 / +115 ｜ −115 / −115 | green / orange |
| pivot left / right | −120 / +120 ｜ +120 / −120 | cyan / magenta |
| arc left / right | +75 / +115 ｜ +115 / +75 | yellow |
| brake / coast | both shorted ｜ both unpowered | red / blue |

Serial output as captured:

```
09:31:19.549 -> 绿灯 | 前进  两轮都向前 | 左轮=前进115 右轮=前进115
09:31:43.563 -> 黄灯 | 左弧线  左轮75慢+右轮115快（弯向左） | 左轮=前进75 右轮=前进115
09:31:55.534 -> 红灯 | 刹车  两轮短接，立刻停住 | 两轮短接制动
```

All eight pass: pivoting really is equal and opposite (±120), and an arc is outer wheel at full speed with the inner one down to 75.

Brake vs coast reuses Day 19's truth table, applied to each motor: brake is `IN1=IN2=1` with **PWM non-zero**; coast is `IN1=IN2=0`, PWM 0. At PWM 0 the output is high-impedance regardless of IN.

### Experiment 2: ultrasonic avoidance

Adds the HC-SR04 on top of Experiment 1, closing the range → decide → act loop.

### Avoidance state machine

```
CRUISE ──cm < 15──▶ BACK ──500 ms──▶ TURN ──400 ms──▶ COMMIT ──300 ms──▶ CRUISE
```

`COMMIT` is not in the guide: measuring right after the turn can see the same wall again 40° later, and the car twitches against it. 300 ms of committed forward motion gets it clear first.

**The threshold needs hysteresis.** A single `dist < 15` oscillates at the boundary (reverse to 16, forward to 14, repeat). Enter at 15 cm, exit at 25 cm, with a 10 cm dead band holding the state.

**`pulseIn()` timeout cut to 6 ms.** Only the first 15 cm matter, so Day 17's 30 ms (400 cm range) is unnecessary. A timeout reads the same as "far away" — both mean go forward — but the worst-case stall drops from 30 ms to 6 ms.

**Turns alternate left and right.** `turnLeftNext` flips after every turn, so two obstacles in a row mean one left turn then one right turn. Seeing it turn left, then right, is by design: always turning the same way drives it deeper into a corner until it is stuck against the wall.

**A hand held in front means endless avoidance.** Hold a hand at 15 cm and the car reverses 0.5 s → turns 0.4 s → commits forward 0.3 s → measures → triggers again, about 1.2 s per round. That is the loop working honestly, not a freeze.

The serial output is plain readable lines rather than bare JSON: avoidance is a sequence, and `{"cm":12.3}` does not tell you which step it is on, why it turned, or which way. Each state change prints its reason, and cruising reports at most once per 500 ms. The firmware prints in Chinese; a sample line:

Captured with a hand held in front repeatedly:

```
前方 25.9cm，通畅 → 直行前进
第1轮｜前方 6.1cm < 15cm，挡住了 → 后退 500ms
第1轮｜原地左转 400ms（左轮后退 / 右轮前进，左右交替，下一轮换另一边）
第1轮｜转向已转开，强制前进 300ms 再重新测距（避免刚转开又看到障碍）
第2轮｜前方 14.8cm < 15cm，挡住了 → 后退 500ms
第2轮｜原地右转 400ms（左轮前进 / 右轮后退，左右交替，下一轮换另一边）
```

📌 **Logs should explain *why*, not just *what*.** What blocks debugging is never "what is the value" but "on what basis did it decide this". JSON is for Python to parse; the log a human watches is written separately.

Both error cases — "out of range (>100 cm)" and "reading invalid (<2 cm)" — are treated as clear ahead: both mean nothing is blocking, and they differ only in the log text, so a spurious sensor reading never makes the car reverse for no reason.

### Results

| Check | Result |
|---|---|
| Two-wheel drive at 6 V (Exp. 1) | ✅ forward / reverse / pivot / arc / brake / coast all pass |
| HC-SR04 alongside two motors | ✅ no interference, stable readings |
| 15 / 25 cm hysteresis | ✅ dead band holds, no twitching at the boundary |
| Alternating turn direction | ✅ L→R→L→R strictly alternating |
| Continuous avoidance (obstacle not removed) | ✅ keeps going round by round, never freezes |
| COMMIT 300 ms forward | ✅ actually clears the obstacle zone |
| Serial step commands `1`-`9`, `lf`…`rc` | ✅ all pass at 6 V |

### Experiment 3: serial step debugging

`1`–`8` each run 6 s then brake for 1 s, so every motion can be watched one at a time. Extras:

| Command | Effect |
|---|---|
| `9` | right wheel only: forward 3 s → reverse 3 s, left unpowered |
| `s` | duty sweep: both wheels 80→255, 1.5 s per step, finds the start threshold |
| `lf lb lz lc` | left wheel forward / reverse / brake / coast |
| `rf rb rz rc` | right wheel forward / reverse / brake / coast |
| `v` | switch 6 V / 3 V calibration |
| `0` | power off now |

The single-wheel commands keep the other channel unpowered, which separates "motor or wiring" from "supply": if every single-wheel command works but two wheels together don't, the motors and wiring are fine.

### Debug log: two wheels behaving "sometimes"

| Symptom | Real cause |
|---|---|
| Motor buzzes but does not turn | peak current too low to break static friction |
| One turns, then the other | the second only starts once the first's current drops |
| Works sometimes | battery voltage sits right at the threshold |
| Right wheel still spinning while braking | same cause, not a code bug |
| All 8 single-wheel commands fine | motors, wiring and code are all sound |

📌 **When symptoms look alike, find a control pair differing in one variable.** Here it was "one wheel vs two" — the same circuit differing only in current draw, which narrows the search straight to the supply.

### Build result

```
实验1-双电机差速：   Sketch uses 324683 bytes (24%) / Global variables 22252 bytes (6%)
实验2-超声波避障：   Sketch uses 324803 bytes (24%) / Global variables 22196 bytes (6%)
实验3-串口单步调试： Sketch uses 327547 bytes (24%) / Global variables 22172 bytes (6%)
```

Both pass (`--fqbn esp32:esp32:esp32s3`) with no new libraries.

### Final power: cutting the USB cord (MP1584EN buck module)

The ESP32 runs off USB right now, so the car is leashed. Once the buck module arrives, wire it like this and unplug:

```
6 V battery box
   ├────────────────────► TB6612 VM (motor power, straight 6 V, not bucked)
   │
   └──► MP1584EN IN ──► OUT 5 V ──► ESP32 "5V" pin
                                     │
   battery − ──┬── TB6612 GND ───────┴── ESP32 GND   (one common ground)
```

⏳ Waiting for the buck module (ordered 2026-09-22, in transit):

1. Measure the MP1584EN output with a multimeter and confirm it really is 5 V before wiring it up
2. Take two taps off the 6 V battery box: one into TB6612 VM, one into the buck module IN
3. Buck OUT 5 V → ESP32 "5V", negative → GND, **one common ground**
4. Keep USB plugged in until the voltages check out; the serial monitor dropping out after unplug is normal
5. Run the car on the floor, re-measure `TRIM_RIGHT` and the voltage sag

> ⚠️ **The motors must not run through the buck module.** Two TT motors starting together draw more stall current than the MP1584EN's 3 A; the module's over-current protection would hiccup — exactly the "supply can't deliver" trap from §三. So 6 V splits into **two parallel paths**: motors straight, only the ESP32 bucked.
>
> ⚠️ **The grounds must be common**, or the TB6612 PWM signals have no return path and the motors ignore every command.

> 📌 Due by Day 27-28 at the latest — those two days build the Wi-Fi-controlled car, which has to run untethered anyway.

### Why TRIM_RIGHT can't be calibrated yet

Calibration needs the car to **run free for at least 2 m**, but the USB cable both powers and tethers it: the cable drags, that drag dominates any drift measurement, and the trim value you'd get out of it is meaningless. Free running requires dropping USB, and dropping USB removes the serial port.

So the real question isn't "when is there time to calibrate" — it's **how data gets back to the computer once USB is gone**. The answer is **Wi-Fi** (Day 25): the ESP32-S3 has Wi-Fi built in, it joins the home router or makes its own AP, and Python reads it over a socket — no extra hardware. BLE needs `bleak` installed, a Bluetooth serial module means another purchase, and there's no SD card module on hand.

```
Day 24  MP1584EN arrives → car drops USB (free to run, but no data channel)
Day 25  Wi-Fi works      → data can reach the computer
Day 25+ calibration possible → send trim down, read the trajectory back
```

> 📌 Calibration **blocks nothing**: drift only makes straight running crooked; obstacle avoidance works fine.

> 📌 Knock-on effect: the "serial live plotting" in the Day 26 guide may well have no serial port left by then — once the car drops USB, live plotting has to read Wi-Fi too. The parsing and CSV-writing logic in the Day 22 `serial_logger.py` needs no change, only `serial.Serial()` swapped for a socket. **ADC2 (GPIO11-20) conflicts with Wi-Fi, so from Day 25 all analog reads go to ADC1 (GPIO1-10).**

---

## Day 22 — Python Refresher

> Date: 2026-09-22
> Status: ✅ all four scripts run — live serial capture pending
>
> Hardware: ESP32-S3 (N16R8) + HC-SR04 (reused from the Day 21 car) + potentiometer or photoresistor ×1
> Core: turn serial output from "for humans" into "for programs" → Python writes CSV / validates config / batch renames
> Code: [`实验1-串口遥测.ino`](./day-22/实验1-串口遥测/实验1-串口遥测.ino) | [`serial_logger.py`](./day-22/serial_logger.py) | [`log_to_csv.py`](./day-22/log_to_csv.py) | [`config_check.py`](./day-22/config_check.py) | [`batch_rename.py`](./day-22/batch_rename.py)
> Data: [`sensor_log.csv`](./day-22/sensor_log.csv) | [`sensor_log.png`](./day-22/sensor_log.png)

Full notes: [`day-22/README.md`](./day-22/README.md)

### Goal

Week 4 begins. Every serial line from the first 21 days was a Chinese log meant for human eyes; today a Python program has to read it — so the first job is **changing the output format**.

### Output format: Chinese log → JSON

```
old: 前方 25.9cm，通畅 → 直行前进
new: {"ms":12345,"dist_cm":25.9,"adc_raw":2048,"voltage":1.65}
```

The Chinese log is written for people. Python would need a regex to dig out `25.9`, the number arrives with no field name, and the regex breaks the moment the log wording changes. One JSON object per line turns into a dict with `json.loads()`, and adding a field needs no parser change.

> Not a spur-of-the-moment decision: Day 26 live plotting and Day 27-28 Wi-Fi telemetry both depend on the data having structure.

Decisions that aren't obvious:

| Decision | Why |
|---|---|
| Emit JSON only, no banner | Python filters noise by "does the line start with `{`?" — the ROM prints `ESP-ROM:esp32s3-...` on boot |
| Timeout returns `-1`, not `0` | `0` reads as "obstacle right in front" and plots as a fake spike down to zero |
| Sample at 10 Hz, not as fast as possible | Past 50 Hz characters start dropping and JSON parses half a line |

The analog input goes to **GPIO1 (ADC1_CH0)**, not GPIO11: ADC2 (GPIO11-20) shares hardware with Wi-Fi, so `analogRead()` fails while Wi-Fi is on. Day 27 runs Wi-Fi and battery monitoring together, so everything uses ADC1 from now on.

### Environment: PEP 668 and venv

`pip install pyserial` fails outright:

```
error: externally-managed-environment
note: ... You can override this, at the risk of breaking your Python installation,
      by passing --break-system-packages.
```

The Homebrew Python 3.14 is marked externally managed under PEP 668 — other programs may depend on the packages it ships with. `--break-system-packages` works, but it removes the guard entirely. The right fix:

```bash
/opt/homebrew/bin/python3 -m venv .venv --system-site-packages
./.venv/bin/python -m pip install pyserial
```

`--system-site-packages` lets the venv reuse the already-installed matplotlib 3.11.1 instead of downloading it again. `.gitignore` excludes `.venv/` — hundreds of files, trivially rebuilt on another machine.

### Script 1: serial → CSV

[`serial_logger.py`](./day-22/serial_logger.py) auto-detects `/dev/cu.usbmodem*`, parses one JSON object per line, writes `sensor_log.csv`.

```csv
timestamp,ms,dist_cm,adc_raw,voltage
2026-09-22T18:42:07.251,100,25.9,2048,1.65
2026-09-22T18:42:07.306,200,-1.0,2100,1.69
2026-09-22T18:42:07.413,400,30.8,1995,1.61
```

Verified with a fake serial port built from a pty (no need to unplug the board to test): feeding "ROM noise + good frame + truncated frame + good frame", the script skipped the noise and the broken frame and wrote exactly the 3 complete rows.

The car is busy running other code, so reflashing the telemetry sketch is inconvenient. [`log_to_csv.py`](./day-22/log_to_csv.py) instead parses the **serial log already saved on Day 21** into a CSV with identical fields. Source: [`实验2-超声波避障-串口打印.txt`](./day-21/实验2-超声波避障-串口打印.txt) — every number in it was genuinely measured on the car at the time, but it was not read live from serial today:

```
49 records, 14 with a valid distance / range 6.1 ~ 37.2 cm
timeout (-1) 26, invalid (-2) 9
```

Both kinds of distance-bearing lines must be matched ("clear" and "blocked"): matching only the first would **drop every sub-15 cm reading** — exactly the range that shows whether hysteresis is working. Timestamps are reconstructed as `seq × 500` (Day 21 sketch `LOG_MS=500`); `timestamp`, `adc_raw`, `voltage` were never printed back then, so they stay empty.

### Script 2: parse and validate config

[`config_check.py`](./day-22/config_check.py) + [`config.json`](./day-22/config.json) pull the Day 21 car parameters into a config file.

**`json.load()` guarantees valid syntax, not sane values** — `duty_cruise: 999` parses fine and only shows up once it reaches the motors. So validation has to happen after loading. Two checks that actually bite:

- `enter_cm >= exit_cm` → hysteresis inverted; the car "trigger → release → trigger" and shivers in place
- GPIO in 22-34 or 43/44 → not broken out on this board (26-32 Flash, 33/34 PSRAM) or used by USB; still compiles, then does nothing on hardware

By design it **collects every problem and returns them at once** — configs are rarely wrong in only one place.

### Script 3: batch rename

[`batch_rename.py`](./day-22/batch_rename.py), `--dry-run` by default.

The first version kept ASCII only, and the dry run showed `实验1-串口遥测.ino` turning into `1.ino` — the Chinese was stripped entirely. **Wrong design**: every asset in this repo is named in Chinese, so stripping it destroys all the information; converting to pinyin needs `pypinyin`, which isn't worth it.

What the script should actually fix is **spaces, brackets, full-width digits, repeated separators** — the characters that break in shells and URLs. So it now keeps Chinese and only does NFKC full-width folding plus separator normalisation. Two more guards: dry-run by default, and skip on name collision rather than overwrite.

### What Went Wrong

| Symptom | Cause | Fix |
|---|---|---|
| `pip install` says externally-managed-environment | PEP 668 protects the Homebrew Python | Use a venv, not `--break-system-packages` |
| `import matplotlib` fails inside venv | Default venv isolates system packages | `venv --system-site-packages` to reuse them |
| Chinese filename renamed to `1.ino` | Script stripped all non-ASCII | Keep Chinese, only normalise separators and full-width |
| Blank line between CSV rows | Python's newline translation | `open(..., newline="")` |
| Ctrl+C doesn't quit the program | `readline()` blocks with no timeout | `serial.Serial(..., timeout=1)` |
| Timeouts plot as a spike to zero | Using 0 for "no reading" | Use -1 instead |
| Short-range readings all missing when parsing the log | Only the "clear" line variant was matched | Match both distance-bearing variants, or every sub-15 cm sample is lost |

### Build & run

```
实验1-串口遥测：Sketch uses 328152 bytes (25%) / Global variables 22320 bytes (6%)
```

✅ Passes (`--fqbn esp32:esp32:esp32s3`). All four scripts exit 0.

⏳ **Live serial** capture pending: flash the telemetry sketch, then `./.venv/bin/python day-22/serial_logger.py --n 300`. Until then [`sensor_log.csv`](./day-22/sensor_log.csv) comes from `log_to_csv.py` parsing the Day 21 saved log — the numbers are real, but they were not read from serial today.

> 📌 **The serial path has a window**: it works only while the car is still on USB. Once the car drops USB on Day 24 and runs freely, there is no serial port — Day 26 live plotting will have to read Wi-Fi instead. The parsing and CSV-writing logic carries over unchanged; only `serial.Serial()` becomes a socket.

### Next Steps

- **Day 23**: Git version control (Learn Git Branching + repos for earlier weeks + `.gitignore`)

---
## Learning Journal Policy

Every day's work lives in a dated folder and includes:

1. **What I built** (photos, GIFs, schematics)
2. **Code** (well-commented, compiles/runs without errors)
3. **Test results** (measured values, oscilloscope captures, serial logs)
4. **What went wrong & how I fixed it** — the most important section
5. **References** (datasheets, tutorial links, forum threads that helped)

> 📌 Test for whether a line belongs here: **is it about what I learned, or about how this document was edited?** Only the former does.

---

## Hardware

See [`教程/第1月-电子学与工作台.md`](./教程/第1月-电子学与工作台.md) and [`进度/第1月-30天逐日指南.md`](./进度/第1月-30天逐日指南.md) for full BOM and budget tiers.
- [`元器件库存清单.md`](./元器件库存清单.md) — parts on hand + shopping list

> 📷 **Can't identify a component by name?** Open [`docs/元器件.jpg`](./docs/元器件.jpg) (breadboard-kit contents poster) and match it by appearance.

---

## Resources

- Falstad Circuit Simulator: https://www.falstad.com/circuit/
- Tinkercad Circuits: https://www.tinkercad.com/circuits
- All About Circuits Textbook: https://www.allaboutcircuits.com/textbook/
- ESP32-S3 Docs: https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/

---

## License

This learning journal is personal and for educational purposes. Third-party resources belong to their respective owners.
