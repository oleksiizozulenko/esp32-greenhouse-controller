#ifndef HAL_IACTUATOR_H
#define HAL_IACTUATOR_H

#include "CommonTypes.h"

enum class ActuatorType {
    UNKNOWN = 0,
    VENTILATION,
    IRRIGATION,
    LIGHT
};

class IActuator {
public:
    virtual ~IActuator() = default;

    virtual bool begin() = 0;
    virtual bool turnOn() = 0;
    virtual bool turnOff() = 0;
    virtual bool isOperating() const = 0;

    virtual int getPin() const = 0;
    virtual ActuatorType getType() const = 0;
    virtual const char* getName() const = 0;
    virtual const char* getStatusText() const = 0;
};

#endif // HAL_IACTUATOR_H

