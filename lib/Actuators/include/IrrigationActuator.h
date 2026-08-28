#ifndef IRRIGATION_ACTUATOR_H
#define IRRIGATION_ACTUATOR_H

#include "IBinaryActuator.h"

class IrrigationActuator : virtual public IBinaryActuator {
protected:
    int pin;
    const char* name;

    explicit IrrigationActuator(int pin = -1, const char* name = "Irrigation")
        : pin(pin), name(name) {}

public:
    ~IrrigationActuator() override = default;

    int getPin() const override { return pin; }
    ActuatorType getType() const override { return ActuatorType::IRRIGATION; }
    const char* getName() const override { return name; }
    const char* getStatusText() const override { return isOn() ? "ON" : "OFF"; }
};

#endif // IRRIGATION_ACTUATOR_H