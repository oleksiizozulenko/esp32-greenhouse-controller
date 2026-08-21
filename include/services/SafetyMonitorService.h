#ifndef SAFETY_MONITOR_SERVICE_H
#define SAFETY_MONITOR_SERVICE_H

#include <Arduino.h>
#include <math.h>
#include <string.h>
#include "../config.h"
#include "SensorsService.h"

struct SystemHealthState {
    bool hasHardwareError;     // True if any present sensor is NaN/inf/out-of-bounds
    bool hasCriticalHazard;    // True if Overheat (>45°C), Frost (<5°C), or Soil Flood (>85%)
    bool hasOperatorAdvisory;  // True if Dry Soil (<30%), High Humidity (>85%), or High Light (>10000lx)
    bool requiresAlarm;        // True if Buzzer 1kHz alarm tone should sound
    char advisoryMsg[24];      // Prioritized prompt banner for MANUAL mode (max 23 chars + null)
};

class SafetyMonitorService {
public:
    SafetyMonitorService();

    SystemHealthState evaluate(const SensorDataMap& readings, bool isAutoMode) const;
};

#endif // SAFETY_MONITOR_SERVICE_H
