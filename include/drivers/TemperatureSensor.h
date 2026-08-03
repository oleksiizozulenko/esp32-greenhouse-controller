#ifndef TEMPERATURE_SENSOR_H
#define TEMPERATURE_SENSOR_H

#include <Arduino.h>
#include "Sensor.h"

class TemperatureSensor : public Sensor {
private:
    float lastTemperature;

public:
    TemperatureSensor(int pin) 
        : Sensor(pin, "Temperature"), lastTemperature(NAN) {}

    void init() override {
        pinMode(pin, INPUT);
    }

    SensorData read() override {
        unsigned long currentTime = millis();
        if (currentTime - lastReadTime < readInterval && !isnan(lastTemperature)) {
            return {lastTemperature, false};
        }

        lastReadTime = currentTime;

        float temperature = adcToPercentage(analogRead(pin));

        if (isnan(temperature)) {
            return {lastTemperature, true};
        } else {
            lastTemperature = temperature;
            return {temperature, false};
        }
    }

    const char* getUnit() const override {
        return "C";
    }
};

#endif // TEMPERATURE_SENSOR_H