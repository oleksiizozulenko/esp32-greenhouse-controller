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
    TemperatureSensor(int pin, DHT* externalDht = nullptr, uint8_t dhtType = DHT_TYPE);
    ~TemperatureSensor() override;

    void init() override;
    SensorData read() override;
    const char* getUnit() const override;
};

#endif // TEMPERATURE_SENSOR_H