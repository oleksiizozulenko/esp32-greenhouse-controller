# RTOS Task Scheduling & Watchdog Specification

## Purpose
Specifies FreeRTOS task priority hierarchy, dual-core placement, inter-task queues, event group bit definitions, 100% event-driven control loop execution, Hardware Task Watchdog Timer (TWDT) liveness monitoring, and structured logging within the `esp32-greenhouse-controller`.

---

## Requirements

### Requirement: FreeRTOS Task Priority & Dual-Core Placement
The system SHALL organize concurrent operations into dedicated FreeRTOS tasks assigned specific execution priorities, stack allocations, and CPU cores to guarantee real-time safety responsiveness.

#### Scenario: Task priority hierarchy and core pinning
- **GIVEN** an ESP32 dual-core microcontroller
- **WHEN** system tasks are spawned in `startRtosTasks()`
- **THEN** tasks SHALL be instantiated with the following priority, stack, and core configuration:

| Task Name | Function Symbol | Priority Level | Stack Size | Core | Execution Role |
|---|---|---|---|---|---|
| `TaskControl` | `vTaskControl` | Priority 3 (High) | 3072 bytes | Core 1 | Safety evaluation, button event queue processing, control & actuator updates |
| `TaskSensors` | `vTaskSensors` | Priority 2 (Medium) | 4096 bytes | Core 1 | Periodic sensor sampling (2000ms), filtering, & queue dispatch |
| `TaskDisplay` | `vTaskDisplay` | Priority 1 (Low) | 3072 bytes | Core 0 | Periodic OLED screen rendering (200ms) & UI ViewModel updates |

#### Scenario: Preemptive priority scheduling & preemption
- **GIVEN** `vTaskControl` (Priority 3) transitions from `BLOCKED` to `READY` state (via event or timer)
- **WHEN** lower priority tasks (`vTaskSensors` or `vTaskDisplay`) are executing
- **THEN** the FreeRTOS Scheduler SHALL immediately preempt lower priority tasks on Core 1 to execute `vTaskControl`

#### Scenario: Shared state synchronization and priority inversion protection
- **GIVEN** shared system state (`globalSystemMode`, `globalHealthState`) accessed concurrently across tasks
- **WHEN** tasks access shared resources
- **THEN** access SHALL be guarded by dedicated mutexes (`modeMutex`, `healthStateMutex`) instantiated via `xSemaphoreCreateMutex()` to enforce FreeRTOS Priority Inheritance

#### Scenario: Stack overflow detection
- **GIVEN** FreeRTOS stack overflow hook `vApplicationStackOverflowHook`
- **WHEN** any task exceeds its allocated stack space
- **THEN** the hook SHALL log a critical error via `ESP_LOGE` and trigger a device reboot (`ESP.restart()`) after 1000ms

---

### Requirement: Event-Driven Inter-Task Communication & Queues
Tasks SHALL communicate asynchronously using dedicated FreeRTOS queues and event groups rather than shared unbounded global memory.

#### Scenario: Queue buffer management
- **GIVEN** inter-task data streams
- **WHEN** synchronization handles are created in `initRtosSynchronization()`
- **THEN** the following queues SHALL be instantiated:
  - `controlSensorQueue`: Depth 2, holds `SensorDataMap` payloads for `vTaskControl`
  - `displaySensorQueue`: Depth 2, holds `SensorDataMap` payloads for `vTaskDisplay`
  - `buttonEventQueue`: Depth 10, holds debounced `ButtonEvent` structs from button ISRs

#### Scenario: Event group bit signaling
- **GIVEN** `systemEventGroup`
- **WHEN** system states occur
- **THEN** the corresponding bit SHALL be asserted:
  - `EVENT_BIT_SENSOR_READY` (`1 << 0`): Asserted by `vTaskSensors` upon publishing new sensor data
  - `EVENT_BIT_BUTTON_EVENT` (`1 << 1`): Asserted from button ISR upon valid debounced button press
  - `EVENT_BIT_SAFETY_WARNING` (`1 << 2`): Asserted on critical safety warning
  - `EVENT_BIT_MODE_CHANGED` (`1 << 3`): Asserted when global operating mode toggles

#### Scenario: Event-driven waking with safety timeout
- **GIVEN** `vTaskControl` waiting on `systemEventGroup`
- **WHEN** `EVENT_BIT_SENSOR_READY`, `EVENT_BIT_BUTTON_EVENT`, or `EVENT_BIT_SAFETY_WARNING` is set
- **THEN** `vTaskControl` SHALL wake immediately to process events
- **WHEN** no events occur for 100 ms
- **THEN** `xEventGroupWaitBits` SHALL time out after 100 ms to execute periodic safety evaluations and diagnostics

---

### Requirement: Hardware Task Watchdog Timer (TWDT) Liveness Monitoring
The system SHALL use the ESP32 Hardware Task Watchdog Timer (`esp_task_wdt`) to detect task deadlocks or freezes and automatically recover system operation.

#### Scenario: TWDT initialization
- **GIVEN** system startup in `setup()`
- **WHEN** `esp_task_wdt_init(WDT_TIMEOUT_SECONDS, true)` is invoked
- **THEN** the hardware Task Watchdog SHALL be configured with a 5-second timeout (`WDT_TIMEOUT_SECONDS = 5`) and automatic panic hardware reboot enabled

#### Scenario: Critical task registration & periodic feeding
- **GIVEN** active tasks `vTaskSensors`, `vTaskControl`, and `vTaskDisplay`
- **WHEN** tasks initialize their loop
- **THEN** each task SHALL register itself with TWDT via `esp_task_wdt_add(NULL)`
- **THEN** each task SHALL feed the watchdog on every loop iteration using `esp_task_wdt_reset()`

#### Scenario: Automatic system reboot on task freeze
- **GIVEN** any registered task (`vTaskControl`, `vTaskSensors`, or `vTaskDisplay`)
- **WHEN** a task deadlocks, freezes, or fails to execute `esp_task_wdt_reset()` for longer than 5 seconds
- **THEN** the TWDT hardware timer SHALL expire
- **THEN** the ESP32 microcontroller SHALL trigger a hardware panic reboot to restore normal operation

---

### Requirement: Structured ESP-IDF Logging & Diagnostics
The system SHALL use unified structured logging with tag-based filtering and runtime memory diagnostics.

#### Scenario: Tag-based log filtering
- **GIVEN** `-D CORE_DEBUG_LEVEL=4` configured in `platformio.ini`
- **WHEN** firmware logs messages
- **THEN** logs SHALL use standard ESP-IDF log macros (`ESP_LOGI`, `ESP_LOGD`, `ESP_LOGW`, `ESP_LOGE`) segregated under tags: `CONTROL`, `SENSORS`, `ACTUATORS`, `SAFETY`, `BUTTON`, `CONTROLLER`, and `DIAG`

#### Scenario: Periodic stack high-water mark diagnostics
- **GIVEN** active tasks `hTaskSensors`, `hTaskControl`, and `hTaskDisplay`
- **WHEN** 10 seconds elapse in `vTaskControl`
- **THEN** `printTaskStackDiagnostics()` SHALL query `uxTaskGetStackHighWaterMark()` for each task and `ESP.getFreeHeap()` and output free stack word counts under tag `DIAG`

