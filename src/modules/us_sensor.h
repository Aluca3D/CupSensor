#ifndef PROJECT_CUPSENSE_US_SENSOR_H
#define PROJECT_CUPSENSE_US_SENSOR_H

void initialize_us_sensor(int TRIGGER_PIN, int ECHO_PIN);

void activate_trigger(int TRIGGER_PIN);

long get_echo(int ECHO_PIN);

#endif //PROJECT_CUPSENSE_US_SENSOR_H
