#include "drivers/SoilSensor.h"

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

    float soilMoisture = adcToPercentage(analogRead(pin));

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
