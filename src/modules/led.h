#ifndef PROJECT_CUPSENSE_LED_H
#define PROJECT_CUPSENSE_LED_H

#include <Adafruit_NeoPixel.h>

extern Adafruit_NeoPixel statusLED;

constexpr uint32_t color888(uint8_t r, uint8_t g, uint8_t b) {
    return ( static_cast<uint32_t>(r) << 16 ) | ( static_cast<uint32_t>(g) << 8 ) | b;
}

enum StatusColor {
    STATUS_OFF     = color888(0,   0,   0),
    STATUS_RED     = color888(255, 0,   0),
    STATUS_GREEN   = color888(0,   255, 0),
    STATUS_BLUE    = color888(0,   0,   255),
    STATUS_YELLOW  = color888(255, 255, 0),
    STATUS_MAGENTA = color888(255, 0,   255),
    STATUS_CYAN    = color888(0,   255, 255),
    STATUS_WHITE   = color888(255, 255, 255),
};

void initializeStatusLED();

void setStatusLED(StatusColor color);

#endif //PROJECT_CUPSENSE_LED_H
