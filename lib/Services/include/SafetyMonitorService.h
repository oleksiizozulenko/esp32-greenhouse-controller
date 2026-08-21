#ifndef SAFETY_MONITOR_SERVICE_H
#define SAFETY_MONITOR_SERVICE_H

#include <Arduino.h>
#include "config.h"
#include "SensorsService.h"

struct SystemHealthState {
    bool hasHardwareError;
    bool hasCriticalHazard;
    bool hasOperatorAdvisory;
    bool requiresAlarm;
    char advisoryMsg[64];
};

class SafetyMonitorService {
public:
    SafetyMonitorService();
    ~SafetyMonitorService() = default;

    SystemHealthState evaluate(const SensorDataMap& readings, bool isAutoMode) const;
};

#endif // SAFETY_MONITOR_SERVICE_H
