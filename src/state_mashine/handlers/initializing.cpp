#include <Arduino.h>

#include "initializing.h"

#include "config.h"
#include "globals.h"
#include "modules/ultra_sonic_sensor.h"
#include "state_mashine/state.h"

[[noreturn]] void initializingTask(void *parameters) {
    Serial.printf("initializingTask started on core %d\n", xPortGetCoreID());

    initializeUltraSonicSensor(US_TRIGGER, US_ECHO);

    SystemState lastSeenState = STATE_OFF;

    for (;;) {
        SystemState current = currentState;
        if (current != lastSeenState) {
            lastSeenState = current;

            if (current == STATE_INITIALIZING) {
                // Stuff that needs to be done on init.
                sendStateEvent(EVENT_DONE);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void createInitTask() {
    xTaskCreatePinnedToCore(
        &initializingTask,
        "initializingTask",
        2048,
        nullptr,
        PRIORITY_NORMAL,
        nullptr,
        CORE_ID_0
    );
}
