#include <Arduino.h>

#include "debuger.h"

#include "config.h"
#include "globals.h"
#include "modules/servo.h"

void printBackSpace(const char *text) {
    const int len = strlen(text);
    for (int i = 0; i < len; ++i) {
        Serial.print("\b");
    }
}

[[noreturn]] void debuggerTask(void *parameters) {
    Serial.printf("debuggerTask started on core %d\n", xPortGetCoreID());
    constexpr char printLine[256] = "";
    for (;;) {
        sprintf(
            const_cast<char *>(printLine),
            "%-5f mm | isForwards %d",
            getDistanceMoved(), getIsForwards()
        );
        Serial.print(printLine);
        printBackSpace(printLine);

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void createDebuggingTask() {
    if (DEBUGGING) {
        xTaskCreatePinnedToCore(
            &debuggerTask,
            "debuggerTask",
            4096,
            nullptr,
            PRIORITY_IDLE,
            nullptr,
            CORE_ID_0
        );
    }
}
