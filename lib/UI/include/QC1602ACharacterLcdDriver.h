#ifndef QC1602A_CHARACTER_LCD_DRIVER_H
#define QC1602A_CHARACTER_LCD_DRIVER_H

#include <Arduino.h>
#include "IDisplay.h"

class QC1602ACharacterLcdDriver : public IDisplay {
private:
    int rsPin, enPin, d4Pin, d5Pin, d6Pin, d7Pin;
    char line0Text[17];
    char line1Text[17];

public:
    QC1602ACharacterLcdDriver(int rs = 13, int en = 12, int d4 = 14, int d5 = 27, int d6 = 26, int d7 = 25);
    ~QC1602ACharacterLcdDriver() override = default;

    bool begin() override;
    void clear() override;
    void setHeader(const char* title) override;
    void setField(uint8_t slotIndex, const DisplayField& field) override;
    void setStatusLine(const char* statusText) override;
    void refresh() override;
};

#endif // QC1602A_CHARACTER_LCD_DRIVER_H
