#ifndef MOCK_ACTUATOR_H
#define MOCK_ACTUATOR_H

#include "IBinaryActuator.h"

class MockActuator : public IBinaryActuator {
private:
    int pin;
    ActuatorType type;
    const char* name;
    bool state;
    int turnOnCalls;
    int turnOffCalls;
    int initCalls;

public:
    MockActuator(int pin, const char* name)
        : pin(pin), type(ActuatorType::UNKNOWN), name(name), state(false), turnOnCalls(0), turnOffCalls(0), initCalls(0) {}

    MockActuator(int pin, ActuatorType type, const char* name)
        : pin(pin), type(type), name(name), state(false), turnOnCalls(0), turnOffCalls(0), initCalls(0) {}

    bool begin() override {
        initCalls++;
        return true;
    }

    bool turnOn() override {
        state = true;
        turnOnCalls++;
        return true;
    }

    bool turnOff() override {
        state = false;
        turnOffCalls++;
        return true;
    }

    bool isOn() const override {
        return state;
    }

    int getPin() const override { return pin; }
    ActuatorType getType() const override { return type; }
    const char* getName() const override { return name; }
    const char* getStatusText() const override { return state ? "ON" : "OFF"; }

    void setState(bool s) { state = s; }
    int getTurnOnCalls() const { return turnOnCalls; }
    int getTurnOffCalls() const { return turnOffCalls; }
    int getInitCalls() const { return initCalls; }
    void resetCallCounts() {
        turnOnCalls = 0;
        turnOffCalls = 0;
        initCalls = 0;
    }
};

#endif // MOCK_ACTUATOR_H

