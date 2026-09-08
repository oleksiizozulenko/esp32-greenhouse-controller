#include "HumiditySensor.h"

HumiditySensor::HumiditySensor(int pin, DHT* externalDht, uint8_t dhtType)
    : Sensor(pin, SensorType::HUMIDITY, "Humidity"), dht(nullptr), isExternalDht(false),
      lastValidHumidity(NAN), lastValidTime(0) {
    if (externalDht != nullptr) {
        dht = externalDht;
        isExternalDht = true;
    } else {
        dht = new DHT(pin, dhtType);
        isExternalDht = false;
    }
}

HumiditySensor::~HumiditySensor() {
    if (!isExternalDht && dht != nullptr) {
        delete dht;
        dht = nullptr;
    }
}

void HumiditySensor::init() {
    pinMode(pin, INPUT_PULLUP);
    if (dht != nullptr) {
        dht->begin();
    }
}

SensorData HumiditySensor::read() {
    unsigned long currentTime = millis();
    if (currentTime - lastReadTime < readInterval && !isnan(lastValidHumidity)) {
        bool isErr = isnan(lastValidHumidity) || (lastValidHumidity < SENSOR_HUMIDITY_MIN_ERROR) || (lastValidHumidity > SENSOR_HUMIDITY_MAX_ERROR);
        return {lastValidHumidity, isErr};
    }

    lastReadTime = currentTime;

    float humidity = dht ? dht->readHumidity() : NAN;
    if (!isnan(humidity)) {
        lastValidHumidity = humidity;
        lastValidTime = currentTime;
        return {humidity, false};
    }

    if (!isnan(lastValidHumidity) && (currentTime - lastValidTime <= DIGITAL_SENSOR_FALLBACK_TIMEOUT)) {
        return {lastValidHumidity, false};
    }

    return {NAN, true};
}

const char* HumiditySensor::getUnit() const {
    return "%";
}
