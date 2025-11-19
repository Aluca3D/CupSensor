#include <Arduino.h>

#include "initializing.h"

#include "config.h"
#include "debug_handler.h"
#include "ESP32Servo.h"
#include "globals.h"
#include "modules/servo.h"
#include "modules/ultra_sonic_sensor.h"
#include "state_machine/state.h"

unsigned long setupHeight = 0;

[[noreturn]] void initializingTask(void *parameters) {
    debugPrint(LOG_INFO, "initializingTask started on core %d", xPortGetCoreID());
    SystemState lastSeenState = STATE_OFF;

    for (;;) {
        const SystemState current = currentState;
        if (current != lastSeenState) {
            lastSeenState = current;

            if (current == STATE_INITIALIZING) {
                // Reset Servo Height
                servoAttach();
                servo.writeMicroseconds(SERVO_SPEED_BACKWARDS);
                delay(100);
                servo.writeMicroseconds(SERVO_SPEED_STOP);
                servoDetach();
                // Set Init Variables
                setupHeight = getAverageDistance(WaterTrigger, WaterEcho);
                const float setupHeightCm = echoToCm(setupHeight);
                debugPrint(LOG_DEBUG, "setupHeight Set to: %d Echo, %.2f cm", setupHeight, setupHeightCm);
                sendStateEvent(EVENT_DONE);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }
    vTaskDelete(nullptr);
}

void createInitTask() {
    xTaskCreatePinnedToCore(
        &initializingTask,
        "initializingTask",
        STACK_SIZE_LARGE,
        nullptr,
        PRIORITY_NORMAL,
        nullptr,
        CORE_ID_0
    );
}
