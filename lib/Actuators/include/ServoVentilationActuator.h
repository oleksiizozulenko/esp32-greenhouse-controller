#ifndef SERVO_VENTILATION_ACTUATOR_H
#define SERVO_VENTILATION_ACTUATOR_H

#include <Arduino.h>
#ifndef UNIT_TEST
#include <ESP32Servo.h>
#else
class Servo {
public:
    void setPeriodHertz(int) {}
    void attach(int, int, int) {}
    bool attached() { return true; }
    void write(int) {}
};
namespace ESP32PWM {
    inline void allocateTimer(int) {}
}
#endif

#include "VentilationActuator.h"

class ServoVentilationActuator : public VentilationActuator {
private:
#ifndef UNIT_TEST
    Servo servo;
#endif
    bool active;
    int openAngle;
    int closeAngle;
    int currentAngle;

public:
    explicit ServoVentilationActuator(int gpioPin, int openAngle = 90, int closeAngle = 0);
    ~ServoVentilationActuator() override = default;

    bool begin() override;
    bool turnOn() override;
    bool turnOff() override;
    bool isOperating() const override;

    bool setAngleDegrees(float angle) override;
    bool setPositionPercent(float percent0to100) override;
    float getPositionPercent() const override;

    const char* getStatusText() const override;
};

#endif // SERVO_VENTILATION_ACTUATOR_H

