# STM32 Fingerprint Access Control System

A standalone biometric access control system implemented in modular C++ for STM32 microcontrollers (such as STM32F103 Bluepill or STM32F411CE Blackpill). The firmware features C++ template-based EXTI interrupt binding, internal flash EEPROM emulation with direct-offset record mapping, an AS608 biometric sensor, LSE-backed internal hardware RTC timekeeping with, and an SSD1306 OLED graphical interface

---

## Table of Contents

1. [Project Description](#project-description)
2. [Hardware and Wiring](#hardware-and-wiring)
   - [Pin Mapping Table](#pin-mapping-table)
   - [Electrical Specifications and Wiring Notes](#electrical-specifications-and-wiring-notes)
   - [System Interconnect Diagram](#system-interconnect-diagram)
   - [Schematic Diagram](#schematic-diagram)
3. [User Manual](#user-manual)
   - [First-Boot Initialization and Master Setup](#first-boot-initialization-and-master-setup)
   - [Keypad Navigation](#keypad-navigation)
   - [Main Menu Structure](#main-menu-structure)
   - [Access Control Mode](#access-control-mode)
   - [User Enrollment Mode](#user-enrollment-mode)
   - [User Deletion Mode](#user-deletion-mode)
   - [Audit Log Mode](#audit-log-mode)
   - [Set Time Mode](#set-time-mode)
4. [Features and Code Walkthrough](#features-and-code-walkthrough)
   - [1. C++ Template-Based Interrupt Engine](#1-c-template-based-interrupt-engine)
   - [2. Flash EEPROM Memory Layout and Direct Offsets](#2-flash-eeprom-memory-layout-and-direct-offsets)
   - [3. Master Authorization Gate and Sole Master Protection](#3-master-authorization-gate-and-sole-master-protection)
   - [4. AS608 Biometric DSP State Machine](#4-as608-biometric-dsp-state-machine)
   - [5. Internal RTC Seeding and Manual Setting Engine](#5-internal-rtc-seeding-and-manual-setting-engine)
   - [6. EEPROM-Backed Circular Audit Logging System](#6-eeprom-backed-circular-audit-logging-system)
   - [7. Constrained OLED Text Input Editor](#7-constrained-oled-text-input-editor)
5. [Function Reference](#function-reference)
   - [Main Application Module (Fingerprint_Access_Control.ino)](#main-application-module-fingerprint_access_controlino)
   - [Biometrics Module (biometrics.h / biometrics.cpp)](#biometrics-module-biometricsh--biometricscpp)
   - [Button Interrupt Module (buttons.h / buttons.cpp)](#button-interrupt-module-buttonsh--buttonscpp)
   - [User Management Module (users.h / users.cpp)](#user-management-module-usersh--userscpp)
   - [Audit Logging Module (logger.h / logger.cpp)](#audit-logging-module-loggerh--loggercpp)
   - [Real-Time Clock Module (myTime.h / myTime.cpp)](#real-time-clock-module-mytimeh--mytimecpp)
   - [Display Engine Module (display.h / display.cpp)](#display-engine-module-displayh--displaycpp)

---

## Project Description

The STM32 Fingerprint Access Control System provides secure, standalone physical security management using optical fingerprint verification. The architecture is engineered specifically for embedded real-time microcontrollers without requiring an external real-time clock IC or external EEPROM chips.

### Core Capabilities
* **Biometric Verification**: Operates an AS608 optical fingerprint sensor over hardware USART at 57,600 baud, executing two-pass template extraction and fused model comparison.
* **Non-Blocking Interrupt Inputs**: Employs static C++ template wrappers to map hardware GPIO External Interrupts (EXTI) directly to C++ class instances with integrated delta-time software debouncing.
* **Direct-Offset EEPROM Emulation**: Manages user persistence in STM32 internal Flash using a packed binary structure (`UserRecord`) validated via an `0xA5` magic byte, enabling O(1) direct address indexing.
* **Sole Master Protection & Privilege Gate**: Prevents administrative lockouts by enforcing authorization checks for administrative actions while guarding the final registered Master user against deletion.
* **Internal Hardware Timekeeping**: Drives the hardware RTC peripheral via a 32.768 kHz Low-Speed External (LSE) crystal oscillator, auto-seeding timestamps from build macros (`__TIME__`, `__DATE__`) on cold boot, with full manual override UI.
* **Non-Volatile Audit Logging**: Stores a 20-entry circular access log buffer in flash EEPROM starting at memory address 3000, preserving time-stamped entry records across power cycles.

---

## Hardware and Wiring

### Pin Mapping Table

| Hardware Subsystem | Signal Name | STM32 Pin | Interface / Mode | Description |
| :--- | :--- | :--- | :--- | :--- |
| **AS608 Fingerprint DSP** | TX | `PA2` | Hardware USART1 RX | Receives serial response packets from sensor (57,600 Baud). |
| **AS608 Fingerprint DSP** | RX | `PA3` | Hardware USART1 TX | Sends serial command packets to sensor (57,600 Baud). |
| **SSD1306 OLED (128x64)** | SCL | `PB6` | Hardware I2C1 SCL | Clock line operating at 400 kHz Fast-Mode. |
| **SSD1306 OLED (128x64)** | SDA | `PB7` | Hardware I2C1 SDA | Data line (I2C Bus Address: `0x3C`). |
| **Nav Keypad (UP)** | Input | `PB12` | EXTI Interrupt | Configured with `INPUT_PULLUP`. Active LOW on button press. |
| **Nav Keypad (DOWN)** | Input | `PB13` | EXTI Interrupt | Configured with `INPUT_PULLUP`. Active LOW on button press. |
| **Nav Keypad (LEFT)** | Input | `PB15` | EXTI Interrupt | Configured with `INPUT_PULLUP`. Active LOW on button press. |
| **Nav Keypad (RIGHT)** | Input | `PB14` | EXTI Interrupt | Configured with `INPUT_PULLUP`. Active LOW on button press. |
| **Door Lock Actuator** | Relay Control | `PC13` | GPIO Output | Drives door lock relay. HIGH = Unlocked, LOW = Locked. |
| **Buzzer / Audio** | Speaker | `PA15` | GPIO / PWM Output | Piezo speaker output line for audio signaling. |
| **LSE Oscillator** | OSC32_IN/OUT | Hardware Pins | LSE Clock Input | Connected to external 32.768 kHz crystal oscillator. |

### Electrical Specifications and Wiring Notes
* **Logic Levels**: The STM32 operates at 3.3V logic. While several STM32 GPIO pins are 5V tolerant, 3.3V power routing is recommended for the SSD1306 OLED and AS608 module logic supply.
* **AS608 Power Supply**: The AS608 optical sensor requires 5V VCC supply capable of delivering peak currents during LED illumination and Fingerprint scanning.
* **Pull-Up Resistors**: Navigation button inputs rely on internal MCU pull-up resistors (`INPUT_PULLUP`). 

### System Interconnect Diagram

```text
                               +----------------------------------------+
                               |            STM32 Microcontroller       |
                               |          (STM32F103 / STM32F411)       |
                               |                                        |
+------------------------+     |                                        |     +------------------------+
|   AS608 Optical DSP    |     |                                        |     |   SSD1306 OLED Display |
|                        |     |                                        |     |        (128 x 64)      |
|  TX -------------------+---->| PA2 (USART1 RX)                        |     |                        |
|  RX <------------------+-----| PA3 (USART1 TX)        PB6 (I2C1 SCL)  |---->| SCL                    |
|  VCC (5V)              |     |                        PB7 (I2C1 SDA)  |---->| SDA                    |
|  GND                   |     |                                        |     | VCC (3.3V), GND        |
+------------------------+     |                                        |     +------------------------+
                               |                                        |
+------------------------+     |                                        |     +------------------------+
|    4-Button Keypad     |     |                                        |     |  Door Actuator Relay   |
|                        |     |                                        |     |                        |
|  UP Button ------------+---->| PB12 (EXTI Line)                       |     |  Control Signal        |
|  DOWN Button ----------+---->| PB13 (EXTI Line)       PC13 (GPIO Out) |---->|  (Active HIGH Unlock)   |
|  LEFT Button ----------+---->| PB15 (EXTI Line)                       |     +------------------------+
|  RIGHT Button ---------+---->| PB14 (EXTI Line)                       |
|  GND                   |     |                                        |     +------------------------+
+------------------------+     |                        PA15 (GPIO Out) |---->| Piezo Buzzer / Speaker |
                               |                                        |     +------------------------+
                               |                        OSC32_IN / OUT  |=====> 32.768 kHz LSE Crystal
                               +----------------------------------------+
```

### Schematic Diagram
![Schematic Diagram](path/to/schematic.png)

---

## User Manual

### First-Boot Initialization and Master Setup
1. **Cold Boot Detection**: Upon powering on, the MCU initializes hardware peripherals, configures RTC clock sources, loads EEPROM memory indexes, and queries the AS608 biometric sensor over USART1.
2. **Zero-User Condition**: If `getEnrolledCount()` returns 0 enrolled users:
   - The system automatically forces **Enrollment Mode**.
   - User #1 is assigned ID `1` and automatically granted **Master** privilege status (`notMaster = 0`).
   - The display prompts for a 4-character name via the text editor widget.
   - The user scans their finger twice to store template #1 in AS608 DSP Flash and EEPROM slot #1.
3. **Normal Boot Condition**: If one or more users exist, the system initializes directly into **Access Control Mode**.

### Keypad Navigation
The keypad consists of 4 directional buttons:
* **UP Button (`PB12`)**: Navigates options upward, increments numbers/letters in text input mode, selects page 1 menu item.
* **DOWN Button (`PB13`)**: Navigates options downward, decrements numbers/letters in text input mode, selects page 1 menu item.
* **LEFT Button (`PB15`)**: Selects options, confirms text inputs, acts as `<YES` or `<DEL` confirmation, executes delete actions.
* **RIGHT Button (`PB14`)**: Acts as back button, cancels dialogs, switches menu pages, exits submodules (`exit>` / `NO>`).

### Main Menu Structure

```text
[ Access Mode ] ── (Press RIGHT to Exit) ──> [ Menu Page 1 ] ── (Press RIGHT) ──> [ Menu Page 2 ]
                                                  │                                   │
                                                  ├── UP: Enroll Mode                 ├── UP: Log Mode
                                                  ├── DOWN: Access Mode               └── DOWN: Set Time Mode
                                                  └── LEFT: Delete Mode
```

### Access Control Mode
* **Default Display**: Displays `"Place Finger"` with header `"Access Mode"`.
* **Verification Flow**:
  1. User places finger on AS608 sensor.
  2. Sensor captures print and checks against onboard DSP template database.
  3. If match is found:
     - OLED displays `"Welcome <NAME>"`.
     - Access entry log is stored into flash EEPROM via `logAccess()`.
     - Door lock relay pin (`PC13`) is set HIGH to unlock door for 4.0 seconds.
     - Door lock relay pin (`PC13`) returns LOW to re-lock door.
  4. If match fails:
     - OLED displays `"Invalid"` for 1.0 second and returns to ready state.

### User Enrollment Mode
* **Privilege Guard**: Requires Master permission scan before entering enrollment.
* **Enrollment Steps**:
  1. System checks for available storage slots (`getNextAvailableID()`). If capacity (300 users) is exceeded, displays `"no space"`.
  2. Prompt for User Name using the 4-character text input widget.
  3. Master Status Query: Displays `"Master??"` with options `<YES` (LEFT) or `NO>` (RIGHT).
  4. Biometric Registration:
     - Displays `"PLACE FINGER"`. Sensor captures first image.
     - Displays `"REMOVE FINGER"`. User removes finger for 2 seconds.
     - Displays `"place finger"`. User places same finger again.
     - Fuses template buffers (`createModel()`) and stores to AS608 DSP Flash (`storeModel()`).
  5. EEPROM Flash Write: Writes `UserRecord` binary struct to EEPROM flash at calculated address offset.
  6. Repeat Query: Displays `"Again??"` (`<YES` / `NO>`).

### User Deletion Mode
* **Privilege Guard**: Requires Master permission scan before entering delete mode.
* **Selection Submenu**: Prompts `"Query Opt"` with options `U-Scroll` (UP) or `D-Name` (DOWN).
* **Option 1: Scroll Mode (`UP`)**:
  1. Displays enrolled user names sequentially: `"Del <NAME>"`.
  2. Navigate users with `UP` and `DOWN` buttons.
  3. Press `LEFT` (`<DEL`) to delete.
  4. **Sole Master Check**: If target user is a Master (`notMaster == 0`) and `getMasterCount() <= 1`, deletion is blocked. Displays `"Cant't Delete"` and aborts.
  5. If valid, deletes AS608 biometric template (`deleteFingerprint`), clears EEPROM slot (`deleteUser`), and updates enrolled count.
* **Option 2: Name Search Mode (`DOWN`)**:
  1. Prompts for 4-character target name using text editor widget.
  2. Searches EEPROM records matching input string.
  3. Evaluates Sole Master rule for any matching records.
  4. Deletes matching biometric templates and EEPROM records. Displays `<NAME> deltd` or `Not Found`.

### Audit Log Mode
* **Privilege Guard**: Requires Master permission scan before entering log mode.
* **Log Viewer Flow**:
  1. Queries total stored records from EEPROM via `getLogCount()`. If 0, displays `"No Logs"`.
  2. Renders newest log entry on screen:
     - Line 1: `Log X/Y` (Index out of total logs)
     - Line 2: `<NAME>`
     - Line 3: `DD/MM/YY HH:MM`
  3. Navigate chronologically through log history using `UP` and `DOWN` buttons.
  4. Press `RIGHT` to return to menu.

### Set Time Mode
* **Privilege Guard**: Requires Master permission scan before entering set time mode.
* **Interactive Clock Editor**:
  1. Renders current RTC time `[HH:MM]` and date `[DD/MM/20YY]`.
  2. Active field cursor pointer (`^`) highlights selected field (0: Hours, 1: Minutes, 2: Day, 3: Month, 4: Year).
  3. `RIGHT` Button: Cycles active editable field (`Hours -> Minutes -> Day -> Month -> Year`).
  4. `UP` / `DOWN` Buttons: Increments or decrements selected parameter with auto-wrapping boundaries.
  5. `LEFT` Button: Commits updated timestamps directly into internal STM32 hardware RTC registers via `setManualDateTime()` and returns to main menu.

---

## Features and Code Walkthrough

### 1. C++ Template-Based Interrupt Engine
Standard Arduino interrupt registration (`attachInterrupt`) requires plain global function pointers with `void()` signatures. Direct member function binding usually requires global pointers or verbose wrapper functions. This firmware resolves ISR instance binding using C++ compile-time template instantiation within the `navButton` struct in `buttons.h`:

```cpp
struct navButton {
private:
  volatile bool state = false;
  volatile uint32_t lastInterruptTime = 0;
  uint8_t pin;

  void stateCheck() {
    uint32_t now = millis();
    if (now - lastInterruptTime > 10) {  // Software debounce window
      state = (digitalRead(pin) == LOW);
      lastInterruptTime = now;
    }
  }

public:
  navButton(uint8_t buttonPin) : pin(buttonPin) {};

  template<navButton& ButtonInstance>
  void begin(uint8_t mode = INPUT_PULLUP, uint32_t interruptMode = CHANGE) {
    pinMode(pin, mode);
    attachInterrupt(digitalPinToInterrupt(pin), isr<ButtonInstance>, (PinStatus)interruptMode);
  }

  bool wasPressed() {
    if (state) {
      state = false; // Latch self-clearing logic
      return true;
    }
    return false;
  }

  template<navButton& ButtonInstance>
  static void isr() {
    ButtonInstance.stateCheck();
  }
};
```

#### Key Architecture Benefits
* **Zero Runtime Vtable Overhead**: Static function addresses are generated by the compiler at compile time via template instantiation (`begin<UP_BUTTON>()`).
* **Non-Blocking Debouncing**: Calculates interrupt delta times (`now - lastInterruptTime > 10`) inside the ISR handler, eliminating blocking `delay()` calls.
* **Atomic Latch Reading**: `wasPressed()` provides single-shot trigger evaluation, returning `true` once and clearing the internal state flag immediately.

---

### 2. Flash EEPROM Memory Layout and Direct Offsets
User credentials are preserved in STM32 internal flash memory using emulated EEPROM bytes. Instead of linearly scanning memory to retrieve records, the firmware computes direct hardware address offsets using array-like math.

```cpp
// users.h
#define MAX_USERS 300

struct UserRecord {
  uint8_t magic;         // Validation byte (0xA5)
  uint16_t id;           // Unique user ID (1 to 300)
  char name[5];          // 4 characters + null terminator
  uint8_t notMaster;     // Role authorization: 0 = Master, 1 = Standard User
};
```

```cpp
// users.cpp
#define VALID_MAGIC 0xA5

int getEEPROMAddress(uint16_t userID) {
  return (userID - 1) * sizeof(UserRecord); // O(1) mathematical memory index calculation
}

bool userExist(uint16_t userID, UserRecord &user) {
  int addr = getEEPROMAddress(userID);
  EEPROM.get(addr, user);
  return (user.magic == VALID_MAGIC); // Flash byte default is 0xFF != 0xA5
}
```

#### Memory Layout
```text
EEPROM Memory Address Map:
+-------------------+--------------------+------------------------+
| Byte Range        | Contents           | Record Capacity        |
+-------------------+--------------------+------------------------+
| 0000 - 2999       | User Records       | Up to 300 User Records |
|                   | (UserRecord struct)| (10 bytes per struct)  |
+-------------------+--------------------+------------------------+
| 3000 - 3479       | Access Audit Logs  | Up to 20 Log Records   |
|                   | (LogRecord struct) | (24 bytes per struct)  |
+-------------------+--------------------+------------------------+
```

---

### 3. Master Authorization Gate and Sole Master Protection
To maintain administrative control and prevent accidental lockout, administrative submodules (Enrollment, Deletion, Log Viewing, and Time Setting) pass through `verifyMasterPermission()` in `Fingerprint_Access_Control.ino`. Additionally, deletion actions check `getMasterCount()`.

```cpp
// Fingerprint_Access_Control.ino
bool verifyMasterPermission(const char *modeHeader) {
  if (enrolledUsers == 0) {
    return true; // Bypass check on empty boot
  }
  display_three_words("Place", "Master", "Finger", 2);
  while (1) {
    if (exit()) return false;
    SensorRsp rsp = getFingerprintID();
    if (!rsp.isError) {
      uint16_t id = (uint16_t)strtol(rsp.msg, NULL, 10);
      UserRecord master = getUser(id);
      if (master.notMaster == 0) { // Verified Master user
        displayTwoWords("Master", master.name, modeHeader);
        delay(1000);
        return true;
      }
    }
  }
}
```

```cpp
// Deletion Guard Check in Delete Mode
if (user.notMaster == 0 && getMasterCount() <= 1) {
  debugln("Blocked deletion of sole Master user.");
  displayTwoWords("Cant't", "Delete", DeleteModeHeader);
  delay(1500);
  continue;
}
```

---

### 4. AS608 Biometric Fingerprint Sensor
The AS608 biometric fingerprint sensor module communicates over USART1 (`PA2`/`PA3`) at 57,600 baud. Enrollment requires a two-pass capture pipeline to extract feature vectors, generate a fused model, and write to the internal flash of the sensor.

```cpp
// biometrics.cpp (Enrollment State Machine Excerpt)
uint8_t getFingerprintEnroll(uint8_t id) {
  // Pass 1: Capture initial fingerprint image
  while (finger.getImage() != FINGERPRINT_OK);
  if (finger.image2Tz(1) != FINGERPRINT_OK) return FINGERPRINT_FEATUREFAIL;

  // Wait for finger removal
  while (finger.getImage() != FINGERPRINT_NOFINGER);

  // Pass 2: Capture second fingerprint image
  while (finger.getImage() != FINGERPRINT_OK);
  if (finger.image2Tz(2) != FINGERPRINT_OK) return FINGERPRINT_FEATUREFAIL;

  // Model Fusion & DSP Flash Persistence
  if (finger.createModel() != FINGERPRINT_OK) return FINGERPRINT_ENROLLMISMATCH;
  if (finger.storeModel(id) != FINGERPRINT_OK) return FINGERPRINT_FLASHERR;

  return FINGERPRINT_OK;
}
```

---

### 5. Internal RTC Seeding and Manual Setting Engine
The system uses the STM32 internal RTC hardware peripheral clocked by a 32.768 kHz Low-Speed External (LSE) crystal oscillator. On first power-up, `timeConfig()` parses compiler preprocessor build macros (`__TIME__`, `__DATE__`) to auto-seed the clock without requiring an external RTC IC.

```cpp
// myTime.cpp
void timeConfig() {
  rtc.setClockSource(STM32RTC::LSE_CLOCK);
  rtc.begin();
  if (!rtc.isTimeSet()) {
    byte hours, minutes, seconds;
    sscanf(__TIME__, "%d:%d:%d", &hours, &minutes, &seconds);

    char monthStr[4];
    int day, year;
    sscanf(__DATE__, "%s %d %d", monthStr, &day, &year);

    const char *months[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                             "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
    int month = 1;
    for (int i = 0; i < 12; i++) {
      if (strcmp(monthStr, months[i]) == 0) { month = i + 1; break; }
    }
    rtc.setTime(hours, minutes, seconds);
    rtc.setDate(day, month, year % 100);
  }
}
```

---

### 6. EEPROM-Backed Circular Audit Logging System
Access logs are saved to a non-volatile 20-record circular log buffer in flash EEPROM starting at address 3000 (`MAX_USERS * sizeof(UserRecord)`).

```cpp
// logger.h
struct LogRecord {
  uint8_t magic;      // 0xA5 validation byte
  char name[5];       // 4-char user name + null
  char time[6];       // Format "HH:MM"
  char date[9];       // Format "DD/MM/YY"
};
```

```cpp
// logger.cpp
void logAccess(const char *name, bool granted) {
  LogRecord logRec;
  logRec.magic = LOG_MAGIC;
  strncpy(logRec.name, name, 4); logRec.name[4] = '\0';
  strncpy(logRec.time, getTime(), sizeof(logRec.time) - 1);
  strncpy(logRec.date, getDate(), sizeof(logRec.date) - 1);

  int addr = LOG_EEPROM_START_ADDR + (logHead * sizeof(LogRecord));
  EEPROM.put(addr, logRec);
  logHead = (logHead + 1) % MAX_LOGS; // Circular buffer index wrapping
}
```

---

### 7. Constrained OLED Text Input Editor
Users are named without a full keyboard using a 4-character cursor text input engine driven by `getUserName()` in `users.cpp`:

```cpp
// users.cpp
void getUserName(char (&name)[5]) {
  while (1) {
    displayTextInput(name, cursorPos);
    if (RIGHT_BUTTON.wasPressed()) {
      cursorPos = (cursorPos + 1) % 4; // Advance cursor position
    }
    if (UP_BUTTON.wasPressed()) {
      char c = name[cursorPos] + 1;
      if (c > 'Z') c = '@'; // ASCII upper bound wrapping
      name[cursorPos] = c;
    } else if (DOWN_BUTTON.wasPressed()) {
      char c = name[cursorPos] - 1;
      if (c < 'A') c = '@'; // ASCII lower bound wrapping
      name[cursorPos] = c;
    }
    if (LEFT_BUTTON.wasPressed()) {
      return; // Confirm and return name
    }
  }
}
```

---

## Function Reference

### Main Application Module (Fingerprint_Access_Control.ino)

#### `void setup()`
* **Signature**: `void setup()`
* **Parameters**: None
* **Return Value**: `void`
* **Description**: Initializes serial debugging (115200 baud), configures RTC timekeeping (`timeConfig`), button interrupts (`buttonConfig`), audit logger (`loggerInit`), and SSD1306 display (`checkDisplay`). Configures door relay pin (`PC13`) as output, locks door, initializes AS608 biometric sensor, counts existing users in EEPROM, and sets initial system mode to `"enroll"` (if zero users) or `"access"`.

#### `void loop()`
* **Signature**: `void loop()`
* **Parameters**: None
* **Return Value**: `void`
* **Description**: Executes the system Finite State Machine across modes: `"menu"`, `"enroll"`, `"access"`, `"delete"`, `"log"`, and `"settime"`. Evaluates input button events to transition states and invoke submodule routines.

#### `bool verifyMasterPermission(const char *modeHeader)`
* **Signature**: `bool verifyMasterPermission(const char *modeHeader)`
* **Parameters**:
  * `modeHeader`: `const char*` - String label displayed at top of OLED screen.
* **Return Value**: `bool` - Returns `true` if a registered Master fingerprint scan is verified; returns `false` if cancelled or unauthorized.
* **Description**: Displays Master authorization prompt on OLED screen. Polls AS608 biometric sensor until a scanned print matches a user record with `notMaster == 0`. Bypasses verification automatically if zero users exist (`enrolledUsers == 0`).

#### `bool exit()`
* **Signature**: `bool exit()`
* **Parameters**: None
* **Return Value**: `bool` - Returns `true` if RIGHT button was pressed; `false` otherwise.
* **Description**: Helper function checking RIGHT navigation button status to trigger mode exit back to main menu.

#### `void lockDoor()`
* **Signature**: `void lockDoor()`
* **Parameters**: None
* **Return Value**: `void`
* **Description**: Sets GPIO pin `PC13` LOW, de-energizing door lock relay mechanism.

#### `void unlockDoor()`
* **Signature**: `void unlockDoor()`
* **Parameters**: None
* **Return Value**: `void`
* **Description**: Sets GPIO pin `PC13` HIGH, energizing door lock relay mechanism.

---

### Biometrics Module (biometrics.h / biometrics.cpp)

#### `void biometricsSetup()`
* **Signature**: `void biometricsSetup()`
* **Parameters**: None
* **Return Value**: `void`
* **Description**: Initializes hardware serial connection (`mySerial`) at 57,600 baud for AS608 sensor interface. Enters persistent verification retry loop (`finger.verifyPassword()`) until sensor connection is confirmed. Prints sensor registers, status flags, capacity, security level, and packet lengths to serial debug output.

#### `uint8_t getFingerprintEnroll(uint8_t id)`
* **Signature**: `uint8_t getFingerprintEnroll(uint8_t id)`
* **Parameters**:
  * `id`: `uint8_t` - Target slot ID (1 to 300) to assign to enrolled print.
* **Return Value**: `uint8_t` - Returns `FINGERPRINT_OK` (0x00) on success; error code status byte otherwise.
* **Description**: Manages two-pass fingerprint image capture, template conversion (`image2Tz(1)` and `image2Tz(2)`), model fusion (`createModel()`), and commits generated template to AS608 internal DSP flash (`storeModel(id)`). Renders step-by-step progress guidance to OLED screen.

#### `SensorRsp getFingerprintID()`
* **Signature**: `SensorRsp getFingerprintID()`
* **Parameters**: None
* **Return Value**: `SensorRsp` - Structure containing message string pointer (`msg`) and error boolean flag (`isError`).
* **Description**: Captures single fingerprint image, converts image to feature vector (`image2Tz()`), and performs search query (`fingerSearch()`) against internal AS608 template database. If match is found, populates `msg` with ASCII user ID string and sets `isError = false`. Returns error status string and `isError = true` on failure or missing finger.

#### `bool deleteFingerprint(uint16_t id)`
* **Signature**: `bool deleteFingerprint(uint16_t id)`
* **Parameters**:
  * `id`: `uint16_t` - Fingerprint template ID to erase.
* **Return Value**: `bool` - Returns `true` if template was successfully removed from AS608 DSP flash; `false` on failure.
* **Description**: Issues `finger.deleteModel(id)` command packet to erase specified fingerprint template from AS608 internal flash memory.

#### `void deleteAllPrints()`
* **Signature**: `void deleteAllPrints()`
* **Parameters**: None
* **Return Value**: `void`
* **Description**: Issues `finger.emptyDatabase()` command packet to clear all stored biometric templates from AS608 DSP memory.

---

### Button Interrupt Module (buttons.h / buttons.cpp)

#### `navButton::begin<ButtonInstance>(uint8_t mode, uint32_t interruptMode)`
* **Signature**: `template<navButton& ButtonInstance> void begin(uint8_t mode = INPUT_PULLUP, uint32_t interruptMode = CHANGE)`
* **Parameters**:
  * `mode`: `uint8_t` - Pin mode configuration (default `INPUT_PULLUP`).
  * `interruptMode`: `uint32_t` - Interrupt trigger condition (default `CHANGE`).
* **Return Value**: `void`
* **Description**: Configures MCU GPIO pin mode and attaches hardware EXTI interrupt handler pointing to static template ISR wrapper `isr<ButtonInstance>()`.

#### `bool navButton::wasPressed()`
* **Signature**: `bool wasPressed()`
* **Parameters**: None
* **Return Value**: `bool` - Returns `true` if button press event latched since last query; `false` otherwise.
* **Description**: Evaluates atomic state flag. If set, clears internal state flag to `false` and returns `true`.

#### `void buttonConfig()`
* **Signature**: `void buttonConfig()`
* **Parameters**: None
* **Return Value**: `void`
* **Description**: Initializes 4 navigation button instances (`UP_BUTTON`, `DOWN_BUTTON`, `LEFT_BUTTON`, `RIGHT_BUTTON`) on pins `PB12`, `PB13`, `PB15`, `PB14` respectively, binding hardware interrupts.

#### `char* choice()`
* **Signature**: `char* choice()`
* **Parameters**: None
* **Return Value**: `char*` - Returns `"yes"` if LEFT button was pressed, `"no"` if RIGHT button was pressed, or `""` if neither.
* **Description**: Helper function evaluating two-choice confirmation dialog inputs.

---

### User Management Module (users.h / users.cpp)

#### `int getEnrolledCount()`
* **Signature**: `int getEnrolledCount()`
* **Parameters**: None
* **Return Value**: `int` - Number of active enrolled user slots in EEPROM flash.
* **Description**: Iterates through user IDs (1 to 300), querying `userExist()`, counting total valid stored user records.

#### `bool userExist(uint16_t userID, UserRecord &user)`
* **Signature**: `bool userExist(uint16_t userID, UserRecord &user)`
* **Parameters**:
  * `userID`: `uint16_t` - Target user ID slot to query.
  * `user`: `UserRecord&` - Reference to record struct populated with slot contents.
* **Return Value**: `bool` - Returns `true` if slot contains `magic == 0xA5`; `false` if slot is unassigned/erased.
* **Description**: Computes EEPROM byte address using `getEEPROMAddress()`, reads `UserRecord` struct from flash EEPROM, and verifies magic validation byte.

#### `UserRecord getUser(uint16_t id)`
* **Signature**: `UserRecord getUser(uint16_t id)`
* **Parameters**:
  * `id`: `uint16_t` - Target user ID slot.
* **Return Value**: `UserRecord` - User record data struct read from EEPROM.
* **Description**: Reads and returns `UserRecord` from calculated EEPROM memory location.

#### `int getEEPROMAddress(uint16_t userID)`
* **Signature**: `int getEEPROMAddress(uint16_t userID)`
* **Parameters**:
  * `userID`: `uint16_t` - User ID (1 to 300).
* **Return Value**: `int` - Direct byte memory address in EEPROM flash.
* **Description**: Calculates address offset as `(userID - 1) * sizeof(UserRecord)`.

#### `void saveUser(UserRecord &user)`
* **Signature**: `void saveUser(UserRecord &user)`
* **Parameters**:
  * `user`: `UserRecord&` - Reference to user record struct to commit.
* **Return Value**: `void`
* **Description**: Sets `user.magic = 0xA5`, calculates address via `getEEPROMAddress()`, and writes record to EEPROM flash via `EEPROM.put()`.

#### `uint16_t getNextAvailableID()`
* **Signature**: `uint16_t getNextAvailableID()`
* **Parameters**: None
* **Return Value**: `uint16_t` - Lowest unused user ID slot (1 to 300); returns `0` if all slots full.
* **Description**: Scans EEPROM slots sequentially to find first slot where `userExist()` returns `false`.

#### `int getMasterCount()`
* **Signature**: `int getMasterCount()`
* **Parameters**: None
* **Return Value**: `int` - Number of active enrolled users with `notMaster == 0`.
* **Description**: Scans user records (1 to 300) and counts active users possessing Master privilege level.

#### `void deleteUser(UserRecord &user)`
* **Signature**: `void deleteUser(UserRecord &user)`
* **Parameters**:
  * `user`: `UserRecord&` - User record struct reference to invalidate.
* **Return Value**: `void`
* **Description**: Sets `user.magic = 0`, writes updated struct to EEPROM flash, decrements global `enrolledUsers` counter, and outputs debug log.

#### `void deleteAll()`
* **Signature**: `void deleteAll()`
* **Parameters**: None
* **Return Value**: `void`
* **Description**: Clears EEPROM memory slots by setting flash bytes to 0 across user storage area.

#### `void getUserName(char (&name)[5])`
* **Signature**: `void getUserName(char (&name)[5])`
* **Parameters**:
  * `name`: `char (&)[5]` - Reference to 5-byte array storing 4 characters + null terminator.
* **Return Value**: `void`
* **Description**: Runs interactive text input UI on OLED screen. Allows user to select character slots with RIGHT button, cycle ASCII characters (`@`, `A`-`Z`) with UP/DOWN buttons, and confirm string entry with LEFT button.

---

### Audit Logging Module (logger.h / logger.cpp)

#### `void loggerInit()`
* **Signature**: `void loggerInit()`
* **Parameters**: None
* **Return Value**: `void`
* **Description**: Scans 20 log EEPROM slots starting at memory address 3000 (`MAX_USERS * sizeof(UserRecord)`), counting existing valid log entries (`magic == 0xA5`).

#### `void logAccess(const char *name, bool granted)`
* **Signature**: `void logAccess(const char *name, bool granted)`
* **Parameters**:
  * `name`: `const char*` - 4-character user name string.
  * `granted`: `bool` - Access result flag.
* **Return Value**: `void`
* **Description**: Constructs `LogRecord` struct, populates `magic = 0xA5`, copies `name`, current RTC time (`getTime()`), and current RTC date (`getDate()`). Writes record into EEPROM flash at circular head index (`logHead`), updating index (`logHead = (logHead + 1) % 20`).

#### `int getLogCount()`
* **Signature**: `int getLogCount()`
* **Parameters**: None
* **Return Value**: `int` - Total count of valid access log records stored in EEPROM.
* **Description**: Scans 20 log slots in EEPROM and returns count of valid slots containing `magic == 0xA5`.

#### `LogRecord getLog(int index)`
* **Signature**: `LogRecord getLog(int index)`
* **Parameters**:
  * `index`: `int` - Log history offset index (0 = newest log, `count-1` = oldest log).
* **Return Value**: `LogRecord` - Log record struct retrieved from EEPROM.
* **Description**: Scans EEPROM log slots, builds array of valid logs, and returns the Nth newest log record.

---

### Real-Time Clock Module (myTime.h / myTime.cpp)

#### `void timeConfig()`
* **Signature**: `void timeConfig()`
* **Parameters**: None
* **Return Value**: `void`
* **Description**: Configures RTC clock source (LSE oscillator), initializes hardware RTC peripheral in 24-hour format, and checks if time is set. If unset, parses compiler macros `__TIME__` (`HH:MM:SS`) and `__DATE__` (`Mmm DD YYYY`) to seed internal hardware RTC registers automatically.

#### `char* getTime()`
* **Signature**: `char* getTime()`
* **Parameters**: None
* **Return Value**: `char*` - Pointer to static string buffer formatted as `"HH:MM"`.
* **Description**: Queries internal RTC hardware registers for hours and minutes, formatting output into static string buffer via `snprintf()`.

#### `char* getDate()`
* **Signature**: `char* getDate()`
* **Parameters**: None
* **Return Value**: `char*` - Pointer to static string buffer formatted as `"DD/MM/YY"`.
* **Description**: Queries internal RTC hardware registers for day, month, and year, formatting output into static string buffer via `snprintf()`.

#### `void setManualDateTime(uint8_t day, uint8_t month, uint8_t year, uint8_t hours, uint8_t minutes)`
* **Signature**: `void setManualDateTime(uint8_t day, uint8_t month, uint8_t year, uint8_t hours, uint8_t minutes)`
* **Parameters**:
  * `day`: `uint8_t` - Day of month (1 to 31).
  * `month`: `uint8_t` - Month of year (1 to 12).
  * `year`: `uint8_t` - 2-digit year (0 to 99).
  * `hours`: `uint8_t` - Hours (0 to 23).
  * `minutes`: `uint8_t` - Minutes (0 to 59).
* **Return Value**: `void`
* **Description**: Writes user-specified date and time values directly into STM32 hardware RTC registers.

---

### Display Engine Module (display.h / display.cpp)

#### `bool checkDisplay()`
* **Signature**: `bool checkDisplay()`
* **Parameters**: None
* **Return Value**: `bool` - Returns `true` if SSD1306 OLED initializes at address `0x3C`; `false` on failure.
* **Description**: Initializes SSD1306 OLED display driver (`display.begin(SSD1306_SWITCHCAPVCC, 0x3C)`).

#### `void displayWord(const char *word, const char *frameTitle, const char* optionR, const char* optionL)`
* **Signature**: `void displayWord(const char *word, const char *frameTitle, const char* optionR="", const char* optionL="")`
* **Parameters**:
  * `word`: `const char*` - Center text string to render.
  * `frameTitle`: `const char*` - Header text string.
  * `optionR`: `const char*` - Right button label text (optional).
  * `optionL`: `const char*` - Left button label text (optional).
* **Return Value**: `void`
* **Description**: Clears screen display buffer, draws header frame, renders centered word, draws option labels, and updates OLED display.

#### `void displayTwoWords(const char *word1, const char *word2, const char *frameTitle, const char* optionR, const char* optionL)`
* **Signature**: `void displayTwoWords(const char *word1, const char *word2, const char *frameTitle, const char* optionR="", const char* optionL="")`
* **Parameters**:
  * `word1`: `const char*` - First text line string.
  * `word2`: `const char*` - Second text line string.
  * `frameTitle`: `const char*` - Header title string.
  * `optionR`: `const char*` - Right option button label.
  * `optionL`: `const char*` - Left option button label.
* **Return Value**: `void`
* **Description**: Renders two centered text lines inside header frame with option labels.

#### `void displayTextInput(char *name, int8_t cursorPos)`
* **Signature**: `void displayTextInput(char *name, int8_t cursorPos)`
* **Parameters**:
  * `name`: `char*` - Pointer to 4-character text string array.
  * `cursorPos`: `int8_t` - Active cursor selection index (0 to 3).
* **Return Value**: `void`
* **Description**: Renders 4-character text string centered on OLED display with underline cursor rectangle beneath active position `cursorPos`.

#### `void displayOption(const char *text, const char* optionR, const char* optionL)`
* **Signature**: `void displayOption(const char *text, const char* optionR="NO>", const char* optionL="<YES")`
* **Parameters**:
  * `text`: `const char*` - Prompt question string.
  * `optionR`: `const char*` - Right option label (default `"NO>"`).
  * `optionL`: `const char*` - Left option label (default `"<YES"`).
* **Return Value**: `void`
* **Description**: Displays prompt question with dual choice navigation labels.

#### `void displayCustomMenu(const char *title, const char *option1, const char *option2)`
* **Signature**: `void displayCustomMenu(const char *title, const char *option1, const char *option2)`
* **Parameters**:
  * `title`: `const char*` - Submenu header title.
  * `option1`: `const char*` - First menu option string.
  * `option2`: `const char*` - Second menu option string.
* **Return Value**: `void`
* **Description**: Renders two-option submenu prompt screen on OLED.

#### `void displayMenu(uint8_t page)`
* **Signature**: `void displayMenu(uint8_t page = 1)`
* **Parameters**:
  * `page`: `uint8_t` - Active menu page index (1 or 2).
* **Return Value**: `void`
* **Description**: Renders 2-page system menu layout with navigation button guides (`U: Enroll`, `D: Access`, `L: Delete`, `R: Next>` on Page 1; `U: Log`, `D: Set Time`, `R: <Back` on Page 2).

#### `void display_three_words(const char* text1, const char* text2, const char* text3, int size)`
* **Signature**: `void display_three_words(const char* text1, const char* text2, const char* text3, int size)`
* **Parameters**:
  * `text1`: `const char*` - Line 1 text.
  * `text2`: `const char*` - Line 2 text.
  * `text3`: `const char*` - Line 3 text.
  * `size`: `int` - Font scale multiplier size.
* **Return Value**: `void`
* **Description**: Renders three stacked centered text lines on screen.

#### `void displayLogEntry(const char *name, const char *date, const char *time, int index, int total)`
* **Signature**: `void displayLogEntry(const char *name, const char *date, const char *time, int index, int total)`
* **Parameters**:
  * `name`: `const char*` - User name string in log entry.
  * `date`: `const char*` - Date timestamp string (`DD/MM/YY`).
  * `time`: `const char*` - Time timestamp string (`HH:MM`).
  * `index`: `int` - Active record index displayed.
  * `total`: `int` - Total records in log buffer.
* **Return Value**: `void`
* **Description**: Formats and renders single access log record entry frame on screen.

#### `void displayTimeInput(uint8_t day, uint8_t month, uint8_t year, uint8_t hours, uint8_t minutes, uint8_t fieldPos)`
* **Signature**: `void displayTimeInput(uint8_t day, uint8_t month, uint8_t year, uint8_t hours, uint8_t minutes, uint8_t fieldPos)`
* **Parameters**:
  * `day`: `uint8_t` - Day parameter value.
  * `month`: `uint8_t` - Month parameter value.
  * `year`: `uint8_t` - 2-digit year value.
  * `hours`: `uint8_t` - Hours parameter value.
  * `minutes`: `uint8_t` - Minutes parameter value.
  * `fieldPos`: `uint8_t` - Active editable cursor field index (0: Hours, 1: Minutes, 2: Day, 3: Month, 4: Year).
* **Return Value**: `void`
* **Description**: Formats interactive time setting GUI display on OLED screen, placing pointer indicator (`^`) under active editable field `fieldPos`.
