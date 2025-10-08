#ifndef PROJECT_CUPSENSE_SERVO_H
#define PROJECT_CUPSENSE_SERVO_H

#include <Arduino.h>

extern portMUX_TYPE servoDataMux;

void initializeServo();

void moveServoForwards();

void moveServoBackwards();

float getDistanceMoved();

bool getIsForwards();

int getServoPulseWidth();

#endif //PROJECT_CUPSENSE_SERVO_H
