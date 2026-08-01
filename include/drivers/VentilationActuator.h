class VentilationActuator: public Actuator {
    public:
        VentilationActuator(int pin) : pin(pin) {
            pinMode(pin, OUTPUT);
        }

        void open() {
            digitalWrite(pin, HIGH);
        }

        void close() {
            digitalWrite(pin, LOW);
        }

    private:
        int pin;
};