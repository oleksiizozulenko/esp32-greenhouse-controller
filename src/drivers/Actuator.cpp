#include "drivers/Actuator.h"

Actuator::Actuator(int pin, ActuatorType type, const char* name)
    : pin(pin), type(type), name(name) {}

Actuator::~Actuator() {}

const char* Actuator::getStatusText() {
    return isOn() ? "ON" : "OFF";
}
