#ifndef ISENSOR_FILTER_H
#define ISENSOR_FILTER_H

/**
 * @brief Represents the output state of a sensor filter operation.
 */
struct FilterResult {
    float value;   ///< Filtered numerical sensor value.
    bool isValid;  ///< True if data is valid; false if signal/hardware error occurred.
};

/**
 * @brief Abstract interface for modular sensor signal filters.
 * 
 * Implementations must perform zero dynamic memory allocation during process()
 * to guarantee ESP32 heap stability and prevent memory fragmentation.
 */
class ISensorFilter {
public:
    virtual ~ISensorFilter() = default;

    /**
     * @brief Process a raw incoming sensor sample.
     * @param input Raw numerical value from sensor driver or preceding filter step.
     * @param inputError True if preceding driver/filter reported a hardware or signal error.
     * @return FilterResult containing filtered value and validity status.
     */
    virtual FilterResult process(float input, bool inputError) = 0;

    /**
     * @brief Reset internal filter state (e.g. clear buffers and timestamps).
     */
    virtual void reset() = 0;
};

#endif // ISENSOR_FILTER_H
