#include <Arduino.h>

#include "touchHandler.h"

#include "config.h"
#include "globals.h"
#include "state_machine/state.h"
#include "modules/touchScreen.h"

// TODO: Replace Serial. with "update screen state and start pump"
void handleButtonAction(ButtonID button) {
    switch (currentState) {
        case STATE_IDLE:
            switch (button) {
                case FLUID_0:
                    Serial.println("Starting fill with fluid 0");
                    break;
                case FLUID_1:
                    Serial.println("Starting fill with fluid 1");
                    break;
                case FLUID_2:
                    Serial.println("Starting fill with fluid 2");
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
                Serial.println("Aborting operation!");
            }
            break;

        case STATE_ERROR:
        case STATE_FINISHED:
            if (button == CONTINUE) {
                Serial.println("Continuing after error/finish");
            }
            break;

        default:
            break;
    }
}

[[noreturn]] void checkUserInputTask(void *parameters) {
    ButtonID lastButton = BUTTON_COUNT;
    ButtonID stableButton = BUTTON_COUNT;
    TickType_t lastChangeTime = 0;

    for (;;) {
        TS_Point point = ts.getPoint();
        const ButtonID currentButton = getPressedButton(point);

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
