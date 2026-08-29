#ifndef LIGHTING_CONFIG_H
#define LIGHTING_CONFIG_H

#include "config.h"

struct LightingConfig {
    // Automation Rules
    float lightDarkThreshold = LIGHT_DARK_THRESHOLD; // 3000.0 lx
    float lightHysteresis = LIGHT_HYSTERESIS;        // 500.0 lx

    // Safety & Alarm Rules
    float lightCriticalHigh = CRITICAL_LIGHT_HIGH;  // 25000.0 lx

    // TODO: Add loadFromJson() / saveToJson() via LittleFS in future iteration
};

#endif // LIGHTING_CONFIG_H
