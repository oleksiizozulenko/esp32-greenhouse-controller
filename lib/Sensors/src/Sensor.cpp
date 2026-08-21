#include "Sensor.h"

Sensor::Sensor(int pin, SensorType type, const char* name, unsigned long readInterval)
    : pin(pin), type(type), name(name), lastReadTime(0), readInterval(readInterval), filter(nullptr) {}

Sensor::~Sensor() {}

void Sensor::setFilter(ISensorFilter* newFilter) {
    filter = newFilter;
}

ISensorFilter* Sensor::getFilter() const {
    return filter;
}

SensorData Sensor::readProcessed() {
    SensorData raw = read();
    if (filter != nullptr) {
        FilterResult res = filter->process(raw.value, raw.isError);
        return { res.value, !res.isValid };
    }
    return raw;
}
