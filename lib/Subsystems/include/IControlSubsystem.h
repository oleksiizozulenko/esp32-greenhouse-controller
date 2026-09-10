#ifndef I_CONTROL_SUBSYSTEM_H
#define I_CONTROL_SUBSYSTEM_H

#include "ControlTypes.h"
#include "SensorsService.h"
#include "SafetyMonitorService.h"

class IControlSubsystem {
public:
    virtual ~IControlSubsystem() = default;

    virtual SubsystemType getType() const = 0;
    virtual const char* getName() const = 0;

    virtual void update(const SensorDataMap& readings, const SystemHealthState& healthState) = 0;

    virtual ControlMode getMode() const = 0;
    virtual void setMode(ControlMode mode) = 0;
    virtual void toggleMode() = 0;

    virtual ManualState getManualState() const = 0;
    virtual void setManualState(ManualState state) = 0;
    virtual void toggleManualState() = 0;

    virtual bool isActuatorOn() const = 0;
    virtual SubsystemStatus getStatus() const = 0;
};

#endif // I_CONTROL_SUBSYSTEM_H
