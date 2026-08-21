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
        lastLightLevel = 100000.0f;
        return {100000.0f, false};
    }

    if (rawAdc >= static_cast<int>(ADC_MAX_VALUE)) {
        rawAdc = static_cast<int>(ADC_MAX_VALUE) - 1;
    }

    float ldrResistance = 10000.0f * (static_cast<float>(rawAdc) / (ADC_MAX_VALUE - static_cast<float>(rawAdc)));
    float lux = pow((50000.0f * pow(10.0f, 0.7f)) / ldrResistance, (1.0f / 0.7f));

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
