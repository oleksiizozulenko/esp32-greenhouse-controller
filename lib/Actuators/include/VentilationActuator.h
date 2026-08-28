#ifndef VENTILATION_ACTUATOR_H
#define VENTILATION_ACTUATOR_H

#include "Actuator.h"

class VentilationActuator : public Actuator {
public:
    explicit VentilationActuator(int pin = -1, const char* name = "Ventilation")
        : Actuator(pin, ActuatorType::VENTILATION, name) {}
    ~VentilationActuator() override = default;
};

#endif // VENTILATION_ACTUATOR_H