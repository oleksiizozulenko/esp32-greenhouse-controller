#include "YellowLedLightActuator.h"

YellowLedLightActuator::YellowLedLightActuator(int gpioPin)
    : LightActuator(gpioPin, "Yellow LED Light"), state(false) {}

void YellowLedLightActuator::init() {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
    state = false;
}

void YellowLedLightActuator::turnOn() {
    digitalWrite(pin, HIGH);
    state = true;
}

void YellowLedLightActuator::turnOff() {
    digitalWrite(pin, LOW);
    state = false;
}

bool YellowLedLightActuator::isOn() {
    return state;
}
