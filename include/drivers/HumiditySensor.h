#ifndef HUMIDITY_SENSOR_H
#define HUMIDITY_SENSOR_H

#include <Arduino.h>
#include <DHT.h>
#include "Sensor.h"

class HumiditySensor : public Sensor {
private:
    DHT* dht;
    bool isExternalDht;
    float lastHumidity;

public:
    HumiditySensor(int pin, DHT* externalDht = nullptr, uint8_t dhtType = DHT_TYPE) 
        : Sensor(pin, "Humidity"), dht(nullptr), isExternalDht(false), lastHumidity(NAN) {
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
        if (currentTime - lastReadTime < readInterval && !isnan(lastHumidity)) {
            bool isErr = isnan(lastHumidity) || (lastHumidity > SENSOR_HUMIDITY_MAX_ERROR);
            return {lastHumidity, isErr};
        }

        lastReadTime = currentTime;

        float humidity = dht ? dht->readHumidity() : NAN;

        if (isnan(humidity) || humidity > SENSOR_HUMIDITY_MAX_ERROR) {
            lastHumidity = humidity;
            return {humidity, true};
        } else {
            lastHumidity = humidity;
            return {humidity, false};
        }
    }

    const char* getUnit() const override {
        return "%";
    }
};

#endif // HUMIDITY_SENSOR_H