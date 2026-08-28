#include "DisplayManager.h"

DisplayManager::DisplayManager(int width, int height)
    : display(width, height, &Wire, -1), isInitialized(false) {}

DisplayManager::~DisplayManager() {}

bool DisplayManager::init() {
    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_I2C_ADDR)) {
        Serial.println("[DISPLAY] SSD1306 OLED not found or disconnected - Bypassing rendering.");
        isInitialized = false;
        return false;
    }
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.print("Greenhouse Init...");
    display.display();
    isInitialized = true;
    return true;
}

void DisplayManager::render(const DisplayViewModel& model) {
    if (!isInitialized) return;

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);

    display.setCursor(0, 0);
    display.printf("[%s] %s", model.modeText, model.healthStatus);

    display.drawLine(0, 9, 128, 9, WHITE);

    display.setCursor(0, 12);
    for (size_t i = 0; i < model.sensorCount && i < 4; ++i) {
        display.printf("%s %s  ", model.sensors[i].label, model.sensors[i].value);
        if (i % 2 == 1) display.setCursor(0, display.getCursorY() + 10);
    }

    display.drawLine(0, 34, 128, 34, WHITE);

    display.setCursor(0, 37);
    for (size_t i = 0; i < model.actuatorCount && i < 4; ++i) {
        display.printf("%s %s  ", model.actuators[i].label, model.actuators[i].value);
        if (i % 2 == 1) display.setCursor(0, display.getCursorY() + 10);
    }

    display.drawLine(0, 53, 128, 53, WHITE);
    display.setCursor(0, 55);
    display.printf("%.21s", model.advisoryBanner);

    display.display();
}
