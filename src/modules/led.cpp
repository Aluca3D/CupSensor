#include <Arduino.h>

#include "led.h"
#include "state_mashine/state.h"

#define LED_RGB_PIN 38

#define LED_RED_PIN 21
#define LED_YELLOW_PIN 20
#define LED_GREEN_PIN 19

void turn_on_led(int LED_PIN) {
    digitalWrite(LED_PIN, HIGH);
}

void turn_off_led(int LED_PIN) {
    digitalWrite(LED_PIN, LOW);
}

[[noreturn]] void led_task(void *parameters) {
    Serial.printf("LED task started on core %d\n", xPortGetCoreID());

    pinMode(LED_RED_PIN, OUTPUT);
    pinMode(LED_YELLOW_PIN, OUTPUT);
    pinMode(LED_GREEN_PIN, OUTPUT);

    SystemState last_seen_state = STATE_OFF;

    for (;;) {
        SystemState current = current_state;

        if (current != last_seen_state) {
            last_seen_state = current;

            switch (current) {
                case STATE_INITIALIZING:
                    turn_on_led(LED_YELLOW_PIN);
                    break;
                case STATE_IDLE:
                    turn_off_led(LED_YELLOW_PIN);
                    turn_on_led(LED_GREEN_PIN);
                    break;
                default:
                    break;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void create_led_task() {
    xTaskCreatePinnedToCore(
        led_task,
        "led_task",
        2048,
        nullptr,
        1,
        nullptr,
        1
    );
}
