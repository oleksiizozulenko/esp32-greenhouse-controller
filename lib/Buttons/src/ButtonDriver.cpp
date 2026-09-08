#include "ButtonDriver.h"
#include "config.h"

#ifndef UNIT_TEST
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include "rom/ets_sys.h"
extern EventGroupHandle_t systemEventGroup;
#define EVENT_BIT_BUTTON_EVENT (1 << 1)
#endif

uint8_t ButtonDriver::nextId = 1;

void IRAM_ATTR ButtonDriver::isrHandler(void* arg) {
    ButtonDriver* driver = static_cast<ButtonDriver*>(arg);
    if (driver != nullptr) {
        unsigned long now = millis();
        int currentState = digitalRead(driver->pin);

        if (currentState == HIGH && driver->lastState == LOW && (now - driver->lastDebounceTime >= driver->debounceDelay)) {
            driver->lastState = HIGH;
            driver->lastDebounceTime = now;
            return;
        }

        if (currentState == LOW) {
            if (driver->lastState == HIGH || (now - driver->lastDebounceTime >= driver->debounceDelay)) {
                if (now - driver->lastDebounceTime >= driver->debounceDelay) {
                    driver->lastState = LOW;
                    driver->lastDebounceTime = now;

#ifndef UNIT_TEST
                    ets_printf("[ISR HARDWARE] Interrupt triggered on GPIO %d (Button Type %d)!\n", driver->pin, (int)driver->type);
#endif

                    if (driver->targetQueue != nullptr) {
                        ButtonEvent evt(driver->type, driver->id, now);
                        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
                        xQueueSendFromISR(driver->targetQueue, &evt, &xHigherPriorityTaskWoken);
                        
#ifndef UNIT_TEST
                        if (systemEventGroup != NULL) {
                            xEventGroupSetBitsFromISR(systemEventGroup, EVENT_BIT_BUTTON_EVENT, &xHigherPriorityTaskWoken);
                        }
#endif

                        if (xHigherPriorityTaskWoken == pdTRUE) {
                            portYIELD_FROM_ISR();
                        }
                    }
                }
            }
        }
    }
}

ButtonDriver::ButtonDriver(int pin, ButtonType type, unsigned long debounceDelay)
    : id(nextId++), pin(pin), type(type), debounceDelay(debounceDelay), lastDebounceTime(0), lastState(HIGH),
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
    lastState = digitalRead(pin);
    lastDebounceTime = millis();
    attachInterruptArg(digitalPinToInterrupt(pin), isrHandler, this, CHANGE);
}

bool ButtonDriver::isPressed() {
    return digitalRead(pin) == LOW;
}

bool ButtonDriver::wasPressed() {
    int currentState = digitalRead(pin);
    bool pressed = false;
    if (currentState == LOW) {
        if (lastState == HIGH) {
            unsigned long now = millis();
            if (now - lastDebounceTime >= debounceDelay) {
                lastDebounceTime = now;
                pressed = true;
                lastState = LOW;
            }
        }
    } else {
        lastState = HIGH;
    }
    return pressed;
}
