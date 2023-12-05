#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "esp_adc/adc_oneshot.h"

#include "globals.h"

typedef struct Controller {
    adc_oneshot_unit_handle_t handle;
    adc_channel_t channel;
    int adcRaw;
    int previousAdc;
} Controller;

void initController(
    Controller* controller,
    const adc_channel_t channel,
    const adc_oneshot_unit_handle_t* handle
);

void getNewAdc(Controller* controller);

void freeController(Controller* controller);

#endif
