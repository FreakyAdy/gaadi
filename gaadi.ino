/*
 * =====================================================================================
 *  Project: ESP32 4WD Bluetooth Car with Dual L298N Motor Drivers
 *  Hardware:
 *    - ESP32 DevKit V1 (30-Pin NodeMCU)
 *    - 2x L298N Dual H-Bridge Motor Drivers (HW-095)
 *    - 4x TT Yellow DC Gear Motors (4WD Chassis)
 *    - Mobile App: "Serial Bluetooth Terminal" (or any Bluetooth RC Car App)
 * =====================================================================================
 */

#include <Arduino.h>
#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

// =====================================================================================
//  CONFIGURATION SETTINGS
// =====================================================================================

// Bluetooth Device Name shown during pairing
const char* DEVICE_NAME = "ESP32_4WD_EV_Car";

// Direction Inversion Flags:
// If any side drives backwards when moving forward, change false to true here!
const bool INVERT_LEFT_MOTORS  = false;
const bool INVERT_RIGHT_MOTORS = false;

// Set to true if you removed the black jumpers on ENA & ENB and connected PWM wires
// Set to false if you kept the black jumpers on ENA & ENB (runs at 100% full speed)
const bool ENABLE_SPEED_CONTROL = true;

// Optional Fail-Safe: Stop car if no command received for 3 seconds (3000 ms)
// Set to 0 to disable fail-safe (car keeps moving until 'S' is sent)
const unsigned long FAILSAFE_TIMEOUT_MS = 0; 

// =====================================================================================
//  PIN DEFINITIONS (ESP32 DevKit V1 30-Pin)
//  These pins are safe general-purpose output pins located consecutively on the board.
// =====================================================================================

// Left Side Motors (Front-Left & Rear-Left)
const int PIN_IN1 = 27;  // Direction Pin 1
const int PIN_IN2 = 26;  // Direction Pin 2
const int PIN_ENA = 14;  // Speed PWM Pin (Channel A)

// Right Side Motors (Front-Right & Rear-Right)
const int PIN_IN3 = 25;  // Direction Pin 3
const int PIN_IN4 = 33;  // Direction Pin 4
const int PIN_ENB = 32;  // Speed PWM Pin (Channel B)

// Optional On-board LED indicator (GPIO 2 on ESP32 DevKit)
const int PIN_STATUS_LED = 2;

// =====================================================================================
//  PWM CONFIGURATION (Universal ESP32 Core 2.x and 3.x Support)
// =====================================================================================
const int PWM_FREQ       = 1000; // 1kHz is optimal for TT DC motors
const int PWM_RESOLUTION = 8;    // 8-bit resolution: values 0 to 255
int currentSpeed         = 200;  // Initial speed (0 = Stop, 255 = Maximum)

#define PWM_CHANNEL_LEFT  0
#define PWM_CHANNEL_RIGHT 1

void setupPWM() {
  if (!ENABLE_SPEED_CONTROL) return;

#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
  // ESP32 Arduino Core v3.0+ API
  ledcAttach(PIN_ENA, PWM_FREQ, PWM_RESOLUTION);
  ledcAttach(PIN_ENB, PWM_FREQ, PWM_RESOLUTION);
#else
  // ESP32 Arduino Core v2.x API
  ledcSetup(PWM_CHANNEL_LEFT, PWM_FREQ, PWM_RESOLUTION);
  ledcSetup(PWM_CHANNEL_RIGHT, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(PIN_ENA, PWM_CHANNEL_LEFT);
  ledcAttachPin(PIN_ENB, PWM_CHANNEL_RIGHT);
#endif
}

void writePWM(int pin, int channel, int speedVal) {
  if (!ENABLE_SPEED_CONTROL) return;

#if defined(ESP_ARDUINO_VERSION_MAJOR) && (ESP_ARDUINO_VERSION_MAJOR >= 3)
  ledcWrite(pin, constrain(speedVal, 0, 255));
#else
  ledcWrite(channel, constrain(speedVal, 0, 255));
#endif
}

// =====================================================================================
//  STATE VARIABLES
// =====================================================================================
char currentMovement = 'S';
unsigned long lastCommandTime = 0;

// Forward declarations
void stopMotors();
void moveForward();
void moveBackward();
void turnLeft();
void turnRight();
void moveForwardLeft();
void moveForwardRight();
void moveBackLeft();
void moveBackRight();

// =====================================================================================
//  SETUP FUNCTION
// =====================================================================================
void setup() {
  // Initialize Serial Monitor (PC USB debugging)
  Serial.begin(115200);
  delay(500);
  Serial.println("\n==============================================");
  Serial.println("   ESP32 4WD Dual L298N Car Initializing...   ");
  Serial.println("==============================================");

  // Configure Motor Direction Pins as OUTPUT
  pinMode(PIN_IN1, OUTPUT);
  pinMode(PIN_IN2, OUTPUT);
  pinMode(PIN_IN3, OUTPUT);
  pinMode(PIN_IN4, OUTPUT);
  pinMode(PIN_STATUS_LED, OUTPUT);

  // Configure PWM for Speed Control
  setupPWM();

  // Stop motors initially
  stopMotors();

  // Start Bluetooth Serial SPP
  if (!SerialBT.begin(DEVICE_NAME)) {
    Serial.println("[-] Error: Bluetooth failed to initialize!");
    while (1) {
      digitalWrite(PIN_STATUS_LED, !digitalRead(PIN_STATUS_LED));
      delay(200);
    }
  }

  digitalWrite(PIN_STATUS_LED, HIGH);
  Serial.printf("[+] Bluetooth Serial ready as: \"%s\"\n", DEVICE_NAME);
  Serial.println("[+] Open 'Serial Bluetooth Terminal' on Android and connect.");
  Serial.println("==============================================\n");
}

// =====================================================================================
//  MOTOR CONTROL FUNCTIONS
// =====================================================================================

void applyMotorSignals(bool in1, bool in2, bool in3, bool in4, int speedLeft, int speedRight) {
  // Invert left/right logic if flags are set
  bool leftFwd  = INVERT_LEFT_MOTORS ? in2 : in1;
  bool leftRev  = INVERT_LEFT_MOTORS ? in1 : in2;
  bool rightFwd = INVERT_RIGHT_MOTORS ? in4 : in3;
  bool rightRev = INVERT_RIGHT_MOTORS ? in3 : in4;

  // Apply Direction signals
  digitalWrite(PIN_IN1, leftFwd ? HIGH : LOW);
  digitalWrite(PIN_IN2, leftRev ? HIGH : LOW);
  digitalWrite(PIN_IN3, rightFwd ? HIGH : LOW);
  digitalWrite(PIN_IN4, rightRev ? HIGH : LOW);

  // Apply Speed via PWM
  writePWM(PIN_ENA, PWM_CHANNEL_LEFT, speedLeft);
  writePWM(PIN_ENB, PWM_CHANNEL_RIGHT, speedRight);
}

void moveForward() {
  currentMovement = 'F';
  applyMotorSignals(HIGH, LOW, HIGH, LOW, currentSpeed, currentSpeed);
}

void moveBackward() {
  currentMovement = 'B';
  applyMotorSignals(LOW, HIGH, LOW, HIGH, currentSpeed, currentSpeed);
}

void turnLeft() {
  // Spin turn on the spot: Left wheels reverse, Right wheels forward
  currentMovement = 'L';
  applyMotorSignals(LOW, HIGH, HIGH, LOW, currentSpeed, currentSpeed);
}

void turnRight() {
  // Spin turn on the spot: Left wheels forward, Right wheels reverse
  currentMovement = 'R';
  applyMotorSignals(HIGH, LOW, LOW, HIGH, currentSpeed, currentSpeed);
}

void moveForwardLeft() {
  // Gentle left curve forward
  currentMovement = 'G';
  applyMotorSignals(HIGH, LOW, HIGH, LOW, currentSpeed / 2, currentSpeed);
}

void moveForwardRight() {
  // Gentle right curve forward
  currentMovement = 'I';
  applyMotorSignals(HIGH, LOW, HIGH, LOW, currentSpeed, currentSpeed / 2);
}

void moveBackLeft() {
  // Gentle left curve backward
  currentMovement = 'H';
  applyMotorSignals(LOW, HIGH, LOW, HIGH, currentSpeed / 2, currentSpeed);
}

void moveBackRight() {
  // Gentle right curve backward
  currentMovement = 'J';
  applyMotorSignals(LOW, HIGH, LOW, HIGH, currentSpeed, currentSpeed / 2);
}

void stopMotors() {
  currentMovement = 'S';
  applyMotorSignals(LOW, LOW, LOW, LOW, 0, 0);
}

// =====================================================================================
//  COMMAND PROCESSOR
// =====================================================================================

void sendFeedback(const char* msg) {
  Serial.println(msg);
  SerialBT.println(msg);
}

void printStatus() {
  String stat = ">> State: ";
  stat += currentMovement;
  stat += " | Speed: ";
  stat += currentSpeed;
  stat += " (";
  stat += String(map(currentSpeed, 0, 255, 0, 100));
  stat += "%)";
  sendFeedback(stat.c_str());
}

void printHelp() {
  SerialBT.println("\n--- ESP32 4WD COMMAND GUIDE ---");
  SerialBT.println(" Movement: F=Forward, B=Backward, L=Left, R=Right, S=Stop");
  SerialBT.println(" Curved:   G=Fwd-Left, I=Fwd-Right, H=Back-Left, J=Back-Right");
  SerialBT.println(" Speeds:   0=10%, 1=20% ... 9=100%, q=Full Speed");
  SerialBT.println(" Presets:  v1=Slow(120), v2=Med(190), v3=Max(255)");
  SerialBT.println(" Status:   ?=Help, #=Status Report");
  SerialBT.println("--------------------------------\n");
}

void processCommand(char cmd) {
  lastCommandTime = millis();

  switch (cmd) {
    // --- Basic Direction Commands (Case insensitive) ---
    case 'F':
    case 'f':
    case 'W':
    case 'w':
      moveForward();
      sendFeedback(">> Moving: FORWARD");
      break;

    case 'B':
    case 'b':
    case 'S':
    case 's':
    case 'X':
    case 'x':
      stopMotors();
      sendFeedback(">> Status: STOPPED");
      break;

    case 'L':
    case 'l':
    case 'A':
    case 'a':
      turnLeft();
      sendFeedback(">> Turning: LEFT");
      break;

    case 'R':
    case 'r':
    case 'D':
    case 'd':
      turnRight();
      sendFeedback(">> Turning: RIGHT");
      break;

    case ' ': // Spacebar = Stop
      stopMotors();
      sendFeedback(">> Status: STOPPED");
      break;

    // --- Diagonal / Curve Commands (standard in RC apps) ---
    case 'G':
    case 'g':
      moveForwardLeft();
      sendFeedback(">> Moving: FORWARD-LEFT");
      break;

    case 'I':
    case 'i':
      moveForwardRight();
      sendFeedback(">> Moving: FORWARD-RIGHT");
      break;

    case 'H':
    case 'h':
      moveBackLeft();
      sendFeedback(">> Moving: BACK-LEFT");
      break;

    case 'J':
    case 'j':
      moveBackRight();
      sendFeedback(">> Moving: BACK-RIGHT");
      break;

    // --- Speed Percentage Settings ('0' to '9', and 'q') ---
    case '0': currentSpeed = 70;  sendFeedback(">> Speed: 10% (70/255)"); break;
    case '1': currentSpeed = 90;  sendFeedback(">> Speed: 20% (90/255)"); break;
    case '2': currentSpeed = 110; sendFeedback(">> Speed: 30% (110/255)"); break;
    case '3': currentSpeed = 130; sendFeedback(">> Speed: 40% (130/255)"); break;
    case '4': currentSpeed = 150; sendFeedback(">> Speed: 50% (150/255)"); break;
    case '5': currentSpeed = 175; sendFeedback(">> Speed: 60% (175/255)"); break;
    case '6': currentSpeed = 195; sendFeedback(">> Speed: 70% (195/255)"); break;
    case '7': currentSpeed = 215; sendFeedback(">> Speed: 80% (215/255)"); break;
    case '8': currentSpeed = 235; sendFeedback(">> Speed: 90% (235/255)"); break;
    case '9':
    case 'q':
    case 'Q': currentSpeed = 255; sendFeedback(">> Speed: 100% (MAX 255)"); break;

    // --- Help & Status queries ---
    case '?':
      printHelp();
      break;

    case '#':
      printStatus();
      break;

    // Ignore carriage return, newline, or null terminators
    case '\r':
    case '\n':
    case '\0':
      break;

    default:
      Serial.print("[?] Unknown command received: ");
      Serial.println(cmd);
      break;
  }

  // Refresh speed if car is currently moving
  if (currentMovement == 'F') moveForward();
  else if (currentMovement == 'B') moveBackward();
  else if (currentMovement == 'L') turnLeft();
  else if (currentMovement == 'R') turnRight();
  else if (currentMovement == 'G') moveForwardLeft();
  else if (currentMovement == 'I') moveForwardRight();
  else if (currentMovement == 'H') moveBackLeft();
  else if (currentMovement == 'J') moveBackRight();
}

// =====================================================================================
//  MAIN LOOP
// =====================================================================================
void loop() {
  // Read incoming characters from mobile Bluetooth terminal
  while (SerialBT.available()) {
    char cmd = (char)SerialBT.read();
    processCommand(cmd);
  }

  // Also allow control from Arduino IDE USB Serial Monitor
  while (Serial.available()) {
    char cmd = (char)Serial.read();
    processCommand(cmd);
  }

  // Optional Fail-Safe: Stop car if Bluetooth disconnects or no commands received
  if (FAILSAFE_TIMEOUT_MS > 0 && currentMovement != 'S') {
    if (millis() - lastCommandTime > FAILSAFE_TIMEOUT_MS) {
      stopMotors();
      sendFeedback(">> [WARNING] Fail-Safe Activated: Car Stopped due to inactivity.");
    }
  }

  delay(5); // Small delay to yield to FreeRTOS watchdog
}
