#include "SoilSensor.h"

SoilSensor::SoilSensor(int pin)
    : Sensor(pin, SensorType::SOIL, "Soil"), lastSoilMoisture(NAN) {}

void SoilSensor::init() {
    pinMode(pin, INPUT);
}

SensorData SoilSensor::read() {
    unsigned long currentTime = millis();
    if (currentTime - lastReadTime < readInterval && !isnan(lastSoilMoisture)) {
        return {lastSoilMoisture, false};
    }

    lastReadTime = currentTime;

    // LM393 analog soil moisture sensors output HIGH (~4095) in air (0% moisture)
    // and LOW (~0) in water (100% moisture).
    float rawPercentage = adcToPercentage(analogRead(pin));
    float soilMoisture = 100.0f - rawPercentage;
    if (soilMoisture < 0.0f) soilMoisture = 0.0f;
    if (soilMoisture > 100.0f) soilMoisture = 100.0f;

    if (isnan(soilMoisture)) {
        return {lastSoilMoisture, true};
    } else {
        lastSoilMoisture = soilMoisture;
        return {soilMoisture, false};
    }
}

const char* SoilSensor::getUnit() const {
    return "%";
}
