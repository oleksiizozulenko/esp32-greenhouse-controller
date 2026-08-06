#ifndef COMPOSITE_FILTER_H
#define COMPOSITE_FILTER_H

#include <stddef.h>
#include "ISensorFilter.h"

/**
 * @brief Composite filter container that chains multiple ISensorFilter instances sequentially.
 * 
 * Implements the Composite / Decorator pattern, allowing filter pipelines of arbitrary size 
 * (e.g. Median -> SlewRateLimiter -> KaufmanFilter) to be attached to any Sensor driver 
 * via a single ISensorFilter pointer interface.
 */
class CompositeFilter : public ISensorFilter {
private:
    ISensorFilter** filters;
    size_t count;

public:
    /**
     * @param filterArray Pointer to array of ISensorFilter pointers.
     * @param filterCount Number of filter stages in array.
     */
    CompositeFilter(ISensorFilter** filterArray, size_t filterCount)
        : filters(filterArray), count(filterCount) {}

    FilterResult process(float input, bool inputError) override {
        FilterResult res = {input, !inputError};
        if (inputError) {
            return {input, false};
        }

        for (size_t i = 0; i < count; ++i) {
            if (filters[i] != nullptr) {
                res = filters[i]->process(res.value, !res.isValid);
                if (!res.isValid) {
                    break;
                }
            }
        }
        return res;
    }

    void reset() override {
        for (size_t i = 0; i < count; ++i) {
            if (filters[i] != nullptr) {
                filters[i]->reset();
            }
        }
    }
};

#endif // COMPOSITE_FILTER_H
