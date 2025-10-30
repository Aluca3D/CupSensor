#include <Arduino.h>

#include "status_LED.h"

#include "modules/led.h"
#include "globals.h"
#include "state_machine/state.h"


[[noreturn]] void LEDTask(void *parameters) {
    Serial.printf("LEDTask started on core %d\n", xPortGetCoreID());

    initializeStatusLED();

    SystemState lastSeenState = STATE_OFF;

    for (;;) {
        const SystemState current = currentState;

        if (current != lastSeenState) {
            lastSeenState = current;

            switch (current) {
                case STATE_OFF:
                    setStatusLED(STATUS_OFF);
                    break;
                case STATE_INITIALIZING:
                    setStatusLED(STATUS_MAGENTA);
                    break;
                case STATE_IDLE:
                    setStatusLED(STATUS_YELLOW);
                    break;
                case STATE_SCANNING_HEIGHT:
                case STATE_SCANNING_FLUID:
                    setStatusLED(STATUS_CYAN);
                    break;
                case STATE_RESET_POSITION:
                case STATE_FILLING:
                    setStatusLED(STATUS_BLUE);
                    break;
                case STATE_ERROR:
                    setStatusLED(STATUS_RED);
                    break;
                case STATE_FINISHED:
                    setStatusLED(STATUS_GREEN);
                    break;
                default:
                    setStatusLED(STATUS_WHITE);
                    break;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(300));
    }
}

void createLEDTask() {
    xTaskCreatePinnedToCore(
        LEDTask,
        "LEDTask",
        2048,
        nullptr,
        PRIORITY_IDLE,
        nullptr,
        CORE_ID_0
    );
}
