#ifndef PROJECT_CUPSENSE_STATE_H
#define PROJECT_CUPSENSE_STATE_H
#include <Arduino.h>

enum SystemState {
    STATE_OFF,
    STATE_INITIALIZING,
    STATE_IDLE,
    STATE_SCANNING_HEIGHT,
    STATE_RESET_POSITION,
    STATE_SCANNING_FLUID,
    STATE_FILLING,
    STATE_ERROR,
    STATE_ABORT,
    STATE_FINISHED,
};

enum SystemEvent {
    EVENT_NONE,
    EVENT_START,
    EVENT_DONE,
    EVENT_SCANN,
    EVENT_ERROR,
    EVENT_STOP,
};

extern volatile SystemState currentState;
extern volatile SystemState lastState;

extern QueueHandle_t stateEventQueue;

void sendStateEvent(SystemEvent event);

void createStateMachineTask();


#endif //PROJECT_CUPSENSE_STATE_H
