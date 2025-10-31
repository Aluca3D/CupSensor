#include <Arduino.h>
#include <ESP32Servo.h>

#include "servo.h"
#include "config.h"
#include "globals.h"
#include "state_machine/handlers/DebugHandler.h"

Servo servo;

portMUX_TYPE servoDataMux = portMUX_INITIALIZER_UNLOCKED;

static bool servoMoving = false;
static bool servoDirectionForward = true;
static float distanceMoved = 0.0f;
static float targetPositionCM = 0.0f;
static unsigned long lastServoUpdate = 0;


static TaskHandle_t servoTaskHandle = nullptr;

float linearSpeed() {
    return (2.0f * PI * COG_RADIUS_CM) / FULL_ROTATION_TIME;
}

void servoAttach() {
    servo.attach(SERVO_PIN);
    delay(SERVO_MOVE_INTERVALS);
}

void servoDetach() {
    servo.writeMicroseconds(SERVO_SPEED_STOP);
    delay(SERVO_MOVE_INTERVALS);
    servo.detach();
}

void servoMoveToo(float positionCM) {
    if (positionCM > MAX_SERVO_MOVEMENT_CM) positionCM = MAX_SERVO_MOVEMENT_CM;
    if (positionCM < MIN_SERVO_MOVEMENT_CM) positionCM = MIN_SERVO_MOVEMENT_CM;

    servoAttach();

    bool needToStart = false;
    int pulseToSend = SERVO_SPEED_STOP;
    {
        portENTER_CRITICAL(&servoDataMux);
        targetPositionCM = positionCM;
        const float currentDistance = distanceMoved;
        servoDirectionForward = (targetPositionCM > currentDistance);
        const float remainingDistance = fabsf(targetPositionCM - currentDistance);

        if (remainingDistance > 0.05f) {
            servoMoving = true;
            lastServoUpdate = millis();
            needToStart = true;
            pulseToSend =
                servoDirectionForward
            ? SERVO_SPEED_FORWARDS
            : SERVO_SPEED_BACKWARDS;
        } else {
            servoMoving = false;
        }
        portEXIT_CRITICAL(&servoDataMux);
    }

    if (needToStart) {
        servo.writeMicroseconds(pulseToSend);
    } else {
        servoDetach();
    }
}

[[noreturn]] static void servoTask(void *pvParameters) {
    debugPrint(LOG_INFO, "servoTask started on core %d", xPortGetCoreID());
    (void) pvParameters;

    constexpr TickType_t delayTicksWhenIdle = pdMS_TO_TICKS(50);

    for (;;) {
        bool isMovingLocal = false;
        bool isForwardLocal = true;
        float localDistance = 0.0f;
        float localTarget = 0.0f;
        unsigned long localLastUpdate = 0;

        portENTER_CRITICAL(&servoDataMux);
        isMovingLocal = servoMoving;
        isForwardLocal = servoDirectionForward;
        localDistance = distanceMoved;
        localTarget = targetPositionCM;
        localLastUpdate = lastServoUpdate;
        portEXIT_CRITICAL(&servoDataMux);

        if (!isMovingLocal) {
            vTaskDelay(delayTicksWhenIdle);
            continue;
        }

        const unsigned long nowMillis = millis();
        float deltaTime = 0.0f;
        if (nowMillis > localLastUpdate) {
            deltaTime = (nowMillis - localLastUpdate) / 1000.0f;
        } else {
            deltaTime = 0.001f;
        }

        const float speed = linearSpeed();
        float newDistance = localDistance;
        bool reachedTarget = false;

        if (isForwardLocal) {
            newDistance += speed * deltaTime;
            if (newDistance >= localTarget) {
                newDistance = localTarget;
                reachedTarget = true;
            }
        } else {
            newDistance -= speed * deltaTime;
            if (newDistance <= localTarget) {
                newDistance = localTarget;
                reachedTarget = true;
            }
        }

        if (newDistance > MAX_SERVO_MOVEMENT_CM) newDistance = MAX_SERVO_MOVEMENT_CM;
        if (newDistance < MIN_SERVO_MOVEMENT_CM) newDistance = MIN_SERVO_MOVEMENT_CM;

        bool shouldDetach = false;

        portENTER_CRITICAL(&servoDataMux);
        distanceMoved = newDistance;
        lastServoUpdate = nowMillis;

        if (reachedTarget) {
            servoMoving = false;
            shouldDetach = true;
        }
        portEXIT_CRITICAL(&servoDataMux);

        if (shouldDetach) {
            servoDetach();
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
    vTaskDelete(nullptr);
}

void createServoTask() {
    if (servoTaskHandle == nullptr) {
        xTaskCreatePinnedToCore(
            servoTask,
            "ServoTask",
            4096,
            nullptr,
            PRIORITY_NORMAL,
            &servoTaskHandle,
            CORE_ID_0
        );
    }
}

float getDistanceMoved() {
    portENTER_CRITICAL(&servoDataMux);
    const float value = distanceMoved;
    portEXIT_CRITICAL(&servoDataMux);
    return value;
}

bool getIsForwards() {
    portENTER_CRITICAL(&servoDataMux);
    const bool value = servoDirectionForward;
    portEXIT_CRITICAL(&servoDataMux);
    return value;
}

bool getIsMoving() {
    portENTER_CRITICAL(&servoDataMux);
    const bool value = servoMoving;
    portEXIT_CRITICAL(&servoDataMux);
    return value;
}

int getServoPulseWidth() {
    portENTER_CRITICAL(&servoDataMux);
    const int value = servo.readMicroseconds();
    portEXIT_CRITICAL(&servoDataMux);
    return value;
}
