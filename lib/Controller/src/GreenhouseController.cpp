#include "GreenhouseController.h"

void GreenhouseController::vActuatorTimerCallback(TimerHandle_t xTimer) {
    TimerContext* ctx = (TimerContext*)pvTimerGetTimerID(xTimer);
    if (ctx != nullptr && ctx->controller != nullptr) {
        IActuator* act = ctx->controller->getActuator(ctx->actuatorType);
        if (act != nullptr && act->isOperating()) {
            Serial.printf("[SAFETY TIMER] %s Timer Expired -> Auto Turning OFF\n", act->getName());
            act->turnOff();
        }
    }
}

GreenhouseController::GreenhouseController(size_t initialCapacity, int redLed, int greenLed, int buzzer)
    : actuators{}, actuatorCount(0), timers{}, timerCount(0),
      redLedPin(redLed), greenLedPin(greenLed), buzzerPin(buzzer) {
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
    ActuatorType targetType = ActuatorType::UNKNOWN;
    if (button == ButtonType::VENTILATION) {
        targetType = ActuatorType::VENTILATION;
    } else if (button == ButtonType::IRRIGATION) {
        targetType = ActuatorType::IRRIGATION;
    } else if (button == ButtonType::LIGHT) {
        targetType = ActuatorType::LIGHT;
    }

    IActuator* act = getActuator(targetType);
    if (act != nullptr) {
        if (act->isOperating()) {
            Serial.printf("[EVENT] Button %d pressed -> Turning OFF %s\n", (int)button, act->getName());
            act->turnOff();
            stopTimerFor(targetType);
        } else {
            Serial.printf("[EVENT] Button %d pressed -> Turning ON %s (With Safety Timer)\n", (int)button, act->getName());
            act->turnOn();

            uint32_t timeoutMs = getActuatorTimeout(targetType);
            startTimerFor(targetType, timeoutMs);
        }
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
    pinMode(redLedPin, OUTPUT);
    pinMode(greenLedPin, OUTPUT);
    pinMode(buzzerPin, OUTPUT);

    digitalWrite(redLedPin, LOW);
    digitalWrite(greenLedPin, HIGH); // Default normal operation
    digitalWrite(buzzerPin, LOW);

    for (size_t i = 0; i < actuatorCount; ++i) {
        if (actuators[i] != nullptr) {
            actuators[i]->begin();
        }
    }
}

void GreenhouseController::updateSystemIndicators(const SystemHealthState& healthState) {
    if (healthState.hasHardwareError) {
        digitalWrite(redLedPin, HIGH);  // LED_RED on (system error)
        digitalWrite(greenLedPin, LOW); // LED_GREEN off
    } else {
        digitalWrite(redLedPin, LOW);   // LED_RED off
        digitalWrite(greenLedPin, HIGH); // LED_GREEN on (all systems working)
    }

    if (healthState.requiresAlarm) {
        tone(buzzerPin, 1000, 100); // 1kHz notification tone
    } else {
        noTone(buzzerPin);
        digitalWrite(buzzerPin, LOW);
    }
}

void GreenhouseController::processAutomatic(const SensorDataMap& readings) {
    SensorData tempData = readings.get(SensorType::TEMPERATURE);
    SensorData humData = readings.get(SensorType::HUMIDITY);
    IActuator* vent = getActuator(ActuatorType::VENTILATION);
    if (vent != nullptr) {
        bool tempError = tempData.isError;
        bool humError = humData.isError;

        bool highTemp = !tempError && (tempData.value > TEMP_THRESHOLD_HIGH);
        bool highHum = !humError && (humData.value > HUMIDITY_THRESHOLD_HIGH);

        bool normalTemp = tempError || (tempData.value < (TEMP_THRESHOLD_HIGH - TEMP_HYSTERESIS));
        bool normalHum = humError || (humData.value < (HUMIDITY_THRESHOLD_HIGH - HUMIDITY_HYSTERESIS));

        if (tempError && humError) {
            if (vent->isOperating()) {
                Serial.printf("[AUTO] Temp & Humidity Sensor Error -> Turning OFF Ventilation (%s)\n", vent->getName());
                vent->turnOff();
                stopTimerFor(ActuatorType::VENTILATION);
            }
        } else if (highTemp || highHum) {
            if (!vent->isOperating()) {
                Serial.printf("[AUTO] High %s -> Opening Ventilation (%s)\n",
                              highTemp ? "Temp" : "Air Humidity", vent->getName());
                vent->turnOn();
            }
        } else if (normalTemp && normalHum && vent->isOperating()) {
            Serial.printf("[AUTO] Normal Temp & Humidity -> Closing Ventilation (%s)\n", vent->getName());
            vent->turnOff();
            stopTimerFor(ActuatorType::VENTILATION);
        }
    }

    SensorData soilData = readings.get(SensorType::SOIL);
    IActuator* irrig = getActuator(ActuatorType::IRRIGATION);
    if (irrig != nullptr) {
        if (soilData.isError) {
            if (irrig->isOperating()) {
                Serial.printf("[AUTO] Soil Sensor Error -> Turning OFF Irrigation (%s)\n", irrig->getName());
                irrig->turnOff();
                stopTimerFor(ActuatorType::IRRIGATION);
            }
        } else if (soilData.value < SOIL_DRY_THRESHOLD) {
            if (!irrig->isOperating()) {
                Serial.printf("[AUTO] Low Soil Moisture (%.2f%% < %d%%) -> Turning ON Irrigation (%s)\n",
                              soilData.value, SOIL_DRY_THRESHOLD, irrig->getName());
                irrig->turnOn();
            }
        } else if (soilData.value > (SOIL_DRY_THRESHOLD + SOIL_HYSTERESIS) && irrig->isOperating()) {
            Serial.printf("[AUTO] Normal Soil Moisture (%.2f%% > %d%%) -> Turning OFF Irrigation (%s)\n",
                          soilData.value, SOIL_DRY_THRESHOLD + SOIL_HYSTERESIS, irrig->getName());
            irrig->turnOff();
            stopTimerFor(ActuatorType::IRRIGATION);
        }
    }

    SensorData lightData = readings.get(SensorType::LIGHT);
    IActuator* light = getActuator(ActuatorType::LIGHT);
    if (light != nullptr) {
        if (lightData.isError) {
            if (light->isOperating()) {
                Serial.printf("[AUTO] Light Sensor Error -> Turning OFF Light (%s)\n", light->getName());
                light->turnOff();
                stopTimerFor(ActuatorType::LIGHT);
            }
        } else if (lightData.value < LIGHT_DARK_THRESHOLD && !light->isOperating()) {
            Serial.printf("[AUTO] Low Light (%.2f < %.2f) -> Turning ON Light (%s)\n",
                          lightData.value, LIGHT_DARK_THRESHOLD, light->getName());
            light->turnOn();
        } else if (lightData.value > (LIGHT_DARK_THRESHOLD + LIGHT_HYSTERESIS) && light->isOperating()) {
            Serial.printf("[AUTO] Normal Light (%.2f > %.2f) -> Turning OFF Light (%s)\n",
                          lightData.value, LIGHT_DARK_THRESHOLD + LIGHT_HYSTERESIS, light->getName());
            light->turnOff();
            stopTimerFor(ActuatorType::LIGHT);
        }
    }
}


void GreenhouseController::processManual(const SensorDataMap& readings, const SystemHealthState& healthState) {
    (void)readings;
    if (healthState.hasCriticalHazard || healthState.hasOperatorAdvisory) {
        Serial.printf("[MANUAL] SAFETY ALERT: %s\n", healthState.advisoryMsg);
    }
}

void GreenhouseController::update(bool isAutoMode, const SensorDataMap& readings, const SystemHealthState& healthState) {
    if (isAutoMode) {
        processAutomatic(readings);
    } else {
        processManual(readings, healthState);
    }

    updateSystemIndicators(healthState);
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
