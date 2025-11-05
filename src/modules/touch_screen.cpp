#include "touch_screen.h"

#include "config.h"
#include "XPT2046_Touchscreen.h"

void initializeTouchScreen() {
    // Screen
    pinMode(SCREEN_LED_PIN, OUTPUT);
    digitalWrite(SCREEN_LED_PIN, HIGH);
    tft.begin();
    tft.setRotation(SCREEN_ROTATION);
    tft.setCursor(0, 0);
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
    tft.setCursor(button.x + 10, button.y + 15);
    tft.setTextSize(FONT_SIZE);
    tft.print(button.label);
}

void resetButton(ButtonID id) {
    const auto &button = BUTTONS[id];
    tft.fillRect(button.x, button.y, button.w, button.h, COLOR_OFF);
}

bool isScreenPressed(TS_Point &point) {
    if (!ts.touched()) return false;
    return (point.z > TOUCH_PRESSURE_PRESSED);
}

ButtonID getTouchScreenButtonPressed(TS_Point &point) {
    if (!isScreenPressed(point)) return BUTTON_COUNT;

    int x = map(point.x, TOUCH_LEFT, TOUCH_RIGHT, 0, SCREEN_WIDTH - 1);
    int y = map(point.y, TOUCH_TOP, TOUCH_BOTTOM, 0, SCREEN_HEIGHT - 1);

    x = constrain(x, 0, SCREEN_WIDTH - 1);
    y = constrain(y, 0, SCREEN_HEIGHT - 1);

    for (int i = 0; i < BUTTON_COUNT; i++) {
        const auto &button = BUTTONS[i];
        if (x >= button.x && x <= button.x + button.w &&
            y >= button.y && y <= button.y + button.h) {
            return static_cast<ButtonID>(i);
        }
    }
    return BUTTON_COUNT;
}
