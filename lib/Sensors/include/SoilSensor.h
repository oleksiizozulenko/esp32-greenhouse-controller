#ifndef SOIL_SENSOR_H
#define SOIL_SENSOR_H

#include <Arduino.h>
#include "Sensor.h"

class SoilSensor : public Sensor {
private:
    float lastSoilMoisture;

public:
    SoilSensor(int pin);

    void init() override;
    SensorData read() override;
    const char* getUnit() const override;
};

#endif // SOIL_SENSOR_H
