#ifndef CUPSENSOR_TOUCHSCREEN_H
#define CUPSENSOR_TOUCHSCREEN_H

#include <Arduino.h>

#include <Adafruit_ILI9341.h>
#include <SPI.h>
#include <XPT2046_Touchscreen.h>

extern Adafruit_ILI9341 tft;
extern SPIClass tsSPI;
extern XPT2046_Touchscreen ts;

constexpr uint16_t color565(uint8_t r, uint8_t g, uint8_t b) {
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

enum ScreenColor {
    COLOR_BLACK = color565(0, 0, 0),
    COLOR_RED = color565(255, 0, 0),
    COLOR_GREEN = color565(0, 255, 0),
    COLOR_BLUE = color565(0, 0, 255),
    COLOR_YELLOW = color565(255, 255, 0),
    COLOR_MAGENTA = color565(255, 0, 255),
    COLOR_CYAN = color565(0, 255, 255),
    COLOR_WHITE = color565(255, 255, 255),
};

struct ButtonRect {
    const char *label;
    int16_t x, y, w, h;
    ScreenColor buttonColor;
    ScreenColor fontColor;
};

enum ButtonID {
    FLUID_0,
    FLUID_1,
    FLUID_2,
    ABORT,
    CONTINUE,
    BUTTON_COUNT // Always Last
};

// TODO: Refine/Test all areas and button positions
constexpr ButtonRect BUTTONS[BUTTON_COUNT] = {
    {"Fluid 0", 15, 20, 90, 150, COLOR_BLUE, COLOR_BLACK},
    {"Fluid 1", 115, 20, 90, 150, COLOR_YELLOW, COLOR_BLACK},
    {"Fluid 2", 215, 20, 90, 150, COLOR_CYAN, COLOR_BLACK},

    {"Abort", 200, 100, 80, 80, COLOR_RED, COLOR_BLACK},
    {"Continue", 200, 100, 80, 80, COLOR_GREEN, COLOR_BLACK}
};

struct TextRegion {
    int16_t x, y, w, h;
    ScreenColor textColor;
};

enum TextRegionID {
    TITLE_TXT,
    ERROR_TXT,
    TEXT_REGION_COUNT
};

bool activeTextRegions[TEXT_REGION_COUNT] = {false, false};
constexpr TextRegion TITLE_REGION = {0, 0, 320, 24, COLOR_WHITE};
constexpr TextRegion ERROR_REGION = {0, 216, 320, 24, COLOR_RED};

constexpr size_t MAX_ACTIVE_BUTTONS = BUTTON_COUNT;

extern std::array<ButtonID, MAX_ACTIVE_BUTTONS> activeButtons;
extern size_t activeButtonsCount;

void initializeTouchScreen();

void setScreenColor(ScreenColor color);

void drawButton(ButtonID id);

void resetButton(ButtonID id);

void drawTextRegion(TextRegionID id, const TextRegion &r, const char *txt);

void resetTextRegion(TextRegionID id, const TextRegion &r);

void resetScreen();

bool isScreenPressed(TS_Point &point);

ButtonID getTouchScreenButtonPressed(TS_Point &point);;

#endif //CUPSENSOR_TOUCHSCREEN_H
