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
    explicit SlewRateLimiter(float maxDeltaPerSec)
        : maxDeltaPerSec(maxDeltaPerSec), lastValue(NAN), lastTime(0) {}

    FilterResult process(float input, bool inputError) override {
        if (inputError) {
            return {input, false};
        }

        unsigned long now = millis();
        if (isnan(lastValue) || lastTime == 0) {
            lastValue = input;
            lastTime = now;
            return {input, true};
        }

        float dt = static_cast<float>(now - lastTime) / 1000.0f;
        if (dt <= 0.0001f) {
            dt = 0.0001f; // Prevent division by zero
        }
        lastTime = now;

        float maxAllowedChange = maxDeltaPerSec * dt;
        float diff = input - lastValue;

        // Clamp change if it exceeds physical rate limit
        if (fabs(diff) > maxAllowedChange) {
            input = lastValue + (diff > 0.0f ? maxAllowedChange : -maxAllowedChange);
        }

        lastValue = input;
        return {input, true};
    }

    void reset() override {
        lastValue = NAN;
        lastTime = 0;
    }
};

#endif // SLEW_RATE_LIMITER_H
