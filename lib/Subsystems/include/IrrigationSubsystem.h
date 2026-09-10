#ifndef IRRIGATION_SUBSYSTEM_H
#define IRRIGATION_SUBSYSTEM_H

#include "IControlSubsystem.h"
#include "IrrigationConfig.h"
#include "IActuator.h"

class IrrigationSubsystem : public IControlSubsystem {
private:
    IActuator* actuator;
    IrrigationConfig config;
    ControlMode mode;
    ManualState manualState;
    bool activeAlarm;

public:
    explicit IrrigationSubsystem(IActuator* actuator = nullptr, const IrrigationConfig& config = IrrigationConfig());

    SubsystemType getType() const override { return SubsystemType::IRRIGATION; }
    const char* getName() const override { return "Irrigation"; }

    // Direct, lightweight typed update
    void update(SensorData soilData, const SystemHealthState& healthState);

    // Generic interface adapter
    void update(const SensorDataMap& readings, const SystemHealthState& healthState) override {
        update(readings.get(SensorType::SOIL), healthState);
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

    const IrrigationConfig& getConfig() const { return config; }
    void setConfig(const IrrigationConfig& newConfig) { config = newConfig; }
};

#endif // IRRIGATION_SUBSYSTEM_H
