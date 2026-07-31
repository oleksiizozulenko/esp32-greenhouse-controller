class SoilSensor: public ISensor {
    private:
        int pin;
        float lastSoilMoisture;
        unsigned long lastReadTime;
        const unsigned long readInterval = 2000; // Read every 2 seconds

    public:
        SoilSensor(int pin) : pin(pin), lastSoilMoisture(NAN), lastReadTime(0) {}

        void init() {
            pinMode(pin, INPUT);
        }

        SensorData read() {
            unsigned long currentTime = millis();
            if (currentTime - lastReadTime < readInterval) {
                return {lastSoilMoisture, false}; // Return last value if not enough time has passed
            }

            lastReadTime = currentTime;

            // Simulate reading from a soil moisture sensor (replace with actual sensor reading code)
            float soilMoisture = analogRead(pin) * (3.3 / 4095.0) * 100; // Example conversion

            if (isnan(soilMoisture)) {
                return {lastSoilMoisture, true}; // Error reading
            } else {
                lastSoilMoisture = soilMoisture;
                return {soilMoisture, false}; // Successful read
            }
        }
};