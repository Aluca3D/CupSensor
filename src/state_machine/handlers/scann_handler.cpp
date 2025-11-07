#include <Arduino.h>

#include "scann_handler.h"

#include "debug_handler.h"
#include "globals.h"
#include "modules/servo.h"
#include "state_machine/state.h"


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
                delay(2000);
                //sendStateEvent(EVENT_DONE);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

[[noreturn]] void scannFluidHeight(void *parameter) {
    debugPrint(LOG_INFO, "scannFluidHeight started on core %d", xPortGetCoreID());
    SystemState lastSeenState = STATE_OFF;

    for (;;) {
        const SystemState current = currentState;
        if (current != lastSeenState) {
            lastSeenState = current;

            if (current == STATE_SCANNING_FLUID) {
                // TODO:
                delay(2000);
                //sendStateEvent(EVENT_DONE);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

void createScannTask() {
    xTaskCreatePinnedToCore(
        scannCupHeight,
        "scannCupHeight",
        4096,
        nullptr,
        PRIORITY_NORMAL,
        nullptr,
        CORE_ID_0
    );
    xTaskCreatePinnedToCore(
        scannFluidHeight,
        "scannFluidHeight",
        4096,
        nullptr,
        PRIORITY_HIGH,
        nullptr,
        CORE_ID_0
    );
}
