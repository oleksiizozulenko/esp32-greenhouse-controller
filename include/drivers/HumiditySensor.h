class HumiditySensor : public Sensor {


    public:
        HumiditySensor(int pin) : pin(pin), lastHumidity(NAN), lastReadTime(0) {}

        void init() {
            pinMode(pin, INPUT);
        }

        SensorData read() {
            unsigned long currentTime = millis();
            if (currentTime - lastReadTime < readInterval) {
                return {lastHumidity, false}; // Return last value if not enough time has passed
            }

            lastReadTime = currentTime;

            // Simulate reading from a humidity sensor (replace with actual sensor reading code)
            float humidity = analogRead(pin) * (3.3 / 4095.0) * 100; // Example conversion

            if (isnan(humidity)) {
                return {lastHumidity, true}; // Error reading
            } else {
                lastHumidity = humidity;
                return {humidity, false}; // Successful read
            }
        }
};