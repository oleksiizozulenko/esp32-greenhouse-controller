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
    HumiditySensor(int pin, DHT* externalDht = nullptr, uint8_t dhtType = DHT_TYPE);
    ~HumiditySensor() override;

    void init() override;
    SensorData read() override;
    const char* getUnit() const override;
};

#endif // HUMIDITY_SENSOR_H