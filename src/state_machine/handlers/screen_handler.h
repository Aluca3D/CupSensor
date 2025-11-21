#ifndef CUPSENSOR_SCREENHANDLER_H
#define CUPSENSOR_SCREENHANDLER_H

void createUpdateScreenTask();

void sendScreenError(const char *fmt, ...);

struct ScreenErrorMessage {
    char text[64];
};

#endif //CUPSENSOR_SCREENHANDLER_H
