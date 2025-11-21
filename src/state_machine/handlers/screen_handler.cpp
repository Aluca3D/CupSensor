#include <Arduino.h>

#include "screen_handler.h"

#include "debug_handler.h"
#include "globals.h"
#include "modules/touch_screen.h"
#include "state_machine/state.h"

QueueHandle_t errorQueue = nullptr;

void sendScreenError(const char *fmt, ...) {
    if (!errorQueue) return;

    ScreenErrorMessage err{};

    va_list args;
    va_start(args, fmt);
    vsnprintf(err.text, sizeof(err.text), fmt, args);
    va_end(args);

    sendStateEvent(EVENT_ERROR);

    if (xQueueSend(errorQueue, &err, pdMS_TO_TICKS(100)) != pdPASS) {
        debugPrint(LOG_WARNING, "Failed to send error: '%s' (queue full)", err.text);
    } else {
        debugPrint(LOG_ERROR, "Sent screen error: '%s'", err.text);
    }
}

void addButton(ButtonID button) {
    drawButton(button);
    activeButtons[activeButtonsCount++] = button;
}

[[noreturn]] void updateScreenTask(void *parameters) {
    debugPrint(LOG_INFO, "updateScreenTask started on core %d", xPortGetCoreID());
    SystemState lastSeenState = STATE_OFF;

    initializeTouchScreen();

    ScreenErrorMessage pendingError{};
    bool hasPendingError = false;

    for (;;) {
        const SystemState current = currentState;

        if (xQueueReceive(errorQueue, &pendingError, 0)) {
            hasPendingError = true;

            currentState = STATE_ERROR;

            debugPrint(LOG_INFO, "Received screen error: %s", pendingError.text);
        }

        if (current != lastSeenState) {
            lastSeenState = current;

            resetScreen();

            activeButtonsCount = 0;

            switch (current) {
                case STATE_IDLE:
                    drawTextRegion(TITLE_TXT, TITLE_REGION, "IDLE");
                    addButton(FLUID_0);
                    addButton(FLUID_1);
                    addButton(FLUID_2);
                    break;

                case STATE_SCANNING_FLUID_A_FILLING:
                case STATE_SCANNING_HEIGHT:
                    drawTextRegion(TITLE_TXT, TITLE_REGION, "SCANNING");
                    addButton(ABORT);
                    break;

                case STATE_ABORT:
                    drawTextRegion(TITLE_TXT, TITLE_REGION, "ABORT");
                    addButton(CONTINUE);
                    break;
                case STATE_ERROR:
                    drawTextRegion(TITLE_TXT, TITLE_REGION, "ERROR");
                    if (hasPendingError)
                        drawTextRegion(ERROR_TXT, ERROR_REGION, pendingError.text);
                    else
                        drawTextRegion(ERROR_TXT, ERROR_REGION, "Unknown Error");

                    addButton(CONTINUE);
                    break;
                case STATE_FINISHED:
                    drawTextRegion(TITLE_TXT, TITLE_REGION, "FINISHED");
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
    errorQueue = xQueueCreate(5, sizeof(ScreenErrorMessage));
    if (!errorQueue) {
        debugPrint(LOG_WARNING, "Failed to create errorQueue!");
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
