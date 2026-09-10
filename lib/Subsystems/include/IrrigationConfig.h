#ifndef IRRIGATION_CONFIG_H
#define IRRIGATION_CONFIG_H

#include "config.h"

struct IrrigationConfig {
    // Automation Rules
    float soilDryThreshold = (float)SOIL_DRY_THRESHOLD; // 40.0%
    float soilHysteresis = (float)SOIL_HYSTERESIS;      // 5.0%

    // Safety & Alarm Rules
    float soilCriticalDryLimit = CRITICAL_SOIL_DRY;      // 20.0%
    float soilCriticalHighLimit = CRITICAL_SOIL_HIGH;    // 85.0% (Overwatering)
    uint32_t maxTimeoutMs = IRRIGATION_TIMEOUT_MS;       // 10000 ms
};

#endif // IRRIGATION_CONFIG_H
