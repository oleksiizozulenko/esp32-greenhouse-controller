#ifndef DHT11_ENVIRONMENT_SENSOR_H
#define DHT11_ENVIRONMENT_SENSOR_H

#include <Arduino.h>
#ifndef UNIT_TEST
#include <DHT.h>
#endif
#include "ITemperatureSensor.h"
#include "IHumiditySensor.h"

class DHT11EnvironmentSensor : public ITemperatureSensor, public IHumiditySensor {
private:
    int pin;
#ifndef UNIT_TEST
    DHT dht;
#endif
    float lastValidTemp;
    float lastValidHum;
    uint32_t lastReadTime;

public:
    explicit DHT11EnvironmentSensor(int dataPin);
    ~DHT11EnvironmentSensor() override = default;

    bool begin() override;
    SensorReadResult<float> read() override;
    SensorReadResult<float> readHumidity();
};

#endif // DHT11_ENVIRONMENT_SENSOR_H
