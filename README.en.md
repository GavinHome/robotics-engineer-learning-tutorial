# Robotics Engineer Learning Tutorial

A hands-on, month-by-month robotics engineering curriculum. Starting from zero electronics experience, progressing to ROS2, control theory, and job-ready robotics skills in 6 months.

## What's Inside

| Folder | Description |
|--------|-------------|
| [`教程/`](./教程/) | Original 1–6 month article-style learning plans |
| [`进度/`](./进度/) | Day-by-day practical extension of Month 1 (30 days) + terminology glossary |
| [`docs/`](./docs/) | ESP32-S3 board source material (schematic + pinout) + component photos ([`元器件.jpg`](./docs/元器件.jpg)) |
| [`day-01/`](./day-01/) … [`day-10/`](./day-10/) | Daily work (screenshots, circuit files, code, notes) |

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
├── docs/                        ← ESP32-S3 board reference (schematic + pinout) + component photos (元器件.jpg)
├── 元器件库存清单.md            ← parts on hand + shopping list
├── day-01/ … day-07/            ← Week 1: circuit theory & simulation
├── day-08/                      ← Day 8: ESP32-S3 board & toolchain setup
├── day-09/                      ← Day 9: first program, Blink (onboard WS2812B + external LED)
└── day-10/                      ← Day 10: PWM breathing LED (duty-cycle brightness + two parallel tasks)
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
