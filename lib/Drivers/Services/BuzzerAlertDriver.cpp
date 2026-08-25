#include "BuzzerAlertDriver.h"

BuzzerAlertDriver::BuzzerAlertDriver(int buzzerPin)
    : pin(buzzerPin), activeAlertCode(0), hasActive(false) {}

bool BuzzerAlertDriver::begin() {
    pinMode(pin, OUTPUT);
    digitalWrite(pin, LOW);
    return true;
}

void BuzzerAlertDriver::raiseAlert(const AlertEvent& alert) {
    activeAlertCode = alert.code;
    hasActive = true;

    if (alert.severity == AlertSeverity::CRITICAL) {
        tone(pin, 2000, 500); // 2kHz tone for 500ms
    } else if (alert.severity == AlertSeverity::WARNING) {
        tone(pin, 1000, 200); // 1kHz tone for 200ms
    }
}

void BuzzerAlertDriver::clearAlert(uint16_t code) {
    if (activeAlertCode == code) {
        clearAll();
    }
}

void BuzzerAlertDriver::clearAll() {
    noTone(pin);
    digitalWrite(pin, LOW);
    activeAlertCode = 0;
    hasActive = false;
}

bool BuzzerAlertDriver::isAlertActive(uint16_t code) const {
    return hasActive && (activeAlertCode == code);
}
