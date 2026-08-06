#ifndef BUTTON_DRIVER_H
#define BUTTON_DRIVER_H

#include <Arduino.h>
#include "../config.h"

enum class ButtonType {
    MODE = 0,
    IRRIGATION,
    VENTILATION,
    LIGHT
};

class IButtonListener {
public:
    virtual ~IButtonListener() = default;
    virtual void onButtonPressed(ButtonType button) = 0;
};

class ButtonDriver {
private:
    int pin;
    ButtonType buttonType;
    IButtonListener* listener;
    bool lastState;
    bool currentState;
    unsigned long lastDebounceTime;
    unsigned long debounceDelay;

public:
    ButtonDriver(int pin, ButtonType buttonType = ButtonType::MODE, unsigned long debounceDelay = DEBOUNCE_DELAY)
        : pin(pin), buttonType(buttonType), listener(nullptr), lastState(HIGH), currentState(HIGH),
          lastDebounceTime(0), debounceDelay(debounceDelay) {
        pinMode(pin, INPUT_PULLUP);
    }

    ButtonDriver(int pin, unsigned long debounceDelay)
        : ButtonDriver(pin, ButtonType::MODE, debounceDelay) {}

    void setListener(IButtonListener* newListener) {
        listener = newListener;
    }

    void init() {
        pinMode(pin, INPUT_PULLUP);
        currentState = digitalRead(pin);
        lastState = currentState;
    }

    bool isPressed() {
        return digitalRead(pin) == LOW;
    }

    // Returns true once per button press event (debounced) and notifies listener if set
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
                    if (listener != nullptr) {
                        listener->onButtonPressed(buttonType);
                    }
                }
            }
        }

        lastState = reading;
        return pressedEvent;
    }

    void checkEvent() {
        wasPressed();
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