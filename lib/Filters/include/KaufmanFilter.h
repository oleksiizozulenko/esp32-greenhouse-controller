#ifndef KAUFMAN_FILTER_H
#define KAUFMAN_FILTER_H

#include <stddef.h>
#include <math.h>
#include "ISensorFilter.h"

/**
 * @brief Kaufman's Adaptive Moving Average (KAMA) signal filter.
 * 
 * Dynamically adjusts its smoothing coefficient (SC) based on signal Efficiency Ratio (ER):
 * ER = |Change over period| / Sum(|Single period changes|)
 * 
 * During noisy or stationary periods (low ER), KAMA applies heavy smoothing to reject noise.
 * During real physical trend changes (high ER), KAMA adjusts rapidly to track real values.
 * 
 * @tparam PERIOD Lookback window for computing Efficiency Ratio. Default is 10.
 */
template <size_t PERIOD = 10>
class KaufmanFilter : public ISensorFilter {
private:
    float prices[PERIOD] = {0.0f};
    size_t count = 0;
    size_t writeIdx = 0;
    float kama = NAN;

    const float fastSC = 2.0f / (2.0f + 1.0f);   // Fast EMA constant (2 periods)
    const float slowSC = 2.0f / (30.0f + 1.0f);  // Slow EMA constant (30 periods)

public:
    KaufmanFilter() = default;

    FilterResult process(float input, bool inputError) override {
        if (inputError) {
            return {input, false};
        }

        if (isnan(kama)) {
            kama = input;
            prices[writeIdx] = input;
            writeIdx = (writeIdx + 1) % PERIOD;
            if (count < PERIOD) count++;
            return {kama, true};
        }

        prices[writeIdx] = input;
        writeIdx = (writeIdx + 1) % PERIOD;
        if (count < PERIOD) count++;

        if (count < PERIOD) {
            // Apply standard exponential moving average until buffer fills
            kama = kama + 0.2f * (input - kama);
            return {kama, true};
        }

        // Compute Efficiency Ratio (ER = Directional Change / Total Volatility)
        size_t oldestIdx = writeIdx; // Pointer to oldest sample in ring buffer
        float change = fabs(input - prices[oldestIdx]);

        float volatility = 0.0f;
        size_t idx = oldestIdx;
        for (size_t i = 0; i < PERIOD - 1; ++i) {
            size_t nextIdx = (idx + 1) % PERIOD;
            volatility += fabs(prices[nextIdx] - prices[idx]);
            idx = nextIdx;
        }

        float er = (volatility > 0.0001f) ? (change / volatility) : 0.0f;
        if (er > 1.0f) er = 1.0f;

        // Compute adaptive smoothing constant (SC)
        float sc = er * (fastSC - slowSC) + slowSC;
        sc = sc * sc; // Square of SC as specified by Kaufman

        kama = kama + sc * (input - kama);
        return {kama, true};
    }

    void reset() override {
        count = 0;
        writeIdx = 0;
        kama = NAN;
        for (size_t i = 0; i < PERIOD; ++i) {
            prices[i] = 0.0f;
        }
    }
};

#endif // KAUFMAN_FILTER_H
