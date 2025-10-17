#ifndef CUPSENSOR_SCREENHANDLER_H
#define CUPSENSOR_SCREENHANDLER_H

#include <Arduino.h>

enum ScreenState {
    SCREEN_OFF,
    SCREEN_IDLE,
    SCREEN_SCANNING,
    SCREEN_FILLING,
    SCREEN_FINISHED,
    SCREEN_ERROR,
};

extern QueueHandle_t screenEventQueue;

void sendScreenEvent(ScreenState activeScreen);

void createCheckUserInputTask();

void createUpdateScreenTask();

#endif //CUPSENSOR_SCREENHANDLER_H
