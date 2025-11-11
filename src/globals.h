#ifndef PROJECT_CUPSENSE_GLOBAL_VARIABLES_H
#define PROJECT_CUPSENSE_GLOBAL_VARIABLES_H

enum TaskPriority : UBaseType_t {
    PRIORITY_IDLE = 0,
    PRIORITY_LOW = 1,
    PRIORITY_NORMAL = 2,
    PRIORITY_HIGH = 3
};

enum CoreID : UBaseType_t {
    CORE_ID_0 = 0,
    CORE_ID_1 = 1,
};

enum TaskStackSize : uint32_t {
    STACK_SIZE_MINIMAL = 512,
    STACK_SIZE_SMALL = 1024,
    STACK_SIZE_MEDIUM = 2048,
    STACK_SIZE_LARGE = 4096,
    STACK_SIZE_HUGE = 8192,
    STACK_SIZE_DEBUG = 16384,
};

extern unsigned long setupHeight;

#endif //PROJECT_CUPSENSE_GLOBAL_VARIABLES_H
