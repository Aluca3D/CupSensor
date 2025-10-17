#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_NeoPixel.h>

#include "config.h"
#include "globals.h"
#include "modules/servo.h"
#include "modules/touchScreen.h"
#include "state_mashine/state.h"
#include "state_mashine/handlers/initializing.h"
#include "state_mashine/handlers/statusLED.h"

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
 * TODO: overwork/Add Debugger (Serial print into Task/s)
 */

void setup() {
    Serial.begin(9600);

    createLEDTask();
    createStateMachineTask();
    createInitTask();

    createServoTask();

    sendStateEvent(EVENT_START);

    // TouchScreen Test
    initializeTouchScreen();

    drawButton(FLUID_0);
    drawButton(FLUID_1);
    drawButton(FLUID_2);
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

    //const TS_Point point = ts.getPoint();
    //Serial.print("Raw X: "); Serial.print(point.x);
    //Serial.print(" Y: "); Serial.println(point.y);

    for (int id = FLUID_0; id <= FLUID_2; id++) {
        if (isButtonPressed(static_cast<ButtonID>(id))) {
            switch (id) {
                case FLUID_0:
                    Serial.println("Button FLUID_0 pressed");
                    break;
                case FLUID_1:
                    Serial.println("Button FLUID_1 pressed");
                    break;
                case FLUID_2:
                    Serial.println("Button FLUID_2 pressed");
                    break;
            }
        }
    }
}
