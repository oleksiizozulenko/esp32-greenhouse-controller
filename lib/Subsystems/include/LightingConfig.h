#ifndef LIGHTING_CONFIG_H
#define LIGHTING_CONFIG_H

#include "config.h"

struct LightingConfig {
    // Automation Rules
    float lightLowThreshold = LIGHT_LOW_THRESHOLD;   // Low light level threshold (3000.0 lx) -> turn ON
    float lightHighThreshold = LIGHT_HIGH_THRESHOLD; // High light level threshold (3500.0 lx) -> turn OFF
    float lightDarkThreshold = LIGHT_DARK_THRESHOLD; // Legacy alias for lightLowThreshold
    float lightHysteresis = LIGHT_HYSTERESIS;        // Legacy alias for hysteresis

    // Safety & Alarm Rules
    float lightCriticalHigh = CRITICAL_LIGHT_HIGH;  // 25000.0 lx

    // TODO: Add loadFromJson() / saveToJson() via LittleFS in future iteration
};

#endif // LIGHTING_CONFIG_H
