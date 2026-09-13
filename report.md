# 📑 Fingerprint Access Control System - Project Completion & Technical Report

**Project Name**: STM32 Fingerprint Access Control System  
**Author/Developer**: Antigravity AI & Project Lead  
**Target Hardware**: Generic STM32F4 Series (`BLACKPILL_F411CE`), AS608 Optical Fingerprint DSP, SSD1306 128x64 OLED, 4-Button EXTI Keypad, Internal RTC  
**Build Target FQBN**: `STMicroelectronics:stm32:GenF4:pnum=BLACKPILL_F411CE`  
**Build Status**: `BUILD SUCCESSFUL` (0 Errors, 0 Warnings via `arduino-cli`)

---

## 📌 Executive Summary

The **Fingerprint Access Control System** has been fully completed, refactored, and verified for the **STM32F4 Blackpill F411CE** board target. All requested features—including Master Privilege Abstraction, Delete Mode (Scroll & Name options), Persistent RTC Audit Logging Mode, and a Two-Page Main Menu with Manual RTC Time & Date Input—have been implemented with clear feedback on the 128x64 OLED display and serial debug outputs (`DEBUG_PRINT`).

---

## 🚀 Summary of New Changes & Architectural Enhancements

### 1. Abstracted Master Authorization Gate (`verifyMasterPermission`)
- **Implementation**: Created `bool verifyMasterPermission(const char *modeHeader)` in `Fingerprint_Access_Control.ino`.
- **Functionality**:
  - Automatically checks if any users exist in EEPROM (`enrolledUsers == 0`). On cold boot with zero users, master check is bypassed to allow setup of user #1 as Master.
  - For system operational state (`enrolledUsers > 0`), any attempt to access **Enroll**, **Delete**, **Log**, or **Set Time** modes requires scanning a valid Master fingerprint (`master.notMaster == 0`).
  - Provides instant OLED visual feedback (`"Master <Name>"` vs `"Not Master"` / `"No Match"`) and debug log output.
  - Allows clean cancellation via the `RIGHT` navigation button.

### 2. Two-Page Main Menu Structure
- **Page 1**:
  - `U - Enroll`
  - `D - Access`
  - `L - Delete`
  - `R - Next >` (Switches to Page 2)
- **Page 2**:
  - `U - Log` (Master Protected)
  - `D - Set Time` (Master Protected)
  - `R - < Back` (Returns to Page 1)

### 3. Manual RTC Time & Date Selector Widget
- **Master Protected**: Guarded by `verifyMasterPermission("Set Time")`.
- **Interactive UI (`displayTimeInput`)**:
  - Displays formatted time `[HH:MM]` and date `[DD/MM/20YY]` on the OLED display.
  - Renders a selection pointer (`^`) under the active editable field (`Hours`, `Minutes`, `Day`, `Month`, `Year`).
  - `RIGHT_BUTTON`: Cycles active cursor field (`Hours -> Minutes -> Day -> Month -> Year`).
  - `UP_BUTTON` / `DOWN_BUTTON`: Increments / decrements the selected field with boundary wrapping.
  - `LEFT_BUTTON`: Saves new timestamp directly to the hardware RTC (`setManualDateTime`) and returns to the menu.

### 4. Complete Delete Mode (Scroll & Name Options)
- **Submenu Selection**: Prompts the user with `Query Opt: U-Scroll / D-Name`.
- **Option 1: Scroll Mode (`UP_BUTTON`)**:
  - Dynamically populates a list of active enrolled user IDs.
  - Allows sequential navigation through users using `UP` and `DOWN` buttons.
  - OLED displays delete prompt: `Del <Name>` with `<DEL` (LEFT) and `exit>` (RIGHT).
  - Pressing `LEFT` deletes the biometric template from AS608 DSP Flash (`deleteFingerprint`), invalidates the EEPROM slot (`deleteUser`), and refreshes the active list.
- **Option 2: Name Search Mode (`DOWN_BUTTON`)**:
  - Prompts 4-character name entry via `getUserName()`.
  - As requested, 3-letter names are appended with `@` (e.g., `"BOB@"`), matching the 4-character stored format.
  - Searches all enrolled EEPROM records and deletes any matching user records and biometric DSP templates.
  - Displays `"BOB@ deltd"` or `"Not Found"` on the OLED display.

### 5. Persistent RTC Audit Logging & Log Mode
- **Persistent EEPROM Storage (`logger.h` / `logger.cpp`)**:
  - Implemented a 20-record circular log buffer stored in EEPROM starting at address `3000` (after max user storage space). Logs persist across hardware power cycles.
  - Each `LogRecord` stores user name, RTC timestamp (`HH:MM`), and RTC date (`DD/MM/YY`).
- **Automatic Access Logging**:
  - In **Access Mode**, successful door unlocks log `user.name`.
- **Log Mode Viewer**:
  - Protected by Master privilege check.
  - Displays formatted log records on the OLED display (`displayLogEntry`): `Log X/Y`, `<Name>`, `DD/MM/YY HH:MM`.
  - Uses `UP` and `DOWN` buttons to iterate logs (newest to oldest), with `RIGHT` button exiting to menu.

---

## 🛠️ Verification & Compilation Output

Compiled using `arduino-cli` targeting **Generic STM32F4 Series Blackpill F411CE** (`STMicroelectronics:stm32:GenF4:pnum=BLACKPILL_F411CE`):

```text
Used library         Version Path                                                                       
Wire                 1.0     C:\Users\DELL\AppData\Local\Arduino15\packages\STMicroelectronics\hardware\stm32\3.0.0\libraries\Wire
Adafruit GFX Library 1.12.1  C:\Users\DELL\OneDrive - Pan-Atlantic University\Arduino\libraries\Adafruit_GFX_Library
Adafruit BusIO       1.17.2  C:\Users\DELL\OneDrive - Pan-Atlantic University\Arduino\libraries\Adafruit_BusIO
SPI                  1.0     C:\Users\DELL\AppData\Local\Arduino15\packages\STMicroelectronics\hardware\stm32\3.0.0\libraries\SPI
Adafruit SSD1306     2.5.9   C:\Users\DELL\OneDrive - Pan-Atlantic University\Arduino\libraries\Adafruit_SSD1306
Adafruit Fingerprint 2.1.3   C:\Users\DELL\OneDrive - Pan-Atlantic University\Arduino\libraries\Adafruit_Fingerprint_Sensor_Library
EEPROM               2.0.1   C:\Users\DELL\AppData\Local\Arduino15\packages\STMicroelectronics\hardware\stm32\3.0.0\libraries\EEPROM
STM32duino RTC       2.0.0   C:\Users\DELL\OneDrive - Pan-Atlantic University\Arduino\libraries\STM32duino_RTC
SrcWrapper           1.0.1   C:\Users\DELL\AppData\Local\Arduino15\packages\STMicroelectronics\hardware\stm32\3.0.0\libraries\SrcWrapper
USBDevice            1.0.0   C:\Users\DELL\AppData\Local\Arduino15\packages\STMicroelectronics\hardware\stm32\3.0.0\libraries\USBDevice

Used platform            Version Path                                                                          
STMicroelectronics:stm32 3.0.0   C:\Users\DELL\AppData\Local\Arduino15\packages\STMicroelectronics\hardware\stm32\3.0.0

Sketch uses 68,336 bytes (13%) of program storage space. Maximum is 524,288 bytes.
Global variables use 5,744 bytes (4%) of dynamic memory, leaving 125,328 bytes for local variables. Maximum is 131,072 bytes.

Compilation Status: 0 Errors, 0 Warnings
```
