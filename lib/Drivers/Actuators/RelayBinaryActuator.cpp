#include "RelayBinaryActuator.h"

RelayBinaryActuator::RelayBinaryActuator(int gpioPin, bool isActiveLow)
    : pin(gpioPin), activeLow(isActiveLow), state(false) {}

bool RelayBinaryActuator::begin() {
    pinMode(pin, OUTPUT);
    turnOff();
    return true;
}

bool RelayBinaryActuator::turnOn() {
    state = true;
    digitalWrite(pin, activeLow ? LOW : HIGH);
    return true;
}

bool RelayBinaryActuator::turnOff() {
    state = false;
    digitalWrite(pin, activeLow ? HIGH : LOW);
    return true;
}

bool RelayBinaryActuator::isOn() const {
    return state;
}

bool RelayBinaryActuator::isOperating() const {
    return state;
}
