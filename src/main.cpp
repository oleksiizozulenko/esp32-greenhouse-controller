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
#include "services/SafetyMonitorService.h"
#include "services/DisplayManager.h"
#include "GreenhouseController.h"

// Sensor Drivers
DHT dht(PIN_DHT, DHT_TYPE);
HumiditySensor humiditySensor(PIN_DHT, &dht);
SoilSensor soilSensor(PIN_SOIL_POT);
TemperatureSensor temperatureSensor(PIN_TEMP, &dht);
LightSensor lightSensor(PIN_LDR);

// Actuator Drivers
VentilationActuator ventActuator(PIN_ACTUATOR_VENT);
IrrigationActuator irrigActuator(PIN_ACTUATOR_IRRIG);
LightActuator lightActuator(PIN_ACTUATOR_LIGHT);

// Button Drivers
// Button Drivers
ButtonDriver btnMode(PIN_BTN_MODE, ButtonType::MODE);
ButtonDriver btnIrrig(PIN_BTN_IRRIG, ButtonType::IRRIGATION);
ButtonDriver btnVent(PIN_BTN_VENT, ButtonType::VENTILATION);
ButtonDriver btnLight(PIN_BTN_LIGHT, ButtonType::LIGHT);

// Services & Managers
DisplayManager displayManager;
SensorsService sensorsService;
SafetyMonitorService safetyMonitorService;
GreenhouseController greenhouseController;

SystemMode currentMode = SystemMode::MANUAL;

SystemMode readModeButton() {
  if (btnMode.wasPressed()) {
    currentMode = toggleSystemMode(currentMode);
  }
  return currentMode;
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

  // Register and Initialize Actuators
  greenhouseController.addActuator(&ventActuator);
  greenhouseController.addActuator(&irrigActuator);
  greenhouseController.addActuator(&lightActuator);
  greenhouseController.begin();

  // Subscribe GreenhouseController to Button Events
  btnIrrig.setListener(&greenhouseController);
  btnVent.setListener(&greenhouseController);
  btnLight.setListener(&greenhouseController);

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
  // 1. Poll button events for event listener notifications
  btnIrrig.checkEvent();
  btnVent.checkEvent();
  btnLight.checkEvent();

  // 2. Read all sensors
  SensorDataMap readings = sensorsService.read();

  // 3. Read mode button state
  SystemMode mode = readModeButton();
  bool isAutoMode = (mode == SystemMode::AUTOMATIC);

  // 4. Safety Evaluation
  SystemHealthState healthState = safetyMonitorService.evaluate(readings, isAutoMode);

  // 5. Process mode logging
  if (isAutoMode) {
    handleAutomaticMode(readings);
  } else {
    handleManualMode();
  }

  // 6. Update GreenhouseController
  greenhouseController.update(isAutoMode, readings, healthState);

  // 6. Update OLED Display
  displayManager.render(greenhouseController.buildDisplayViewModel(isAutoMode, readings, healthState));

  delay(10); // Simulation pacing
}
