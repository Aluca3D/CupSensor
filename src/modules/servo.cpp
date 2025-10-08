#include <Arduino.h>
#include <ESP32Servo.h>

#include "servo.h"
#include "config.h"

Servo servo;
portMUX_TYPE servoDataMux = portMUX_INITIALIZER_UNLOCKED;

unsigned long lastUpdate = 0;
float distanceMoved = 0.0;
unsigned long currentMillis = millis();
bool isForwards = true;
float sLastUpdate = (currentMillis - lastUpdate) / 1000.0;

float linearSpeed() {
    return (2 * PI * COG_RADIUS_CM) / FULL_ROTATION_TIME;
}

void initializeServo() {
    servo.attach(SERVO_PIN);
    servo.writeMicroseconds(SERVO_SPEED_BACKWARDS);
    delay(1000);
}

// TODO: redo move servoFor/Back and/or add servoMove(int positionCM) / stopServo() / resetServo()
void moveServoForwards() {
    isForwards = true;
    if (distanceMoved < MAX_SERVO_MOVEMENT_CM) {
        lastUpdate = currentMillis;
        servo.writeMicroseconds(SERVO_SPEED_FORWARDS);
        distanceMoved += linearSpeed() * sLastUpdate;
    } else {
        servo.writeMicroseconds(SERVO_SPEED_STOP);
    }
    delay(SERVO_DELAY);
}

void moveServoBackwards() {
    isForwards = false;
    if (distanceMoved > MIN_SERVO_MOVEMENT_CM) {
        lastUpdate = currentMillis;
        servo.writeMicroseconds(SERVO_SPEED_BACKWARDS);
        distanceMoved -= linearSpeed() * sLastUpdate;
    } else {
        servo.writeMicroseconds(SERVO_SPEED_STOP);
    }
    delay(SERVO_DELAY);
}

float getDistanceMoved() {
    taskENTER_CRITICAL(&servoDataMux);
    const float value = distanceMoved;
    taskEXIT_CRITICAL(&servoDataMux);
    return value;
}

bool getIsForwards() {
    taskENTER_CRITICAL(&servoDataMux);
    const bool value = isForwards;
    taskEXIT_CRITICAL(&servoDataMux);
    return value;
}

int getServoPulseWidth() {
    taskENTER_CRITICAL(&servoDataMux);
    const int value = servo.readMicroseconds();
    taskEXIT_CRITICAL(&servoDataMux);
    return value;
}
