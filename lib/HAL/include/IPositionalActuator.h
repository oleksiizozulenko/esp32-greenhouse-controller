#ifndef HAL_IPOSITIONAL_ACTUATOR_H
#define HAL_IPOSITIONAL_ACTUATOR_H

#include "IActuator.h"

class IPositionalActuator : virtual public IActuator {
public:
    virtual ~IPositionalActuator() = default;
    virtual bool setAngleDegrees(float angle) = 0;
    virtual bool setPositionPercent(float percent0to100) = 0;
    virtual float getPositionPercent() const = 0;
};

#endif // HAL_IPOSITIONAL_ACTUATOR_H

