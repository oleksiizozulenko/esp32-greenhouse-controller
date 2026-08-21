#ifndef VENTILATION_ACTUATOR_H
#define VENTILATION_ACTUATOR_H

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

#include "config.h"
#include "Actuator.h"

class VentilationActuator : public Actuator {
private:
    Servo servo;
    bool active;
    int openAngle;
    int closeAngle;

public:
    VentilationActuator(int pin, int openAngle = SERVO_OPEN_ANGLE, int closeAngle = SERVO_CLOSE_ANGLE);

    void init() override;
    void turnOn() override;
    void turnOff() override;
    bool isOn() override;
    const char* getStatusText() override;
    void open();
    void close();
};

#endif // VENTILATION_ACTUATOR_H