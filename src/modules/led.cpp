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
                case STATE_OFF:
                    setStatusLED(Adafruit_NeoPixel::Color(0, 0, 0)); // OFF
                    break;
                case STATE_INITIALIZING:
                    setStatusLED(Adafruit_NeoPixel::Color(255, 0, 255)); // Magenta
                    break;
                case STATE_IDLE:
                    setStatusLED(Adafruit_NeoPixel::Color(255, 255, 0)); // Yellow
                    break;
                case STATE_SCANNING_HEIGHT:
                case STATE_SCANNING_FLUID:
                    setStatusLED(Adafruit_NeoPixel::Color(0, 255, 255)); // Cyan
                    break;
                case STATE_RESET_POSITION:
                case STATE_FILLING:
                    setStatusLED(Adafruit_NeoPixel::Color(0, 0, 255)); // Blue
                    break;
                case STATE_ERROR:
                    setStatusLED(Adafruit_NeoPixel::Color(255, 0, 0)); // Red
                    break;
                case STATE_FINISHED:
                    setStatusLED(Adafruit_NeoPixel::Color(0, 255, 0)); // Green
                    break;
                default:
                    setStatusLED(Adafruit_NeoPixel::Color(0, 0, 0)); // OFF
                    break;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(300));
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
