#include "CompositeFilter.h"

CompositeFilter::CompositeFilter(ISensorFilter** filterArray, size_t filterCount)
    : filters(filterArray), count(filterCount) {}

FilterResult CompositeFilter::process(float input, bool inputError) {
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

void CompositeFilter::reset() {
    for (size_t i = 0; i < count; ++i) {
        if (filters[i] != nullptr) {
            filters[i]->reset();
        }
    }
}
