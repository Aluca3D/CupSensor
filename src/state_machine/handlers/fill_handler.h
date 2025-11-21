#ifndef CUPSENSOR_FILL_HANDLER_H
#define CUPSENSOR_FILL_HANDLER_H

#include "modules/touch_screen.h"

void sendPressedButton(ButtonID buttonPressed);

ButtonID receivePressedButton();

void createFillTask();

#endif //CUPSENSOR_FILL_HANDLER_H