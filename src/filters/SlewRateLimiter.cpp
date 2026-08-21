#include "filters/SlewRateLimiter.h"

SlewRateLimiter::SlewRateLimiter(float maxDeltaPerSec)
    : maxDeltaPerSec(maxDeltaPerSec), lastValue(NAN), lastTime(0) {}

FilterResult SlewRateLimiter::process(float input, bool inputError) {
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

void SlewRateLimiter::reset() {
    lastValue = NAN;
    lastTime = 0;
}
