#include <Arduino.h>

#include "touch_handler.h"

#include "config.h"
#include "debug_handler.h"
#include "globals.h"
#include "fill_handler.h"
#include "state_machine/state.h"
#include "modules/touch_screen.h"

void handleButtonAction(SystemState current, ButtonID button) {
    debugPrint(LOG_DEBUG, "Current: State=%d, Button=%d", current, button);
    switch (current) {
        case STATE_IDLE:
            switch (button) {
                case FLUID_0:
                case FLUID_1:
                case FLUID_2:
                    sendPressedButton(button);
                    sendStateEvent(EVENT_START);
                    break;
                default:
                    break;
            }
            break;
        case STATE_FILLING:
        case STATE_SCANNING_FLUID_A_FILLING:
        case STATE_RESET_POSITION:
        case STATE_SCANNING_HEIGHT:
            if (button == ABORT) {
                sendPressedButton(ABORT);
                sendStateEvent(EVENT_STOP);
            }
            break;

        case STATE_FINISHED:
        case STATE_ERROR:
        case STATE_ABORT:
            if (button == CONTINUE) {
                sendPressedButton(CONTINUE);
                sendStateEvent(EVENT_DONE);
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
        SystemState current = currentState;
        const ButtonID currentButton = getTouchScreenButtonPressed(point);

        if (currentButton != lastButton) {
            lastChangeTime = xTaskGetTickCount();
            lastButton = currentButton;
        }

        if (xTaskGetTickCount() - lastChangeTime > pdMS_TO_TICKS(TOUCH_DEBOUNCE_DELAY_MS)) {
            if (currentButton != BUTTON_COUNT) {
                if (!buttonPressed) {
                    handleButtonAction(current, currentButton);
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
        PRIORITY_NORMAL,
        nullptr,
        CORE_ID_1
    );
}
