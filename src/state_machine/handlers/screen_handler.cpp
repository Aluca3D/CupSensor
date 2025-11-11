#include <Arduino.h>

#include "screen_handler.h"

#include "debug_handler.h"
#include "globals.h"
#include "modules/touch_screen.h"
#include "state_machine/state.h"

void addButton(ButtonID button) {
    drawButton(button);
    activeButtons[activeButtonsCount++] = button;
}

// TODO: Add all screen States and new Screen Print Functions (to Modules)
[[noreturn]] void updateScreenTask(void *parameters) {
    debugPrint(LOG_INFO, "updateScreenTask started on core %d", xPortGetCoreID());
    SystemState lastSeenState = STATE_OFF;

    initializeTouchScreen();

    for (;;) {
        const SystemState current = currentState;

        if (current != lastSeenState) {
            lastSeenState = current;

            resetScreen();

            activeButtonsCount = 0;

            switch (current) {
                case STATE_IDLE:
                    addButton(FLUID_0);
                    addButton(FLUID_1);
                    addButton(FLUID_2);
                    break;

                case STATE_SCANNING_FLUID_A_FILLING:
                case STATE_SCANNING_HEIGHT:
                    addButton(ABORT);
                    break;

                case STATE_ABORT:
                case STATE_ERROR:
                case STATE_FINISHED:
                    addButton(CONTINUE);
                    break;
                default:
                    break;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void createUpdateScreenTask() {
    xTaskCreatePinnedToCore(
        updateScreenTask,
        "updateScreenTask",
        4096,
        nullptr,
        PRIORITY_NORMAL,
        nullptr,
        CORE_ID_1
    );
}
