#ifndef CUPSENSOR_FILL_HANDLER_H
#define CUPSENSOR_FILL_HANDLER_H

#include <Arduino.h>

#include "modules/touch_screen.h"

extern QueueHandle_t buttonQueue;

void sendPressedButton(ButtonID buttonPressed);

ButtonID receivePressedButton();

void createFillTask();

#endif //CUPSENSOR_FILL_HANDLER_H