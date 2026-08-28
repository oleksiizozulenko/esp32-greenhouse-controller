#ifndef IRRIGATION_ACTUATOR_H
#define IRRIGATION_ACTUATOR_H

#include "Actuator.h"

class IrrigationActuator : public Actuator {
public:
    explicit IrrigationActuator(int pin = -1, const char* name = "Irrigation")
        : Actuator(pin, ActuatorType::IRRIGATION, name) {}
    ~IrrigationActuator() override = default;
};

#endif // IRRIGATION_ACTUATOR_H