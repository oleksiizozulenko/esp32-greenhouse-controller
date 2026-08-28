#include "ServoVentilationActuator.h"

ServoVentilationActuator::ServoVentilationActuator(int gpioPin, int openAngle, int closeAngle)
    : VentilationActuator(gpioPin, "Servo Ventilation"), active(false), openAngle(openAngle), closeAngle(closeAngle), currentAngle(closeAngle) {}

bool ServoVentilationActuator::begin() {
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
    currentAngle = closeAngle;
    return true;
}

bool ServoVentilationActuator::turnOn() {
#ifndef UNIT_TEST
    if (!servo.attached()) {
        servo.attach(pin, 500, 2400);
    }
    servo.write(openAngle);
    delay(150);
#endif
    active = true;
    currentAngle = openAngle;
    return true;
}

bool ServoVentilationActuator::turnOff() {
#ifndef UNIT_TEST
    if (!servo.attached()) {
        servo.attach(pin, 500, 2400);
    }
    servo.write(closeAngle);
    delay(150);
#endif
    active = false;
    currentAngle = closeAngle;
    return true;
}

bool ServoVentilationActuator::isOperating() const {
    return active;
}

bool ServoVentilationActuator::setAngleDegrees(float angle) {
    int target = static_cast<int>(angle);
#ifndef UNIT_TEST
    if (!servo.attached()) {
        servo.attach(pin, 500, 2400);
    }
    servo.write(target);
    delay(150);
#endif
    currentAngle = target;
    active = (currentAngle != closeAngle);
    return true;
}

bool ServoVentilationActuator::setPositionPercent(float percent0to100) {
    if (percent0to100 < 0.0f) percent0to100 = 0.0f;
    if (percent0to100 > 100.0f) percent0to100 = 100.0f;
    float targetAngle = closeAngle + (percent0to100 / 100.0f) * (openAngle - closeAngle);
    return setAngleDegrees(targetAngle);
}

float ServoVentilationActuator::getPositionPercent() const {
    if (openAngle == closeAngle) return 0.0f;
    float pct = (static_cast<float>(currentAngle - closeAngle) / static_cast<float>(openAngle - closeAngle)) * 100.0f;
    if (pct < 0.0f) return 0.0f;
    if (pct > 100.0f) return 100.0f;
    return pct;
}

const char* ServoVentilationActuator::getStatusText() const {
    return active ? "OPEN" : "CLOSE";
}

