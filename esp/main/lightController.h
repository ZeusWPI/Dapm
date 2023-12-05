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
    Light* lights;
    uint8_t amountOfLights;
} LightController;

Result initLightController(
    LightController* lightController,
    const adc_channel_t channel, 
    const adc_oneshot_unit_handle_t* handle,
    uint8_t amountOfLights,
    uint32_t* lights,
    bool isLight,
    esp_http_client_handle_t httpClient
);

void updateLightController(LightController* lightController);

Result makeHttpClient(esp_http_client_handle_t* client, char* url);

void freeLightController(LightController* lightController);

#endif
