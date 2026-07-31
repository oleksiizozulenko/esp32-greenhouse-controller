#include <Adafruit_SSD1306.h>


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

    public:
        ScreenState currentScreen = SCREEN_MAIN;

        DisplayManager() : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1) {}

        void init() {
            Wire.begin(OLED_SDA, OLED_SCL);

            if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDR)) {
                Serial.println(F("SSD1306 allocation failed"));
                for(;;);
            }
            display.clearDisplay();
            display.setTextColor(SSD1306_WHITE);
        }

        void render() {
            display.clearDisplay();
            display.setCursor(0, 0);

            switch (currentScreen) {
                case SCREEN_MAIN:
                    display.setTextSize(1);
                    display.println("Main Screen");
                    break;
            }

            display.display();
        }

};