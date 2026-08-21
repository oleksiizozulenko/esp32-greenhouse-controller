#ifndef RTOS_TASKS_H
#define RTOS_TASKS_H

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

#define WDT_TIMEOUT_SECONDS 5

// FreeRTOS Event Group Bitmasks
#define EVENT_BIT_SENSOR_READY    (1 << 0)
#define EVENT_BIT_BUTTON_EVENT    (1 << 1)
#define EVENT_BIT_SAFETY_WARNING  (1 << 2)
#define EVENT_BIT_MODE_CHANGED    (1 << 3)

// Global Drivers & Services References
extern SensorsService sensorsService;
extern SafetyMonitorService safetyMonitorService;
extern GreenhouseController greenhouseController;
extern DisplayManager displayManager;

extern ButtonDriver btnMode;
extern ButtonDriver btnIrrig;
extern ButtonDriver btnVent;
extern ButtonDriver btnLight;

// Inter-task Queues & Synchronization Handles
extern QueueHandle_t buttonEventQueue;
extern QueueHandle_t controlSensorQueue;
extern QueueHandle_t displaySensorQueue;
extern SemaphoreHandle_t modeMutex;
extern SemaphoreHandle_t healthStateMutex;
extern EventGroupHandle_t systemEventGroup;

// Shared State Accessors
void setGlobalHealthState(const SystemHealthState& state);
SystemHealthState getGlobalHealthState();

// Task Prototypes
void vTaskSensors(void* pvParameters);
void vTaskControl(void* pvParameters);
void vTaskDisplay(void* pvParameters);

// Task Orchestration Initialization
void initRtosSynchronization();
void startRtosTasks();
void printTaskStackDiagnostics();

#endif // RTOS_TASKS_H
