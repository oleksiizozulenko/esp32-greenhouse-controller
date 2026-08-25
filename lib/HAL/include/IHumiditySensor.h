#ifndef HAL_IHUMIDITY_SENSOR_H
#define HAL_IHUMIDITY_SENSOR_H

#include "ISensor.h"

class IHumiditySensor : public ISensor<float> {
public:
    virtual ~IHumiditySensor() = default;
    // Inherits begin() and read() returning SensorReadResult<float> in % Relative Humidity
};

#endif // HAL_IHUMIDITY_SENSOR_H
