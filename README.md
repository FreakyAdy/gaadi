# 🏎️ ESP32 4WD Bluetooth EV Car with Dual L298N Motor Drivers

Complete guide, wiring diagrams, mobile app setup, and firmware for the **4-Wheel Drive (4WD) Smart Robot Car** controlled via **Serial Bluetooth Terminal** on your Android phone.

---

## 📋 Table of Contents
1. [System Overview & Hardware Components](#1-system-overview--hardware-components)
2. [Why Two L298N Drivers?](#2-why-two-l298n-drivers)
3. [Full Circuit Wiring Diagram & Pinout Tables](#3-full-circuit-wiring-diagram--pinout-tables)
   - [A. ESP32 to Motor Drivers (Control Signals)](#a-esp32-to-motor-drivers-control-signals)
   - [B. Motor Driver to TT Motors](#b-motor-driver-to-tt-motors)
   - [C. Power & Common Ground (Crucial!)](#c-power--common-ground-crucial)
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

## 3. Full Circuit Wiring Diagram & Pinout Tables

### A. ESP32 to Motor Drivers (Control Signals)

The ESP32 pins are routed via your breadboard so that control signals are shared cleanly:

| ESP32 Pin (DevKit V1) | Label on Board | Driver 1 (Left Driver) | Driver 2 (Right Driver) | Function |
| :--- | :--- | :--- | :--- | :--- |
| **GPIO 14** | `D14` | **ENA** *(jumper off)* | **ENA** *(jumper off)* | Left Side Speed (PWM) |
| **GPIO 27** | `D27` | **IN1** | — | Left Side Forward |
| **GPIO 26** | `D26` | **IN2** | — | Left Side Backward |
| **GPIO 25** | `D25` | — | **IN3** | Right Side Forward |
| **GPIO 33** | `D33` | — | **IN4** | Right Side Backward |
| **GPIO 32** | `D32` | — | **ENB** *(jumper off)* | Right Side Speed (PWM) |

> **Note on Wiring Both Channels on Each Driver:**
> Since each L298N has two motor channels (Channel A: OUT1/OUT2, Channel B: OUT3/OUT4):
> - On **Driver 1 (Left Side)**:
>   - Connect ESP32 `D27` to both **IN1** and **IN3**.
>   - Connect ESP32 `D26` to both **IN2** and **IN4**.
>   - Connect ESP32 `D14` to both **ENA** and **ENB** (or bridge them).
> - On **Driver 2 (Right Side)**:
>   - Connect ESP32 `D25` to both **IN1** and **IN3**.
>   - Connect ESP32 `D33` to both **IN2** and **IN4**.
>   - Connect ESP32 `D32` to both **ENA** and **ENB** (or bridge them).

---

### B. Motor Driver to TT Motors

#### Driver 1 (Left Side Motors):
- **OUT1 & OUT2** (Screw Terminal 1) ➔ **Front-Left Motor** (Red & Black wires)
- **OUT3 & OUT4** (Screw Terminal 2) ➔ **Rear-Left Motor** (Red & Black wires)

#### Driver 2 (Right Side Motors):
- **OUT1 & OUT2** (Screw Terminal 1) ➔ **Front-Right Motor** (Red & Black wires)
- **OUT3 & OUT4** (Screw Terminal 2) ➔ **Rear-Right Motor** (Red & Black wires)

---

### C. Power & Common Ground (Crucial!)

> ⚠️ **CRITICAL: COMMON GROUND**  
> You **MUST** connect the **GND** of your battery pack, the **GND** of Driver 1, the **GND** of Driver 2, and the **GND** of the ESP32 together! If grounds are not connected together, the logic signals will float and motors will behave erratically or refuse to move.

```
       [ BATTERY PACK (7.4V - 12V) ]
         (+) Positive       (-) Negative (GND)
             │                     │
   ┌─────────┴─────────┐           ├───────────────────────────────┐
   │                   │           │                               │
   ▼                   ▼           ▼                               ▼
Driver 1 (12V)    Driver 2 (12V)  Driver 1 (GND) ── Driver 2 (GND) ── ESP32 (GND)
   │                   │
   └──────[ 5V ]───────┘ (Regulated 5V from Driver 1 with 5V Jumper ON)
             │
             ▼
        ESP32 (VIN)   <--- Power ESP32 from Driver's 5V out OR use USB Power Bank
```

1. **Battery (+) Positive wire**:
   - Goes into Driver 1 `12V` (or `VMS`) terminal AND Driver 2 `12V` terminal.
2. **Battery (-) Negative wire**:
   - Goes into Driver 1 `GND` terminal, Driver 2 `GND` terminal, and the **ESP32 GND pin** on your breadboard.
3. **Powering the ESP32**:
   - **Method 1 (Recommended for testing)**: Power the ESP32 via a standard USB cable/power bank to its Micro-USB port. (Remember: keep battery GND tied to ESP32 GND!).
   - **Method 2 (Standalone)**: With the black 5V regulator jumper in place on Driver 1, connect the **5V terminal** of Driver 1 to the **VIN pin** of the ESP32.

---

### D. ENA & ENB Jumpers Explained

Look closely at the 6 pins (`ENA`, `IN1`, `IN2`, `IN3`, `IN4`, `ENB`) on each L298N board:
- By default, they come with a small **black plastic jumper cap** bridging `ENA` to 5V and `ENB` to 5V.
- **If the jumpers are KEPT ON**: The motors run at 100% full speed all the time. You only need to connect `IN1`, `IN2`, `IN3`, `IN4`. In the code, set `const bool ENABLE_SPEED_CONTROL = false;`.
- **If the jumpers are REMOVED**: You can connect `ENA` to ESP32 `D14` and `ENB` to ESP32 `D32`. This allows variable speed control (0-255 PWM) and the speed buttons (0-9) will adjust speed in real time!

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
