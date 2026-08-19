# RTOS Task Scheduling & Watchdog Specification

## Purpose
Specifies the FreeRTOS task priority hierarchy, dual-core placement, preemptive scheduling rules, 100% event-driven control loop execution, and the Hardware Task Watchdog Timer (TWDT) liveness monitoring system within the `esp32-greenhouse-controller`.

---

## Requirements

### Requirement: FreeRTOS Task Priority & Dual-Core Placement
The system SHALL organize concurrent operations into dedicated FreeRTOS tasks assigned specific execution priorities and CPU cores to guarantee real-time safety responsiveness.

#### Scenario: Task priority hierarchy and core pinning
- **GIVEN** an ESP32 dual-core microcontroller
- **WHEN** system tasks are created in `setup()`
- **THEN** tasks SHALL be instantiated with the following priority and core configuration:

| Task Name | Function Symbol | Priority Level | Assigned CPU Core | Execution Role |
|---|---|---|---|---|
| `TaskControl` | `vTaskControl` | Priority 3 (High) | Core 1 | Safety evaluation, button ISR event processing, control updates |
| `TaskSensors` | `vTaskSensors` | Priority 2 (Medium) | Core 1 | Periodic sensor sampling & event broadcasting |
| `TaskDisplay` | `vTaskDisplay` | Priority 1 (Low) | Core 0 | OLED screen rendering & UI ViewModel updates |

#### Scenario: Preemptive priority scheduling & preemption
- **GIVEN** `vTaskControl` (Priority 3) transitions from `BLOCKED` to `READY` state (via event or timer)
- **WHEN** lower priority tasks (`vTaskSensors` or `vTaskDisplay`) are executing
- **THEN** the FreeRTOS Scheduler SHALL immediately preempt lower priority tasks on Core 1 to execute `vTaskControl`

#### Scenario: Priority inversion protection
- **GIVEN** shared system state (`globalReadings`, `currentMode`) accessed concurrently across tasks
- **WHEN** tasks acquire shared resources
- **THEN** `sensorMutex` and `modeMutex` SHALL be instantiated using `xSemaphoreCreateMutex()` to enforce FreeRTOS Priority Inheritance and prevent unbounded priority inversion

---

### Requirement: 100% Event-Driven TaskControl Execution
`vTaskControl` SHALL execute in an event-driven loop that blocks until notified by system events, while maintaining a bounded fallback timeout for safety checks.

#### Scenario: Instant event-driven waking
- **GIVEN** `vTaskControl` waiting on `systemEventGroup`
- **WHEN** `vTaskSensors` posts `EVENT_BIT_SENSOR_READY` or a button press posts `EVENT_BIT_BUTTON_EVENT`
- **THEN** `vTaskControl` SHALL wake up instantly without waiting for fixed polling delays

#### Scenario: Fallback periodic safety timeout
- **GIVEN** `vTaskControl` waiting on `systemEventGroup`
- **WHEN** no system events occur for 100 ms
- **THEN** `xEventGroupWaitBits` SHALL time out after 100 ms to allow `vTaskControl` to execute periodic safety monitor evaluations and memory diagnostics

---

### Requirement: Hardware Task Watchdog Timer (TWDT) Liveness Monitoring
The system SHALL use the ESP32 Hardware Task Watchdog Timer (`esp_task_wdt`) to detect task deadlocks or freezes and automatically recover system operation.

#### Scenario: TWDT initialization
- **GIVEN** system startup in `setup()`
- **WHEN** `esp_task_wdt_init(5, true)` is invoked
- **THEN** the hardware Task Watchdog SHALL be configured with a 5-second timeout and automatic panic hardware reboot enabled

#### Scenario: Critical task registration & periodic feeding
- **GIVEN** active tasks `vTaskSensors` and `vTaskControl`
- **WHEN** tasks initialize their loop
- **THEN** each task SHALL register itself with TWDT via `esp_task_wdt_add(NULL)`
- **THEN** each task SHALL feed the watchdog on every loop iteration using `esp_task_wdt_reset()`

#### Scenario: Automatic system reboot on task freeze
- **GIVEN** a registered task (`vTaskControl` or `vTaskSensors`)
- **WHEN** the task deadlocks, freezes, or fails to execute `esp_task_wdt_reset()` for longer than 5 seconds
- **THEN** the TWDT hardware timer SHALL expire
- **THEN** the ESP32 microcontroller SHALL trigger a hardware panic reboot to restore normal operation
