#include <Arduino.h>

#include "touch_handler.h"

#include "config.h"
#include "debug_handler.h"
#include "globals.h"
#include "scann_handler.h"
#include "state_machine/state.h"
#include "modules/touch_screen.h"

void handleButtonAction(ButtonID button) {
    switch (currentState) {
        case STATE_IDLE:
            switch (button) {
                case FLUID_0:
                    debugPrint(LOG_DEBUG, "Starting fill with FLUID_0");
                    sendPressedButton(FLUID_0);
                    break;
                case FLUID_1:
                    debugPrint(LOG_DEBUG, "Starting fill with FLUID_1");
                    sendPressedButton(FLUID_1);
                    break;
                case FLUID_2:
                    debugPrint(LOG_DEBUG, "Starting fill with FLUID_2");
                    sendPressedButton(FLUID_2);
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
                sendPressedButton(ABORT);
            }
            break;

        case STATE_ERROR:
        case STATE_FINISHED:
            if (button == CONTINUE) {
                debugPrint(LOG_DEBUG, "Starting fill with CONTINUE");
                sendPressedButton(CONTINUE);
            }
            break;

        default:
            break;
    }
}

[[noreturn]] void checkUserInputTask(void *parameters) {
    debugPrint(LOG_INFO, "checkUserInputTask started on core %d", xPortGetCoreID());

    ButtonID lastButton = BUTTON_COUNT;
    TickType_t lastChangeTime = 0;
    bool buttonPressed = false;

    for (;;) {
        TS_Point point = ts.getPoint();
        const ButtonID currentButton = getTouchScreenButtonPressed(point);

        if (currentButton != lastButton) {
            lastChangeTime = xTaskGetTickCount();
            lastButton = currentButton;
        }

        if (xTaskGetTickCount() - lastChangeTime > pdMS_TO_TICKS(TOUCH_DEBOUNCE_DELAY_MS)) {
            if (currentButton != BUTTON_COUNT) {
                if (!buttonPressed) {
                    handleButtonAction(currentButton);
                    buttonPressed = true;
                }
            } else {
                buttonPressed = false;
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
