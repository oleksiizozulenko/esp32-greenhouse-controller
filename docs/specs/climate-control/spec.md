# Climate Control Specification

## Purpose
Specifies autonomous microclimate regulation, target environmental thresholds, hysteresis algorithm behavior, operating mode semantics, and critical emergency overrides for the ESP32 Greenhouse Controller.

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
- **AND** relative humidity is below `65.0%`
- **THEN** the system SHALL actuate the ventilation window servo to the closed position (0°)

#### Scenario: Temperature within hysteresis band
- **GIVEN** the temperature is between `26.0°C` and `28.0°C`
- **WHEN** new temperature readings are processed
- **THEN** the ventilation window servo SHALL maintain its previous position without state chatter

#### Scenario: Automatic mode humidity ventilation trigger
- **GIVEN** the system is in `SystemMode::AUTOMATIC`
- **WHEN** relative humidity exceeds `70.0%` (`HUMIDITY_THRESHOLD_HIGH`)
- **THEN** the system SHALL actuate the ventilation window servo to the open position (90°)

#### Scenario: Automatic mode humidity ventilation recovery with hysteresis
- **GIVEN** the ventilation window is currently open (90°)
- **WHEN** relative humidity drops below `65.0%` (`HUMIDITY_THRESHOLD_HIGH - HUMIDITY_HYSTERESIS`)
- **AND** ambient air temperature is below `26.0°C`
- **THEN** the system SHALL actuate the ventilation window servo to the closed position (0°)

#### Scenario: Humidity within hysteresis band
- **GIVEN** relative humidity is between `65.0%` and `70.0%`
- **WHEN** new humidity readings are processed
- **THEN** the ventilation window servo SHALL maintain its previous position without state chatter

#### Scenario: Dual-variable ventilation arbitration
- **GIVEN** `VentilationSubsystem` running in `ControlMode::AUTO`
- **WHEN** either temperature exceeds `28.0°C` OR humidity exceeds `70.0%`
- **THEN** the ventilation window SHALL open
- **AND** the window SHALL close only when BOTH temperature drops below `26.0°C` AND humidity drops below `65.0%`

#### Scenario: Automatic mode soil irrigation trigger
- **GIVEN** the system is in `SystemMode::AUTOMATIC`
- **WHEN** soil moisture drops below `40.0%` (`SOIL_DRY_THRESHOLD`)
- **THEN** the system SHALL activate the irrigation actuator

#### Scenario: Automatic mode soil irrigation recovery with hysteresis
- **GIVEN** the irrigation actuator is active
- **WHEN** soil moisture exceeds `45.0%` (`SOIL_DRY_THRESHOLD + SOIL_HYSTERESIS`)
- **THEN** the system SHALL deactivate the irrigation actuator

#### Scenario: Automatic mode supplemental lighting trigger
- **GIVEN** the system is in `SystemMode::AUTOMATIC`
- **WHEN** ambient light level drops below `300.0 lx` (`LIGHT_LOW_THRESHOLD`)
- **THEN** the system SHALL activate the growth light actuator

#### Scenario: Automatic mode supplemental lighting recovery with hysteresis
- **GIVEN** the growth light actuator is active
- **WHEN** ambient light level exceeds `1000.0 lx` (`LIGHT_HIGH_THRESHOLD`)
- **THEN** the system SHALL deactivate the growth light actuator

---

### Requirement: Critical Emergency Environmental Overrides
The controller SHALL enforce immediate safety overrides when environmental conditions cross critical hazard boundaries.

#### Scenario: Critical temperature overheat override
- **GIVEN** ambient air temperature reaches or exceeds `45.0°C` (`CRITICAL_TEMP_HIGH`)
- **WHEN** `VentilationSubsystem` updates in `ControlMode::AUTO`
- **THEN** the system SHALL force the ventilation window servo to the open position (90°) regardless of humidity readings

#### Scenario: Critical temperature frost hazard override
- **GIVEN** ambient air temperature drops below `5.0°C` (`CRITICAL_TEMP_LOW`)
- **WHEN** `VentilationSubsystem` updates in `ControlMode::AUTO`
- **THEN** the system SHALL force the ventilation window servo to the closed position (0°) to conserve greenhouse heat

#### Scenario: Critical soil flood hazard override
- **GIVEN** soil moisture reaches or exceeds `85.0%` (`CRITICAL_SOIL_HIGH`)
- **WHEN** `IrrigationSubsystem` updates in either `ControlMode::AUTO` or `ControlMode::MANUAL`
- **THEN** the system SHALL immediately deactivate the irrigation actuator and set `manualState` to `ManualState::OFF`

---

### Requirement: System Operating Modes & Graceful Degradation
The system SHALL support two distinct operating modes (`AUTOMATIC` and `MANUAL`) toggled via a dedicated hardware mode button, and gracefully degrade subsystems when sensor data is unavailable.

#### Scenario: Centralized mode push-button toggle
- **GIVEN** a debounced press event on `PIN_BTN_MODE`
- **WHEN** the mode state transitions
- **THEN** `GreenhouseController` SHALL toggle between `SystemMode::AUTOMATIC` and `SystemMode::MANUAL` under `modeMutex`
- **AND** all active FreeRTOS actuator auto-off timers SHALL be stopped
- **AND** all subsystem manual states SHALL be reset to `ManualState::OFF` when entering `AUTOMATIC` mode

#### Scenario: Manual mode actuator control
- **GIVEN** the system is in `SystemMode::MANUAL`
- **WHEN** automatic threshold triggers occur
- **THEN** automatic actuator state changes SHALL be suppressed, allowing direct operator button overrides without auto-off timers

#### Scenario: Degraded sensor fallback to manual
- **GIVEN** a subsystem's associated sensor reading is missing or reports hardware error (`!readings.has(type)`)
- **WHEN** `GreenhouseController::update` executes
- **THEN** the affected subsystem SHALL fall back to `ControlMode::MANUAL` in an inactive safe state
- **AND** unaffected subsystems with valid sensor readings SHALL continue operating in their configured mode
