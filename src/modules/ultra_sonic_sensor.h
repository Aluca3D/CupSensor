#ifndef PROJECT_CUPSENSE_US_SENSOR_H
#define PROJECT_CUPSENSE_US_SENSOR_H

#include "config.h"

enum UltraSonicSensors {
    WaterEcho = US_WATER_ECHO_PIN,
    WaterTrigger = US_WATER_TRIGGER_PIN,
    HeightEcho = US_HEIGHT_ECHO_PIN,
    HeightTrigger = US_HEIGHT_TRIGGER_PIN,
};

void initializeUltraSonicSensors();

unsigned long getDistance(UltraSonicSensors TRIGGER, UltraSonicSensors ECHO);

unsigned long getAverageDistance(UltraSonicSensors TRIGGER, UltraSonicSensors ECHO);

void getCupHeight(); // Maybe better in Handler

void getFluidHeight(); // Maybe better in Handler

float echoToCm(unsigned long echoTime);

#endif //PROJECT_CUPSENSE_US_SENSOR_H
