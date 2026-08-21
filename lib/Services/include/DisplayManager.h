#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>

#ifndef UNIT_TEST
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#else
#ifndef SSD1306_SWITCHCAPVCC
#define SSD1306_SWITCHCAPVCC 0
#endif
#ifndef WHITE
#define WHITE 1
#endif
class Adafruit_SSD1306 {
public:
    Adafruit_SSD1306(int w = 128, int h = 64, void* wire = nullptr, int rst = -1) {}
    bool begin(uint8_t switchvcc = 0, uint8_t i2caddr = 0) { return true; }
    void clearDisplay() {}
    void setTextSize(uint8_t s) {}
    void setTextColor(uint16_t c) {}
    void setCursor(int16_t x, int16_t y) {}
    void print(const char* s) {}
    void printf(const char* fmt, ...) {}
    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {}
    void display() {}
};
#endif

#include "config.h"
#include "DisplayViewModel.h"

class DisplayManager {
private:
    Adafruit_SSD1306 display;

public:
    DisplayManager(int width = 128, int height = 64);
    ~DisplayManager();

    bool init();
    void render(const DisplayViewModel& model);
};

#endif // DISPLAY_MANAGER_H