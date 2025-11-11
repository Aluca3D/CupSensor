#include <Arduino.h>

#include "screen_handler.h"

#include "debug_handler.h"
#include "globals.h"
#include "modules/touch_screen.h"
#include "state_machine/state.h"

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
                    drawButton(FLUID_0);
                    drawButton(FLUID_1);
                    drawButton(FLUID_2);

                    activeButtons[activeButtonsCount++] = FLUID_0;
                    activeButtons[activeButtonsCount++] = FLUID_1;
                    activeButtons[activeButtonsCount++] = FLUID_2;
                    break;
                case STATE_SCANNING_FLUID_A_FILLING:
                case STATE_SCANNING_HEIGHT:
                    drawButton(ABORT);
                    activeButtons[activeButtonsCount++] = ABORT;
                    break;
                case STATE_ABORT:
                    drawButton(CONTINUE);
                    activeButtons[activeButtonsCount++] = CONTINUE;
                    break;
                case STATE_ERROR:
                    drawButton(CONTINUE);
                    activeButtons[activeButtonsCount++] = CONTINUE;
                    break;
                case STATE_FINISHED:
                    drawButton(CONTINUE);
                    activeButtons[activeButtonsCount++] = CONTINUE;
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
