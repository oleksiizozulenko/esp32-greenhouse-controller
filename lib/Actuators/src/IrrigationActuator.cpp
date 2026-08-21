#include "IrrigationActuator.h"

IrrigationActuator::IrrigationActuator(int pin, int numPixels)
    : Actuator(pin, ActuatorType::IRRIGATION, "Irrigation"),
      pixels(numPixels, pin, NEO_GRB + NEO_KHZ800),
      active(false) {}

void IrrigationActuator::init() {
    pixels.begin();
    pixels.clear();
    pixels.show();
    active = false;
}

void IrrigationActuator::turnOn() {
    for (uint16_t i = 0; i < pixels.numPixels(); i++) {
        pixels.setPixelColor(i, pixels.Color(0, 150, 255));
    }
    pixels.show();
    active = true;
}

void IrrigationActuator::turnOff() {
    pixels.clear();
    pixels.show();
    active = false;
}

bool IrrigationActuator::isOn() {
    return active;
}
