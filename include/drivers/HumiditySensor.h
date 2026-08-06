#ifndef HUMIDITY_SENSOR_H
#define HUMIDITY_SENSOR_H

#include <Arduino.h>
#include <DHT.h>
#include "Sensor.h"

class HumiditySensor : public Sensor {
private:
    DHT* dht;
    bool isExternalDht;
    float lastValidHumidity;
    unsigned long lastValidTime;

public:
    HumiditySensor(int pin, DHT* externalDht = nullptr, uint8_t dhtType = DHT_TYPE)
        : Sensor(pin, SensorType::HUMIDITY, "Humidity"), dht(nullptr), isExternalDht(false),
          lastValidHumidity(NAN), lastValidTime(0) {
        if (externalDht != nullptr) {
            dht = externalDht;
            isExternalDht = true;
        } else {
            dht = new DHT(pin, dhtType);
            isExternalDht = false;
        }
    }

    ~HumiditySensor() override {
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
        if (currentTime - lastReadTime < readInterval && !isnan(lastValidHumidity)) {
            bool isErr = isnan(lastValidHumidity) || (lastValidHumidity < SENSOR_HUMIDITY_MIN_ERROR) || (lastValidHumidity > SENSOR_HUMIDITY_MAX_ERROR);
            return {lastValidHumidity, isErr};
        }

        lastReadTime = currentTime;

        float humidity = NAN;
        for (uint8_t attempt = 0; attempt < DIGITAL_SENSOR_MAX_RETRIES; ++attempt) {
            humidity = dht ? dht->readHumidity() : NAN;
            if (!isnan(humidity)) {
                lastValidHumidity = humidity;
                lastValidTime = currentTime;
                return {humidity, false};
            }
            if (attempt < DIGITAL_SENSOR_MAX_RETRIES - 1) {
                delay(DIGITAL_SENSOR_RETRY_DELAY_MS);
            }
        }

        if (!isnan(lastValidHumidity) && (currentTime - lastValidTime <= DIGITAL_SENSOR_FALLBACK_TIMEOUT)) {
            return {lastValidHumidity, false};
        }

        return {humidity, true};
    }

    const char* getUnit() const override {
        return "%";
    }
};

#endif // HUMIDITY_SENSOR_H