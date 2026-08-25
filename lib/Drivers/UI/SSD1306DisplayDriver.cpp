#include "SSD1306DisplayDriver.h"
#include "config.h"

#ifndef UNIT_TEST
SSD1306DisplayDriver::SSD1306DisplayDriver(uint8_t width, uint8_t height, TwoWire* wireBus, int8_t rstPin)
    : display(width, height, wireBus, rstPin) {
    headerText[0] = '\0';
    statusText[0] = '\0';
}

bool SSD1306DisplayDriver::begin() {
    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_I2C_ADDR)) {
        return false;
    }
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println(F("System Ready"));
    display.display();
    return true;
}

void SSD1306DisplayDriver::clear() {
    display.clearDisplay();
}

void SSD1306DisplayDriver::setHeader(const char* title) {
    if (title) {
        strncpy(headerText, title, sizeof(headerText) - 1);
        headerText[sizeof(headerText) - 1] = '\0';
    }
}

void SSD1306DisplayDriver::setField(uint8_t slotIndex, const DisplayField& field) {
    int yOffset = 16 + (slotIndex * 12);
    if (yOffset > 48) return;

    display.setCursor(0, yOffset);
    display.print(field.label);
    display.print(F(": "));
    display.print(field.value, field.precision);
    display.print(F(" "));
    display.print(field.unit);
}

void SSD1306DisplayDriver::setStatusLine(const char* statusMsg) {
    if (statusMsg) {
        strncpy(statusText, statusMsg, sizeof(statusText) - 1);
        statusText[sizeof(statusText) - 1] = '\0';
    }
}

void SSD1306DisplayDriver::refresh() {
    display.clearDisplay();

    if (headerText[0] != '\0') {
        display.setCursor(0, 0);
        display.setTextSize(1);
        display.println(headerText);
        display.drawLine(0, 10, 128, 10, SSD1306_WHITE);
    }

    if (statusText[0] != '\0') {
        display.setCursor(0, 54);
        display.print(statusText);
    }

    display.display();
}
#else
SSD1306DisplayDriver::SSD1306DisplayDriver(uint8_t width, uint8_t height, void* wireBus, int8_t rstPin) {
    headerText[0] = '\0';
    statusText[0] = '\0';
}

bool SSD1306DisplayDriver::begin() { return true; }
void SSD1306DisplayDriver::clear() {}
void SSD1306DisplayDriver::setHeader(const char* title) {
    if (title) { strncpy(headerText, title, sizeof(headerText) - 1); }
}
void SSD1306DisplayDriver::setField(uint8_t slotIndex, const DisplayField& field) {}
void SSD1306DisplayDriver::setStatusLine(const char* statusMsg) {
    if (statusMsg) { strncpy(statusText, statusMsg, sizeof(statusText) - 1); }
}
void SSD1306DisplayDriver::refresh() {}
#endif

