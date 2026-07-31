class TemperatureSensor : public ISensor {
    private:
        int pin;
        float lastTemperature;
        unsigned long lastReadTime;
        const unsigned long readInterval = 2000; // Read every 2 seconds

    public:
        TemperatureSensor(int pin) : pin(pin), lastTemperature(NAN), lastReadTime(0) {}

        void init() {
            pinMode(pin, INPUT);
        }

        SensorData read() {
            unsigned long currentTime = millis();
            if (currentTime - lastReadTime < readInterval) {
                return {lastTemperature, false}; // Return last value if not enough time has passed
            }

            lastReadTime = currentTime;

            // Simulate reading from a temperature sensor (replace with actual sensor reading code)
            float temperature = analogRead(pin) * (3.3 / 4095.0) * 100; // Example conversion

            if (isnan(temperature)) {
                return {lastTemperature, true}; // Error reading
            } else {
                lastTemperature = temperature;
                return {temperature, false}; // Successful read
            }
        }
};