#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

enum class SensorType {
    UNKNOWN = 0,
    TEMPERATURE,
    HUMIDITY,
    SOIL,
    LIGHT
};

enum class ActuatorType {
    UNKNOWN = 0,
    VENTILATION,
    IRRIGATION,
    LIGHT
};

enum class SystemMode {
    MANUAL = 0,
    AUTOMATIC
};

inline SystemMode toggleSystemMode(SystemMode currentMode) {
    return currentMode == SystemMode::AUTOMATIC ? SystemMode::MANUAL : SystemMode::AUTOMATIC;
}

// ==========================================
// 1. PIN DEFINITIONS (Hardware Mapping)
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
#define PIN_LED_RED         4   // System error indicator LED
#define PIN_LED_GREEN       15  // All systems normal indicator LED
#define PIN_ACTUATOR_VENT   5   // Ventilation actuator (Servo)

#define PIN_ACTUATOR_IRRIG  PIN_LED_RING
#define PIN_ACTUATOR_LIGHT  PIN_LED_STRIP

#define NUM_PIXELS_RING     16  // Number of NeoPixels on irrigation ring
#define NUM_PIXELS_STRIP    20  // Number of NeoPixels on light strip

#define SERVO_OPEN_ANGLE    90  // Ventilation servo open angle
#define SERVO_CLOSE_ANGLE   0   // Ventilation servo close angle

// ==========================================
// 2. THRESHOLDS & HYSTERESIS
// ==========================================

// Temperature (Ventilation)
#define TEMP_THRESHOLD_HIGH   28.0f // High temperature threshold to activate ventilation (°C)
#define TEMP_HYSTERESIS       2.0f  // Hysteresis (°C) -> turns off at (28.0 - 2.0 = 26.0°C)

// Air Humidity (Ventilation)
#define HUMIDITY_THRESHOLD_HIGH 70.0f // High air humidity threshold to activate ventilation (%)
#define HUMIDITY_HYSTERESIS     5.0f  // Hysteresis (%) -> turns off at (70.0 - 5.0 = 65.0%)

// Soil Moisture (Irrigation)
#define SOIL_DRY_THRESHOLD    30    // Soil moisture below 30% -> turn on irrigation
#define SOIL_HYSTERESIS       5     // Hysteresis (%) -> turn off at 35%

// Light Intensity (Lux)
#define LIGHT_DARK_THRESHOLD  3000.0f  // Low light / darkness threshold (lx)
#define LIGHT_HYSTERESIS      500.0f   // Light hysteresis (lx) -> turns off at 3500.0 lx

// Sensor Error Thresholds
#define SENSOR_TEMP_MIN_ERROR     -5.0f      // Temperature < -5°C -> Sensor Error
#define SENSOR_TEMP_MAX_ERROR     80.0f      // Temperature > 80°C -> Sensor Error
#define SENSOR_HUMIDITY_MIN_ERROR 0.0f       // Humidity < 0% -> Sensor Error
#define SENSOR_HUMIDITY_MAX_ERROR 90.0f      // Humidity > 90% -> Sensor Error
#define SENSOR_SOIL_MIN_ERROR     0.0f       // Soil moisture < 0% -> Sensor Error
#define SENSOR_SOIL_MAX_ERROR     100.0f     // Soil moisture > 100% -> Sensor Error
#define SENSOR_LIGHT_MIN_ERROR    0.0f       // Min lux
#define SENSOR_LIGHT_MAX_ERROR    100000.0f  // Max lux

// Critical Emergency Thresholds
#define CRITICAL_LIGHT_HIGH       25000.0f   // Critical high light (>25000 lx)
#define CRITICAL_TEMP_HIGH        45.0f      // Critical high temperature (>45°C)
#define CRITICAL_TEMP_LOW         5.0f       // Freezing temperature (<5°C)
#define CRITICAL_HUMIDITY_HIGH    85.0f      // Critical high humidity (>85%)
#define CRITICAL_SOIL_HIGH        85.0f      // Overwatering (>85%)
#define CRITICAL_SOIL_LOW         30.0f      // Dry soil (<30%)

// ==========================================
// 3. SYSTEM TIMINGS
// ==========================================

#define SENSOR_READ_INTERVAL  2000  // Sensor polling interval (ms)
#define OLED_REFRESH_INTERVAL 500   // Display refresh interval (ms)
#define DEBOUNCE_DELAY        50    // Button debounce delay (ms)

// ==========================================
// 4. ADC & CONVERSION CONSTANTS
// ==========================================

#define ADC_MAX_VALUE         4095.0f // 12-bit ADC resolution (2^12 - 1)
#define ADC_REF_VOLTAGE       3.3f    // Reference voltage (V)
#define PERCENTAGE_FACTOR     100.0f  // Percentage scale multiplier

// ==========================================
// 5. SENSOR VERIFICATION & FILTERING CONFIG
// ==========================================

// Digital Bus Integrity
#define DIGITAL_SENSOR_MAX_RETRIES     3      // Max retries on digital bus CRC/ACK read failure
#define DIGITAL_SENSOR_RETRY_DELAY_MS  50     // Delay between retry attempts (ms)
#define DIGITAL_SENSOR_FALLBACK_TIMEOUT 10000 // Retention duration for last valid reading (ms)

// Analog Signal Filtering Default Rates
#define SOIL_MAX_SLEW_PER_SEC          10.0f  // Max soil moisture change rate (%/sec)
#define LIGHT_MAX_SLEW_PER_SEC         2000.0f// Max light intensity change rate (lux/sec)
#define TEMP_MAX_SLEW_PER_SEC          2.0f   // Max temperature change rate (°C/sec)
#define HUMIDITY_MAX_SLEW_PER_SEC      5.0f   // Max air humidity change rate (%/sec)

#endif // CONFIG_H