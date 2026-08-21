#ifndef SLEW_RATE_LIMITER_H
#define SLEW_RATE_LIMITER_H

#include <Arduino.h>
#include <math.h>
#include "ISensorFilter.h"

/**
 * @brief Slew-Rate Limiter (Rate-of-Change Limiter) filter.
 * 
 * Clamps maximum allowed change between consecutive sensor readings based on elapsed time 
 * and maximum physical change rate per second (maxDeltaPerSec). Prevents physically 
 * impossible rate jumps (e.g. soil moisture jumping 50% in 100ms).
 */
class SlewRateLimiter : public ISensorFilter {
private:
    float maxDeltaPerSec;  ///< Maximum allowed rate of change per second (units/sec)
    float lastValue;       ///< Last clamped valid output value
    unsigned long lastTime;///< Timestamp of last processed reading (ms)

public:
    /**
     * @param maxDeltaPerSec Max physical change allowed per second.
     */
    explicit SlewRateLimiter(float maxDeltaPerSec);

    FilterResult process(float input, bool inputError) override;
    void reset() override;
};

#endif // SLEW_RATE_LIMITER_H
