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
#include "drivers/buttons/ButtonDriver.h"
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

// System Mode & Mutexes / Queues / Event Groups
SystemMode currentMode = SystemMode::MANUAL;
SemaphoreHandle_t sensorMutex = NULL;
SemaphoreHandle_t modeMutex = NULL;
QueueHandle_t buttonEventQueue = NULL;

// FreeRTOS Event Group Handle & Bitmask Definitions
EventGroupHandle_t systemEventGroup = NULL;
#define EVENT_BIT_SENSOR_READY    (1 << 0) // Bit 0: New sensor data sampled
#define EVENT_BIT_BUTTON_EVENT    (1 << 1) // Bit 1: Hardware button event queued
#define EVENT_BIT_SAFETY_WARNING  (1 << 2) // Bit 2: Safety warning condition triggered
#define EVENT_BIT_MODE_CHANGED    (1 << 3) // Bit 3: System mode toggled (AUTO/MANUAL)

// FreeRTOS Task Handles for Memory Profiling
TaskHandle_t hTaskSensors = NULL;
TaskHandle_t hTaskControl = NULL;
TaskHandle_t hTaskDisplay = NULL;

// Shared State Guarded by Mutex
SensorDataMap globalReadings;
SystemHealthState globalHealthState;

void printTaskStackDiagnostics() {
  static unsigned long lastDiag = 0;
  if (millis() - lastDiag > 10000) {
    lastDiag = millis();
    Serial.println("\n========== FreeRTOS Task Memory Diagnostics ==========");
    if (hTaskSensors) {
      UBaseType_t hwmSensors = uxTaskGetStackHighWaterMark(hTaskSensors);
      Serial.printf(" [TaskSensors] Free Stack: %u words (%u bytes)\n",
                    (unsigned int)hwmSensors, (unsigned int)(hwmSensors * sizeof(StackType_t)));
    }
    if (hTaskControl) {
      UBaseType_t hwmControl = uxTaskGetStackHighWaterMark(hTaskControl);
      Serial.printf(" [TaskControl] Free Stack: %u words (%u bytes)\n",
                    (unsigned int)hwmControl, (unsigned int)(hwmControl * sizeof(StackType_t)));
    }
    if (hTaskDisplay) {
      UBaseType_t hwmDisplay = uxTaskGetStackHighWaterMark(hTaskDisplay);
      Serial.printf(" [TaskDisplay] Free Stack: %u words (%u bytes)\n",
                    (unsigned int)hwmDisplay, (unsigned int)(hwmDisplay * sizeof(StackType_t)));
    }
    Serial.printf(" [System] Total Heap Free: %u bytes\n", (unsigned int)ESP.getFreeHeap());
    Serial.println("======================================================\n");
  }
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

// 1. TaskSensors: Samples sensors every 2000ms and broadcasts EVENT_BIT_SENSOR_READY
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

    // Publish event: Broadcast SENSOR_READY bit to all subscribers!
    if (systemEventGroup != NULL) {
      xEventGroupSetBits(systemEventGroup, EVENT_BIT_SENSOR_READY);
    }

    vTaskDelayUntil(&xLastWakeTime, xFrequency);
  }
}

// 2. TaskControl: Subscriber 1 -> Listens for SENSOR_READY, BUTTON_EVENT, and SAFETY_WARNING
void vTaskControl(void* pvParameters) {
  (void)pvParameters;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xFrequency = pdMS_TO_TICKS(100);

  for (;;) {
    // Check subscribed bits from Event Group
    if (systemEventGroup != NULL) {
      EventBits_t bits = xEventGroupWaitBits(
          systemEventGroup,
          EVENT_BIT_SENSOR_READY | EVENT_BIT_BUTTON_EVENT | EVENT_BIT_SAFETY_WARNING,
          pdTRUE,  // Clear bits on exit
          pdFALSE, // Wake on ANY bit
          0        // Non-blocking poll
      );

      if (bits & EVENT_BIT_SENSOR_READY) {
        Serial.println("[EVENT GROUP] vTaskControl notified: EVENT_BIT_SENSOR_READY!");
      }
      if (bits & EVENT_BIT_BUTTON_EVENT) {
        Serial.println("[EVENT GROUP] vTaskControl notified: EVENT_BIT_BUTTON_EVENT!");
      }
      if (bits & EVENT_BIT_SAFETY_WARNING) {
        Serial.println("[EVENT GROUP] vTaskControl notified: EVENT_BIT_SAFETY_WARNING!");
      }
    }

    // A. Drain and process queued hardware interrupt events
    ButtonEvent evt;
    while (xQueueReceive(buttonEventQueue, &evt, 0) == pdTRUE) {
      if (evt.type == ButtonType::MODE) {
        if (xSemaphoreTake(modeMutex, portMAX_DELAY) == pdTRUE) {
          currentMode = toggleSystemMode(currentMode);
          Serial.printf("[ISR QUEUE EVENT] Mode button (ID %u) pressed at %lu ms -> Mode toggled to: %s\n",
                        evt.buttonId, evt.timestamp, currentMode == SystemMode::AUTOMATIC ? "AUTOMATIC" : "MANUAL");
          xSemaphoreGive(modeMutex);

          if (systemEventGroup != NULL) {
            xEventGroupSetBits(systemEventGroup, EVENT_BIT_MODE_CHANGED);
          }
        }
      } else {
        Serial.printf("[ISR QUEUE EVENT] Actuator ButtonType: %d, ID: %u pressed at %lu ms -> Notifying Controller\n",
                      (int)evt.type, evt.buttonId, evt.timestamp);
        greenhouseController.onButtonPressed(evt.type);

        if (systemEventGroup != NULL) {
          xEventGroupSetBits(systemEventGroup, EVENT_BIT_BUTTON_EVENT);
        }
      }
    }

    // B. Copy latest sensor readings
    SensorDataMap readingsCopy;
    if (xSemaphoreTake(sensorMutex, portMAX_DELAY) == pdTRUE) {
      readingsCopy = globalReadings;
      xSemaphoreGive(sensorMutex);
    }

    // C. Read current system mode safely
    SystemMode mode = SystemMode::MANUAL;
    if (xSemaphoreTake(modeMutex, portMAX_DELAY) == pdTRUE) {
      mode = currentMode;
      xSemaphoreGive(modeMutex);
    }
    bool isAutoMode = (mode == SystemMode::AUTOMATIC);

    // D. Evaluate safety conditions
    SystemHealthState healthState = safetyMonitorService.evaluate(readingsCopy, isAutoMode);

    if (xSemaphoreTake(sensorMutex, portMAX_DELAY) == pdTRUE) {
      globalHealthState = healthState;
      xSemaphoreGive(sensorMutex);
    }

    // E. Mode logging
    if (isAutoMode) {
      handleAutomaticMode(readingsCopy);
    } else {
      handleManualMode();
    }

    // F. Execute automatic/manual control updates
    greenhouseController.update(isAutoMode, readingsCopy, healthState);

    // G. Run periodic task memory diagnostics
    printTaskStackDiagnostics();

    vTaskDelayUntil(&xLastWakeTime, xFrequency);
  }
}

// 3. TaskDisplay: Subscriber 2 -> Listens for SENSOR_READY, MODE_CHANGED, and SAFETY_WARNING on Core 0
void vTaskDisplay(void* pvParameters) {
  (void)pvParameters;
  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xFrequency = pdMS_TO_TICKS(200);

  for (;;) {
    // Check subscribed bits from Event Group
    if (systemEventGroup != NULL) {
      EventBits_t bits = xEventGroupWaitBits(
          systemEventGroup,
          EVENT_BIT_SENSOR_READY | EVENT_BIT_MODE_CHANGED | EVENT_BIT_SAFETY_WARNING,
          pdTRUE,  // Clear bits on exit
          pdFALSE, // Wake on ANY bit
          0        // Non-blocking poll
      );

      if (bits & EVENT_BIT_SENSOR_READY) {
        Serial.println("[EVENT GROUP] vTaskDisplay notified in parallel: EVENT_BIT_SENSOR_READY!");
      }
      if (bits & EVENT_BIT_MODE_CHANGED) {
        Serial.println("[EVENT GROUP] vTaskDisplay notified in parallel: EVENT_BIT_MODE_CHANGED!");
      }
    }

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
  Serial.println("Greenhouse Controller Starting (FreeRTOS Event Groups Mode)...");

  // Create Synchronization Mutexes, Event Queue, and Event Group
  sensorMutex = xSemaphoreCreateMutex();
  modeMutex = xSemaphoreCreateMutex();
  buttonEventQueue = xQueueCreate(10, sizeof(ButtonEvent));
  systemEventGroup = xEventGroupCreate();

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

  // Initialize Hardware Button Drivers & Attach ISRs directly
  btnMode.attachInterruptHandler(buttonEventQueue);
  btnIrrig.attachInterruptHandler(buttonEventQueue);
  btnVent.attachInterruptHandler(buttonEventQueue);
  btnLight.attachInterruptHandler(buttonEventQueue);

  // Initialize Display
  displayManager.init();
  Serial.println("Greenhouse Controller Hardware Ready. Creating FreeRTOS Tasks...");

  // Create FreeRTOS Tasks & Save Handles for Memory Diagnostics
  xTaskCreatePinnedToCore(vTaskSensors, "TaskSensors", 4096, NULL, 2, &hTaskSensors, 1);
  xTaskCreatePinnedToCore(vTaskControl, "TaskControl", 4096, NULL, 3, &hTaskControl, 1);
  xTaskCreatePinnedToCore(vTaskDisplay, "TaskDisplay", 4096, NULL, 1, &hTaskDisplay, 0);

  Serial.println("FreeRTOS Tasks & Hardware ISRs Started Successfully.");
}

void loop() {
  // FreeRTOS scheduler handles tasks. Delete default loop task to reclaim stack memory.
  vTaskDelete(NULL);
}



