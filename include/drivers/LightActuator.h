#ifndef LIGHT_ACTUATOR_H
#define LIGHT_ACTUATOR_H

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "../config.h"
#include "Actuator.h"

class LightActuator : public Actuator {
private:
    Adafruit_NeoPixel pixels;
    bool active;

public:
    LightActuator(int pin = PIN_LED_STRIP, int numPixels = NUM_PIXELS_STRIP)
        : Actuator(pin, ActuatorType::LIGHT, "Light"),
          pixels(numPixels, pin, NEO_GRB + NEO_KHZ800),
          active(false) {}

    void init() override {
        pixels.begin();
        pixels.clear();
        pixels.show();
        active = false;
    }

    void turnOn() override {
        // Warm white/yellow grow light color for LED_STRIP
        for (uint16_t i = 0; i < pixels.numPixels(); i++) {
            pixels.setPixelColor(i, pixels.Color(255, 255, 200));
        }
        pixels.show();
        active = true;
    }

    void turnOff() override {
        pixels.clear();
        pixels.show();
        active = false;
    }

    bool isOn() override {
        return active;
    }
};

#endif // LIGHT_ACTUATOR_H
