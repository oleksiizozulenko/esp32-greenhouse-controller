#include "SystemAlertService.h"

SystemAlertService::SystemAlertService(int redLed, int greenLed, int buzzer)
    : redLedPin(redLed), greenLedPin(greenLed), buzzerPin(buzzer),
      buzzerActive(false), alarmActive(false), activeAlertCode(0) {}

bool SystemAlertService::begin() {
    pinMode(redLedPin, OUTPUT);
    pinMode(greenLedPin, OUTPUT);
    pinMode(buzzerPin, OUTPUT);

    digitalWrite(redLedPin, LOW);
    digitalWrite(greenLedPin, HIGH);
    digitalWrite(buzzerPin, LOW);
    buzzerActive = false;
    alarmActive = false;
    activeAlertCode = 0;
    return true;
}

void SystemAlertService::update(const SystemHealthState& healthState) {
    digitalWrite(greenLedPin, HIGH); // System powered on & working (ALWAYS ON)

    if (healthState.hasHardwareError || healthState.hasCriticalHazard || alarmActive) {
        digitalWrite(redLedPin, HIGH);  // Red LED on for active alert/hazard
    } else {
        digitalWrite(redLedPin, LOW);
    }

    if (healthState.requiresAlarm || alarmActive) {
        tone(buzzerPin, 1000, 100);
        buzzerActive = true;
    } else if (buzzerActive) {
        noTone(buzzerPin);
        digitalWrite(buzzerPin, LOW);
        buzzerActive = false;
    } else {
        digitalWrite(buzzerPin, LOW);
    }
}

void SystemAlertService::raiseAlert(const AlertEvent& alert) {
    activeAlertCode = alert.code;
    alarmActive = true;
    digitalWrite(redLedPin, HIGH);
    digitalWrite(greenLedPin, LOW);
    tone(buzzerPin, 1000, 100);
    buzzerActive = true;
}

void SystemAlertService::clearAlert(uint16_t code) {
    if (activeAlertCode == code) {
        clearAll();
    }
}

void SystemAlertService::clearAll() {
    activeAlertCode = 0;
    alarmActive = false;
    digitalWrite(redLedPin, LOW);
    digitalWrite(greenLedPin, HIGH);
    if (buzzerActive) {
        noTone(buzzerPin);
        digitalWrite(buzzerPin, LOW);
        buzzerActive = false;
    }
}

bool SystemAlertService::isAlertActive(uint16_t code) const {
    return alarmActive && (activeAlertCode == code);
}
