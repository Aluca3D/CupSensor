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
    servo.attach(SERVO_PIN);
    servoReset();
}

void servoStop() {
    servo.writeMicroseconds(SERVO_SPEED_STOP);
}

void servoMoveToo(float positionCM) {
    if (positionCM > MAX_SERVO_MOVEMENT_CM) positionCM = MAX_SERVO_MOVEMENT_CM;
    if (positionCM < MIN_SERVO_MOVEMENT_CM) positionCM = MIN_SERVO_MOVEMENT_CM;

    targetPositionCM = positionCM;

    servoDirectionForward = (targetPositionCM > distanceMoved);

    if (fabs(targetPositionCM - distanceMoved) > 0.05) {
        servoMoving = true;
        if (servoDirectionForward) {
            servo.writeMicroseconds(SERVO_SPEED_FORWARDS);
        } else {
            servo.writeMicroseconds(SERVO_SPEED_BACKWARDS);
        }
        lastServoUpdate = millis();
    } else {
        servoStop();
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
                servoStop();
                servoMoving = false;
            }
        } else {
            distanceMoved -= linearSpeed() * deltaTime;
            if (distanceMoved <= targetPositionCM) {
                distanceMoved = targetPositionCM;
                servoStop();
                servoMoving = false;
            }
        }
    }
}

void servoReset() {
    servo.writeMicroseconds(SERVO_SPEED_BACKWARDS);
    delay(5000);
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
