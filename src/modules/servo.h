#ifndef PROJECT_CUPSENSE_SERVO_H
#define PROJECT_CUPSENSE_SERVO_H

#include <Arduino.h>

extern portMUX_TYPE servoDataMux;

void servoMoveToo(float positionCM);

void servoAttach();

void servoDetach();

void createServoTask();

float getDistanceMoved();

bool getIsForwards();

bool getIsMoving();

int getServoPulseWidth();

#endif //PROJECT_CUPSENSE_SERVO_H
