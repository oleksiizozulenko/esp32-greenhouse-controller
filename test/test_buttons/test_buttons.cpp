#include <unity.h>
#include "../Arduino.h"
#include "ButtonDriver.h"

// Definition of global buttonEventQueue stub for unit test environment
QueueHandle_t buttonEventQueue = nullptr;

void setUp(void) {
    resetMockArduinoState();
}

void tearDown(void) {}

void test_button_driver_auto_id_generation(void) {
    ButtonDriver b1(12, ButtonType::MODE);
    ButtonDriver b2(14, ButtonType::IRRIGATION);
    ButtonDriver b3(27, ButtonType::VENTILATION);

    TEST_ASSERT_EQUAL_UINT8(b1.getId() + 1, b2.getId());
    TEST_ASSERT_EQUAL_UINT8(b2.getId() + 1, b3.getId());
}

void test_button_event_struct(void) {
    ButtonEvent evt(ButtonType::IRRIGATION, 5, 123456);
    TEST_ASSERT_EQUAL_INT((int)ButtonType::IRRIGATION, (int)evt.type);
    TEST_ASSERT_EQUAL_UINT8(5, evt.buttonId);
    TEST_ASSERT_EQUAL_UINT32(123456, evt.timestamp);
}

void test_button_driver_getters(void) {
    ButtonDriver btn(26, ButtonType::LIGHT);
    TEST_ASSERT_EQUAL_INT(26, btn.getPin());
    TEST_ASSERT_EQUAL_INT((int)ButtonType::LIGHT, (int)btn.getType());
}

void test_button_driver_debouncing(void) {
    ButtonDriver btn(14, ButtonType::IRRIGATION, 50);
    btn.init();

    // Default state: HIGH (not pressed)
    TEST_ASSERT_FALSE(btn.isPressed());
    TEST_ASSERT_FALSE(btn.wasPressed());

    // Press button (LOW) at t = 0
    setMockPinValue(14, LOW);
    TEST_ASSERT_TRUE(btn.isPressed());

    // Within debounce window (t = 10ms < 50ms), should not register press event yet
    advanceSimulatedMillis(10);
    TEST_ASSERT_FALSE(btn.wasPressed());

    // After debounce window (t = 61ms > 50ms), should register press event once
    advanceSimulatedMillis(51);
    TEST_ASSERT_TRUE(btn.wasPressed());

    // Consecutive check without state change should return false
    TEST_ASSERT_FALSE(btn.wasPressed());

    // Release button (HIGH)
    setMockPinValue(14, HIGH);
    advanceSimulatedMillis(60);
    TEST_ASSERT_FALSE(btn.wasPressed());
}

void test_button_driver_attach_interrupt(void) {
    QueueHandle_t mockQueue = (QueueHandle_t)0x5555;
    ButtonDriver btn(14, ButtonType::VENTILATION, 50);
    btn.attachInterruptHandler(mockQueue);

    TEST_ASSERT_EQUAL_INT(14, btn.getPin());
    TEST_ASSERT_EQUAL_INT((int)ButtonType::VENTILATION, (int)btn.getType());
}

void test_button_driver_isr_debouncing(void) {
    QueueHandle_t mockQueue = (QueueHandle_t)0x5555;
    ButtonDriver btn(14, ButtonType::VENTILATION, 50);
    btn.attachInterruptHandler(mockQueue);

    getMockArduinoState().isrQueueSendCount = 0;
    setMockPinValue(14, HIGH);

    // 1. Initial Press (Falling edge to LOW) at t = 100ms
    setSimulatedMillis(100);
    setMockPinValue(14, LOW);
    ButtonDriver::isrHandler(&btn);
    TEST_ASSERT_EQUAL_INT(1, getMockArduinoState().isrQueueSendCount);

    // 2. Press contact bounce (Rising and Falling edges within 50ms window)
    setSimulatedMillis(102);
    setMockPinValue(14, HIGH);
    ButtonDriver::isrHandler(&btn);

    setSimulatedMillis(105);
    setMockPinValue(14, LOW);
    ButtonDriver::isrHandler(&btn);

    TEST_ASSERT_EQUAL_INT(1, getMockArduinoState().isrQueueSendCount);

    // 3. Release button at t = 500ms (Pin rises to HIGH)
    setSimulatedMillis(500);
    setMockPinValue(14, HIGH);
    ButtonDriver::isrHandler(&btn);
    TEST_ASSERT_EQUAL_INT(1, getMockArduinoState().isrQueueSendCount);

    // 4. Release contact bounce at t = 505ms (Pin drops LOW briefly)
    setSimulatedMillis(505);
    setMockPinValue(14, LOW);
    ButtonDriver::isrHandler(&btn);

    setSimulatedMillis(508);
    setMockPinValue(14, HIGH);
    ButtonDriver::isrHandler(&btn);

    TEST_ASSERT_EQUAL_INT(1, getMockArduinoState().isrQueueSendCount);

    // 5. Subsequent Press at t = 1000ms (Pin goes LOW >50ms after release)
    setSimulatedMillis(1000);
    setMockPinValue(14, LOW);
    ButtonDriver::isrHandler(&btn);
    TEST_ASSERT_EQUAL_INT(2, getMockArduinoState().isrQueueSendCount);
}

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;
    UNITY_BEGIN();

    RUN_TEST(test_button_driver_auto_id_generation);
    RUN_TEST(test_button_event_struct);
    RUN_TEST(test_button_driver_getters);
    RUN_TEST(test_button_driver_debouncing);
    RUN_TEST(test_button_driver_attach_interrupt);
    RUN_TEST(test_button_driver_isr_debouncing);

    return UNITY_END();
}
