#ifndef HAL_ITONE_ACTUATOR_H
#define HAL_ITONE_ACTUATOR_H

#include "IActuator.h"

class IToneActuator : public IActuator {
public:
    virtual ~IToneActuator() = default;
    virtual bool playTone(uint16_t frequencyHz, uint32_t durationMs = 0) = 0;
    virtual bool playTrack(uint8_t trackId) = 0;
    virtual bool stop() = 0;
};

#endif // HAL_ITONE_ACTUATOR_H
