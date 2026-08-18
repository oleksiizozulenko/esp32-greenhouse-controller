# Sensor Filtering Specification

## Purpose
Specifies multi-tier analog signal filtering (`ISensorFilter`), digital bus integrity retries, zero-heap Decorator architecture, and 3.3V ADC-to-Lux photoelectric conversions.

---

## Requirements

### Requirement: Analog Signal Filtering Pipeline
The system SHALL support modular decorator filters implementing `ISensorFilter` to remove hardware noise, ADC spikes, and non-physical signal jumps without dynamic heap allocations (`malloc`/`new`).

#### Scenario: Spike suppression via MedianFilter
- **GIVEN** a sliding buffer of 5 analog samples containing a single transient spike (e.g. `[20, 21, 999, 22, 21]`)
- **WHEN** `MedianFilter<5>::process` evaluates the buffer
- **THEN** the spike value (`999`) SHALL be dropped, returning the median value (`21`)

#### Scenario: Physical rate clamping via SlewRateLimiter
- **GIVEN** a maximum allowed slew rate delta per second
- **WHEN** an abrupt physical step change occurs (e.g. soil reading jumps by 40% in 100ms)
- **THEN** `SlewRateLimiter::process` SHALL clamp the output rate of change to the maximum allowed physical speed

#### Scenario: Adaptive volatility smoothing via KaufmanFilter
- **GIVEN** noisy or fluctuating valid sensor data
- **WHEN** `KaufmanFilter::process` calculates the Efficiency Ratio ($ER$) and Smoothing Coefficient ($SC$)
- **THEN** the filter SHALL apply dynamic exponential moving average smoothing proportional to signal volatility

---

### Requirement: Digital Bus Retry & Fallback Logic
Digital bus drivers (e.g., DHT22 1-Wire) SHALL implement bus integrity retries and valid data timeout fallback.

#### Scenario: CRC or ACK read error retry
- **GIVEN** a digital sensor read failure (CRC mismatch or missing ACK)
- **WHEN** `DigitalSensor` executes a read attempt
- **THEN** it SHALL retry up to 3 times with a 50ms inter-try delay before reporting failure

#### Scenario: Timeout fallback during temporary bus disconnect
- **GIVEN** a digital sensor has failed consecutive reads
- **WHEN** the elapsed error duration is under `10000 ms` (`validDataTimeoutMs`)
- **THEN** the driver SHALL return the last known valid cached reading with `isError = false`

#### Scenario: Timeout expiration on sustained bus disconnect
- **GIVEN** a digital sensor remains disconnected beyond `10000 ms`
- **WHEN** `DigitalSensor::read` executes
- **THEN** the driver SHALL set `isError = true` to trigger system safety alarms

---

### Requirement: 3.3V ESP32 ADC Light Sensor Conversion
The LDR driver (`LightSensor`) SHALL compute ambient illuminance in Lux using 12-bit ADC values ($0..4095$) and a $10\,\text{k}\Omega$ pull-down resistor divider powered from $3.3\text{V}$.

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
