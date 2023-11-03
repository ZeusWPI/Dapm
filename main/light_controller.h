#ifndef LIGHT_CONTROLLER_H
#define LIGHT_CONTROLLER_H

#include <stdint.h>

#include "esp_adc/adc_oneshot.h"

#include "globals.h"
#include "light_controller_manager.h"


typedef struct LightController {
    Controller controller;
    Light* lights;
    uint8_t amountOfLights;
    int minDifference;          // Minimum difference in ADC value before updating the lights
    int64_t lastUpdated;
    char** lightUris;           // All uri's for the lights
} LightController;


Result initLightController(
    LightController* lightController,
    const adc_channel_t channel, 
    const adc_oneshot_unit_handle_t* handle,
    const bool isLight,
    const uint8_t amountOfUris,
    const char** serverUris
);

// Get new adc and update the light value
void updateLightController(LightControllerManager* manager, LightController* lightController);

void freeLightController(LightController* LightController);

#endif
