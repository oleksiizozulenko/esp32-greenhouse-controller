#ifndef DOT_MATRIX_8X8_IRRIGATION_ACTUATOR_H
#define DOT_MATRIX_8X8_IRRIGATION_ACTUATOR_H

#include <Arduino.h>
#include "IBinaryActuator.h"

class DotMatrix8x8IrrigationActuator : public IBinaryActuator {
private:
    int dataPin;
    int clockPin;
    int csPin;
    bool activeState;

public:
    DotMatrix8x8IrrigationActuator(int dinPin, int clkPin, int csPin);
    ~DotMatrix8x8IrrigationActuator() override = default;

    bool begin() override;
    bool turnOn() override;
    bool turnOff() override;
    bool isOn() const override;
    bool isOperating() const override;
};

#endif // DOT_MATRIX_8X8_IRRIGATION_ACTUATOR_H
