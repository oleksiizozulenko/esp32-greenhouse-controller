#include "LightSensor.h"

LightSensor::LightSensor(int pin)
    : Sensor(pin, SensorType::LIGHT, "Light"), lastLightLevel(NAN) {}

void LightSensor::init() {
    pinMode(pin, INPUT);
}

SensorData LightSensor::read() {
    unsigned long currentTime = millis();
    if (currentTime - lastReadTime < readInterval && !isnan(lastLightLevel)) {
        bool isErr = !isfinite(lastLightLevel);
        return {lastLightLevel, isErr};
    }

    lastReadTime = currentTime;

    int rawAdc = analogRead(pin);
    if (rawAdc <= 0) {
        lastLightLevel = 0.0f;
        return {0.0f, false};
    }

    if (rawAdc >= static_cast<int>(ADC_MAX_VALUE)) {
        rawAdc = static_cast<int>(ADC_MAX_VALUE) - 1;
    }

    float ldrResistance = 10000.0f * ((ADC_MAX_VALUE / static_cast<float>(rawAdc)) - 1.0f);
    float lux = pow(250593.5f / ldrResistance, (1.0f / 0.7f));

    if (!isfinite(lux)) {
        lastLightLevel = 0.0f;
        return {0.0f, true};
    } else {
        lastLightLevel = lux;
        return {lux, false};
    }
}

const char* LightSensor::getUnit() const {
    return "lx";
}
