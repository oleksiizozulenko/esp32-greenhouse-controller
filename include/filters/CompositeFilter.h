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
    CompositeFilter(ISensorFilter** filterArray, size_t filterCount);

    FilterResult process(float input, bool inputError) override;
    void reset() override;
};

#endif // COMPOSITE_FILTER_H
