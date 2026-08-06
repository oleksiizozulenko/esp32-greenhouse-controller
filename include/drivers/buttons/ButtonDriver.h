#ifndef BUTTON_DRIVER_H
#define BUTTON_DRIVER_H

#include <Arduino.h>
#include "../../config.h"
#include "ButtonType.h"
#include "ButtonEvent.h"
#include "IButtonListener.h"

class ButtonDriver {
private:
    int pin;
    ButtonType buttonType;
    bool lastState;
    bool currentState;
    unsigned long lastDebounceTime;
    unsigned long debounceDelay;

public:
    ButtonDriver(int pin, ButtonType buttonType = ButtonType::MODE, unsigned long debounceDelay = DEBOUNCE_DELAY)
        : pin(pin), buttonType(buttonType), lastState(HIGH), currentState(HIGH),
          lastDebounceTime(0), debounceDelay(debounceDelay) {}

    ButtonDriver(int pin, unsigned long debounceDelay)
        : ButtonDriver(pin, ButtonType::MODE, debounceDelay) {}

    void init() {
        pinMode(pin, INPUT_PULLUP);
        currentState = digitalRead(pin);
        lastState = currentState;
    }

    bool isPressed() const {
        return digitalRead(pin) == LOW;
    }

    // Returns true once per debounced button press event
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
    ButtonType getType() const { return buttonType; }
};

#endif // BUTTON_DRIVER_H
