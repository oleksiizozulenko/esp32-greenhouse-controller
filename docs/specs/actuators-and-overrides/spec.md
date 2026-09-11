# Actuators & Overrides Specification

## Purpose
Specifies physical hardware pin assignments, actuator driver behavior (ventilation servo, 8x8 matrix irrigation, yellow LED light), status indicators (`LED_GREEN`, `LED_RED`, `BUZZER`), and debounced push-button manual overrides across system operating modes.

---

## Requirements

### Requirement: Physical GPIO Pin Assignment
The system hardware interfaces SHALL conform to the pin configuration defined in `config.h`.

#### Scenario: Hardware pin allocation
- **GIVEN** an ESP32 microcontroller board
- **WHEN** firmware pin constants are initialized
- **THEN** digital, analog, I2C, and PWM pins SHALL be assigned according to the pinout map:

| Component / Function | Config Constant | GPIO Pin | Interface Type / Logic |
|---|---|---|---|
| DHT11 Temp & Humidity | `PIN_DHT` | `GPIO 23` | Digital Single-Bus |
| LDR Light Sensor | `PIN_LDR` | `GPIO 35` | Analog (ADC1_CH7, 0..3.3V) |
| Soil Moisture Potentiometer | `PIN_SOIL_POT` | `GPIO 34` | Analog (ADC1_CH6, 0..3.3V) |
| Mode Selector Button | `PIN_BTN_MODE` | `GPIO 32` | Digital Input (Pullup, Active-LOW) |
| Irrigation Manual Button | `PIN_BTN_IRRIG` | `GPIO 14` | Digital Input (Pullup, Active-LOW) |
| Ventilation Manual Button | `PIN_BTN_VENT` | `GPIO 27` | Digital Input (Pullup, Active-LOW) |
| Light Manual Button | `PIN_BTN_LIGHT` | `GPIO 26` | Digital Input (Pullup, Active-LOW) |
| Ventilation Servo PWM | `PIN_ACTUATOR_VENT` | `GPIO 13` | 50Hz PWM Output (0°..90°) |
| Irrigation 8x8 Dot Matrix | `PIN_ACTUATOR_IRRIG` | `GPIO 33` | Digital Output (**Active-LOW**) |
| Growth Light Yellow LED | `PIN_ACTUATOR_LIGHT` | `GPIO 25` | Digital Output (Active-HIGH) |
| Status LED Normal | `PIN_LED_GREEN` | `GPIO 15` | Digital Output (Constant ON) |
| Status LED Error | `PIN_LED_RED` | `GPIO 4` | Digital Output (Active on Alert/Hazard) |
| Acoustic Alert Buzzer | `PIN_BUZZER` | `GPIO 18` | PWM / Tone (1kHz on Alarm) |
| OLED Display I2C SDA | `PIN_OLED_SDA` | `GPIO 21` | I2C Data (Address 0x3C) |
| OLED Display I2C SCL | `PIN_OLED_SCL` | `GPIO 22` | I2C Clock (Address 0x3C) |

---

### Requirement: Actuator Drivers & Positioning
Actuators SHALL execute state transitions based on commands from `GreenhouseController` conforming to their respective electrical polarities.

#### Scenario: Ventilation window positioning
- **GIVEN** a ventilation command from `GreenhouseController`
- **WHEN** the window is commanded `OPEN`
- **THEN** `ServoVentilationActuator` SHALL write a $90^\circ$ PWM pulse to `GPIO 13` (`PIN_ACTUATOR_VENT`)
- **WHEN** the window is commanded `CLOSED`
- **THEN** `ServoVentilationActuator` SHALL write a $0^\circ$ PWM pulse to `GPIO 13` (`PIN_ACTUATOR_VENT`)

#### Scenario: Irrigation 8x8 matrix active-LOW driving
- **GIVEN** an irrigation command from `GreenhouseController`
- **WHEN** irrigation is commanded `ACTIVE`
- **THEN** `DotMatrix8x8IrrigationActuator` SHALL drive `GPIO 33` (`PIN_ACTUATOR_IRRIG`) `LOW`
- **WHEN** irrigation is commanded `INACTIVE`
- **THEN** `DotMatrix8x8IrrigationActuator` SHALL drive `GPIO 33` (`PIN_ACTUATOR_IRRIG`) `HIGH`

#### Scenario: Supplemental yellow LED light driving
- **GIVEN** a lighting command from `GreenhouseController`
- **WHEN** supplemental light is commanded `ACTIVE`
- **THEN** `YellowLedLightActuator` SHALL drive `GPIO 25` (`PIN_ACTUATOR_LIGHT`) `HIGH`
- **WHEN** supplemental light is commanded `INACTIVE`
- **THEN** `YellowLedLightActuator` SHALL drive `GPIO 25` (`PIN_ACTUATOR_LIGHT`) `LOW`

---

### Requirement: Hardware Push-Button Manual Overrides
The system SHALL support push-button manual actuator overrides with mode-specific execution lifecycles and microsecond interrupt debouncing.

#### Scenario: Manual override activation while in AUTOMATIC mode
- **GIVEN** the global system is in `SystemMode::AUTOMATIC`
- **AND** a target actuator is currently inactive/OFF
- **WHEN** the associated button (`PIN_BTN_IRRIG`, `PIN_BTN_VENT`, or `PIN_BTN_LIGHT`) is pressed
- **THEN** `GreenhouseController` SHALL set the target subsystem to `ControlMode::MANUAL` and `ManualState::ON`
- **AND** `GreenhouseController` SHALL turn the actuator `ON`
- **AND** `GreenhouseController` SHALL start a one-shot FreeRTOS safety auto-off timer (10s for Irrigation, 30s for Vent, 60s for Light)

#### Scenario: Manual override deactivation while in AUTOMATIC mode
- **GIVEN** the global system is in `SystemMode::AUTOMATIC`
- **AND** a target actuator is currently active/ON (via auto rule or prior manual override)
- **WHEN** the associated button is pressed
- **THEN** `GreenhouseController` SHALL set the target subsystem to `ControlMode::MANUAL` and `ManualState::OFF`
- **AND** `GreenhouseController` SHALL turn the actuator `OFF`
- **AND** `GreenhouseController` SHALL stop any running safety timer for that actuator
- **AND** automatic rules SHALL NOT turn the actuator back ON until the button is toggled again or global mode is cycled

#### Scenario: Manual toggle while in MANUAL mode
- **GIVEN** the global system is in `SystemMode::MANUAL`
- **WHEN** an actuator button is pressed
- **THEN** `GreenhouseController` SHALL toggle the target subsystem's `manualState` between `ON` and `OFF`
- **AND** the actuator SHALL turn `ON` or `OFF` accordingly
- **AND** NO safety auto-off timer SHALL be armed (actuator operates indefinitely until toggled off, subject to critical overwater safety)

#### Scenario: Microsecond interrupt debounce & dispatch
- **GIVEN** an active-LOW button falling edge on any button GPIO
- **WHEN** `ButtonDriver::isrHandler` executes
- **THEN** the ISR SHALL query `esp_timer_get_time() / 1000ULL` to verify the 50ms debounce threshold
- **AND** on a valid edge, the ISR SHALL post a `ButtonEvent` to `buttonEventQueue` via `xQueueSendFromISR`
- **AND** the ISR SHALL set `EVENT_BIT_BUTTON_EVENT` in `systemEventGroup` via `xEventGroupSetBitsFromISR`


