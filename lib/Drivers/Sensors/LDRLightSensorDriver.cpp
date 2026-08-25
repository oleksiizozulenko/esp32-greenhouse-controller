#include "LDRLightSensorDriver.h"
#include "config.h"

LDRLightSensorDriver::LDRLightSensorDriver(int analogPin) : pin(analogPin) {}

bool LDRLightSensorDriver::begin() {
    pinMode(pin, INPUT);
    return true;
}

SensorReadResult<float> LDRLightSensorDriver::read() {
    uint32_t now = millis();
    int rawAdc = analogRead(pin);
    float voltage = (static_cast<float>(rawAdc) / ADC_MAX_VALUE) * ADC_REF_VOLTAGE;
    float lux = voltage * 3000.0f; // Approx conversion

    if (lux < SENSOR_LIGHT_MIN_ERROR || lux > SENSOR_LIGHT_MAX_ERROR) {
        return {lux, SensorStatus::Error_OutOfRange, now};
    }

    return {lux, SensorStatus::OK, now};
}
