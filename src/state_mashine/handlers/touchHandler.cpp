#include <Arduino.h>

#include "touchHandler.h"

#include "config.h"
#include "globals.h"
#include "state_mashine/state.h"
#include "modules/touchScreen.h"

// TODO: Replace Serial. with "update screen state and start pump"
void handleButtonAction(SystemState state, ButtonID button) {
    switch (state) {
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
// TODO: Fix checkUserInputTask
// (only getting Button 2, and only sometimes, maybe implement better debaunce)
[[noreturn]] void checkUserInputTask(void *parameters) {
    int stableCount = 0;
    ButtonID lastButton = BUTTON_COUNT;
    bool pressedHandled = false;

    for (;;) {
        const SystemState current = currentState;
        TS_Point point = ts.getPoint();

        ButtonID pressedButton = BUTTON_COUNT;

        if (isScreenPressed(point)) {
            switch (current) {
                case STATE_IDLE:
                    if (isButtonPressed(FLUID_0, point)) pressedButton = FLUID_0;
                    else if (isButtonPressed(FLUID_1, point)) pressedButton = FLUID_1;
                    else if (isButtonPressed(FLUID_2, point)) pressedButton = FLUID_2;
                    break;

                case STATE_FILLING:
                case STATE_SCANNING_HEIGHT:
                case STATE_SCANNING_FLUID:
                case STATE_RESET_POSITION:
                    if (isButtonPressed(ABORT, point)) pressedButton = ABORT;
                    break;

                case STATE_ERROR:
                case STATE_FINISHED:
                    if (isButtonPressed(CONTINUE, point)) pressedButton = CONTINUE;
                    break;

                default:
                    break;
            }
        }

        if (pressedButton != BUTTON_COUNT && pressedButton == lastButton) {
            stableCount++;
            if (stableCount == DEBOUNCE_COUNT && !pressedHandled) {
                Serial.printf("Button %d pressed in state %d\n", pressedButton, current);
                pressedHandled = true;

                handleButtonAction(current, pressedButton);
            }
        } else {
            if (pressedButton == BUTTON_COUNT) {
                pressedHandled = false;
            }
            stableCount = 0;
            lastButton = pressedButton;
        }
        Serial.printf("pressed%d, last%d \n",pressedButton, lastButton);
        vTaskDelay(pdMS_TO_TICKS(50));
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
