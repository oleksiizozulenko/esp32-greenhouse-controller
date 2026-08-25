#ifndef HAL_COMMON_TYPES_H
#define HAL_COMMON_TYPES_H

#include <stdint.h>

// Sensor reading status codes
enum class SensorStatus : uint8_t {
    OK = 0,
    Error_Timeout,
    Error_OutOfRange,
    Error_HardwareFault,
    Uninitialized
};

// Generic templated sensor result container
template <typename T>
struct SensorReadResult {
    T value;
    SensorStatus status;
    uint32_t timestampMs;

    bool isValid() const { return status == SensorStatus::OK; }
};

// Generic Actuator states
enum class ActuatorState : uint8_t {
    OFF = 0,
    ON,
    OPERATING,
    ERROR
};

// Universal Display field for key-value display slots
struct DisplayField {
    const char* label;   // e.g. "Temp", "Humidity"
    float value;         // e.g. 24.5
    const char* unit;    // e.g. "°C", "%"
    uint8_t precision;   // Decimal precision
};

// Universal Alert Severities
enum class AlertSeverity : uint8_t {
    INFO = 0,
    WARNING,
    CRITICAL
};

// Universal Alert Event
struct AlertEvent {
    uint16_t code;
    AlertSeverity severity;
    const char* message;
};

#endif // HAL_COMMON_TYPES_H
