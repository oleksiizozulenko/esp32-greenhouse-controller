#include "SensorsService.h"
#include "Logging.h"

static const char* TAG = "SENSORS";

// SensorDataMap methods
SensorDataMap::SensorDataMap() : entryCount(0) {
    for (size_t i = 0; i < MAX_SENSOR_ENTRIES; ++i) {
        entries[i].sensor = nullptr;
        entries[i].data = {0.0f, true};
    }
}

SensorDataMap::SensorDataMap(size_t count) : entryCount(count > MAX_SENSOR_ENTRIES ? MAX_SENSOR_ENTRIES : count) {
    for (size_t i = 0; i < MAX_SENSOR_ENTRIES; ++i) {
        entries[i].sensor = nullptr;
        entries[i].data = {0.0f, true};
    }
}

SensorDataMap::SensorDataMap(const SensorDataMap& other) : entryCount(other.entryCount) {
    for (size_t i = 0; i < MAX_SENSOR_ENTRIES; ++i) {
        entries[i] = other.entries[i];
    }
}

SensorDataMap& SensorDataMap::operator=(const SensorDataMap& other) {
    if (this != &other) {
        entryCount = other.entryCount;
        for (size_t i = 0; i < MAX_SENSOR_ENTRIES; ++i) {
            entries[i] = other.entries[i];
        }
    }
    return *this;
}

SensorDataMap::SensorDataMap(SensorDataMap&& other) noexcept : entryCount(other.entryCount) {
    for (size_t i = 0; i < MAX_SENSOR_ENTRIES; ++i) {
        entries[i] = other.entries[i];
    }
    other.entryCount = 0;
}

SensorDataMap& SensorDataMap::operator=(SensorDataMap&& other) noexcept {
    if (this != &other) {
        entryCount = other.entryCount;
        for (size_t i = 0; i < MAX_SENSOR_ENTRIES; ++i) {
            entries[i] = other.entries[i];
        }
        other.entryCount = 0;
    }
    return *this;
}

SensorData SensorDataMap::get(const Sensor* sensor) const {
    for (size_t i = 0; i < entryCount; ++i) {
        if (entries[i].sensor == sensor) {
            return entries[i].data;
        }
    }
    return {0.0f, true};
}

SensorData SensorDataMap::get(SensorType type) const {
    for (size_t i = 0; i < entryCount; ++i) {
        if (entries[i].sensor != nullptr && entries[i].sensor->getType() == type) {
            return entries[i].data;
        }
    }
    return {0.0f, true};
}

bool SensorDataMap::has(SensorType type) const {
    for (size_t i = 0; i < entryCount; ++i) {
        if (entries[i].sensor != nullptr && entries[i].sensor->getType() == type) {
            return true;
        }
    }
    return false;
}

// SensorsService methods
SensorsService::SensorsService(unsigned long readInterval)
    : sensors{}, sensorCount(0), lastReadTime(0), readInterval(readInterval) {}

SensorsService::SensorsService(size_t initialCapacity, unsigned long readInterval)
    : sensors{}, sensorCount(0), lastReadTime(0), readInterval(readInterval) {
    (void)initialCapacity;
}

SensorsService::SensorsService(Sensor** sensorList, size_t listCount, unsigned long readInterval)
    : sensors{}, sensorCount(0), lastReadTime(0), readInterval(readInterval) {
    for (size_t i = 0; i < listCount; ++i) {
        addSensor(sensorList[i]);
    }
}

bool SensorsService::addSensor(Sensor* sensor) {
    if (sensor == nullptr || sensorCount >= MAX_SENSORS) return false;
    sensors[sensorCount++] = sensor;
    return true;
}

void SensorsService::begin() {
    for (size_t i = 0; i < sensorCount; ++i) {
        if (sensors[i] != nullptr) {
            sensors[i]->init();
        }
    }
}

size_t SensorsService::getSensorCount() const {
    return sensorCount;
}

Sensor* SensorsService::getSensor(size_t index) const {
    if (index < sensorCount) {
        return sensors[index];
    }
    return nullptr;
}

SensorDataMap SensorsService::readAll() {
    SensorDataMap results(sensorCount);
    for (size_t i = 0; i < sensorCount; ++i) {
        results[i].sensor = sensors[i];
        if (sensors[i] != nullptr) {
            results[i].data = sensors[i]->readProcessed();
        } else {
            results[i].data = {0.0f, true};
        }
    }
    return results;
}

SensorDataMap SensorsService::read() {
    return readAll();
}

void SensorsService::checkAndUpdate() {
    unsigned long currentTime = millis();
    if (currentTime - lastReadTime < readInterval) {
        return;
    }
    lastReadTime = currentTime;

    for (size_t i = 0; i < sensorCount; ++i) {
        if (sensors[i] == nullptr) continue;
        SensorData data = sensors[i]->readProcessed();
        if (data.isError) {
            ESP_LOGE(TAG, "Error reading sensor %s", sensors[i]->getName());
        } else {
            ESP_LOGD(TAG, "Sensor %s value: %.2f", sensors[i]->getName(), data.value);
        }
    }
}
