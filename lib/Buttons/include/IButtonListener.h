#ifndef I_BUTTON_LISTENER_H
#define I_BUTTON_LISTENER_H

#include "ButtonType.h"

class IButtonListener {
public:
    virtual ~IButtonListener() = default;
    virtual void onButtonPressed(ButtonType button) = 0;
};

#endif // I_BUTTON_LISTENER_H
