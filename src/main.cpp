#include <Arduino.h>
#include "config.h"
#include "drivers/Sensor.h"
#include "drivers/HumiditySensor.h"
#include "drivers/SoilSensor.h"
#include "drivers/TemperatureSensor.h"
#include "drivers/LightSensor.h"
#include "drivers/VentilationActuator.h"
#include "drivers/IrrigationActuator.h"
#include "drivers/LightActuator.h"
#include "drivers/ButtonDriver.h"
#include "services/SensorsService.h"
#include "services/ActuatorsService.h"
#include "services/DisplayManager.h"

// Sensor Drivers
HumiditySensor humiditySensor(PIN_DHT);
SoilSensor soilSensor(PIN_SOIL_POT);
TemperatureSensor temperatureSensor(PIN_TEMP);
LightSensor lightSensor(PIN_LDR);

// Actuator Drivers
VentilationActuator ventActuator(PIN_ACTUATOR_VENT);
IrrigationActuator irrigActuator(PIN_ACTUATOR_IRRIG);
LightActuator lightActuator(PIN_ACTUATOR_LIGHT);

// Button Drivers
ButtonDriver btnMode(PIN_BTN_MODE);
ButtonDriver btnIrrig(PIN_BTN_IRRIG);
ButtonDriver btnVent(PIN_BTN_VENT);
ButtonDriver btnLight(PIN_BTN_LIGHT);

// Services & Managers
DisplayManager displayManager;
SensorsService sensorsService;
ActuatorsService actuatorsService(&ventActuator, &irrigActuator, &lightActuator);

enum SystemMode {
  MODE_MANUAL,
  MODE_AUTOMATIC
};

SystemMode readModeButton() {
  // If Mode button is pressed (LOW with INPUT_PULLUP), system is in AUTOMATIC mode.
  // Otherwise, system is in MANUAL mode.
  if (btnMode.isPressed()) {
    return MODE_AUTOMATIC;
  } else {
    return MODE_MANUAL;
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Greenhouse Controller Starting...");

  // Initialize Sensors
  sensorsService.addSensor(&humiditySensor);
  sensorsService.addSensor(&soilSensor);
  sensorsService.addSensor(&temperatureSensor);
  sensorsService.addSensor(&lightSensor);
  sensorsService.begin();

  // Initialize Actuators
  actuatorsService.begin();

  // Initialize Buttons
  btnMode.init();
  btnIrrig.init();
  btnVent.init();
  btnLight.init();

  // Initialize Display
  displayManager.init();
  Serial.println("Greenhouse Controller Ready.");
}

void handleManualMode() {
  static unsigned long lastLog = 0;
  if (millis() - lastLog > 5000) {
    Serial.println("[MODE] Manual Mode Active");
    lastLog = millis();
  }
}

void handleAutomaticMode(const SensorDataMap& readings) {
  static unsigned long lastLog = 0;
  if (millis() - lastLog > 5000) {
    Serial.println("[MODE] Automatic Mode Active");
    for (size_t i = 0; i < readings.size(); ++i) {
      Sensor* sensor = readings[i].sensor;
      SensorData data = readings[i].data;
      if (data.isError) {
        Serial.printf("Sensor Error: %s\n", sensor ? sensor->getName() : "Unknown");
      } else {
        Serial.printf("Sensor %s: %.2f\n", sensor ? sensor->getName() : "Unknown", data.value);
      }
    }
    lastLog = millis();
  }
}

void loop() {
  // 1. Read all sensors
  SensorDataMap readings = sensorsService.read();

  // 2. Read mode button state
  SystemMode mode = readModeButton();

  // 3. Process mode logging
  if (mode == MODE_AUTOMATIC) {
    handleAutomaticMode(readings);
  } else {
    handleManualMode();
  }

  // 4. Update ActuatorsService (executes automation logic in AUTO, button toggles in MANUAL)
  actuatorsService.update(mode == MODE_AUTOMATIC, readings, &btnIrrig, &btnVent, &btnLight);

  // 5. Update OLED Display
  displayManager.render(mode == MODE_AUTOMATIC, readings, actuatorsService);

  delay(10); // Simulation pacing
}
