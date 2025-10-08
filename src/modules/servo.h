#ifndef PROJECT_CUPSENSE_SERVO_H
#define PROJECT_CUPSENSE_SERVO_H


extern unsigned long lastUpdate;
extern float distanceMoved;
extern bool isForwards;
extern unsigned long currentMillis;

void initializeServo();

void moveServoForwards();

void moveServoBackwards();

void stopServo();

void printDistanceMoved();

#endif //PROJECT_CUPSENSE_SERVO_H
