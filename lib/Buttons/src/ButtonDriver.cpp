#include "ButtonDriver.h"

uint8_t ButtonDriver::nextId = 1;

void IRAM_ATTR ButtonDriver::isrHandler(void* arg) {
    ButtonDriver* driver = static_cast<ButtonDriver*>(arg);
    if (driver != nullptr) {
        unsigned long now = millis();
        if (now - driver->lastDebounceTime > BUTTON_DEBOUNCE_DELAY_MS) {
            driver->lastDebounceTime = now;
            
            if (driver->targetQueue != nullptr) {
                ButtonEvent evt(driver->type, driver->id, now);
                BaseType_t xHigherPriorityTaskWoken = pdFALSE;
                xQueueSendFromISR(driver->targetQueue, &evt, &xHigherPriorityTaskWoken);
                if (xHigherPriorityTaskWoken == pdTRUE) {
                    portYIELD_FROM_ISR();
                }
            }

            if (driver->listener != nullptr) {
                driver->listener->onButtonPressed(driver->type);
            }
        }
    }
}

ButtonDriver::ButtonDriver(int pin, ButtonType type)
    : id(nextId++), pin(pin), type(type), lastDebounceTime(0), lastState(HIGH),
      targetQueue(nullptr), listener(nullptr) {}

ButtonDriver::~ButtonDriver() {}

void ButtonDriver::init() {
    pinMode(pin, INPUT_PULLUP);
}

void ButtonDriver::setListener(IButtonListener* newListener) {
    listener = newListener;
}

void ButtonDriver::attachInterruptHandler(QueueHandle_t queue) {
    init();
    targetQueue = queue;
    attachInterruptArg(digitalPinToInterrupt(pin), isrHandler, this, FALLING);
}

bool ButtonDriver::isPressed() {
    return digitalRead(pin) == LOW;
}
