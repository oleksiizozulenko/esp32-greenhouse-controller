#ifndef VENTILATION_SUBSYSTEM_H
#define VENTILATION_SUBSYSTEM_H

#include "IControlSubsystem.h"
#include "VentilationConfig.h"
#include "IActuator.h"

class VentilationSubsystem : public IControlSubsystem {
private:
    IActuator* actuator;
    VentilationConfig config;
    ControlMode mode;
    ManualState manualState;
    bool activeAlarm;

public:
    explicit VentilationSubsystem(IActuator* actuator = nullptr, const VentilationConfig& config = VentilationConfig());

    SubsystemType getType() const override { return SubsystemType::VENTILATION; }
    const char* getName() const override { return "Ventilation"; }

    // Direct, lightweight typed update
    void update(SensorData tempData, SensorData humData, const SystemHealthState& healthState);

    // Generic interface adapter
    void update(const SensorDataMap& readings, const SystemHealthState& healthState) override {
        update(readings.get(SensorType::TEMPERATURE), readings.get(SensorType::HUMIDITY), healthState);
    }

    ControlMode getMode() const override { return mode; }
    void setMode(ControlMode newMode) override { mode = newMode; }
    void toggleMode() override { mode = (mode == ControlMode::AUTO ? ControlMode::MANUAL : ControlMode::AUTO); }

    ManualState getManualState() const override { return manualState; }
    void setManualState(ManualState newState) override { manualState = newState; }
    void toggleManualState() override { manualState = (manualState == ManualState::ON ? ManualState::OFF : ManualState::ON); }

    bool isActuatorOn() const override;
    SubsystemStatus getStatus() const override;

    void setActuator(IActuator* act) { actuator = act; }
    IActuator* getActuator() const { return actuator; }

    const VentilationConfig& getConfig() const { return config; }
    void setConfig(const VentilationConfig& newConfig) { config = newConfig; }
};

#endif // VENTILATION_SUBSYSTEM_H
