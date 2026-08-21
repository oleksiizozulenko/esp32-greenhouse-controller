#ifndef UNIT_TEST

#include <Arduino.h>
#include "config.h"
#include "Sensor.h"
#include "HumiditySensor.h"
#include "SoilSensor.h"
#include "TemperatureSensor.h"
#include "LightSensor.h"
#include "VentilationActuator.h"
#include "IrrigationActuator.h"
#include "LightActuator.h"
#include "ButtonDriver.h"
#include "SensorsService.h"
#include "SafetyMonitorService.h"
#include "DisplayManager.h"
#include "GreenhouseController.h"
#include <esp_task_wdt.h>

#define WDT_TIMEOUT_SECONDS 5

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
SemaphoreHandle_t modeMutex = NULL;
QueueHandle_t buttonEventQueue = NULL;

// Lock-Free Sensor Data Streaming Queues (Pass-by-Value)
QueueHandle_t controlSensorQueue = NULL;
QueueHandle_t displaySensorQueue = NULL;

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

// Shared Health State Guarded by Mutex
SystemHealthState globalHealthState;
SemaphoreHandle_t healthStateMutex = NULL;

void setGlobalHealthState(const SystemHealthState& state) {
  if (healthStateMutex != NULL && xSemaphoreTake(healthStateMutex, portMAX_DELAY) == pdTRUE) {
    globalHealthState = state;
    xSemaphoreGive(healthStateMutex);
  } else {
    globalHealthState = state;
  }
}

SystemHealthState getGlobalHealthState() {
  SystemHealthState copy;
  if (healthStateMutex != NULL && xSemaphoreTake(healthStateMutex, portMAX_DELAY) == pdTRUE) {
    copy = globalHealthState;
    xSemaphoreGive(healthStateMutex);
  } else {
    copy = globalHealthState;
  }
  return copy;
}

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

// 1. TaskSensors: Samples sensors every 2000ms & streams data via lock-free queues
void vTaskSensors(void* pvParameters) {
  (void)pvParameters;
  esp_task_wdt_add(NULL); // Register vTaskSensors with Task Watchdog Timer

  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xFrequency = pdMS_TO_TICKS(2000);

  for (;;) {
    esp_task_wdt_reset(); // Feed Task Watchdog Timer

    SensorDataMap readings = sensorsService.read();

    // Lock-Free Streaming: Send copies of readings to subscriber queues (Zero Mutex Locks!)
    if (controlSensorQueue != NULL) {
      xQueueSend(controlSensorQueue, &readings, 0);
    }
    if (displaySensorQueue != NULL) {
      xQueueSend(displaySensorQueue, &readings, 0);
    }

    // Publish event: Broadcast SENSOR_READY bit to wake subscribers instantly!
    if (systemEventGroup != NULL) {
      xEventGroupSetBits(systemEventGroup, EVENT_BIT_SENSOR_READY);
    }

    vTaskDelayUntil(&xLastWakeTime, xFrequency);
  }
}

// 2. TaskControl: Subscriber 1 -> Consumes streamed sensor data & processes safety/control loop
void vTaskControl(void* pvParameters) {
  (void)pvParameters;
  esp_task_wdt_add(NULL); // Register vTaskControl with Task Watchdog Timer

  static SensorDataMap lastReadings;

  for (;;) {
    esp_task_wdt_reset(); // Feed Task Watchdog Timer

    // 1. BLOCK until an event occurs (SENSOR_READY, BUTTON_EVENT, SAFETY_WARNING)
    //    OR until fallback timeout of 100ms expires for periodic safety evaluation!
    EventBits_t bits = 0;
    if (systemEventGroup != NULL) {
      bits = xEventGroupWaitBits(
          systemEventGroup,
          EVENT_BIT_SENSOR_READY | EVENT_BIT_BUTTON_EVENT | EVENT_BIT_SAFETY_WARNING,
          pdTRUE,             // Clear bits on exit
          pdFALSE,            // Wake on ANY bit (OR logic)
          pdMS_TO_TICKS(100)  // BLOCK up to 100ms (Wakes INSTANTLY when an event fires!)
      );

      if (bits & EVENT_BIT_SENSOR_READY) {
        Serial.println("[LOCK-FREE STREAM] vTaskControl woken INSTANTLY by fresh sensor data!");
      }
      if (bits & EVENT_BIT_BUTTON_EVENT) {
        Serial.println("[LOCK-FREE STREAM] vTaskControl woken INSTANTLY by button press!");
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

    // B. Consume fresh streamed sensor data (Lock-Free!)
    SensorDataMap newReadings;
    if (controlSensorQueue != NULL && xQueueReceive(controlSensorQueue, &newReadings, 0) == pdTRUE) {
      lastReadings = newReadings;
    }

    // C. Read current system mode safely
    SystemMode mode = SystemMode::MANUAL;
    if (xSemaphoreTake(modeMutex, portMAX_DELAY) == pdTRUE) {
      mode = currentMode;
      xSemaphoreGive(modeMutex);
    }
    bool isAutoMode = (mode == SystemMode::AUTOMATIC);

    // D. Evaluate safety conditions using local readings copy
    SystemHealthState healthState = safetyMonitorService.evaluate(lastReadings, isAutoMode);
    setGlobalHealthState(healthState);

    // E. Mode logging
    if (isAutoMode) {
      handleAutomaticMode(lastReadings);
    } else {
      handleManualMode();
    }

    // F. Execute automatic/manual control updates
    greenhouseController.update(isAutoMode, lastReadings, healthState);

    // G. Run periodic task memory diagnostics
    printTaskStackDiagnostics();
  }
}

// 3. TaskDisplay: Subscriber 2 -> Consumes streamed sensor data & renders OLED on Core 0
void vTaskDisplay(void* pvParameters) {
  (void)pvParameters;
  esp_task_wdt_add(NULL); // Register vTaskDisplay with Task Watchdog Timer

  TickType_t xLastWakeTime = xTaskGetTickCount();
  const TickType_t xFrequency = pdMS_TO_TICKS(200);

  static SensorDataMap lastDisplayReadings;

  for (;;) {
    esp_task_wdt_reset(); // Feed Task Watchdog Timer

    // Check subscribed bits from Event Group
    if (systemEventGroup != NULL) {
      xEventGroupWaitBits(
          systemEventGroup,
          EVENT_BIT_SENSOR_READY | EVENT_BIT_MODE_CHANGED | EVENT_BIT_SAFETY_WARNING,
          pdTRUE,  // Clear bits on exit
          pdFALSE, // Wake on ANY bit
          0        // Non-blocking poll
      );
    }

    // Lock-Free Stream: Consume latest readings from display queue
    SensorDataMap newDisplayReadings;
    if (displaySensorQueue != NULL && xQueueReceive(displaySensorQueue, &newDisplayReadings, 0) == pdTRUE) {
      lastDisplayReadings = newDisplayReadings;
    }

    SystemMode mode = SystemMode::MANUAL;
    if (xSemaphoreTake(modeMutex, portMAX_DELAY) == pdTRUE) {
      mode = currentMode;
      xSemaphoreGive(modeMutex);
    }
    bool isAutoMode = (mode == SystemMode::AUTOMATIC);

    SystemHealthState currentHealth = getGlobalHealthState();
    DisplayViewModel vm = greenhouseController.buildDisplayViewModel(isAutoMode, lastDisplayReadings, currentHealth);
    displayManager.render(vm);

    vTaskDelayUntil(&xLastWakeTime, xFrequency);
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Greenhouse Controller Starting (Lock-Free Sensor Streaming Mode)...");

  // Initialize Task Watchdog Timer (5-second timeout, Panic Reboot = true)
  esp_task_wdt_init(WDT_TIMEOUT_SECONDS, true);

  // Create Lock-Free Sensor Streaming Queues (Depth = 2)
  controlSensorQueue = xQueueCreate(2, sizeof(SensorDataMap));
  displaySensorQueue = xQueueCreate(2, sizeof(SensorDataMap));

  // Create Synchronization Mutexes, Event Queue, and Event Group
  modeMutex = xSemaphoreCreateMutex();
  healthStateMutex = xSemaphoreCreateMutex();
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

  // Create FreeRTOS Tasks with Tuned Stack Allocations & Save Handles for Diagnostics
  xTaskCreatePinnedToCore(vTaskSensors, "TaskSensors", 2048, NULL, 2, &hTaskSensors, 1);
  xTaskCreatePinnedToCore(vTaskControl, "TaskControl", 3072, NULL, 3, &hTaskControl, 1);
  xTaskCreatePinnedToCore(vTaskDisplay, "TaskDisplay", 3072, NULL, 1, &hTaskDisplay, 0);

  Serial.println("FreeRTOS Tasks & Hardware ISRs Started Successfully.");
}

// -------------------------------------------------------------------
// FreeRTOS Stack Overflow Protection Hook
// -------------------------------------------------------------------
extern "C" void vApplicationStackOverflowHook(TaskHandle_t xTask, char* pcTaskName) {
  (void)xTask;
  Serial.printf("\n[STACK OVERFLOW] CRITICAL ALARM: Task '%s' overflowed its stack!\n", pcTaskName ? pcTaskName : "Unknown");
  Serial.println("[STACK OVERFLOW] Emergency rebooting ESP32 in 1000ms...\n");
  delay(1000);
  ESP.restart();
}

void loop() {
  // FreeRTOS scheduler handles tasks. Delete default loop task to reclaim stack memory.
  vTaskDelete(NULL);
}

#endif // UNIT_TEST

