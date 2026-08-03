#ifndef MOCK_SENSOR_H
#define MOCK_SENSOR_H

#include "../include/drivers/Sensor.h"

class MockSensor : public Sensor {
private:
    SensorData currentData;
    const char* unit;
    int initCalls;
    int readCalls;

public:
    MockSensor(int pin, const char* name, const char* unit = "units", unsigned long readInterval = 2000)
        : Sensor(pin, name, readInterval), currentData({0.0f, false}), unit(unit), initCalls(0), readCalls(0) {}

    void init() override {
        initCalls++;
    }

    SensorData read() override {
        readCalls++;
        return currentData;
    }

    const char* getUnit() const override {
        return unit;
    }

    void setData(float value, bool isError = false) {
        currentData.value = value;
        currentData.isError = isError;
    }

    int getInitCalls() const { return initCalls; }
    int getReadCalls() const { return readCalls; }

    static float testAdcToVoltage(int rawAdc) {
        return Sensor::adcToVoltage(rawAdc);
    }

    static float testAdcToPercentage(int rawAdc) {
        return Sensor::adcToPercentage(rawAdc);
    }
};

#endif // MOCK_SENSOR_H
