#ifndef BUTTON_EVENT_H
#define BUTTON_EVENT_H

#include <Arduino.h>
#include "ButtonType.h"

struct ButtonEvent {
    ButtonType type;
    uint8_t buttonId;
    unsigned long timestamp;

    ButtonEvent() : type(ButtonType::MODE), buttonId(0), timestamp(0) {}
    ButtonEvent(ButtonType type, uint8_t buttonId = 0, unsigned long timestamp = millis())
        : type(type), buttonId(buttonId), timestamp(timestamp) {}
};

#endif // BUTTON_EVENT_H
