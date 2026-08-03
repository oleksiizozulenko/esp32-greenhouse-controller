# Greenhouse Controller Unit Testing

This directory contains host-native unit tests for the ESP32 Greenhouse Controller logic built using [PlatformIO Test Runner](https://docs.platformio.org/en/latest/advanced/unit-testing/index.html) and the **Unity** testing framework.

Tests execute natively on your development machine (macOS/Linux/Windows) in seconds without requiring an attached physical ESP32 board.

---

## 🚀 Running the Tests

To build and run all native unit test suites, execute:

```bash
pio test -e native
```

To run a specific test suite (e.g., `test_automation_service`):

```bash
pio test -e native -f test_automation_service
```

---

## 📁 Directory Structure & Mock Infrastructure

```
test/
├── Arduino.h                 # Mock header for Arduino hardware calls (pin IO, tone, millis, Serial)
├── ArduinoMock.cpp           # Global MockSerial instance definition
├── MockActuator.h            # Test double for Actuator interface
├── MockSensor.h              # Test double for Sensor interface
├── test_automation_service/
│   └── test_automation_service.cpp   # AutomationService logic, hysteresis, manual overrides & alerts
├── test_sensors_service/
│   └── test_sensors_service.cpp      # SensorsService & SensorDataMap memory/data operations
└── test_drivers/
    └── test_drivers.cpp             # Driver helpers (ADC conversion formulas, status text)
```

---

## 🧪 Comprehensive Catalog of Covered Test Cases

### 1. Automation Service Logic (`test_automation_service.cpp`)

#### 🌡️ Temperature & Ventilation Control (`TEMP_THRESHOLD_HIGH = 28.0°C`, `TEMP_HYSTERESIS = 2.0°C`)
1. **High Temp Opening**: `temp > 28.0°C` opens ventilation (`vent->turnOn()`) and activates high-alert alarm.
2. **Hysteresis Upper Band**: Temperature at `27.0°C` while ventilation is OPEN $\rightarrow$ Ventilation **remains OPEN** to prevent rapid oscillation.
3. **Cool Temp Closing**: `temp < 26.0°C` (`28.0°C - 2.0°C`) closes ventilation (`vent->turnOff()`).
4. **Hysteresis Lower Band**: Temperature at `27.0°C` while ventilation is CLOSED $\rightarrow$ Ventilation **remains CLOSED**.
5. **Sensor Error Protection**: Temperature sensor reporting `isError = true` preserves state without false triggers.

#### 💧 Soil Moisture & Irrigation Control (`SOIL_DRY_THRESHOLD = 30%`, `SOIL_HYSTERESIS = 5%`)
6. **Dry Soil Activation**: Soil moisture `< 30%` turns **ON** irrigation (`irrig->turnOn()`) and activates high-alert alarm.
7. **Hysteresis Watering Band**: Soil moisture at `33%` while irrigation is ON $\rightarrow$ Irrigation **remains ON**.
8. **Moist Soil Deactivation**: Soil moisture `> 35%` (`30% + 5%`) turns **OFF** irrigation (`irrig->turnOff()`).
9. **Hysteresis Drying Band**: Soil moisture drops to `33%` while irrigation is OFF $\rightarrow$ Irrigation **remains OFF**.
10. **Sensor Error Protection**: Soil sensor reporting `isError = true` preserves irrigation state.

#### ☀️ Ambient Light & Lighting Control (`LIGHT_DARK_THRESHOLD = 500`, `LIGHT_HYSTERESIS = 50`)
11. **Darkness Activation**: Light level `< 500` turns **ON** supplemental light (`light->turnOn()`).
12. **Hysteresis Lighting Band**: Light level at `525` while Light is ON $\rightarrow$ Light **remains ON**.
13. **Daylight Deactivation**: Light level `> 550` (`500 + 50`) turns **OFF** supplemental light (`light->turnOff()`).
14. **Hysteresis Brightening Band**: Light level drops to `525` while Light is OFF $\rightarrow$ Light **remains OFF**.
15. **Sensor Error Protection**: Light sensor reporting `isError = true` preserves light state.

#### 🔘 Manual Override Mode & Debouncing
16. **Manual Button Toggles**: Pressing Irrigation, Ventilation, or Light buttons in manual mode correctly toggles state (OFF $\rightarrow$ ON $\rightarrow$ OFF) with debouncing.
17. **Null Driver Safety**: Passing `nullptr` for button drivers does not cause crashes or undefined behavior.

#### 🚨 System Indicators & Alarms
18. **Normal Status Indicator**: All sensors healthy (`isError = false`) $\rightarrow$ GREEN LED = HIGH, RED LED = LOW.
19. **System Fault Indicator**: Any sensor in error state (`isError = true`) $\rightarrow$ RED LED = HIGH, GREEN LED = LOW.
20. **High Alert Buzzer Alarm**: Temperature `> 28°C` or Soil `< 30%` triggers 1kHz notification tone (`tone()`).

---

### 2. Sensors Service & Data Map Logic (`test_sensors_service.cpp`)

21. **Dynamic Registration & Indexing**: Adding sensors dynamically expands array capacity and updates `getSensorCount()`.
22. **Safe Index Retrieval**: `getSensor(index)` returns valid sensor pointers or `nullptr` for out-of-bounds indices.
23. **Null Sensor Handling**: `addSensor(nullptr)` returns `false` gracefully without corrupting list.
24. **Batch Read All (`readAll`)**: Queries all registered sensors and compiles an accurate `SensorDataMap`.
25. **Key Lookup by Name**: `SensorDataMap::get("Temperature")` retrieves data by sensor name; unknown keys return `{0.0f, isError: true}`.
26. **Pointer Lookup**: `SensorDataMap::get(sensorPtr)` retrieves data matching exact sensor pointer.
27. **Copy & Move Semantics**: Verified copy constructor, copy assignment, move constructor, and move assignment for `SensorDataMap` to ensure memory safety without dangling pointers or double frees.

---

### 3. Drivers & Helpers (`test_drivers.cpp`)

28. **ADC Conversions**: Validated `Sensor::adcToVoltage` (0 to 3.3V mapping) and `Sensor::adcToPercentage` (0 to 100% mapping) across min, max, and mid-range raw ADC values.
29. **Actuator Status Text**: Verified `getStatusText()` returns `"ON"` when active and `"OFF"` when inactive.
