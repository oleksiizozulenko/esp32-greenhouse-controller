#ifndef UNIT_TEST

#include <Arduino.h>
#include <esp_task_wdt.h>
#include "rtos_tasks.h"
#include "Logging.h"

#include "ServoVentilationActuator.h"
#include "DotMatrix8x8IrrigationActuator.h"
#include "YellowLedLightActuator.h"

// Hardware Driver Instantiations
DHT dht(PIN_DHT, DHT_TYPE);
HumiditySensor humiditySensor(PIN_DHT, &dht);
SoilSensor soilSensor(PIN_SOIL_POT);
TemperatureSensor temperatureSensor(PIN_DHT, &dht);
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

void debugRawDhtPin(int pin) {
  Serial.printf("\n--- [RAW HARDWARE DIAGNOSTIC TEST ON GPIO %d] ---\n", pin);
  
  pinMode(pin, INPUT_PULLUP);
  delay(100);
  int idleState = digitalRead(pin);
  Serial.printf("  1. Idle Voltage State (INPUT_PULLUP): %s (%d)\n", idleState == HIGH ? "HIGH (~3.3V)" : "LOW (~0V)", idleState);

  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
  delay(20);
  
  pinMode(pin, INPUT_PULLUP);
  
  unsigned long timeout = micros();
  while (digitalRead(pin) == HIGH) {
    if (micros() - timeout > 5000) break;
  }
  
  int responseLowTime = 0;
  int responseHighTime = 0;
  unsigned long lowStart = micros();
  if (digitalRead(pin) == LOW) {
    while (digitalRead(pin) == LOW) {
      if (micros() - lowStart > 5000) break;
    }
    responseLowTime = micros() - lowStart;
    
    unsigned long highStart = micros();
    while (digitalRead(pin) == HIGH) {
      if (micros() - highStart > 5000) break;
    }
    responseHighTime = micros() - highStart;
  }
  
  Serial.printf("  2. Start Pulse Sent -> Sensor Response LOW: %d us, HIGH: %d us\n", responseLowTime, responseHighTime);
  if (responseLowTime > 0) {
    Serial.println("  ==> SUCCESS: Sensor is physically responding to start pulse!");
  } else {
    Serial.println("  ==> FAIL: No response pulse detected from sensor.");
  }
  Serial.println("---------------------------------------------------\n");
}

void setup() {
  Serial.begin(115200);
  Serial.println("Greenhouse Controller Starting (Modular RTOS Tasks Mode)...");

  debugRawDhtPin(PIN_DHT);

  esp_task_wdt_init(WDT_TIMEOUT_SECONDS, true);

  initRtosSynchronization();


  // --- 1. SENSORS  ---
  // sensorsService.addSensor(&humiditySensor); // Temporarily disabled (awaiting replacement DHT sensor)
  sensorsService.addSensor(&soilSensor);
  // sensorsService.addSensor(&temperatureSensor); // Temporarily disabled (awaiting replacement DHT sensor)
  sensorsService.addSensor(&lightSensor);
  sensorsService.begin();

  // --- 2. ACTUATORS ---
  greenhouseController.addActuator(&ventActuator);
  greenhouseController.addActuator(&irrigActuator);
  greenhouseController.addActuator(&lightActuator);
  greenhouseController.begin();

  // --- 3. BUTTONS ---
  btnMode.attachInterruptHandler(buttonEventQueue);
  btnIrrig.attachInterruptHandler(buttonEventQueue);
  btnVent.attachInterruptHandler(buttonEventQueue);
  btnLight.attachInterruptHandler(buttonEventQueue);

  displayManager.init();

  esp_log_level_set("*", ESP_LOG_INFO);
  esp_log_level_set("CONTROLLER", ESP_LOG_INFO);
  esp_log_level_set("SAFETY", ESP_LOG_INFO);
  esp_log_level_set("SENSORS", ESP_LOG_INFO);
  esp_log_level_set("CONTROL", ESP_LOG_INFO);
  esp_log_level_set("DIAG", ESP_LOG_DEBUG);

  Serial.println("Greenhouse Hardware Ready. Spawning FreeRTOS Tasks...");
  startRtosTasks();
  Serial.println("FreeRTOS Tasks & Hardware ISRs Active.");
}

extern "C" void vApplicationStackOverflowHook(TaskHandle_t xTask, char* pcTaskName) {
  (void)xTask;
  ESP_LOGE("CRITICAL", "[STACK OVERFLOW] Task '%s' overflowed its stack!", pcTaskName ? pcTaskName : "Unknown");
  delay(1000);
  ESP.restart();
}

void loop() {
  vTaskDelete(NULL);
}

#endif // UNIT_TEST
