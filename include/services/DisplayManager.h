#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>

#ifndef UNIT_TEST
#include <Adafruit_SSD1306.h>
#else
#ifndef SSD1306_SWITCHCAPVCC
#define SSD1306_SWITCHCAPVCC 0
#endif
#ifndef SSD1306_WHITE
#define SSD1306_WHITE 1
#endif
struct TwoWireMock {
    void begin(int, int) {}
};
static TwoWireMock Wire;
class Adafruit_SSD1306 {
public:
    Adafruit_SSD1306(uint8_t w = 0, uint8_t h = 0, TwoWireMock* wire = nullptr, int rst = -1) {}
    bool begin(uint8_t switchvcc = 0, uint8_t i2caddr = 0) { return true; }
    void clearDisplay() {}
    void setTextColor(uint16_t c) {}
    void setTextSize(uint8_t s) {}
    void setCursor(int16_t x, int16_t y) {}
    void print(const char*) {}
    void print(int) {}
    void print(float) {}
    void println(const char* = "") {}
    void display() {}
};
#endif

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
    DisplayManager(unsigned long refreshInterval = OLED_REFRESH_INTERVAL);

    void init();
    void render(const DisplayViewModel& vm);
};

#endif // DISPLAY_MANAGER_H