#include "IrrigationSubsystem.h"

IrrigationSubsystem::IrrigationSubsystem(IActuator* actuator, const IrrigationConfig& config)
    : actuator(actuator), config(config), mode(ControlMode::AUTO), manualState(ManualState::OFF), activeAlarm(false) {}

bool IrrigationSubsystem::isActuatorOn() const {
    return actuator != nullptr ? actuator->isOperating() : false;
}

SubsystemStatus IrrigationSubsystem::getStatus() const {
    return SubsystemStatus{
        SubsystemType::IRRIGATION,
        mode,
        manualState,
        isActuatorOn(),
        activeAlarm
    };
}

void IrrigationSubsystem::update(SensorData soilData, SystemHealthState& healthState) {
    bool isCriticalDry = !soilData.isError && (soilData.value <= config.soilCriticalDryLimit);
    bool isOverwatered = !soilData.isError && (soilData.value >= config.soilCriticalHighLimit);

    if (isCriticalDry || isOverwatered) {
        activeAlarm = true;
        healthState.hasCriticalHazard = isCriticalDry;
        healthState.hasOperatorAdvisory = isOverwatered;
        snprintf(healthState.advisoryMsg, sizeof(healthState.advisoryMsg),
                 "[ALARM] Irrigation: Soil %s!", isCriticalDry ? "Critically Dry" : "Overwatered");
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
    if (soilData.isError) {
        if (actuator->isOperating()) {
            actuator->turnOff();
        }
    } else if (soilData.value < config.soilDryThreshold) {
        if (!actuator->isOperating()) {
            actuator->turnOn();
        }
    } else if (soilData.value > (config.soilDryThreshold + config.soilHysteresis) && actuator->isOperating()) {
        actuator->turnOff();
    }
}
