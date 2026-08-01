class ButtonDriver {
    public:
        ButtonDriver(int pin) : pin(pin) {
            pinMode(pin, INPUT_PULLUP);
        }

        bool isPressed() {
            return digitalRead(pin) == LOW;
        }

        void toggle() {
            pinMode(pin, OUTPUT);
            digitalWrite(pin, LOW);
            delay(100); // Press duration
            pinMode(pin, INPUT_PULLUP); // Return to input mode
        }

    private:
        int pin;
};