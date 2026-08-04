#ifndef TEMPERATURE_SENSOR_H
#define TEMPERATURE_SENSOR_H

#include <Arduino.h>
#include <DHT.h>
#include "Sensor.h"

class TemperatureSensor : public Sensor {
private:
    DHT* dht;
    bool isExternalDht;
    float lastTemperature;

public:
    TemperatureSensor(int pin, DHT* externalDht = nullptr, uint8_t dhtType = DHT_TYPE) 
        : Sensor(pin, SensorType::TEMPERATURE, "Temperature"), dht(nullptr), isExternalDht(false), lastTemperature(NAN) {
        if (externalDht != nullptr) {
            dht = externalDht;
            isExternalDht = true;
        } else {
            dht = new DHT(pin, dhtType);
            isExternalDht = false;
        }
    }

    ~TemperatureSensor() override {
        if (!isExternalDht && dht != nullptr) {
            delete dht;
            dht = nullptr;
        }
    }

    void init() override {
        pinMode(pin, INPUT);
        if (dht != nullptr) {
            dht->begin();
        }
    }

    SensorData read() override {
        unsigned long currentTime = millis();
        if (currentTime - lastReadTime < readInterval && !isnan(lastTemperature)) {
            bool isErr = isnan(lastTemperature) || (lastTemperature < SENSOR_TEMP_MIN_ERROR);
            return {lastTemperature, isErr};
        }

        lastReadTime = currentTime;

        float temperature = dht ? dht->readTemperature() : NAN;

        if (isnan(temperature)) {
            lastTemperature = temperature;
            return {temperature, true};
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