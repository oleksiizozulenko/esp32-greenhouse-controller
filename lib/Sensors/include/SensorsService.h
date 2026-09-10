#ifndef SENSORS_SERVICE_H
#define SENSORS_SERVICE_H

#include <Arduino.h>
#include "Sensor.h"

static const size_t MAX_SENSOR_ENTRIES = 16;

struct SensorDataEntry {
    Sensor* sensor;
    SensorData data;
};

class SensorDataMap {
private:
    SensorDataEntry entries[MAX_SENSOR_ENTRIES];
    size_t entryCount;

public:
    SensorDataMap();
    explicit SensorDataMap(size_t count);
    ~SensorDataMap() = default;

    SensorDataMap(const SensorDataMap& other);
    SensorDataMap& operator=(const SensorDataMap& other);
    SensorDataMap(SensorDataMap&& other) noexcept;
    SensorDataMap& operator=(SensorDataMap&& other) noexcept;

    size_t size() const { return entryCount; }
    size_t count() const { return entryCount; }

    SensorDataEntry& operator[](size_t index) { return entries[index]; }
    const SensorDataEntry& operator[](size_t index) const { return entries[index]; }

    SensorData get(const Sensor* sensor) const;
    SensorData get(SensorType type) const;
    bool has(SensorType type) const;

    SensorDataEntry* begin() { return entries; }
    SensorDataEntry* end() { return entries + entryCount; }
    const SensorDataEntry* begin() const { return entries; }
    const SensorDataEntry* end() const { return entries + entryCount; }
};

class SensorsService {
public:
    static constexpr size_t MAX_SENSORS = 16;

private:
    Sensor* sensors[MAX_SENSORS];
    size_t sensorCount;
    unsigned long lastReadTime;
    const unsigned long readInterval;

public:
    explicit SensorsService(unsigned long readInterval = 2000);
    SensorsService(size_t initialCapacity, unsigned long readInterval);
    SensorsService(Sensor** sensorList, size_t listCount, unsigned long readInterval = 2000);
    ~SensorsService() = default;

    SensorsService(const SensorsService&) = delete;
    SensorsService& operator=(const SensorsService&) = delete;

    bool addSensor(Sensor* sensor);
    void begin();

    size_t getSensorCount() const;
    Sensor* getSensor(size_t index) const;

    SensorDataMap readAll();
    SensorDataMap read();

    void checkAndUpdate();
};

#endif // SENSORS_SERVICE_H
