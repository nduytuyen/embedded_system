```markdown
# STM32 Smart Irrigation System

**Version:** 2.1  
**Last Updated:** November 30, 2025  
**Platform:** STM32F103C8T6 (Blue Pill)

---

## 📋 Table of Contents

1. [Overview](#overview)
2. [Key Features](#key-features)
3. [Hardware Requirements](#hardware-requirements)
4. [Pin Configuration](#pin-configuration)
5. [System Architecture](#system-architecture)
6. [Operating Modes](#operating-modes)
7. [Getting Started](#getting-started)
8. [User Guide](#user-guide)
9. [Troubleshooting](#troubleshooting)
10. [API Reference](#api-reference)

---

## 🎯 Overview

An intelligent plant watering system built on STM32F103C8T6 microcontroller with multiple sensors and automated control modes. The system combines real-time monitoring, scheduled irrigation, and environmental sensing for optimal plant care.

### What's New in v2.1
- ✅ Added **DHT11 temperature & humidity sensor** support
- ✅ DWT/SysTick-based microsecond delays (no timer required)
- ✅ Improved sensor reliability with timeout protection
- ✅ Enhanced debug output via UART

---

## ✨ Key Features

### Sensors & Monitoring
- 🌡️ **DHT11** - Temperature and humidity monitoring
- 💧 **Soil Moisture Sensor** - Analog capacitive sensor (0-100%)
- 🕐 **DS3231 RTC** - Real-time clock with battery backup
- 📊 **16x2 LCD** - I2C display for status and menus

### Control Modes
- 🎮 **ARTISAN Mode** - Manual pump control
- 🤖 **AUTO Mode** - Moisture-triggered automatic watering
- ⏰ **TIMER Mode** - RTC-based scheduled watering

### Communication
- 📡 **UART Debug** - 115200 baud serial output
- 🔧 **I2C Bus** - RTC, LCD, and sensor communication

---

## 🔧 Hardware Requirements

### Components List

| Component | Part Number | Quantity | Notes |
|-----------|------------|----------|-------|
| **Microcontroller** | STM32F103C8T6 | 1 | Blue Pill board |
| **RTC Module** | DS3231 | 1 | I2C address: 0x68 |
| **LCD Display** | 1602 + PCF8574T | 1 | I2C address: 0x27 |
| **DHT11 Sensor** | DHT11 | 1 | Temperature & humidity |
| **Moisture Sensor** | Capacitive | 1 | Analog output |
| **Water Pump** | 5V DC | 1 | Mini submersible pump |
| **Relay Module** | 5V 1-Channel | 1 | With optocoupler |
| **Push Buttons** | Tactile 6x6mm | 6 | Active LOW |
| **Power Supply** | 5V/2A | 1 | Micro USB or DC jack |
| **Resistor** | 4.7kΩ | 1 | Pull-up for DHT11 (optional) |

### Wiring Diagram

```
┌─────────────────────────────────────────────────────────┐
│                   STM32F103C8T6 Connections             │
├─────────────────┬───────────┬───────────────────────────┤
│ Function        │ STM32 Pin │ Component Connection      │
├─────────────────┼───────────┼───────────────────────────┤
│ Moisture ADC    │ PA0       │ Sensor AO                 │
│ Button ARTISAN  │ PA1       │ Button → GND (pull-up)    │
│ Button AUTO     │ PA2       │ Button → GND (pull-up)    │
│ Button TIMER    │ PA3       │ Button → GND (pull-up)    │
│ Button RESET    │ PA4       │ Button → GND (pull-up)    │
│ Button INC      │ PA5       │ Button → GND (pull-up)    │
│ Button DEC      │ PA6       │ Button → GND (pull-up)    │
│ UART TX (Debug) │ PA9       │ USB-Serial RX             │
│ UART RX (Debug) │ PA10      │ USB-Serial TX             │
│ Pump Control    │ PA11      │ Relay IN                  │
│ I2C2 SCL        │ PB10      │ RTC SCL + LCD SCL         │
│ I2C2 SDA        │ PB11      │ RTC SDA + LCD SDA         │
│ DHT11 Data      │ PB6       │ DHT11 Data (configurable) │
└─────────────────┴───────────┴───────────────────────────┘

Power Distribution:
├─ STM32:           5V → 3.3V regulator onboard
├─ DHT11:           3.3V VCC, GND
├─ Moisture Sensor: 3.3V VCC, GND, AO → PA0
├─ DS3231:          3.3V VCC, GND, SCL → PB10, SDA → PB11
├─ LCD (PCF8574):   5V VCC, GND, SCL → PB10, SDA → PB11
└─ Pump Relay:      5V VCC, GND, Signal → PA11
```

---

## 📌 Pin Configuration

### GPIO Pin Map

```
/* Board Support Package Pin Definitions */

// ADC
#define MOISTURE_ADC_CHANNEL    ADC_CHANNEL_0  // PA0

// Buttons (Active LOW with internal pull-up)
#define BUTTON_ARTISAN_PIN      GPIO_PIN_1     // PA1
#define BUTTON_AUTO_PIN         GPIO_PIN_2     // PA2
#define BUTTON_TIMER_PIN        GPIO_PIN_3     // PA3
#define BUTTON_RESET_PIN        GPIO_PIN_4     // PA4
#define BUTTON_INC_PIN          GPIO_PIN_5     // PA5
#define BUTTON_DEC_PIN          GPIO_PIN_6     // PA6

// DHT11 Sensor
#define DHT11_GPIO_PORT         GPIOB
#define DHT11_GPIO_PIN          GPIO_PIN_6

// Pump Control
#define PUMP_PIN                GPIO_PIN_11    // PA11

// I2C2 (Hardware I2C)
#define I2C_SCL_PIN             GPIO_PIN_10    // PB10
#define I2C_SDA_PIN             GPIO_PIN_11    // PB11

// UART1 (Debug)
#define UART_TX_PIN             GPIO_PIN_9     // PA9
#define UART_RX_PIN             GPIO_PIN_10    // PA10
```

---

## 🏗️ System Architecture

### Layered Architecture

```
┌───────────────────────────────────────────────────────┐
│                 Application Layer (APP)               │
│  ┌─────────────────────────────────────────────────┐  │
│  │ -  State Machine (8 states)                      │  │
│  │ -  Mode Control (ARTISAN/AUTO/TIMER)             │  │
│  │ -  Schedule Management                           │  │
│  │ -  Sensor Data Integration                       │  │
│  └─────────────────────────────────────────────────┘  │
└─────────────────────────┬─────────────────────────────┘
                          │
┌─────────────────────────┴─────────────────────────────┐
│               Middleware Layer (MID)                  │
│  ┌─────────────────────────────────────────────────┐  │
│  │ -  Button Handler (debouncing, event detection) │  │
│  │ -  Display Manager (menu navigation)            │  │
│  │ -  RTC High-Level Operations                    │  │
│  └─────────────────────────────────────────────────┘  │
└─────────────────────────┬─────────────────────────────┘
                          │
┌─────────────────────────┴─────────────────────────────┐
│         Board Support Package Layer (BSP)             │
│  ┌─────────────────────────────────────────────────┐  │
│  │ -  LCD Driver (PCF8574T I2C)                     │  │
│  │ -  RTC Driver (DS3231)                           │  │
│  │ -  DHT11 Driver (Temperature & Humidity)         │  │
│  │ -  Moisture Sensor (ADC)                         │  │
│  │ -  Button Driver (GPIO)                          │  │
│  │ -  Pump Driver (GPIO)                            │  │
│  └─────────────────────────────────────────────────┘  │
└─────────────────────────┬─────────────────────────────┘
                          │
┌─────────────────────────┴─────────────────────────────┐
│          Hardware Abstraction Layer (HAL)             │
│  ┌─────────────────────────────────────────────────┐  │
│  │ -  STM32 HAL Library                             │  │
│  │ -  Peripherals: I2C, ADC, GPIO, UART, RTC       │  │
│  │ -  DWT Cycle Counter (microsecond timing)       │  │
│  └─────────────────────────────────────────────────┘  │
└───────────────────────────────────────────────────────┘
```

### File Structure

```
Irrigation_System/
│
├── Core/
│   ├── Src/
│   │   └── main.c                    # Main program entry
│   └── Inc/
│       └── main.h
│
├── App/
│   ├── app_irrigation.c              # State machine & logic
│   └── app_irrigation.h
│
├── Middleware/
│   ├── mid_button.c                  # Button handling
│   ├── mid_button.h
│   ├── mid_display.c                 # Display management
│   └── mid_display.h
│
├── BSP/
│   ├── bsp_dht11.c                   # DHT11 driver (new)
│   ├── bsp_dht11.h
│   ├── bsp_lcd.c                     # LCD I2C driver
│   ├── bsp_lcd.h
│   ├── bsp_rtc.c                     # DS3231 RTC driver
│   ├── bsp_rtc.h
│   ├── bsp_moisture.c                # Moisture sensor ADC
│   ├── bsp_moisture.h
│   ├── bsp_pump.c                    # Pump relay control
│   ├── bsp_pump.h
│   ├── bsp_button.c                  # Low-level button GPIO
│   └── bsp_button.h
│
└── README.md                         # This file
```

---

## 🎮 Operating Modes

### Mode 1: ARTISAN (Manual Control)

**Purpose:** Direct manual control of irrigation pump

**Behavior:**
- Pump runs continuously while mode is active
- Real-time moisture and environment display
- User manually stops via RESET button

**LCD Display:**
```
┌────────────────┐
│Mode: ARTISAN   │
│M:45% T:25C H:60│
└────────────────┘
```

**Use Cases:**
- Initial system testing
- Emergency watering
- Manual soil preparation
- Pump verification

---

### Mode 2: AUTO (Moisture-Based Automation)

**Purpose:** Automatic watering based on soil moisture levels

**Behavior:**
- Monitors soil moisture every 500ms
- Pump ON when moisture < 40%
- Pump OFF when moisture ≥ 50%
- 10% hysteresis prevents cycling

**LCD Display:**
```
┌────────────────┐
│Mode: AUTO      │
│M:38% P:ON  T:24│
└────────────────┘
```

**Thresholds:**
| Condition | Moisture Level | Action |
|-----------|---------------|---------|
| Dry Soil | < 40% | Pump ON |
| Optimal | 40% - 50% | No change |
| Wet Soil | ≥ 50% | Pump OFF |

**Use Cases:**
- Daily automatic care
- Vacation/away mode
- Greenhouse automation
- Consistent moisture maintenance

---

### Mode 3: TIMER (Schedule-Based Watering)

**Purpose:** Water plants at specific times daily

**Sub-Modes:**
1. **TIMER_DISPLAY** - Show current time
2. **TIMER_MENU** - Choose setup option
3. **TIMER_SET_TIME** - Configure RTC
4. **TIMER_SET_SCHEDULE** - Set watering schedule

**LCD Display (Running):**
```
┌────────────────┐
│Mode: TIMER     │
│Time: 14:30:25  │
└────────────────┘
```

**Schedule Configuration:**
- Start time: HH:MM (24-hour format)
- Duration: 1-99 minutes
- Daily automatic execution

**Example:**
```
Schedule: 06:30, Duration: 15 min
→ Pump activates at 6:30 AM
→ Pump deactivates at 6:45 AM
→ Repeats daily
```

**Use Cases:**
- Early morning watering
- Water conservation schedules
- Consistent plant routines
- Time-optimized irrigation

---

## 🚀 Getting Started

### 1. Hardware Assembly

```
Step 1: Connect I2C Devices
  ├─ DS3231 RTC → PB10 (SCL), PB11 (SDA)
  └─ LCD 1602   → PB10 (SCL), PB11 (SDA)

Step 2: Connect Sensors
  ├─ DHT11          → PB6 (Data), 3.3V, GND
  └─ Moisture       → PA0 (AO), 3.3V, GND

Step 3: Connect Buttons
  ├─ 6x buttons → PA1-PA6
  └─ Other side → GND (internal pull-ups enabled)

Step 4: Connect Pump Circuit
  ├─ PA11 → Relay Signal
  ├─ Relay COM → Pump +
  ├─ Pump - → GND
  └─ Relay VCC → 5V, GND

Step 5: Connect Debug UART
  ├─ PA9 (TX) → USB-Serial RX
  └─ GND      → USB-Serial GND
```

### 2. Software Setup

#### Prerequisites
- **STM32CubeIDE** v1.12 or later
- **STM32CubeMX** v6.8 or later
- **ST-Link V2** programmer
- **USB-Serial adapter** for debug output

#### Clock Configuration
```
// main.c - SystemClock_Config()
SYSCLK:      72 MHz (HSE 8MHz × PLL MUL9)
AHB:         72 MHz
APB1:        36 MHz (÷2)
APB2:        72 MHz
APB1 Timer:  72 MHz (auto ×2)
```

#### Build and Flash

```
# Clone repository
git clone <your-repo-url>
cd Irrigation_System

# Open in STM32CubeIDE
# Build: Ctrl + B
# Flash: Run → Debug (F11)

# Or use command line
make clean
make all
st-flash write build/Irrigation_System.bin 0x8000000
```

### 3. Initial Configuration

#### 3A. Set Current Time (First Boot)

```
1. Power ON → System shows "CHOOSE MODE"
2. Press [TIMER] button
3. Press [TIMER] again → TIMER MENU appears
4. Press [INC] until ">TIME" is highlighted
5. Press [TIMER] to enter SET TIME
6. Adjust hours:   [INC]/[DEC], then [TIMER]
7. Adjust minutes: [INC]/[DEC], then [TIMER]  
8. Adjust seconds: [INC]/[DEC], then [TIMER]
9. Time saved to DS3231 RTC
```

#### 3B. Calibrate Moisture Sensor

```
# Connect to UART (115200 baud)
# Monitor raw ADC values

# Sensor in DRY soil   → ADC ≈ 3800
# Sensor in WET soil   → ADC ≈ 1500

# Update in bsp_moisture.c:
#define MOISTURE_DRY_VALUE   3800  // Your dry reading
#define MOISTURE_WET_VALUE   1500  // Your wet reading
```

#### 3C. Configure Watering Schedule

```
1. From TIMER_DISPLAY, press [TIMER]
2. Press [DEC] to select ">SCHEDULE"
3. Press [TIMER] to enter
4. Set start hour:   [INC]/[DEC], then [TIMER]
5. Set start minute: [INC]/[DEC], then [TIMER]
6. Set duration:     [INC]/[DEC], then [TIMER]
7. Schedule saved → Automatic watering enabled
```

---

## 📖 User Guide

### Button Reference

```
Physical Layout:
┌─────────────────────────────────────┐
│  [ARTISAN]  [AUTO]  [TIMER]         │
│                                     │
│  [RESET]    [INC]   [DEC]           │
└─────────────────────────────────────┘
```

| Button | Function by Context |
|--------|-------------------|
| **ARTISAN** | Enter manual mode (from MENU) |
| **AUTO** | Enter automatic mode (from MENU) |
| **TIMER** | Enter/navigate timer mode (context-sensitive) |
| **RESET** | Return to MENU / Cancel operation |
| **INC** | Increase value / Navigate up |
| **DEC** | Decrease value / Navigate down |

### State Machine Flowchart

```
        [STARTUP]
            ↓
         [MENU] ←───────────────────┐
       /    |    \                  │
      /     |     \                 │
ARTISAN   AUTO   TIMER_DISPLAY     │
   ↓       ↓         ↓              │
  RESET   RESET   [TIMER] pressed  │
                      ↓              │
                 TIMER_MENU          │
                  /      \           │
          >TIME          >SCHEDULE   │
            ↓                ↓       │
    TIMER_SET_TIME   TIMER_SET_SCHEDULE
            ↓                ↓       │
    [Save/RESET]     [Save/RESET]   │
            └────────────┴───────────┘
```

### LCD Screen Reference

#### MENU Screen
```
┌────────────────┐
│  CHOOSE MODE   │  ← System idle
│ART/AUTO/TIMER  │  ← Press button
└────────────────┘
```

#### ARTISAN Screen
```
┌────────────────┐
│Mode: ARTISAN   │  ← Mode name
│M:45% T:25C H:60│  ← Live sensor data
└────────────────┘
 M=Moisture T=Temp H=Humidity
```

#### AUTO Screen  
```
┌────────────────┐
│Mode: AUTO      │  ← Mode name
│M:38% P:ON  T:24│  ← Moisture, Pump, Temp
└────────────────┘
```

#### TIMER Screens
```
Display Mode:
┌────────────────┐
│Mode: TIMER     │
│Time: 14:30:25  │  ← Updates every 1s
└────────────────┘

Menu Mode:
┌────────────────┐
│TIMER: INC/DEC  │  ← Instructions
│>TIME SCHEDULE  │  ← Selection cursor
└────────────────┘

Set Time Mode:
┌────────────────┐
│Set Time:       │
│ 14:30:25       │  ← Editable fields
└────────────────┘
  ↑↑  ↑↑  ↑↑
  HH  MM  SS

Set Schedule Mode:
┌────────────────┐
│Set Schedule:   │
│06:30 D:15m     │  ← Start time + Duration
└────────────────┘
```

### DHT11 Sensor Information

**Specifications:**
- Temperature: 0°C to 50°C (±2°C accuracy)
- Humidity: 20% to 90% RH (±5% accuracy)
- Sampling: Once every 2 seconds (hardware limitation)
- Communication: 1-wire digital protocol

**Reading Sensor Data:**
```
// In your code:
if (BSP_DHT11_Read()) {
    float temp = BSP_DHT11_GetTemperature();  // °C
    float hum = BSP_DHT11_GetHumidity();      // %RH
    printf("T:%.1f°C H:%.1f%%\n", temp, hum);
}
```

**Timing (DWT-based):**
- System automatically detects DWT availability
- Falls back to software delay if DWT unavailable
- No external timer required

---

## 🔧 Troubleshooting

### Issue 1: LCD Shows Nothing

**Symptoms:** Backlight ON, no text visible

**Solutions:**
1. **Adjust Contrast Potentiometer**
   ```
   Locate blue trim pot on PCF8574 module
   Rotate slowly until text appears
   ```

2. **Check I2C Address**
   ```
   // Try in bsp_lcd.h:
   #define LCD_I2C_ADDR  (0x3F << 1)  // Alternative address
   ```

3. **Verify Wiring**
   ```
   LCD VCC → 5V (NOT 3.3V)
   LCD GND → GND
   LCD SDA → PB11
   LCD SCL → PB10
   ```

---

### Issue 2: DHT11 Read Timeout

**Symptoms:** "ERROR: DHT11 no response" in UART

**Solutions:**
1. **Check Pull-up Resistor**
   ```
   DHT11 Data pin → 4.7kΩ → 3.3V
   (Or enable internal pull-up in code)
   ```

2. **Verify Pin Configuration**
   ```
   // In bsp_dht11.h:
   #define DHT11_GPIO_PORT  GPIOB
   #define DHT11_GPIO_PIN   GPIO_PIN_6  // Confirm correct pin
   ```

3. **Check Sensor Power**
   ```
   DHT11 VCC → 3.3V (stable power)
   DHT11 GND → GND
   Wait 1 second after power-on
   ```

4. **DWT Initialization**
   ```
   # Check UART output:
   "DHT11: Using DWT for microsecond delays"  ✓ Good
   "DHT11: Using software delay"              ⚠ Less accurate
   ```

---

### Issue 3: RTC Time Incorrect

**Symptoms:** Time resets, doesn't advance, or shows wrong time

**Solutions:**
1. **Check DS3231 Battery**
   ```
   Measure voltage on CR2032 battery
   Should be > 2.5V
   Replace if < 2.5V
   ```

2. **Verify I2C Communication**
   ```
   # Check UART output:
   "Device found at address 0x68"  ✓
   "DS3231 detected!"              ✓
   
   # If not found:
   - Check SDA/SCL connections
   - Try I2C bus scanner
   ```

3. **Re-initialize RTC**
   ```
   Enter TIMER → TIMER MENU → >TIME
   Set correct time
   Time persists after power cycle (with battery)
   ```

---

### Issue 4: Moisture Always 0% or 100%

**Symptoms:** Sensor readings stuck at extreme values

**Solutions:**
1. **Calibrate Sensor Range**
   ```
   // Monitor UART for raw ADC values
   // Then update in bsp_moisture.c:
   
   #define MOISTURE_DRY_VALUE   3800  // ADC when dry
   #define MOISTURE_WET_VALUE   1500  // ADC when wet
   
   // Percentage calculation:
   // 0% = DRY_VALUE
   // 100% = WET_VALUE
   ```

2. **Check Sensor Type**
   ```
   Capacitive sensor (recommended):
   - More reliable
   - Corrosion resistant
   - ADC range: ~1500-3800
   
   Resistive sensor:
   - May need different calibration
   - Prone to corrosion
   ```

3. **Test ADC**
   ```
   # In ARTISAN mode, check UART:
   "Moisture: 45%, ADC: 2650"
   
   # Verify ADC responds to moisture changes
   # If stuck at same value → hardware issue
   ```

---

### Issue 5: Buttons Not Responding

**Symptoms:** Button presses ignored

**Solutions:**
1. **Check Pull-up Configuration**
   ```
   // In main.c MX_GPIO_Init():
   GPIO_InitStruct.Pull = GPIO_PULLUP;  // Must be enabled
   ```

2. **Verify Active-LOW Logic**
   ```
   // In bsp_button.c:
   // Pressed = GPIO_PIN_RESET (LOW)
   // Released = GPIO_PIN_SET (HIGH)
   ```

3. **Test Individual Buttons**
   ```
   # Monitor UART output:
   "Button: ARTISAN pressed"  ✓
   "Button: AUTO pressed"     ✓
   # etc.
   
   # If no output → hardware connection issue
   ```

4. **Check Debounce Timing**
   ```
   // In bsp_button.c:
   #define DEBOUNCE_TIME_MS  50  // Adjust if needed
   ```

---

### Issue 6: Pump Doesn't Activate

**Symptoms:** AUTO/ARTISAN modes don't turn pump ON

**Solutions:**
1. **Verify Relay Operation**
   ```
   # Check UART:
   "Pump: ON"   → PA11 should be HIGH
   "Pump: OFF"  → PA11 should be LOW
   
   # Measure with multimeter:
   PA11 HIGH = 3.3V
   PA11 LOW = 0V
   ```

2. **Check Relay Circuit**
   ```
   STM32 PA11 → 1kΩ resistor → PC817 LED+
   PC817 LED- → GND
   PC817 OUT+ → Relay IN
   PC817 OUT- → GND
   Relay VCC → 5V
   Flyback diode across relay coil
   ```

3. **Test Pump Manually**
   ```
   Connect pump directly to 5V
   Should run → If not, pump is faulty
   ```

---

### Debug via UART

**Setup:**
```
STM32 PA9 (TX) → USB-Serial RX
STM32 GND      → USB-Serial GND

Baud rate: 115200
Data bits: 8
Parity: None
Stop bits: 1
```

**Typical Boot Output:**
```
=================================
STM32 Irrigation System v2.1
=================================
Scanning I2C bus...
Device found at address 0x27  (LCD)
Device found at address 0x57  (DS3231 EEPROM)
Device found at address 0x68  (DS3231 RTC)
Total devices found: 3

DS3231 detected!
RTC initialized successfully.

DHT11: Using DWT for microsecond delays
DHT11 initialized successfully
NOTE: No external timer required

Moisture sensor initialized.
Pump initialized.
Buttons initialized.
Display initialized.

System initialized.
=================================

[14:30:25] State: 1, Moisture: 67%, Pump: OFF
DHT11: Temp=25.0°C, Humidity=60.0%
```

---

## 📚 API Reference

### BSP Layer APIs

#### DHT11 Sensor (bsp_dht11.h)

```
// Initialize DHT11 sensor
bool BSP_DHT11_Init(void);

// Read temperature and humidity (call every 2+ seconds)
bool BSP_DHT11_Read(void);

// Get last temperature reading (°C)
float BSP_DHT11_GetTemperature(void);

// Get last humidity reading (%RH)
float BSP_DHT11_GetHumidity(void);

// Check if sensor is ready
bool BSP_DHT11_IsReady(void);
```

#### Moisture Sensor (bsp_moisture.h)

```
// Initialize ADC for moisture sensor
bool BSP_Moisture_Init(ADC_HandleTypeDef *hadc);

// Read moisture percentage (0-100%)
uint8_t BSP_Moisture_GetPercent(void);

// Get raw ADC value (0-4095)
uint16_t BSP_Moisture_GetRaw(void);
```

#### RTC (bsp_rtc.h)

```
// Initialize DS3231 RTC
bool BSP_RTC_Init(I2C_HandleTypeDef *hi2c);

// Get current time
void BSP_RTC_GetTime(RTC_Time_t *time);

// Set current time
void BSP_RTC_SetTime(RTC_Time_t *time);

// Check if RTC is available
bool BSP_RTC_IsAvailable(void);
```

#### Pump Control (bsp_pump.h)

```
// Initialize pump GPIO
void BSP_Pump_Init(void);

// Turn pump ON
void BSP_Pump_On(void);

// Turn pump OFF
void BSP_Pump_Off(void);

// Get pump state (true=ON, false=OFF)
bool BSP_Pump_GetState(void);
```

#### LCD Display (bsp_lcd.h)

```
// Initialize I2C LCD
void BSP_LCD_Init(I2C_HandleTypeDef *hi2c);

// Clear display
void BSP_LCD_Clear(void);

// Set cursor position
void BSP_LCD_SetCursor(uint8_t row, uint8_t col);

// Send string to LCD
void BSP_LCD_SendString(const char *str);

// Backlight control
void BSP_LCD_Backlight(bool on);
```

### Middleware Layer APIs

#### Button Handler (mid_button.h)

```
// Initialize button module
void MID_Button_Init(void);

// Update button states (call in main loop)
void MID_Button_Update(void);

// Check if button is pressed
bool MID_Button_IsPressed(Button_t button);

// Button types:
typedef enum {
    BUTTON_ARTISAN,
    BUTTON_AUTO,
    BUTTON_TIMER,
    BUTTON_RESET,
    BUTTON_INC,
    BUTTON_DEC
} Button_t;
```

#### Display Manager (mid_display.h)

```
// Initialize display manager
void MID_Display_Init(I2C_HandleTypeDef *hi2c);

// Show main menu
void MID_Display_ShowMenu(void);

// Show ARTISAN mode screen
void MID_Display_ShowArtisan(uint8_t moisture);

// Show AUTO mode screen
void MID_Display_ShowAuto(uint8_t moisture, bool pumpOn);

// Show TIMER mode screen
void MID_Display_ShowTime(RTC_Time_t *time);

// Clear display
void MID_Display_Clear(void);
```

### Application Layer APIs

#### Main Application (app_irrigation.h)

```
// Initialize application
void APP_Irrigation_Init(ADC_HandleTypeDef *hadc, 
                        I2C_HandleTypeDef *hi2c,
                        UART_HandleTypeDef *huart);

// Run state machine (call in main loop)
void APP_Irrigation_Run(void);

// Get current system state
SystemState_t APP_Irrigation_GetState(void);

// System states:
typedef enum {
    STATE_STARTUP,
    STATE_MENU,
    STATE_ARTISAN,
    STATE_AUTO,
    STATE_TIMER_DISPLAY,
    STATE_TIMER_MENU,
    STATE_TIMER_SET_TIME,
    STATE_TIMER_SET_SCHEDULE
} SystemState_t;
```

---

## ⚙️ Configuration

### Moisture Sensor Calibration

Edit `bsp_moisture.c`:

```
// Adjust these values based on your sensor
#define MOISTURE_DRY_VALUE   3800  // ADC value in dry soil
#define MOISTURE_WET_VALUE   1500  // ADC value in wet soil
```

### AUTO Mode Thresholds

Edit `app_irrigation.c`:

```
// Moisture thresholds for AUTO mode
#define AUTO_MOISTURE_LOW_THRESHOLD   40  // Turn pump ON below this
#define AUTO_MOISTURE_HIGH_THRESHOLD  50  // Turn pump OFF above this
```

### DHT11 Pin Configuration

Edit `bsp_dht11.h`:

```
// Change if using different GPIO pin
#define DHT11_GPIO_PORT  GPIOB
#define DHT11_GPIO_PIN   GPIO_PIN_6
#define DHT11_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOB_CLK_ENABLE()
```

### I2C Addresses

Edit `bsp_lcd.c` and `bsp_rtc.c`:

```
// LCD address (try 0x27 or 0x3F)
#define LCD_I2C_ADDR   (0x27 << 1)

// RTC address (standard)
#define DS3231_ADDRESS  0x68
```

---

## 📊 Performance Specifications

| Metric | Value | Requirement |
|--------|-------|-------------|
| System Boot Time | < 500ms | ✓ Met |
| Button Response | < 100ms | ✓ Met |
| Moisture Read Rate | 2 Hz (500ms) | ✓ Met |
| DHT11 Read Rate | 0.5 Hz (2s) | ✓ Hardware limit |
| RTC Update | 1 Hz (1s) | ✓ Met |
| Display Refresh | 2 Hz (500ms) | ✓ Met |
| Continuous Operation | >100 hours tested | ✓ Met |

### Power Consumption

| Component | Current | Notes |
|-----------|---------|-------|
| STM32F103 | ~50mA | Active @72MHz |
| DS3231 RTC | 200µA | + Battery backup |
| LCD Backlight | ~40mA | Can be disabled |
| DHT11 | ~2.5mA | During measurement |
| Moisture Sensor | ~5mA | Continuous |
| Pump (OFF) | 0mA | Relay idle |
| Pump (ON) | ~500mA | Separate supply |
| **Total (Idle)** | **~100mA** | 5V @ 0.5W |
| **Total (Pumping)** | **~600mA** | 5V @ 3W |

---

## 🔐 Safety & Compliance

### Electrical Safety
- ✅ Optocoupler isolation (PC817) between MCU and relay
- ✅ Flyback diode protection across relay coil
- ✅ Separate power domains for logic and pump
- ✅ Current limiting resistors on all GPIO outputs

### Software Protections
- ✅ Watchdog timer (optional, can be enabled)
- ✅ Button debouncing (50ms)
- ✅ Sensor timeout protection (200 iterations)
- ✅ Invalid state detection and recovery
- ✅ Pump auto-stop in AUTO/TIMER modes

### Water Safety
- ⚠️ **Do not submerge electronics in water**
- ⚠️ **Use waterproof enclosure for outdoor installation**
- ⚠️ **Ensure pump has dry-run protection**
- ⚠️ **Test system before unattended operation**

---

## 📝 Maintenance Schedule

### Weekly
- [ ] Check moisture sensor for corrosion/buildup
- [ ] Verify pump operation
- [ ] Inspect wiring connections
- [ ] Check RTC time accuracy

### Monthly
- [ ] Clean moisture sensor probes
- [ ] Test all operating modes
- [ ] Verify AUTO mode thresholds
- [ ] Check relay operation

### Yearly
- [ ] Replace DS3231 battery (CR2032)
- [ ] Clean pump filter/impeller
- [ ] Re-calibrate moisture sensor
- [ ] Update firmware if available

---

## 🤝 Contributing

Contributions welcome! Please:
1. Fork the repository
2. Create feature branch (`git checkout -b feature/amazing-feature`)
3. Commit changes (`git commit -m 'Add amazing feature'`)
4. Push to branch (`git push origin feature/amazing-feature`)
5. Open Pull Request

---

## 📄 License

This project is licensed under the MIT License - see LICENSE file for details.

---

## 📞 Support & Contact

- **Issues**: Report bugs via GitHub Issues
- **Documentation**: Full SRS available in `/docs` folder
- **Hardware Files**: Schematics and PCB files in `/hardware` folder

---

## 🙏 Acknowledgments

- STMicroelectronics for HAL library
- ControllersTech for DHT11 tutorial reference
- Community contributors

---

**Project Status:** ✅ Stable (v2.1)  
**Last Tested:** November 30, 2025  
**Tested On:** STM32F103C8T6 (Blue Pill)

---

*For detailed technical specifications, refer to the Software Requirements Specification (SRS) document.*
```