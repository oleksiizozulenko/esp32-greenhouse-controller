#ifndef DOT_MATRIX_8X8_IRRIGATION_ACTUATOR_H
#define DOT_MATRIX_8X8_IRRIGATION_ACTUATOR_H

#include <Arduino.h>
#include "IrrigationActuator.h"

class DotMatrix8x8IrrigationActuator : public IrrigationActuator {
private:
    bool activeState;

public:
    explicit DotMatrix8x8IrrigationActuator(int gpioPin);
    ~DotMatrix8x8IrrigationActuator() override = default;

    bool begin() override;
    bool turnOn() override;
    bool turnOff() override;
    bool isOn() const override;
};

#endif // DOT_MATRIX_8X8_IRRIGATION_ACTUATOR_H

