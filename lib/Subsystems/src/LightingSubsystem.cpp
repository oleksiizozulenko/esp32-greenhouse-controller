#include "LightingSubsystem.h"

LightingSubsystem::LightingSubsystem(IActuator* actuator, const LightingConfig& config)
    : actuator(actuator), config(config), mode(ControlMode::AUTO), manualState(ManualState::OFF), activeAlarm(false) {}

bool LightingSubsystem::isActuatorOn() const {
    return actuator != nullptr ? actuator->isOperating() : false;
}

SubsystemStatus LightingSubsystem::getStatus() const {
    return SubsystemStatus{
        SubsystemType::LIGHTING,
        mode,
        manualState,
        isActuatorOn(),
        activeAlarm
    };
}

void LightingSubsystem::update(SensorData lightData, SystemHealthState& healthState) {
    bool isCriticalHigh = !lightData.isError && (lightData.value >= config.lightCriticalHigh);

    if (isCriticalHigh) {
        activeAlarm = true;
        healthState.hasOperatorAdvisory = true;
        snprintf(healthState.advisoryMsg, sizeof(healthState.advisoryMsg),
                 "[ALARM] Lighting: Extreme High Light Level!");
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
    if (lightData.isError) {
        if (actuator->isOperating()) {
            actuator->turnOff();
        }
    } else if (lightData.value < config.lightLowThreshold && !actuator->isOperating()) {
        actuator->turnOn();
    } else if (lightData.value > config.lightHighThreshold && actuator->isOperating()) {
        actuator->turnOff();
    }
}
