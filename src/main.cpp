#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_NeoPixel.h>

#include "config.h"
#include "globals.h"
#include "modules/servo.h"
#include "state_mashine/state.h"
#include "state_mashine/handlers/initializing.h"
#include "state_mashine/handlers/screenHandler.h"
#include "state_mashine/handlers/statusLED.h"
#include "state_mashine/handlers/touchHandler.h"

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
 * TODO: overwork/check stateMachine and handler (LED)
 * TODO: add StopAll Function
 * (For Error/Abort to stop scanning, Servo and Pumps)
 * TODO: overwork/add US Sensor Functions
 * (Update Initializer for Both sensors, add calculation functions for Fluid Level/ Cup Height set Initial Height, etc)
 */

void setup() {
    Serial.begin(9600);

    createLEDTask();
    createStateMachineTask();
    createInitTask();

    createServoTask();

    createUpdateScreenTask();
    createCheckUserInputTask();

    sendStateEvent(EVENT_START);
    sendScreenEvent(SCREEN_EVENT_START);
}

void loop() {
    //if (!getIsMoving()) {
    //    vTaskDelay(pdMS_TO_TICKS(1000));  // wait before reversing
    //    if (getDistanceMoved() <= 0.05) {
    //        servoMoveToo(10.0f);
    //    } else {
    //        servoMoveToo(0.0f);
    //    }
    //}
    //vTaskDelay(pdMS_TO_TICKS(10));  // yield time even while moving
}
