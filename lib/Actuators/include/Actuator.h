#ifndef ACTUATOR_H
#define ACTUATOR_H

#include <Arduino.h>


enum class ActuatorType {
    UNKNOWN = 0,
    VENTILATION,
    IRRIGATION,
    LIGHT
};


class Actuator {
protected:
    int pin;
    ActuatorType type;
    const char* name;

    Actuator(int pin, ActuatorType type = ActuatorType::UNKNOWN, const char* name = "Actuator");

public:
    virtual ~Actuator();

    virtual void init() = 0;
    virtual void turnOn() = 0;
    virtual void turnOff() = 0;
    virtual bool isOn() = 0;

    virtual const char* getStatusText();

    int getPin() const { return pin; }
    ActuatorType getType() const { return type; }
    const char* getName() const { return name; }
};

#endif // ACTUATOR_H