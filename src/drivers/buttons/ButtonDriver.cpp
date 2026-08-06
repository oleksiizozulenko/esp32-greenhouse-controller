#include "drivers/buttons/ButtonDriver.h"

void IRAM_ATTR ButtonDriver::isrHandler(void* arg) {
    ButtonDriver* btn = static_cast<ButtonDriver*>(arg);
    if (btn == nullptr) return;

    unsigned long now = millis();
    if (now - btn->lastDebounceTime > btn->debounceDelay) {
        btn->lastDebounceTime = now;

        ButtonEvent evt(btn->buttonType, btn->id, now);
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;

        if (btn->eventQueue != NULL) {
            xQueueSendFromISR(btn->eventQueue, &evt, &xHigherPriorityTaskWoken);
        }

        if (xHigherPriorityTaskWoken) {
            portYIELD_FROM_ISR();
        }
    }
}
