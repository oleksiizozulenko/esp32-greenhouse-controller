#include "SafetyMonitorService.h"
#include "Logging.h"

static const char* TAG = "SAFETY";

SafetyMonitorService::SafetyMonitorService() {}

SystemHealthState SafetyMonitorService::evaluate(const SensorDataMap& readings, bool isAutoMode) const {
    (void)isAutoMode;
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
            ESP_LOGW(TAG, "Hardware Fault on %s (Pin %d): isError=%d, value=%.2f",
                     s->getName(), s->getPin(), d.isError, d.value);
            state.hasHardwareError = true;
            break;
        }

        switch (s->getType()) {
            case SensorType::TEMPERATURE:
                if (d.value < SENSOR_TEMP_MIN_ERROR || d.value > SENSOR_TEMP_MAX_ERROR) {
                    ESP_LOGW(TAG, "Temp Out of Range on %s (Pin %d): %.2f C (Limits: %.1f .. %.1f)",
                             s->getName(), s->getPin(), d.value, SENSOR_TEMP_MIN_ERROR, SENSOR_TEMP_MAX_ERROR);
                    state.hasHardwareError = true;
                }
                break;
            case SensorType::HUMIDITY:
                if (d.value < SENSOR_HUMIDITY_MIN_ERROR || d.value > SENSOR_HUMIDITY_MAX_ERROR) {
                    ESP_LOGW(TAG, "Humidity Out of Range on %s (Pin %d): %.2f %% (Limits: %.1f .. %.1f)",
                             s->getName(), s->getPin(), d.value, SENSOR_HUMIDITY_MIN_ERROR, SENSOR_HUMIDITY_MAX_ERROR);
                    state.hasHardwareError = true;
                }
                break;
            case SensorType::SOIL:
                if (d.value < SENSOR_SOIL_MIN_ERROR || d.value > SENSOR_SOIL_MAX_ERROR) {
                    ESP_LOGW(TAG, "Soil Out of Range on %s (Pin %d): %.2f %% (Limits: %.1f .. %.1f)",
                             s->getName(), s->getPin(), d.value, SENSOR_SOIL_MIN_ERROR, SENSOR_SOIL_MAX_ERROR);
                    state.hasHardwareError = true;
                }
                break;
            case SensorType::LIGHT:
                if (d.value < SENSOR_LIGHT_MIN_ERROR || d.value > SENSOR_LIGHT_MAX_ERROR) {
                    ESP_LOGW(TAG, "Light Out of Range on %s (Pin %d): %.2f lx (Limits: %.1f .. %.1f)",
                             s->getName(), s->getPin(), d.value, SENSOR_LIGHT_MIN_ERROR, SENSOR_LIGHT_MAX_ERROR);
                    state.hasHardwareError = true;
                }
                break;
            default:
                break;
        }

        if (state.hasHardwareError) {
            break;
        }
    }

    if (state.hasHardwareError) {
        state.requiresAlarm = true;
        snprintf(state.advisoryMsg, sizeof(state.advisoryMsg), "SENSOR ERROR!");
        return state;
    }

    // Extract sensor values safely
    SensorData tempData = readings.get(SensorType::TEMPERATURE);
    SensorData humData = readings.get(SensorType::HUMIDITY);
    SensorData soilData = readings.get(SensorType::SOIL);
    SensorData lightData = readings.get(SensorType::LIGHT);

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
        state.requiresAlarm = false;
        snprintf(state.advisoryMsg, sizeof(state.advisoryMsg), "HUMID HIGH! Press VENT");
        return state;
    }

    // Priority 6: Dry Soil (Soil < 30%)
    if (!soilData.isError && !isnan(soilData.value) && soilData.value < CRITICAL_SOIL_LOW) {
        state.hasOperatorAdvisory = true;
        state.requiresAlarm = false;
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
