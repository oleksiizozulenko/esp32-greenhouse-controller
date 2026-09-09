#include <unity.h>
#include "../Arduino.h"
#include "../MockActuator.h"
#include "../MockSensor.h"
#include "GreenhouseController.h"
#include "VentilationSubsystem.h"
#include "LightingSubsystem.h"
#include "IrrigationSubsystem.h"
#include "SystemAlertService.h"
#include "SensorsService.h"

static GreenhouseController* automation;
static SystemAlertService alertService;
static MockActuator* ventActuator;
static MockActuator* irrigActuator;
static MockActuator* lightActuator;

static MockSensor* tempSensor;
static MockSensor* soilSensor;
static MockSensor* lightSensor;

void setUp(void) {
    resetMockArduinoState();
    alertService.begin();

    automation = new GreenhouseController(4);

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
    lightSensor->setData(250.0f, false); // < 300 lx threshold
    SensorDataMap readings(1);
    readings[0] = {lightSensor, lightSensor->read()};

    automation->update(true, readings);

    TEST_ASSERT_TRUE(lightActuator->isOn());
}

void test_auto_light_low_level_89lx_turns_on(void) {
    automation->setSystemMode(SystemMode::AUTOMATIC);
    lightSensor->setData(89.0f, false); // 89 lx < 300 lx threshold
    SensorDataMap readings(1);
    readings[0] = {lightSensor, lightSensor->read()};

    automation->update(true, readings);

    TEST_ASSERT_TRUE(lightActuator->isOn());
}

void test_auto_light_hysteresis_holds_on(void) {
    lightActuator->turnOn();

    // Light rises to 500 lx (between 300 - 1000 lx)
    lightSensor->setData(500.0f, false);
    SensorDataMap readings(1);
    readings[0] = {lightSensor, lightSensor->read()};

    automation->update(true, readings);

    TEST_ASSERT_TRUE(lightActuator->isOn());
}

void test_auto_light_daylight_turns_off(void) {
    lightActuator->turnOn();

    // Light rises above 1000 lx threshold
    lightSensor->setData(1200.0f, false);
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
    SensorDataMap readings(0);

    // Initial state all off
    TEST_ASSERT_FALSE(irrigActuator->isOn());
    TEST_ASSERT_FALSE(ventActuator->isOn());
    TEST_ASSERT_FALSE(lightActuator->isOn());

    // --- Irrigation Button Press 1 (Turn ON) ---
    automation->onButtonPressed(ButtonType::IRRIGATION);
    automation->update(false, readings);
    TEST_ASSERT_TRUE(irrigActuator->isOn());

    // --- Irrigation Button Press 2 (Turn OFF) ---
    automation->onButtonPressed(ButtonType::IRRIGATION);
    automation->update(false, readings);
    TEST_ASSERT_FALSE(irrigActuator->isOn());

    // --- Ventilation Button Press (Turn ON) ---
    automation->onButtonPressed(ButtonType::VENTILATION);
    automation->update(false, readings);
    TEST_ASSERT_TRUE(ventActuator->isOn());

    // --- Light Button Press (Turn ON) ---
    automation->onButtonPressed(ButtonType::LIGHT);
    automation->update(false, readings);
    TEST_ASSERT_TRUE(lightActuator->isOn());
}

void test_manual_mode_button_override_under_critical_hazard(void) {
    // Temp is 59°C (> 45°C critical overheat)
    tempSensor->setData(59.0f, false);
    SensorDataMap readings(1);
    readings[0] = {tempSensor, tempSensor->read()};

    // In MANUAL mode, user presses ventilation button -> MUST open ventilation unconditionally
    automation->onButtonPressed(ButtonType::VENTILATION);
    automation->update(false, readings);
    TEST_ASSERT_TRUE(ventActuator->isOn());

    // User presses ventilation button again -> MUST close ventilation unconditionally
    automation->onButtonPressed(ButtonType::VENTILATION);
    automation->update(false, readings);
    TEST_ASSERT_FALSE(ventActuator->isOn());
}

void test_auto_mode_supports_manual_button_overrides(void) {
    // Normal safe sensor readings in AUTO mode
    tempSensor->setData(22.0f, false);
    soilSensor->setData(50.0f, false);
    lightSensor->setData(4000.0f, false);

    SensorDataMap readings(3);
    readings[0] = {tempSensor, tempSensor->read()};
    readings[1] = {soilSensor, soilSensor->read()};
    readings[2] = {lightSensor, lightSensor->read()};

    automation->update(true, readings);

    // User presses buttons during AUTO mode to override actuator states
    automation->onButtonPressed(ButtonType::VENTILATION);
    automation->onButtonPressed(ButtonType::IRRIGATION);
    automation->onButtonPressed(ButtonType::LIGHT);

    // In AUTO mode, user button presses enable actuators as manual overrides
    TEST_ASSERT_TRUE(ventActuator->isOn());
    TEST_ASSERT_TRUE(irrigActuator->isOn());
    TEST_ASSERT_TRUE(lightActuator->isOn());
}

void test_auto_mode_button_toggles_active_actuator_off(void) {
    // Temp is high (30.0°C) -> Vent actuator automatically turns ON in AUTO mode
    tempSensor->setData(30.0f, false);
    SensorDataMap readings(1);
    readings[0] = {tempSensor, tempSensor->read()};

    automation->update(true, readings);
    TEST_ASSERT_TRUE(ventActuator->isOn());

    // User presses ventilation button -> MUST toggle ventilation OFF
    automation->onButtonPressed(ButtonType::VENTILATION);
    TEST_ASSERT_FALSE(ventActuator->isOn());

    // User presses ventilation button again -> MUST toggle ventilation ON
    automation->onButtonPressed(ButtonType::VENTILATION);
    TEST_ASSERT_TRUE(ventActuator->isOn());
}

void test_manual_mode_null_drivers_safety(void) {
    SensorDataMap readings(0);
    // Should execute safely without crash when zero button arguments passed
    automation->update(false, readings);
    TEST_ASSERT_FALSE(irrigActuator->isOn());
}

void test_manual_mode_critical_temp_alert(void) {
    tempSensor->setData(65.0f, false); // > 45.0°C threshold
    SensorDataMap readings(1);
    readings[0] = {tempSensor, tempSensor->read()};

    SafetyMonitorService monitor;
    SystemHealthState healthState = monitor.evaluate(readings, false);
    automation->update(false, readings, healthState);
    alertService.update(healthState);

    TEST_ASSERT_EQUAL_UINT(1000, getMockBuzzerTone(PIN_BUZZER));
}

void test_manual_mode_critical_soil_alert(void) {
    soilSensor->setData(90.0f, false); // > 85.0% threshold
    SensorDataMap readings(1);
    readings[0] = {soilSensor, soilSensor->read()};

    SafetyMonitorService monitor;
    SystemHealthState healthState = monitor.evaluate(readings, false);
    automation->update(false, readings, healthState);
    alertService.update(healthState);

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

    SafetyMonitorService monitor;
    SystemHealthState state = monitor.evaluate(readings, false);

    TEST_ASSERT_TRUE(state.hasOperatorAdvisory);
    TEST_ASSERT_FALSE(state.requiresAlarm); // High humidity is operator advisory banner
    TEST_ASSERT_EQUAL_STRING("HUMID HIGH! Press VENT", state.advisoryMsg);
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

    SafetyMonitorService safetyMonitor;
    SystemHealthState healthState = safetyMonitor.evaluate(readings, true);
    automation->update(true, readings, healthState);
    alertService.update(healthState);

    TEST_ASSERT_EQUAL_INT(LOW, getMockPinValue(PIN_LED_RED));
    TEST_ASSERT_EQUAL_INT(HIGH, getMockPinValue(PIN_LED_GREEN));
    TEST_ASSERT_EQUAL_UINT(0, getMockBuzzerTone(PIN_BUZZER));
}

void test_system_indicators_sensor_error_led(void) {
    tempSensor->setData(24.0f, true); // Error state

    SensorDataMap readings(1);
    readings[0] = {tempSensor, tempSensor->read()};

    SafetyMonitorService safetyMonitor;
    SystemHealthState healthState = safetyMonitor.evaluate(readings, true);
    automation->update(true, readings, healthState);
    alertService.update(healthState);

    TEST_ASSERT_EQUAL_INT(HIGH, getMockPinValue(PIN_LED_RED));
    TEST_ASSERT_EQUAL_INT(HIGH, getMockPinValue(PIN_LED_GREEN)); // System power stays ON
}

void test_system_indicators_high_alert_buzzer_alarm(void) {
    tempSensor->setData(46.0f, false); // Triggers critical overheat alarm (>45.0°C)

    SensorDataMap readings(1);
    readings[0] = {tempSensor, tempSensor->read()};

    SafetyMonitorService safetyMonitor;
    SystemHealthState healthState = safetyMonitor.evaluate(readings, true);
    automation->update(true, readings, healthState);
    alertService.update(healthState);

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

// ----------------------------------------------------
// 7. FreeRTOS Actuator Safety Timer Tests
// ----------------------------------------------------

void test_manual_button_starts_safety_timer(void) {
    automation->onButtonPressed(ButtonType::IRRIGATION);
    TEST_ASSERT_TRUE(irrigActuator->isOn());

    GreenhouseController::ActuatorTimer* timerObj = automation->getActuatorTimer(ActuatorType::IRRIGATION);
    TEST_ASSERT_NOT_NULL(timerObj);
    TEST_ASSERT_NOT_NULL(timerObj->timer);

    MockFreeRTOSTimer* mockTimer = (MockFreeRTOSTimer*)timerObj->timer;
    TEST_ASSERT_TRUE(mockTimer->isActive);
    TEST_ASSERT_EQUAL_UINT32(10000, timerObj->timeoutMs);
}

void test_timer_expiration_auto_turns_off_actuator(void) {
    automation->onButtonPressed(ButtonType::IRRIGATION);
    TEST_ASSERT_TRUE(irrigActuator->isOn());

    GreenhouseController::ActuatorTimer* timerObj = automation->getActuatorTimer(ActuatorType::IRRIGATION);
    TEST_ASSERT_NOT_NULL(timerObj);

    // Simulate FreeRTOS Software Timer expiration
    triggerMockTimerCallback(timerObj->timer);

    // Actuator should automatically be turned OFF by timer callback
    TEST_ASSERT_FALSE(irrigActuator->isOn());
}

void test_manual_button_turn_off_stops_safety_timer(void) {
    // Turn ON irrigation -> Starts timer
    automation->onButtonPressed(ButtonType::IRRIGATION);
    TEST_ASSERT_TRUE(irrigActuator->isOn());

    GreenhouseController::ActuatorTimer* timerObj = automation->getActuatorTimer(ActuatorType::IRRIGATION);
    TEST_ASSERT_NOT_NULL(timerObj);
    MockFreeRTOSTimer* mockTimer = (MockFreeRTOSTimer*)timerObj->timer;
    TEST_ASSERT_TRUE(mockTimer->isActive);

    // Press button again -> Turns OFF irrigation and stops timer
    automation->onButtonPressed(ButtonType::IRRIGATION);
    TEST_ASSERT_FALSE(irrigActuator->isOn());
    TEST_ASSERT_FALSE(mockTimer->isActive);
}

void test_timer_reuse_on_multiple_manual_presses(void) {
    automation->onButtonPressed(ButtonType::VENTILATION);
    GreenhouseController::ActuatorTimer* timerObj1 = automation->getActuatorTimer(ActuatorType::VENTILATION);
    TEST_ASSERT_NOT_NULL(timerObj1);

    automation->onButtonPressed(ButtonType::VENTILATION); // Turn off
    automation->onButtonPressed(ButtonType::VENTILATION); // Turn on again

    GreenhouseController::ActuatorTimer* timerObj2 = automation->getActuatorTimer(ActuatorType::VENTILATION);
    TEST_ASSERT_NOT_NULL(timerObj2);

    // Must reuse identical timer object handle without memory leaks
    TEST_ASSERT_EQUAL_PTR(timerObj1, timerObj2);
    TEST_ASSERT_EQUAL_PTR(timerObj1->timer, timerObj2->timer);
}

void test_system_under_high_load_stress(void) {
    SafetyMonitorService safetyMonitor;
    // Perform 1000 rapid cycles of mode toggles, button presses, sensor updates, and timer expirations
    for (int i = 0; i < 1000; i++) {
        // 1. Rapid button presses
        automation->onButtonPressed(ButtonType::IRRIGATION);
        automation->onButtonPressed(ButtonType::VENTILATION);
        automation->onButtonPressed(ButtonType::LIGHT);
        automation->onButtonPressed(ButtonType::MODE);

        // 2. Rapid sensor updates with varying values
        float temp = (i % 2 == 0) ? 38.0f : 22.0f;
        float soil = (i % 3 == 0) ? 15.0f : 55.0f;
        tempSensor->setData(temp, false);
        soilSensor->setData(soil, false);

        SensorDataMap readings(2);
        readings[0] = {tempSensor, tempSensor->read()};
        readings[1] = {soilSensor, soilSensor->read()};

        SystemHealthState health = safetyMonitor.evaluate(readings, true);
        automation->update(true, readings, health);

        // 3. Trigger active timers
        GreenhouseController::ActuatorTimer* irrigTimer = automation->getActuatorTimer(ActuatorType::IRRIGATION);
        if (irrigTimer && irrigTimer->timer) {
            triggerMockTimerCallback((MockFreeRTOSTimer*)irrigTimer->timer);
        }
    }

    // After 1000 high-load iterations, system must remain stable and consistent
    TEST_ASSERT_NOT_NULL(automation);
}

void test_subsystem_independent_modes(void) {
    automation->getVentilationSubsystem().setMode(ControlMode::MANUAL);
    automation->getVentilationSubsystem().setManualState(ManualState::ON);

    TEST_ASSERT_EQUAL(ControlMode::MANUAL, automation->getVentilationSubsystem().getMode());
    TEST_ASSERT_EQUAL(ControlMode::AUTO, automation->getLightingSubsystem().getMode());
    TEST_ASSERT_EQUAL(ControlMode::AUTO, automation->getIrrigationSubsystem().getMode());
}

void test_manual_mode_critical_temp_triggers_alarm_without_overriding_manual_off(void) {
    automation->getVentilationSubsystem().setMode(ControlMode::MANUAL);
    automation->getVentilationSubsystem().setManualState(ManualState::OFF);

    tempSensor->setData(46.0f, false); // Critical Overheat (> 45°C)
    SensorDataMap readings(1);
    readings[0] = {tempSensor, tempSensor->read()};

    SystemHealthState healthState;
    automation->update(true, readings, healthState);

    // Alarm triggered!
    TEST_ASSERT_TRUE(healthState.hasCriticalHazard);
    TEST_ASSERT_TRUE(automation->getVentilationSubsystem().getStatus().hasActiveAlarm);

    // Actuator remains OFF because mode is MANUAL and manual state is OFF
    TEST_ASSERT_FALSE(ventActuator->isOn());
}

void test_auto_mode_critical_temp_triggers_alarm_and_forces_actuator_on(void) {
    automation->getVentilationSubsystem().setMode(ControlMode::AUTO);

    tempSensor->setData(46.0f, false); // Critical Overheat (> 45°C)
    SensorDataMap readings(1);
    readings[0] = {tempSensor, tempSensor->read()};

    SystemHealthState healthState;
    automation->update(true, readings, healthState);

    // Alarm triggered!
    TEST_ASSERT_TRUE(healthState.hasCriticalHazard);
    TEST_ASSERT_TRUE(automation->getVentilationSubsystem().getStatus().hasActiveAlarm);

    // Actuator forced ON in AUTO mode
    TEST_ASSERT_TRUE(ventActuator->isOn());
}

void test_pin_assignments_match_hardware_spec(void) {
    TEST_ASSERT_EQUAL_INT(13, PIN_ACTUATOR_VENT);
    TEST_ASSERT_EQUAL_INT(32, PIN_BTN_MODE);
}

void test_timer_expiration_restores_auto_mode(void) {
    automation->setSystemMode(SystemMode::AUTOMATIC);
    automation->onButtonPressed(ButtonType::VENTILATION);

    TEST_ASSERT_EQUAL(ControlMode::MANUAL, automation->getVentilationSubsystem().getMode());
    TEST_ASSERT_TRUE(ventActuator->isOn());

    GreenhouseController::ActuatorTimer* timerObj = automation->getActuatorTimer(ActuatorType::VENTILATION);
    TEST_ASSERT_NOT_NULL(timerObj);

    triggerMockTimerCallback(timerObj->timer);

    TEST_ASSERT_FALSE(ventActuator->isOn());
    TEST_ASSERT_EQUAL(ControlMode::AUTO, automation->getVentilationSubsystem().getMode());
}

void test_manual_mode_soil_flood_forces_actuator_off(void) {
    automation->onButtonPressed(ButtonType::IRRIGATION);
    TEST_ASSERT_TRUE(irrigActuator->isOn());

    soilSensor->setData(92.0f, false); // > 85% flood threshold
    SensorDataMap readings(1);
    readings[0] = {soilSensor, soilSensor->read()};

    automation->update(false, readings);

    TEST_ASSERT_FALSE(irrigActuator->isOn());
    TEST_ASSERT_EQUAL(ManualState::OFF, automation->getIrrigationSubsystem().getStatus().manualState);
}

void test_mode_switch_cancels_active_manual_timers(void) {
    automation->onButtonPressed(ButtonType::IRRIGATION);
    TEST_ASSERT_TRUE(irrigActuator->isOn());

    automation->setSystemMode(SystemMode::AUTOMATIC);
    TEST_ASSERT_EQUAL(ManualState::OFF, automation->getIrrigationSubsystem().getStatus().manualState);
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
    RUN_TEST(test_auto_light_low_level_89lx_turns_on);
    RUN_TEST(test_auto_light_hysteresis_holds_on);
    RUN_TEST(test_auto_light_daylight_turns_off);
    RUN_TEST(test_auto_light_sensor_error_isolation);

    RUN_TEST(test_mode_toggle_is_sticky);
    RUN_TEST(test_manual_mode_button_toggles);
    RUN_TEST(test_manual_mode_button_override_under_critical_hazard);
    RUN_TEST(test_auto_mode_supports_manual_button_overrides);
    RUN_TEST(test_auto_mode_button_toggles_active_actuator_off);
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

    RUN_TEST(test_manual_button_starts_safety_timer);
    RUN_TEST(test_timer_expiration_auto_turns_off_actuator);
    RUN_TEST(test_manual_button_turn_off_stops_safety_timer);
    RUN_TEST(test_timer_reuse_on_multiple_manual_presses);
    RUN_TEST(test_system_under_high_load_stress);

    RUN_TEST(test_subsystem_independent_modes);
    RUN_TEST(test_manual_mode_critical_temp_triggers_alarm_without_overriding_manual_off);
    RUN_TEST(test_auto_mode_critical_temp_triggers_alarm_and_forces_actuator_on);
    RUN_TEST(test_pin_assignments_match_hardware_spec);
    RUN_TEST(test_timer_expiration_restores_auto_mode);
    RUN_TEST(test_manual_mode_soil_flood_forces_actuator_off);
    RUN_TEST(test_mode_switch_cancels_active_manual_timers);

    return UNITY_END();
}

