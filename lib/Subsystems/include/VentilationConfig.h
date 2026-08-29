#ifndef VENTILATION_CONFIG_H
#define VENTILATION_CONFIG_H

#include "config.h"

struct VentilationConfig {
    // Automation Rules
    float tempHighThreshold = TEMP_THRESHOLD_HIGH; // 28.0°C
    float tempHysteresis = TEMP_HYSTERESIS;       // 2.0°C
    float humidityHighThreshold = HUMIDITY_THRESHOLD_HIGH; // 70.0%
    float humidityHysteresis = HUMIDITY_HYSTERESIS; // 5.0%

    // Safety & Alarm Rules
    float tempCriticalLimit = CRITICAL_TEMP_HIGH; // 45.0°C
    float humidityCriticalLimit = CRITICAL_HUMIDITY_HIGH; // 85.0%

    // TODO: Add loadFromJson() / saveToJson() via LittleFS in future iteration
};

#endif // VENTILATION_CONFIG_H
