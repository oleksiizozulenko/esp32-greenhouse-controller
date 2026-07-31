#include <Arduino.h>
#include "config.h"
#include "drivers/Sensor.h"
#include "drivers/HumiditySensor.h"
#include "drivers/SoilSensor.h"
#include "drivers/TemperatureSensor.h"
#include "drivers/LightSensor.h"

HumiditySensor humiditySensor(PIN_DHT);
SoilSensor soilSensor(PIN_SOIL_POT);
TemperatureSensor temperatureSensor(PIN_TEMP);
LightSensor lightSensor(PIN_LDR);

DisplayManager displayManager;
SensorsService sensorsService({&temperatureSensor, &humiditySensor, &soilSensor, &lightSensor});

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Hello, ESP32!");

  humiditySensor.init();
  soilSensor.init();
  temperatureSensor.init();
  lightSensor.init();

    displayManager.init();
}

void loop() {
  // put your main code here, to run repeatedly:

   sensorsService.checkAndUpdate();
  displayManager.render();

  delay(10); // this speeds up the simulation
}
