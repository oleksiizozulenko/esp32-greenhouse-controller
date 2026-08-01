class Actuator {
    protected:
        Actuator(int pin) : pin(pin) {}

    private:
        int pin;

    public:
        virtual ~Actuator() {}
        virtual void turnOn() = 0;
        virtual void turnOff() = 0;
        virtual bool isOn() = 0;
};