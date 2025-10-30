#include <Arduino.h>

#include "ultra_sonic_sensor.h"

void initializeUltraSonicSensors() {
    pinMode(WaterEcho, INPUT);
    pinMode(WaterTrigger, OUTPUT);
    pinMode(HeightEcho, INPUT);
    pinMode(HeightTrigger, OUTPUT);
}

void activateTrigger(UltraSonicSensors TRIGGER) {
    digitalWrite(TRIGGER, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIGGER, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIGGER, LOW);
}

unsigned long getEcho(UltraSonicSensors ECHO) {
    return pulseIn(ECHO, HIGH, PULSEIN_TIMEOUT_US);
}

unsigned long getDistance(UltraSonicSensors TRIGGER, UltraSonicSensors ECHO) {
    activateTrigger(TRIGGER);
    const unsigned  long echoTime = getEcho(ECHO);
    return echoTime;
}

unsigned long getAverageDistance(UltraSonicSensors TRIGGER, UltraSonicSensors ECHO) {
    unsigned long total = 0;
    int validReadings = 0;

    for (int i = 0; i < NUMBER_OF_READINGS; i++) {
        const unsigned  long echoTime = getDistance(TRIGGER, ECHO);
        if (echoTime > 0) {
            total += echoTime;
            validReadings++;
        }
        delay(10);
    }
    if (validReadings > 0) {
        return total / validReadings;
    }
    return 0;
}
