#ifndef BUTTON_EVENT_H
#define BUTTON_EVENT_H

#include <Arduino.h>
#include "ButtonType.h"

struct ButtonEvent {
    ButtonType type;
    unsigned long timestamp;

    ButtonEvent() : type(ButtonType::MODE), timestamp(0) {}
    ButtonEvent(ButtonType type, unsigned long timestamp = millis())
        : type(type), timestamp(timestamp) {}
};

#endif // BUTTON_EVENT_H
