#ifndef DHT22_ENVIRONMENT_SENSOR_H
#define DHT22_ENVIRONMENT_SENSOR_H

#include <Arduino.h>
#include <DHT.h>
#include "Sensors/ITemperatureSensor.h"
#include "Sensors/IHumiditySensor.h"

class DHT22EnvironmentSensor : public ITemperatureSensor, public IHumiditySensor {
private:
    int pin;
    DHT dht;
    float lastValidTemp;
    float lastValidHum;
    uint32_t lastReadTime;

public:
    explicit DHT22EnvironmentSensor(int dataPin);
    ~DHT22EnvironmentSensor() override = default;

    bool begin() override;
    SensorReadResult<float> read() override; // Implements ITemperatureSensor::read()
    SensorReadResult<float> readHumidity();   // Specific humidity read helper
};

#endif // DHT22_ENVIRONMENT_SENSOR_H
