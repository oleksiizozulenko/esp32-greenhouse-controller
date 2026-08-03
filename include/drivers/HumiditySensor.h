#ifndef HUMIDITY_SENSOR_H
#define HUMIDITY_SENSOR_H

#include <Arduino.h>
#include "Sensor.h"

class HumiditySensor : public Sensor {
private:
    float lastHumidity;

public:
    HumiditySensor(int pin) 
        : Sensor(pin, "Humidity"), lastHumidity(NAN) {}

    void init() override {
        pinMode(pin, INPUT);
    }

    SensorData read() override {
        unsigned long currentTime = millis();
        if (currentTime - lastReadTime < readInterval && !isnan(lastHumidity)) {
            return {lastHumidity, false};
        }

        lastReadTime = currentTime;

        float humidity = adcToPercentage(analogRead(pin));

        if (isnan(humidity)) {
            return {lastHumidity, true};
        } else {
            lastHumidity = humidity;
            return {humidity, false};
        }
    }
};

#endif // HUMIDITY_SENSOR_H