#ifndef PROJECT_CUPSENSE_SERVO_H
#define PROJECT_CUPSENSE_SERVO_H

void initializeServo();

void moveServoForwards();

void moveServoBackwards();

float getDistanceMoved();

bool getIsForwards();

float getServoSpeed();

#endif //PROJECT_CUPSENSE_SERVO_H
