#ifndef ANALOG_SOIL_SENSOR_DRIVER_H
#define ANALOG_SOIL_SENSOR_DRIVER_H

#include <Arduino.h>
#include "Sensors/ISoilSensor.h"

class AnalogSoilSensorDriver : public ISoilSensor {
private:
    int pin;

public:
    explicit AnalogSoilSensorDriver(int analogPin);
    ~AnalogSoilSensorDriver() override = default;

    bool begin() override;
    SensorReadResult<float> read() override;
};

#endif // ANALOG_SOIL_SENSOR_DRIVER_H
