#ifndef HAL_ISENSOR_H
#define HAL_ISENSOR_H

#include "../CommonTypes.h"

template <typename T>
class ISensor {
public:
    virtual ~ISensor() = default;
    virtual bool begin() = 0;
    virtual SensorReadResult<T> read() = 0;
};

#endif // HAL_ISENSOR_H
