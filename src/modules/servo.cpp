#include <Arduino.h>
#include <ESP32Servo.h>

#include "servo.h"
#include "config.h"

Servo servo;
portMUX_TYPE servoDataMux = portMUX_INITIALIZER_UNLOCKED;

bool servoMoving = false;
bool servoDirectionForward = true;
float distanceMoved = 0.0;
float targetPositionCM = 0.0;
unsigned long lastServoUpdate = 0;

float linearSpeed() {
    return (2 * PI * COG_RADIUS_CM) / FULL_ROTATION_TIME;
}

void initializeServo() {
    servoAttach();
    servo.writeMicroseconds(SERVO_SPEED_BACKWARDS);
    delay(5000);
    servoDetach();
}

void servoAttach() {
    servo.attach(SERVO_PIN);
}

void servoDetach() {
    servo.writeMicroseconds(SERVO_SPEED_STOP);
    delay(50);
    servo.detach();
}

void servoReset() {
    servoMoveToo(0.0);
    if (distanceMoved == 0.0) {
        servo.writeMicroseconds(SERVO_SPEED_BACKWARDS);
        delay(2000);
    }
}

void servoMoveToo(float positionCM) {
    if (positionCM > MAX_SERVO_MOVEMENT_CM) positionCM = MAX_SERVO_MOVEMENT_CM;
    if (positionCM < MIN_SERVO_MOVEMENT_CM) positionCM = MIN_SERVO_MOVEMENT_CM;

    servoAttach();

    targetPositionCM = positionCM;
    servoDirectionForward = (targetPositionCM > distanceMoved);

    const float remainingDistance = fabs(targetPositionCM - distanceMoved);

    if (remainingDistance > 0.05) {
        servoMoving = true;

        int pulse;

        // 🔹 Soft stop: slow when within 0.5 cm
        if (remainingDistance < 0.5) {
            pulse = servoDirectionForward
                        ? SERVO_SPEED_FORWARDS - 20 // slightly slower
                        : SERVO_SPEED_BACKWARDS + 20;
        } else {
            pulse = servoDirectionForward
                        ? SERVO_SPEED_FORWARDS
                        : SERVO_SPEED_BACKWARDS;
        }

        servo.writeMicroseconds(pulse);
        lastServoUpdate = millis();
    } else {
        servoDetach();
        servoMoving = false;
    }
}


void updateServoMotion() {
    if (!servoMoving) return;

    const unsigned long currentMillis = millis();
    const float deltaTime = (currentMillis - lastServoUpdate) / 1000.0;

    if (deltaTime > 0) {
        lastServoUpdate = currentMillis;

        if (servoDirectionForward) {
            distanceMoved += linearSpeed() * deltaTime;
            if (distanceMoved >= targetPositionCM) {
                distanceMoved = targetPositionCM;
                servoDetach();
                servoMoving = false;
            }
        } else {
            distanceMoved -= linearSpeed() * deltaTime;
            if (distanceMoved <= targetPositionCM) {
                distanceMoved = targetPositionCM;
                servoDetach();
                servoMoving = false;
            }
        }
        if (distanceMoved > MAX_SERVO_MOVEMENT_CM) distanceMoved = MAX_SERVO_MOVEMENT_CM;
        if (distanceMoved < MIN_SERVO_MOVEMENT_CM) distanceMoved = MIN_SERVO_MOVEMENT_CM;
    }
}

float getDistanceMoved() {
    taskENTER_CRITICAL(&servoDataMux);
    const float value = distanceMoved;
    taskEXIT_CRITICAL(&servoDataMux);
    return value;
}

bool getIsForwards() {
    taskENTER_CRITICAL(&servoDataMux);
    const bool value = servoDirectionForward;
    taskEXIT_CRITICAL(&servoDataMux);
    return value;
}

bool getIsMoving() {
    taskENTER_CRITICAL(&servoDataMux);
    const bool value = servoMoving;
    taskEXIT_CRITICAL(&servoDataMux);
    return value;
}

int getServoPulseWidth() {
    taskENTER_CRITICAL(&servoDataMux);
    const int value = servo.readMicroseconds();
    taskEXIT_CRITICAL(&servoDataMux);
    return value;
}
