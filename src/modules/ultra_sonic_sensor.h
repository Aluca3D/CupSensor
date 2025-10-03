#ifndef PROJECT_CUPSENSE_US_SENSOR_H
#define PROJECT_CUPSENSE_US_SENSOR_H

void initializeUltraSonicSensor(int TRIGGER_PIN, int ECHO_PIN);

long getDistance(int TRIGGER_PIN, int ECHO_PIN);

#endif //PROJECT_CUPSENSE_US_SENSOR_H
