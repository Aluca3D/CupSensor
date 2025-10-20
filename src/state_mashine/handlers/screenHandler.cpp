#include <Arduino.h>

#include "screenHandler.h"

#include "globals.h"
#include "modules/touchScreen.h"
#include "state_mashine/state.h"

volatile ScreenState currentScreen = SCREEN_OFF;
volatile ScreenState lastScreen = SCREEN_OFF;
QueueHandle_t screenEventQueue = nullptr;

void sendScreenEvent(ScreenEvent event) {
    if (screenEventQueue) {
        xQueueSend(screenEventQueue, &event, 0);
    }
}

// TODO: Replace Serial. with "update state and pump"
[[noreturn]] void checkUserInputTask(void *parameters) {
    SystemState lastSeenState = STATE_OFF;

    for (;;) {
        const SystemState current = currentState;

        if (current != lastSeenState) {
            lastSeenState = current;

            switch (current) {
                case STATE_IDLE:
                    if (isButtonPressed(FLUID_0)) {
                        Serial.println("Button FLUID_0 pressed");
                    } else if (isButtonPressed(FLUID_1)) {
                        Serial.println("Button FLUID_1 pressed");
                    } else if (isButtonPressed(FLUID_2)) {
                        Serial.println("Button FLUID_2 pressed");
                    }
                    break;
                case STATE_SCANNING_HEIGHT:
                case STATE_SCANNING_FLUID:
                case STATE_RESET_POSITION:
                case STATE_FILLING:
                    if (isButtonPressed(ABORT)) {
                        Serial.println("Button ABORT pressed");
                    }
                    break;
                case STATE_ERROR:
                case STATE_FINISHED:
                    if (isButtonPressed(CONTINUE)) {
                        Serial.println("Button CONTINUE pressed");
                    }
                    break;
                default:
                    break;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

// TODO: Add all screen States and new Screen Print Functions (Id needed or time)
[[noreturn]] void updateScreenTask(void *parameters) {
    initializeTouchScreen();

    ScreenEvent event = SCREEN_EVENT_NONE;
    for (;;) {
        if (xQueueReceive(screenEventQueue, &event, portMAX_DELAY)) {
            lastScreen = currentScreen;

            switch (currentScreen) {
                case SCREEN_OFF:
                    if (event == SCREEN_EVENT_START) {
                        setScreenColor(COLOR_YELLOW);
                        drawButton(FLUID_0);
                        drawButton(FLUID_1);
                        drawButton(FLUID_2);
                        currentScreen = SCREEN_IDLE;
                    } else if (event == SCREEN_EVENT_ERROR) {
                        currentScreen = SCREEN_ERROR;
                    }
                    break;
                case SCREEN_IDLE:
                    if (event == SCREEN_EVENT_START) {
                        currentScreen = SCREEN_SCANNING;
                    } else if (event == SCREEN_EVENT_ERROR) {
                        currentScreen = SCREEN_ERROR;
                    }
                    break;
                default:
                    break;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100));
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
