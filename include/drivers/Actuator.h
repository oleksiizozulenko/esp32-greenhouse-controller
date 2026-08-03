#ifndef ACTUATOR_H
#define ACTUATOR_H

#include <Arduino.h>

class Actuator {
protected:
    int pin;
    const char* name;

    Actuator(int pin, const char* name = "Actuator") : pin(pin), name(name) {}

public:
    virtual ~Actuator() {}

    virtual void init() = 0;
    virtual void turnOn() = 0;
    virtual void turnOff() = 0;
    virtual bool isOn() = 0;

    virtual const char* getStatusText() { return isOn() ? "ON" : "OFF"; }

    int getPin() const { return pin; }
    const char* getName() const { return name; }
};

#endif // ACTUATOR_H