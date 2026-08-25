#include "QC1602ACharacterLcdDriver.h"


QC1602ACharacterLcdDriver::QC1602ACharacterLcdDriver(int rs, int en, int d4, int d5, int d6, int d7)
    : rsPin(rs), enPin(en), d4Pin(d4), d5Pin(d5), d6Pin(d6), d7Pin(d7) {
    line0Text[0] = '\0';
    line1Text[0] = '\0';
}

bool QC1602ACharacterLcdDriver::begin() {
    pinMode(rsPin, OUTPUT);
    pinMode(enPin, OUTPUT);
    pinMode(d4Pin, OUTPUT);
    pinMode(d5Pin, OUTPUT);
    pinMode(d6Pin, OUTPUT);
    pinMode(d7Pin, OUTPUT);
    return true;
}

void QC1602ACharacterLcdDriver::clear() {
    line0Text[0] = '\0';
    line1Text[0] = '\0';
}

void QC1602ACharacterLcdDriver::setHeader(const char* title) {
    if (title) {
        strncpy(line0Text, title, 16);
        line0Text[16] = '\0';
    }
}

void QC1602ACharacterLcdDriver::setField(uint8_t slotIndex, const DisplayField& field) {
    char buf[17];
    snprintf(buf, sizeof(buf), "%s:%.1f%s", field.label, field.value, field.unit);
    if (slotIndex == 0) {
        strncpy(line0Text, buf, 16);
        line0Text[16] = '\0';
    } else {
        strncpy(line1Text, buf, 16);
        line1Text[16] = '\0';
    }
}

void QC1602ACharacterLcdDriver::setStatusLine(const char* statusText) {
    if (statusText) {
        strncpy(line1Text, statusText, 16);
        line1Text[16] = '\0';
    }
}

void QC1602ACharacterLcdDriver::refresh() {
    // 16x2 Character LCD refresh logic
}
