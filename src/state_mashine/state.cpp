#include <Arduino.h>

#include "state.h"
#include "helper/global_variables.h"

volatile SystemState current_state = STATE_OFF;
volatile SystemState last_state = STATE_OFF;
QueueHandle_t state_event_queue = nullptr;

void sendStateEvent(SystemEvent event) {
    if (state_event_queue) {
        // Send event to the queue (non-blocking)
        xQueueSend(state_event_queue, &event, 0);
    }
}

[[noreturn]] void stateMachineTask(void *pvParameters) {
    Serial.printf("State Machine task started on core %d\n", xPortGetCoreID());
    SystemEvent event = EVENT_NONE;

    for (;;) {
        if (xQueueReceive(state_event_queue, &event, portMAX_DELAY)) {
            last_state = current_state;

            switch (current_state) {
                case STATE_OFF:
                    if (event == EVENT_START) {
                        current_state = STATE_INITIALIZING;
                    }
                    break;

                case STATE_INITIALIZING:
                    if (event == EVENT_DONE) {
                        current_state = STATE_IDLE;
                    } else if (event == EVENT_ERROR) {
                        current_state = STATE_ERROR;
                    }
                    break;

                case STATE_IDLE:
                    if (event == EVENT_START) {
                        current_state = STATE_SCANNING_HEIGHT;
                    } else if (event == EVENT_ERROR) {
                        current_state = STATE_ERROR;
                    }
                    break;

                case STATE_SCANNING_HEIGHT:
                    if (event == EVENT_DONE) {
                        current_state = STATE_SCANNING_FLUID;
                    } else if (event == EVENT_ERROR) {
                        current_state = STATE_ERROR;
                    }
                    break;

                case STATE_SCANNING_FLUID:
                    if (event == EVENT_DONE) {
                        current_state = STATE_FILLING;
                    } else if (event == EVENT_ERROR) {
                        current_state = STATE_ERROR;
                    }
                    break;

                case STATE_FILLING:
                    if (event == EVENT_DONE) {
                        current_state = STATE_FINISHED;
                    } else if (event == EVENT_ERROR) {
                        current_state = STATE_ERROR;
                    }
                    break;

                case STATE_FINISHED:
                    if (event == EVENT_STOP) {
                        current_state = STATE_IDLE;
                    }
                    break;

                case STATE_ERROR:
                    if (event == EVENT_STOP) {
                        current_state = STATE_FINISHED;
                    }
                    break;

                default:
                    break;
            }

            // Optional debug log
            Serial.printf("Transition: %d → %d (event=%d)\n", last_state, current_state, event);
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void startStateMachineTask() {
    state_event_queue = xQueueCreate(10, sizeof(SystemEvent));
    if (!state_event_queue) {
        Serial.println("Failed to create state queue!");
        while (true) {
        } // stop
    }

    xTaskCreatePinnedToCore(
        stateMachineTask,
        "StateMachineTask",
        4096,
        nullptr,
        PRIORITY_HIGH,
        nullptr,
        CORE_ID_0
    );
}
