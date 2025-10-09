#include <Arduino.h>

#include "debugger.h"

#include "config.h"
#include "globals.h"
#include "modules/servo.h"

void printBackSpace(const char *text) {
    const int len = strlen(text);
    for (int i = 0; i < len; ++i) {
        Serial.print("\b");
    }
}

// Todo add all debug info into here
[[noreturn]] void debuggerTask(void *parameters) {
    Serial.begin(9600);
    Serial.printf("debuggerTask started on core %d\n", xPortGetCoreID());
    constexpr char printLine[256] = "";
    for (;;) {
        sprintf(
            const_cast<char *>(printLine),
            "SERVO: %-10f mm | isForwards %-1d | speed %-10d | moving %-1d",
            getDistanceMoved() * 10, getIsForwards(), getServoPulseWidth(), getIsMoving()
        );
        Serial.print(printLine);
        printBackSpace(printLine);

        vTaskDelay(pdMS_TO_TICKS(100));
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
