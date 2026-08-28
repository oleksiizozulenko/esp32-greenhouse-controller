#ifndef HAL_IBINARY_ACTUATOR_H
#define HAL_IBINARY_ACTUATOR_H

#include "IActuator.h"

class IBinaryActuator : virtual public IActuator {
public:
    virtual ~IBinaryActuator() = default;
    virtual bool isOn() const = 0;
    bool isOperating() const override { return isOn(); }
};

#endif // HAL_IBINARY_ACTUATOR_H

