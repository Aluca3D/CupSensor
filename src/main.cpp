#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_NeoPixel.h>

#include "config.h"
#include "globals.h"
#include "modules/pump_relay.h"
#include "modules/servo.h"
#include "modules/ultra_sonic_sensor.h"
#include "state_machine/state.h"
#include "state_machine/handlers/debug_handler.h"
#include "state_machine/handlers/initializing.h"
#include "state_machine/handlers/screen_handler.h"
#include "state_machine/handlers/status_LED.h"
#include "state_machine/handlers/touch_handler.h"

Adafruit_NeoPixel statusLED(
    ONBOARD_RGB_PIN, ONBOARD_RGB_PIN, NEO_GRBW + NEO_KHZ800
);

Adafruit_ILI9341 tft(
    SCREEN_CS_PIN, SCREEN_DC_PIN,
    SCREEN_MOSI_PIN, SCREEN_SCK_PIN,
    SCREEN_RESET_PIN,SCREEN_MISO_PIN
);

SPIClass tsSPI(VSPI);
XPT2046_Touchscreen ts(TOUCH_CS_PIN, TOUCH_IRQ_PIN);

/*
 * TODO: add StopAll Function (if needed)
 * (For Error/Abort to stop scanning, Servo and Pumps)
 * TODO: add Print Task/Function
 * (Replace all Serial.Print for smother workflow)
 */

[[noreturn]] void test(void *pvParameters) {
    for (;;) {
        if (!getIsMoving() && currentState != STATE_INITIALIZING) {
            vTaskDelay(pdMS_TO_TICKS(1000)); // wait before reversing
            if (getDistanceMoved() <= 0.05) {
                servoMoveToo(10.0f);
            } else {
                servoMoveToo(0.0f);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100)); // yield time even while moving
    }
}

void setup() {
    debugBegin();

    initializePumpRelays();
    initializeUltraSonicSensors();

    createLEDTask();
    createStateMachineTask();
    createInitTask();

    createServoTask();

    createUpdateScreenTask();
    createCheckUserInputTask();

    xTaskCreatePinnedToCore(
        test,
        "testTask",
        4096,
        nullptr,
        PRIORITY_NORMAL,
        nullptr,
        CORE_ID_0
    );

    sendStateEvent(EVENT_START);
    sendScreenEvent(SCREEN_EVENT_START);
}

void loop() {
}
