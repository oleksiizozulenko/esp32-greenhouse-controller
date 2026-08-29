#ifndef BUTTON_DRIVER_H
#define BUTTON_DRIVER_H

#include <Arduino.h>
#include "config.h"
#include "ButtonType.h"
#include "ButtonEvent.h"
#include "IButtonListener.h"

class ButtonDriver {
private:
    uint8_t id;
    int pin;
    ButtonType type;
    unsigned long debounceDelay;
    volatile unsigned long lastDebounceTime;
    volatile int lastState;
    QueueHandle_t targetQueue;
    IButtonListener* listener;

    static uint8_t nextId;
public:
    static void IRAM_ATTR isrHandler(void* arg);
    ButtonDriver(int pin, ButtonType type = ButtonType::UNKNOWN, unsigned long debounceDelay = BUTTON_DEBOUNCE_DELAY_MS);
    ~ButtonDriver();

    void init();
    void setListener(IButtonListener* newListener);
    void attachInterruptHandler(QueueHandle_t queue);
    bool isPressed();
    bool wasPressed();

    uint8_t getId() const { return id; }
    int getPin() const { return pin; }
    ButtonType getType() const { return type; }
    unsigned long getDebounceDelay() const { return debounceDelay; }
};

#endif // BUTTON_DRIVER_H
