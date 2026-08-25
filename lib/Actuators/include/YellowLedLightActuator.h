#ifndef YELLOW_LED_LIGHT_ACTUATOR_H
#define YELLOW_LED_LIGHT_ACTUATOR_H

#include <Arduino.h>
#include "IBinaryActuator.h"

class YellowLedLightActuator : public IBinaryActuator {
private:
    int pin;
    bool state;

public:
    explicit YellowLedLightActuator(int gpioPin);
    ~YellowLedLightActuator() override = default;

    bool begin() override;
    bool turnOn() override;
    bool turnOff() override;
    bool isOn() const override;
    bool isOperating() const override;
};

#endif // YELLOW_LED_LIGHT_ACTUATOR_H
