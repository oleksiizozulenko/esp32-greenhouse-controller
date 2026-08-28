#include "ServoVentilationActuator.h"

ServoVentilationActuator::ServoVentilationActuator(int gpioPin, int openAngle, int closeAngle)
    : VentilationActuator(gpioPin, "Servo Ventilation"), active(false), openAngle(openAngle), closeAngle(closeAngle) {}

void ServoVentilationActuator::init() {
#ifndef UNIT_TEST
    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);
    ESP32PWM::allocateTimer(2);
    ESP32PWM::allocateTimer(3);
    servo.setPeriodHertz(50);
    servo.attach(pin, 500, 2400);
    delay(250);
    servo.write(closeAngle);
#endif
    active = false;
}

void ServoVentilationActuator::turnOn() {
#ifndef UNIT_TEST
    if (!servo.attached()) {
        servo.attach(pin, 500, 2400);
    }
    servo.write(openAngle);
    delay(150);
#endif
    active = true;
}

void ServoVentilationActuator::turnOff() {
#ifndef UNIT_TEST
    if (!servo.attached()) {
        servo.attach(pin, 500, 2400);
    }
    servo.write(closeAngle);
    delay(150);
#endif
    active = false;
}

bool ServoVentilationActuator::isOn() {
    return active;
}

const char* ServoVentilationActuator::getStatusText() {
    return active ? "OPEN" : "CLOSE";
}
