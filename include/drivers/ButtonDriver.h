#ifndef BUTTON_DRIVER_H
#define BUTTON_DRIVER_H

#include <Arduino.h>
#include "../config.h"

class ButtonDriver {
private:
    int pin;
    bool lastState;
    bool currentState;
    unsigned long lastDebounceTime;
    unsigned long debounceDelay;

public:
    ButtonDriver(int pin, unsigned long debounceDelay = DEBOUNCE_DELAY)
        : pin(pin), lastState(HIGH), currentState(HIGH), lastDebounceTime(0), debounceDelay(debounceDelay) {
        pinMode(pin, INPUT_PULLUP);
    }

    void init() {
        pinMode(pin, INPUT_PULLUP);
        currentState = digitalRead(pin);
        lastState = currentState;
    }

    bool isPressed() {
        return digitalRead(pin) == LOW;
    }

    // Returns true once per button press event (debounced)
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

    int getPin() const { return pin; }
};

#endif // BUTTON_DRIVER_H