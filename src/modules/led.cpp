#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#include "led.h"

#include "config.h"

void initializeStatusLED() {
    statusLED.begin();
    statusLED.setBrightness(ONBOARD_RGB_BRIGHTNESS);
}

void setStatusLED(uint32_t color) {
    statusLED.setPixelColor(0, color);
    statusLED.show();
}
