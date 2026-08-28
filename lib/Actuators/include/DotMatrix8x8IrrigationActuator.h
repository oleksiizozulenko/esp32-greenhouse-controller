#ifndef DOT_MATRIX_8X8_IRRIGATION_ACTUATOR_H
#define DOT_MATRIX_8X8_IRRIGATION_ACTUATOR_H

#include <Arduino.h>
#include "IrrigationActuator.h"

class DotMatrix8x8IrrigationActuator : public IrrigationActuator {
private:
    int dataPin;
    bool activeState;

public:
    explicit DotMatrix8x8IrrigationActuator(int gpioPin);
    ~DotMatrix8x8IrrigationActuator() override = default;

    void init() override;
    bool begin() { init(); return true; }
    void turnOn() override;
    void turnOff() override;
    bool isOn() override;
    bool isOperating() const { return activeState; }
};

#endif // DOT_MATRIX_8X8_IRRIGATION_ACTUATOR_H
