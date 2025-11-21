#include <Arduino.h>

#include "state.h"

#include "globals.h"
#include "handlers/debug_handler.h"

volatile SystemState currentState = STATE_OFF;
volatile SystemState lastState = STATE_OFF;
QueueHandle_t stateEventQueue = nullptr;

void sendStateEvent(SystemEvent event) {
    if (!stateEventQueue) return;

    debugPrint(LOG_INFO, "Attempting sending event: %d", event);
    if (xQueueSend(stateEventQueue, &event, pdMS_TO_TICKS(100)) != pdPASS) {
        debugPrint(LOG_WARNING, "Failed to send event: %d (queue full)", event);
    } else {
        debugPrint(LOG_INFO, "Succeeded in sending event: %d", event);
    }
}

SystemEvent receiveStateEvent() {
    if (!stateEventQueue) return EVENT_NONE;

    SystemEvent event = EVENT_NONE;
    if (xQueueReceive(stateEventQueue, &event, pdMS_TO_TICKS(100))) {
        debugPrint(LOG_INFO, "Received event: %d", event);
        return event;
    }

    return EVENT_NONE;
}

void handleState(SystemEvent event, SystemState state) {
    switch (state) {
        case STATE_OFF:
            if (event == EVENT_START) {
                currentState = STATE_INITIALIZING;
            }
            break;

        case STATE_INITIALIZING:
            if (event == EVENT_DONE) {
                currentState = STATE_IDLE;
            } else if (event == EVENT_ERROR) {
                currentState = STATE_ERROR;
            }
            break;

        case STATE_IDLE:
            if (event == EVENT_START) {
                currentState = STATE_SCANNING_HEIGHT;
            } else if (event == EVENT_ERROR) {
                currentState = STATE_ERROR;
            }
            break;

        case STATE_SCANNING_HEIGHT:
            if (event == EVENT_DONE) {
                currentState = STATE_SCANNING_FLUID_A_FILLING;
            } else if (event == EVENT_STOP) {
                currentState = STATE_ABORT;
            } else if (event == EVENT_ERROR) {
                currentState = STATE_ERROR;
            }
            break;

        case STATE_SCANNING_FLUID_A_FILLING:
            if (event == EVENT_DONE) {
                currentState = STATE_FINISHED;
            } else if (event == EVENT_STOP) {
                currentState = STATE_ABORT;
            } else if (event == EVENT_ERROR) {
                currentState = STATE_ERROR;
            }
            break;

        case STATE_ABORT:
        case STATE_ERROR:
        case STATE_FINISHED:
            if (event == EVENT_DONE) {
                currentState = STATE_IDLE;
            }
            break;

        default:
            break;
    }
}

[[noreturn]] void stateMachineTask(void *pvParameters) {
    debugPrint(LOG_INFO, "stateMachineTask started on core %d", xPortGetCoreID());

    for (;;) {
        const SystemEvent event = receiveStateEvent();
        if (event != EVENT_NONE) {
            lastState = currentState;

            handleState(event, currentState);

            debugPrint(LOG_DEBUG, "Transition State: %d → %d (event=%d)", lastState, currentState, event);
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void createStateMachineTask() {
    stateEventQueue = xQueueCreate(10, sizeof(SystemEvent));
    if (!stateEventQueue) {
        debugPrint(LOG_WARNING, "Failed to create state queue!");
        while (true) {
        }
    }

    xTaskCreatePinnedToCore(
        stateMachineTask,
        "stateMachineTask",
        STACK_SIZE_LARGE,
        nullptr,
        PRIORITY_HIGH,
        nullptr,
        CORE_ID_0
    );
}
