#ifndef HAL_IDISPLAY_H
#define HAL_IDISPLAY_H

#include "CommonTypes.h"


class IDisplay {
public:
    virtual ~IDisplay() = default;
    virtual bool begin() = 0;
    virtual void clear() = 0;
    virtual void setHeader(const char* title) = 0;
    virtual void setField(uint8_t slotIndex, const DisplayField& field) = 0;
    virtual void setStatusLine(const char* statusText) = 0;
    virtual void refresh() = 0;
};

#endif // HAL_IDISPLAY_H
