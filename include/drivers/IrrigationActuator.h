#ifndef IRRIGATION_ACTUATOR_H
#define IRRIGATION_ACTUATOR_H

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "../config.h"
#include "Actuator.h"

class IrrigationActuator : public Actuator {
private:
    Adafruit_NeoPixel pixels;
    bool active;

public:
    IrrigationActuator(int pin = PIN_LED_RING, int numPixels = NUM_PIXELS_RING)
        : Actuator(pin, ActuatorType::IRRIGATION, "Irrigation"),
          pixels(numPixels, pin, NEO_GRB + NEO_KHZ800),
          active(false) {}

    void init() override {
        pixels.begin();
        pixels.clear();
        pixels.show();
        active = false;
    }

    void turnOn() override {
        // Blue/Cyan water color for LED_RING
        for (uint16_t i = 0; i < pixels.numPixels(); i++) {
            pixels.setPixelColor(i, pixels.Color(0, 150, 255));
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

#endif // IRRIGATION_ACTUATOR_H