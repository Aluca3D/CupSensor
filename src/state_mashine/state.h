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
    STATE_FINISHED,
    STATE_ERROR,
};

enum SystemEvent {
    EVENT_NONE,
    EVENT_START,
    EVENT_DONE,
    EVENT_ERROR,
    EVENT_STOP,
};

extern volatile SystemState currentState;
extern volatile SystemState lastState;

extern QueueHandle_t stateEventQueue;

void startStateMachineTask();
void sendStateEvent(SystemEvent event);

#endif //PROJECT_CUPSENSE_STATE_H