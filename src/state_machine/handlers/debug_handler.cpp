#include "debug_handler.h"

#include "globals.h"

#if DEBUG_ENABLED

QueueHandle_t debugQueue = nullptr;
SemaphoreHandle_t debugMutex = nullptr;
DebugLevel currentDebuLevel = LOG_DEBUG;

void debugSendToQueue(const char *message) {
    if (debugQueue) {
        xQueueSend(debugQueue, message, 0);
    }
}

void debugPrint(DebugLevel level, const char *message, ...) {
    if (!debugMutex) return;
    if (xSemaphoreTake(debugMutex, 0) != pdTRUE) return;

    char buffer[DEBUG_MESSAGE_LENGTH];
    const char *tag =
            (level == LOG_WARNING)
                ? "[WAR] "
                : (level == LOG_ERROR)
                      ? "[ERR] "
                      : (level == LOG_INFO)
                            ? "[INF] "
                            : "[DBG] ";

    snprintf(buffer, sizeof(buffer), "%s", tag);
    va_list args;
    va_start(args, message);
    vsnprintf(buffer + strlen(buffer), sizeof(buffer) - strlen(buffer), message, args);
    va_end(args);

    xSemaphoreGive(debugMutex);

    debugSendToQueue(buffer);
}


[[noreturn]] void debugTask(void *parameters) {
    char buffer[DEBUG_MESSAGE_LENGTH];

    for (;;) {
        if (xQueueReceive(debugQueue, buffer, portMAX_DELAY) == pdPASS) {
            Serial.println(buffer);
        }
    }
}

void debugBegin() {
    Serial.begin(DEBUG_SERIAL);

    debugQueue = xQueueCreate(30, 256);
    debugMutex = xSemaphoreCreateMutex();

    xTaskCreatePinnedToCore(
        debugTask,
        "DebugTask",
        4096,
        nullptr,
        PRIORITY_IDLE,
        nullptr,
        CORE_ID_0);
}

#endif
