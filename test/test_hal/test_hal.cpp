#include <unity.h>
#include "CommonTypes.h"
#include "DHT11EnvironmentSensor.h"
#include "MH0080SoilSensorDriver.h"
#include "AnalogLdrLightSensorDriver.h"
#include "DotMatrix8x8IrrigationActuator.h"
#include "QC1602ACharacterLcdDriver.h"


void test_mh0080_soil_sensor() {
    MH0080SoilSensorDriver soilSensor(34);
    soilSensor.begin();

    SensorReadResult<float> result = soilSensor.read();
    TEST_ASSERT_TRUE(result.isValid() || result.status == SensorStatus::Error_OutOfRange);
}

void test_analog_ldr_light_sensor() {
    AnalogLdrLightSensorDriver ldrSensor(35);
    ldrSensor.begin();

    SensorReadResult<float> result = ldrSensor.read();
    TEST_ASSERT_TRUE(result.isValid() || result.status == SensorStatus::Error_OutOfRange);
}

void test_dht11_environment_sensor() {
    DHT11EnvironmentSensor dht11(19);
    dht11.begin();

    SensorReadResult<float> tempResult = dht11.read();
    TEST_ASSERT_TRUE(tempResult.isValid() || tempResult.status == SensorStatus::Error_HardwareFault);
}

void test_dot_matrix_irrigation_actuator() {
    DotMatrix8x8IrrigationActuator dotMatrix(16);
    dotMatrix.begin();

    TEST_ASSERT_FALSE(dotMatrix.isOn());
    dotMatrix.turnOn();
    TEST_ASSERT_TRUE(dotMatrix.isOn());

    dotMatrix.turnOff();
    TEST_ASSERT_FALSE(dotMatrix.isOn());
}

void test_qc1602a_character_lcd_driver() {
    QC1602ACharacterLcdDriver lcd(13, 12, 14, 27, 26, 25);
    TEST_ASSERT_TRUE(lcd.begin());

    lcd.setHeader("Greenhouse");
    lcd.setField(0, {"Temp", 24.5f, "C", 1});
    lcd.refresh();
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_mh0080_soil_sensor);
    RUN_TEST(test_analog_ldr_light_sensor);
    RUN_TEST(test_dht11_environment_sensor);
    RUN_TEST(test_dot_matrix_irrigation_actuator);
    RUN_TEST(test_qc1602a_character_lcd_driver);
    return UNITY_END();
}
