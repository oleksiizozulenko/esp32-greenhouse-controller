#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include "../config.h"
#include "SensorsService.h"
#include "AutomationService.h"

#define SCREEN_ADDR 0x3C
#define OLED_SDA 21
#define OLED_SCL 22
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

class DisplayManager {
private:
    Adafruit_SSD1306 display;
    unsigned long lastRefreshTime;
    const unsigned long refreshInterval;

public:
    DisplayManager(unsigned long refreshInterval = OLED_REFRESH_INTERVAL)
        : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1),
          lastRefreshTime(0),
          refreshInterval(refreshInterval) {}

    void init() {
        Wire.begin(OLED_SDA, OLED_SCL);

        if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDR)) {
            Serial.println(F("SSD1306 allocation failed"));
            return;
        }
        display.clearDisplay();
        display.setTextColor(SSD1306_WHITE);
        display.setTextSize(1);
        display.setCursor(0, 0);
        display.println(F("Greenhouse System"));
        display.println(F("Initializing..."));
        display.display();
    }

    void render(bool isAutoMode, const SensorDataMap& readings, const AutomationService& automationService) {
        unsigned long currentTime = millis();
        if (currentTime - lastRefreshTime < refreshInterval) {
            return;
        }
        lastRefreshTime = currentTime;

        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);

        // 1. Header: Mode & System Health Status ([OK] or [ERR])
        display.setCursor(0, 0);
        display.print("MODE: ");
        display.print(isAutoMode ? "AUTO" : "MANUAL");

        bool hasError = false;
        for (size_t i = 0; i < readings.size(); ++i) {
            if (readings[i].data.isError) {
                hasError = true;
                break;
            }
        }
        display.setCursor(95, 0);
        display.print(hasError ? "[ERR]" : "[OK]");

        display.setCursor(0, 9);
        display.println("---------------------");

        // 2. Generic Dynamic Sensors Section (OCP-Compliant)
        int yPos = 18;
        size_t sensorCount = readings.size();
        for (size_t i = 0; i < sensorCount && i < 4; i += 2) {
            // Left column item
            Sensor* s1 = readings[i].sensor;
            SensorData d1 = readings[i].data;
            if (s1 != nullptr) {
                display.setCursor(0, yPos);
                char label[6];
                snprintf(label, sizeof(label), "%.4s:", s1->getName());
                display.print(label);
                if (d1.isError) {
                    display.print("ERR");
                } else {
                    display.print(d1.value, 1);
                    display.print(s1->getUnit());
                }
            }

            // Right column item
            if (i + 1 < sensorCount) {
                Sensor* s2 = readings[i + 1].sensor;
                SensorData d2 = readings[i + 1].data;
                if (s2 != nullptr) {
                    display.setCursor(64, yPos);
                    char label[6];
                    snprintf(label, sizeof(label), "%.4s:", s2->getName());
                    display.print(label);
                    if (d2.isError) {
                        display.print("ERR");
                    } else {
                        display.print(d2.value, 1);
                        display.print(s2->getUnit());
                    }
                }
            }

            yPos += 10;
        }

        display.setCursor(0, 37);
        display.println("---------------------");

        // 3. Generic Dynamic Actuators Section (OCP-Compliant)
        yPos = 46;
        size_t actCount = automationService.getActuatorCount();
        for (size_t i = 0; i < actCount && i < 4; i += 2) {
            // Left column item
            Actuator* a1 = automationService.getActuator(i);
            if (a1 != nullptr) {
                display.setCursor(0, yPos);
                char label[6];
                snprintf(label, sizeof(label), "%.4s:", a1->getName());
                display.print(label);
                display.print(a1->getStatusText());
            }

            // Right column item
            if (i + 1 < actCount) {
                Actuator* a2 = automationService.getActuator(i + 1);
                if (a2 != nullptr) {
                    display.setCursor(64, yPos);
                    char label[6];
                    snprintf(label, sizeof(label), "%.4s:", a2->getName());
                    display.print(label);
                    display.print(a2->getStatusText());
                }
            }

            yPos += 10;
        }

        display.display();
    }
};

#endif // DISPLAY_MANAGER_H