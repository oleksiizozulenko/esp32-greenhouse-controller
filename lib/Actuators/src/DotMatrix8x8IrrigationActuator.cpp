#include "DotMatrix8x8IrrigationActuator.h"


DotMatrix8x8IrrigationActuator::DotMatrix8x8IrrigationActuator(int dinPin, int clkPin, int csPin)
    : dataPin(dinPin), clockPin(clkPin), csPin(csPin), activeState(false) {}

bool DotMatrix8x8IrrigationActuator::begin() {
    pinMode(dataPin, OUTPUT);
    pinMode(clockPin, OUTPUT);
    pinMode(csPin, OUTPUT);
    turnOff();
    return true;
}

bool DotMatrix8x8IrrigationActuator::turnOn() {
    activeState = true;
    digitalWrite(csPin, LOW);
    digitalWrite(dataPin, HIGH); // Display active pattern on 8x8 matrix
    digitalWrite(csPin, HIGH);
    return true;
}

bool DotMatrix8x8IrrigationActuator::turnOff() {
    activeState = false;
    digitalWrite(csPin, LOW);
    digitalWrite(dataPin, LOW);
    digitalWrite(csPin, HIGH);
    return true;
}

bool DotMatrix8x8IrrigationActuator::isOn() const {
    return activeState;
}

bool DotMatrix8x8IrrigationActuator::isOperating() const {
    return activeState;
}
