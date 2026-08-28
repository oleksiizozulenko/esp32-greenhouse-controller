#ifndef LIGHT_ACTUATOR_H
#define LIGHT_ACTUATOR_H

#include "Actuator.h"

class LightActuator : public Actuator {
public:
    explicit LightActuator(int pin = -1, const char* name = "Light")
        : Actuator(pin, ActuatorType::LIGHT, name) {}
    ~LightActuator() override = default;
};

#endif // LIGHT_ACTUATOR_H
