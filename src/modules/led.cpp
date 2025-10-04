#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#include "led.h"

#include "config.h"
#include "../globals.h"
#include "state_mashine/state.h"

Adafruit_NeoPixel statusLED(ONBOARD_RGB, ONBOARD_RGB, NEO_GRBW + NEO_KHZ800);

void initializeLED() {
    statusLED.begin();
    statusLED.setBrightness(ONBOARD_RGB_BRIGHTNESS);
}

void setStatusLED(const uint32_t color) {
    statusLED.setPixelColor(0, color);
    statusLED.show();
}

[[noreturn]] void LEDTask(void *parameters) {
    Serial.printf("LEDTask started on core %d\n", xPortGetCoreID());

    initializeLED();

    SystemState lastSeenState = STATE_OFF;

    for (;;) {
        SystemState current = currentState;

        if (current != lastSeenState) {
            lastSeenState = current;

            switch (current) {
                case STATE_INITIALIZING:
                    setStatusLED(Adafruit_NeoPixel::Color(255, 0, 0));
                    break;
                case STATE_IDLE:
                    setStatusLED(Adafruit_NeoPixel::Color(255, 255, 0));
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
