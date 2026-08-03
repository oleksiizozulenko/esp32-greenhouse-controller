#include <unity.h>
#include "../Arduino.h"
#include "../MockSensor.h"
#include "../../include/services/SensorsService.h"

static SensorsService* sensorsService;
static MockSensor* tempSensor;
static MockSensor* soilSensor;
static MockSensor* lightSensor;

void setUp(void) {
    resetMockArduinoState();
    
    sensorsService = new SensorsService(2, 2000);

    tempSensor = new MockSensor(PIN_TEMP, "Temperature", "°C");
    soilSensor = new MockSensor(PIN_SOIL_POT, "Soil", "%");
    lightSensor = new MockSensor(PIN_LDR, "Light", "lux");

    sensorsService->addSensor(tempSensor);
    sensorsService->addSensor(soilSensor);
    sensorsService->addSensor(lightSensor); // Forces dynamic expansion
}

void tearDown(void) {
    delete sensorsService;
    delete tempSensor;
    delete soilSensor;
    delete lightSensor;
}

// ----------------------------------------------------
// 1. SensorsService Registration & Indexing
// ----------------------------------------------------

void test_sensors_service_registration_and_indexing(void) {
    TEST_ASSERT_EQUAL_UINT(3, sensorsService->getSensorCount());
    TEST_ASSERT_EQUAL_PTR(tempSensor, sensorsService->getSensor(0));
    TEST_ASSERT_EQUAL_PTR(soilSensor, sensorsService->getSensor(1));
    TEST_ASSERT_EQUAL_PTR(lightSensor, sensorsService->getSensor(2));
    TEST_ASSERT_NULL(sensorsService->getSensor(99));
}

void test_sensors_service_add_null_returns_false(void) {
    TEST_ASSERT_FALSE(sensorsService->addSensor(nullptr));
    TEST_ASSERT_EQUAL_UINT(3, sensorsService->getSensorCount());
}

// ----------------------------------------------------
// 2. SensorDataMap Operations & Lookups
// ----------------------------------------------------

void test_sensor_data_map_read_all(void) {
    tempSensor->setData(25.5f, false);
    soilSensor->setData(45.0f, false);
    lightSensor->setData(750.0f, false);

    SensorDataMap readings = sensorsService->readAll();

    TEST_ASSERT_EQUAL_UINT(3, readings.size());

    SensorData tData = readings.get("Temperature");
    TEST_ASSERT_FALSE(tData.isError);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 25.5f, tData.value);

    SensorData sData = readings.get("Soil");
    TEST_ASSERT_FALSE(sData.isError);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 45.0f, sData.value);

    SensorData lData = readings.get("Light");
    TEST_ASSERT_FALSE(lData.isError);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 750.0f, lData.value);
}

void test_sensor_data_map_lookup_by_pointer(void) {
    tempSensor->setData(22.0f, false);
    SensorDataMap readings = sensorsService->readAll();

    SensorData data = readings.get(tempSensor);
    TEST_ASSERT_FALSE(data.isError);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 22.0f, data.value);
}

void test_sensor_data_map_non_existent_key(void) {
    SensorDataMap readings = sensorsService->readAll();

    SensorData data = readings.get("UnknownKey");
    TEST_ASSERT_TRUE(data.isError);

    SensorData nullPtrData = readings.get(static_cast<const char*>(nullptr));
    TEST_ASSERT_TRUE(nullPtrData.isError);
}

// ----------------------------------------------------
// 3. SensorDataMap Memory Copy & Move Semantics
// ----------------------------------------------------

void test_sensor_data_map_copy_constructor(void) {
    tempSensor->setData(18.5f, false);
    SensorDataMap original = sensorsService->readAll();

    SensorDataMap copy(original);

    TEST_ASSERT_EQUAL_UINT(original.size(), copy.size());
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 18.5f, copy.get("Temperature").value);
}

void test_sensor_data_map_copy_assignment(void) {
    tempSensor->setData(19.5f, false);
    SensorDataMap original = sensorsService->readAll();

    SensorDataMap assigned;
    assigned = original;

    TEST_ASSERT_EQUAL_UINT(original.size(), assigned.size());
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 19.5f, assigned.get("Temperature").value);
}

void test_sensor_data_map_move_semantics(void) {
    tempSensor->setData(31.0f, false);
    SensorDataMap original = sensorsService->readAll();

    SensorDataMap moved(std::move(original));

    TEST_ASSERT_EQUAL_UINT(3, moved.size());
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 31.0f, moved.get("Temperature").value);
    TEST_ASSERT_EQUAL_UINT(0, original.size()); // original was moved
}

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;
    UNITY_BEGIN();

    RUN_TEST(test_sensors_service_registration_and_indexing);
    RUN_TEST(test_sensors_service_add_null_returns_false);

    RUN_TEST(test_sensor_data_map_read_all);
    RUN_TEST(test_sensor_data_map_lookup_by_pointer);
    RUN_TEST(test_sensor_data_map_non_existent_key);

    RUN_TEST(test_sensor_data_map_copy_constructor);
    RUN_TEST(test_sensor_data_map_copy_assignment);
    RUN_TEST(test_sensor_data_map_move_semantics);

    return UNITY_END();
}
