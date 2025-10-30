#include <Arduino.h>

#include "initializing.h"

#include "config.h"
#include "ESP32Servo.h"
#include "globals.h"
#include "modules/servo.h"
#include "modules/ultra_sonic_sensor.h"
#include "state_machine/state.h"

unsigned long setupHeight = 0;

[[noreturn]] void initializingTask(void *parameters) {
    SystemState lastSeenState = STATE_OFF;

    for (;;) {
        SystemState current = currentState;
        if (current != lastSeenState) {
            lastSeenState = current;

            if (current == STATE_INITIALIZING) {
                // Reset Servo Height TODO: Check if needed
                servoAttach();
                servo.writeMicroseconds(SERVO_SPEED_BACKWARDS);
                delay(100);
                servo.writeMicroseconds(SERVO_SPEED_STOP);
                servoDetach();
                // Set Init Variables
                setupHeight = getAverageDistance(WaterTrigger, WaterEcho);
                sendStateEvent(EVENT_DONE);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void createInitTask() {
    xTaskCreatePinnedToCore(
        &initializingTask,
        "initializingTask",
        2048,
        nullptr,
        PRIORITY_NORMAL,
        nullptr,
        CORE_ID_0
    );
}
