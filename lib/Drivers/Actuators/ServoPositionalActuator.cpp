#include "ServoPositionalActuator.h"

ServoPositionalActuator::ServoPositionalActuator(int gpioPin)
    : pin(gpioPin), currentAngle(0.0f), operating(false) {}

bool ServoPositionalActuator::begin() {
#ifndef UNIT_TEST
    servo.attach(pin);
#endif
    setAngleDegrees(0.0f);
    return true;
}

bool ServoPositionalActuator::turnOff() {
    return setAngleDegrees(0.0f);
}

bool ServoPositionalActuator::isOperating() const {
    return operating;
}

bool ServoPositionalActuator::setAngleDegrees(float angle) {
    if (angle < 0.0f) angle = 0.0f;
    if (angle > 180.0f) angle = 180.0f;

#ifndef UNIT_TEST
    servo.write(static_cast<int>(angle));
#endif
    currentAngle = angle;
    operating = (angle > 0.0f);
    return true;
}


bool ServoPositionalActuator::setPositionPercent(float percent0to100) {
    if (percent0to100 < 0.0f) percent0to100 = 0.0f;
    if (percent0to100 > 100.0f) percent0to100 = 100.0f;

    float angle = (percent0to100 / 100.0f) * 180.0f;
    return setAngleDegrees(angle);
}

float ServoPositionalActuator::getPositionPercent() const {
    return (currentAngle / 180.0f) * 100.0f;
}
