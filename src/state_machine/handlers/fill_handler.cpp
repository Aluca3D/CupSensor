#include "fill_handler.h"

#include "debug_handler.h"
#include "globals.h"
#include "modules/pump_relay.h"
#include "modules/touch_screen.h"
#include "state_machine/state.h"

QueueHandle_t buttonQueue = nullptr;

void sendPressedButton(ButtonID buttonPressed) {
    if (buttonQueue && buttonPressed != BUTTON_COUNT) {
        debugPrint(LOG_INFO, "Sending button pressed %d", buttonPressed);
        xQueueSend(buttonQueue, &buttonPressed, 0);
    }
}

ButtonID receivePressedButton() {
    ButtonID buttonPressed = BUTTON_COUNT;
    if (xQueueReceive(buttonQueue, &buttonPressed, portMAX_DELAY)) {
        debugPrint(LOG_INFO, "Received button pressed %d", buttonPressed);
        return buttonPressed;
    }
    return BUTTON_COUNT;
}

[[noreturn]] void fillCupTask(void *parameter) {
    debugPrint(LOG_INFO, "fillCupTask started on core %d", xPortGetCoreID());
    SystemState lastSeenState = STATE_OFF;
    PumpRelays pumpRelay = PUMP_NONE;

    for (;;) {
        SystemState current = currentState;

        if (uxQueueMessagesWaiting(buttonQueue) > 0) {
            ButtonID buttonPressed = receivePressedButton();

            switch (buttonPressed) {
                case FLUID_0: pumpRelay = PUMP_1;
                    break;
                case FLUID_1: pumpRelay = PUMP_2;
                    break;
                case FLUID_2: pumpRelay = PUMP_3;
                    break;
                default: pumpRelay = PUMP_NONE;
                    break;
            }

            debugPrint(LOG_INFO, "Selected fluid button: %d -> relay %d", buttonPressed, pumpRelay);
        }

        if (current != lastSeenState) {
            lastSeenState = current;

            if (current == STATE_FILLING && pumpRelay != PUMP_NONE) {
                debugPrint(LOG_INFO, "Starting pump %d", pumpRelay);
                startPump(pumpRelay);
                // Remove
                delay(10000);
                sendStateEvent(EVENT_DONE);
            } else if (current == STATE_FINISHED && pumpRelay != PUMP_NONE) {
                debugPrint(LOG_INFO, "Stopping pump %d", pumpRelay);
                stopPump(pumpRelay);
                pumpRelay = PUMP_NONE;
            } else if (current == STATE_ABORT) {
                pumpRelay = PUMP_NONE;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}


void createFillTask() {
    buttonQueue = xQueueCreate(BUTTON_COUNT, sizeof(ButtonID));
    if (!buttonQueue) {
        debugPrint(LOG_ERROR, "Failed to create button queue\n");
        while (true) {
        }
    }

    xTaskCreatePinnedToCore(
        fillCupTask,
        "fillCupTask",
        4096,
        nullptr,
        PRIORITY_HIGH,
        nullptr,
        CORE_ID_0
    );
}
