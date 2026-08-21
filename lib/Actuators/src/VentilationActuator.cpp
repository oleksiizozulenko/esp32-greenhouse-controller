#include "VentilationActuator.h"

VentilationActuator::VentilationActuator(int pin, int openAngle, int closeAngle)
    : Actuator(pin, ActuatorType::VENTILATION, "Ventilation"), active(false), openAngle(openAngle), closeAngle(closeAngle) {}

void VentilationActuator::init() {
    ESP32PWM::allocateTimer(0);
    ESP32PWM::allocateTimer(1);
    ESP32PWM::allocateTimer(2);
    ESP32PWM::allocateTimer(3);
    servo.setPeriodHertz(50);
    servo.attach(pin, 500, 2400);
    delay(250);
    servo.write(closeAngle);
    active = false;
}

void VentilationActuator::turnOn() {
    if (!servo.attached()) {
        servo.attach(pin, 500, 2400);
    }
    servo.write(openAngle);
    delay(150);
    active = true;
}

void VentilationActuator::turnOff() {
    if (!servo.attached()) {
        servo.attach(pin, 500, 2400);
    }
    servo.write(closeAngle);
    delay(150);
    active = false;
}

bool VentilationActuator::isOn() {
    return active;
}

const char* VentilationActuator::getStatusText() {
    return active ? "OPEN" : "CLOSE";
}

void VentilationActuator::open() {
    turnOn();
}

void VentilationActuator::close() {
    turnOff();
}
