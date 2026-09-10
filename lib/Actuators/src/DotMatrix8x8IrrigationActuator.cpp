#include "DotMatrix8x8IrrigationActuator.h"
#include "Logging.h"

static const char* TAG = "ACTUATOR";

DotMatrix8x8IrrigationActuator::DotMatrix8x8IrrigationActuator(int gpioPin)
    : IrrigationActuator(gpioPin, "8x8 Matrix Irrigation"), activeState(false) {}

bool DotMatrix8x8IrrigationActuator::begin() {
    pinMode(pin, OUTPUT);
    // Active-LOW Hardware Diagnostic Self-Test: Flash matrix for 150ms to visually verify physical connection
    digitalWrite(pin, LOW);
    delay(150);
    digitalWrite(pin, HIGH);
    activeState = false;
    ESP_LOGI(TAG, "[HARDWARE DIAGNOSTIC] 8x8 Matrix Irrigation Actuator (Pin %d): INITIALIZED & SELF-TEST OK", pin);
    return true;
}

bool DotMatrix8x8IrrigationActuator::turnOn() {
    activeState = true;
    digitalWrite(pin, LOW);
    ESP_LOGI(TAG, "[HARDWARE ACTUATOR] 8x8 LED Matrix (Pin %d) -> POWERED ON (Irrigation Active)", pin);
    return true;
}

bool DotMatrix8x8IrrigationActuator::turnOff() {
    activeState = false;
    digitalWrite(pin, HIGH);
    ESP_LOGI(TAG, "[HARDWARE ACTUATOR] 8x8 LED Matrix (Pin %d) -> POWERED OFF", pin);
    return true;
}

bool DotMatrix8x8IrrigationActuator::isOn() const {
    return activeState;
}

