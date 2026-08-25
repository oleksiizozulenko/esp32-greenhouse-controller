#ifndef HAL_ILIGHT_SENSOR_H
#define HAL_ILIGHT_SENSOR_H

#include "ISensor.h"

class ILightSensor : public ISensor<float> {
public:
    virtual ~ILightSensor() = default;
    // Inherits begin() and read() returning SensorReadResult<float> in Lux
};

#endif // HAL_ILIGHT_SENSOR_H
