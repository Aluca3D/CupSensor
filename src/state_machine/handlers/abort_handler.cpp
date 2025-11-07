#include <Arduino.h>

#include "abort_handler.h"

#include "debug_handler.h"
#include "globals.h"
#include "modules/pump_relay.h"
#include "state_machine/state.h"


[[noreturn]] void abortTask(void *parameters) {
    debugPrint(LOG_INFO, "initializingTask started on core %d", xPortGetCoreID());
    SystemState lastSeenState = STATE_OFF;

    for (;;) {
        const SystemState current = currentState;
        if (current != lastSeenState) {
            lastSeenState = current;

            if (current == STATE_ABORT) {
                debugPrint(LOG_INFO, "Stopping All Pumps");
                stopAllPumps();
            }
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void createAbortTask() {
    xTaskCreatePinnedToCore(
        &abortTask,
        "abortTask",
        2048,
        nullptr,
        PRIORITY_NORMAL,
        nullptr,
        CORE_ID_0
    );
}
