#include <Arduino.h>


#define RELAY_IRRIGATION_PIN 25
#define RELAY_VENTILATION_PIN 26
#define RELAY_LIGHT_PIN 27

#define OLED_SDA_PIN 21
#define OLED_SDC_PIN 22

#define BUSSER_PIN  18
//#define LED_PIN

#define BTN_MODE_PIN 13
#define BTN_IRRIGATION_PIN 12
#define BTN_VENTILATION_PIN 14
#define BTN_LIGHT_PIN 16

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Hello, ESP32!");
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(10); // this speeds up the simulation
}
