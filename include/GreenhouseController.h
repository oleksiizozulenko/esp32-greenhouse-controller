#ifndef GREENHOUSE_CONTROLLER_H
#define GREENHOUSE_CONTROLLER_H

#include <Arduino.h>
#ifndef UNIT_TEST
#include <freertos/FreeRTOS.h>
#include <freertos/timers.h>
#endif
#include "config.h"
#include "drivers/Actuator.h"
#include "drivers/buttons/ButtonDriver.h"
#include "ui/DisplayViewModel.h"
#include "services/SensorsService.h"
#include "services/SafetyMonitorService.h"

class GreenhouseController : public IButtonListener {
public:
    struct TimerContext {
        GreenhouseController* controller;
        ActuatorType actuatorType;
    };

    struct ActuatorTimer {
        ActuatorType type;
        TimerHandle_t timer;
        uint32_t timeoutMs;
        TimerContext* context;
    };

private:
    Actuator** actuators;
    size_t capacity;
    size_t actuatorCount;

    ActuatorTimer** timers;
    size_t timerCapacity;
    size_t timerCount;

    int redLedPin;
    int greenLedPin;
    int buzzerPin;

    static void vActuatorTimerCallback(TimerHandle_t xTimer);
    bool addActuatorTimer(ActuatorTimer* timerObj);

public:
    GreenhouseController(size_t initialCapacity = 4,
                         int redLed = PIN_LED_RED,
                         int greenLed = PIN_LED_GREEN,
                         int buzzer = PIN_BUZZER);
    ~GreenhouseController();

    GreenhouseController(const GreenhouseController&) = delete;
    GreenhouseController& operator=(const GreenhouseController&) = delete;

    bool addActuator(Actuator* actuator);
    size_t getActuatorCount() const;
    Actuator* getActuator(size_t index) const;
    Actuator* getActuator(ActuatorType type) const;
    ActuatorTimer* getActuatorTimer(ActuatorType type) const;

    void startTimerFor(ActuatorType type, uint32_t timeoutMs);
    void stopTimerFor(ActuatorType type);

    void onButtonPressed(ButtonType button) override;
    uint32_t getActuatorTimeout(ActuatorType type) const;

    void begin();
    void updateSystemIndicators(const SystemHealthState& healthState);

    void processAutomatic(const SensorDataMap& readings);
    void processManual(const SensorDataMap& readings, const SystemHealthState& healthState);

    void update(bool isAutoMode, const SensorDataMap& readings, const SystemHealthState& healthState);
    void update(bool isAutoMode, const SensorDataMap& readings);

    DisplayViewModel buildDisplayViewModel(bool isAutoMode, const SensorDataMap& readings, const SystemHealthState& healthState) const;
};

#endif // GREENHOUSE_CONTROLLER_H
