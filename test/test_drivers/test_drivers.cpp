#include <unity.h>
#include "../Arduino.h"
#include "../MockSensor.h"
#include "../MockActuator.h"
#include "../../include/drivers/TemperatureSensor.h"
#include "../../include/drivers/HumiditySensor.h"
#include "../../include/drivers/LightSensor.h"

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

void test_dht_temperature_sensor_error_below_min(void) {
    DHT dht(19, DHT22);
    dht.setTemperature(-6.0f); // Below -5.0°C threshold -> Error
    TemperatureSensor tempSensor(19, &dht);
    tempSensor.init();

    SensorData data = tempSensor.read();
    TEST_ASSERT_TRUE(data.isError);

    // -5.0°C should be valid (not < -5.0)
    dht.setTemperature(-5.0f);
    // Reset interval timer to force re-read
    delay(2001);
    data = tempSensor.read();
    TEST_ASSERT_FALSE(data.isError);
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

void test_dht_humidity_sensor_error_above_max(void) {
    DHT dht(19, DHT22);
    dht.setHumidity(91.5f); // > 90% threshold -> Error
    HumiditySensor humSensor(19, &dht);
    humSensor.init();

    SensorData data = humSensor.read();
    TEST_ASSERT_TRUE(data.isError);

    // 90.0% should be valid (<= 90% threshold)
    dht.setHumidity(90.0f);
    delay(2500);
    data = humSensor.read();
    TEST_ASSERT_FALSE(data.isError);
}

void test_light_sensor_adc_to_lux_conversion(void) {
    LightSensor lightSensor(35);
    lightSensor.init();

    TEST_ASSERT_EQUAL_STRING("lx", lightSensor.getUnit());

    // 0 ADC -> 0.0 lx
    setMockAnalogRead(35, 0);
    SensorData d0 = lightSensor.read();
    TEST_ASSERT_FALSE(d0.isError);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 0.0f, d0.value);

    // 2050 ADC -> ~100 lx
    setMockAnalogRead(35, 2050);
    advanceSimulatedMillis(2001);
    SensorData d100 = lightSensor.read();
    TEST_ASSERT_FALSE(d100.isError);
    TEST_ASSERT_FLOAT_WITHIN(15.0f, 100.0f, d100.value);

    // 2971 ADC -> ~400 lx
    setMockAnalogRead(35, 2971);
    advanceSimulatedMillis(2001);
    SensorData d400 = lightSensor.read();
    TEST_ASSERT_FALSE(d400.isError);
    TEST_ASSERT_FLOAT_WITHIN(30.0f, 400.0f, d400.value);

    // 3938 ADC -> ~10000 lx
    setMockAnalogRead(35, 3938);
    advanceSimulatedMillis(2001);
    SensorData d10000 = lightSensor.read();
    TEST_ASSERT_FALSE(d10000.isError);
    TEST_ASSERT_FLOAT_WITHIN(800.0f, 10000.0f, d10000.value);
}

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;
    UNITY_BEGIN();

    RUN_TEST(test_adc_conversions);
    RUN_TEST(test_actuator_status_text);
    RUN_TEST(test_dht_temperature_sensor);
    RUN_TEST(test_dht_temperature_sensor_error_below_min);
    RUN_TEST(test_dht_humidity_sensor);
    RUN_TEST(test_dht_humidity_sensor_error_above_max);
    RUN_TEST(test_light_sensor_adc_to_lux_conversion);

    return UNITY_END();
}
