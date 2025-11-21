#include "debug_handler.h"

#include "globals.h"

#if DEBUG_ENABLED

volatile bool debugPrintTaskReady = false;

QueueHandle_t debugQueue = nullptr;
SemaphoreHandle_t debugMutex = nullptr;

void debugSendToQueue(const char *message) {
    if (!debugQueue) return;

    char buffer[DEBUG_MESSAGE_LENGTH];
    strncpy(buffer, message, DEBUG_MESSAGE_LENGTH - 1);
    buffer[DEBUG_MESSAGE_LENGTH - 1] = '\0';
    xQueueSend(debugQueue, buffer, 0);
}

void debugPrint(DebugLevel level, const char *message, ...) {
    if (!debugMutex) return;
    if (xSemaphoreTake(debugMutex, pdMS_TO_TICKS(5)) != pdTRUE) return;

    char buffer[DEBUG_MESSAGE_LENGTH];

    static const char *tags[] = {
        "[ERR] ",
        "[WAR] ",
        "[INF] ",
        "[DBG] ",
    };
    const char *tag = tags[level];

    va_list args;
    va_start(args, message);

    const int numChar = snprintf(buffer, sizeof(buffer), "%s", tag);
    if (numChar > 0 && numChar < sizeof(buffer)) {
        vsnprintf(buffer + numChar, sizeof(buffer) - numChar, message, args);
    }
    va_end(args);

    xSemaphoreGive(debugMutex);

    debugSendToQueue(buffer);
}

[[noreturn]] void debugTask(void *parameters) {
    debugPrintTaskReady = true;
    debugPrint(LOG_INFO, "debugTask started on core %d", xPortGetCoreID());

    char buffer[DEBUG_MESSAGE_LENGTH];

    for (;;) {
        if (xQueueReceive(debugQueue, buffer, portMAX_DELAY) == pdPASS) {
            Serial.println(buffer);
        }
    }
}

void debugBegin() {
    Serial.begin(DEBUG_SERIAL);

    debugQueue = xQueueCreate(30, DEBUG_MESSAGE_LENGTH);
    debugMutex = xSemaphoreCreateMutex();

    xTaskCreatePinnedToCore(
        debugTask,
        "DebugTask",
        STACK_SIZE_LARGE,
        nullptr,
        PRIORITY_IDLE,
        nullptr,
        CORE_ID_0
    );

    while (!debugPrintTaskReady) {
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

#endif
