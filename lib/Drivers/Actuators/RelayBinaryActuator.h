#ifndef RELAY_BINARY_ACTUATOR_H
#define RELAY_BINARY_ACTUATOR_H

#include <Arduino.h>
#include "Actuators/IBinaryActuator.h"

class RelayBinaryActuator : public IBinaryActuator {
private:
    int pin;
    bool activeLow;
    bool state;

public:
    RelayBinaryActuator(int gpioPin, bool isActiveLow = false);
    ~RelayBinaryActuator() override = default;

    bool begin() override;
    bool turnOn() override;
    bool turnOff() override;
    bool isOn() const override;
    bool isOperating() const override;
};

#endif // RELAY_BINARY_ACTUATOR_H
