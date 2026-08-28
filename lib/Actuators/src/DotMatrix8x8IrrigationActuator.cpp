#include "DotMatrix8x8IrrigationActuator.h"

DotMatrix8x8IrrigationActuator::DotMatrix8x8IrrigationActuator(int gpioPin)
    : IrrigationActuator(gpioPin, "8x8 Matrix Irrigation"), activeState(false) {}

bool DotMatrix8x8IrrigationActuator::begin() {
    pinMode(pin, OUTPUT);
    turnOff();
    return true;
}

bool DotMatrix8x8IrrigationActuator::turnOn() {
    activeState = true;
    digitalWrite(pin, HIGH);
    return true;
}

bool DotMatrix8x8IrrigationActuator::turnOff() {
    activeState = false;
    digitalWrite(pin, LOW);
    return true;
}

bool DotMatrix8x8IrrigationActuator::isOn() const {
    return activeState;
}

