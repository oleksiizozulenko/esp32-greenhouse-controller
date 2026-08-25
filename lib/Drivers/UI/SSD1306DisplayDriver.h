#ifndef SSD1306_DISPLAY_DRIVER_H
#define SSD1306_DISPLAY_DRIVER_H

#include <Arduino.h>
#ifndef UNIT_TEST
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#endif
#include "UI/IDisplay.h"

class SSD1306DisplayDriver : public IDisplay {
private:
#ifndef UNIT_TEST
    Adafruit_SSD1306 display;
#endif
    char headerText[32];
    char statusText[32];


public:
#ifndef UNIT_TEST
    SSD1306DisplayDriver(uint8_t width = 128, uint8_t height = 64, TwoWire* wireBus = &Wire, int8_t rstPin = -1);
#else
    SSD1306DisplayDriver(uint8_t width = 128, uint8_t height = 64, void* wireBus = nullptr, int8_t rstPin = -1);
#endif
    ~SSD1306DisplayDriver() override = default;


    bool begin() override;
    void clear() override;
    void setHeader(const char* title) override;
    void setField(uint8_t slotIndex, const DisplayField& field) override;
    void setStatusLine(const char* statusMsg) override;
    void refresh() override;
};

#endif // SSD1306_DISPLAY_DRIVER_H
