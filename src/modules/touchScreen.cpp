#include "touchScreen.h"

#include "config.h"
#include "XPT2046_Touchscreen.h"

void initializeTouchScreen() {
    // Screen
    pinMode(SCREEN_LED_PIN, OUTPUT);
    digitalWrite(SCREEN_LED_PIN, HIGH);
    tft.begin();
    tft.setRotation(SCREEN_ROTATION);
    setScreenColor(COLOR_OFF);
    // Touch
    tsSPI.begin(
        TOUCH_CLK_PIN, TOUCH_DO_PIN,
        TOUCH_DIN_PIN, TOUCH_CS_PIN
    );
    ts.begin(tsSPI);
    ts.setRotation(SCREEN_ROTATION);
}

void setScreenColor(ScreenColor color) {
    tft.fillScreen(color);
}

void drawButton(ButtonID id) {
    const auto &button = BUTTONS[id];
    tft.fillRect(button.x, button.y, button.w, button.h, button.color);
    tft.setCursor(button.x + 10, button.y + 10);
    tft.print(button.label);
}

void resetButton(ButtonID id) {
    const auto &button = BUTTONS[id];
    tft.fillRect(button.x, button.y, button.w, button.h, COLOR_OFF);
}

// TODO: Make more precise (if possible)
bool isButtonPressed(ButtonID id) {
    const auto &button = BUTTONS[id];

    if (!ts.touched()) return false;

    const TS_Point point = ts.getPoint();
    if (point.z < TOUCH_PRESSURE_PRESSED) {
        int x = map(point.x, TOUCH_LEFT, TOUCH_RIGHT, 0, SCREEN_WIDTH - 1);
        int y = map(point.y, TOUCH_TOP, TOUCH_BOTTOM, 0, SCREEN_HEIGHT - 1);

        x = constrain(x, 0, SCREEN_WIDTH - 1);
        y = constrain(y, 0, SCREEN_HEIGHT - 1);

        if (x >= button.x && x <= button.x + button.w &&
            y >= button.y && y <= button.y + button.h) {
            return true;
        }
    }
    return false;
}
