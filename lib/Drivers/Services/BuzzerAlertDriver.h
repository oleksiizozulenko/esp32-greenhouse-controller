#ifndef BUZZER_ALERT_DRIVER_H
#define BUZZER_ALERT_DRIVER_H

#include <Arduino.h>
#include "Services/IAlertService.h"

class BuzzerAlertDriver : public IAlertService {
private:
    int pin;
    uint16_t activeAlertCode;
    bool hasActive;

public:
    explicit BuzzerAlertDriver(int buzzerPin);
    ~BuzzerAlertDriver() override = default;

    bool begin() override;
    void raiseAlert(const AlertEvent& alert) override;
    void clearAlert(uint16_t code) override;
    void clearAll() override;
    bool isAlertActive(uint16_t code) const override;
};

#endif // BUZZER_ALERT_DRIVER_H
