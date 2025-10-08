#ifndef PROJECT_CUPSENSE_SERVO_H
#define PROJECT_CUPSENSE_SERVO_H

void initializeServo();

void moveServoForwards();

void moveServoBackwards();

void stopServo();

float getDistanceMoved();

bool getIsForwards();

#endif //PROJECT_CUPSENSE_SERVO_H
