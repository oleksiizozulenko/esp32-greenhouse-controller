#include "MH0080SoilSensorDriver.h"
#include "config.h"


MH0080SoilSensorDriver::MH0080SoilSensorDriver(int analogPin) : pin(analogPin) {}

bool MH0080SoilSensorDriver::begin() {
    pinMode(pin, INPUT);
    return true;
}

SensorReadResult<float> MH0080SoilSensorDriver::read() {
    uint32_t now = millis();
    int rawAdc = analogRead(pin);

    // MH-0080 is inverted: 4095 is dry soil (0%), 0 is wet soil (100%)
    float rawPct = (static_cast<float>(rawAdc) / ADC_MAX_VALUE) * PERCENTAGE_FACTOR;
    float moisturePct = 100.0f - rawPct;

    if (moisturePct < SENSOR_SOIL_MIN_ERROR || moisturePct > SENSOR_SOIL_MAX_ERROR) {
        return {moisturePct, SensorStatus::Error_OutOfRange, now};
    }

    return {moisturePct, SensorStatus::OK, now};
}
