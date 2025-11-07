#include <Arduino.h>

#include "state.h"

#include "globals.h"
#include "handlers/debug_handler.h"

volatile SystemState currentState = STATE_OFF;
volatile SystemState lastState = STATE_OFF;
QueueHandle_t stateEventQueue = nullptr;

void sendStateEvent(SystemEvent event) {
    if (stateEventQueue) {
        xQueueSend(stateEventQueue, &event, 0);
    }
}

[[noreturn]] void stateMachineTask(void *pvParameters) {
    debugPrint(LOG_INFO, "stateMachineTask started on core %d", xPortGetCoreID());
    SystemEvent event = EVENT_NONE;

    for (;;) {
        if (xQueueReceive(stateEventQueue, &event, portMAX_DELAY)) {
            lastState = currentState;

            switch (currentState) {
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
                        currentState = STATE_SCANNING_FLUID;
                    } else if (event == EVENT_STOP) {
                        currentState = STATE_ABORT;
                    } else if (event == EVENT_ERROR) {
                        currentState = STATE_ERROR;
                    }
                    break;

                case STATE_SCANNING_FLUID:
                    if (event == EVENT_DONE) {
                        currentState = STATE_FILLING;
                    } else if (event == EVENT_STOP) {
                        currentState = STATE_ABORT;
                    } else if (event == EVENT_ERROR) {
                        currentState = STATE_ERROR;
                    }
                    break;

                case STATE_FILLING:
                    if (event == EVENT_SCANN) {
                        currentState = STATE_SCANNING_FLUID;
                    } else if (event == EVENT_DONE) {
                        currentState = STATE_FINISHED;
                    } else if (event == EVENT_STOP) {
                        currentState = STATE_ABORT;
                    } else if (event == EVENT_ERROR) {
                        currentState = STATE_ERROR;
                    }
                    break;

                case STATE_RESET_POSITION:
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
        4096,
        nullptr,
        PRIORITY_HIGH,
        nullptr,
        CORE_ID_0
    );
}
