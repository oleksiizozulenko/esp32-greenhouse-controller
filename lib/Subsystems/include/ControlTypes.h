#ifndef CONTROL_TYPES_H
#define CONTROL_TYPES_H

enum class ControlMode {
    AUTO,
    MANUAL
};

enum class ManualState {
    OFF,
    ON
};

enum class SubsystemType {
    VENTILATION,
    LIGHTING,
    IRRIGATION,
    UNKNOWN
};

struct SubsystemStatus {
    SubsystemType type;
    ControlMode mode;
    ManualState manualState;
    bool isActuatorOn;
    bool hasActiveAlarm;
};

#endif // CONTROL_TYPES_H
