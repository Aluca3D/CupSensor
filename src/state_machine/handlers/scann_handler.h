#ifndef CUPSENSOR_SCANN_HANDLER_H
#define CUPSENSOR_SCANN_HANDLER_H

#include <Arduino.h>

extern QueueHandle_t cupFinishedFillingQueue;

extern portMUX_TYPE cupHeightMux;

void sendIsCupFull(bool cupIsFull);

bool receiveIsCupFull();

void createScannTask();

#endif //CUPSENSOR_SCANN_HANDLER_H