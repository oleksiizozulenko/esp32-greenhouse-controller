class IrrigationActuator : public Actuator {
    public:
        virtual void turnOn() = 0;
        virtual void turnOff() = 0;
        virtual bool isOn() = 0;
};