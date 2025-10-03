#include <Arduino.h>

#include "led.h"

#include "config.h"
#include "../globals.h"
#include "state_mashine/state.h"

void turnOnLED(int LED_PIN) {
    digitalWrite(LED_PIN, HIGH);
}

void turnOffLED(int LED_PIN) {
    digitalWrite(LED_PIN, LOW);
}

[[noreturn]] void LEDTask(void *parameters) {
    Serial.printf("LEDTask started on core %d\n", xPortGetCoreID());

    pinMode(LED_RED_PIN, OUTPUT);
    pinMode(LED_YELLOW_PIN, OUTPUT);
    pinMode(LED_GREEN_PIN, OUTPUT);

    SystemState lastSeenState = STATE_OFF;

    for (;;) {
        SystemState current = currentState;

        if (current != lastSeenState) {
            lastSeenState = current;

            switch (current) {
                case STATE_INITIALIZING:
                    turnOnLED(LED_YELLOW_PIN);
                    break;
                case STATE_IDLE:
                    turnOffLED(LED_YELLOW_PIN);
                    turnOnLED(LED_GREEN_PIN);
                    break;
                default:
                    break;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void createLEDTask() {
    xTaskCreatePinnedToCore(
        LEDTask,
        "LEDTask",
        2048,
        nullptr,
        PRIORITY_IDLE,
        nullptr,
        CORE_ID_0
    );
}
