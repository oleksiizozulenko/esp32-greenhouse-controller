#include "YellowLedLightActuator.h"

YellowLedLightActuator::YellowLedLightActuator(int gpioPin)
    : LightActuator(gpioPin, "Yellow LED Light"), state(false) {}

bool YellowLedLightActuator::begin() {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
    state = false;
    return true;
}

bool YellowLedLightActuator::turnOn() {
    digitalWrite(pin, HIGH);
    state = true;
    return true;
}

bool YellowLedLightActuator::turnOff() {
    digitalWrite(pin, LOW);
    state = false;
    return true;
}

bool YellowLedLightActuator::isOn() const {
    return state;
}

