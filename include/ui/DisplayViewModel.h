#ifndef DISPLAY_VIEW_MODEL_H
#define DISPLAY_VIEW_MODEL_H

#include <Arduino.h>

struct DisplayViewModel {
    char modeText[8];       // "AUTO" or "MANUAL"
    char healthStatus[8];   // "[OK]" or "[ERR]"
    
    struct LineItem {
        char label[8];      // e.g. "Temp:"
        char value[10];     // e.g. "25.4C" or "ERR"
    };

    LineItem sensors[4];
    size_t sensorCount;

    LineItem actuators[4];
    size_t actuatorCount;

    char advisoryBanner[24]; // Max 23 chars + null (safely fits "TEMP HIGH! Press VENT")
};

#endif // DISPLAY_VIEW_MODEL_H
