#ifndef LIGHT_CONTROLLER_MANAGER_H
#define LIGHT_CONTROLLER_MANAGER_H


#include <stdint.h>

#include "esp_adc/adc_oneshot.h"

#include "globals.h"
#include "light_controller.h"


typedef struct LightControllerManager {
    LightController* lightControllers;      // Holds all LightControllers
    uint8_t amountOfLightControllers;       // Amount of LightControllers in lightControllers
    uint8_t currentLightId;                 // Id of the light that currently has a session open
    adc_oneshot_unit_handle_t handle;       // Only supports adc unit 1
} LightControllerManager;


Result initLightControllerManager(LightControllerManager* manager, const uint8_t totalAmountOfLightControllers);

// Add a new Light Controller
Result addLightController(
    LightControllerManager manager*, 
    const adc_channel_t channel,
    const bool isLight;
    const uint8_t amountOfUris;
    const char** serverUris;
);

void UpdateLightControllers(LightControllerManager* manager);

void freeLightController(LightControllerManager* manager);


#endif
