#ifndef LIGHT_ACTUATOR_H
#define LIGHT_ACTUATOR_H

#include "IBinaryActuator.h"

class LightActuator : virtual public IBinaryActuator {
protected:
    int pin;
    const char* name;

    explicit LightActuator(int pin = -1, const char* name = "Light")
        : pin(pin), name(name) {}

public:
    ~LightActuator() override = default;

    int getPin() const override { return pin; }
    ActuatorType getType() const override { return ActuatorType::LIGHT; }
    const char* getName() const override { return name; }
    const char* getStatusText() const override { return isOn() ? "ON" : "OFF"; }
};

#endif // LIGHT_ACTUATOR_H

