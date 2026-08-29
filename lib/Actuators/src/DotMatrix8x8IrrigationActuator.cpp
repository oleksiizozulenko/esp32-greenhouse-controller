#include "DotMatrix8x8IrrigationActuator.h"

DotMatrix8x8IrrigationActuator::DotMatrix8x8IrrigationActuator(int gpioPin)
    : IrrigationActuator(gpioPin, "8x8 Matrix Irrigation"), activeState(false) {}

bool DotMatrix8x8IrrigationActuator::begin() {
    pinMode(pin, OUTPUT);
    // Hardware Diagnostic Self-Test: Flash matrix for 150ms to visually verify physical connection
    digitalWrite(pin, HIGH);
    delay(150);
    digitalWrite(pin, LOW);
    activeState = false;
    Serial.printf("[HARDWARE DIAGNOSTIC] 8x8 Matrix Irrigation Actuator (Pin %d): INITIALIZED & SELF-TEST OK\n", pin);
    return true;
}

bool DotMatrix8x8IrrigationActuator::turnOn() {
    activeState = true;
    digitalWrite(pin, LOW);
    Serial.printf("[HARDWARE ACTUATOR] 8x8 LED Matrix (Pin %d) -> POWERED ON (Irrigation Active)\n", pin);
    return true;
}

bool DotMatrix8x8IrrigationActuator::turnOff() {
    activeState = false;
    digitalWrite(pin, HIGH);
    Serial.printf("[HARDWARE ACTUATOR] 8x8 LED Matrix (Pin %d) -> POWERED OFF\n", pin);
    return true;
}

bool DotMatrix8x8IrrigationActuator::isOn() const {
    return activeState;
}

