#include <unity.h>
#include "CommonTypes.h"
#include "Sensors/DHT22EnvironmentSensor.h"
#include "Sensors/AnalogSoilSensorDriver.h"
#include "Sensors/LDRLightSensorDriver.h"
#include "Actuators/RelayBinaryActuator.h"
#include "Actuators/ServoPositionalActuator.h"
#include "Services/BuzzerAlertDriver.h"

void test_relay_binary_actuator() {
    RelayBinaryActuator relay(4, false); // Active HIGH
    relay.begin();

    TEST_ASSERT_FALSE(relay.isOn());
    TEST_ASSERT_FALSE(relay.isOperating());

    relay.turnOn();
    TEST_ASSERT_TRUE(relay.isOn());
    TEST_ASSERT_TRUE(relay.isOperating());

    relay.turnOff();
    TEST_ASSERT_FALSE(relay.isOn());
}

void test_servo_positional_actuator() {
    ServoPositionalActuator servo(5);
    servo.begin();

    TEST_ASSERT_EQUAL_FLOAT(0.0f, servo.getPositionPercent());

    servo.setPositionPercent(50.0f);
    TEST_ASSERT_EQUAL_FLOAT(50.0f, servo.getPositionPercent());

    servo.setPositionPercent(150.0f); // Should clamp to 100%
    TEST_ASSERT_EQUAL_FLOAT(100.0f, servo.getPositionPercent());

    servo.turnOff();
    TEST_ASSERT_EQUAL_FLOAT(0.0f, servo.getPositionPercent());
}

void test_buzzer_alert_driver() {
    BuzzerAlertDriver buzzer(18);
    buzzer.begin();

    AlertEvent alert{101, AlertSeverity::CRITICAL, "High Temp"};
    buzzer.raiseAlert(alert);

    TEST_ASSERT_TRUE(buzzer.isAlertActive(101));
    TEST_ASSERT_FALSE(buzzer.isAlertActive(102));

    buzzer.clearAlert(101);
    TEST_ASSERT_FALSE(buzzer.isAlertActive(101));
}

void test_soil_sensor_driver() {
    AnalogSoilSensorDriver soilSensor(34);
    soilSensor.begin();

    SensorReadResult<float> result = soilSensor.read();
    TEST_ASSERT_TRUE(result.isValid() || result.status == SensorStatus::Error_OutOfRange);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_relay_binary_actuator);
    RUN_TEST(test_servo_positional_actuator);
    RUN_TEST(test_buzzer_alert_driver);
    RUN_TEST(test_soil_sensor_driver);
    return UNITY_END();
}
