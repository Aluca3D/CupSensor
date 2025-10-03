#include <Arduino.h>

#include "modules/led.h"
#include "state_mashine/state.h"
#include "state_mashine/handlers/initializing.h"


void setup() {
    Serial.begin(9600);
    Serial.println("Starting...");

    startStateMachineTask();

    createLEDTask();
    createInitTask();

    sendStateEvent(EVENT_START);
}


void loop() {
    // for testing
}
