#ifndef HAL_IBINARY_ACTUATOR_H
#define HAL_IBINARY_ACTUATOR_H

#include "IActuator.h"

class IBinaryActuator : public IActuator {
public:
    virtual ~IBinaryActuator() = default;
    virtual bool turnOn() = 0;
    virtual bool isOn() const = 0;
};

#endif // HAL_IBINARY_ACTUATOR_H
