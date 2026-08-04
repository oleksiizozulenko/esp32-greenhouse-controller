#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include "../config.h"
#include "../ui/DisplayViewModel.h"

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

    void render(const DisplayViewModel& vm) {
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
        display.print(vm.modeText);

        display.setCursor(95, 0);
        display.print(vm.healthStatus);

        display.setCursor(0, 9);
        display.println("---------------------");

        // 2. Generic Dynamic Sensors Section
        int yPos = 18;
        for (size_t i = 0; i < vm.sensorCount && i < 4; i += 2) {
            // Left column item
            display.setCursor(0, yPos);
            display.print(vm.sensors[i].label);
            display.print(vm.sensors[i].value);

            // Right column item
            if (i + 1 < vm.sensorCount) {
                display.setCursor(64, yPos);
                display.print(vm.sensors[i + 1].label);
                display.print(vm.sensors[i + 1].value);
            }

            yPos += 10;
        }

        display.setCursor(0, 37);
        display.println("---------------------");

        // 3. Actuators Section or Advisory Banner
        if (vm.advisoryBanner[0] != '\0') {
            display.setCursor(0, 46);
            display.print(vm.advisoryBanner);
        } else {
            yPos = 46;
            for (size_t i = 0; i < vm.actuatorCount && i < 4; i += 2) {
                // Left column item
                display.setCursor(0, yPos);
                display.print(vm.actuators[i].label);
                display.print(vm.actuators[i].value);

                // Right column item
                if (i + 1 < vm.actuatorCount) {
                    display.setCursor(64, yPos);
                    display.print(vm.actuators[i + 1].label);
                    display.print(vm.actuators[i + 1].value);
                }

                yPos += 10;
            }
        }

        display.display();
    }
};

#endif // DISPLAY_MANAGER_H