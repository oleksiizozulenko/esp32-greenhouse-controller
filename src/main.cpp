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
ButtonDriver btnMode(PIN_BTN_MODE, ButtonType::MODE);
ButtonDriver btnIrrig(PIN_BTN_IRRIG, ButtonType::IRRIGATION);
ButtonDriver btnVent(PIN_BTN_VENT, ButtonType::VENTILATION);
ButtonDriver btnLight(PIN_BTN_LIGHT, ButtonType::LIGHT);

// Services & Managers
DisplayManager displayManager;
SensorsService sensorsService;
SafetyMonitorService safetyMonitorService;
GreenhouseController greenhouseController;

// System Mode & Mutexes
SystemMode currentMode = SystemMode::MANUAL;
SemaphoreHandle_t sensorMutex = NULL;
SemaphoreHandle_t modeMutex = NULL;

// Shared State Guarded by Mutex
SensorDataMap globalReadings;
SystemHealthState globalHealthState;

SystemMode readModeButton() {
  if (btnMode.wasPressed()) {
    if (xSemaphoreTake(modeMutex, portMAX_DELAY) == pdTRUE) {
      currentMode = toggleSystemMode(currentMode);
      xSemaphoreGive(modeMutex);
    }
  }
  SystemMode mode = SystemMode::MANUAL;
  if (xSemaphoreTake(modeMutex, portMAX_DELAY) == pdTRUE) {
    mode = currentMode;
    xSemaphoreGive(modeMutex);
  }
  return mode;
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

// -------------------------------------------------------------------
// FreeRTOS Task Definitions
// -------------------------------------------------------------------

// 1. TaskSensors: Samples sensors every 2000ms
void vTaskSensors(void* pvParameters) {
  (void)pvParameters;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xFrequency = pdMS_TO_TICKS(2000);

  for (;;) {
    SensorDataMap readings = sensorsService.read();

    if (xSemaphoreTake(sensorMutex, portMAX_DELAY) == pdTRUE) {
      globalReadings = readings;
      xSemaphoreGive(sensorMutex);
    }

    vTaskDelayUntil(&xLastWakeTime, xFrequency);
  }
}

// 2. TaskButtons: Polls button state and triggers event listeners every 20ms
void vTaskButtons(void* pvParameters) {
  (void)pvParameters;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xFrequency = pdMS_TO_TICKS(20);

  for (;;) {
    btnIrrig.checkEvent();
    btnVent.checkEvent();
    btnLight.checkEvent();

    readModeButton();

    vTaskDelayUntil(&xLastWakeTime, xFrequency);
  }
}

// 3. TaskControl: Runs automatic control logic and safety monitoring every 100ms
void vTaskControl(void* pvParameters) {
  (void)pvParameters;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xFrequency = pdMS_TO_TICKS(100);

  for (;;) {
    SensorDataMap readingsCopy;
    if (xSemaphoreTake(sensorMutex, portMAX_DELAY) == pdTRUE) {
      readingsCopy = globalReadings;
      xSemaphoreGive(sensorMutex);
    }

    SystemMode mode = SystemMode::MANUAL;
    if (xSemaphoreTake(modeMutex, portMAX_DELAY) == pdTRUE) {
      mode = currentMode;
      xSemaphoreGive(modeMutex);
    }
    bool isAutoMode = (mode == SystemMode::AUTOMATIC);

    SystemHealthState healthState = safetyMonitorService.evaluate(readingsCopy, isAutoMode);

    if (xSemaphoreTake(sensorMutex, portMAX_DELAY) == pdTRUE) {
      globalHealthState = healthState;
      xSemaphoreGive(sensorMutex);
    }

    if (isAutoMode) {
      handleAutomaticMode(readingsCopy);
    } else {
      handleManualMode();
    }

    greenhouseController.update(isAutoMode, readingsCopy, healthState);

    vTaskDelayUntil(&xLastWakeTime, xFrequency);
  }
}

// 4. TaskDisplay: Renders OLED ViewModel every 200ms on Core 0
void vTaskDisplay(void* pvParameters) {
  (void)pvParameters;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xFrequency = pdMS_TO_TICKS(200);

  for (;;) {
    SensorDataMap readingsCopy;
    SystemHealthState healthCopy;

    if (xSemaphoreTake(sensorMutex, portMAX_DELAY) == pdTRUE) {
      readingsCopy = globalReadings;
      healthCopy = globalHealthState;
      xSemaphoreGive(sensorMutex);
    }

    SystemMode mode = SystemMode::MANUAL;
    if (xSemaphoreTake(modeMutex, portMAX_DELAY) == pdTRUE) {
      mode = currentMode;
      xSemaphoreGive(modeMutex);
    }
    bool isAutoMode = (mode == SystemMode::AUTOMATIC);

    DisplayViewModel vm = greenhouseController.buildDisplayViewModel(isAutoMode, readingsCopy, healthCopy);
    displayManager.render(vm);

    vTaskDelayUntil(&xLastWakeTime, xFrequency);
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Greenhouse Controller Starting (RTOS Mode)...");

  // Create Synchronization Mutexes
  sensorMutex = xSemaphoreCreateMutex();
  modeMutex = xSemaphoreCreateMutex();

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
  Serial.println("Greenhouse Controller Hardware Ready. Creating FreeRTOS Tasks...");

  // Create FreeRTOS Tasks
  xTaskCreatePinnedToCore(vTaskSensors, "TaskSensors", 4096, NULL, 2, NULL, 1);
  xTaskCreatePinnedToCore(vTaskButtons, "TaskButtons", 2048, NULL, 3, NULL, 1);
  xTaskCreatePinnedToCore(vTaskControl, "TaskControl", 4096, NULL, 3, NULL, 1);
  xTaskCreatePinnedToCore(vTaskDisplay, "TaskDisplay", 4096, NULL, 1, NULL, 0);

  Serial.println("FreeRTOS Tasks Started Successfully.");
}

void loop() {
  // FreeRTOS scheduler handles tasks. Delete default loop task to reclaim stack memory.
  vTaskDelete(NULL);
}

