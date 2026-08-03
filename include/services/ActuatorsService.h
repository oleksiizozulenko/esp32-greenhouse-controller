#ifndef ACTUATORS_SERVICE_H
#define ACTUATORS_SERVICE_H

#include <Arduino.h>
#include "../config.h"
#include "../drivers/Actuator.h"
#include "../drivers/VentilationActuator.h"
#include "../drivers/IrrigationActuator.h"
#include "../drivers/LightActuator.h"
#include "../drivers/ButtonDriver.h"
#include "SensorsService.h"

class ActuatorsService {
private:
    VentilationActuator* ventActuator;
    IrrigationActuator* irrigActuator;
    LightActuator* lightActuator;

    int redLedPin;
    int greenLedPin;
    int buzzerPin;

public:
    ActuatorsService(VentilationActuator* vent = nullptr,
                     IrrigationActuator* irrig = nullptr,
                     LightActuator* light = nullptr,
                     int redLed = PIN_LED_RED,
                     int greenLed = PIN_LED_GREEN,
                     int buzzer = PIN_BUZZER)
        : ventActuator(vent), irrigActuator(irrig), lightActuator(light),
          redLedPin(redLed), greenLedPin(greenLed), buzzerPin(buzzer) {}

    void setVentilationActuator(VentilationActuator* vent) { ventActuator = vent; }
    void setIrrigationActuator(IrrigationActuator* irrig) { irrigActuator = irrig; }
    void setLightActuator(LightActuator* light) { lightActuator = light; }

    VentilationActuator* getVentilationActuator() const { return ventActuator; }
    IrrigationActuator* getIrrigationActuator() const { return irrigActuator; }
    LightActuator* getLightActuator() const { return lightActuator; }

    void begin() {
        // Initialize Status Indicators
        pinMode(redLedPin, OUTPUT);
        pinMode(greenLedPin, OUTPUT);
        pinMode(buzzerPin, OUTPUT);

        digitalWrite(redLedPin, LOW);
        digitalWrite(greenLedPin, HIGH); // Default normal operation
        digitalWrite(buzzerPin, LOW);

        // Initialize Actuators
        if (ventActuator != nullptr) {
            ventActuator->init();
        }
        if (irrigActuator != nullptr) {
            irrigActuator->init();
        }
        if (lightActuator != nullptr) {
            lightActuator->init();
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
        if (!tempData.isError && ventActuator != nullptr) {
            if (tempData.value > TEMP_THRESHOLD_HIGH) {
                highAlertActive = true;
                if (!ventActuator->isOn()) {
                    Serial.printf("[AUTO] High Temp (%.2f°C > %.2f°C) -> Opening Servo Ventilation\n",
                                  tempData.value, TEMP_THRESHOLD_HIGH);
                    ventActuator->turnOn();
                }
            } else if (tempData.value < (TEMP_THRESHOLD_HIGH - TEMP_HYSTERESIS) && ventActuator->isOn()) {
                Serial.printf("[AUTO] Normal Temp (%.2f°C < %.2f°C) -> Closing Servo Ventilation\n",
                              tempData.value, TEMP_THRESHOLD_HIGH - TEMP_HYSTERESIS);
                ventActuator->turnOff();
            }
        }

        // 2. Soil Moisture vs LED_RING Irrigation Control
        SensorData soilData = readings.get("Soil");
        if (!soilData.isError && irrigActuator != nullptr) {
            if (soilData.value < SOIL_DRY_THRESHOLD) {
                highAlertActive = true;
                if (!irrigActuator->isOn()) {
                    Serial.printf("[AUTO] Low Soil Moisture (%.2f%% < %d%%) -> Turning ON LED_RING Irrigation\n",
                                  soilData.value, SOIL_DRY_THRESHOLD);
                    irrigActuator->turnOn();
                }
            } else if (soilData.value > (SOIL_DRY_THRESHOLD + SOIL_HYSTERESIS) && irrigActuator->isOn()) {
                Serial.printf("[AUTO] Normal Soil Moisture (%.2f%% > %d%%) -> Turning OFF LED_RING Irrigation\n",
                              soilData.value, SOIL_DRY_THRESHOLD + SOIL_HYSTERESIS);
                irrigActuator->turnOff();
            }
        }

        // 3. Light Sensor vs LED_STRIP Light Control
        SensorData lightData = readings.get("Light");
        if (!lightData.isError && lightActuator != nullptr) {
            if (lightData.value < LIGHT_DARK_THRESHOLD && !lightActuator->isOn()) {
                Serial.printf("[AUTO] Low Light (%.2f < %d) -> Turning ON LED_STRIP Light\n",
                              lightData.value, LIGHT_DARK_THRESHOLD);
                lightActuator->turnOn();
            } else if (lightData.value > (LIGHT_DARK_THRESHOLD + LIGHT_HYSTERESIS) && lightActuator->isOn()) {
                Serial.printf("[AUTO] Normal Light (%.2f > %d) -> Turning OFF LED_STRIP Light\n",
                              lightData.value, LIGHT_DARK_THRESHOLD + LIGHT_HYSTERESIS);
                lightActuator->turnOff();
            }
        }
    }

    void processManual(ButtonDriver* btnIrrig, ButtonDriver* btnVent, ButtonDriver* btnLight) {
        if (btnIrrig != nullptr && btnIrrig->wasPressed() && irrigActuator != nullptr) {
            if (irrigActuator->isOn()) {
                Serial.println("[MANUAL] Irrigation Button pressed -> Turning OFF LED_RING Irrigation");
                irrigActuator->turnOff();
            } else {
                Serial.println("[MANUAL] Irrigation Button pressed -> Turning ON LED_RING Irrigation");
                irrigActuator->turnOn();
            }
        }

        if (btnVent != nullptr && btnVent->wasPressed() && ventActuator != nullptr) {
            if (ventActuator->isOn()) {
                Serial.println("[MANUAL] Ventilation Button pressed -> Closing Servo Ventilation");
                ventActuator->turnOff();
            } else {
                Serial.println("[MANUAL] Ventilation Button pressed -> Opening Servo Ventilation");
                ventActuator->turnOn();
            }
        }

        if (btnLight != nullptr && btnLight->wasPressed() && lightActuator != nullptr) {
            if (lightActuator->isOn()) {
                Serial.println("[MANUAL] Light Button pressed -> Turning OFF LED_STRIP Light");
                lightActuator->turnOff();
            } else {
                Serial.println("[MANUAL] Light Button pressed -> Turning ON LED_STRIP Light");
                lightActuator->turnOn();
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

#endif // ACTUATORS_SERVICE_H