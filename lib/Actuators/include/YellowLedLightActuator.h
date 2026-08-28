#ifndef YELLOW_LED_LIGHT_ACTUATOR_H
#define YELLOW_LED_LIGHT_ACTUATOR_H

#include <Arduino.h>
#include "LightActuator.h"

class YellowLedLightActuator : public LightActuator {
private:
    bool state;

public:
    explicit YellowLedLightActuator(int gpioPin);
    ~YellowLedLightActuator() override = default;

    bool begin() override;
    bool turnOn() override;
    bool turnOff() override;
    bool isOn() const override;
};

#endif // YELLOW_LED_LIGHT_ACTUATOR_H

