#include <Arduino.h>

#include "us_sensor.h"

void initialize_us_sensor(int TRIGGER_PIN, int ECHO_PIN) {
    pinMode(TRIGGER_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
}

void activate_trigger(int TRIGGER_PIN) {
    digitalWrite(TRIGGER_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIGGER_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIGGER_PIN, LOW);
}

long get_echo(int ECHO_PIN) {
    return pulseIn(ECHO_PIN, HIGH);
}
