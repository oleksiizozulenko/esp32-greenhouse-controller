#ifndef VENTILATION_ACTUATOR_H
#define VENTILATION_ACTUATOR_H

#include "IPositionalActuator.h"

class VentilationActuator : virtual public IPositionalActuator {
protected:
    int pin;
    const char* name;

    explicit VentilationActuator(int pin = -1, const char* name = "Ventilation")
        : pin(pin), name(name) {}

public:
    ~VentilationActuator() override = default;

    int getPin() const override { return pin; }
    ActuatorType getType() const override { return ActuatorType::VENTILATION; }
    const char* getName() const override { return name; }
    const char* getStatusText() const override { return isOperating() ? "OPEN" : "CLOSE"; }
};

#endif // VENTILATION_ACTUATOR_H