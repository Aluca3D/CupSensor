#include "fill_handler.h"

#include "debug_handler.h"
#include "globals.h"
#include "scann_handler.h"
#include "modules/pump_relay.h"
#include "modules/touch_screen.h"
#include "state_machine/state.h"

QueueHandle_t buttonQueue = nullptr;

void sendPressedButton(ButtonID buttonPressed) {
    if (!buttonQueue) return;

    if (buttonPressed == BUTTON_COUNT) return;

    debugPrint(LOG_INFO, "Attempting sending buttonPressed: %d", buttonPressed);
    if (xQueueSend(buttonQueue, &buttonPressed, pdMS_TO_TICKS(100)) != pdPASS) {
        debugPrint(LOG_WARNING, "Failed to send buttonPressed: %d (queue full)", buttonPressed);
    } else {
        debugPrint(LOG_INFO, "Succeeded in sending buttonPressed: %d", buttonPressed);
    }
}

ButtonID receivePressedButton() {
    if (!buttonQueue) return BUTTON_COUNT;

    ButtonID buttonPressed = BUTTON_COUNT;
    if (xQueueReceive(buttonQueue, &buttonPressed, pdMS_TO_TICKS(100))) {
        debugPrint(LOG_INFO, "Received button pressed %d", buttonPressed);
        return buttonPressed;
    }
    return BUTTON_COUNT;
}

[[noreturn]] void fillCupTask(void *parameter) {
    debugPrint(LOG_INFO, "fillCupTask started on core %d", xPortGetCoreID());
    SystemState lastSeenState = STATE_OFF;
    PumpRelays pumpRelay = PUMP_NONE;
    bool alreadyFull = false;

    for (;;) {
        const SystemState current = getCurrentState();

        const ButtonID buttonPressed = receivePressedButton();
        if (buttonPressed != BUTTON_COUNT) {
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

        const bool cupIsFull = receiveIsCupFull();
        if (cupIsFull && !alreadyFull) {
            sendStateEvent(EVENT_FILL_DONE);
            alreadyFull = true;
        } else if (!cupIsFull) {
            alreadyFull = false;
        }

        if (current != lastSeenState) {
            lastSeenState = current;

            if (current == STATE_SCANNING_FLUID_A_FILLING && pumpRelay != PUMP_NONE) {
                debugPrint(LOG_INFO, "Starting pump %d", pumpRelay);
                startPump(pumpRelay);
            } else if (current == STATE_FINISHED && pumpRelay != PUMP_NONE) {
                debugPrint(LOG_INFO, "Stopping pump %d", pumpRelay);
                stopPump(pumpRelay);
                pumpRelay = PUMP_NONE;
            } else if (
                (current == STATE_ABORT || current == STATE_ERROR || current == STATE_IDLE)
                && pumpRelay != PUMP_NONE
            ) {
                debugPrint(LOG_INFO, "Stopping pump %d", pumpRelay);
                stopPump(pumpRelay);
                pumpRelay = PUMP_NONE;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

void createFillTask() {
    buttonQueue = xQueueCreate(BUTTON_COUNT, sizeof(ButtonID));
    if (!buttonQueue) {
        debugPrint(LOG_WARNING, "Failed to create button queue");
        while (true) {
        }
    }

    xTaskCreatePinnedToCore(
        fillCupTask,
        "fillCupTask",
        STACK_SIZE_LARGE,
        nullptr,
        PRIORITY_HIGH,
        nullptr,
        CORE_ID_0
    );
}
