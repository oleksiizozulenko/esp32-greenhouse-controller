#ifndef AUTOMATION_SERVICE_H
#define AUTOMATION_SERVICE_H

#include <Arduino.h>
#include "../config.h"
#include "../drivers/Actuator.h"
#include "../drivers/ButtonDriver.h"
#include "SensorsService.h"

static inline bool streq_custom_automation(const char* s1, const char* s2) {
    if (s1 == s2) return true;
    if (!s1 || !s2) return false;
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *s1 == *s2;
}

class AutomationService {
private:
    Actuator** actuators;
    size_t capacity;
    size_t actuatorCount;

    int redLedPin;
    int greenLedPin;
    int buzzerPin;

public:
    AutomationService(size_t initialCapacity = 4,
                      int redLed = PIN_LED_RED,
                      int greenLed = PIN_LED_GREEN,
                      int buzzer = PIN_BUZZER)
        : actuators(nullptr), capacity(0), actuatorCount(0),
          redLedPin(redLed), greenLedPin(greenLed), buzzerPin(buzzer) {
        if (initialCapacity > 0) {
            capacity = initialCapacity;
            actuators = new Actuator*[capacity];
        }
    }

    ~AutomationService() {
        if (actuators != nullptr) {
            delete[] actuators;
            actuators = nullptr;
        }
    }

    AutomationService(const AutomationService&) = delete;
    AutomationService& operator=(const AutomationService&) = delete;

    bool addActuator(Actuator* actuator) {
        if (actuator == nullptr) return false;

        if (actuatorCount >= capacity) {
            size_t newCapacity = (capacity == 0) ? 4 : capacity * 2;
            Actuator** newActuators = new Actuator*[newCapacity];
            for (size_t i = 0; i < actuatorCount; ++i) {
                newActuators[i] = actuators[i];
            }
            if (actuators != nullptr) {
                delete[] actuators;
            }
            actuators = newActuators;
            capacity = newCapacity;
        }
        actuators[actuatorCount++] = actuator;
        return true;
    }

    size_t getActuatorCount() const { return actuatorCount; }

    Actuator* getActuator(size_t index) const {
        if (index < actuatorCount) {
            return actuators[index];
        }
        return nullptr;
    }

    Actuator* getActuator(const char* name) const {
        if (name == nullptr) return nullptr;
        for (size_t i = 0; i < actuatorCount; ++i) {
            if (actuators[i] != nullptr && streq_custom_automation(actuators[i]->getName(), name)) {
                return actuators[i];
            }
        }
        return nullptr;
    }

    void begin() {
        // Initialize Status Indicators
        pinMode(redLedPin, OUTPUT);
        pinMode(greenLedPin, OUTPUT);
        pinMode(buzzerPin, OUTPUT);

        digitalWrite(redLedPin, LOW);
        digitalWrite(greenLedPin, HIGH); // Default normal operation
        digitalWrite(buzzerPin, LOW);

        // Initialize all registered actuators
        for (size_t i = 0; i < actuatorCount; ++i) {
            if (actuators[i] != nullptr) {
                actuators[i]->init();
            }
        }
    }

    void updateSystemIndicators(const SensorDataMap& readings, bool highAlertActive) {
        bool hasError = false;

        for (size_t i = 0; i < readings.size(); ++i) {
            if (readings[i].data.isError) {
                hasError = true;
                break;
            }
        }

        if (hasError) {
            digitalWrite(redLedPin, HIGH);  // LED_RED on (system error)
            digitalWrite(greenLedPin, LOW); // LED_GREEN off
        } else {
            digitalWrite(redLedPin, LOW);   // LED_RED off
            digitalWrite(greenLedPin, HIGH); // LED_GREEN on (all systems working)
        }

        // Buzzer alert for high/critical sensor readings
        if (highAlertActive) {
            tone(buzzerPin, 1000, 100); // 1kHz notification tone
        } else {
            noTone(buzzerPin);
            digitalWrite(buzzerPin, LOW);
        }
    }

    void processAutomatic(const SensorDataMap& readings, bool& highAlertActive) {
        highAlertActive = false;

        // 1. Temperature vs Servo Ventilation Control
        SensorData tempData = readings.get("Temperature");
        Actuator* vent = getActuator("Ventilation");
        if (vent != nullptr) {
            if (tempData.isError) {
                if (vent->isOn()) {
                    Serial.printf("[AUTO] Temp Sensor Error -> Turning OFF Ventilation (%s)\n", vent->getName());
                    vent->turnOff();
                }
            } else if (tempData.value > TEMP_THRESHOLD_HIGH) {
                highAlertActive = true;
                if (!vent->isOn()) {
                    Serial.printf("[AUTO] High Temp (%.2f°C > %.2f°C) -> Opening Ventilation (%s)\n",
                                  tempData.value, TEMP_THRESHOLD_HIGH, vent->getName());
                    vent->turnOn();
                }
            } else if (tempData.value < (TEMP_THRESHOLD_HIGH - TEMP_HYSTERESIS) && vent->isOn()) {
                Serial.printf("[AUTO] Normal Temp (%.2f°C < %.2f°C) -> Closing Ventilation (%s)\n",
                              tempData.value, TEMP_THRESHOLD_HIGH - TEMP_HYSTERESIS, vent->getName());
                vent->turnOff();
            }
        }

        // 2. Soil Moisture vs LED_RING Irrigation Control
        SensorData soilData = readings.get("Soil");
        Actuator* irrig = getActuator("Irrigation");
        if (irrig != nullptr) {
            if (soilData.isError) {
                if (irrig->isOn()) {
                    Serial.printf("[AUTO] Soil Sensor Error -> Turning OFF Irrigation (%s)\n", irrig->getName());
                    irrig->turnOff();
                }
            } else if (soilData.value < SOIL_DRY_THRESHOLD) {
                highAlertActive = true;
                if (!irrig->isOn()) {
                    Serial.printf("[AUTO] Low Soil Moisture (%.2f%% < %d%%) -> Turning ON Irrigation (%s)\n",
                                  soilData.value, SOIL_DRY_THRESHOLD, irrig->getName());
                    irrig->turnOn();
                }
            } else if (soilData.value > (SOIL_DRY_THRESHOLD + SOIL_HYSTERESIS) && irrig->isOn()) {
                Serial.printf("[AUTO] Normal Soil Moisture (%.2f%% > %d%%) -> Turning OFF Irrigation (%s)\n",
                              soilData.value, SOIL_DRY_THRESHOLD + SOIL_HYSTERESIS, irrig->getName());
                irrig->turnOff();
            }
        }

        // 3. Light Sensor vs LED_STRIP Light Control
        SensorData lightData = readings.get("Light");
        Actuator* light = getActuator("Light");
        if (light != nullptr) {
            if (lightData.isError) {
                if (light->isOn()) {
                    Serial.printf("[AUTO] Light Sensor Error -> Turning OFF Light (%s)\n", light->getName());
                    light->turnOff();
                }
            } else if (lightData.value < LIGHT_DARK_THRESHOLD && !light->isOn()) {
                Serial.printf("[AUTO] Low Light (%.2f < %d) -> Turning ON Light (%s)\n",
                              lightData.value, LIGHT_DARK_THRESHOLD, light->getName());
                light->turnOn();
            } else if (lightData.value > (LIGHT_DARK_THRESHOLD + LIGHT_HYSTERESIS) && light->isOn()) {
                Serial.printf("[AUTO] Normal Light (%.2f > %d) -> Turning OFF Light (%s)\n",
                              lightData.value, LIGHT_DARK_THRESHOLD + LIGHT_HYSTERESIS, light->getName());
                light->turnOff();
            }
        }
    }

    void processManual(ButtonDriver* btnIrrig, ButtonDriver* btnVent, ButtonDriver* btnLight) {
        Actuator* irrig = getActuator("Irrigation");
        if (btnIrrig != nullptr && btnIrrig->wasPressed() && irrig != nullptr) {
            if (irrig->isOn()) {
                Serial.println("[MANUAL] Irrigation Button pressed -> Turning OFF Irrigation");
                irrig->turnOff();
            } else {
                Serial.println("[MANUAL] Irrigation Button pressed -> Turning ON Irrigation");
                irrig->turnOn();
            }
        }

        Actuator* vent = getActuator("Ventilation");
        if (btnVent != nullptr && btnVent->wasPressed() && vent != nullptr) {
            if (vent->isOn()) {
                Serial.println("[MANUAL] Ventilation Button pressed -> Closing Ventilation");
                vent->turnOff();
            } else {
                Serial.println("[MANUAL] Ventilation Button pressed -> Opening Ventilation");
                vent->turnOn();
            }
        }

        Actuator* light = getActuator("Light");
        if (btnLight != nullptr && btnLight->wasPressed() && light != nullptr) {
            if (light->isOn()) {
                Serial.println("[MANUAL] Light Button pressed -> Turning OFF Light");
                light->turnOff();
            } else {
                Serial.println("[MANUAL] Light Button pressed -> Turning ON Light");
                light->turnOn();
            }
        }
    }

    void update(bool isAutoMode, const SensorDataMap& readings,
                ButtonDriver* btnIrrig = nullptr,
                ButtonDriver* btnVent = nullptr,
                ButtonDriver* btnLight = nullptr) {
        bool highAlertActive = false;

        if (isAutoMode) {
            processAutomatic(readings, highAlertActive);
        } else {
            processManual(btnIrrig, btnVent, btnLight);
        }

        // Update LED_RED (Error), LED_GREEN (Working), and Buzzer (High data alarm)
        updateSystemIndicators(readings, highAlertActive);
    }
};

#endif // AUTOMATION_SERVICE_H
