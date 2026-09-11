# UI & Display Specification

## Purpose
Specifies SSD1306 128x64 OLED rendering, `DisplayManager` view renderer decoupling, dynamic `DisplayViewModel` struct construction, line item formatting, structured screen divisions, advisory banner presentation, and hardware failure bypass.

---

## Requirements

### Requirement: Decoupled View Renderer Architecture
The display rendering layer SHALL be strictly decoupled from sensor polling and safety evaluation by consuming an immutable `DisplayViewModel`.

#### Scenario: DisplayViewModel construction
- **GIVEN** updated sensor data (`SensorDataMap`), safety health state (`SystemHealthState`), and active mode
- **WHEN** `GreenhouseController::buildDisplayViewModel` prepares the UI render payload
- **THEN** it SHALL populate `DisplayViewModel` with formatted string primitives:
  - `modeText`: `"AUTO"` or `"MANUAL"`
  - `healthStatus`: `"[ERR]"` if `healthState.hasHardwareError`, otherwise `"[OK]"`
  - `sensors`: up to 4 items formatted with 4-character label and value (`"%.4s:"`, `"%.1f%s"` or `"ERR"`)
  - `actuators`: up to 4 items formatted with 4-character label and status text (`"%.4s:"`, `"%s"`)
  - `advisoryBanner`: prioritized health advisory string

#### Scenario: OLED Screen update cycle
- **GIVEN** a populated `DisplayViewModel`
- **WHEN** `DisplayManager::render(model)` is called by `vTaskDisplay`
- **THEN** `DisplayManager` SHALL clear the SSD1306 screen buffer, draw text elements and dividing lines, and execute `display.display()`

#### Scenario: Hardware disconnection bypass
- **GIVEN** SSD1306 OLED display is disconnected or fails I2C initialization at address `0x3C` (`SCREEN_I2C_ADDR`)
- **WHEN** `DisplayManager::init()` or `DisplayManager::render()` executes
- **THEN** display operations SHALL gracefully bypass rendering without freezing FreeRTOS tasks or locking the I2C bus

---

### Requirement: Display Layout & Screen Division Hierarchy
The 128x64 pixel OLED display layout SHALL present critical metrics and state clearly using a structured 4-section division.

#### Scenario: Header section rendering
- **GIVEN** `DisplayViewModel` contains `modeText` and `healthStatus`
- **WHEN** the header line is rendered at cursor `(0, 0)`
- **THEN** it SHALL be drawn as `[%s] %s` (e.g. `[AUTO] [OK]`)
- **AND** a horizontal divider line SHALL be drawn across `(0, 9)` to `(128, 9)`

#### Scenario: Sensor metrics 2-column rendering
- **GIVEN** array of sensor `LineItem` structs
- **WHEN** sensor metrics are rendered starting at cursor `(0, 12)`
- **THEN** metrics SHALL be displayed in a 2-column layout (items 0-1 on line 1, items 2-3 on line 2)
- **AND** a horizontal divider line SHALL be drawn across `(0, 34)` to `(128, 34)`

#### Scenario: Actuator status 2-column rendering
- **GIVEN** array of actuator `LineItem` structs
- **WHEN** actuator statuses are rendered starting at cursor `(0, 37)`
- **THEN** statuses SHALL be displayed in a 2-column layout (items 0-1 on line 1, items 2-3 on line 2)
- **AND** a horizontal divider line SHALL be drawn across `(0, 53)` to `(128, 53)`

#### Scenario: Advisory prompt banner rendering
- **GIVEN** `advisoryBanner` string in `DisplayViewModel`
- **WHEN** the banner is rendered starting at cursor `(0, 55)`
- **THEN** it SHALL be displayed on the bottom line clamped to 21 characters (`%.21s`) for high-priority operator visibility

