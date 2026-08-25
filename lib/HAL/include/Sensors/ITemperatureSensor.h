#ifndef HAL_ITEMPERATURE_SENSOR_H
#define HAL_ITEMPERATURE_SENSOR_H

#include "ISensor.h"

class ITemperatureSensor : public ISensor<float> {
public:
    virtual ~ITemperatureSensor() = default;
    // Inherits begin() and read() returning SensorReadResult<float> in Degrees Celsius
};

#endif // HAL_ITEMPERATURE_SENSOR_H
