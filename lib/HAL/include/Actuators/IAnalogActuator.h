#ifndef HAL_IANALOG_ACTUATOR_H
#define HAL_IANALOG_ACTUATOR_H

#include "IActuator.h"

class IAnalogActuator : public IActuator {
public:
    virtual ~IAnalogActuator() = default;
    virtual bool setPercentage(float percent0to100) = 0;
    virtual float getPercentage() const = 0;
};

#endif // HAL_IANALOG_ACTUATOR_H
