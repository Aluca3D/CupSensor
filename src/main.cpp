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
#include "state_mashine/handlers/statusLED.h"

Adafruit_NeoPixel statusLED(ONBOARD_RGB_PIN, ONBOARD_RGB_PIN, NEO_GRBW + NEO_KHZ800);

/*
 * TODO: overwork/check stateMachine and handler (LED)
 */

Adafruit_ILI9341 tft(
    SCREEN_CS_PIN, SCREEN_DC_PIN,
    SCREEN_MOSI_PIN, SCREEN_SCK_PIN,
    SCREEN_RESET_PIN,SCREEN_MISO_PIN
);

[[noreturn]] void duckTask(void *pvParameters) {

    // duck parameters
    const int cx = 160, cy = 120; // center of rotation
    const int bodyR = 40; // body radius
    const int headR = 20; // head radius
    const int beakLen = 15, beakH = 10; // beak length & height
    const int eyeR = 3; // eye radius

    // precompute unrotated offsets relative to (0,0)=center:
    const float head_dx = 0;
    const float head_dy = -bodyR - headR;
    const float p1_dx = head_dx + headR; // base of beak
    const float p1_dy = head_dy;
    const float p2_dx = head_dx + headR + beakLen; // top of beak
    const float p2_dy = head_dy - beakH / 2.0;
    const float p3_dx = head_dx + headR + beakLen; // bottom of beak
    const float p3_dy = head_dy + beakH / 2.0;
    const float eye_dx = head_dx + headR / 2.0; // eye position
    const float eye_dy = head_dy - headR / 2.0;

    for (;;) {
        static float angle = 0;
        const float a = angle * (3.14159265 / 180.0);
        float c = cos(a), s = sin(a);

        // clear
        tft.fillScreen(tft.color565(0, 0, 0));

        // draw body
        uint16_t yellow = tft.color565(255, 255, 0);
        tft.fillCircle(cx, cy, bodyR, yellow);

        // head
        int16_t hx = cx + int16_t(head_dx * c - head_dy * s);
        int16_t hy = cy + int16_t(head_dx * s + head_dy * c);
        tft.fillCircle(hx, hy, headR, yellow);

        // beak triangle
        uint16_t orange = tft.color565(255, 165, 0);
        int16_t x1 = cx + int16_t(p1_dx * c - p1_dy * s);
        int16_t y1 = cy + int16_t(p1_dx * s + p1_dy * c);
        int16_t x2 = cx + int16_t(p2_dx * c - p2_dy * s);
        int16_t y2 = cy + int16_t(p2_dx * s + p2_dy * c);
        int16_t x3 = cx + int16_t(p3_dx * c - p3_dy * s);
        int16_t y3 = cy + int16_t(p3_dx * s + p3_dy * c);
        tft.fillTriangle(x1, y1, x2, y2, x3, y3, orange);

        // eye
        uint16_t black = tft.color565(0, 0, 0);
        int16_t ex = cx + int16_t(eye_dx * c - eye_dy * s);
        int16_t ey = cy + int16_t(eye_dx * s + eye_dy * c);
        tft.fillCircle(ex, ey, eyeR, black);

        // next frame
        angle += 5;
        if (angle >= 360) angle -= 360;
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void setup() {
    Serial.begin(9600);

    createLEDTask();
    createStateMachineTask();
    createInitTask();

    createServoTask();

    sendStateEvent(EVENT_START);

    // TouchScreen Test

    pinMode(SCREEN_LED_PIN, OUTPUT);
    digitalWrite(SCREEN_LED_PIN, HIGH); // backlight on
    tft.begin();
    tft.setRotation(1);
    tft.fillScreen(tft.color565(0, 0, 0));

    xTaskCreatePinnedToCore(
           duckTask,
           "duckTask",
           8192,
           nullptr,
           PRIORITY_HIGH,
           nullptr,
           CORE_ID_1
       );
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
