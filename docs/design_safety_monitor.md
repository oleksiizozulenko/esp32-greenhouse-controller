# Design Document: Safety Monitor & GreenhouseController Architecture

## 1. Overview & Purpose
This document specifies the architectural refactoring of the former automation logic into `GreenhouseController` (`include/GreenhouseController.h`), the introduction of `SafetyMonitorService` (`include/services/SafetyMonitorService.h`), the update of sensor and actuator drivers to use `SensorType`/`ActuatorType` identities, and the creation of a decoupled View Renderer `DisplayManager` (`include/services/DisplayManager.h`) consuming `DisplayViewModel` (`include/ui/DisplayViewModel.h`).

---

## 2. System Architecture & Ownership Model

`main.cpp` serves as the Application Composition Root:

```
                                  +--------------------+
                                  |      main.cpp      |
                                  +---------+----------+
                                            |
         +--------------------+-------------+--------------+--------------------+
         |                    |                            |                    |
         v                    v                            v                    v
  SensorsService    SafetyMonitorService         GreenhouseController     DisplayManager
```

1. **`SensorsService`**: Reads hardware pins and builds `SensorDataMap`.
2. **`SafetyMonitorService`**: Evaluates `SensorDataMap` against domain bounds using `SensorType`-based lookups and produces `SystemHealthState`.
3. **`GreenhouseController`**: Accepts `readings`, `healthState`, and button states, handles actuator logic (AUTO) or manual button toggles (MANUAL), and constructs `DisplayViewModel`.
4. **`DisplayManager`**: Takes `DisplayViewModel` and draws pixel primitives on SSD1306 OLED.

---

## 3. Light Sensor 3.3V ESP32 ADC-to-Lux Math & Wiring (`LightSensor.h`)

### 3.1 Wiring & Electrical Equation
In `diagram.json`, `ldr1:VCC` is wired to ESP32 `3V3`, `ldr1:SIG` to pin `35` (12-bit ADC, $V_{ref} = 3.3\text{V}$), and `ldr1:GND` to `GND`. The onboard $10\,\text{k}\Omega$ pull-down resistor forms a voltage divider powered from $V_{CC} = 3.3\text{V}$:

$$R_{LDR} = 10000 \times \left(\frac{4095.0}{\text{rawAdc}} - 1.0\right)$$

$$\text{Lux} = \left(\frac{250593.5}{R_{LDR}}\right)^{\frac{1}{0.7}}$$

### 3.2 Wokwi 3.3V ESP32 Reference Test Points
- **$\text{rawADC} = 2050$** $\rightarrow R_{LDR} \approx 9.97\,\text{k}\Omega \rightarrow \mathbf{100\,\text{lx}}$
- **$\text{rawADC} = 2971$** $\rightarrow R_{LDR} \approx 3.78\,\text{k}\Omega \rightarrow \mathbf{400\,\text{lx}}$
- **$\text{rawADC} = 3093$** $\rightarrow R_{LDR} \approx 3.24\,\text{k}\Omega \rightarrow \mathbf{500\,\text{lx}}$ (Dark Threshold)
- **$\text{rawADC} = 3938$** $\rightarrow R_{LDR} \approx 397\,\Omega \rightarrow \mathbf{10000\,\text{lx}}$ (Critical Daylight)

### 3.3 Option A Hardware Boundary Ownership Rule
- **`LightSensor::read()` (Driver Level)**: Returns `{lux, isErr}` where `isErr` is `true` **only** for mathematical invalidity (`!isfinite(lux)` or division by zero when `rawADC == 0`). Driver does **not** evaluate `SENSOR_LIGHT_MIN_ERROR` or `SENSOR_LIGHT_MAX_ERROR`.
- **`SafetyMonitorService` (Domain Level)**: Owns all domain boundary checks (`lux < SENSOR_LIGHT_MIN_ERROR` or `lux > SENSOR_LIGHT_MAX_ERROR`).
- **Driver boundary rule**: drivers report read validity only (`isErr` for NaN/inf/math failure); domain-range validation stays in `SafetyMonitorService`.

---

## 4. Boundary & Threshold Specification (`config.h`)

```cpp
// Temperature Bounds (°C)
#define SENSOR_TEMP_MIN_ERROR     -5.0f   // °C (Exclusive lower: < -5.0)
#define SENSOR_TEMP_MAX_ERROR     60.0f   // °C (Exclusive upper: > 60.0)

// Humidity Bounds (%)
#define SENSOR_HUMIDITY_MIN_ERROR 0.0f    // % (Inclusive valid: 0.0 .. 90.0)
#define SENSOR_HUMIDITY_MAX_ERROR 90.0f   // % (DHT saturation: > 90.0)

// Soil Moisture Bounds (%)
#define SENSOR_SOIL_MIN_ERROR     0.0f    // % (Inclusive valid: 0.0 .. 100.0)
#define SENSOR_SOIL_MAX_ERROR     100.0f  // %

// Light Sensor Bounds (Lux)
#define SENSOR_LIGHT_MIN_ERROR    0.0f        // lx (Inclusive valid)
#define SENSOR_LIGHT_MAX_ERROR    100000.0f   // lx (Inclusive valid)
#define LIGHT_DARK_THRESHOLD      500.0f      // lx (Turn ON light if < 500 lx)
#define LIGHT_HYSTERESIS          50.0f       // lx (Turn OFF light if > 550 lx)
#define CRITICAL_LIGHT_HIGH       10000.0f    // lx (Full daylight / High Light)

// Physical Hazard Limits
#define CRITICAL_TEMP_HIGH        45.0f   // °C (Overheat)
#define CRITICAL_TEMP_LOW         5.0f    // °C (Frost)
#define CRITICAL_HUMIDITY_HIGH    85.0f   // % (High Humidity)
#define CRITICAL_SOIL_HIGH        85.0f   // % (Soil Flood)
#define CRITICAL_SOIL_LOW         30.0f   // % (Dry Soil)
```

---

## 5. `SystemHealthState` & Deterministic Priority Matrix

### 5.1 Struct Definition
```cpp
struct SystemHealthState {
    bool hasHardwareError;     // True if any present sensor is NaN/inf/out-of-bounds
    bool hasCriticalHazard;    // True if Overheat (>45°C), Frost (<5°C), or Soil Flood (>85%)
    bool hasOperatorAdvisory;  // True if Dry Soil (<30%), High Humidity (>85%), or High Light (>10000lx)
    bool requiresAlarm;        // True if Buzzer 1kHz alarm tone should sound
    char advisoryMsg[24];      // Prioritized prompt banner for MANUAL mode (max 23 chars + null)
};
```

### 5.2 Deterministic Priority Matrix

| Priority | Condition | `hasHardwareError` | `hasCriticalHazard` | `hasOperatorAdvisory` | `requiresAlarm` | `advisoryMsg` (MANUAL Mode) |
|---|---|---|---|---|---|---|
| 1 (Highest) | Hardware Error / NaN / Inf / Domain Error | `true` | `false` | `false` | `true` | `"SENSOR ERROR!"` |
| 2 | Temp > 45°C | `false` | `true` | `false` | `true` | `"TEMP HIGH! Press VENT"` |
| 3 | Temp < 5°C | `false` | `true` | `false` | `true` | `"FROST RISK! Temp Low"` |
| 4 | Soil > 85% | `false` | `true` | `false` | `true` | `"SOIL FLOOD! Stop Water"` |
| 5 | Humidity > 85% | `false` | `false` | `true` | `true` (Alarm active) | `"HUMID HIGH! Press VENT"` |
| 6 | Soil < 30% | `false` | `false` | `true` | `isAutoMode` | `"SOIL DRY! Press IRRIG"` |
| 7 | Light > 10000 lx | `false` | `false` | `true` | `false` | `"LIGHT HIGH! Press LIGHT"` |
| 8 (Lowest) | Normal | `false` | `false` | `false` | `false` | `""` |

---

## 6. UI View Model (`include/ui/DisplayViewModel.h`)

```cpp
#ifndef DISPLAY_VIEW_MODEL_H
#define DISPLAY_VIEW_MODEL_H

#include <Arduino.h>

struct DisplayViewModel {
    char modeText[8];       // "AUTO" or "MANUAL"
    char healthStatus[8];   // "[OK]" or "[ERR]"
    
    struct LineItem {
        char label[8];      // "Light:"
        char value[10];     // "450lx" or "ERR"
    };

    LineItem sensors[4];
    size_t sensorCount;

    LineItem actuators[4];
    size_t actuatorCount;

    char advisoryBanner[24]; // Max 23 chars + null (safely fits "TEMP HIGH! Press VENT")
};

#endif // DISPLAY_VIEW_MODEL_H
```
