#ifndef LIGHT_SENSOR_H
#define LIGHT_SENSOR_H

#include <Arduino.h>
#include <math.h>
#include "Sensor.h"

class LightSensor : public Sensor {
private:
    float lastLightLevel;

public:
    LightSensor(int pin);

    void init() override;
    SensorData read() override;
    const char* getUnit() const override;
};

#endif // LIGHT_SENSOR_H