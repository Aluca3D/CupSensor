#include <Arduino.h>
#include <Adafruit_NeoPixel.h>


#include "statusLED.h"

#include "modules/led.h"
#include "globals.h"
#include "state_mashine/state.h"


[[noreturn]] void LEDTask(void *parameters) {
    Serial.printf("LEDTask started on core %d\n", xPortGetCoreID());

    initializeStatusLED();

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
