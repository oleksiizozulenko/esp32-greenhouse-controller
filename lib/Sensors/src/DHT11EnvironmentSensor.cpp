#include "DHT11EnvironmentSensor.h"
#include "config.h"


#ifndef UNIT_TEST
DHT11EnvironmentSensor::DHT11EnvironmentSensor(int dataPin)
    : pin(dataPin), dht(dataPin, DHT11), lastValidTemp(NAN), lastValidHum(NAN), lastReadTime(0) {}

bool DHT11EnvironmentSensor::begin() {
    pinMode(pin, INPUT);
    dht.begin();
    return true;
}

SensorReadResult<float> DHT11EnvironmentSensor::read() {
    uint32_t now = millis();
    float temp = dht.readTemperature();
    if (!isnan(temp)) {
        lastValidTemp = temp;
        return {temp, SensorStatus::OK, now};
    }
    
    if (!isnan(lastValidTemp) && (now - lastReadTime <= DIGITAL_SENSOR_FALLBACK_TIMEOUT)) {
        return {lastValidTemp, SensorStatus::OK, now};
    }

    return {NAN, SensorStatus::Error_HardwareFault, now};
}

SensorReadResult<float> DHT11EnvironmentSensor::readHumidity() {
    uint32_t now = millis();
    float hum = dht.readHumidity();
    if (!isnan(hum)) {
        lastValidHum = hum;
        return {hum, SensorStatus::OK, now};
    }

    if (!isnan(lastValidHum) && (now - lastReadTime <= DIGITAL_SENSOR_FALLBACK_TIMEOUT)) {
        return {lastValidHum, SensorStatus::OK, now};
    }

    return {NAN, SensorStatus::Error_HardwareFault, now};
}
#else
DHT11EnvironmentSensor::DHT11EnvironmentSensor(int dataPin)
    : pin(dataPin), lastValidTemp(24.0f), lastValidHum(55.0f), lastReadTime(0) {}

bool DHT11EnvironmentSensor::begin() { return true; }
SensorReadResult<float> DHT11EnvironmentSensor::read() { return {24.0f, SensorStatus::OK, 1000}; }
SensorReadResult<float> DHT11EnvironmentSensor::readHumidity() { return {55.0f, SensorStatus::OK, 1000}; }
#endif
