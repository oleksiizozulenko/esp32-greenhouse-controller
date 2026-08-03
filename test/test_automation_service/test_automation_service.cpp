#include <unity.h>
#include "../Arduino.h"
#include "../MockActuator.h"
#include "../MockSensor.h"
#include "../../include/services/AutomationService.h"
#include "../../include/services/SensorsService.h"

static AutomationService* automation;
static MockActuator* ventActuator;
static MockActuator* irrigActuator;
static MockActuator* lightActuator;

static MockSensor* tempSensor;
static MockSensor* soilSensor;
static MockSensor* lightSensor;

void setUp(void) {
    resetMockArduinoState();
    
    automation = new AutomationService(4, PIN_LED_RED, PIN_LED_GREEN, PIN_BUZZER);
    
    ventActuator = new MockActuator(PIN_ACTUATOR_VENT, "Ventilation");
    irrigActuator = new MockActuator(PIN_ACTUATOR_IRRIG, "Irrigation");
    lightActuator = new MockActuator(PIN_ACTUATOR_LIGHT, "Light");

    automation->addActuator(ventActuator);
    automation->addActuator(irrigActuator);
    automation->addActuator(lightActuator);

    tempSensor = new MockSensor(PIN_TEMP, "Temperature", "°C");
    soilSensor = new MockSensor(PIN_SOIL_POT, "Soil", "%");
    lightSensor = new MockSensor(PIN_LDR, "Light", "lux");

    automation->begin();
}

void tearDown(void) {
    delete automation;
    delete ventActuator;
    delete irrigActuator;
    delete lightActuator;
    delete tempSensor;
    delete soilSensor;
    delete lightSensor;
}

// ----------------------------------------------------
// 1. Actuator Registration & Lookup Tests
// ----------------------------------------------------

void test_actuator_registration_and_lookup(void) {
    TEST_ASSERT_EQUAL_UINT(3, automation->getActuatorCount());
    TEST_ASSERT_EQUAL_PTR(ventActuator, automation->getActuator("Ventilation"));
    TEST_ASSERT_EQUAL_PTR(irrigActuator, automation->getActuator("Irrigation"));
    TEST_ASSERT_EQUAL_PTR(lightActuator, automation->getActuator("Light"));
    TEST_ASSERT_NULL(automation->getActuator("NonExistent"));
    TEST_ASSERT_NULL(automation->getActuator(99));
}

// ----------------------------------------------------
// 2. Automatic Ventilation Control Tests
// ----------------------------------------------------

void test_auto_ventilation_high_temp_opens(void) {
    tempSensor->setData(29.0f, false); // > 28.0°C threshold
    soilSensor->setData(50.0f, false);
    lightSensor->setData(600.0f, false);

    SensorDataMap readings(3);
    readings[0] = {tempSensor, tempSensor->read()};
    readings[1] = {soilSensor, soilSensor->read()};
    readings[2] = {lightSensor, lightSensor->read()};

    automation->update(true, readings);

    TEST_ASSERT_TRUE(ventActuator->isOn());
    TEST_ASSERT_EQUAL_INT(1, ventActuator->getTurnOnCalls());
}

void test_auto_ventilation_hysteresis_holds_open(void) {
    // First trigger open
    tempSensor->setData(30.0f, false);
    SensorDataMap readings1(1);
    readings1[0] = {tempSensor, tempSensor->read()};
    automation->update(true, readings1);
    TEST_ASSERT_TRUE(ventActuator->isOn());

    // Drop temp into hysteresis zone (27.0°C is between 26.0 and 28.0)
    tempSensor->setData(27.0f, false);
    SensorDataMap readings2(1);
    readings2[0] = {tempSensor, tempSensor->read()};
    automation->update(true, readings2);

    TEST_ASSERT_TRUE(ventActuator->isOn()); // Holds OPEN
}

void test_auto_ventilation_low_temp_closes(void) {
    // Open ventilation first
    ventActuator->turnOn();

    // Temp drops below (28.0 - 2.0 = 26.0°C)
    tempSensor->setData(25.5f, false);
    SensorDataMap readings(1);
    readings[0] = {tempSensor, tempSensor->read()};
    automation->update(true, readings);

    TEST_ASSERT_FALSE(ventActuator->isOn());
}

void test_auto_ventilation_hysteresis_holds_closed(void) {
    ventActuator->turnOff();

    // Temp rises to 27.0°C (in hysteresis zone while closed)
    tempSensor->setData(27.0f, false);
    SensorDataMap readings(1);
    readings[0] = {tempSensor, tempSensor->read()};
    automation->update(true, readings);

    TEST_ASSERT_FALSE(ventActuator->isOn());
}

void test_auto_ventilation_sensor_error_isolation(void) {
    // Actuator is initially ON
    ventActuator->turnOn();

    // Sensor reports error
    tempSensor->setData(35.0f, true);
    SensorDataMap readings(1);
    readings[0] = {tempSensor, tempSensor->read()};
    automation->update(true, readings);

    TEST_ASSERT_FALSE(ventActuator->isOn()); // Turns OFF due to isError=true
}

// ----------------------------------------------------
// 3. Automatic Irrigation Control Tests
// ----------------------------------------------------

void test_auto_irrigation_dry_soil_turns_on(void) {
    soilSensor->setData(25.0f, false); // < 30% threshold
    SensorDataMap readings(1);
    readings[0] = {soilSensor, soilSensor->read()};

    automation->update(true, readings);

    TEST_ASSERT_TRUE(irrigActuator->isOn());
    TEST_ASSERT_EQUAL_INT(1, irrigActuator->getTurnOnCalls());
}

void test_auto_irrigation_hysteresis_holds_on(void) {
    irrigActuator->turnOn();

    // Moisture rises to 33% (in hysteresis zone 30% - 35%)
    soilSensor->setData(33.0f, false);
    SensorDataMap readings(1);
    readings[0] = {soilSensor, soilSensor->read()};

    automation->update(true, readings);

    TEST_ASSERT_TRUE(irrigActuator->isOn());
}

void test_auto_irrigation_sufficient_moisture_turns_off(void) {
    irrigActuator->turnOn();

    // Moisture exceeds 35% (30 + 5)
    soilSensor->setData(36.0f, false);
    SensorDataMap readings(1);
    readings[0] = {soilSensor, soilSensor->read()};

    automation->update(true, readings);

    TEST_ASSERT_FALSE(irrigActuator->isOn());
}

void test_auto_irrigation_sensor_error_isolation(void) {
    // Actuator is initially ON
    irrigActuator->turnOn();

    // Sensor error
    soilSensor->setData(10.0f, true);
    SensorDataMap readings(1);
    readings[0] = {soilSensor, soilSensor->read()};

    automation->update(true, readings);

    TEST_ASSERT_FALSE(irrigActuator->isOn()); // Turns OFF due to isError=true
}

// ----------------------------------------------------
// 4. Automatic Light Control Tests
// ----------------------------------------------------

void test_auto_light_darkness_turns_on(void) {
    lightSensor->setData(450.0f, false); // < 500 threshold
    SensorDataMap readings(1);
    readings[0] = {lightSensor, lightSensor->read()};

    automation->update(true, readings);

    TEST_ASSERT_TRUE(lightActuator->isOn());
}

void test_auto_light_hysteresis_holds_on(void) {
    lightActuator->turnOn();

    // Light rises to 525 (in hysteresis zone 500 - 550)
    lightSensor->setData(525.0f, false);
    SensorDataMap readings(1);
    readings[0] = {lightSensor, lightSensor->read()};

    automation->update(true, readings);

    TEST_ASSERT_TRUE(lightActuator->isOn());
}

void test_auto_light_daylight_turns_off(void) {
    lightActuator->turnOn();

    // Light rises above 550 (500 + 50)
    lightSensor->setData(600.0f, false);
    SensorDataMap readings(1);
    readings[0] = {lightSensor, lightSensor->read()};

    automation->update(true, readings);

    TEST_ASSERT_FALSE(lightActuator->isOn());
}

void test_auto_light_sensor_error_isolation(void) {
    // Actuator is initially ON
    lightActuator->turnOn();

    lightSensor->setData(100.0f, true);
    SensorDataMap readings(1);
    readings[0] = {lightSensor, lightSensor->read()};

    automation->update(true, readings);

    TEST_ASSERT_FALSE(lightActuator->isOn()); // Turns OFF due to isError=true
}

// ----------------------------------------------------
// 5. Manual Mode Control Tests
// ----------------------------------------------------

static void pressButton(ButtonDriver& btn, int pin) {
    setMockPinValue(pin, LOW);
    btn.wasPressed(); // Register state change to start debounce timer
    advanceSimulatedMillis(60);
}

static void releaseButton(ButtonDriver& btn, int pin) {
    setMockPinValue(pin, HIGH);
    btn.wasPressed(); // Register state change
    advanceSimulatedMillis(60);
    btn.wasPressed(); // Complete release debounce
}

void test_manual_mode_button_toggles(void) {
    ButtonDriver btnIrrig(PIN_BTN_IRRIG);
    ButtonDriver btnVent(PIN_BTN_VENT);
    ButtonDriver btnLight(PIN_BTN_LIGHT);

    SensorDataMap readings(0);

    // Initial state all off
    TEST_ASSERT_FALSE(irrigActuator->isOn());
    TEST_ASSERT_FALSE(ventActuator->isOn());
    TEST_ASSERT_FALSE(lightActuator->isOn());

    // --- Irrigation Button Press 1 (Turn ON) ---
    pressButton(btnIrrig, PIN_BTN_IRRIG);
    automation->update(false, readings, &btnIrrig, &btnVent, &btnLight);
    TEST_ASSERT_TRUE(irrigActuator->isOn());

    // Irrigation Button Release
    releaseButton(btnIrrig, PIN_BTN_IRRIG);

    // --- Irrigation Button Press 2 (Turn OFF) ---
    pressButton(btnIrrig, PIN_BTN_IRRIG);
    automation->update(false, readings, &btnIrrig, &btnVent, &btnLight);
    TEST_ASSERT_FALSE(irrigActuator->isOn());

    // Irrigation Button Release
    releaseButton(btnIrrig, PIN_BTN_IRRIG);

    // --- Ventilation Button Press (Turn ON) ---
    pressButton(btnVent, PIN_BTN_VENT);
    automation->update(false, readings, &btnIrrig, &btnVent, &btnLight);
    TEST_ASSERT_TRUE(ventActuator->isOn());

    // --- Light Button Press (Turn ON) ---
    pressButton(btnLight, PIN_BTN_LIGHT);
    automation->update(false, readings, &btnIrrig, &btnVent, &btnLight);
    TEST_ASSERT_TRUE(lightActuator->isOn());
}

void test_manual_mode_null_drivers_safety(void) {
    SensorDataMap readings(0);
    // Should execute safely without crash when null pointers passed
    automation->update(false, readings, nullptr, nullptr, nullptr);
    TEST_ASSERT_FALSE(irrigActuator->isOn());
}

void test_manual_mode_critical_temp_alert(void) {
    tempSensor->setData(65.0f, false); // > 60.0°C threshold
    SensorDataMap readings(1);
    readings[0] = {tempSensor, tempSensor->read()};

    automation->update(false, readings);

    TEST_ASSERT_EQUAL_UINT(1000, getMockBuzzerTone(PIN_BUZZER));
}

void test_manual_mode_critical_soil_alert(void) {
    soilSensor->setData(90.0f, false); // > 85.0% threshold
    SensorDataMap readings(1);
    readings[0] = {soilSensor, soilSensor->read()};

    automation->update(false, readings);

    TEST_ASSERT_EQUAL_UINT(1000, getMockBuzzerTone(PIN_BUZZER));
}

void test_manual_mode_critical_light_alert(void) {
    lightSensor->setData(12000.0f, false); // > 10000 threshold
    SensorDataMap readings(1);
    readings[0] = {lightSensor, lightSensor->read()};

    automation->update(false, readings);

    TEST_ASSERT_EQUAL_UINT(1000, getMockBuzzerTone(PIN_BUZZER));
}

void test_manual_mode_critical_humidity_alert(void) {
    MockSensor humSensor(PIN_DHT, "Humidity", "%");
    humSensor.setData(88.0f, false); // > 85.0% threshold
    SensorDataMap readings(1);
    readings[0] = {&humSensor, humSensor.read()};

    automation->update(false, readings);

    TEST_ASSERT_EQUAL_UINT(1000, getMockBuzzerTone(PIN_BUZZER));
}

void test_manual_mode_safe_sensors_no_alert(void) {
    tempSensor->setData(25.0f, false);
    soilSensor->setData(50.0f, false);
    lightSensor->setData(5000.0f, false);

    SensorDataMap readings(3);
    readings[0] = {tempSensor, tempSensor->read()};
    readings[1] = {soilSensor, soilSensor->read()};
    readings[2] = {lightSensor, lightSensor->read()};

    automation->update(false, readings);

    TEST_ASSERT_EQUAL_UINT(0, getMockBuzzerTone(PIN_BUZZER));
}

// ----------------------------------------------------
// 6. System Indicators & Buzzer Alarm Tests
// ----------------------------------------------------

void test_system_indicators_normal_operation(void) {
    tempSensor->setData(24.0f, false);
    soilSensor->setData(50.0f, false);

    SensorDataMap readings(2);
    readings[0] = {tempSensor, tempSensor->read()};
    readings[1] = {soilSensor, soilSensor->read()};

    automation->update(true, readings);

    TEST_ASSERT_EQUAL_INT(LOW, getMockPinValue(PIN_LED_RED));
    TEST_ASSERT_EQUAL_INT(HIGH, getMockPinValue(PIN_LED_GREEN));
    TEST_ASSERT_EQUAL_UINT(0, getMockBuzzerTone(PIN_BUZZER));
}

void test_system_indicators_sensor_error_led(void) {
    tempSensor->setData(24.0f, true); // Error state

    SensorDataMap readings(1);
    readings[0] = {tempSensor, tempSensor->read()};

    automation->update(true, readings);

    TEST_ASSERT_EQUAL_INT(HIGH, getMockPinValue(PIN_LED_RED));
    TEST_ASSERT_EQUAL_INT(LOW, getMockPinValue(PIN_LED_GREEN));
}

void test_system_indicators_high_alert_buzzer_alarm(void) {
    tempSensor->setData(30.0f, false); // Triggers high temp alert

    SensorDataMap readings(1);
    readings[0] = {tempSensor, tempSensor->read()};

    automation->update(true, readings);

    TEST_ASSERT_EQUAL_UINT(1000, getMockBuzzerTone(PIN_BUZZER));
}

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;
    UNITY_BEGIN();

    RUN_TEST(test_actuator_registration_and_lookup);
    
    RUN_TEST(test_auto_ventilation_high_temp_opens);
    RUN_TEST(test_auto_ventilation_hysteresis_holds_open);
    RUN_TEST(test_auto_ventilation_low_temp_closes);
    RUN_TEST(test_auto_ventilation_hysteresis_holds_closed);
    RUN_TEST(test_auto_ventilation_sensor_error_isolation);

    RUN_TEST(test_auto_irrigation_dry_soil_turns_on);
    RUN_TEST(test_auto_irrigation_hysteresis_holds_on);
    RUN_TEST(test_auto_irrigation_sufficient_moisture_turns_off);
    RUN_TEST(test_auto_irrigation_sensor_error_isolation);

    RUN_TEST(test_auto_light_darkness_turns_on);
    RUN_TEST(test_auto_light_hysteresis_holds_on);
    RUN_TEST(test_auto_light_daylight_turns_off);
    RUN_TEST(test_auto_light_sensor_error_isolation);

    RUN_TEST(test_manual_mode_button_toggles);
    RUN_TEST(test_manual_mode_null_drivers_safety);
    RUN_TEST(test_manual_mode_critical_temp_alert);
    RUN_TEST(test_manual_mode_critical_soil_alert);
    RUN_TEST(test_manual_mode_critical_light_alert);
    RUN_TEST(test_manual_mode_critical_humidity_alert);
    RUN_TEST(test_manual_mode_safe_sensors_no_alert);

    RUN_TEST(test_system_indicators_normal_operation);
    RUN_TEST(test_system_indicators_sensor_error_led);
    RUN_TEST(test_system_indicators_high_alert_buzzer_alarm);

    return UNITY_END();
}
