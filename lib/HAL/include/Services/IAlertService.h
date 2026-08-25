#ifndef HAL_IALERT_SERVICE_H
#define HAL_IALERT_SERVICE_H

#include "../CommonTypes.h"

class IAlertService {
public:
    virtual ~IAlertService() = default;
    virtual bool begin() = 0;
    virtual void raiseAlert(const AlertEvent& alert) = 0;
    virtual void clearAlert(uint16_t code) = 0;
    virtual void clearAll() = 0;
    virtual bool isAlertActive(uint16_t code) const = 0;
};

#endif // HAL_IALERT_SERVICE_H
