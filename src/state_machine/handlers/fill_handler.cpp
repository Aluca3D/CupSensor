#include "fill_handler.h"

#include "debug_handler.h"
#include "modules/touch_screen.h"

QueueHandle_t buttonQueue = nullptr;

void sendPressedButton(ButtonID buttonPressed) {
    if (buttonQueue && buttonPressed != BUTTON_COUNT) {
        debugPrint(LOG_DEBUG, "Sending button pressed %d", buttonPressed);
        xQueueSend(buttonQueue, &buttonPressed, 0);
    }
}

ButtonID receivePressedButton() {
    ButtonID buttonPressed = BUTTON_COUNT;
    if (xQueueReceive(buttonQueue, &buttonPressed, portMAX_DELAY)) {
        debugPrint(LOG_DEBUG, "Received button pressed %d", buttonPressed);
        return buttonPressed;
    }
    return BUTTON_COUNT;
}

void createFillTask() {
    buttonQueue = xQueueCreate(BUTTON_COUNT, sizeof(ButtonID));
    if (!buttonQueue) {
        debugPrint(LOG_ERROR, "Failed to create button queue\n");
        while (true) {
        }
    }

    // TODO: add xTask
}
