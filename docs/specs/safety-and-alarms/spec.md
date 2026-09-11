# Safety & Alarms Specification

## Purpose
Specifies `SafetyMonitorService` health state evaluation, deterministic priority arbitration, acoustic buzzer and LED indicator driving via `SystemAlertService`, and advisory banner generation for operator guidance.

---

## Requirements

### Requirement: System Health & Safety Monitoring
The system SHALL continuously evaluate sensor readings against physical hazard boundaries and error states to construct a centralized `SystemHealthState`.

#### Scenario: Hardware and domain sensor error detection
- **GIVEN** any registered sensor returns NaN, infinity, math read failure (`isError == true`), or value outside valid domain ranges (`< SENSOR_*_MIN_ERROR` or `> SENSOR_*_MAX_ERROR`)
- **WHEN** `SafetyMonitorService::evaluate` evaluates system health
- **THEN** `hasHardwareError` SHALL be set to `true`, `requiresAlarm` SHALL be `true`, and advisory message SHALL be `"SENSOR ERROR!"`

#### Scenario: Critical temperature overheat hazard
- **GIVEN** temperature reading exceeds `45.0°C` (`CRITICAL_TEMP_HIGH`)
- **WHEN** health evaluation executes without hardware error
- **THEN** `hasCriticalHazard` SHALL be set to `true`, `requiresAlarm` SHALL be `true`, and advisory message SHALL prompt `"TEMP HIGH! Press VENT"`

#### Scenario: Critical temperature frost hazard
- **GIVEN** temperature reading drops below `5.0°C` (`CRITICAL_TEMP_LOW`)
- **WHEN** health evaluation executes without hardware error
- **THEN** `hasCriticalHazard` SHALL be set to `true`, `requiresAlarm` SHALL be `true`, and advisory message SHALL prompt `"FROST RISK! Temp Low"`

#### Scenario: Critical soil flood hazard
- **GIVEN** soil moisture reading exceeds `85.0%` (`CRITICAL_SOIL_HIGH`)
- **WHEN** health evaluation executes without hardware error
- **THEN** `hasCriticalHazard` SHALL be set to `true`, `requiresAlarm` SHALL be `true`, and advisory message SHALL prompt `"SOIL FLOOD! Stop Water"`

#### Scenario: Non-acoustic operator advisories
- **GIVEN** sensor readings cross advisory thresholds (Humidity > 85%, Soil < 30%, or Light > 25000 lx)
- **WHEN** health evaluation executes without critical hazards
- **THEN** `hasOperatorAdvisory` SHALL be set to `true`
- **AND** `requiresAlarm` SHALL remain `false` (no acoustic buzzer alarm)
- **AND** the corresponding advisory message banner SHALL be populated for UI display

---

### Requirement: Deterministic Priority Matrix
When multiple warnings or error conditions coincide, `SafetyMonitorService` SHALL prioritize advisory messages and alarm indicators according to a strict 8-level hierarchy.

| Priority | Condition | `hasHardwareError` | `hasCriticalHazard` | `hasOperatorAdvisory` | `requiresAlarm` | `advisoryMsg` Banner |
|---|---|---|---|---|---|---|
| 1 (Highest) | Hardware / Domain Range Error | `true` | `false` | `false` | `true` | `"SENSOR ERROR!"` |
| 2 | Temperature > 45.0°C (`CRITICAL_TEMP_HIGH`) | `false` | `true` | `false` | `true` | `"TEMP HIGH! Press VENT"` |
| 3 | Temperature < 5.0°C (`CRITICAL_TEMP_LOW`) | `false` | `true` | `false` | `true` | `"FROST RISK! Temp Low"` |
| 4 | Soil Moisture > 85.0% (`CRITICAL_SOIL_HIGH`) | `false` | `true` | `false` | `true` | `"SOIL FLOOD! Stop Water"` |
| 5 | Air Humidity > 85.0% (`CRITICAL_HUMIDITY_HIGH`) | `false` | `false` | `true` | `false` | `"HUMID HIGH! Press VENT"` |
| 6 | Soil Moisture < 30.0% (`CRITICAL_SOIL_LOW`) | `false` | `false` | `true` | `false` | `"SOIL DRY! Press IRRIG"` |
| 7 | Light Intensity > 25000.0 lx (`CRITICAL_LIGHT_HIGH`) | `false` | `false` | `true` | `false` | `"LIGHT HIGH! Press LIGHT"` |
| 8 (Lowest) | Normal Operation | `false` | `false` | `false` | `false` | `""` |

#### Scenario: Coinciding hardware error and temperature overheat
- **GIVEN** a sensor read error and a temperature reading of `48.0°C`
- **WHEN** health state evaluation resolves priority
- **THEN** Priority 1 SHALL override Priority 2, setting `hasHardwareError = true` and displaying `"SENSOR ERROR!"`

---

### Requirement: System Alert Indicators & Acoustic Alarms
The system SHALL actuate status LEDs and an acoustic buzzer via `SystemAlertService` based on `SystemHealthState`.

#### Scenario: Normal operational power indication
- **GIVEN** system startup and operational status
- **WHEN** `SystemAlertService::update` executes
- **THEN** `PIN_LED_GREEN` (`GPIO 15`) SHALL remain continuously illuminated (`HIGH`)

#### Scenario: Visual hazard alert indication
- **GIVEN** `hasHardwareError == true` OR `hasCriticalHazard == true` OR `alarmActive == true`
- **WHEN** `SystemAlertService::update` executes
- **THEN** `PIN_LED_RED` (`GPIO 4`) SHALL illuminate (`HIGH`)
- **WHEN** no error or critical hazard exists
- **THEN** `PIN_LED_RED` SHALL be turned `LOW`

#### Scenario: Acoustic buzzer alarm activation
- **GIVEN** `requiresAlarm == true` OR `alarmActive == true`
- **WHEN** `SystemAlertService::update` executes
- **THEN** `PIN_BUZZER` (`GPIO 18`) SHALL sound a 1000Hz pulsed tone

