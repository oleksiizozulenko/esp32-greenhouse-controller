#ifndef HAL_ISOIL_SENSOR_H
#define HAL_ISOIL_SENSOR_H

#include "ISensor.h"

class ISoilSensor : public ISensor<float> {
public:
    virtual ~ISoilSensor() = default;
    // Inherits begin() and read() returning SensorReadResult<float> in % Moisture
};

#endif // HAL_ISOIL_SENSOR_H
