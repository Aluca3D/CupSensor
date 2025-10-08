#ifndef PROJECT_CUPSENSE_LED_H
#define PROJECT_CUPSENSE_LED_H

#include <Adafruit_NeoPixel.h>

extern Adafruit_NeoPixel statusLED;


void initializeStatusLED();

void setStatusLED(uint32_t color);

#endif //PROJECT_CUPSENSE_LED_H
