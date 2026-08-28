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
#include "IPositionalActuator.h"

class ServoVentilationActuator : public VentilationActuator {
private:
#ifndef UNIT_TEST
    Servo servo;
#endif
    bool active;
    int openAngle;
    int closeAngle;

public:
    explicit ServoVentilationActuator(int gpioPin, int openAngle = 90, int closeAngle = 0);
    ~ServoVentilationActuator() override = default;

    void init() override;
    void turnOn() override;
    void turnOff() override;
    bool isOn() override;
    const char* getStatusText() override;
};

#endif // SERVO_VENTILATION_ACTUATOR_H
