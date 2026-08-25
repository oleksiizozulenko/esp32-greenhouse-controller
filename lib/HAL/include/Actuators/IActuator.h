#ifndef HAL_IACTUATOR_H
#define HAL_IACTUATOR_H

#include "../CommonTypes.h"

class IActuator {
public:
    virtual ~IActuator() = default;
    virtual bool begin() = 0;
    virtual bool turnOff() = 0;
    virtual bool isOperating() const = 0;
};

#endif // HAL_IACTUATOR_H
