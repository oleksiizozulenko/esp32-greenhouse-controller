# Actuators & Overrides Specification

## Purpose
Specifies physical hardware pin assignments, actuator driver behavior (ventilation servo, irrigation relay, light strip relay), status indicators (`LED_GREEN`, `LED_RED`), and debounced push-button manual overrides.

---

## Requirements

### Requirement: Physical GPIO Pin Assignment
The system hardware interfaces SHALL conform to the pin configuration defined in `config.h`.

#### Scenario: Hardware pin allocation
- **GIVEN** an ESP32 microcontroller board
- **WHEN** firmware pin constants are initialized
- **THEN** digital, analog, I2C, and PWM pins SHALL be assigned according to the pinout map:

| Component / Function | Config Constant | GPIO Pin | Interface Type |
|---|---|---|---|
| DHT22 Temp & Humidity | `PIN_DHT` / `PIN_TEMP` | `GPIO 19` | Digital 1-Wire |
| LDR Light Sensor | `PIN_LDR` | `GPIO 35` | Analog (ADC1_CH7) |
| Soil Moisture Sensor | `PIN_SOIL_POT` | `GPIO 34` | Analog (ADC1_CH6) |
| Mode Selector Button | `PIN_BTN_MODE` | `GPIO 12` | Digital Input (Pullup) |
| Irrigation Manual Button | `PIN_BTN_IRRIG` | `GPIO 14` | Digital Input (Pullup) |
| Ventilation Manual Button | `PIN_BTN_VENT` | `GPIO 27` | Digital Input (Pullup) |
| Light Manual Button | `PIN_BTN_LIGHT` | `GPIO 26` | Digital Input (Pullup) |
| Ventilation Servo PWM | `PIN_ACTUATOR_VENT` | `GPIO 5` | Servo PWM Output |
| Irrigation Relay | `PIN_ACTUATOR_IRRIG` | `GPIO 16` | Digital Relay Output |
| Growth Light Relay | `PIN_ACTUATOR_LIGHT` | `GPIO 17` | Digital Relay Output |
| Status LED Normal | `PIN_LED_GREEN` | `GPIO 15` | Digital Output |
| Status LED Error | `PIN_LED_RED` | `GPIO 4` | Digital Output |
| Acoustic Alert Buzzer | `PIN_BUZZER` | `GPIO 18` | PWM / Tone |
| OLED Display I2C SDA | `PIN_OLED_SDA` | `GPIO 21` | I2C Data |
| OLED Display I2C SCL | `PIN_OLED_SCL` | `GPIO 22` | I2C Clock |

---

### Requirement: Actuator Drivers & Positioning
Actuators SHALL execute state transitions based on commands from `GreenhouseController`.

#### Scenario: Ventilation window positioning
- **GIVEN** a ventilation command from the controller
- **WHEN** the window is commanded `OPEN`
- **THEN** `VentilationActuator` SHALL write a $90^\circ$ PWM angle to `GPIO 5`
- **WHEN** the window is commanded `CLOSED`
- **THEN** `VentilationActuator` SHALL write a $0^\circ$ PWM angle to `GPIO 5`

#### Scenario: Irrigation pump relay driving
- **GIVEN** an irrigation command from the controller
- **WHEN** irrigation is commanded `ACTIVE`
- **THEN** `IrrigationActuator` SHALL drive `GPIO 16` `HIGH`
- **WHEN** irrigation is commanded `INACTIVE`
- **THEN** `IrrigationActuator` SHALL drive `GPIO 16` `LOW`

#### Scenario: Supplemental light relay driving
- **GIVEN** a lighting command from the controller
- **WHEN** supplemental light is commanded `ACTIVE`
- **THEN** `LightActuator` SHALL drive `GPIO 17` `HIGH`
- **WHEN** supplemental light is commanded `INACTIVE`
- **THEN** `LightActuator` SHALL drive `GPIO 17` `LOW`

---

### Requirement: Hardware Push-Button Manual Overrides
In `SystemMode::MANUAL`, operators SHALL have direct hardware button override capability over actuators.

#### Scenario: Manual irrigation button press
- **GIVEN** the system is in `SystemMode::MANUAL`
- **WHEN** `PIN_BTN_IRRIG` is pressed and debounced
- **THEN** the system SHALL toggle the current state of the irrigation pump
- **AND** if turned `ON`, the system SHALL start a 10-second FreeRTOS safety auto-off timer (`IRRIGATION_TIMEOUT_MS`)

#### Scenario: Manual ventilation button press
- **GIVEN** the system is in `SystemMode::MANUAL`
- **WHEN** `PIN_BTN_VENT` is pressed and debounced
- **THEN** the system SHALL toggle the ventilation servo position between $0^\circ$ and $90^\circ$
- **AND** if turned `ON`, the system SHALL start a 30-second FreeRTOS safety auto-off timer (`VENTILATION_TIMEOUT_MS`)

#### Scenario: Manual light button press
- **GIVEN** the system is in `SystemMode::MANUAL`
- **WHEN** `PIN_BTN_LIGHT` is pressed and debounced
- **THEN** the system SHALL toggle the supplemental growth light state
- **AND** if turned `ON`, the system SHALL start a 60-second FreeRTOS safety auto-off timer (`LIGHT_TIMEOUT_MS`)

