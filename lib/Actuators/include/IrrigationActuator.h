#ifndef IRRIGATION_ACTUATOR_H
#define IRRIGATION_ACTUATOR_H

#include <Arduino.h>

#ifndef UNIT_TEST
#include <Adafruit_NeoPixel.h>
#else
#ifndef NEO_GRB
#define NEO_GRB 0
#endif
#ifndef NEO_KHZ800
#define NEO_KHZ800 0
#endif
class Adafruit_NeoPixel {
public:
    Adafruit_NeoPixel(uint16_t n = 0, int16_t p = 0, uint8_t t = 0) {}
    void begin() {}
    void clear() {}
    void show() {}
    void setPixelColor(uint16_t n, uint32_t c) {}
    static uint32_t Color(uint8_t r, uint8_t g, uint8_t b) { return 0; }
    uint16_t numPixels() const { return 0; }
};
#endif

#include "config.h"
#include "Actuator.h"

class IrrigationActuator : public Actuator {
private:
    Adafruit_NeoPixel pixels;
    bool active;

public:
    IrrigationActuator(int pin = PIN_LED_RING, int numPixels = NUM_PIXELS_RING);

    void init() override;
    void turnOn() override;
    void turnOff() override;
    bool isOn() override;
};

#endif // IRRIGATION_ACTUATOR_H