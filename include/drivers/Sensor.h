
struct SensorData {
    float value;
    bool isError;
};

class Sensor {
    protected:
        Sensor(int pin) : pin(pin),  lastReadTime(0) {}

    int pin;
        float lastLightLevel;
        unsigned long lastReadTime;
        const unsigned long readInterval = 2000; // Read every 2 seconds


    public:
        virtual ~Sensor() {}
        virtual void init() = 0;
        virtual SensorData read() = 0;
};