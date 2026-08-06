#include <unity.h>
#include <cstdlib>
#include <math.h>
#include "../Arduino.h"
#include "../MockSensor.h"
#include "../../include/filters/ISensorFilter.h"
#include "../../include/filters/MedianFilter.h"
#include "../../include/filters/SlewRateLimiter.h"
#include "../../include/filters/KaufmanFilter.h"
#include "../../include/filters/CompositeFilter.h"

static size_t gAllocationCount = 0;

void* operator new(std::size_t size) {
    ++gAllocationCount;
    return std::malloc(size);
}

void operator delete(void* ptr) noexcept {
    std::free(ptr);
}

void setUp(void) {
    resetMockArduinoState();
    gAllocationCount = 0;
}

void tearDown(void) {}

// ----------------------------------------------------
// 1. MedianFilter Tests
// ----------------------------------------------------

void test_median_filter_spike_suppression(void) {
    MedianFilter<5> filter;

    // Stream values with a huge noise spike in the middle: 20, 21, 999, 22, 21
    FilterResult r1 = filter.process(20.0f, false);
    FilterResult r2 = filter.process(21.0f, false);
    FilterResult r3 = filter.process(999.0f, false); // Spike!
    FilterResult r4 = filter.process(22.0f, false);
    FilterResult r5 = filter.process(21.0f, false);

    TEST_ASSERT_TRUE(r5.isValid);
    // Buffer contains [20, 21, 999, 22, 21]. Sorted: [20, 21, 21, 22, 999]. Median is 21.0
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 21.0f, r5.value);
}

void test_median_filter_error_propagation(void) {
    MedianFilter<3> filter;
    FilterResult r = filter.process(100.0f, true); // Hardware error
    TEST_ASSERT_FALSE(r.isValid);
}

// ----------------------------------------------------
// 2. SlewRateLimiter Tests
// ----------------------------------------------------

void test_slew_rate_limiter_clamping(void) {
    SlewRateLimiter limiter(10.0f); // Max 10 units per second

    setSimulatedMillis(1000);
    FilterResult r1 = limiter.process(50.0f, false); // Initial reading: 50
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 50.0f, r1.value);

    // 500ms later, sudden jump from 50 to 90 (delta 40)
    // Max allowed change in 0.5s = 10 * 0.5 = 5.0
    // Result should be clamped to 50 + 5 = 55.0
    setSimulatedMillis(1500);
    FilterResult r2 = limiter.process(90.0f, false);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 55.0f, r2.value);
}

// ----------------------------------------------------
// 3. KaufmanFilter Tests
// ----------------------------------------------------

void test_kaufman_filter_smoothing(void) {
    KaufmanFilter<5> kaufman;

    // Feed steady input: 30.0
    for (int i = 0; i < 5; ++i) {
        setSimulatedMillis(i * 100);
        kaufman.process(30.0f, false);
    }

    // Step input to 50.0
    FilterResult r = kaufman.process(50.0f, false);
    TEST_ASSERT_TRUE(r.isValid);
    TEST_ASSERT_TRUE(r.value > 30.0f && r.value < 50.0f);
}

// ----------------------------------------------------
// 4. CompositeFilter Tests & Zero Allocation
// ----------------------------------------------------

void test_composite_filter_pipeline(void) {
    MedianFilter<5> median;
    SlewRateLimiter slew(10.0f);

    ISensorFilter* chain[2] = { &median, &slew };
    CompositeFilter composite(chain, 2);

    setSimulatedMillis(1000);
    FilterResult r1 = composite.process(20.0f, false);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 20.0f, r1.value);

    // Test zero allocation during processing
    gAllocationCount = 0;
    setSimulatedMillis(2000);
    FilterResult r2 = composite.process(22.0f, false);
    TEST_ASSERT_EQUAL_UINT(0, gAllocationCount);
    TEST_ASSERT_TRUE(r2.isValid);
}

void test_sensor_integration_with_filter(void) {
    MockSensor sensor(19, SensorType::SOIL, "Soil", "%");
    MedianFilter<3> median;
    sensor.setFilter(&median);

    sensor.setData(10.0f, false);
    sensor.readProcessed();

    sensor.setData(999.0f, false); // Noise spike
    sensor.readProcessed();

    sensor.setData(12.0f, false);
    SensorData finalRes = sensor.readProcessed();

    // Buffer [10, 999, 12]. Sorted: [10, 12, 999]. Median: 12.0
    TEST_ASSERT_FALSE(finalRes.isError);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 12.0f, finalRes.value);
}

int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    UNITY_BEGIN();

    RUN_TEST(test_median_filter_spike_suppression);
    RUN_TEST(test_median_filter_error_propagation);
    RUN_TEST(test_slew_rate_limiter_clamping);
    RUN_TEST(test_kaufman_filter_smoothing);
    RUN_TEST(test_composite_filter_pipeline);
    RUN_TEST(test_sensor_integration_with_filter);

    return UNITY_END();
}
