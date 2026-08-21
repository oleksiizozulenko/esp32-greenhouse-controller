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
    ~SafetyMonitorService();

    SystemHealthState evaluate(const SensorDataMap& readings, bool isAutoMode);
};

#endif // SAFETY_MONITOR_SERVICE_H
