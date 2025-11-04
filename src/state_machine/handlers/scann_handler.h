#ifndef CUPSENSOR_SCANN_HANDLER_H
#define CUPSENSOR_SCANN_HANDLER_H

#include <Arduino.h>

#include "modules/touch_screen.h"

extern QueueHandle_t buttonQueue;

void sendPressedButton(ButtonID buttonPressed);

ButtonID receivePressedButton();

void createScannTask();

#endif //CUPSENSOR_SCANN_HANDLER_H