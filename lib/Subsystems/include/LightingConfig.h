#ifndef LIGHTING_CONFIG_H
#define LIGHTING_CONFIG_H

#include "config.h"

struct LightingConfig {
    // Automation Rules
    float lightLowThreshold = LIGHT_LOW_THRESHOLD;   // Low light level threshold (300.0 lx) -> turn ON
    float lightHighThreshold = LIGHT_HIGH_THRESHOLD; // High light level threshold (1000.0 lx) -> turn OFF

    // Safety & Alarm Rules
    float lightCriticalHigh = CRITICAL_LIGHT_HIGH;  // 25000.0 lx
};

#endif // LIGHTING_CONFIG_H
