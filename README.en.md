# Robotics Engineer Learning Tutorial

A hands-on, month-by-month robotics engineering curriculum. Starting from zero electronics experience, progressing to ROS2, control theory, and job-ready robotics skills in 6 months.

## What's Inside

| Folder | Description |
|--------|-------------|
| [`教程/`](./教程/) | Original 1–6 month article-style learning plans |
| [`进度/`](./进度/) | Day-by-day practical extension of Month 1 (30 days) + terminology glossary |
| [`docs/`](./docs/) | ESP32-S3 board source material (schematic + pinout) + component photos ([`元器件.jpg`](./docs/元器件.jpg)) + `小车模块分工表.md` (role of each Day 17–21 module in the finished robot) |
| [`day-01/`](./day-01/) … [`day-17/`](./day-17/) | Daily work (screenshots, circuit files, code, notes) |

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
- **Week 3** — Soldering, HC-SR04 ultrasonic, MPU-6050 IMU, DC motors + TB6612 driver
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

**Still to do**: fix the linear `map()` changing far too fast at the low end (**gamma correction**); and take a **fixed-camera** brightness comparison (the current three are handheld, and auto-exposure inverted the ordering).

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
| Outstanding | ⏳ Good-vs-bad (cold joint reference) photo |

> 📌 General methodology: **any continuity test must first ask "was it already connected?"** Without a control measurement, a beep proves nothing. Same approach applies to Day 16 desoldering and Day 21 wiring troubleshooting.

### Bonus Exercise: Deliberately Solder One Cold Joint

"Shiny" vs "dull and grainy" cannot be imagined from words. How to make a bad one: press the iron on for **only 1 second** (the workpiece never reaches temperature), or **wiggle the wire while it freezes** → the solder sits on the surface as a grey, granular, tofu-dregs mess. Shoot the good and the bad side by side for the clearest comparison.

### Deviations from the Guide

| Guide says | This README | Why |
|------------|-------------|-----|
| Practice on perfboard | Use a **scrap PCB** instead | 3 perfboards are in the mail; what you're practicing is solder volume and timing, so scrap is equivalent |
| Solder two DuPont **terminals** | Solder a **DuPont-to-DuPont splice** | No crimp terminals on hand; the splice directly produces the extension leads Day 21 needs |
| Good-vs-bad comparison photo | Deferred | You need to know what good and bad look like before you can deliberately make a bad one — makes more sense at the end of the practice |

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

Three acceptance criteria: ① no cold joints ✅ passed ② no shorts (>1MΩ between adjacent pads) ⏳ pending ③ neat joints ⚠️ flawed.

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

### Still To Do

- ⏳ **Touch-up solder**: improve joint shape (too little solder + it climbed the lead)
- ⏳ **Touch-up solder** (optional): too little solder + it climbed the lead → build it back into a "mound"
- 📌 **Desoldering**: technique recorded in `day-16/README.md` §8 — **no separate practice session**, just follow it when a real job comes up (wrong hole, reclaiming parts, Day 21 rework)
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

> Date: 2026-09-18 (wiring revised 2026-09-19)
> Status: ✅ **Complete** (2026-09-19): all five LED tiers hit, open-air control passed, four points off by −0.1 to −1.0 cm; VCC confirmed on the 3V3 header
>
> Hardware: ESP32-S3 (N16R8) + HC-SR04 ultrasonic module (wide-voltage 3–5.5 V version)
> Core: timing-based communication → Trig trigger / Echo pulse → speed-of-sound conversion → timeout & range checks
> Code: [`实验1-超声波测距.ino`](./day-17/实验1-超声波测距/实验1-超声波测距.ino)
>
> ⚠️ **2026-09-19 wiring revision**: VCC changed from the previously written "5 V" to **3V3** (see the wiring section below)

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
✅ **The 3V3 supply choice holds**: 173 cm still reads reliably, so there is no need to fall back to the "5 V + divider" plan (VCC confirmed on the 3V3 header). <br>
⚠️ **Readings skew low systematically** — all four points are negative (−0.1 to −1.0 cm), so this is not jitter. Main cause: room-temperature sound speed (~346 m/s) exceeds the 343 m/s used in the code, about −0.87% in theory; an offset in the measurement origin contributes too. Day 21 only asks "is something there?", so a sub-1 cm bias is irrelevant — **recorded, not compensated**. <br>
⏳ **Pending**: spread over 10 consecutive reads, the 30 cm boundary row, and a 3–4 m point.

### Deviations from the Guide

| Guide says | This README | Why |
|------------|-------------|-----|
| Trig→GPIO5, Echo→GPIO18 | **GPIO4 / GPIO5** instead | Adjacent on the header; keeps clear of Day 18 I2C |
| "Level shifting required" / VCC→5 V | **VCC→3V3, no divider** | 5Vin is an input with no drive capability (Day 14 trap); Echo at 3.3 V is directly safe |
| Output: measurements + timing diagram | ✅ Data filled in, diagram drawn | See the table above and [`day-17/时序图.png`](./day-17/时序图.png) |

**Timing diagram**: [`day-17/时序图.png`](./day-17/时序图.png)

### Next Steps

- ⏳ Ten consecutive reads at one distance; record the spread (expect ±0.3 cm)
- ⏳ The 30 cm boundary row; a 3–4 m point (needed to separate the temperature term from the origin offset)
- **Day 18**: MPU-6050 IMU (I2C) — ⚠️ install Adafruit MPU6050 + BusIO + Unified Sensor libraries first; module arrived 2026-09-19

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
