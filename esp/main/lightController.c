#include <stdint.h>
#include <stdbool.h>

#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"

#include "globals.h"
#include "lightController.h"
#include "controller.h"

#define MIN_DIFFERENCE ((LIGHT_CHANGE - LIGHT_MIN) * (ADC_MAX - ADC_MIN)) / (LIGHT_MAX - LIGHT_MIN) + ADC_MIN

uint8_t convertAdcToLight(int adc);

Result initLightController(
    LightController* lightController, 
    const adc_channel_t channel, 
    const adc_oneshot_unit_handle_t* handle,
    uint8_t amountOfLights,
    uint32_t* lights,
    bool isLight,
    esp_http_client_handle_t httpClient
) {
    initController(&lightController->controller, channel, handle);

    lightController->lights = malloc(sizeof(Light) * amountOfLights);

    if (lightController->lights == NULL) {
        ESP_LOGE(TAG, "Error allocating memory for Lights");
        return MEMORY_ISSUE;
    }

    for (uint8_t i = 0; i < amountOfLights; i++) {
        initLight(&lightController->lights[i], isLight, lights[i], httpClient);
    }

    lightController->amountOfLights = amountOfLights;
    
    return OK;
}

void updateLightController(LightController* lightController) {
    getNewAdc(&lightController->controller);

    if (
        lightController->controller.adcRaw <= lightController->controller.previousAdc - MIN_DIFFERENCE ||
        lightController->controller.adcRaw >= lightController->controller.previousAdc + MIN_DIFFERENCE
    ) {
        lightController->controller.previousAdc = lightController->controller.adcRaw;
        uint8_t newBrightness = convertAdcToLight(lightController->controller.adcRaw);

        for (uint8_t i = 0; i < lightController->amountOfLights; i++) {
            updateBrightness(&lightController->lights[i], newBrightness);
        }
    }
}

void freeLightController(LightController* lightController) {
    freeController(&lightController->controller);

    free(lightController);
}

uint8_t convertAdcToLight(int adc) {
    uint8_t output = ((adc - ADC_MIN) * (LIGHT_MAX - LIGHT_MIN)) / (ADC_MAX - ADC_MIN) + LIGHT_MIN;

    // Make sure off and max value can be reached
    if (output < LIGHT_MIN + LIGHT_CHANGE) {
        output = LIGHT_MIN;
    } else if (output > LIGHT_MAX - LIGHT_CHANGE) {
        output = LIGHT_MAX;
    }

    return output;
}
