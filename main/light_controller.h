#ifndef LIGHT_CONTROLLER_H
#define LIGHT_CONTROLLER_H

#include <stdint.h>

#include "types.h"
#include "controller.h"
#include "light.h"


Result initLightController(
    LightController* lightController,
    const adc_channel_t channel, 
    const adc_unit_t unit,
    const uint8_t amount_of_uris,
    const bool isLight,
    char** server_uris
);

// Get new adc and update the light value
void updateLightController(LightController* lightController);

void freeLightController(LightController* LightController);

#endif
