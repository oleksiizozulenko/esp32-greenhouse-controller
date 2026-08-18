# UI & Display Specification

## Purpose
Specifies SSD1306 128x64 OLED rendering, `DisplayManager` view renderer decoupling, dynamic `DisplayViewModel` struct construction, line item formatting, and banner status updates.

---

## Requirements

### Requirement: Decoupled View Renderer Architecture
The display rendering layer SHALL be strictly decoupled from sensor polling and safety evaluation by consuming an immutable `DisplayViewModel`.

#### Scenario: DisplayViewModel construction
- **GIVEN** updated sensor data, safety health state, and active mode
- **WHEN** `GreenhouseController` prepares the UI render payload
- **THEN** it SHALL populate `DisplayViewModel` with formatted string primitives for mode, health status, sensor readings, actuator states, and advisory banners

#### Scenario: OLED Screen update cycle
- **GIVEN** a populated `DisplayViewModel`
- **WHEN** `DisplayManager::render(model)` is called
- **THEN** `DisplayManager` SHALL clear the SSD1306 screen buffer, draw text elements, and execute `display.display()`

---

### Requirement: Display Layout & Information Hierarchy
The 128x64 pixel OLED display layout SHALL present critical metrics and state clearly without text overlap.

#### Scenario: Header line rendering
- **GIVEN** `DisplayViewModel` contains `modeText` ("AUTO" / "MANUAL") and `healthStatus` ("[OK]" / "[ERR]")
- **WHEN** the header line is rendered
- **THEN** mode text SHALL display on the top-left and health status SHALL display on the top-right

#### Scenario: Sensor and Actuator line items
- **GIVEN** array of sensor `LineItem` structs
- **WHEN** metrics are rendered
- **THEN** each line SHALL format label and value (e.g. `Temp: 24.5C`, `Hum: 60%`, `Soil: 45%`, `Light: 520lx`)

#### Scenario: Advisory prompt banner rendering
- **GIVEN** `advisoryBanner` string is non-empty (e.g. `"TEMP HIGH! Press VENT"`)
- **WHEN** advisory banner is rendered
- **THEN** it SHALL be displayed at the bottom of the screen with inverse text highlighting or bold font style for operator visibility
