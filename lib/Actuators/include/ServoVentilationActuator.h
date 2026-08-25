#ifndef SERVO_VENTILATION_ACTUATOR_H
#define SERVO_VENTILATION_ACTUATOR_H

#include <Arduino.h>
#ifndef UNIT_TEST
#include <ESP32Servo.h>
#endif
#include "IPositionalActuator.h"

class ServoVentilationActuator : public IPositionalActuator {
private:
    int pin;
#ifndef UNIT_TEST
    Servo servo;
#endif
    float currentAngle;
    bool operating;

public:
    explicit ServoVentilationActuator(int gpioPin);
    ~ServoVentilationActuator() override = default;

    bool begin() override;
    bool turnOff() override;
    bool isOperating() const override;
    bool setAngleDegrees(float angle) override;
    bool setPositionPercent(float percent0to100) override;
    float getPositionPercent() const override;
};

#endif // SERVO_VENTILATION_ACTUATOR_H
