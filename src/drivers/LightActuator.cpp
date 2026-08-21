#include "drivers/LightActuator.h"

LightActuator::LightActuator(int pin, int numPixels)
    : Actuator(pin, ActuatorType::LIGHT, "Light"),
      pixels(numPixels, pin, NEO_GRB + NEO_KHZ800),
      active(false) {}

void LightActuator::init() {
    pixels.begin();
    pixels.clear();
    pixels.show();
    active = false;
}

void LightActuator::turnOn() {
    for (uint16_t i = 0; i < pixels.numPixels(); i++) {
        pixels.setPixelColor(i, pixels.Color(255, 255, 200));
    }
    pixels.show();
    active = true;
}

void LightActuator::turnOff() {
    pixels.clear();
    pixels.show();
    active = false;
}

bool LightActuator::isOn() {
    return active;
}
