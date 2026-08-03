#include <unity.h>
#include "../Arduino.h"
#include "../MockSensor.h"
#include "../MockActuator.h"

void setUp(void) {
    resetMockArduinoState();
}

void tearDown(void) {}

void test_adc_conversions(void) {
    // 0 ADC -> 0.0V, 0.0%
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, MockSensor::testAdcToVoltage(0));
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, MockSensor::testAdcToPercentage(0));

    // 4095 ADC (12-bit max) -> 3.3V, 100.0%
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 3.3f, MockSensor::testAdcToVoltage(4095));
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 100.0f, MockSensor::testAdcToPercentage(4095));

    // Mid scale 2047 ADC -> ~1.65V, ~50%
    TEST_ASSERT_FLOAT_WITHIN(0.05f, 1.65f, MockSensor::testAdcToVoltage(2047));
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 50.0f, MockSensor::testAdcToPercentage(2047));
}

void test_actuator_status_text(void) {
    MockActuator act(5, "TestActuator");
    
    TEST_ASSERT_FALSE(act.isOn());
    TEST_ASSERT_EQUAL_STRING("OFF", act.getStatusText());

    act.turnOn();
    TEST_ASSERT_TRUE(act.isOn());
    TEST_ASSERT_EQUAL_STRING("ON", act.getStatusText());
}

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;
    UNITY_BEGIN();

    RUN_TEST(test_adc_conversions);
    RUN_TEST(test_actuator_status_text);

    return UNITY_END();
}
