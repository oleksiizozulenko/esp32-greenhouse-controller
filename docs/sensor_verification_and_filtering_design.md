# Sensor Verification & Signal Filtering Architecture

## Overview
This document details the multi-tier sensor verification and signal processing framework for the ESP32 Greenhouse Controller. It replaces simplistic threshold checks (`if (val > max)`) with protocol-level digital bus integrity verification and multi-stage analog signal filtering to eliminate hardware bounce, electromagnetic interference (EMI), and transient ADC spikes.

---

## Decision Log

| # | Decision | Alternatives Considered | Rationale |
|---|---|---|---|
| 1 | **Combined Scope**: Digital bus CRC/ACK + Analog signal pipeline | Analog only, Digital only | Both digital (DHT) and analog (Soil/Light) suffer from hardware garbage and need tailored integrity checks. |
| 2 | **Hybrid Analog Pipeline**: Median $\rightarrow$ Slew-Rate Limiter $\rightarrow$ Kaufman Filter | Median-only, Kaufman-only | Median drops spike noise; slew-rate limits physical jump speed; Kaufman smooths valid readings. |
| 3 | **Architecture**: Modular `ISensorFilter` Decorator Pattern (Option 1) | Monolithic `Sensor`, Service-level pipeline | Decoupled math logic, testable in PlatformIO `test/`, zero runtime heap allocations. |
| 4 | **Flexible Binding**: Pointer to `ISensorFilter` in `Sensor` base class | Fixed `<N>` template in `Sensor` base class | Removes hardcoded filter limits. Allows single filters, composite filter chains, or zero filters without template bloat. |

---

## Architectural Components

### 1. Filter Interface (`ISensorFilter`)
Every filter implements `ISensorFilter` without dynamic memory allocations (`malloc`/`new`) during processing cycles:

```cpp
struct FilterResult {
    float value;
    bool isValid;
};

class ISensorFilter {
public:
    virtual ~ISensorFilter() = default;
    virtual FilterResult process(float input, bool inputError) = 0;
    virtual void reset() = 0;
};
```

---

### 2. Concrete Analog Filters

#### Median Filter (`MedianFilter<WINDOW_SIZE>`)
Drops isolated hardware spikes by selecting the median value from a small static sliding buffer (default $N=5$).

#### Slew-Rate Limiter (`SlewRateLimiter`)
Enforces maximum physical change rates per second (e.g. soil moisture delta cannot exceed 5%/sec), clamping physically impossible data jumps.

#### Kaufman Adaptive Moving Average (`KaufmanFilter`)
Dynamically adjusts smoothing coefficient ($SC$) based on data volatility (Efficiency Ratio $ER$):
$$ER = \frac{|\text{Change over } n \text{ periods}|}{\sum |\text{Single period changes}|}$$
$$SC = \left[ ER \cdot \left(\frac{2}{2+1} - \frac{2}{30+1}\right) + \frac{2}{30+1} \right]^2$$
$$Value_t = Value_{t-1} + SC \cdot (Raw_t - Value_{t-1})$$

---

### 3. Digital Bus Integrity & Fallback Logic (`DigitalSensor`)

For digital sensors (DHT11/22, OneWire, I2C):
1. **Retry Policy**: Retries up to $N=3$ times with a small inter-try delay ($50\text{ ms}$) on CRC/ACK failure.
2. **Timeout Fallback**: On sustained read failure, returns the last known valid reading for up to `validDataTimeoutMs` ($10\text{ s}$). Once expired, flags `isError = true`.

---

### 4. Integration with Base `Sensor` Class

`Sensor.h` holds a pointer to `ISensorFilter`:

```cpp
class Sensor {
protected:
    ISensorFilter* filter = nullptr;

public:
    void setFilter(ISensorFilter* newFilter) {
        filter = newFilter;
    }

    SensorData readProcessed() {
        SensorData raw = read();
        if (filter != nullptr) {
            FilterResult res = filter->process(raw.value, raw.isError);
            return { res.value, !res.isValid };
        }
        return raw;
    }
};
```

---

## Verification & Testing Strategy

1. **Unit Tests (`test/test_filters.cpp`)**:
   - `test_median_filter`: Validates spike suppression on `[20, 21, 999, 22, 21]`.
   - `test_slew_rate_limiter`: Validates clamping on abrupt step changes.
   - `test_kaufman_filter`: Validates adaptive smoothing response to noisy inputs vs real trends.
   - `test_digital_sensor_retry`: Validates retry loops and unexpired fallback timeout.
2. **Hardware Integration Testing**:
   - Flash ESP32 target and verify `SensorsService` log outputs under line disconnect / simulated hardware noise.
