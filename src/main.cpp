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
#include "state_machine/handlers/abort_handler.h"
#include "state_machine/handlers/debug_handler.h"
#include "state_machine/handlers/fill_handler.h"
#include "state_machine/handlers/initializing.h"
#include "state_machine/handlers/scann_handler.h"
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
 * TODO: add errorHandler
 * TODO: add StopAll Function (if needed)
 * (For Error/Abort to stop scanning, Servo and Pumps)
 *
 * TODO: When all done att little Additions
 *  - Screen Write Function (to write different Screens like Finished/Error/FillStatus, etc) with Information on it
 */

void setup() {
    debugBegin();

    initializePumpRelays();
    initializeUltraSonicSensors();

    createLEDTask();
    createServoTask();

    createUpdateScreenTask();
    createCheckUserInputTask();
    createStateMachineTask();

    createAbortTask();
    createInitTask();
    createScannTask();
    createFillTask();

    sendStateEvent(EVENT_START);
}

void loop() {
}
