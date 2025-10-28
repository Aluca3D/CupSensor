#include "pump_relay.h"

#include <Arduino.h>

void initializePumpRelays() {
    pinMode(PUMP_1, OUTPUT);
    pinMode(PUMP_2, OUTPUT);
    pinMode(PUMP_3, OUTPUT);
}

void startPump(PumpRelays pump) {
    digitalWrite(pump, HIGH);
}

void stopPump(PumpRelays pump) {
    digitalWrite(pump, LOW);
}

void stopAllPumps() {
    constexpr PumpRelays pumpList[] = {PUMP_1, PUMP_2, PUMP_3};

    for (PumpRelays pump: pumpList) {
        stopPump(pump);
    }
}
