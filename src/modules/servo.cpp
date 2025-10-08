#include <Arduino.h>
#include <ESP32Servo.h>

#include "servo.h"
#include "config.h"

Servo servo;

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
}

void moveServoForwards() {
    if (distanceMoved < MAX_SERVO_MOVEMENT_CM) {
        lastUpdate = currentMillis;
        isForwards = true;
        servo.write(180 - SERVO_SPEED);
        distanceMoved += linearSpeed() * sLastUpdate;
    } else {
        stopServo();
    }
}

void moveServoBackwards() {
    if (distanceMoved > MIN_SERVO_MOVEMENT_CM) {
        lastUpdate = currentMillis;
        isForwards = false;
        servo.write(SERVO_SPEED);
        distanceMoved -= linearSpeed() * sLastUpdate;
    } else {
        stopServo();
    }
}

void stopServo() {
    servo.write(90);
}

float getDistanceMoved() {
    return distanceMoved * 10;
}

bool getIsForwards() {
    return isForwards;
}
