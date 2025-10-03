#include <Arduino.h>

#include "initializing.h"
#include "helper/global_variables.h"
#include "state_mashine/state.h"

[[noreturn]] void initializing_task(void *parameters) {
    Serial.printf("Init task started on core %d\n", xPortGetCoreID());

    SystemState last_seen_state = STATE_OFF;

    for (;;) {
        SystemState current = current_state;
        if (current != last_seen_state) {
            last_seen_state = current;

            if (current == STATE_INITIALIZING) {
                // Stuff that needs to be done on init.
                sendStateEvent(EVENT_DONE);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void create_init_task() {
    xTaskCreatePinnedToCore(
        &initializing_task,
        "initializing_task",
        2048,
        nullptr,
        PRIORITY_NORMAL,
        nullptr,
        CORE_ID_0
    );
}
