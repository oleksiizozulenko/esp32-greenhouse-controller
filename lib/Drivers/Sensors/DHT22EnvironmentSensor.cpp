#include "DHT22EnvironmentSensor.h"
#include "config.h"

DHT22EnvironmentSensor::DHT22EnvironmentSensor(int dataPin)
    : pin(dataPin), dht(dataPin, DHT22), lastValidTemp(NAN), lastValidHum(NAN), lastReadTime(0) {}

bool DHT22EnvironmentSensor::begin() {
    pinMode(pin, INPUT);
    dht.begin();
    return true;
}

SensorReadResult<float> DHT22EnvironmentSensor::read() {
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

SensorReadResult<float> DHT22EnvironmentSensor::readHumidity() {
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
