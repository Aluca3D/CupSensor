#include <Arduino.h>

#include "screen_handler.h"

#include "debug_handler.h"
#include "globals.h"
#include "modules/touch_screen.h"

volatile ScreenState currentScreen = SCREEN_OFF;
volatile ScreenState lastScreen = SCREEN_OFF;
QueueHandle_t screenEventQueue = nullptr;

void sendScreenEvent(ScreenEvent event) {
    if (screenEventQueue) {
        xQueueSend(screenEventQueue, &event, 0);
    }
}

// TODO: Add all screen States and new Screen Print Functions (to Modules)
[[noreturn]] void updateScreenTask(void *parameters) {
    debugPrint(LOG_INFO, "updateScreenTask started on core %d", xPortGetCoreID());
    initializeTouchScreen();

    ScreenEvent event = SCREEN_EVENT_NONE;
    for (;;) {
        if (xQueueReceive(screenEventQueue, &event, portMAX_DELAY)) {
            lastScreen = currentScreen;

            resetScreen();

            activeButtonsCount = 0;

            switch (currentScreen) {
                case SCREEN_OFF:
                    if (event == SCREEN_EVENT_START) {
                        drawButton(FLUID_0);
                        drawButton(FLUID_1);
                        drawButton(FLUID_2);

                        activeButtons[activeButtonsCount++] = FLUID_0;
                        activeButtons[activeButtonsCount++] = FLUID_1;
                        activeButtons[activeButtonsCount++] = FLUID_2;

                        currentScreen = SCREEN_IDLE;
                    } else if (event == SCREEN_EVENT_ERROR) {
                        currentScreen = SCREEN_ERROR;
                    }
                    break;
                case SCREEN_IDLE:
                    if (event == SCREEN_EVENT_START) {

                        drawButton(ABORT);

                        activeButtons[activeButtonsCount++] = ABORT;

                        currentScreen = SCREEN_SCANNING;
                    } else if (event == SCREEN_EVENT_ERROR) {

                        drawButton(CONTINUE);

                        activeButtons[activeButtonsCount++] = CONTINUE;

                        currentScreen = SCREEN_ERROR;
                    }
                    break;
                case SCREEN_SCANNING:
                case SCREEN_FILLING:
                    if (event == SCREEN_EVENT_STOP) {
                        drawButton(CONTINUE);
                        activeButtons[activeButtonsCount++] = CONTINUE;
                        currentScreen = SCREEN_ABORT;
                    } else if (event == SCREEN_EVENT_DONE) {
                        drawButton(CONTINUE);
                        activeButtons[activeButtonsCount++] = CONTINUE;
                        currentScreen = SCREEN_FINISHED;
                    }
                    break;
                case SCREEN_ABORT:
                case SCREEN_ERROR:
                case SCREEN_FINISHED:
                    if (event == SCREEN_EVENT_DONE) {
                        drawButton(FLUID_0);
                        drawButton(FLUID_1);
                        drawButton(FLUID_2);

                        activeButtons[activeButtonsCount++] = FLUID_0;
                        activeButtons[activeButtonsCount++] = FLUID_1;
                        activeButtons[activeButtonsCount++] = FLUID_2;

                        currentScreen = SCREEN_IDLE;
                    }
                    break;
                default:
                    break;
            }
            debugPrint(LOG_DEBUG, "Transition Screen: %d -> %d (event=%d)", lastScreen, currentScreen, event);
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void createUpdateScreenTask() {
    screenEventQueue = xQueueCreate(10, sizeof(ScreenEvent));
    if (!screenEventQueue) {
        debugPrint(LOG_WARNING, "Failed to create screen queue!");
        while (true) {
        }
    }

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
