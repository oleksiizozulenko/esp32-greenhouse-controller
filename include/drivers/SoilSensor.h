#ifndef SOIL_SENSOR_H
#define SOIL_SENSOR_H

#include <Arduino.h>
#include "Sensor.h"

class SoilSensor : public Sensor {
private:
    float lastSoilMoisture;

public:
    SoilSensor(int pin)
        : Sensor(pin, SensorType::SOIL, "Soil"), lastSoilMoisture(NAN) {}

    void init() override {
        pinMode(pin, INPUT);
    }

    SensorData read() override {
        unsigned long currentTime = millis();
        if (currentTime - lastReadTime < readInterval && !isnan(lastSoilMoisture)) {
            return {lastSoilMoisture, false};
        }

        lastReadTime = currentTime;

        float soilMoisture = adcToPercentage(analogRead(pin));

        if (isnan(soilMoisture)) {
            return {lastSoilMoisture, true};
        } else {
            lastSoilMoisture = soilMoisture;
            return {soilMoisture, false};
        }
    }

    const char* getUnit() const override {
        return "%";
    }
};

#endif // SOIL_SENSOR_H
