# 🏎️ ESP32 4WD Bluetooth EV Car with Dual L298N Motor Drivers

Complete guide, wiring diagrams, mobile app setup, and firmware for the **4-Wheel Drive (4WD) Smart Robot Car** controlled via **Serial Bluetooth Terminal** on your Android phone.

---

## 📋 Table of Contents
1. [System Overview & Hardware Components](#1-system-overview--hardware-components)
2. [Why Two L298N Drivers?](#2-why-two-l298n-drivers)
3. [Full Circuit Wiring Diagram & Breadboard Distribution](#3-full-circuit-wiring-diagram--breadboard-distribution)
   - [Breadboard Top-Down Layout Map](#how-the-breadboard-works-as-a-distribution-hub)
   - [A. Breadboard Row-by-Row Connection Table](#a-breadboard-row-by-row-connection-table)
   - [B. Motor Driver Screw Terminals to TT Motors](#b-motor-driver-screw-terminals-to-wheels)
   - [C. Power & Common Ground Distribution](#c-power--common-ground-distribution-eliminating-twisted-wires)
   - [D. ENA & ENB Jumpers Explained](#d-ena--enb-jumpers-explained)
4. [Arduino IDE Installation & Flashing](#4-arduino-ide-installation--flashing)
5. [Serial Bluetooth Terminal App Setup](#5-serial-bluetooth-terminal-app-setup)
   - [Pairing with Android](#pairing-with-android)
   - [Configuring On-Screen Control Buttons](#configuring-on-screen-control-buttons)
6. [Supported Bluetooth Commands](#6-supported-bluetooth-commands)
7. [Troubleshooting Guide (Brownouts, Reverse Spin, Boot Issues)](#7-troubleshooting-guide)

---

## 1. System Overview & Hardware Components

As shown in your project photos:
- **Microcontroller**: ESP32 DevKit V1 (30-Pin NodeMCU, ESP-WROOM-32 with Classic Bluetooth SPP).
- **Motor Drivers**: 2x L298N Dual H-Bridge modules (HW-095 with red PCB & heatsink).
- **Motors**: 4x Yellow TT DC Gear Motors (front-left, rear-left, front-right, rear-right).
- **Chassis**: 4WD clear acrylic dual-plate chassis with 4 rubber wheels.
- **Power**: Battery pack (4x AA holder or 2x 18650 Li-ion batteries recommended).
- **Breadboard & Jumpers**: Mini breadboard for signal and ground distribution.

---

## 2. Why Two L298N Drivers?

A single L298N can supply up to ~2A peak (~1.2A continuous per channel). 
When running 4 DC gear motors under load or turning on high-friction surfaces (carpet/tile), 4 motors can pull 2A to 3.5A total:
- **Driver 1 (Left Driver)**: Dedicated to the **Front-Left** and **Rear-Left** motors.
- **Driver 2 (Right Driver)**: Dedicated to the **Front-Right** and **Rear-Right** motors.
*(Alternative configuration: Driver 1 for Front Axle, Driver 2 for Rear Axle).*

Using two drivers prevents thermal throttling, gives maximum torque to all 4 wheels, and keeps your motors running smoothly.

---

## 3. Full Circuit Wiring Diagram & Breadboard Distribution

### ⚠️ IMPORTANT SAFETY ALERT (Based on your latest photo)
> In your latest setup photo, there are **bare, exposed copper wires twisted together** on the red power lines and white wire. 
> - **Risk**: If exposed copper touches the acrylic chassis edges, motor terminals, or the ESP32 pins, it will cause an immediate **short circuit** that will burn the ESP32 chip or the L298N drivers!
> - **Solution**: **Do NOT twist bare wires.** Instead, use the **Breadboard Power Rails** or screw terminals directly as explained below.

---

### How the Breadboard Works as a Distribution Hub

In a 4WD car with two L298N drivers, the breadboard acts as a **central splitter**:
1. **Each vertical row (5 holes)** connects one ESP32 signal pin to **both** motor drivers at the same time.
2. **The long horizontal rails (`+` and `-`)** distribute Common Ground and Power cleanly without any messy twisted wires.

```
 BREADBOARD LAYOUT MAP (Top-Down View):
 ═══════════════════════════════════════════════════════════════════════════════════════════════
  [-] POWER RAIL (COMMON GND):   [Battery (-)]  [Driver 1 GND]  [Driver 2 GND]  [ESP32 GND]
  [+] POWER RAIL (BATTERY 12V):  [Battery (+)]  [Driver 1 12V]  [Driver 2 12V]  (Eliminates twisted wires!)
 ───────────────────────────────────────────────────────────────────────────────────────────────
  ROW 1:  [ESP32 D14 (PWM)]  ────>  [Driver 1 ENA]         ────>  [Driver 2 ENA]   (Left Speed)
  ROW 2:  [ESP32 D27 (DIR)]  ────>  [Driver 1 IN1]         ────>  [Driver 2 IN1]   (Left Forward)
  ROW 3:  [ESP32 D26 (DIR)]  ────>  [Driver 1 IN2]         ────>  [Driver 2 IN2]   (Left Backward)
  ROW 4:  [ESP32 D25 (DIR)]  ────>  [Driver 1 IN3]         ────>  [Driver 2 IN3]   (Right Forward)
  ROW 5:  [ESP32 D33 (DIR)]  ────>  [Driver 1 IN4]         ────>  [Driver 2 IN4]   (Right Backward)
  ROW 6:  [ESP32 D32 (PWM)]  ────>  [Driver 1 ENB]         ────>  [Driver 2 ENB]   (Right Speed)
 ═══════════════════════════════════════════════════════════════════════════════════════════════
```

---

### A. Breadboard Row-by-Row Connection Table

Plug jumper wires into the same breadboard row to bridge them together:

| Breadboard Row | Signal Name | ESP32 Pin | Driver 1 (Front/Left) | Driver 2 (Rear/Right) | Description |
| :---: | :---: | :---: | :---: | :---: | :--- |
| **Blue (-) Rail** | **COMMON GND** | `GND` | `GND` terminal | `GND` terminal | **Tied with Battery (-)** |
| **Red (+) Rail** | **BATTERY POWER** | — | `12V` terminal | `12V` terminal | **Tied with Battery (+)** |
| **Row 1** | **Left Speed** | `D14` (GPIO 14) | `ENA` | `ENA` | Left PWM Speed Control |
| **Row 2** | **Left Forward** | `D27` (GPIO 27) | `IN1` | `IN1` | Left Wheels Forward |
| **Row 3** | **Left Backward** | `D26` (GPIO 26) | `IN2` | `IN2` | Left Wheels Backward |
| **Row 4** | **Right Forward** | `D25` (GPIO 25) | `IN3` | `IN3` | Right Wheels Forward |
| **Row 5** | **Right Backward** | `D33` (GPIO 33) | `IN4` | `IN4` | Right Wheels Backward |
| **Row 6** | **Right Speed** | `D32` (GPIO 32) | `ENB` | `ENB` | Right PWM Speed Control |

> 💡 **Driver 1 & 2 Roles:**
> - **Driver 1**: Controls the **Front Motors** (Left Wheel on OUT1/OUT2, Right Wheel on OUT3/OUT4).
> - **Driver 2**: Controls the **Rear Motors** (Left Wheel on OUT1/OUT2, Right Wheel on OUT3/OUT4).
> - Because Row 2 (`D27`) feeds `IN1` on **both** drivers, Front-Left and Rear-Left always spin forward together!
> - Because Row 4 (`D25`) feeds `IN3` on **both** drivers, Front-Right and Rear-Right always spin forward together!

---

### B. Motor Driver Screw Terminals to Wheels

#### Driver 1 (Front Axle):
- **OUT1 & OUT2** (Screw Terminal Block 1) ➔ **Front-Left Motor** (Red & Black wires)
- **OUT3 & OUT4** (Screw Terminal Block 2) ➔ **Front-Right Motor** (Red & Black wires)

#### Driver 2 (Rear Axle):
- **OUT1 & OUT2** (Screw Terminal Block 1) ➔ **Rear-Left Motor** (Red & Black wires)
- **OUT3 & OUT4** (Screw Terminal Block 2) ➔ **Rear-Right Motor** (Red & Black wires)

---

### C. Power & Common Ground Distribution (Eliminating Twisted Wires)

Instead of twisting loose wires together by hand, use the breadboard side rails:

1. **Battery Negative (-) Wire** ➔ Plug into Breadboard **Blue Rail (`-`)**.
2. **From Blue Rail (`-`)**, run 3 jumper wires:
   - Jumper 1 ➔ Driver 1 `GND` terminal.
   - Jumper 2 ➔ Driver 2 `GND` terminal.
   - Jumper 3 ➔ ESP32 `GND` pin.
3. **Battery Positive (+) Wire** ➔ Plug into Breadboard **Red Rail (`+`)**.
4. **From Red Rail (`+`)**, run 2 jumper wires:
   - Jumper 1 ➔ Driver 1 `12V` (or `VMS`) terminal.
   - Jumper 2 ➔ Driver 2 `12V` (or `VMS`) terminal.
5. **Powering the ESP32**:
   - **Recommended**: Power the ESP32 via a USB cable connected to a 5V power bank or laptop.
   - **Standalone**: Take a wire from Driver 1's `5V` terminal (with the black 5V regulator jumper ON) and connect it to ESP32 **VIN** pin.

---

### D. ENA & ENB Jumpers Explained

Look at the 6-pin headers (`ENA`, `IN1`, `IN2`, `IN3`, `IN4`, `ENB`) on each L298N board:
- **If you keep the black jumper caps ON ENA & ENB**:
  - The motors run at 100% full speed all the time.
  - You do not need Row 1 (`D14`) or Row 6 (`D32`).
  - In `gaadi.ino`, set: `const bool ENABLE_SPEED_CONTROL = false;`
- **If you REMOVE the black jumper caps**:
  - Connect Row 1 (`D14`) to both `ENA` pins and Row 6 (`D32`) to both `ENB` pins.
  - You can now change speed dynamically between 10% and 100% directly from the phone app!

---

## 4. Arduino IDE Installation & Flashing

### Step 1: Install ESP32 Board Core
1. Open **Arduino IDE** (version 2.x recommended).
2. Go to **File ➔ Preferences**.
3. In **Additional Boards Manager URLs**, paste:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```
4. Go to **Tools ➔ Board ➔ Boards Manager...**, search for `esp32` by **Espressif Systems**, and click **Install**.

### Step 2: Open and Configure Sketch
1. Open `ESP32_Bluetooth_4WD_Car.ino` in Arduino IDE.
2. In **Tools ➔ Board ➔ esp32**, select **"ESP32 Dev Module"** (or **"DOIT ESP32 DEVKIT V1"**).
3. Connect your ESP32 to PC with a high-quality Micro-USB data cable.
4. In **Tools ➔ Port**, select the COM port of your ESP32 (e.g., `COM3`, `COM4`, etc.).
5. Set **Upload Speed** to `921600` (or `115200` if upload fails).

### Step 3: Flash the Code
1. Click the **Upload (➔)** button.
2. If the console shows `Connecting........_____.....`:
   - Press and hold the **BOOT** button on the ESP32 board until the flashing percentage starts (0%... 10%...).
3. Once finished, open **Tools ➔ Serial Monitor** at baud rate **115200**.
4. You will see:
   ```text
   ==============================================
      ESP32 4WD Dual L298N Car Initializing...   
   ==============================================
   [+] Bluetooth Serial ready as: "ESP32_4WD_EV_Car"
   [+] Open 'Serial Bluetooth Terminal' on Android and connect.
   ==============================================
   ```

---

## 5. Serial Bluetooth Terminal App Setup

### Pairing with Android
1. Open your phone's **Settings ➔ Bluetooth**.
2. Turn on Bluetooth and scan for devices.
3. Look for **`ESP32_4WD_EV_Car`** and tap **Pair**. (No PIN is required; if prompted, enter `1234` or `0000`).

### Using "Serial Bluetooth Terminal" (by Kai Morich)
1. Download **Serial Bluetooth Terminal** from the [Google Play Store](https://play.google.com/store/apps/details?id=de.kai_morich.serial_bluetooth_terminal).
2. Open the app, tap the **Menu (☰)** on the top-left ➔ **Devices**.
3. Under the **Bluetooth Classic** tab, select **`ESP32_4WD_EV_Car`**.
4. Tap the **Connect icon** (plug icon) on the top bar.
5. You will see:
   ```text
   Connected
   ```
6. You can now type any command into the terminal bar (e.g. `F` and send) or customize buttons!

---

### Configuring On-Screen Control Buttons (Macro Buttons M1 - M6)

At the bottom of the app, there is a row of custom buttons (`M1`, `M2`, `M3`, `M4`, `M5`, `M6`):

1. **Press and hold** any button (e.g. `M1`) to edit it:
   - **Name**: `▲ FWD`
   - **Value**: `F`
2. Configure the rest:
   - **M2**: Name: `▼ BACK` | Value: `B`
   - **M3**: Name: `◀ LEFT` | Value: `L`
   - **M4**: Name: `▶ RIGHT` | Value: `R`
   - **M5**: Name: `⏹ STOP` | Value: `S`
   - **M6**: Name: `⚡ SPEED` | Value: `9`
3. Swipe right on the button row to access a second bank of buttons (`M7` - `M12`):
   - **M7**: Name: `Slow` | Value: `2`
   - **M8**: Name: `Med` | Value: `5`
   - **M9**: Name: `Fast` | Value: `9`
   - **M10**: Name: `Info ?` | Value: `?`

> 💡 **Tip:** In app settings under **Send**, you can set **Newline** to `None` or `LF`. Our code handles both automatically!

---

## 6. Supported Bluetooth Commands

| Command | Action | Behavior |
| :---: | :--- | :--- |
| **`F`** or **`w`** | Move Forward | All 4 wheels spin forward |
| **`B`** or **`s`** | Move Backward | All 4 wheels spin backward |
| **`L`** or **`a`** | Spin Turn Left | Left wheels reverse, Right wheels forward |
| **`R`** or **`d`** | Spin Turn Right | Left wheels forward, Right wheels reverse |
| **`S`** or **`x`** or `Space` | Stop Motors | All motors brake/coast to stop |
| **`G`** | Forward-Left | Gentle curving forward turn |
| **`I`** | Forward-Right | Gentle curving forward turn |
| **`H`** | Back-Left | Gentle curving backward turn |
| **`J`** | Back-Right | Gentle curving backward turn |
| **`0`** to **`9`** | Speed Selection | Sets speed from 10% (0) to 100% (9) |
| **`q`** | Max Speed | Sets speed to 100% (255) |
| **`?`** | Help Menu | Prints quick command list back to app screen |
| **`#`** | Status Report | Displays current movement state and speed |

---

## 7. Troubleshooting Guide

### 1. The ESP32 restarts / disconnects Bluetooth whenever a motor moves (Brownout Reset)
- **Root Cause**: 4x standard AA alkaline batteries (1.5V x 4 = 6V) drop drastically under the starting current spike of 4 DC motors. When voltage dips below 4.5V, the ESP32 triggers a Brownout Reset.
- **Fix**:
  - Use **two 18650 3.7V rechargeable Li-ion batteries** in series (7.4V - 8.4V).
  - OR power the ESP32 via a power bank plugged into its Micro-USB port, while the battery pack exclusively powers the L298N `12V` pin (remember to keep GND connected!).

### 2. A motor or one whole side spins backwards when moving Forward (`F`)
- You do **NOT** need to unsolder any wires!
- **Software Fix**: In `ESP32_Bluetooth_4WD_Car.ino`, change:
  ```cpp
  const bool INVERT_LEFT_MOTORS  = true;  // or INVERT_RIGHT_MOTORS = true;
  ```
- **Hardware Fix**: Simply swap the two wires in the L298N green/blue terminal block for that motor.

### 3. ESP32 Bluetooth doesn't show up on Phone
- Make sure you are using an original **ESP32 (ESP-WROOM-32)** (which is what is shown in your photo). ESP32-S2/S3/C3 chips do not have classic Bluetooth SPP.
- Check the Arduino Serial Monitor at 115200 baud to ensure `BluetoothSerial ready` was printed.
- Verify the status LED on GPIO 2 is illuminated.

### 4. Upload Error: `Failed to connect to ESP32: Timed out waiting for packet header`
- When you click Upload and see `Connecting........_____`, press and hold the **BOOT** button on your ESP32 for 2 seconds, then release it.
