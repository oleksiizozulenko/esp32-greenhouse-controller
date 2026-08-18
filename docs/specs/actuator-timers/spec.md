# Actuator Timers & Safety Auto-Off Specification

## Purpose
Specifies the FreeRTOS Software Timer mechanism (`TimerHandle_t`) for actuator safety auto-shutoff, timeout configurations, and lifecycle management within `GreenhouseController`.

---

## Requirements

### Requirement: FreeRTOS Actuator Safety Auto-Off Timers
The system SHALL use non-blocking FreeRTOS software timers (`xTimerCreate`, `xTimerReset`, `xTimerStop`) to automatically shut off active actuators after pre-configured timeout durations.

#### Scenario: Manual activation auto-shutoff trigger
- **GIVEN** an actuator is turned ON manually via push-button press (`onButtonPressed`)
- **WHEN** the actuator turns `ON`
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
- **THEN** the callback SHALL safely turn `OFF` the target actuator
- **THEN** the callback SHALL log an audit message `[SAFETY TIMER] <ActuatorName> Timer Expired -> Auto Turning OFF`

#### Scenario: Manual deactivation before timer expiration
- **GIVEN** an active actuator with a running safety timer
- **WHEN** the operator manually turns `OFF` the actuator via button press before timer expiration
- **THEN** `GreenhouseController` SHALL stop the active timer (`xTimerStop`) to prevent delayed execution

#### Scenario: Re-triggering active timer
- **GIVEN** an actuator that is already `ON` with an active running timer
- **WHEN** the actuator timer is re-triggered
- **THEN** `GreenhouseController` SHALL call `xTimerReset` to restart the countdown from zero

---

### Requirement: Memory Management & Timer Handle Lifecycle
Actuator timers SHALL reuse handles to prevent dynamic heap fragmentation and memory leaks.

#### Scenario: Timer handle instantiation & reuse
- **GIVEN** `GreenhouseController`
- **WHEN** a timer is requested for an `ActuatorType` for the first time
- **THEN** `GreenhouseController` SHALL lazily instantiate a single `ActuatorTimer` and `TimerHandle_t`
- **WHEN** subsequent timer activations occur for the same `ActuatorType`
- **THEN** the system SHALL reuse the existing `TimerHandle_t` without allocating additional dynamic memory

#### Scenario: Controller destruction cleanup
- **GIVEN** `GreenhouseController` lifecycle termination
- **WHEN** `~GreenhouseController()` is invoked
- **THEN** all active FreeRTOS timers SHALL be stopped (`xTimerStop`), deleted (`xTimerDelete`), and freed from memory
