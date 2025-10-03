#include <Arduino.h>

#include "config.h"
#include "modules/led.h"
#include "modules/us_sensor.h"
#include "state_mashine/state.h"
#include "state_mashine/handlers/initializing.h"


#define US_ECHO 6
#define US_TRIGGER 7


[[noreturn]] void measureDistance(void *param) {
    initialize_us_sensor(US_TRIGGER, US_ECHO);
    for (;;) {
        activate_trigger(US_TRIGGER);

        const long duration = get_echo(US_ECHO);
        const long distance = duration * 0.0343 / 2;

        Serial.print(distance);
        Serial.println("cm");

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

void setup() {
    Serial.begin(9600);
    Serial.println("Starting...");

    startStateMachineTask();

    create_led_task();
    create_init_task();

    sendStateEvent(EVENT_START);
}


void loop() {
    // for testing
}
