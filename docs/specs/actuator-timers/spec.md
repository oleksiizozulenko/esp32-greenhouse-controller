# Actuator Timers & Safety Auto-Off Specification

## Purpose
Specifies the FreeRTOS Software Timer mechanism (`TimerHandle_t`) for actuator safety auto-shutoff during manual overrides in AUTOMATIC mode, timeout configurations, callback execution, and lifecycle management within `GreenhouseController`.

---

## Requirements

### Requirement: FreeRTOS Actuator Safety Auto-Off Timers
The system SHALL use non-blocking FreeRTOS software timers (`xTimerCreate`, `xTimerReset`, `xTimerStop`) to automatically shut off active actuators after pre-configured timeout durations during manual overrides in `AUTOMATIC` mode.

#### Scenario: Manual activation auto-shutoff trigger in AUTOMATIC mode
- **GIVEN** the system is in `SystemMode::AUTOMATIC`
- **AND** an actuator is currently inactive/OFF
- **WHEN** the actuator is turned ON manually via push-button press (`onButtonPressed`)
- **THEN** `GreenhouseController` SHALL start or reset a one-shot FreeRTOS software timer associated with that `ActuatorType`
- **THEN** the timer duration SHALL match the configuration:

| ActuatorType | Config Constant | Timeout Duration |
|---|---|---|
| `IRRIGATION` | `IRRIGATION_TIMEOUT_MS` | 10,000 ms (10 sec) |
| `VENTILATION` | `VENTILATION_TIMEOUT_MS` | 30,000 ms (30 sec) |
| `LIGHT` | `LIGHT_TIMEOUT_MS` | 60,000 ms (60 sec) |

#### Scenario: Timer expiration auto-shutoff execution
- **GIVEN** a running actuator safety timer
- **WHEN** the software timer duration expires
- **THEN** the timer callback (`vActuatorTimerCallback`) SHALL execute inside the FreeRTOS Timer Service Task
- **THEN** the callback SHALL verify that global mode is `SystemMode::AUTOMATIC`
- **THEN** the callback SHALL reset subsystem manual state to `ManualState::OFF` and restore subsystem mode to `ControlMode::AUTO`
- **THEN** the callback SHALL safely turn `OFF` the target actuator
- **THEN** the callback SHALL log an audit message `[SAFETY TIMER] <SubsystemName> Timer Expired -> Turning Manual State OFF`

#### Scenario: Manual deactivation before timer expiration
- **GIVEN** an active actuator with a running safety timer in `SystemMode::AUTOMATIC`
- **WHEN** the operator manually turns `OFF` the actuator via button press before timer expiration
- **THEN** `GreenhouseController` SHALL stop the active timer (`xTimerStop`) to prevent delayed execution
- **AND** `GreenhouseController` SHALL set subsystem mode to `ControlMode::MANUAL` and `ManualState::OFF` to prevent immediate auto re-triggering

#### Scenario: Re-triggering active timer
- **GIVEN** an actuator that is already `ON` with an active running timer
- **WHEN** the actuator timer is re-triggered
- **THEN** `GreenhouseController` SHALL call `xTimerReset` to restart the countdown from zero

#### Scenario: Manual mode timer exclusion
- **GIVEN** the system is in `SystemMode::MANUAL`
- **WHEN** an actuator is toggled ON or OFF via button press
- **THEN** `GreenhouseController` SHALL NOT arm or reset safety auto-off timers (actuators run indefinitely until toggled off)

#### Scenario: Global mode transition timer cleanup
- **GIVEN** any active running actuator safety timers
- **WHEN** `GreenhouseController::setSystemMode` is invoked to change operating modes
- **THEN** all active FreeRTOS actuator timers SHALL be stopped via `stopTimerFor` to prevent timer leakage across modes

---

### Requirement: Memory Management & Timer Handle Lifecycle
Actuator timers SHALL reuse handles to prevent dynamic heap fragmentation and memory leaks.

#### Scenario: Timer handle instantiation & reuse
- **GIVEN** `GreenhouseController`
- **WHEN** a timer is requested for an `ActuatorType` for the first time
- **THEN** `GreenhouseController` SHALL lazily instantiate a single `ActuatorTimer` and `TimerHandle_t` with `TimerContext{this, type}`
- **WHEN** subsequent timer activations occur for the same `ActuatorType`
- **THEN** the system SHALL reuse the existing `TimerHandle_t` without allocating additional dynamic memory

#### Scenario: Controller destruction cleanup
- **GIVEN** `GreenhouseController` lifecycle termination
- **WHEN** `~GreenhouseController()` is invoked
- **THEN** all active FreeRTOS timers SHALL be stopped (`xTimerStop`), deleted (`xTimerDelete`), and freed from memory

