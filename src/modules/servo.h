#ifndef PROJECT_CUPSENSE_SERVO_H
#define PROJECT_CUPSENSE_SERVO_H

#include <Arduino.h>

extern portMUX_TYPE servoDataMux;

void initializeServo();

void servoMoveToo(float positionCM);

void servoStop();

void updateServoMotion();

void servoReset();

float getDistanceMoved();

bool getIsForwards();

bool getIsMoving();

int getServoPulseWidth();

#endif //PROJECT_CUPSENSE_SERVO_H
