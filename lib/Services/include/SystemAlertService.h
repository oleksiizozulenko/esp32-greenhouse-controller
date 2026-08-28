#ifndef SYSTEM_ALERT_SERVICE_H
#define SYSTEM_ALERT_SERVICE_H

#include <Arduino.h>
#include "config.h"
#include "IAlertService.h"
#include "SafetyMonitorService.h"

class SystemAlertService : public IAlertService {
private:
    int redLedPin;
    int greenLedPin;
    int buzzerPin;
    bool buzzerActive;
    bool alarmActive;
    uint16_t activeAlertCode;

public:
    SystemAlertService(int redLed = PIN_LED_RED,
                       int greenLed = PIN_LED_GREEN,
                       int buzzer = PIN_BUZZER);
    ~SystemAlertService() override = default;

    bool begin() override;
    void update(const SystemHealthState& healthState);

    void raiseAlert(const AlertEvent& alert) override;
    void clearAlert(uint16_t code) override;
    void clearAll() override;
    bool isAlertActive(uint16_t code) const override;
};

#endif // SYSTEM_ALERT_SERVICE_H
