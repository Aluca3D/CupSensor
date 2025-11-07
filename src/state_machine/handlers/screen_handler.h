#ifndef CUPSENSOR_SCREENHANDLER_H
#define CUPSENSOR_SCREENHANDLER_H

#include <Arduino.h>


extern QueueHandle_t screenEventQueue;

void createUpdateScreenTask();

#endif //CUPSENSOR_SCREENHANDLER_H
