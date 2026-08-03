#ifndef MOCK_DHT_H
#define MOCK_DHT_H

#include <cstdint>
#include <cmath>

#ifndef DHT22
#define DHT22 22
#endif

#ifndef DHT11
#define DHT11 11
#endif

class DHT {
private:
    uint8_t _pin;
    uint8_t _type;
    float _temperature;
    float _humidity;

public:
    DHT(uint8_t pin, uint8_t type)
        : _pin(pin), _type(type), _temperature(25.0f), _humidity(50.0f) {}

    void begin() {}

    float readTemperature(bool S = false, bool force = false) {
        (void)S; (void)force;
        return _temperature;
    }

    float readHumidity(bool force = false) {
        (void)force;
        return _humidity;
    }

    void setTemperature(float temp) {
        _temperature = temp;
    }

    void setHumidity(float hum) {
        _humidity = hum;
    }

    uint8_t getPin() const { return _pin; }
    uint8_t getType() const { return _type; }
};

#endif // MOCK_DHT_H
