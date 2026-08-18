# Safety & Alarms Specification

## Purpose
Specifies `SafetyMonitorService` health state evaluation, physical boundary checks, acoustic buzzer alarm logic, and advisory banner prompt generation for operator guidance.

---

## Requirements

### Requirement: System Health & Safety Monitoring
The system SHALL continuously evaluate sensor readings against physical hazard boundaries and error states to construct a centralized `SystemHealthState`.

#### Scenario: Hardware sensor failure detection
- **GIVEN** any registered sensor returns NaN, infinity, or math error (`isErr == true`)
- **WHEN** `SafetyMonitorService` evaluates system health
- **THEN** `hasHardwareError` SHALL be set to `true`, `requiresAlarm` SHALL be `true`, and advisory message SHALL be `"SENSOR ERROR!"`

#### Scenario: Critical temperature overheat hazard
- **GIVEN** temperature reading exceeds `45.0°C` (`CRITICAL_TEMP_HIGH`)
- **WHEN** health evaluation executes
- **THEN** `hasCriticalHazard` SHALL be set to `true`, `requiresAlarm` SHALL be `true`, and advisory message in MANUAL mode SHALL prompt `"TEMP HIGH! Press VENT"`

#### Scenario: Critical temperature frost hazard
- **GIVEN** temperature reading drops below `5.0°C` (`CRITICAL_TEMP_LOW`)
- **WHEN** health evaluation executes
- **THEN** `hasCriticalHazard` SHALL be set to `true`, `requiresAlarm` SHALL be `true`, and advisory message in MANUAL mode SHALL prompt `"FROST RISK! Temp Low"`

#### Scenario: Critical soil flood hazard
- **GIVEN** soil moisture reading exceeds `85.0%` (`CRITICAL_SOIL_HIGH`)
- **WHEN** health evaluation executes
- **THEN** `hasCriticalHazard` SHALL be set to `true`, `requiresAlarm` SHALL be `true`, and advisory message in MANUAL mode SHALL prompt `"SOIL FLOOD! Stop Water"`

---

### Requirement: Deterministic Priority Matrix
When multiple warnings or error conditions coincide, `SafetyMonitorService` SHALL prioritize advisory messages and alarm indicators according to a strict 8-level hierarchy.

| Priority | Condition | `hasHardwareError` | `hasCriticalHazard` | `hasOperatorAdvisory` | `requiresAlarm` | `advisoryMsg` (MANUAL Mode) |
|---|---|---|---|---|---|---|
| 1 (Highest) | Hardware Error / NaN / Inf | `true` | `false` | `false` | `true` | `"SENSOR ERROR!"` |
| 2 | Temp > 45°C | `false` | `true` | `false` | `true` | `"TEMP HIGH! Press VENT"` |
| 3 | Temp < 5°C | `false` | `true` | `false` | `true` | `"FROST RISK! Temp Low"` |
| 4 | Soil > 85% | `false` | `true` | `false` | `true` | `"SOIL FLOOD! Stop Water"` |
| 5 | Humidity > 85% | `false` | `false` | `true` | `true` | `"HUMID HIGH! Press VENT"` |
| 6 | Soil < 30% | `false` | `false` | `true` | `isAutoMode` | `"SOIL DRY! Press IRRIG"` |
| 7 | Light > 10000 lx | `false` | `false` | `true` | `false` | `"LIGHT HIGH! Press LIGHT"` |
| 8 (Lowest) | Normal Operation | `false` | `false` | `false` | `false` | `""` |

#### Scenario: Coinciding hardware error and temperature overheat
- **GIVEN** a sensor read error and a temperature reading of `48.0°C`
- **WHEN** health state evaluation resolves priority
- **THEN** Priority 1 SHALL override Priority 2, displaying `"SENSOR ERROR!"`
