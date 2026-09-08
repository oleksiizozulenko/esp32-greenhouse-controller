#include "GreenhouseController.h"

void GreenhouseController::vActuatorTimerCallback(TimerHandle_t xTimer) {
    TimerContext* ctx = (TimerContext*)pvTimerGetTimerID(xTimer);
    if (ctx != nullptr && ctx->controller != nullptr) {
        IControlSubsystem* sub = ctx->controller->getSubsystem(
            ctx->actuatorType == ActuatorType::VENTILATION ? SubsystemType::VENTILATION :
            ctx->actuatorType == ActuatorType::LIGHT ? SubsystemType::LIGHTING :
            ctx->actuatorType == ActuatorType::IRRIGATION ? SubsystemType::IRRIGATION : SubsystemType::UNKNOWN
        );
        IActuator* act = ctx->controller->getActuator(ctx->actuatorType);
        if (sub != nullptr && sub->getMode() == ControlMode::MANUAL) {
            Serial.printf("[SAFETY TIMER] %s Timer Expired -> Turning Manual State OFF\n", sub->getName());
            sub->setManualState(ManualState::OFF);
            if (ctx->controller->getSystemMode() == SystemMode::AUTOMATIC) {
                sub->setMode(ControlMode::AUTO);
            }
            if (act != nullptr && act->isOperating()) {
                act->turnOff();
            }
        }
    }
}

GreenhouseController::GreenhouseController(size_t initialCapacity)
    : actuators{}, actuatorCount(0), globalSystemMode(SystemMode::AUTOMATIC), timers{}, timerCount(0) {
    (void)initialCapacity;
    for (size_t i = 0; i < MAX_ACTUATORS; ++i) {
        actuators[i] = nullptr;
        timers[i].active = false;
        timers[i].timer = NULL;
    }
}

GreenhouseController::~GreenhouseController() {
    for (size_t i = 0; i < timerCount; ++i) {
        if (timers[i].active && timers[i].timer != NULL) {
            xTimerStop(timers[i].timer, 0);
            xTimerDelete(timers[i].timer, 0);
            timers[i].timer = NULL;
            timers[i].active = false;
        }
    }
}

bool GreenhouseController::addActuator(IActuator* actuator) {
    if (actuator == nullptr || actuatorCount >= MAX_ACTUATORS) return false;
    actuators[actuatorCount++] = actuator;

    switch (actuator->getType()) {
        case ActuatorType::VENTILATION:
            ventilationSubsystem.setActuator(actuator);
            break;
        case ActuatorType::LIGHT:
            lightingSubsystem.setActuator(actuator);
            break;
        case ActuatorType::IRRIGATION:
            irrigationSubsystem.setActuator(actuator);
            break;
        default:
            break;
    }
    return true;
}

size_t GreenhouseController::getActuatorCount() const {
    return actuatorCount;
}

IActuator* GreenhouseController::getActuator(size_t index) const {
    if (index < actuatorCount) {
        return actuators[index];
    }
    return nullptr;
}

IActuator* GreenhouseController::getActuator(ActuatorType type) const {
    for (size_t i = 0; i < actuatorCount; ++i) {
        if (actuators[i] != nullptr && actuators[i]->getType() == type) {
            return actuators[i];
        }
    }
    return nullptr;
}

IControlSubsystem* GreenhouseController::getSubsystem(SubsystemType type) {
    switch (type) {
        case SubsystemType::VENTILATION: return &ventilationSubsystem;
        case SubsystemType::LIGHTING: return &lightingSubsystem;
        case SubsystemType::IRRIGATION: return &irrigationSubsystem;
        default: return nullptr;
    }
}

const IControlSubsystem* GreenhouseController::getSubsystem(SubsystemType type) const {
    switch (type) {
        case SubsystemType::VENTILATION: return &ventilationSubsystem;
        case SubsystemType::LIGHTING: return &lightingSubsystem;
        case SubsystemType::IRRIGATION: return &irrigationSubsystem;
        default: return nullptr;
    }
}

IControlSubsystem* GreenhouseController::getSubsystemForButton(ButtonType button) {
    switch (button) {
        case ButtonType::VENTILATION: return &ventilationSubsystem;
        case ButtonType::LIGHT: return &lightingSubsystem;
        case ButtonType::IRRIGATION: return &irrigationSubsystem;
        default: return nullptr;
    }
}

void GreenhouseController::setSystemMode(SystemMode mode) {
    globalSystemMode = mode;
    ControlMode targetMode = (mode == SystemMode::AUTOMATIC ? ControlMode::AUTO : ControlMode::MANUAL);
    ventilationSubsystem.setMode(targetMode);
    lightingSubsystem.setMode(targetMode);
    irrigationSubsystem.setMode(targetMode);
}

GreenhouseController::ActuatorTimer* GreenhouseController::getActuatorTimer(ActuatorType type) {
    for (size_t i = 0; i < timerCount; ++i) {
        if (timers[i].active && timers[i].type == type) {
            return &timers[i];
        }
    }
    return nullptr;
}

const GreenhouseController::ActuatorTimer* GreenhouseController::getActuatorTimer(ActuatorType type) const {
    for (size_t i = 0; i < timerCount; ++i) {
        if (timers[i].active && timers[i].type == type) {
            return &timers[i];
        }
    }
    return nullptr;
}

void GreenhouseController::startTimerFor(ActuatorType type, uint32_t timeoutMs) {
    ActuatorTimer* timerObj = getActuatorTimer(type);
    if (timerObj == nullptr) {
        IActuator* act = getActuator(type);
        if (act == nullptr || timerCount >= MAX_ACTUATORS) return;

        timerObj = &timers[timerCount++];
        timerObj->type = type;
        timerObj->timeoutMs = timeoutMs;
        timerObj->context = TimerContext{this, type};
        timerObj->timer = xTimerCreate(
            act->getName(),
            pdMS_TO_TICKS(timeoutMs),
            pdFALSE, // One-shot
            (void*)&timerObj->context,
            vActuatorTimerCallback
        );
        if (timerObj->timer != NULL) {
            timerObj->active = true;
        }
    }

    if (timerObj != nullptr && timerObj->timer != NULL) {
        xTimerReset(timerObj->timer, 0);
    }
}

void GreenhouseController::stopTimerFor(ActuatorType type) {
    ActuatorTimer* timerObj = getActuatorTimer(type);
    if (timerObj != nullptr && timerObj->timer != NULL) {
        xTimerStop(timerObj->timer, 0);
    }
}

void GreenhouseController::onButtonPressed(ButtonType button) {
    IControlSubsystem* sub = getSubsystemForButton(button);
    if (sub == nullptr) {
        Serial.printf("[MANUAL EVENT] Button %d Pressed -> No Subsystem registered!\n", (int)button);
        return;
    }

    ActuatorType targetType = (button == ButtonType::VENTILATION) ? ActuatorType::VENTILATION :
                              (button == ButtonType::LIGHT) ? ActuatorType::LIGHT :
                              (button == ButtonType::IRRIGATION) ? ActuatorType::IRRIGATION : ActuatorType::UNKNOWN;

    IActuator* act = getActuator(targetType);

    bool currentlyActive = (act != nullptr && act->isOperating()) || 
                           (sub->getMode() == ControlMode::MANUAL && sub->getManualState() == ManualState::ON);

    if (currentlyActive) {
        Serial.printf("[MANUAL EVENT] Button Pressed -> Toggling %s Subsystem to OFF\n", sub->getName());
        sub->setMode(ControlMode::MANUAL);
        sub->setManualState(ManualState::OFF);
        if (act != nullptr) {
            act->turnOff();
        }
        if (globalSystemMode == SystemMode::AUTOMATIC) {
            sub->setMode(ControlMode::AUTO);
        }
        stopTimerFor(targetType);
    } else {
        Serial.printf("[MANUAL EVENT] Button Pressed -> Toggling %s Subsystem to ON\n", sub->getName());
        sub->setMode(ControlMode::MANUAL);
        sub->setManualState(ManualState::ON);
        if (act != nullptr) {
            act->turnOn();
        }
        startTimerFor(targetType, getActuatorTimeout(targetType));
    }
}

uint32_t GreenhouseController::getActuatorTimeout(ActuatorType type) const {
    switch (type) {
        case ActuatorType::IRRIGATION:
            return IRRIGATION_TIMEOUT_MS;
        case ActuatorType::VENTILATION:
            return VENTILATION_TIMEOUT_MS;
        case ActuatorType::LIGHT:
            return LIGHT_TIMEOUT_MS;
        default:
            return 0;
    }
}

void GreenhouseController::begin() {
    for (size_t i = 0; i < actuatorCount; ++i) {
        if (actuators[i] != nullptr) {
            actuators[i]->begin();
        }
    }
}

void GreenhouseController::update(bool isAutoMode, const SensorDataMap& readings, SystemHealthState& healthState) {
    SystemMode targetMode = isAutoMode ? SystemMode::AUTOMATIC : SystemMode::MANUAL;
    if (targetMode != globalSystemMode) {
        setSystemMode(targetMode);
    } else if (!isAutoMode) {
        ventilationSubsystem.setMode(ControlMode::MANUAL);
        lightingSubsystem.setMode(ControlMode::MANUAL);
        irrigationSubsystem.setMode(ControlMode::MANUAL);
    }

    SensorData temp = readings.get(SensorType::TEMPERATURE);
    SensorData hum = readings.get(SensorType::HUMIDITY);
    SensorData light = readings.get(SensorType::LIGHT);
    SensorData soil = readings.get(SensorType::SOIL);

    ventilationSubsystem.update(temp, hum, healthState);
    lightingSubsystem.update(light, healthState);
    irrigationSubsystem.update(soil, healthState);
}

void GreenhouseController::update(bool isAutoMode, const SensorDataMap& readings) {
    SafetyMonitorService safetyMonitor;
    SystemHealthState healthState = safetyMonitor.evaluate(readings, isAutoMode);
    update(isAutoMode, readings, healthState);
}

DisplayViewModel GreenhouseController::buildDisplayViewModel(bool isAutoMode, const SensorDataMap& readings, const SystemHealthState& healthState) const {
    DisplayViewModel vm;
    memset(&vm, 0, sizeof(DisplayViewModel));

    snprintf(vm.modeText, sizeof(vm.modeText), "%s", isAutoMode ? "AUTO" : "MANUAL");
    snprintf(vm.healthStatus, sizeof(vm.healthStatus), "%s", healthState.hasHardwareError ? "[ERR]" : "[OK]");

    vm.sensorCount = readings.size() < 4 ? readings.size() : 4;
    for (size_t i = 0; i < vm.sensorCount; ++i) {
        Sensor* s = readings[i].sensor;
        SensorData d = readings[i].data;
        if (s != nullptr) {
            snprintf(vm.sensors[i].label, sizeof(vm.sensors[i].label), "%.4s:", s->getName());
            if (d.isError) {
                snprintf(vm.sensors[i].value, sizeof(vm.sensors[i].value), "ERR");
            } else {
                snprintf(vm.sensors[i].value, sizeof(vm.sensors[i].value), "%.1f%s", d.value, s->getUnit());
            }
        }
    }

    vm.actuatorCount = actuatorCount < 4 ? actuatorCount : 4;
    for (size_t i = 0; i < vm.actuatorCount; ++i) {
        if (actuators[i] != nullptr) {
            snprintf(vm.actuators[i].label, sizeof(vm.actuators[i].label), "%.4s:", actuators[i]->getName());
            snprintf(vm.actuators[i].value, sizeof(vm.actuators[i].value), "%s", actuators[i]->getStatusText());
        }
    }

    snprintf(vm.advisoryBanner, sizeof(vm.advisoryBanner), "%s", healthState.advisoryMsg);
    return vm;
}
