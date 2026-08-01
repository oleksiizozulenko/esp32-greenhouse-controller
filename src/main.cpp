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

  sensorsService.addSensor(&humiditySensor);
  sensorsService.addSensor(&soilSensor);
  sensorsService.addSensor(&temperatureSensor);
  sensorsService.addSensor(&lightSensor);

  sensorsService.begin();

  actuatorsService.addActuator(&ventilationActuator);
  actuatorsService.addActuator(&irrigationActuator);
  actuatorsService.addActuator(&lightActuator);

  actuatorsService.begin();

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
  // Implement manual mode logic here
  Serial.println("Manual Mode Active");

  if(btnIrrigationPressed()) {
    actuatorsService.toggleIrrigation();
   // displayManager.render();  // dispaly the current state of the irrigation actuator
  }
  if(btnLightPressed()) {
    actuatorsService.toggleLight();
  }
    if(btnVentilationPressed()) {
        actuatorsService.toggleVentilation();
    }
}

void handleAutomaticMode() {
  // Implement automatic mode logic here
  Serial.println("Automatic Mode Active");

   SensorsData* sensorsValue[] = sensorsService.read();

  for (auto sensorData : sensorsValue) {
    if (sensorData->isError) {
      Serial.println("Error reading from sensor");
    } else {
        Serial.printf("Sensor value: %.2f\n", sensorData->value);
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
