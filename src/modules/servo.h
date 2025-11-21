#ifndef PROJECT_CUPSENSE_SERVO_H
#define PROJECT_CUPSENSE_SERVO_H

#include <ESP32Servo.h>

extern Servo servo;

void servoMoveToo(float positionCM);

void servoAttach();

void servoDetach();

void createServoTask();

float getServoPosition();

bool getIsForwards();

bool getIsMoving();

int getServoPulseWidth();

#endif //PROJECT_CUPSENSE_SERVO_H
