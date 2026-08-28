#ifndef UNIT_TEST

#include <Arduino.h>
#include <esp_task_wdt.h>
#include "rtos_tasks.h"

#include "ServoVentilationActuator.h"
#include "DotMatrix8x8IrrigationActuator.h"
#include "YellowLedLightActuator.h"

// Hardware Driver Instantiations
DHT dht(PIN_DHT, DHT_TYPE);
HumiditySensor humiditySensor(PIN_DHT, &dht);
SoilSensor soilSensor(PIN_SOIL_POT);
TemperatureSensor temperatureSensor(PIN_TEMP, &dht);
LightSensor lightSensor(PIN_LDR);

ServoVentilationActuator ventActuator(PIN_ACTUATOR_VENT);
DotMatrix8x8IrrigationActuator irrigActuator(PIN_ACTUATOR_IRRIG);
YellowLedLightActuator lightActuator(PIN_ACTUATOR_LIGHT);

ButtonDriver btnMode(PIN_BTN_MODE, ButtonType::MODE);
ButtonDriver btnIrrig(PIN_BTN_IRRIG, ButtonType::IRRIGATION);
ButtonDriver btnVent(PIN_BTN_VENT, ButtonType::VENTILATION);
ButtonDriver btnLight(PIN_BTN_LIGHT, ButtonType::LIGHT);

DisplayManager displayManager;
SensorsService sensorsService;
SafetyMonitorService safetyMonitorService;
GreenhouseController greenhouseController;

void setup() {
  Serial.begin(115200);
  Serial.println("Greenhouse Controller Starting (Modular RTOS Tasks Mode)...");

  esp_task_wdt_init(WDT_TIMEOUT_SECONDS, true);

  initRtosSynchronization();

  // ======================================================
  // STEP 0: Minimal Baseline (All Hardware Disabled)
  // Enable items one by one to verify each component.
  // ======================================================

  // --- 1. SENSORS (Currently Disabled for Bring-Up) ---
  // sensorsService.addSensor(&humiditySensor);
  // sensorsService.addSensor(&soilSensor);
  // sensorsService.addSensor(&temperatureSensor);
  // sensorsService.addSensor(&lightSensor);
  sensorsService.begin();

  // --- 2. ACTUATORS ---
  // greenhouseController.addActuator(&ventActuator);
  greenhouseController.addActuator(&irrigActuator); // STEP 1: Enable 8x8 Matrix Actuator (GPIO 33)
  // greenhouseController.addActuator(&lightActuator);
  greenhouseController.begin();

  // --- 3. BUTTONS ---
  // btnMode.attachInterruptHandler(buttonEventQueue);
  btnIrrig.attachInterruptHandler(buttonEventQueue); // STEP 1: Enable Irrigation Button (GPIO 14)
  // btnVent.attachInterruptHandler(buttonEventQueue);
  // btnLight.attachInterruptHandler(buttonEventQueue);

  displayManager.init();

  Serial.println("Greenhouse Hardware Ready. Spawning FreeRTOS Tasks...");
  startRtosTasks();
  Serial.println("FreeRTOS Tasks & Hardware ISRs Active.");
}

extern "C" void vApplicationStackOverflowHook(TaskHandle_t xTask, char* pcTaskName) {
  (void)xTask;
  Serial.printf("\n[STACK OVERFLOW] CRITICAL ALARM: Task '%s' overflowed its stack!\n", pcTaskName ? pcTaskName : "Unknown");
  Serial.println("[STACK OVERFLOW] Emergency rebooting ESP32 in 1000ms...\n");
  delay(1000);
  ESP.restart();
}

void loop() {
  vTaskDelete(NULL);
}

#endif // UNIT_TEST
