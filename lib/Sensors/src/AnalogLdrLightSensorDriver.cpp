#include "AnalogLdrLightSensorDriver.h"
#include "config.h"
#include <math.h>

AnalogLdrLightSensorDriver::AnalogLdrLightSensorDriver(int analogPin) : pin(analogPin) {}

bool AnalogLdrLightSensorDriver::begin() {
    pinMode(pin, INPUT);
    return true;
}

SensorReadResult<float> AnalogLdrLightSensorDriver::read() {
    uint32_t now = millis();
    int rawAdc = analogRead(pin);

    if (rawAdc <= 0) {
        return {0.0f, SensorStatus::OK, now};
    }

    if (rawAdc >= static_cast<int>(ADC_MAX_VALUE)) {
        rawAdc = static_cast<int>(ADC_MAX_VALUE) - 1;
    }

    float ldrResistance = 10000.0f * ((ADC_MAX_VALUE / static_cast<float>(rawAdc)) - 1.0f);
    float lux = pow(250593.5f / ldrResistance, 1.0f / 0.7f);

    if (!isfinite(lux)) {
        return {0.0f, SensorStatus::Error_HardwareFault, now};
    }

    if (lux < SENSOR_LIGHT_MIN_ERROR || lux > SENSOR_LIGHT_MAX_ERROR) {
        return {lux, SensorStatus::Error_OutOfRange, now};
    }

    return {lux, SensorStatus::OK, now};
}
