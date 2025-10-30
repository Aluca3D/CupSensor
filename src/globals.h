#ifndef PROJECT_CUPSENSE_GLOBAL_VARIABLES_H
#define PROJECT_CUPSENSE_GLOBAL_VARIABLES_H

enum TaskPriority {
    PRIORITY_IDLE = 0,
    PRIORITY_LOW = 1,
    PRIORITY_NORMAL = 2,
    PRIORITY_HIGH = 3
};

enum CoreID {
    CORE_ID_0 = 0,
    CORE_ID_1 = 1,
};

extern unsigned long setupHeight;

#endif //PROJECT_CUPSENSE_GLOBAL_VARIABLES_H
