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
    SafetyMonitorService() {}

    SystemHealthState evaluate(const SensorDataMap& readings, bool isAutoMode) const {
        SystemHealthState state;
        state.hasHardwareError = false;
        state.hasCriticalHazard = false;
        state.hasOperatorAdvisory = false;
        state.requiresAlarm = false;
        state.advisoryMsg[0] = '\0';

        // 1. Check Hardware & Domain Errors across present sensors
        for (size_t i = 0; i < readings.size(); ++i) {
            Sensor* s = readings[i].sensor;
            SensorData d = readings[i].data;
            if (s == nullptr) continue;

            if (d.isError || !isfinite(d.value)) {
                state.hasHardwareError = true;
                break;
            }

            const char* name = s->getName();
            if (name != nullptr) {
                if (strcmp(name, "Temperature") == 0) {
                    if (d.value < SENSOR_TEMP_MIN_ERROR || d.value > SENSOR_TEMP_MAX_ERROR) {
                        state.hasHardwareError = true;
                        break;
                    }
                } else if (strcmp(name, "Humidity") == 0) {
                    if (d.value < SENSOR_HUMIDITY_MIN_ERROR || d.value > SENSOR_HUMIDITY_MAX_ERROR) {
                        state.hasHardwareError = true;
                        break;
                    }
                } else if (strcmp(name, "Soil") == 0) {
                    if (d.value < SENSOR_SOIL_MIN_ERROR || d.value > SENSOR_SOIL_MAX_ERROR) {
                        state.hasHardwareError = true;
                        break;
                    }
                } else if (strcmp(name, "Light") == 0) {
                    if (d.value < SENSOR_LIGHT_MIN_ERROR || d.value > SENSOR_LIGHT_MAX_ERROR) {
                        state.hasHardwareError = true;
                        break;
                    }
                }
            }
        }

        if (state.hasHardwareError) {
            state.requiresAlarm = true;
            snprintf(state.advisoryMsg, sizeof(state.advisoryMsg), "SENSOR ERROR!");
            return state;
        }

        // Extract sensor values safely
        SensorData tempData = readings.get("Temperature");
        SensorData humData = readings.get("Humidity");
        SensorData soilData = readings.get("Soil");
        SensorData lightData = readings.get("Light");

        // Priority 2: Overheat (Temp > 45°C)
        if (!tempData.isError && !isnan(tempData.value) && tempData.value > CRITICAL_TEMP_HIGH) {
            state.hasCriticalHazard = true;
            state.requiresAlarm = true;
            snprintf(state.advisoryMsg, sizeof(state.advisoryMsg), "TEMP HIGH! Press VENT");
            return state;
        }

        // Priority 3: Frost (Temp < 5°C)
        if (!tempData.isError && !isnan(tempData.value) && tempData.value < CRITICAL_TEMP_LOW) {
            state.hasCriticalHazard = true;
            state.requiresAlarm = true;
            snprintf(state.advisoryMsg, sizeof(state.advisoryMsg), "FROST RISK! Temp Low");
            return state;
        }

        // Priority 4: Soil Flood (Soil > 85%)
        if (!soilData.isError && !isnan(soilData.value) && soilData.value > CRITICAL_SOIL_HIGH) {
            state.hasCriticalHazard = true;
            state.requiresAlarm = true;
            snprintf(state.advisoryMsg, sizeof(state.advisoryMsg), "SOIL FLOOD! Stop Water");
            return state;
        }

        // Priority 5: High Humidity (Humidity > 85%)
        if (!humData.isError && !isnan(humData.value) && humData.value > CRITICAL_HUMIDITY_HIGH) {
            state.hasOperatorAdvisory = true;
            state.requiresAlarm = true;
            snprintf(state.advisoryMsg, sizeof(state.advisoryMsg), "HUMID HIGH! Press VENT");
            return state;
        }

        // Priority 6: Dry Soil (Soil < 30%)
        if (!soilData.isError && !isnan(soilData.value) && soilData.value < CRITICAL_SOIL_LOW) {
            state.hasOperatorAdvisory = true;
            state.requiresAlarm = isAutoMode;
            snprintf(state.advisoryMsg, sizeof(state.advisoryMsg), "SOIL DRY! Press IRRIG");
            return state;
        }

        // Priority 7: High Light (Light > 10000 lx)
        if (!lightData.isError && !isnan(lightData.value) && lightData.value > CRITICAL_LIGHT_HIGH) {
            state.hasOperatorAdvisory = true;
            state.requiresAlarm = false;
            snprintf(state.advisoryMsg, sizeof(state.advisoryMsg), "LIGHT HIGH! Press LIGHT");
            return state;
        }

        // Priority 8: Normal
        return state;
    }
};

#endif // SAFETY_MONITOR_SERVICE_H
