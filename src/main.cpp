#include <Arduino.h>

#include "Adafruit_NeoPixel.h"
#include "config.h"
#include "modules/servo.h"
#include "state_mashine/state.h"
#include "state_mashine/handlers/initializing.h"
#include "state_mashine/handlers/statusLED.h"

Adafruit_NeoPixel statusLED(ONBOARD_RGB, ONBOARD_RGB, NEO_GRBW + NEO_KHZ800);

/*
 * TODO: overwork/check stateMachine and handler (LED)
 */

void setup() {
    Serial.begin(9600);

    createLEDTask();
    createStateMachineTask();
    createInitTask();

    CreateServoTask();

    sendStateEvent(EVENT_START);
}

void loop() {
    if (!getIsMoving()) {
        vTaskDelay(pdMS_TO_TICKS(1000));  // wait before reversing
        if (getDistanceMoved() <= 0.05) {
            servoMoveToo(10.0f);
        } else {
            servoMoveToo(0.0f);
        }
    }
    vTaskDelay(pdMS_TO_TICKS(10));  // yield time even while moving
}
