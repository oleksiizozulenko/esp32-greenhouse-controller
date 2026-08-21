#include "SlewRateLimiter.h"

SlewRateLimiter::SlewRateLimiter(float maxDeltaPerSec)
    : maxDeltaPerSec(maxDeltaPerSec), lastValue(NAN), lastTime(0) {}

FilterResult SlewRateLimiter::process(float input, bool inputError) {
    if (inputError) {
        return {input, false};
    }

    unsigned long currentTime = millis();

    if (isnan(lastValue) || lastTime == 0) {
        lastValue = input;
        lastTime = currentTime;
        return {input, true};
    }

    float elapsedTimeSec = (currentTime - lastTime) / 1000.0f;
    if (elapsedTimeSec <= 0.0001f) {
        elapsedTimeSec = 0.0001f;
    }

    float maxAllowedChange = maxDeltaPerSec * elapsedTimeSec;
    float delta = input - lastValue;

    if (fabsf(delta) > maxAllowedChange) {
        if (delta > 0) {
            lastValue += maxAllowedChange;
        } else {
            lastValue -= maxAllowedChange;
        }
    } else {
        lastValue = input;
    }

    lastTime = currentTime;
    return {lastValue, true};
}

void SlewRateLimiter::reset() {
    lastValue = NAN;
    lastTime = 0;
}
