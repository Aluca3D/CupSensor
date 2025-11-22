#include "touch_screen.h"

#include "config.h"
#include "XPT2046_Touchscreen.h"
#include "state_machine/handlers/debug_handler.h"

bool activeScreenTextRegionsCount[TEXT_REGION_COUNT] = {false, false};
std::array<ButtonID, MAX_ACTIVE_BUTTONS> activeButtons{};
size_t activeButtonsCount = 0;

void initializeTouchScreen() {
    // Screen
    pinMode(SCREEN_LED_PIN, OUTPUT);
    digitalWrite(SCREEN_LED_PIN, HIGH);
    tft.begin();
    tft.setRotation(SCREEN_ROTATION);
    tft.setCursor(0, 0);
    setScreenColor(COLOR_BLACK);
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

    const int16_t x_center = button.x + (button.w / 2) - (strlen(button.label) * 6 * FONT_SIZE) / 2;
    const int16_t y_center = button.y + (button.h / 2) - (8 * FONT_SIZE) / 2;

    tft.fillRect(button.x, button.y, button.w, button.h, button.buttonColor);
    tft.setCursor(x_center, y_center);
    tft.setTextSize(FONT_SIZE);
    tft.setTextColor(button.fontColor);
    tft.print(button.label);
}

void resetButton(ButtonID id) {
    const auto &button = BUTTONS[id];
    tft.fillRect(button.x, button.y, button.w, button.h, COLOR_BLACK);
}

void drawTextRegion(TextRegionID id, const TextRegion &r, const char *txt) {
    activeScreenTextRegionsCount[id] = true;
    tft.fillRect(r.x, r.y, r.w, r.h, COLOR_BLACK);
    tft.setTextSize(FONT_SIZE);
    tft.setTextColor(r.textColor);

    constexpr int char_w = 6 * FONT_SIZE; // default 5x7 font = 6 wide incl spacing
    constexpr int char_h = 8 * FONT_SIZE;

    const int max_chars_per_line = r.w / char_w;
    const int max_lines = r.h / char_h;

    int cursor_x = r.x;
    int cursor_y = r.y;

    int line_chars = 0;
    int lines_used = 1;

    const char *p = txt;

    while (*p && lines_used <= max_lines) {
        if (*p == '\n') {
            cursor_y += char_h;
            cursor_x = r.x;
            line_chars = 0;
            lines_used++;
            p++;
            continue;
        }

        if (*p == ' ') {
            const char *w = p + 1;
            int word_len = 0;
            while (*w && *w != ' ' && *w != '\n')
                word_len++, w++;

            if (line_chars + 1 + word_len > max_chars_per_line) {
                cursor_y += char_h;
                cursor_x = r.x;
                line_chars = 0;
                lines_used++;
                if (lines_used > max_lines)
                    break;
            }
        }

        if (line_chars >= max_chars_per_line) {
            cursor_y += char_h;
            cursor_x = r.x;
            line_chars = 0;
            lines_used++;
            if (lines_used > max_lines)
                break;
        }

        tft.setCursor(cursor_x, cursor_y);
        tft.print(*p);

        cursor_x += char_w;
        line_chars++;

        p++;
    }
}

void resetTextRegion(TextRegionID id, const TextRegion &r) {
    activeScreenTextRegionsCount[id] = false;
    tft.fillRect(r.x, r.y, r.w, r.h, COLOR_BLACK);
}

void resetScreen() {
    for (int i = 0; i < TEXT_REGION_COUNT; i++) {
        if (activeScreenTextRegionsCount[i]) {
            debugPrint(LOG_INFO, "Resetting Text Region: %d", i);

            switch (i) {
                case TITLE_TXT: resetTextRegion(TITLE_TXT, TITLE_REGION);
                    break;
                case ERROR_TXT: resetTextRegion(ERROR_TXT, ERROR_REGION);
                    break;
                default: break;
            }
        }
    }
    if (activeButtonsCount == 0) {
        debugPrint(LOG_WARNING, "resetScreen called but no active buttons");
        return;
    }
    for (size_t i = 0; i < activeButtonsCount; i++) {
        const ButtonID id = activeButtons[i];
        debugPrint(LOG_INFO, "Resetting Button: %d", id);
        resetButton(id);
    }
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

    for (size_t i = 0; i < activeButtonsCount; i++) {
        const ButtonID id = activeButtons[i];
        const auto &button = BUTTONS[id];
        if (x >= button.x && x <= button.x + button.w &&
            y >= button.y && y <= button.y + button.h) {
            return id;
        }
    }
    return BUTTON_COUNT;
}
