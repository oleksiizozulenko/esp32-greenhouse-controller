#include <unity.h>
#include "../Arduino.h"
#include "../MockActuator.h"
#include "../MockSensor.h"
#include "../../include/GreenhouseController.h"
#include "../../include/services/SensorsService.h"

static GreenhouseController* automation;
static MockActuator* ventActuator;
static MockActuator* irrigActuator;
static MockActuator* lightActuator;

static MockSensor* tempSensor;
static MockSensor* soilSensor;
static MockSensor* lightSensor;

void setUp(void) {
    resetMockArduinoState();

    automation = new GreenhouseController(4, PIN_LED_RED, PIN_LED_GREEN, PIN_BUZZER);

    ventActuator = new MockActuator(PIN_ACTUATOR_VENT, ActuatorType::VENTILATION, "Ventilation");
    irrigActuator = new MockActuator(PIN_ACTUATOR_IRRIG, ActuatorType::IRRIGATION, "Irrigation");
    lightActuator = new MockActuator(PIN_ACTUATOR_LIGHT, ActuatorType::LIGHT, "Light");

    automation->addActuator(ventActuator);
    automation->addActuator(irrigActuator);
    automation->addActuator(lightActuator);

    tempSensor = new MockSensor(PIN_TEMP, SensorType::TEMPERATURE, "Temperature", "°C");
    soilSensor = new MockSensor(PIN_SOIL_POT, SensorType::SOIL, "Soil", "%");
    lightSensor = new MockSensor(PIN_LDR, SensorType::LIGHT, "Light", "lux");

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
    TEST_ASSERT_EQUAL_PTR(ventActuator, automation->getActuator(ActuatorType::VENTILATION));
    TEST_ASSERT_EQUAL_PTR(irrigActuator, automation->getActuator(ActuatorType::IRRIGATION));
    TEST_ASSERT_EQUAL_PTR(lightActuator, automation->getActuator(ActuatorType::LIGHT));
    TEST_ASSERT_NULL(automation->getActuator(ActuatorType::UNKNOWN));
    TEST_ASSERT_NULL(automation->getActuator(99));
}



// ----------------------------------------------------
// 2. Automatic Ventilation Control Tests
// ----------------------------------------------------

void test_auto_ventilation_high_temp_opens(void) {
    tempSensor->setData(29.0f, false); // > 28.0°C threshold
    soilSensor->setData(50.0f, false);
    lightSensor->setData(4000.0f, false);

    SensorDataMap readings(3);
    readings[0] = {tempSensor, tempSensor->read()};
    readings[1] = {soilSensor, soilSensor->read()};
    readings[2] = {lightSensor, lightSensor->read()};

    automation->update(true, readings);

    TEST_ASSERT_TRUE(ventActuator->isOn());
    TEST_ASSERT_EQUAL_INT(1, ventActuator->getTurnOnCalls());
}

void test_auto_ventilation_high_humidity_opens(void) {
    MockSensor humSensor(PIN_DHT, SensorType::HUMIDITY, "Humidity", "%");
    tempSensor->setData(24.0f, false); // Normal temp (< 28°C)
    humSensor.setData(75.0f, false);   // > 70% threshold

    SensorDataMap readings(2);
    readings[0] = {tempSensor, tempSensor->read()};
    readings[1] = {&humSensor, humSensor.read()};

    automation->update(true, readings);

    TEST_ASSERT_TRUE(ventActuator->isOn());
}

void test_auto_ventilation_humidity_hysteresis_holds_open(void) {
    MockSensor humSensor(PIN_DHT, SensorType::HUMIDITY, "Humidity", "%");
    humSensor.setData(75.0f, false);
    SensorDataMap readings1(1);
    readings1[0] = {&humSensor, humSensor.read()};
    automation->update(true, readings1);
    TEST_ASSERT_TRUE(ventActuator->isOn());

    // Drops to 67% (between 65% and 70%)
    humSensor.setData(67.0f, false);
    SensorDataMap readings2(1);
    readings2[0] = {&humSensor, humSensor.read()};
    automation->update(true, readings2);

    TEST_ASSERT_TRUE(ventActuator->isOn()); // Holds open
}

void test_auto_ventilation_normal_temp_and_humidity_closes(void) {
    MockSensor humSensor(PIN_DHT, SensorType::HUMIDITY, "Humidity", "%");
    ventActuator->turnOn();

    tempSensor->setData(25.0f, false); // < 26.0°C (28 - 2)
    humSensor.setData(62.0f, false);   // < 65.0% (70 - 5)
    SensorDataMap readings(2);
    readings[0] = {tempSensor, tempSensor->read()};
    readings[1] = {&humSensor, humSensor.read()};

    automation->update(true, readings);

    TEST_ASSERT_FALSE(ventActuator->isOn());
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

void test_auto_irrigation_high_moisture_80_does_not_turn_on(void) {
    soilSensor->setData(80.0f, false); // High soil moisture (~80%)
    SensorDataMap readings(1);
    readings[0] = {soilSensor, soilSensor->read()};

    automation->update(true, readings);

    TEST_ASSERT_FALSE(irrigActuator->isOn());
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
    lightSensor->setData(2500.0f, false); // < 3000 lx threshold
    SensorDataMap readings(1);
    readings[0] = {lightSensor, lightSensor->read()};

    automation->update(true, readings);

    TEST_ASSERT_TRUE(lightActuator->isOn());
}

void test_auto_light_hysteresis_holds_on(void) {
    lightActuator->turnOn();

    // Light rises to 3200 (in hysteresis zone 3000 - 3500 lx)
    lightSensor->setData(3200.0f, false);
    SensorDataMap readings(1);
    readings[0] = {lightSensor, lightSensor->read()};

    automation->update(true, readings);

    TEST_ASSERT_TRUE(lightActuator->isOn());
}

void test_auto_light_daylight_turns_off(void) {
    lightActuator->turnOn();

    // Light rises above 3500 lx (3000 + 500)
    lightSensor->setData(3600.0f, false);
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
// 5. Manual Mode Control Tests & Mode Isolation
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

void test_mode_toggle_is_sticky(void) {
    SystemMode mode = SystemMode::MANUAL;
    mode = toggleSystemMode(mode);
    TEST_ASSERT_EQUAL_INT(SystemMode::AUTOMATIC, mode);
    mode = toggleSystemMode(mode);
    TEST_ASSERT_EQUAL_INT(SystemMode::MANUAL, mode);
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

void test_manual_mode_button_override_under_critical_hazard(void) {
    ButtonDriver btnVent(PIN_BTN_VENT);

    // Temp is 59°C (> 45°C critical overheat)
    tempSensor->setData(59.0f, false);
    SensorDataMap readings(1);
    readings[0] = {tempSensor, tempSensor->read()};

    // In MANUAL mode, user presses ventilation button -> MUST open ventilation unconditionally
    pressButton(btnVent, PIN_BTN_VENT);
    automation->update(false, readings, nullptr, &btnVent, nullptr);
    TEST_ASSERT_TRUE(ventActuator->isOn());

    releaseButton(btnVent, PIN_BTN_VENT);

    // User presses ventilation button again -> MUST close ventilation unconditionally
    pressButton(btnVent, PIN_BTN_VENT);
    automation->update(false, readings, nullptr, &btnVent, nullptr);
    TEST_ASSERT_FALSE(ventActuator->isOn());
}

void test_auto_mode_ignores_manual_buttons(void) {
    ButtonDriver btnVent(PIN_BTN_VENT);
    ButtonDriver btnIrrig(PIN_BTN_IRRIG);
    ButtonDriver btnLight(PIN_BTN_LIGHT);

    // Normal safe sensor readings in AUTO mode
    tempSensor->setData(22.0f, false);
    soilSensor->setData(50.0f, false);
    lightSensor->setData(4000.0f, false);

    SensorDataMap readings(3);
    readings[0] = {tempSensor, tempSensor->read()};
    readings[1] = {soilSensor, soilSensor->read()};
    readings[2] = {lightSensor, lightSensor->read()};

    // User presses buttons during AUTO mode
    pressButton(btnVent, PIN_BTN_VENT);
    pressButton(btnIrrig, PIN_BTN_IRRIG);
    pressButton(btnLight, PIN_BTN_LIGHT);

    automation->update(true, readings, &btnIrrig, &btnVent, &btnLight);

    // In AUTO mode, manual button presses MUST NOT turn actuators ON
    TEST_ASSERT_FALSE(ventActuator->isOn());
    TEST_ASSERT_FALSE(irrigActuator->isOn());
    TEST_ASSERT_FALSE(lightActuator->isOn());
}

void test_manual_mode_null_drivers_safety(void) {
    SensorDataMap readings(0);
    // Should execute safely without crash when null pointers passed
    automation->update(false, readings, nullptr, nullptr, nullptr);
    TEST_ASSERT_FALSE(irrigActuator->isOn());
}

void test_manual_mode_critical_temp_alert(void) {
    tempSensor->setData(65.0f, false); // > 45.0°C threshold
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
    lightSensor->setData(26000.0f, false); // > 25000 lx threshold
    SensorDataMap readings(1);
    readings[0] = {lightSensor, lightSensor->read()};

    SafetyMonitorService monitor;
    SystemHealthState state = monitor.evaluate(readings, false);

    TEST_ASSERT_TRUE(state.hasOperatorAdvisory);
    TEST_ASSERT_FALSE(state.requiresAlarm); // High light is operator advisory (no loud buzzer)
    TEST_ASSERT_EQUAL_STRING("LIGHT HIGH! Press LIGHT", state.advisoryMsg);
}

void test_manual_mode_critical_humidity_alert(void) {
    MockSensor humSensor(PIN_DHT, SensorType::HUMIDITY, "Humidity", "%");
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
    tempSensor->setData(46.0f, false); // Triggers critical overheat alarm (>45.0°C)

    SensorDataMap readings(1);
    readings[0] = {tempSensor, tempSensor->read()};

    automation->update(true, readings);

    TEST_ASSERT_EQUAL_UINT(1000, getMockBuzzerTone(PIN_BUZZER));
}

// ----------------------------------------------------
// 7. SafetyMonitorService Tests
// ----------------------------------------------------

void test_safety_nan_and_inf_per_sensor(void) {
    SafetyMonitorService monitor;

    // NaN in temperature -> Hardware error
    tempSensor->setData(NAN, false);
    SensorDataMap readings(1);
    readings[0] = {tempSensor, tempSensor->read()};

    SystemHealthState state = monitor.evaluate(readings, true);
    TEST_ASSERT_TRUE(state.hasHardwareError);
    TEST_ASSERT_TRUE(state.requiresAlarm);
    TEST_ASSERT_EQUAL_STRING("SENSOR ERROR!", state.advisoryMsg);
}

void test_safety_missing_sensor_in_map(void) {
    SafetyMonitorService monitor;

    // Map contains only Temperature (safe value) -> No hardware error from missing sensors
    tempSensor->setData(25.0f, false);
    SensorDataMap readings(1);
    readings[0] = {tempSensor, tempSensor->read()};

    SystemHealthState state = monitor.evaluate(readings, true);
    TEST_ASSERT_FALSE(state.hasHardwareError);
    TEST_ASSERT_FALSE(state.hasCriticalHazard);
    TEST_ASSERT_EQUAL_STRING("", state.advisoryMsg);
}

void test_safety_exact_boundary_inclusivity(void) {
    SafetyMonitorService monitor;

    // 0.0 lx is valid inclusive bound -> No error
    MockSensor lightS(PIN_LDR, SensorType::LIGHT, "Light", "lx");
    lightS.setData(0.0f, false);
    SensorDataMap readings(1);
    readings[0] = {&lightS, lightS.read()};

    SystemHealthState state = monitor.evaluate(readings, true);
    TEST_ASSERT_FALSE(state.hasHardwareError);

    // 100000.0 lx is valid inclusive max bound -> No error
    lightS.setData(100000.0f, false);
    advanceSimulatedMillis(2001);
    readings[0] = {&lightS, lightS.read()};
    state = monitor.evaluate(readings, true);
    TEST_ASSERT_FALSE(state.hasHardwareError);

    // 100000.1 lx is out-of-bounds -> Hardware Error
    lightS.setData(100000.1f, false);
    advanceSimulatedMillis(2001);
    readings[0] = {&lightS, lightS.read()};
    state = monitor.evaluate(readings, true);
    TEST_ASSERT_TRUE(state.hasHardwareError);
    TEST_ASSERT_EQUAL_STRING("SENSOR ERROR!", state.advisoryMsg);

    // 25000.0 lx -> Normal
    lightS.setData(25000.0f, false);
    advanceSimulatedMillis(2001);
    readings[0] = {&lightS, lightS.read()};
    state = monitor.evaluate(readings, true);
    TEST_ASSERT_FALSE(state.hasOperatorAdvisory);

    // 25000.1 lx -> High Light Advisory
    lightS.setData(25000.1f, false);
    advanceSimulatedMillis(2001);
    readings[0] = {&lightS, lightS.read()};
    state = monitor.evaluate(readings, true);
    TEST_ASSERT_TRUE(state.hasOperatorAdvisory);
    TEST_ASSERT_EQUAL_STRING("LIGHT HIGH! Press LIGHT", state.advisoryMsg);
}

void test_safety_conflicting_hazards_priority(void) {
    SafetyMonitorService monitor;

    // Both Overheat (46°C) and Dry Soil (20%) present -> Overheat (Priority 2) wins over Dry Soil (Priority 6)
    tempSensor->setData(46.0f, false);
    soilSensor->setData(20.0f, false);
    SensorDataMap readings(2);
    readings[0] = {tempSensor, tempSensor->read()};
    readings[1] = {soilSensor, soilSensor->read()};

    SystemHealthState state = monitor.evaluate(readings, true);
    TEST_ASSERT_TRUE(state.hasCriticalHazard);
    TEST_ASSERT_TRUE(state.requiresAlarm);
    TEST_ASSERT_EQUAL_STRING("TEMP HIGH! Press VENT", state.advisoryMsg);
}

void test_safety_hardware_error_plus_hazard(void) {
    SafetyMonitorService monitor;

    // Hardware Error (NaN) on Soil AND Overheat (50°C) -> Hardware Error (Priority 1) wins
    soilSensor->setData(NAN, false);
    tempSensor->setData(50.0f, false);
    SensorDataMap readings(2);
    readings[0] = {soilSensor, soilSensor->read()};
    readings[1] = {tempSensor, tempSensor->read()};

    SystemHealthState state = monitor.evaluate(readings, true);
    TEST_ASSERT_TRUE(state.hasHardwareError);
    TEST_ASSERT_TRUE(state.requiresAlarm);
    TEST_ASSERT_EQUAL_STRING("SENSOR ERROR!", state.advisoryMsg);
}

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;
    UNITY_BEGIN();

    RUN_TEST(test_actuator_registration_and_lookup);

    RUN_TEST(test_auto_ventilation_high_temp_opens);
    RUN_TEST(test_auto_ventilation_high_humidity_opens);
    RUN_TEST(test_auto_ventilation_humidity_hysteresis_holds_open);
    RUN_TEST(test_auto_ventilation_normal_temp_and_humidity_closes);
    RUN_TEST(test_auto_ventilation_hysteresis_holds_open);
    RUN_TEST(test_auto_ventilation_low_temp_closes);
    RUN_TEST(test_auto_ventilation_hysteresis_holds_closed);
    RUN_TEST(test_auto_ventilation_sensor_error_isolation);

    RUN_TEST(test_auto_irrigation_dry_soil_turns_on);
    RUN_TEST(test_auto_irrigation_high_moisture_80_does_not_turn_on);
    RUN_TEST(test_auto_irrigation_hysteresis_holds_on);
    RUN_TEST(test_auto_irrigation_sufficient_moisture_turns_off);
    RUN_TEST(test_auto_irrigation_sensor_error_isolation);

    RUN_TEST(test_auto_light_darkness_turns_on);
    RUN_TEST(test_auto_light_hysteresis_holds_on);
    RUN_TEST(test_auto_light_daylight_turns_off);
    RUN_TEST(test_auto_light_sensor_error_isolation);

    RUN_TEST(test_mode_toggle_is_sticky);
    RUN_TEST(test_manual_mode_button_toggles);
    RUN_TEST(test_manual_mode_button_override_under_critical_hazard);
    RUN_TEST(test_auto_mode_ignores_manual_buttons);
    RUN_TEST(test_manual_mode_null_drivers_safety);
    RUN_TEST(test_manual_mode_critical_temp_alert);
    RUN_TEST(test_manual_mode_critical_soil_alert);
    RUN_TEST(test_manual_mode_critical_light_alert);
    RUN_TEST(test_manual_mode_critical_humidity_alert);
    RUN_TEST(test_manual_mode_safe_sensors_no_alert);

    RUN_TEST(test_system_indicators_normal_operation);
    RUN_TEST(test_system_indicators_sensor_error_led);
    RUN_TEST(test_system_indicators_high_alert_buzzer_alarm);

    RUN_TEST(test_safety_nan_and_inf_per_sensor);
    RUN_TEST(test_safety_missing_sensor_in_map);
    RUN_TEST(test_safety_exact_boundary_inclusivity);
    RUN_TEST(test_safety_conflicting_hazards_priority);
    RUN_TEST(test_safety_hardware_error_plus_hazard);

    return UNITY_END();
}

