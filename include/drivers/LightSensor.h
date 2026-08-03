#ifndef LIGHT_SENSOR_H
#define LIGHT_SENSOR_H

#include <Arduino.h>
#include "Sensor.h"

class LightSensor : public Sensor {
private:
    float lastLightLevel;

public:
    LightSensor(int pin) 
        : Sensor(pin, "Light"), lastLightLevel(NAN) {}

    void init() override {
        pinMode(pin, INPUT);
    }

    SensorData read() override {
        unsigned long currentTime = millis();
        if (currentTime - lastReadTime < readInterval && !isnan(lastLightLevel)) {
            bool isErr = isnan(lastLightLevel) || (lastLightLevel >= SENSOR_LIGHT_MAX_ERROR);
            return {lastLightLevel, isErr};
        }

        lastReadTime = currentTime;

        float lightLevel = adcToPercentage(analogRead(pin));

        if (isnan(lightLevel) || lightLevel >= SENSOR_LIGHT_MAX_ERROR) {
            lastLightLevel = lightLevel;
            return {lightLevel, true};
        } else {
            lastLightLevel = lightLevel;
            return {lightLevel, false};
        }
    }

    const char* getUnit() const override {
        return "";
    }
};

#endif // LIGHT_SENSOR_H