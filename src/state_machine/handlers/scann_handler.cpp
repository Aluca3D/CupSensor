#include <Arduino.h>

#include "scann_handler.h"

#include "debug_handler.h"
#include "globals.h"
#include "modules/servo.h"
#include "modules/ultra_sonic_sensor.h"
#include "state_machine/state.h"

QueueHandle_t cupFinishedFillingQueue = nullptr;

volatile float cupHeightCM = -1.0f;

void sendIsCupFull(bool cupIsFull) {
    if (!cupFinishedFillingQueue) return;

    debugPrint(LOG_INFO, "Attempting sending cupIsFull: %d", cupIsFull);
    if (xQueueSend(cupFinishedFillingQueue, &cupIsFull, pdMS_TO_TICKS(100)) != pdPASS) {
        debugPrint(LOG_WARNING, "Failed to send cupIsFull (queue full)");
    } else {
        debugPrint(LOG_INFO, "Succeeded in sending cupIsFull: %d", cupIsFull);
    }
}

bool receiveIsCupFull() {
    if (!cupFinishedFillingQueue) return false;

    bool cupIsFull = false;
    if (xQueueReceive(cupFinishedFillingQueue, &cupIsFull, pdMS_TO_TICKS(100))) {
        debugPrint(LOG_INFO, "Received cupIsFull: %d", cupIsFull);
        return cupIsFull;
    }
    return false;
}

static float getAverageDistanceCm() {
    unsigned long echoUS = getAverageDistance(HeightTrigger, HeightEcho);
    if (echoUS == 0) return -1.0f;
    return echoToCm(echoUS);
}

//TODO Test if works
[[noreturn]] void scannCupHeight(void *parameter) {
    debugPrint(LOG_INFO, "scannCupHeight started on core %d", xPortGetCoreID());
    SystemState lastSeenState = STATE_OFF;

    for (;;) {
        const SystemState current = currentState;
        if (current != lastSeenState) {
            lastSeenState = current;

            if (current == STATE_SCANNING_HEIGHT) {
                debugPrint(LOG_INFO, "Starting cup height scan...");

                float rimDownPos = -1.0f;
                float rimUpPos = -1.0f;

                servoMoveToo(MIN_SERVO_MOVEMENT_CM);
                while (getIsMoving()) vTaskDelay(pdMS_TO_TICKS(5));
                vTaskDelay(pdMS_TO_TICKS(50));

                float lastDist = -1.0f;
                for (float pos = MIN_SERVO_MOVEMENT_CM; pos < MAX_SERVO_MOVEMENT_CM; pos += SCAN_STEP_CM) {
                    servoMoveToo(pos);
                    while (getIsMoving()) vTaskDelay(pdMS_TO_TICKS(5));

                    const float distCm = getAverageDistanceCm();
                    debugPrint(LOG_INFO, "DOWN Scan pos=%.2f cm dist=%.2f cm", pos, distCm);

                    if (distCm > 0 && lastDist > 0) {
                        const float diff = distCm - lastDist;
                        if (diff >= RIM_JUMP_THRESHOLD_CM) {
                            rimDownPos = pos;
                            debugPrint(LOG_INFO, "RIM Detected (down) at %.2f cm (Δ=%.2f)", pos, diff);
                            vTaskDelay(pdMS_TO_TICKS(RIM_DEBOUNCE_MS));
                            break;
                        }
                    }
                    if (distCm > 0) lastDist = distCm;
                    vTaskDelay(pdMS_TO_TICKS(10));
                }
                if (rimDownPos < 0.0f) {
                    debugPrint(LOG_WARNING, "RIM (down) NOT found.");
                }

                servoMoveToo(MAX_SERVO_MOVEMENT_CM);
                while (getIsMoving()) vTaskDelay(pdMS_TO_TICKS(5));
                vTaskDelay(pdMS_TO_TICKS(50));

                lastDist = -1.0f;
                for (float pos = MAX_SERVO_MOVEMENT_CM; pos >= MIN_SERVO_MOVEMENT_CM; pos -= SCAN_STEP_CM) {
                    servoMoveToo(pos);
                    while (getIsMoving()) vTaskDelay(pdMS_TO_TICKS(5));

                    const float distCm = getAverageDistanceCm();
                    debugPrint(LOG_INFO, "UP Scan pos=%.2f cm dist=%.2f cm", pos, distCm);

                    if (distCm > 0 && lastDist > 0) {
                        const float diff = distCm - lastDist;
                        if (diff >= RIM_JUMP_THRESHOLD_CM) {
                            rimUpPos = pos;
                            debugPrint(LOG_INFO, "RIM Detected (UP) at %.2f cm (Δ=%.2f)", pos, diff);
                            vTaskDelay(pdMS_TO_TICKS(RIM_DEBOUNCE_MS));
                            break;
                        }
                    }
                    if (distCm > 0) lastDist = distCm;
                    vTaskDelay(pdMS_TO_TICKS(10));
                }
                if (rimUpPos < 0.0f) {
                    debugPrint(LOG_WARNING, "RIM (up) NOT found.");
                }

                if (rimDownPos > 0.0f && rimUpPos > 0.0f) {
                    const float avgPos = (rimDownPos + rimUpPos) * 0.5f;
                    float finalPos = avgPos - RIM_BUFFER_CM;
                    if (finalPos < MIN_SERVO_MOVEMENT_CM) finalPos = MIN_SERVO_MOVEMENT_CM;
                    if (finalPos > MAX_SERVO_MOVEMENT_CM) finalPos = MAX_SERVO_MOVEMENT_CM;

                    portENTER_CRITICAL(&servoDataMux);
                    cupHeightCM = finalPos;
                    portEXIT_CRITICAL(&servoDataMux);

                    debugPrint(LOG_INFO, "Cup height (servo pos) = %.2f cm (avg=%.2f buf=%.2f)",
                               finalPos, avgPos, RIM_BUFFER_CM);
                } else {
                    portENTER_CRITICAL(&servoDataMux);
                    cupHeightCM = -1.0f;
                    portEXIT_CRITICAL(&servoDataMux);
                    debugPrint(LOG_WARNING, "Cup height scan FAILED (rimDown=%.2f rimUp=%.2f)", rimDownPos, rimUpPos);
                }

                sendStateEvent(EVENT_DONE);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }
    vTaskDelete(nullptr);
}

[[noreturn]] void scannFluidHeight(void *parameter) {
    debugPrint(LOG_INFO, "scannFluidHeight started on core %d", xPortGetCoreID());
    SystemState lastSeenState = STATE_OFF;

    for (;;) {
        const SystemState current = currentState;
        if (current != lastSeenState) {
            lastSeenState = current;

            if (current == STATE_SCANNING_FLUID_A_FILLING) {
                // TODO: Checker Logic that checks CupHeight and Fluid Level
                sendIsCupFull(true);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }
    vTaskDelete(nullptr);
}

void createScannTask() {
    cupFinishedFillingQueue = xQueueCreate(4, sizeof(bool));
    if (cupFinishedFillingQueue == nullptr) {
        debugPrint(LOG_ERROR, "Failed to create cupFinishedFillingQueue");
        while (true) {
        }
    }

    xTaskCreatePinnedToCore(
        scannCupHeight,
        "scannCupHeight",
        STACK_SIZE_LARGE,
        nullptr,
        PRIORITY_NORMAL,
        nullptr,
        CORE_ID_0
    );
    xTaskCreatePinnedToCore(
        scannFluidHeight,
        "scannFluidHeight",
        STACK_SIZE_LARGE,
        nullptr,
        PRIORITY_HIGH,
        nullptr,
        CORE_ID_0
    );
}
