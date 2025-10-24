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

enum ScreenEvent {
    SCREEN_EVENT_NONE,
    SCREEN_EVENT_START,
    SCREEN_EVENT_DONE,
    SCREEN_EVENT_ERROR,
    SCREEN_EVENT_STOP,
};

extern QueueHandle_t screenEventQueue;
extern volatile ScreenState currentScreen;
extern volatile ScreenState lastScreen;

void sendScreenEvent(ScreenEvent event);

void createUpdateScreenTask();

#endif //CUPSENSOR_SCREENHANDLER_H
