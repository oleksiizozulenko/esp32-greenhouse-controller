#include "DotMatrix8x8IrrigationActuator.h"

DotMatrix8x8IrrigationActuator::DotMatrix8x8IrrigationActuator(int gpioPin)
    : IrrigationActuator(gpioPin, "8x8 Matrix Irrigation"), activeState(false) {}


void DotMatrix8x8IrrigationActuator::init() {
    pinMode(dataPin, OUTPUT);

    turnOff();
}

void DotMatrix8x8IrrigationActuator::turnOn() {
    activeState = true;
    digitalWrite(dataPin, HIGH);

}

void DotMatrix8x8IrrigationActuator::turnOff() {
    activeState = false;
    digitalWrite(dataPin, LOW);

}

bool DotMatrix8x8IrrigationActuator::isOn() {
    return activeState;
}
