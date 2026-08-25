#include "AnalogSoilSensorDriver.h"
#include "config.h"

AnalogSoilSensorDriver::AnalogSoilSensorDriver(int analogPin) : pin(analogPin) {}

bool AnalogSoilSensorDriver::begin() {
    pinMode(pin, INPUT);
    return true;
}

SensorReadResult<float> AnalogSoilSensorDriver::read() {
    uint32_t now = millis();
    int rawAdc = analogRead(pin);
    float moisturePct = (static_cast<float>(rawAdc) / ADC_MAX_VALUE) * PERCENTAGE_FACTOR;

    if (moisturePct < SENSOR_SOIL_MIN_ERROR || moisturePct > SENSOR_SOIL_MAX_ERROR) {
        return {moisturePct, SensorStatus::Error_OutOfRange, now};
    }

    return {moisturePct, SensorStatus::OK, now};
}
