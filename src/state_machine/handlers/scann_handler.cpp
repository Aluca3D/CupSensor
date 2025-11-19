#include <Arduino.h>

#include "scann_handler.h"

#include "debug_handler.h"
#include "globals.h"
#include "modules/servo.h"
#include "modules/ultra_sonic_sensor.h"
#include "state_machine/state.h"

QueueHandle_t cupFinishedFillingQueue = nullptr;
portMUX_TYPE cupHeightMux = portMUX_INITIALIZER_UNLOCKED;

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

void setCupHeightCM(float value) {
    portENTER_CRITICAL(&cupHeightMux);
    cupHeightCM = value;
    portEXIT_CRITICAL(&cupHeightMux);
}

float getCupHeightCM() {
    portENTER_CRITICAL(&cupHeightMux);
    const float value = cupHeightCM;
    portEXIT_CRITICAL(&cupHeightMux);
    return value;
}

static float getAverageDistanceCm() {
    const unsigned long echoUS = getAverageDistance(HeightTrigger, HeightEcho);
    if (echoUS == 0 || echoToCm(echoUS) >= MAX_SCANN_DISTANCE_HEIGHT_CM) return -1.0f;
    return echoToCm(echoUS);
}

//TODO:
// - Test if works (and Calibrate)
// - Add/Check if Cup is too tall into scanUp (optional)

bool isJumpDetected(float previous, float current) {
    if (previous < 0.0f && current > 0.0f) return true;

    if (previous > 0.0f && current < 0.0f) return true;

    if (previous > 0.0f && current > 0.0f) {
        if ((current - previous) >= RIM_JUMP_THRESHOLD_CM) return true;
    }

    return false;
}

float scanDown(SystemState state) {
    float rimDownCM = -1.0f;
    float lastDistanceCM = -1.0f;

    servoMoveToo(MIN_SERVO_MOVEMENT_CM);
    while (getIsMoving()) vTaskDelay(pdMS_TO_TICKS(5));
    vTaskDelay(pdMS_TO_TICKS(50));

    for (float positionCM = MIN_SERVO_MOVEMENT_CM; positionCM <= MAX_SERVO_MOVEMENT_CM; positionCM += SCAN_STEP_CM) {
        if (state == STATE_ABORT) break;

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
        if (state == STATE_ABORT) break;

        servoMoveToo(positionCM);
        while (getIsMoving()) vTaskDelay(pdMS_TO_TICKS(5));

        const float distanceCM = getAverageDistanceCm();
        debugPrint(LOG_INFO, "UP Scan pos=%.2f cm dist=%.2f cm", positionCM, distanceCM);

        if (!firstRun && isJumpDetected(lastDistanceCM, distanceCM)) {
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

                        setCupHeightCM(finalSize);


                        debugPrint(
                            LOG_INFO,
                            "Final Position: %.2f cm (rimDownPos: %.2f cm rimUpPos: %.2f cm)",
                            finalSize, rimDownPos, rimUpPos
                        );
                    } else {
                        setCupHeightCM(-1.0f);
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
    vTaskDelete(nullptr);
}

//TODO:
// - Doesnt Work
[[noreturn]] void scannFluidHeight(void *parameter) {
    debugPrint(LOG_INFO, "scannFluidHeight started on core %d", xPortGetCoreID());
    SystemState lastSeenState = STATE_OFF;

    for (;;) {
        const SystemState current = currentState;
        if (current != lastSeenState) {
            lastSeenState = current;

            if (current == STATE_SCANNING_FLUID_A_FILLING) {
                const float baseHeightCM = echoToCm(setupHeight);
                constexpr float positionDifferenceCM = 1.0f; // Check

                const float actualSensorPositionCm = baseHeightCM - positionDifferenceCM;
                const float waterLevelCm = echoToCm(getAverageDistance(WaterTrigger, WaterEcho)) - 1.0f; // Check

                if (actualSensorPositionCm >= waterLevelCm) {
                    sendIsCupFull(true);
                }
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
