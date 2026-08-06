#ifndef MEDIAN_FILTER_H
#define MEDIAN_FILTER_H

#include <stddef.h>
#include "ISensorFilter.h"

/**
 * @brief Sliding Median Filter for rejecting transient ADC spikes and hardware bounce ("брязкотіння").
 * 
 * Maintains a fixed circular sample buffer of size N and computes the median value 
 * using insertion sort on a stack-allocated copy. Zero heap allocation.
 * 
 * @tparam N Window size (number of historical samples to evaluate). Default is 5.
 */
template <size_t N = 5>
class MedianFilter : public ISensorFilter {
private:
    float buffer[N] = {0.0f};
    size_t count = 0;
    size_t writeIdx = 0;

public:
    MedianFilter() = default;

    /**
     * @brief Process input value by inserting into sliding buffer and taking median.
     */
    FilterResult process(float input, bool inputError) override {
        if (inputError) {
            return {input, false};
        }

        buffer[writeIdx] = input;
        writeIdx = (writeIdx + 1) % N;
        if (count < N) {
            count++;
        }

        // Copy active window elements to stack array for sorting
        float temp[N];
        for (size_t i = 0; i < count; ++i) {
            temp[i] = buffer[i];
        }

        // In-place insertion sort (optimal efficiency for small fixed N)
        for (size_t i = 1; i < count; ++i) {
            float key = temp[i];
            int j = static_cast<int>(i) - 1;
            while (j >= 0 && temp[j] > key) {
                temp[j + 1] = temp[j];
                j--;
            }
            temp[j + 1] = key;
        }

        float median = temp[count / 2];
        return {median, true};
    }

    /**
     * @brief Clear internal sample buffer.
     */
    void reset() override {
        count = 0;
        writeIdx = 0;
        for (size_t i = 0; i < N; ++i) {
            buffer[i] = 0.0f;
        }
    }
};

#endif // MEDIAN_FILTER_H
