#ifndef CONTROLLER_C
#define CONTROLLER_C

#include "esp_adc/adc_oneshot.h"

#include "../globals.h"


// Struct for the data regarding the rotating things
typedef struct Controller {
    adc_oneshot_unit_handle_t handle;
    adc_channel_t channel;
    int adc_raw;
} Controller;


Result initController(Controller* controller, const adc_channel_t channel, adc_oneshot_unit_handle_t* handle);

// Update adc value
void getNewAdc(Controller* controller);

void freeController(Controller* controller);


#endif
