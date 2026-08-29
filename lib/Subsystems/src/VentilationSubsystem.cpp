#include "VentilationSubsystem.h"

VentilationSubsystem::VentilationSubsystem(IActuator* actuator, const VentilationConfig& config)
    : actuator(actuator), config(config), mode(ControlMode::AUTO), manualState(ManualState::OFF), activeAlarm(false) {}

bool VentilationSubsystem::isActuatorOn() const {
    return actuator != nullptr ? actuator->isOperating() : false;
}

SubsystemStatus VentilationSubsystem::getStatus() const {
    return SubsystemStatus{
        SubsystemType::VENTILATION,
        mode,
        manualState,
        isActuatorOn(),
        activeAlarm
    };
}

void VentilationSubsystem::update(SensorData tempData, SensorData humData, SystemHealthState& healthState) {
    bool isCriticalTemp = !tempData.isError && (tempData.value >= config.tempCriticalLimit);
    bool isCriticalHum = !humData.isError && (humData.value >= config.humidityCriticalLimit);

    if (isCriticalTemp || isCriticalHum) {
        activeAlarm = true;
        healthState.hasCriticalHazard = true;
        snprintf(healthState.advisoryMsg, sizeof(healthState.advisoryMsg),
                 "[ALARM] Ventilation: High %s Hazard!", isCriticalTemp ? "Temp" : "Humidity");
    } else {
        activeAlarm = false;
    }

    if (actuator == nullptr) return;

    if (mode == ControlMode::MANUAL) {
        if (manualState == ManualState::ON) {
            if (!actuator->isOperating()) {
                actuator->turnOn();
            }
        } else {
            if (actuator->isOperating()) {
                actuator->turnOff();
            }
        }
        return;
    }

    // AUTO Mode
    if (isCriticalTemp || isCriticalHum) {
        if (!actuator->isOperating()) {
            actuator->turnOn();
        }
        return;
    }

    bool tempError = tempData.isError;
    bool humError = humData.isError;

    bool highTemp = !tempError && (tempData.value > config.tempHighThreshold);
    bool highHum = !humError && (humData.value > config.humidityHighThreshold);

    bool normalTemp = tempError || (tempData.value < (config.tempHighThreshold - config.tempHysteresis));
    bool normalHum = humError || (humData.value < (config.humidityHighThreshold - config.humidityHysteresis));

    if (tempError && humError) {
        if (actuator->isOperating()) {
            actuator->turnOff();
        }
    } else if (highTemp || highHum) {
        if (!actuator->isOperating()) {
            actuator->turnOn();
        }
    } else if (normalTemp && normalHum && actuator->isOperating()) {
        actuator->turnOff();
    }
}
