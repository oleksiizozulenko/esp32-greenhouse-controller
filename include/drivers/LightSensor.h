class LightSensor : public Sensor {
    private:
        float lastLightLevel;

    public:
        LightSensor(int pin) : pin(pin), lastLightLevel(NAN), lastReadTime(0) {}

        void init() {
            pinMode(pin, INPUT);
        }

        SensorData read() {
            unsigned long currentTime = millis();
            if (currentTime - lastReadTime < readInterval) {
                return {lastLightLevel, false}; // Return last value if not enough time has passed
            }

            lastReadTime = currentTime;

            // Simulate reading from a light sensor (replace with actual sensor reading code)
            float lightLevel = analogRead(pin) * (3.3 / 4095.0) * 100; // Example conversion

            if (isnan(lightLevel)) {
                return {lastLightLevel, true}; // Error reading
            } else {
                lastLightLevel = lightLevel;
                return {lightLevel, false}; // Successful read
            }
        }
};