#ifndef LDR_LIGHT_SENSOR_DRIVER_H
#define LDR_LIGHT_SENSOR_DRIVER_H

#include <Arduino.h>
#include "Sensors/ILightSensor.h"

class LDRLightSensorDriver : public ILightSensor {
private:
    int pin;

public:
    explicit LDRLightSensorDriver(int analogPin);
    ~LDRLightSensorDriver() override = default;

    bool begin() override;
    SensorReadResult<float> read() override;
};

#endif // LDR_LIGHT_SENSOR_DRIVER_H
