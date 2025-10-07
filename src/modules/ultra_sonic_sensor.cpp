#include <Arduino.h>

#include "ultra_sonic_sensor.h"

void initializeUltraSonicSensor(int TRIGGER_PIN, int ECHO_PIN) {
    pinMode(TRIGGER_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
}

void activateTrigger(int TRIGGER_PIN) {
    digitalWrite(TRIGGER_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIGGER_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIGGER_PIN, LOW);
}

long getEcho(int ECHO_PIN) {
    return pulseIn(ECHO_PIN, HIGH);
}

long getDistance(int TRIGGER_PIN, int ECHO_PIN) {
    activateTrigger(TRIGGER_PIN);
    long distance = getEcho(ECHO_PIN);
    return distance;
}
