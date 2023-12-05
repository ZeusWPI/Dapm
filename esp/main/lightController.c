#include <stdint.h>
#include <stdbool.h>

#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"

#include "globals.h"
#include "lightController.h"
#include "controller.h"

#define MIN_DIFFERENCE ((LIGHT_CHANGE - LIGHT_MIN) * (ADC_MAX - ADC_MIN)) / (LIGHT_MAX - LIGHT_MIN) + ADC_MIN

uint8_t convertAdcToLight(int adc);

void initLightController(
    LightController* lightController, 
    const adc_channel_t channel, 
    const adc_oneshot_unit_handle_t* handle,
    uint32_t* lights,
    esp_http_client_handle_t* httpClient
) {
    initController(&lightController->controller, channel, handle);
    initLight(&lightController->light, lights, httpClient);
}

Result updateLightController(LightController* lightController) {
    getNewAdc(&lightController->controller);

    if (
        lightController->controller.adcRaw <= lightController->controller.previousAdc - MIN_DIFFERENCE ||
        lightController->controller.adcRaw >= lightController->controller.previousAdc + MIN_DIFFERENCE
    ) {
        lightController->controller.previousAdc = lightController->controller.adcRaw;
        uint8_t newBrightness = convertAdcToLight(lightController->controller.adcRaw);

        Result result = updateBrightness(&lightController->light, newBrightness);
        if (result != OK) {
            if (! wifiConnected) {
                return NO_WIFI;
            } else {
                return result;
            }
        }
    }

    return OK;
}

void freeLightController(LightController* lightController) {
    freeController(&lightController->controller);
    freeLight(&lightController->light);

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
