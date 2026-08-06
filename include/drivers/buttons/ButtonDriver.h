#ifndef BUTTON_DRIVER_H
#define BUTTON_DRIVER_H

#include <Arduino.h>
#include "../../config.h"
#include "ButtonType.h"
#include "ButtonEvent.h"
#include "IButtonListener.h"

class ButtonDriver {
private:
    static uint8_t generateNextId() {
        static uint8_t counter = 0;
        return ++counter;
    }

    uint8_t id;
    int pin;
    ButtonType buttonType;
    bool lastState;
    bool currentState;
    volatile unsigned long lastDebounceTime;
    unsigned long debounceDelay;
    QueueHandle_t eventQueue;

    static void IRAM_ATTR isrHandler(void* arg);

public:
    ButtonDriver(int pin, ButtonType buttonType = ButtonType::MODE, unsigned long debounceDelay = DEBOUNCE_DELAY)
        : id(generateNextId()), pin(pin), buttonType(buttonType), lastState(HIGH), currentState(HIGH),
          lastDebounceTime(0), debounceDelay(debounceDelay), eventQueue(NULL) {}

    ButtonDriver(int pin, unsigned long debounceDelay)
        : ButtonDriver(pin, ButtonType::MODE, debounceDelay) {}

    void init() {
        pinMode(pin, INPUT_PULLUP);
        currentState = digitalRead(pin);
        lastState = currentState;
    }

    void attachInterruptHandler(QueueHandle_t queue) {
        this->eventQueue = queue;
        init();
        attachInterruptArg(digitalPinToInterrupt(pin), isrHandler, this, FALLING);
    }

    bool isPressed() const {
        return digitalRead(pin) == LOW;
    }

    // Returns true once per debounced button press event (polling fallback)
    bool wasPressed() {
        bool reading = digitalRead(pin);
        bool pressedEvent = false;

        if (reading != lastState) {
            lastDebounceTime = millis();
        }

        if ((millis() - lastDebounceTime) > debounceDelay) {
            if (reading != currentState) {
                currentState = reading;
                if (currentState == LOW) {
                    pressedEvent = true;
                }
            }
        }

        lastState = reading;
        return pressedEvent;
    }

    void toggle() {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, LOW);
        delay(100);
        pinMode(pin, INPUT_PULLUP);
    }

    uint8_t getId() const { return id; }
    int getPin() const { return pin; }
    ButtonType getType() const { return buttonType; }
};

#endif // BUTTON_DRIVER_H
