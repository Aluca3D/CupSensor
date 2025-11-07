#ifndef PROJECT_CUPSENSE_RELAY_H
#define PROJECT_CUPSENSE_RELAY_H
#include "config.h"

enum PumpRelays {
    PUMP_1 = RELAY_0_PIN,
    PUMP_2 = RELAY_1_PIN,
    PUMP_3 = RELAY_2_PIN,
    PUMP_NONE = 0
};

void initializePumpRelays();

void startPump(PumpRelays pump);

void stopPump(PumpRelays pump);

void stopAllPumps();

#endif //PROJECT_CUPSENSE_RELAY_H
