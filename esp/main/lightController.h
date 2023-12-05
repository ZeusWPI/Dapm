#ifndef LIGHT_CONTROLLER_H
#define LIGHT_CONTROLLER_H

#include <stdint.h>
#include <stdbool.h>

#include "esp_adc/adc_oneshot.h"
#include "esp_http_client.h"

#include "globals.h"
#include "controller.h"
#include "light.h"

typedef struct LightController {
    Controller controller;
    Light light;
} LightController;

void initLightController(
    LightController* lightController,
    const adc_channel_t channel, 
    const adc_oneshot_unit_handle_t* handle,
    uint32_t* lights,
    esp_http_client_handle_t* httpClient
);

Result updateLightController(LightController* lightController);

void freeLightController(LightController* lightController);

#endif
