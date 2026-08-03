#ifndef MOCK_ARDUINO_H
#define MOCK_ARDUINO_H

#include <iostream>
#include <cstdio>
#include <cstdarg>
#include <cstdint>
#include <cstring>
#include <map>

#ifndef HIGH
#define HIGH 0x1
#endif

#ifndef LOW
#define LOW 0x0
#endif

#ifndef INPUT
#define INPUT 0x0
#endif

#ifndef OUTPUT
#define OUTPUT 0x1
#endif

#ifndef INPUT_PULLUP
#define INPUT_PULLUP 0x2
#endif

#ifndef DHT22
#define DHT22 22
#endif

struct MockArduinoState {
    std::map<int, int> pinModes;
    std::map<int, int> pinValues;
    std::map<int, unsigned int> buzzerTones;
    unsigned long simulatedMillis = 0;
};

inline MockArduinoState& getMockArduinoState() {
    static MockArduinoState state;
    return state;
}

inline void resetMockArduinoState() {
    getMockArduinoState().pinModes.clear();
    getMockArduinoState().pinValues.clear();
    getMockArduinoState().buzzerTones.clear();
    getMockArduinoState().simulatedMillis = 0;
}

inline void setMockPinValue(int pin, int value) {
    getMockArduinoState().pinValues[pin] = value;
}

inline int getMockPinValue(int pin) {
    auto it = getMockArduinoState().pinValues.find(pin);
    if (it != getMockArduinoState().pinValues.end()) {
        return it->second;
    }
    return LOW;
}

inline int getMockPinMode(int pin) {
    auto it = getMockArduinoState().pinModes.find(pin);
    if (it != getMockArduinoState().pinModes.end()) {
        return it->second;
    }
    return INPUT;
}

inline unsigned int getMockBuzzerTone(int pin) {
    auto it = getMockArduinoState().buzzerTones.find(pin);
    if (it != getMockArduinoState().buzzerTones.end()) {
        return it->second;
    }
    return 0;
}

inline void setSimulatedMillis(unsigned long ms) {
    getMockArduinoState().simulatedMillis = ms;
}

inline void advanceSimulatedMillis(unsigned long ms) {
    getMockArduinoState().simulatedMillis += ms;
}

inline unsigned long millis() {
    return getMockArduinoState().simulatedMillis;
}

inline void delay(unsigned long ms) {
    getMockArduinoState().simulatedMillis += ms;
}

inline void pinMode(int pin, int mode) {
    getMockArduinoState().pinModes[pin] = mode;
}

inline void digitalWrite(int pin, int val) {
    getMockArduinoState().pinValues[pin] = val;
}

inline int digitalRead(int pin) {
    auto it = getMockArduinoState().pinValues.find(pin);
    if (it != getMockArduinoState().pinValues.end()) {
        return it->second;
    }
    return HIGH; // default HIGH for INPUT_PULLUP
}

inline int analogRead(int pin) {
    auto it = getMockArduinoState().pinValues.find(pin);
    if (it != getMockArduinoState().pinValues.end()) {
        return it->second;
    }
    return 0;
}

inline void setMockAnalogRead(int pin, int value) {
    getMockArduinoState().pinValues[pin] = value;
}

inline void tone(int pin, unsigned int frequency, unsigned long duration = 0) {
    (void)duration;
    getMockArduinoState().buzzerTones[pin] = frequency;
}

inline void noTone(int pin) {
    getMockArduinoState().buzzerTones[pin] = 0;
}

class MockSerial {
public:
    void begin(unsigned long baud) { (void)baud; }

    int printf(const char* format, ...) {
        va_list args;
        va_start(args, format);
        int result = vprintf(format, args);
        va_end(args);
        return result;
    }

    size_t print(const char* str) {
        std::cout << str;
        return strlen(str);
    }

    size_t println(const char* str = "") {
        std::cout << str << std::endl;
        return strlen(str) + 1;
    }

    size_t print(int val) {
        std::cout << val;
        return 1;
    }

    size_t println(int val) {
        std::cout << val << std::endl;
        return 1;
    }

    size_t print(float val) {
        std::cout << val;
        return 1;
    }

    size_t println(float val) {
        std::cout << val << std::endl;
        return 1;
    }
};

extern MockSerial Serial;

#endif // MOCK_ARDUINO_H
