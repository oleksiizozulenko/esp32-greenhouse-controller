# Sensor Filtering Specification

## Purpose
Specifies multi-tier analog signal filtering (`ISensorFilter`), zero-heap Decorator architecture, digital single-bus integrity retries (DHT11), driver-domain error boundary separation (Option A), and 3.3V ADC-to-Lux photoelectric conversions.

---

## Requirements

### Requirement: Analog Signal Filtering Pipeline
The system SHALL support modular decorator filters implementing `ISensorFilter` to remove hardware noise, ADC spikes, and non-physical signal jumps without dynamic heap allocations (`malloc`/`new`).

#### Scenario: Spike suppression via MedianFilter
- **GIVEN** a sliding buffer of 5 analog samples containing a single transient spike (e.g. `[20, 21, 999, 22, 21]`)
- **WHEN** `MedianFilter<5>::process` evaluates the buffer
- **THEN** the spike value (`999`) SHALL be dropped, returning the median value (`21`)

#### Scenario: Physical rate clamping via SlewRateLimiter
- **GIVEN** a calibrated maximum allowed slew rate delta per second:
  - Soil Moisture: `10.0% / sec` (`SOIL_MAX_SLEW_PER_SEC`)
  - Ambient Light: `2000.0 lx / sec` (`LIGHT_MAX_SLEW_PER_SEC`)
  - Temperature: `2.0°C / sec` (`TEMP_MAX_SLEW_PER_SEC`)
  - Air Humidity: `5.0% / sec` (`HUMIDITY_MAX_SLEW_PER_SEC`)
- **WHEN** an abrupt non-physical step change occurs
- **THEN** `SlewRateLimiter::process` SHALL clamp the output rate of change to the configured physical limit

#### Scenario: Adaptive volatility smoothing via KaufmanFilter
- **GIVEN** noisy or fluctuating valid sensor data
- **WHEN** `KaufmanFilter::process` calculates the Efficiency Ratio ($ER$) and Smoothing Coefficient ($SC$)
- **THEN** the filter SHALL apply dynamic exponential moving average smoothing proportional to signal volatility

---

### Requirement: Digital Single-Bus Retry & Fallback Logic
Digital single-bus drivers (DHT11 on `PIN_DHT` / GPIO 23) SHALL implement bus integrity retries and valid data timeout fallback.

#### Scenario: Read error retry
- **GIVEN** a digital sensor read failure (checksum error or missing bus response)
- **WHEN** the driver executes a read attempt
- **THEN** it SHALL retry up to 3 times (`DIGITAL_SENSOR_MAX_RETRIES`) with a 50ms delay (`DIGITAL_SENSOR_RETRY_DELAY_MS`) before reporting failure

#### Scenario: Timeout fallback during temporary bus disconnect
- **GIVEN** a digital sensor has failed consecutive reads
- **WHEN** the elapsed error duration is under `10000 ms` (`DIGITAL_SENSOR_FALLBACK_TIMEOUT`)
- **THEN** the driver SHALL return the last known valid cached reading with `isError = false`

#### Scenario: Timeout expiration on sustained bus disconnect
- **GIVEN** a digital sensor remains disconnected beyond `10000 ms`
- **WHEN** `DigitalSensor::read` executes
- **THEN** the driver SHALL set `isError = true` to trigger system safety alarms

---

### Requirement: Driver and Domain Error Boundary Separation (Option A)
Sensor drivers and domain safety evaluators SHALL maintain a strict separation of validation concerns.

#### Scenario: Driver-level physical read validity
- **GIVEN** a sensor driver sampling physical hardware
- **WHEN** `read()` executes
- **THEN** `isError` SHALL be set to `true` strictly for physical/mathematical invalidity (non-finite values, division by zero, or bus read failure)
- **AND** the driver SHALL NOT evaluate domain range bounds (`SENSOR_*_MIN_ERROR` / `SENSOR_*_MAX_ERROR`)

#### Scenario: Domain-level range validation
- **GIVEN** a populated `SensorDataMap`
- **WHEN** `SafetyMonitorService::evaluate` evaluates health
- **THEN** domain range boundaries SHALL be validated against domain limits, asserting `hasHardwareError = true` if breached

---

### Requirement: 3.3V ESP32 ADC Light Sensor Conversion
The LDR driver (`LightSensor`) SHALL compute ambient illuminance in Lux using 12-bit ADC values ($0..4095$) and a $10\,\text{k}\Omega$ pull-down resistor divider powered from $3.3\text{V}$ on `PIN_LDR` (GPIO 35, ADC1_CH7).

#### Scenario: LDR resistance calculation
- **GIVEN** raw 12-bit ADC reading `rawADC` ($1..4094$)
- **WHEN** `LightSensor::read` processes the sample
- **THEN** LDR resistance $R_{LDR}$ SHALL be calculated as:
  $$R_{LDR} = 10000 \times \left(\frac{4095.0}{\text{rawAdc}} - 1.0\right)$$

#### Scenario: Lux conversion
- **GIVEN** calculated $R_{LDR}$
- **WHEN** Lux intensity is computed
- **THEN** Lux SHALL be derived via the logarithmic photoresistor characteristic:
  $$\text{Lux} = \left(\frac{250593.5}{R_{LDR}}\right)^{\frac{1}{0.7}}$$

