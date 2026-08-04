#ifndef VENTILATION_ACTUATOR_H
#define VENTILATION_ACTUATOR_H

#include <Arduino.h>
#include <ESP32Servo.h>
#include "../config.h"
#include "Actuator.h"

class VentilationActuator : public Actuator {
private:
    Servo servo;
    bool active;
    int openAngle;
    int closeAngle;

public:
    VentilationActuator(int pin, int openAngle = SERVO_OPEN_ANGLE, int closeAngle = SERVO_CLOSE_ANGLE)
        : Actuator(pin, ActuatorType::VENTILATION, "Ventilation"), active(false), openAngle(openAngle), closeAngle(closeAngle) {}

    void init() override {
        servo.attach(pin, 500, 2400);
        turnOff();
    }

    void turnOn() override {
        servo.write(openAngle);
        active = true;
    }

    void turnOff() override {
        servo.write(closeAngle);
        active = false;
    }

    bool isOn() override {
        return active;
    }

    const char* getStatusText() override {
        return active ? "OPEN" : "CLOSE";
    }

    void open() {
        turnOn();
    }

    void close() {
        turnOff();
    }
};

#endif // VENTILATION_ACTUATOR_H