#ifndef UNIT_TEST

#include "rtos_tasks.h"
#include "SystemAlertService.h"
#include "Logging.h"
#include <esp_task_wdt.h>

static const char* TAG_CTRL = "CONTROL";
static const char* TAG_SENS = "SENSORS";
static const char* TAG_DIAG = "DIAG";
static const char* TAG_ACT = "ACTUATORS";

static SystemAlertService systemAlertService;

// Inter-task Queues & Synchronization Handles Definitions
QueueHandle_t buttonEventQueue = NULL;
QueueHandle_t controlSensorQueue = NULL;
QueueHandle_t displaySensorQueue = NULL;
SemaphoreHandle_t modeMutex = NULL;
SemaphoreHandle_t healthStateMutex = NULL;
EventGroupHandle_t systemEventGroup = NULL;

// System Mode & State Variables
SystemHealthState globalHealthState;

// Task Handles for Memory Profiling
TaskHandle_t hTaskSensors = NULL;
TaskHandle_t hTaskControl = NULL;
TaskHandle_t hTaskDisplay = NULL;

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
        if (hTaskSensors) {
            UBaseType_t hwmSensors = uxTaskGetStackHighWaterMark(hTaskSensors);
            ESP_LOGD(TAG_DIAG, " [TaskSensors] Free Stack: %u words (%u bytes)",
                     (unsigned int)hwmSensors, (unsigned int)(hwmSensors * sizeof(StackType_t)));
        }
        if (hTaskControl) {
            UBaseType_t hwmControl = uxTaskGetStackHighWaterMark(hTaskControl);
            ESP_LOGD(TAG_DIAG, " [TaskControl] Free Stack: %u words (%u bytes)",
                     (unsigned int)hwmControl, (unsigned int)(hwmControl * sizeof(StackType_t)));
        }
        if (hTaskDisplay) {
            UBaseType_t hwmDisplay = uxTaskGetStackHighWaterMark(hTaskDisplay);
            ESP_LOGD(TAG_DIAG, " [TaskDisplay] Free Stack: %u words (%u bytes)",
                     (unsigned int)hwmDisplay, (unsigned int)(hwmDisplay * sizeof(StackType_t)));
        }
        ESP_LOGD(TAG_DIAG, " [System] Total Heap Free: %u bytes", (unsigned int)ESP.getFreeHeap());
    }
}

void handleManualMode() {
    static unsigned long lastLog = 0;
    if (millis() - lastLog > 5000) {
        ESP_LOGI(TAG_CTRL, "Manual Mode Active");
        lastLog = millis();
    }
}

void handleAutomaticMode(const SensorDataMap& readings) {
    static unsigned long lastLog = 0;
    if (millis() - lastLog > 5000) {
        ESP_LOGI(TAG_CTRL, "Automatic Mode Active");
        for (size_t i = 0; i < readings.size(); ++i) {
            Sensor* sensor = readings[i].sensor;
            SensorData data = readings[i].data;
            if (data.isError) {
                ESP_LOGW(TAG_SENS, "Sensor Error: %s", sensor ? sensor->getName() : "Unknown");
            } else {
                ESP_LOGD(TAG_SENS, "Sensor %s: %.2f", sensor ? sensor->getName() : "Unknown", data.value);
            }
        }
        lastLog = millis();
    }
}

void initRtosSynchronization() {
    controlSensorQueue = xQueueCreate(2, sizeof(SensorDataMap));
    displaySensorQueue = xQueueCreate(2, sizeof(SensorDataMap));

    modeMutex = xSemaphoreCreateMutex();
    healthStateMutex = xSemaphoreCreateMutex();
    buttonEventQueue = xQueueCreate(10, sizeof(ButtonEvent));
    systemEventGroup = xEventGroupCreate();
}

void startRtosTasks() {
    systemAlertService.begin();
    xTaskCreatePinnedToCore(vTaskSensors, "TaskSensors", 4096, NULL, 2, &hTaskSensors, 1);
    xTaskCreatePinnedToCore(vTaskControl, "TaskControl", 3072, NULL, 3, &hTaskControl, 1);
    xTaskCreatePinnedToCore(vTaskDisplay, "TaskDisplay", 3072, NULL, 1, &hTaskDisplay, 0);
}

void vTaskSensors(void* pvParameters) {
    (void)pvParameters;
    esp_task_wdt_add(NULL);

    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(2000);

    for (;;) {
        esp_task_wdt_reset();

        SensorDataMap readings = sensorsService.read();

        for (size_t i = 0; i < readings.size(); ++i) {
            Sensor* s = readings[i].sensor;
            SensorData d = readings[i].data;
            if (s != nullptr) {
                if (d.isError) {
                    ESP_LOGW(TAG_SENS, "  -> %s (Pin %d): [ERROR / NAN]", s->getName(), s->getPin());
                } else {
                    ESP_LOGD(TAG_SENS, "  -> %s (Pin %d): %.2f %s [OK]", s->getName(), s->getPin(), d.value, s->getUnit());
                }
            }
        }

        if (controlSensorQueue != NULL) {
            xQueueSend(controlSensorQueue, &readings, 0);
        }
        if (displaySensorQueue != NULL) {
            xQueueSend(displaySensorQueue, &readings, 0);
        }

        if (systemEventGroup != NULL) {
            xEventGroupSetBits(systemEventGroup, EVENT_BIT_SENSOR_READY);
        }

        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

void vTaskControl(void* pvParameters) {
    (void)pvParameters;
    esp_task_wdt_add(NULL);

    static SensorDataMap lastReadings;

    for (;;) {
        esp_task_wdt_reset();

        EventBits_t bits = 0;
        if (systemEventGroup != NULL) {
            bits = xEventGroupWaitBits(
                systemEventGroup,
                EVENT_BIT_SENSOR_READY | EVENT_BIT_BUTTON_EVENT | EVENT_BIT_SAFETY_WARNING,
                pdTRUE,
                pdFALSE,
                pdMS_TO_TICKS(100)
            );

            if (bits & EVENT_BIT_SENSOR_READY) {
                ESP_LOGD(TAG_CTRL, "vTaskControl woken by sensor data");
            }
            if (bits & EVENT_BIT_BUTTON_EVENT) {
                ESP_LOGD(TAG_CTRL, "vTaskControl woken by button press");
            }
        }

        ButtonEvent evt;
        while (xQueueReceive(buttonEventQueue, &evt, 0) == pdTRUE) {
            if (evt.type == ButtonType::MODE) {
                if (xSemaphoreTake(modeMutex, portMAX_DELAY) == pdTRUE) {
                    SystemMode newMode = toggleSystemMode(greenhouseController.getSystemMode());
                    greenhouseController.setSystemMode(newMode);
                    ESP_LOGI(TAG_CTRL, "[MODE] Button (ID %u) pressed -> Mode toggled to: %s",
                             evt.buttonId, newMode == SystemMode::AUTOMATIC ? "AUTOMATIC" : "MANUAL");
                    xSemaphoreGive(modeMutex);

                    if (systemEventGroup != NULL) {
                        xEventGroupSetBits(systemEventGroup, EVENT_BIT_MODE_CHANGED);
                    }
                }
            } else {
                ESP_LOGI(TAG_CTRL, "[BUTTON] Actuator ButtonType: %d, ID: %u -> Notifying Controller",
                         (int)evt.type, evt.buttonId);
                greenhouseController.onButtonPressed(evt.type);
            }
        }

        SensorDataMap newReadings;
        if (controlSensorQueue != NULL && xQueueReceive(controlSensorQueue, &newReadings, 0) == pdTRUE) {
            lastReadings = newReadings;
        }

        SystemMode mode = SystemMode::AUTOMATIC;
        if (xSemaphoreTake(modeMutex, portMAX_DELAY) == pdTRUE) {
            mode = greenhouseController.getSystemMode();
            xSemaphoreGive(modeMutex);
        }
        bool isAutoMode = (mode == SystemMode::AUTOMATIC);

        SystemHealthState healthState = safetyMonitorService.evaluate(lastReadings, isAutoMode);
        setGlobalHealthState(healthState);

        if (isAutoMode) {
            handleAutomaticMode(lastReadings);
        } else {
            handleManualMode();
        }

        greenhouseController.update(isAutoMode, lastReadings, healthState);
        systemAlertService.update(healthState);

        bool forceLogActuators = (bits & (EVENT_BIT_SENSOR_READY | EVENT_BIT_BUTTON_EVENT)) != 0;
        static bool lastActuatorState[GreenhouseController::MAX_ACTUATORS] = {false};
        static bool actuatorStateInit = false;

        bool actuatorStateChanged = false;
        size_t actCount = greenhouseController.getActuatorCount();
        for (size_t i = 0; i < actCount && i < GreenhouseController::MAX_ACTUATORS; ++i) {
            IActuator* act = greenhouseController.getActuator(i);
            if (act != nullptr) {
                bool op = act->isOperating();
                if (!actuatorStateInit || op != lastActuatorState[i]) {
                    actuatorStateChanged = true;
                    lastActuatorState[i] = op;
                }
            }
        }
        actuatorStateInit = true;

        if (forceLogActuators || actuatorStateChanged) {
            for (size_t i = 0; i < actCount; ++i) {
                IActuator* act = greenhouseController.getActuator(i);
                if (act != nullptr) {
                    ESP_LOGD(TAG_ACT, "  -> %s (Pin %d): %s [%s]",
                             act->getName(),
                             act->getPin(),
                             act->getStatusText(),
                             act->isOperating() ? "ENABLED" : "DISABLED");
                }
            }
        }

        printTaskStackDiagnostics();
    }
}

void vTaskDisplay(void* pvParameters) {
    (void)pvParameters;
    esp_task_wdt_add(NULL);

    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = pdMS_TO_TICKS(200);

    static SensorDataMap lastDisplayReadings;

    for (;;) {
        esp_task_wdt_reset();

        if (systemEventGroup != NULL) {
            xEventGroupWaitBits(
                systemEventGroup,
                EVENT_BIT_SENSOR_READY | EVENT_BIT_MODE_CHANGED | EVENT_BIT_SAFETY_WARNING,
                pdTRUE,
                pdFALSE,
                0
            );
        }

        SensorDataMap newDisplayReadings;
        if (displaySensorQueue != NULL && xQueueReceive(displaySensorQueue, &newDisplayReadings, 0) == pdTRUE) {
            lastDisplayReadings = newDisplayReadings;
        }

        SystemMode mode = SystemMode::AUTOMATIC;
        if (xSemaphoreTake(modeMutex, portMAX_DELAY) == pdTRUE) {
            mode = greenhouseController.getSystemMode();
            xSemaphoreGive(modeMutex);
        }
        bool isAutoMode = (mode == SystemMode::AUTOMATIC);

        SystemHealthState currentHealth = getGlobalHealthState();
        DisplayViewModel vm = greenhouseController.buildDisplayViewModel(isAutoMode, lastDisplayReadings, currentHealth);
        displayManager.render(vm);

        vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

#endif // UNIT_TEST
