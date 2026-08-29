#ifndef LIGHTING_SUBSYSTEM_H
#define LIGHTING_SUBSYSTEM_H

#include "IControlSubsystem.h"
#include "LightingConfig.h"
#include "IActuator.h"

class LightingSubsystem : public IControlSubsystem {
private:
    IActuator* actuator;
    LightingConfig config;
    ControlMode mode;
    ManualState manualState;
    bool activeAlarm;

public:
    explicit LightingSubsystem(IActuator* actuator = nullptr, const LightingConfig& config = LightingConfig());

    SubsystemType getType() const override { return SubsystemType::LIGHTING; }
    const char* getName() const override { return "Lighting"; }

    // Direct, lightweight typed update
    void update(SensorData lightData, SystemHealthState& healthState);

    // Generic interface adapter
    void update(const SensorDataMap& readings, SystemHealthState& healthState) override {
        update(readings.get(SensorType::LIGHT), healthState);
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

    const LightingConfig& getConfig() const { return config; }
    void setConfig(const LightingConfig& newConfig) { config = newConfig; }
};

#endif // LIGHTING_SUBSYSTEM_H
