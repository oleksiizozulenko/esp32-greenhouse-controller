#ifndef GREENHOUSE_CONTROLLER_H
#define GREENHOUSE_CONTROLLER_H

#include <Arduino.h>
#ifndef UNIT_TEST
#include <freertos/FreeRTOS.h>
#include <freertos/timers.h>
#endif
#include "config.h"
#include "IActuator.h"
#include "ButtonDriver.h"
#include "DisplayViewModel.h"
#include "SensorsService.h"
#include "SafetyMonitorService.h"

class GreenhouseController : public IButtonListener {
public:
    static constexpr size_t MAX_ACTUATORS = 16;

    struct TimerContext {
        GreenhouseController* controller;
        ActuatorType actuatorType;
    };

    struct ActuatorTimer {
        ActuatorType type;
        TimerHandle_t timer;
        uint32_t timeoutMs;
        TimerContext context;
        bool active;
    };

private:
    IActuator* actuators[MAX_ACTUATORS];
    size_t actuatorCount;

    ActuatorTimer timers[MAX_ACTUATORS];
    size_t timerCount;

    static void vActuatorTimerCallback(TimerHandle_t xTimer);

public:
    explicit GreenhouseController(size_t initialCapacity = 4);
    ~GreenhouseController();

    GreenhouseController(const GreenhouseController&) = delete;
    GreenhouseController& operator=(const GreenhouseController&) = delete;

    bool addActuator(IActuator* actuator);
    size_t getActuatorCount() const;
    IActuator* getActuator(size_t index) const;
    IActuator* getActuator(ActuatorType type) const;

    ActuatorTimer* getActuatorTimer(ActuatorType type);
    const ActuatorTimer* getActuatorTimer(ActuatorType type) const;

    void startTimerFor(ActuatorType type, uint32_t timeoutMs);
    void stopTimerFor(ActuatorType type);

    void onButtonPressed(ButtonType button) override;
    uint32_t getActuatorTimeout(ActuatorType type) const;

    void begin();

    void processAutomatic(const SensorDataMap& readings);
    void processManual(const SensorDataMap& readings, const SystemHealthState& healthState);

    void update(bool isAutoMode, const SensorDataMap& readings, const SystemHealthState& healthState);
    void update(bool isAutoMode, const SensorDataMap& readings);

    DisplayViewModel buildDisplayViewModel(bool isAutoMode, const SensorDataMap& readings, const SystemHealthState& healthState) const;
};

#endif // GREENHOUSE_CONTROLLER_H
