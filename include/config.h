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
#define PIN_OLED_SDA        21
#define PIN_OLED_SCL        22
#define PIN_BUZZER          18
#define PIN_LED_STRIP       17  // Light actuator (LED Strip)
#define PIN_LED_RING        16  // Irrigation actuator (LED Ring)
#define PIN_LED_RED         4   // Error in system LED
#define PIN_LED_GREEN       15  // All systems working LED
#define PIN_ACTUATOR_VENT   5   // Ventilation actuator (Servo)

#define PIN_ACTUATOR_IRRIG  PIN_LED_RING
#define PIN_ACTUATOR_LIGHT  PIN_LED_STRIP

#define NUM_PIXELS_RING     16  // Number of NeoPixels on irrigation ring
#define NUM_PIXELS_STRIP    20  // Number of NeoPixels on light strip

#define SERVO_OPEN_ANGLE    90  // Ventilation servo open angle
#define SERVO_CLOSE_ANGLE   0   // Ventilation servo close angle

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

// Пороги помилок сенсорів (Sensor Error Thresholds)
#define SENSOR_LIGHT_MAX_ERROR   100.0f // Максимальне значення світла (насичення / помилка)
#define SENSOR_HUMIDITY_MAX_ERROR 90.0f  // Вологість > 90% -> Помилка системи
#define SENSOR_TEMP_MIN_ERROR     -5.0f  // Температура < -5°C -> Помилка системи

// Пороги критичних ситуацій у Manual Mode (Critical Emergency Thresholds)
#define CRITICAL_LIGHT_HIGH       10000.0f // Критична освітленість (>10000 lumen/lux)
#define CRITICAL_TEMP_HIGH        60.0f    // Критична температура (>60°C)
#define CRITICAL_HUMIDITY_HIGH    85.0f    // Критична вологість (>85%)
#define CRITICAL_SOIL_HIGH        85.0f    // Критична вологість ґрунту / потенціометра (>85%)

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