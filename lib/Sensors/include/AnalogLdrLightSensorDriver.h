#ifndef ANALOG_LDR_LIGHT_SENSOR_DRIVER_H
#define ANALOG_LDR_LIGHT_SENSOR_DRIVER_H

#include <Arduino.h>
#include "ILightSensor.h"

class AnalogLdrLightSensorDriver : public ILightSensor {
private:
    int pin;

public:
    explicit AnalogLdrLightSensorDriver(int analogPin);
    ~AnalogLdrLightSensorDriver() override = default;

    bool begin() override;
    SensorReadResult<float> read() override;
};

#endif // ANALOG_LDR_LIGHT_SENSOR_DRIVER_H
