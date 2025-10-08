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

// TODO: Move 2 debug handler/functions
void printBackSpace(const char *text) {
    const int len = strlen(text);
    for (int i = 0; i < len; ++i) {
        Serial.print("\b");
    }
}

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

void printDistanceMoved() {
    constexpr char printLine[256] = "";
    sprintf(
        const_cast<char *>(printLine),
        "%-5f mm | isForwards %d",
        distanceMoved * 10, isForwards
    );
    Serial.print(printLine);
    printBackSpace(printLine);
}
