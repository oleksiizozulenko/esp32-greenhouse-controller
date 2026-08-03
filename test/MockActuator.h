#ifndef MOCK_ACTUATOR_H
#define MOCK_ACTUATOR_H

#include "../include/drivers/Actuator.h"

class MockActuator : public Actuator {
private:
    bool state;
    int turnOnCalls;
    int turnOffCalls;
    int initCalls;

public:
    MockActuator(int pin, const char* name)
        : Actuator(pin, name), state(false), turnOnCalls(0), turnOffCalls(0), initCalls(0) {}

    void init() override {
        initCalls++;
    }

    void turnOn() override {
        state = true;
        turnOnCalls++;
    }

    void turnOff() override {
        state = false;
        turnOffCalls++;
    }

    bool isOn() override {
        return state;
    }

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
