#ifndef SENSORS_SERVICE_H
#define SENSORS_SERVICE_H

#include <Arduino.h>
#include "../drivers/Sensor.h"


static inline bool streq_custom(const char* s1, const char* s2) {
    if (s1 == s2) return true;
    if (!s1 || !s2) return false;
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *s1 == *s2;
}


struct SensorDataEntry {
    Sensor* sensor;
    SensorData data;
};


class SensorDataMap {
private:
    SensorDataEntry* entries;
    size_t entryCount;

public:
    SensorDataMap() : entries(nullptr), entryCount(0) {}

    explicit SensorDataMap(size_t count) : entries(nullptr), entryCount(count) {
        if (entryCount > 0) {
            entries = new SensorDataEntry[entryCount];
            for (size_t i = 0; i < entryCount; ++i) {
                entries[i].sensor = nullptr;
                entries[i].data = {0.0f, true};
            }
        }
    }

    ~SensorDataMap() {
        if (entries != nullptr) {
            delete[] entries;
            entries = nullptr;
        }
    }

    SensorDataMap(const SensorDataMap& other) : entries(nullptr), entryCount(other.entryCount) {
        if (entryCount > 0) {
            entries = new SensorDataEntry[entryCount];
            for (size_t i = 0; i < entryCount; ++i) {
                entries[i] = other.entries[i];
            }
        }
    }

    SensorDataMap& operator=(const SensorDataMap& other) {
        if (this != &other) {
            if (entries != nullptr) {
                delete[] entries;
            }
            entryCount = other.entryCount;
            if (entryCount > 0) {
                entries = new SensorDataEntry[entryCount];
                for (size_t i = 0; i < entryCount; ++i) {
                    entries[i] = other.entries[i];
                }
            } else {
                entries = nullptr;
            }
        }
        return *this;
    }

    SensorDataMap(SensorDataMap&& other) noexcept : entries(other.entries), entryCount(other.entryCount) {
        other.entries = nullptr;
        other.entryCount = 0;
    }

    SensorDataMap& operator=(SensorDataMap&& other) noexcept {
        if (this != &other) {
            if (entries != nullptr) {
                delete[] entries;
            }
            entries = other.entries;
            entryCount = other.entryCount;
            other.entries = nullptr;
            other.entryCount = 0;
        }
        return *this;
    }

    size_t size() const { return entryCount; }
    size_t count() const { return entryCount; }


    SensorDataEntry& operator[](size_t index) { return entries[index]; }
    const SensorDataEntry& operator[](size_t index) const { return entries[index]; }


    SensorData get(const Sensor* sensor) const {
        for (size_t i = 0; i < entryCount; ++i) {
            if (entries[i].sensor == sensor) {
                return entries[i].data;
            }
        }
        return {0.0f, true};
    }

    SensorData get(const char* name) const {
        if (name == nullptr) return {0.0f, true};
        for (size_t i = 0; i < entryCount; ++i) {
            if (entries[i].sensor != nullptr) {
                const char* sName = entries[i].sensor->getName();
                if (streq_custom(sName, name)) {
                    return entries[i].data;
                }
            }
        }
        return {0.0f, true};
    }

    SensorDataEntry* begin() { return entries; }
    SensorDataEntry* end() { return entries + entryCount; }
    const SensorDataEntry* begin() const { return entries; }
    const SensorDataEntry* end() const { return entries + entryCount; }
};

class SensorsService {
private:
    Sensor** sensors;
    size_t capacity;
    size_t sensorCount;
    unsigned long lastReadTime;
    const unsigned long readInterval;

public:
    SensorsService(size_t initialCapacity = 4, unsigned long readInterval = 2000)
        : sensors(nullptr), capacity(0), sensorCount(0), lastReadTime(0), readInterval(readInterval) {
        if (initialCapacity > 0) {
            capacity = initialCapacity;
            sensors = new Sensor*[capacity];
        }
    }

    SensorsService(Sensor** sensorList, size_t listCount, unsigned long readInterval = 2000)
        : sensors(nullptr), capacity(0), sensorCount(0), lastReadTime(0), readInterval(readInterval) {
        for (size_t i = 0; i < listCount; ++i) {
            addSensor(sensorList[i]);
        }
    }

    ~SensorsService() {
        if (sensors != nullptr) {
            delete[] sensors;
            sensors = nullptr;
        }
    }

    SensorsService(const SensorsService&) = delete;
    SensorsService& operator=(const SensorsService&) = delete;

    bool addSensor(Sensor* sensor) {
        if (sensor == nullptr) return false;

        if (sensorCount >= capacity) {
            size_t newCapacity = (capacity == 0) ? 4 : capacity * 2;
            Sensor** newSensors = new Sensor*[newCapacity];
            for (size_t i = 0; i < sensorCount; ++i) {
                newSensors[i] = sensors[i];
            }
            if (sensors != nullptr) {
                delete[] sensors;
            }
            sensors = newSensors;
            capacity = newCapacity;
        }
        sensors[sensorCount++] = sensor;
        return true;
    }

    void begin() {
        for (size_t i = 0; i < sensorCount; ++i) {
            if (sensors[i] != nullptr) {
                sensors[i]->init();
            }
        }
    }

    size_t getSensorCount() const { return sensorCount; }

    Sensor* getSensor(size_t index) const {
        if (index < sensorCount) {
            return sensors[index];
        }
        return nullptr;
    }

    SensorDataMap readAll() {
        SensorDataMap results(sensorCount);
        for (size_t i = 0; i < sensorCount; ++i) {
            results[i].sensor = sensors[i];
            if (sensors[i] != nullptr) {
                results[i].data = sensors[i]->read();
            } else {
                results[i].data = {0.0f, true};
            }
        }
        return results;
    }

    SensorDataMap read() {
        return readAll();
    }

    void checkAndUpdate() {
        unsigned long currentTime = millis();
        if (currentTime - lastReadTime < readInterval) {
            return;
        }
        lastReadTime = currentTime;

        for (size_t i = 0; i < sensorCount; ++i) {
            if (sensors[i] == nullptr) continue;
            SensorData data = sensors[i]->read();
            if (data.isError) {
                Serial.printf("Error reading sensor %s\n", sensors[i]->getName());
            } else {
                Serial.printf("Sensor %s value: %.2f\n", sensors[i]->getName(), data.value);
            }
        }
    }
};

#endif // SENSORS_SERVICE_H
