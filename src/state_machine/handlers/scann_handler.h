#ifndef CUPSENSOR_SCANN_HANDLER_H
#define CUPSENSOR_SCANN_HANDLER_H

void sendIsCupFull(bool cupIsFull);

bool receiveIsCupFull();

void createScannTask();

#endif //CUPSENSOR_SCANN_HANDLER_H
