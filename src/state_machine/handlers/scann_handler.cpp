#include <Arduino.h>

#include "scann_handler.h"

#include "debug_handler.h"
#include "globals.h"
#include "state_machine/state.h"

QueueHandle_t cupFinishedFillingQueue = nullptr;

void sendIsCupFull(bool cupIsFull) {
    if (!cupFinishedFillingQueue) return;

    debugPrint(LOG_INFO, "Attempting sending cupIsFull: %d", cupIsFull);
    if (xQueueSend(cupFinishedFillingQueue, &cupIsFull, pdMS_TO_TICKS(100)) != pdPASS) {
        debugPrint(LOG_WARNING, "Failed to send cupIsFull (queue full)");
    } else {
        debugPrint(LOG_INFO, "Succeeded in sending cupIsFull: %d", cupIsFull);
    }
}

bool receiveIsCupFull() {
    if (!cupFinishedFillingQueue) return false;

    bool cupIsFull = false;
    if (xQueueReceive(cupFinishedFillingQueue, &cupIsFull, pdMS_TO_TICKS(100))) {
        debugPrint(LOG_INFO, "Received cupIsFull: %d", cupIsFull);
        return cupIsFull;
    }
    return false;
}

[[noreturn]] void scannCupHeight(void *parameter) {
    debugPrint(LOG_INFO, "scannCupHeight started on core %d", xPortGetCoreID());
    SystemState lastSeenState = STATE_OFF;

    for (;;) {
        const SystemState current = currentState;
        if (current != lastSeenState) {
            lastSeenState = current;

            if (current == STATE_SCANNING_HEIGHT) {
                //if (!getIsMoving() && currentState != STATE_INITIALIZING) {
                //    vTaskDelay(pdMS_TO_TICKS(1000)); // wait before reversing
                //    if (getDistanceMoved() <= 0.05) {
                //        servoMoveToo(10.0f);
                //    } else {
                //        servoMoveToo(0.0f);
                //    }
                //}

                // Todo: Write Scann Height Logic
                delay(4000);
                sendStateEvent(EVENT_DONE);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }
    vTaskDelete(nullptr);
}

[[noreturn]] void scannFluidHeight(void *parameter) {
    debugPrint(LOG_INFO, "scannFluidHeight started on core %d", xPortGetCoreID());
    SystemState lastSeenState = STATE_OFF;

    for (;;) {
        const SystemState current = currentState;
        if (current != lastSeenState) {
            lastSeenState = current;

            if (current == STATE_SCANNING_FLUID_A_FILLING) {
                // TODO: Checker Logic that checks CupHeight and Fluid Level
                sendIsCupFull(true);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }
    vTaskDelete(nullptr);
}

void createScannTask() {
    cupFinishedFillingQueue = xQueueCreate(4, sizeof(bool));
    if (cupFinishedFillingQueue == nullptr) {
        debugPrint(LOG_ERROR, "Failed to create cupFinishedFillingQueue");
        while (true) {
        }
    }

    xTaskCreatePinnedToCore(
        scannCupHeight,
        "scannCupHeight",
        STACK_SIZE_LARGE,
        nullptr,
        PRIORITY_NORMAL,
        nullptr,
        CORE_ID_0
    );
    xTaskCreatePinnedToCore(
        scannFluidHeight,
        "scannFluidHeight",
        STACK_SIZE_LARGE,
        nullptr,
        PRIORITY_HIGH,
        nullptr,
        CORE_ID_0
    );
}
