#include <Arduino.h>

#include "scann_handler.h"

#include "debug_handler.h"
#include "globals.h"
#include "screen_handler.h"
#include "modules/servo.h"
#include "modules/ultra_sonic_sensor.h"
#include "state_machine/state.h"

QueueHandle_t cupFinishedFillingQueue = nullptr;
portMUX_TYPE cupRimHeightMUX = portMUX_INITIALIZER_UNLOCKED;

volatile float cupRimHeightCM = -1.0f;
volatile bool cupTooTall = false;

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

void setCupRimHeightCM(float value) {
    portENTER_CRITICAL(&cupRimHeightMUX);
    cupRimHeightCM = value;
    portEXIT_CRITICAL(&cupRimHeightMUX);
}

float getCupRimHeightCM() {
    portENTER_CRITICAL(&cupRimHeightMUX);
    const float value = cupRimHeightCM;
    portEXIT_CRITICAL(&cupRimHeightMUX);
    return value;
}

static float getAverageDistanceCm() {
    const unsigned long echoUS = getAverageDistance(HeightTrigger, HeightEcho);
    if (echoUS == 0 || echoToCm(echoUS) >= MAX_SCANN_DISTANCE_HEIGHT_CM) return -1.0f;
    return echoToCm(echoUS);
}

//TODO:
// - Test if works (and Calibrate)

bool isJumpDetected(float previous, float current) {
    if (previous < 0.0f && current > 0.0f) return true;

    if (previous > 0.0f && current < 0.0f) return true;

    if (previous > 0.0f && current > 0.0f) {
        if ((current - previous) >= RIM_JUMP_THRESHOLD_CM) return true;
    }

    return false;
}

void setCupTooTall(bool value) {
    cupTooTall = value;
}

bool getCupTooTall() {
    const bool value = cupTooTall;
    return value;
}

float scanDown(SystemState state) {
    float rimDownCM = -1.0f;
    float lastDistanceCM = -1.0f;

    servoMoveToo(MIN_SERVO_MOVEMENT_CM);
    while (getIsMoving()) vTaskDelay(pdMS_TO_TICKS(5));
    vTaskDelay(pdMS_TO_TICKS(50));

    for (float positionCM = MIN_SERVO_MOVEMENT_CM; positionCM <= MAX_SERVO_MOVEMENT_CM; positionCM += SCAN_STEP_CM) {
        if (state == STATE_ABORT || state == STATE_ERROR) break;

        servoMoveToo(positionCM);
        while (getIsMoving()) vTaskDelay(pdMS_TO_TICKS(5));

        const float distanceCM = getAverageDistanceCm();
        debugPrint(LOG_INFO, "DOWN Scan pos=%.2f cm dist=%.2f cm", positionCM, distanceCM);

        if (isJumpDetected(lastDistanceCM, distanceCM)) {
            const float differenceCM = distanceCM - lastDistanceCM;
            if (differenceCM >= RIM_JUMP_THRESHOLD_CM) {
                rimDownCM = positionCM;
                debugPrint(LOG_INFO, "RIM Detected (down) at %.2f cm (Δ=%.2f cm)", positionCM, differenceCM);
                vTaskDelay(pdMS_TO_TICKS(RIM_DEBOUNCE_MS));
                break;
            }
        }
        if (distanceCM != -1.0f) lastDistanceCM = distanceCM;
    }

    if (rimDownCM < 0.0f && lastDistanceCM > 0.0f) {
        rimDownCM = MAX_CUP_SIZE;
        setCupTooTall(true);
        debugPrint(LOG_WARNING, "Cup too tall (DOWN), using MAX_CUP_SIZE = %.2f cm", rimDownCM);
    } else if (rimDownCM < 0.0f && lastDistanceCM < 0.0f) {
        debugPrint(LOG_WARNING, "RIM (down) NOT found.");
    }

    return rimDownCM;
}

float scanUp(SystemState state) {
    float rimUpCM = -1.0f;
    float lastDistanceCM = -1.0f;
    bool firstRun = true;

    servoMoveToo(MAX_SERVO_MOVEMENT_CM);
    while (getIsMoving()) vTaskDelay(pdMS_TO_TICKS(5));
    vTaskDelay(pdMS_TO_TICKS(50));

    for (float positionCM = MAX_SERVO_MOVEMENT_CM; positionCM >= MIN_SERVO_MOVEMENT_CM; positionCM -= SCAN_STEP_CM) {
        if (state == STATE_ABORT || state == STATE_ERROR) break;

        if (getCupTooTall()) {
            rimUpCM = MAX_CUP_SIZE;
            debugPrint(LOG_WARNING, "Cup too tall (UP), using MAX_CUP_SIZE = %.2f cm", rimUpCM);
            setCupTooTall(false);
            return rimUpCM;
        }

        servoMoveToo(positionCM);
        while (getIsMoving()) vTaskDelay(pdMS_TO_TICKS(5));

        const float distanceCM = getAverageDistanceCm();
        debugPrint(LOG_INFO, "UP Scan pos=%.2f cm dist=%.2f cm", positionCM, distanceCM);

        if (isJumpDetected(lastDistanceCM, distanceCM) && !firstRun) {
            const float differenceCM = distanceCM - lastDistanceCM;
            if (differenceCM >= RIM_JUMP_THRESHOLD_CM) {
                rimUpCM = positionCM;
                debugPrint(LOG_INFO, "RIM Detected (up) at %.2f cm (Δ=%.2f cm)", positionCM, differenceCM);
                vTaskDelay(pdMS_TO_TICKS(RIM_DEBOUNCE_MS));
                break;
            }
        }

        if (distanceCM != -1.0f) {
            lastDistanceCM = distanceCM;
            firstRun = false;
        }
    }

    if (rimUpCM < 0.0f) debugPrint(LOG_WARNING, "RIM (up) NOT found.");

    return rimUpCM;
}

float calculateDifferenceCm(float rimDownPos, float rimUpPos) {
    const float avgPos = (rimDownPos + rimUpPos) / 2.0f;
    const float finalPos = avgPos - RIM_BUFFER_CM;
    return finalPos;
}

[[noreturn]] void scannCupHeight(void *parameter) {
    debugPrint(LOG_INFO, "scannCupHeight started on core %d", xPortGetCoreID());
    SystemState lastSeenState = STATE_OFF;

    for (;;) {
        const SystemState current = currentState;
        if (current != lastSeenState) {
            lastSeenState = current;

            if (current == STATE_SCANNING_HEIGHT) {
                debugPrint(LOG_INFO, "Starting cup height scan...");

                bool abort = false;

                const float rimDownPos = scanDown(current);
                const float rimUpPos = scanUp(current);

                if (current == STATE_ABORT) {
                    debugPrint(LOG_INFO, "Scann Aborted");
                    abort = true;
                }

                if (!abort) {
                    if (rimDownPos > 0.0f && rimUpPos > 0.0f) {
                        float finalSize = calculateDifferenceCm(rimDownPos, rimUpPos);

                        if (finalSize < MIN_SERVO_MOVEMENT_CM) finalSize = MIN_SERVO_MOVEMENT_CM;
                        if (finalSize > MAX_SERVO_MOVEMENT_CM) finalSize = MAX_SERVO_MOVEMENT_CM;

                        setCupRimHeightCM(finalSize);
                        debugPrint(
                            LOG_INFO,
                            "Final Position: %.2f cm (rimDownPos: %.2f cm rimUpPos: %.2f cm)",
                            finalSize, rimDownPos, rimUpPos
                        );
                    } else {
                        setCupRimHeightCM(-1.0f);
                        debugPrint(
                            LOG_WARNING,
                            "Cup height scan FAILED (rimDown=%.2f cm rimUp=%.2f cm)",
                            rimDownPos, rimUpPos
                        );
                    }
                }

                // Resetting Servo
                servoMoveToo(MIN_SERVO_MOVEMENT_CM);
                while (getIsMoving()) vTaskDelay(pdMS_TO_TICKS(5));
                vTaskDelay(pdMS_TO_TICKS(50));

                sendStateEvent(EVENT_DONE);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

// TODO: TEST!!
[[noreturn]] void scannFluidHeight(void *parameter) {
    debugPrint(LOG_INFO, "scannFluidHeight started on core %d", xPortGetCoreID());
    SystemState lastSeenState = STATE_OFF;

    for (;;) {
        const SystemState current = currentState;
        if (current != lastSeenState) {
            lastSeenState = current;

            if (current == STATE_SCANNING_FLUID_A_FILLING) {
                constexpr float positionDifferenceCM = 2.0f;
                bool scanningWaterLevel = true;

                const float baseHeightCM = echoToCm(setupHeight);
                const float actualCupRimPositionCM = positionDifferenceCM + getCupRimHeightCM();
                const float targetFillLevel = actualCupRimPositionCM + CUP_BUFFER_CM;

                debugPrint(
                    LOG_INFO,
                    "getCupRimHeightCM: %.2f cm actualCupRimPositionCM: %.2f",
                    getCupRimHeightCM(), actualCupRimPositionCM
                );

                while (scanningWaterLevel) {
                    if (currentState == STATE_ABORT) {
                        debugPrint(LOG_INFO, "Abort detected");
                        sendIsCupFull(true);
                        scanningWaterLevel = false;
                        break;
                    }
                    if (getCupRimHeightCM() < 0.0f) {
                        sendIsCupFull(true);
                        scanningWaterLevel = false;
                        break;
                    }
                    if (targetFillLevel >= baseHeightCM) {
                        sendScreenError("Error, targetFillLevel: %.2f cm is bigger then baseHeightCM: %.2f cm",
                                        targetFillLevel, baseHeightCM);
                        sendIsCupFull(true);
                        scanningWaterLevel = false;
                        break;
                    }

                    const float waterLevelCM = echoToCm(getAverageDistance(WaterTrigger, WaterEcho));
                    debugPrint(
                        LOG_INFO,
                        "WaterLevelCM: %.2f cm targetFillLevel: %.2f",
                        waterLevelCM, targetFillLevel
                    );

                    if (waterLevelCM >= targetFillLevel) {
                        debugPrint(LOG_INFO, "Cup Is full");
                        sendIsCupFull(true);
                        scanningWaterLevel = false;
                    }

                    vTaskDelay(pdMS_TO_TICKS(WATER_LEVEL_SCAN_INTERVAL_MS));
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

void createScannTask() {
    cupFinishedFillingQueue = xQueueCreate(4, sizeof(bool));
    if (cupFinishedFillingQueue == nullptr) {
        debugPrint(LOG_WARNING, "Failed to create cupFinishedFillingQueue");
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
