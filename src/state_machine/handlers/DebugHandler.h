#ifndef PROJECT_CUPSENSE_DEBUGHANDLER_H
#define PROJECT_CUPSENSE_DEBUGHANDLER_H

#include <Arduino.h>
#include "config.h"

extern SemaphoreHandle_t debugMutex;
extern QueueHandle_t debugQueue;

enum DebugLevel : uint8_t {
    LOG_ERROR,
    LOG_WARNING,
    LOG_INFO,
    LOG_DEBUG
};

extern DebugLevel currentDebuLevel;

#if DEBUG_ENABLED

void debugBegin();

void debugPrint(DebugLevel level, const char *message, ...);

#else

inline void debugBegin() {
};

inline void debugPrint(DebugLevel level, const char *message, ...) {
};

#endif

#endif //PROJECT_CUPSENSE_DEBUGHANDLER_H
