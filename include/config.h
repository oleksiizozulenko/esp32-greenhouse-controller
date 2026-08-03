#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ==========================================
// 1. PIN DEFINITIONS (Прив'язка до схеми)
// ==========================================

// Sensors
#define PIN_DHT           19
#define DHT_TYPE          DHT22
#define PIN_TEMP          19
#define PIN_LDR           35
#define PIN_SOIL_POT      34

// Buttons
#define PIN_BTN_MODE      12
#define PIN_BTN_IRRIG     14
#define PIN_BTN_VENT      27
#define PIN_BTN_LIGHT     26

// Actuators & Indicators
#define PIN_OLED_SDA      21
#define PIN_OLED_SCL      22
#define PIN_BUZZER        18
#define PIN_LED_STRIP     17
#define PIN_LED_RING      16
#define PIN_LED_RED       4
#define PIN_LED_GREEN     15
#define PIN_ACTUATOR_VENT 5

// ==========================================
// 2. THRESHOLDS & HYSTERESIS (Пороги та Гістерезис)
// ==========================================

// Температура (Вентиляція)
#define TEMP_THRESHOLD_HIGH   28.0f // Поріг увімкнення вентиляції (°C)
#define TEMP_HYSTERESIS       2.0f  // Гістерезис (°C) -> вимкнеться при (28 - 2 = 26°C)

// Вологість ґрунту (Полив)
#define SOIL_DRY_THRESHOLD    30    // Нижче 30% вологості -> увімкнути полив
#define SOIL_HYSTERESIS       5     // Гістерезис (%) -> вимкнути при 35%

// Освітленість (Світло)
#define LIGHT_DARK_THRESHOLD  500   // Поріг темряви (значення ADC / lux)
#define LIGHT_HYSTERESIS      50    // Гістерезис для світла

// ==========================================
// 3. SYSTEM TIMINGS (Таймаути та інтервали)
// ==========================================

#define SENSOR_READ_INTERVAL  2000  // Інтервал опитування сенсорів (мс)
#define OLED_REFRESH_INTERVAL 500   // Інтервал оновлення екрану (мс)
#define DEBOUNCE_DELAY        50    // Антидребізг кнопок (мс)

// ==========================================
// 4. ADC & CONVERSION CONSTANTS
// ==========================================

#define ADC_MAX_VALUE         4095.0f // 12-bit ADC resolution (2^12 - 1)
#define ADC_REF_VOLTAGE       3.3f    // Reference voltage (V)
#define PERCENTAGE_FACTOR     100.0f  // Percentage scale multiplier

#endif // CONFIG_H