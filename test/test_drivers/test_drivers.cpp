#include <unity.h>
#include "../Arduino.h"
#include "../MockSensor.h"
#include "../MockActuator.h"
#include "../../include/drivers/TemperatureSensor.h"
#include "../../include/drivers/HumiditySensor.h"

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

void test_dht_temperature_sensor(void) {
    DHT dht(19, DHT22);
    dht.setTemperature(23.5f);
    TemperatureSensor tempSensor(19, &dht);
    tempSensor.init();

    SensorData data = tempSensor.read();
    TEST_ASSERT_FALSE(data.isError);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 23.5f, data.value);
    TEST_ASSERT_EQUAL_STRING("C", tempSensor.getUnit());
}

void test_dht_humidity_sensor(void) {
    DHT dht(19, DHT22);
    dht.setHumidity(62.0f);
    HumiditySensor humSensor(19, &dht);
    humSensor.init();

    SensorData data = humSensor.read();
    TEST_ASSERT_FALSE(data.isError);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 62.0f, data.value);
    TEST_ASSERT_EQUAL_STRING("%", humSensor.getUnit());
}

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;
    UNITY_BEGIN();

    RUN_TEST(test_adc_conversions);
    RUN_TEST(test_actuator_status_text);
    RUN_TEST(test_dht_temperature_sensor);
    RUN_TEST(test_dht_humidity_sensor);

    return UNITY_END();
}
