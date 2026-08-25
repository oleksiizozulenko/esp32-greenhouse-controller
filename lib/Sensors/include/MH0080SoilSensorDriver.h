#ifndef MH0080_SOIL_SENSOR_DRIVER_H
#define MH0080_SOIL_SENSOR_DRIVER_H

#include <Arduino.h>
#include "ISoilSensor.h"

class MH0080SoilSensorDriver : public ISoilSensor {
private:
    int pin;

public:
    explicit MH0080SoilSensorDriver(int analogPin);
    ~MH0080SoilSensorDriver() override = default;

    bool begin() override;
    SensorReadResult<float> read() override;
};

#endif // MH0080_SOIL_SENSOR_DRIVER_H
