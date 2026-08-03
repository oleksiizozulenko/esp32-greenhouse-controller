#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include "../config.h"
#include "SensorsService.h"
#include "ActuatorsService.h"

#define SCREEN_ADDR 0x3C
#define OLED_SDA 21
#define OLED_SCL 22
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

enum ScreenState {
    SCREEN_MAIN
};

class DisplayManager {
private:
    Adafruit_SSD1306 display;
    unsigned long lastRefreshTime;
    const unsigned long refreshInterval;

public:
    ScreenState currentScreen;

    DisplayManager(unsigned long refreshInterval = OLED_REFRESH_INTERVAL)
        : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1),
          lastRefreshTime(0),
          refreshInterval(refreshInterval),
          currentScreen(SCREEN_MAIN) {}

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

    void render(bool isAutoMode, const SensorDataMap& readings, const ActuatorsService& actuatorsService) {
        unsigned long currentTime = millis();
        if (currentTime - lastRefreshTime < refreshInterval) {
            return;
        }
        lastRefreshTime = currentTime;

        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);

        // Header: Mode & System Health Status ([OK] or [ERR])
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
        if (hasError) {
            display.print("[ERR]");
        } else {
            display.print("[OK]");
        }

        display.setCursor(0, 9);
        display.println("---------------------");

        // Sensors Section
        SensorData tempData = readings.get("Temperature");
        SensorData humData = readings.get("Humidity");
        SensorData soilData = readings.get("Soil");
        SensorData lightData = readings.get("Light");

        // Line 1: Temp & Humidity
        display.setCursor(0, 18);
        display.print("T:");
        if (tempData.isError) {
            display.print("ERR  ");
        } else {
            display.print(tempData.value, 1);
            display.print("C  ");
        }

        display.setCursor(64, 18);
        display.print("H:");
        if (humData.isError) {
            display.print("ERR");
        } else {
            display.print(humData.value, 1);
            display.print("%");
        }

        // Line 2: Soil & Light
        display.setCursor(0, 28);
        display.print("S:");
        if (soilData.isError) {
            display.print("ERR  ");
        } else {
            display.print(soilData.value, 1);
            display.print("%  ");
        }

        display.setCursor(64, 28);
        display.print("L:");
        if (lightData.isError) {
            display.print("ERR");
        } else {
            display.print(lightData.value, 0);
        }

        display.setCursor(0, 37);
        display.println("---------------------");

        // Actuators Section
        bool ventOn = actuatorsService.getVentilationActuator() ? actuatorsService.getVentilationActuator()->isOn() : false;
        bool irrigOn = actuatorsService.getIrrigationActuator() ? actuatorsService.getIrrigationActuator()->isOn() : false;
        bool lightOn = actuatorsService.getLightActuator() ? actuatorsService.getLightActuator()->isOn() : false;

        display.setCursor(0, 46);
        display.print("Vent: ");
        display.print(ventOn ? "OPEN " : "CLOSE");

        display.setCursor(64, 46);
        display.print("Irrig: ");
        display.print(irrigOn ? "ON " : "OFF");

        display.setCursor(0, 56);
        display.print("Light: ");
        display.print(lightOn ? "ON" : "OFF");

        display.display();
    }
};

#endif // DISPLAY_MANAGER_H