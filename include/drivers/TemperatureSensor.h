#ifndef TEMPERATURE_SENSOR_H
#define TEMPERATURE_SENSOR_H

#include <Arduino.h>
#include <DHT.h>
#include "Sensor.h"

class TemperatureSensor : public Sensor {
private:
    DHT* dht;
    bool isExternalDht;
    float lastValidTemperature;
    unsigned long lastValidTime;

public:
    TemperatureSensor(int pin, DHT* externalDht = nullptr, uint8_t dhtType = DHT_TYPE)
        : Sensor(pin, SensorType::TEMPERATURE, "Temperature"), dht(nullptr), isExternalDht(false),
          lastValidTemperature(NAN), lastValidTime(0) {
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
        if (currentTime - lastReadTime < readInterval && !isnan(lastValidTemperature)) {
            bool isErr = isnan(lastValidTemperature) || (lastValidTemperature < SENSOR_TEMP_MIN_ERROR) || (lastValidTemperature > SENSOR_TEMP_MAX_ERROR);
            return {lastValidTemperature, isErr};
        }

        lastReadTime = currentTime;

        float temp = NAN;
        for (uint8_t attempt = 0; attempt < DIGITAL_SENSOR_MAX_RETRIES; ++attempt) {
            temp = dht ? dht->readTemperature() : NAN;
            if (!isnan(temp)) {
                lastValidTemperature = temp;
                lastValidTime = currentTime;
                return {temp, false};
            }
            if (attempt < DIGITAL_SENSOR_MAX_RETRIES - 1) {
                delay(DIGITAL_SENSOR_RETRY_DELAY_MS);
            }
        }

        if (!isnan(lastValidTemperature) && (currentTime - lastValidTime <= DIGITAL_SENSOR_FALLBACK_TIMEOUT)) {
            return {lastValidTemperature, false};
        }

        return {temp, true};
    }

    const char* getUnit() const override {
        return "C";
    }
};

#endif // TEMPERATURE_SENSOR_H