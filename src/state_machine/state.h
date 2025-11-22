#ifndef PROJECT_CUPSENSE_STATE_H
#define PROJECT_CUPSENSE_STATE_H

enum SystemState {
    STATE_OFF,
    STATE_INITIALIZING,
    STATE_IDLE,
    STATE_SCANNING_HEIGHT,
    STATE_SCANNING_FLUID_A_FILLING,
    STATE_ERROR,
    STATE_ABORT,
    STATE_FINISHED,
};

enum SystemEvent {
    EVENT_NONE,
    EVENT_START,
    EVENT_DONE,
    EVENT_FILL_DONE,
    EVENT_SCANN,
    EVENT_ERROR,
    EVENT_STOP,
};

SystemState getCurrentState();

SystemState getlastState();

void sendStateEvent(SystemEvent event);

void createStateMachineTask();

#endif //PROJECT_CUPSENSE_STATE_H
