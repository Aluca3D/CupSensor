#include <Arduino.h>

#include "Adafruit_NeoPixel.h"
#include "config.h"
#include "modules/servo.h"
#include "state_mashine/state.h"
#include "state_mashine/handlers/debugger.h"
#include "state_mashine/handlers/initializing.h"
#include "state_mashine/handlers/statusLED.h"

Adafruit_NeoPixel statusLED(ONBOARD_RGB, ONBOARD_RGB, NEO_GRBW + NEO_KHZ800);

/*
 * TODO: overwork/check stateMachine and handler (LED)
 * TODO: overwork servoMoveToo and updateServoMotion (queue based)
 * TODO: delete createDebuggingTask once sure everything works
 */

void setup() {
    Serial.begin(9600);

    createDebuggingTask();

    createLEDTask();
    createStateMachineTask();
    createInitTask();

    initializeServo();
    servoMoveToo(10);

    sendStateEvent(EVENT_START);
}

void loop() {
    updateServoMotion();
}
