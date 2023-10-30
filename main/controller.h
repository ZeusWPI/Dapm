#ifndef CONTROLLER_C
#define CONTROLLER_C

#include "esp_adc/adc_oneshot.h"

#include "types.h"


Result initController(Controller* controller, const adc_channel_t channel, const adc_unit_t unit);

// Update adc value
void getNewAdc(Controller* controller);

void freeController(Controller* controller);


#endif
