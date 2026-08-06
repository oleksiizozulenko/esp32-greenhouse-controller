
#ifndef SENSOR_H
#define SENSOR_H

#include <Arduino.h>
#include "../config.h"
#include "../filters/ISensorFilter.h"

struct SensorData {
    float value;
    bool isError;
};

class Sensor {
protected:
    int pin;
    SensorType type;
    const char* name;
    unsigned long lastReadTime;
    const unsigned long readInterval;
    ISensorFilter* filter;

    static inline float adcToVoltage(int rawAdc) {
        return (static_cast<float>(rawAdc) / ADC_MAX_VALUE) * ADC_REF_VOLTAGE;
    }

    static inline float adcToPercentage(int rawAdc) {
        return (static_cast<float>(rawAdc) / ADC_MAX_VALUE) * PERCENTAGE_FACTOR;
    }

public:
    Sensor(int pin, SensorType type = SensorType::UNKNOWN, const char* name = "Sensor", unsigned long readInterval = SENSOR_READ_INTERVAL)
        : pin(pin), type(type), name(name), lastReadTime(0), readInterval(readInterval), filter(nullptr) {}

    virtual ~Sensor() {}
    virtual void init() = 0;
    virtual SensorData read() = 0;
    virtual const char* getUnit() const = 0;

    void setFilter(ISensorFilter* newFilter) {
        filter = newFilter;
    }

    ISensorFilter* getFilter() const {
        return filter;
    }

    SensorData readProcessed() {
        SensorData raw = read();
        if (filter != nullptr) {
            FilterResult res = filter->process(raw.value, raw.isError);
            return { res.value, !res.isValid };
        }
        return raw;
    }

    int getPin() const { return pin; }
    SensorType getType() const { return type; }
    const char* getName() const { return name; }
};

typedef Sensor ISensor;

#endif // SENSOR_H