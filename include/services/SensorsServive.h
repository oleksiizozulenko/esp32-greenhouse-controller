
struct ConnectedSensors {
    ISensor* temperatureSensor;
    ISensor* humiditySensor;
    ISensor* soilSensor;
};

class SensorsService {
    private:
        ConnectedSensors sensors;
        unsigned long lastReadTime;
        const unsigned long readInterval = 2000; // Read every 2 seconds



    public:
        SensorsService(ConnectedSensors sensors)
        : sensors(sensors), lastReadTime(0) {}

        void checkAndUpdate() {
            unsigned long currentTime = millis();
            if (currentTime - lastReadTime < readInterval) {
                return;
            }
            lastReadTime = currentTime;

            for (auto sensor : {sensors.temperatureSensor, sensors.humiditySensor, sensors.soilSensor}) {
                SensorData data = sensor->read();
                if (data.isError) {
                    Serial.println("Error reading from sensor");
                } else {
                    Serial.printf("Sensor value: %.2f\n", data.value);
                }
            }


        }


};