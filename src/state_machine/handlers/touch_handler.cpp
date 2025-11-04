#include <Arduino.h>

#include "touch_handler.h"

#include "config.h"
#include "DebugHandler.h"
#include "globals.h"
#include "state_machine/state.h"
#include "modules/touch_screen.h"

// TODO: Replace Serial. with "update screen state and start pump"
void handleButtonAction(ButtonID button) {
    switch (currentState) {
        case STATE_IDLE:
            switch (button) {
                case FLUID_0:
                    debugPrint(LOG_DEBUG, "Starting fill with FLUID_0");
                    break;
                case FLUID_1:
                    debugPrint(LOG_DEBUG, "Starting fill with FLUID_1");
                    break;
                case FLUID_2:
                    debugPrint(LOG_DEBUG, "Starting fill with FLUID_2");
                    break;
                default:
                    break;
            }
            break;

        case STATE_FILLING:
        case STATE_SCANNING_FLUID:
        case STATE_RESET_POSITION:
        case STATE_SCANNING_HEIGHT:
            if (button == ABORT) {
                debugPrint(LOG_DEBUG, "Starting fill with ABORT");
            }
            break;

        case STATE_ERROR:
        case STATE_FINISHED:
            if (button == CONTINUE) {
                debugPrint(LOG_DEBUG, "Starting fill with CONTINUE");
            }
            break;

        default:
            break;
    }
}

[[noreturn]] void checkUserInputTask(void *parameters) {
    debugPrint(LOG_INFO, "checkUserInputTask started on core %d", xPortGetCoreID());

    ButtonID lastButton = BUTTON_COUNT;
    ButtonID stableButton = BUTTON_COUNT;
    TickType_t lastChangeTime = 0;

    for (;;) {
        TS_Point point = ts.getPoint();
        const ButtonID currentButton = getTouchScreenButtonPressed(point);

        if (currentButton != lastButton) {
            lastChangeTime = xTaskGetTickCount();
            lastButton = currentButton;
        }

        if (xTaskGetTickCount() - lastChangeTime > pdMS_TO_TICKS(TOUCH_DEBOUNCE_DELAY_MS)) {
            if (stableButton != currentButton) {
                stableButton = currentButton;

                if (stableButton != BUTTON_COUNT) {
                    handleButtonAction(stableButton);
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

void createCheckUserInputTask() {
    xTaskCreatePinnedToCore(
        checkUserInputTask,
        "checkUserInputTask",
        4096,
        nullptr,
        PRIORITY_HIGH,
        nullptr,
        CORE_ID_1
    );
}
