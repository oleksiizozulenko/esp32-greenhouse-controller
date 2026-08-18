# Climate Control Specification

## Purpose
Specifies autonomous microclimate regulation, target environmental thresholds, hysteresis algorithm behavior, and operating mode semantics for the ESP32 Greenhouse Controller.

---

## Requirements

### Requirement: Autonomous Climate Regulation
The controller SHALL continuously evaluate ambient sensor readings against target environmental thresholds and adjust actuators to maintain optimal greenhouse growing conditions.

#### Scenario: Automatic mode temperature ventilation trigger
- **GIVEN** the system is in `SystemMode::AUTOMATIC`
- **WHEN** ambient air temperature exceeds `28.0°C` (`TEMP_THRESHOLD_HIGH`)
- **THEN** the system SHALL actuate the ventilation window servo to the open position (90°)

#### Scenario: Automatic mode temperature ventilation recovery with hysteresis
- **GIVEN** the ventilation window is currently open (90°)
- **WHEN** ambient air temperature drops below `26.0°C` (`TEMP_THRESHOLD_HIGH - TEMP_HYSTERESIS`)
- **THEN** the system SHALL actuate the ventilation window servo to the closed position (0°)

#### Scenario: Temperature within hysteresis band
- **GIVEN** the temperature is between `26.0°C` and `28.0°C`
- **WHEN** new temperature readings are processed
- **THEN** the ventilation window servo SHALL maintain its previous position without state chatter

#### Scenario: Automatic mode soil irrigation trigger
- **GIVEN** the system is in `SystemMode::AUTOMATIC`
- **WHEN** soil moisture drops below `30.0%` (`SOIL_DRY_THRESHOLD`)
- **THEN** the system SHALL activate the irrigation pump actuator

#### Scenario: Automatic mode soil irrigation recovery with hysteresis
- **GIVEN** the irrigation pump is active
- **WHEN** soil moisture exceeds `35.0%` (`SOIL_DRY_THRESHOLD + SOIL_HYSTERESIS`)
- **THEN** the system SHALL deactivate the irrigation pump actuator

#### Scenario: Automatic mode supplemental lighting trigger
- **GIVEN** the system is in `SystemMode::AUTOMATIC`
- **WHEN** ambient light level drops below `500.0 lx` (`LIGHT_DARK_THRESHOLD`)
- **THEN** the system SHALL activate the growth light actuator

#### Scenario: Automatic mode supplemental lighting recovery with hysteresis
- **GIVEN** the growth light actuator is active
- **WHEN** ambient light level exceeds `550.0 lx` (`LIGHT_DARK_THRESHOLD + LIGHT_HYSTERESIS`)
- **THEN** the system SHALL deactivate the growth light actuator

---

### Requirement: System Operating Modes
The system SHALL support two distinct operating modes (`AUTOMATIC` and `MANUAL`) toggled via a dedicated hardware mode button.

#### Scenario: Mode push-button toggle
- **GIVEN** a debounced press event on `PIN_BTN_MODE`
- **WHEN** the mode state transitions
- **THEN** the system SHALL toggle between `SystemMode::AUTOMATIC` and `SystemMode::MANUAL`

#### Scenario: Manual mode actuator control
- **GIVEN** the system is in `SystemMode::MANUAL`
- **WHEN** automatic threshold triggers occur
- **THEN** automatic actuator state changes SHALL be suppressed, allowing direct operator button overrides
