#include <Arduino.h>
#include "config.h"
#include "drivers/Sensor.h"
#include "drivers/HumiditySensor.h"
#include "drivers/SoilSensor.h"
#include "drivers/TemperatureSensor.h"
#include "drivers/LightSensor.h"
#include "services/SensorsService.h"
#include "services/DisplayManager.h"

HumiditySensor humiditySensor(PIN_DHT);
SoilSensor soilSensor(PIN_SOIL_POT);
TemperatureSensor temperatureSensor(PIN_TEMP);
LightSensor lightSensor(PIN_LDR);

DisplayManager displayManager;
SensorsService sensorsService;

void setup() {

  Serial.begin(115200);
  Serial.println("Hello, ESP32!");

  humiditySensor.init();
  soilSensor.init();
  temperatureSensor.init();
  lightSensor.init();

  sensorsService.addSensor(&humiditySensor);
  sensorsService.addSensor(&soilSensor);
  sensorsService.addSensor(&temperatureSensor);
  sensorsService.addSensor(&lightSensor);

  sensorsService.begin();
  displayManager.init();
}

enum SystemMode {
  MODE_MANUAL,
  MODE_AUTOMATIC
};

SystemMode readModeButton() {
  if (digitalRead(PIN_BTN_MODE) == LOW) {
    return MODE_MANUAL;
  } else {
    return MODE_AUTOMATIC;
  }
}

void handleManualMode() {
  Serial.println("Manual Mode Active");
}

void handleAutomaticMode() {
  Serial.println("Automatic Mode Active");

  SensorDataMap readings = sensorsService.read();

  for (size_t i = 0; i < readings.size(); ++i) {
    Sensor* sensor = readings[i].sensor;
    SensorData data = readings[i].data;
    if (data.isError) {
      Serial.printf("Error reading sensor: %s\n", sensor ? sensor->getName() : "Unknown");
    } else {
      Serial.printf("Sensor %s value: %.2f\n", sensor ? sensor->getName() : "Unknown", data.value);
    }
  }
}


void loop() {
  // put your main code here, to run repeatedly:

  SystemMode mode = readModeButton(); // Implement this function to read the mode button state

  switch (mode) {
    case MODE_MANUAL:
      handleManualMode();
      break;
    case MODE_AUTOMATIC:
      handleAutomaticMode();
      break;

  }

  delay(10); // this speeds up the simulation
}
