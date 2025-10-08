#include <Arduino.h>

#include "Adafruit_NeoPixel.h"
#include "config.h"
#include "modules/servo.h"
#include "state_mashine/state.h"
#include "state_mashine/handlers/initializing.h"
#include "state_mashine/handlers/statusLED.h"

Adafruit_NeoPixel statusLED(ONBOARD_RGB, ONBOARD_RGB, NEO_GRBW + NEO_KHZ800);

void setup() {
    Serial.begin(9600);
    Serial.println("Starting...");

    startStateMachineTask();

    createLEDTask();
    createInitTask();

    sendStateEvent(EVENT_START);
    // for testing
    pinMode(37, INPUT_PULLUP);
    pinMode(36, INPUT_PULLUP);
    initializeServo();
}

void loop() {
    // Determine servo direction and motion
    if (digitalRead(36) == LOW) {
        if (digitalRead(37) == LOW) {
            moveServoForwards();
        } else {
            moveServoBackwards();
        }
    } else {
        stopServo();
    }
    printDistanceMoved();
}
