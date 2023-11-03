#include <stdint.h>

#include "esp_log.h"

#include "types.h"
#include "light_controller.h"
#include "controller.h"
#include "light.h"


bool wifiConnected;
// Free's the current Lights and initiates a new connection
void reconstructLights(LightController* lightController);
// Get light value given an adc value
uint8_t convertAdcToLight(int adc);

// Header functions

Result initLightController(
    LightController* lightController,
    const adc_channel_t channel, 
    const adc_oneshot_unit_handle_t* handle,
    const bool isLight,
    const uint8_t amountOfUris,
    char** lightUris
) {
    // Allocate Controller
    Result result = initController(&lightController->controller, channel, handle);

    if (result != OK) {
        return result;
    }

    // Allocate lights
    lightController->lights = malloc(sizeof(Light) * amountOfUris);

    if (lightController->lights == NULL) {
        ESP_LOGE(TAG, "Error allocating memory for lights inside LightController");
        freeController(&lightController->controller);
        return MEMORY_ISSUE;
    }

    for (uint8_t i = 0; i < amountOfUris; i++) {
        result = initLight(&lightController->lights[i], isLight, lightUris[i]);
        if (result != OK) {
            // Free all previous allocated lights
            for (uint8_t j = 0; j < i; j++) {
                 freeLight(&lightController->lights[j]);
            }
            free(lightController->lights);
            freeController(&lightController->controller);

            return result;
        }
    }

    lightController->amountOfLights = amountOfUris;

    // Avoid float number for faster calculation
    int minDifference = (unsigned int) ((LIGHT_CHANGE - LIGHT_MIN) * (ADC_MAX - ADC_MIN)) / (LIGHT_MAX - LIGHT_MIN) + ADC_MIN;
    lightController->minDifference = minDifference;

    // This way of calculating when to send a keep alive message will result in sending it a bit less often than specified in SEND_ALIVE_S
    // However it's much faster than getting the current time every time

    lightController->lightUris = lightUris;

    return OK;
}

void updateLightController(LightController* lightController) {
    for (uint8_t i = 0; i < lightController->amountOfLights; i++) {
        coapProcesses(&lightController->lights[i]);
    }

    getNewAdc(&lightController->controller);
    ESP_LOGI(TAG, "Value: %d", lightController->controller.adc_raw);

    // Only update if the adc has changed enough
    if (
        lightController->controller.adc_raw <= lightController->previousAdc - lightController->minDifference ||
        lightController->controller.adc_raw >= lightController->previousAdc + lightController->minDifference
    ) {

        // Update all lights
        for (uint8_t i = 0; i < lightController->amountOfLights; i++) {
            ESP_LOGI(TAG, "Amount of lights: %d, current %d", lightController->amountOfLights, i);
            uint8_t newBrightness = convertAdcToLight(lightController->controller.adc_raw);
            Result result = updateBrightness(&lightController->lights[i], newBrightness);
            if (result == SEND_ERROR && wifiConnected) {
                // if send message didn't go through reconnect with lights (if we still have wifi connection)
                reconstructLights(lightController);
                return;
            }
        }
        lightController->previousAdc = lightController->controller.adc_raw;
        lightController->skipped = 0;

    } else {

        // Increase amount of times it was skipped
        lightController->skipped++;
        // If light got skipped more than maximum allowed of times than send a keep alive message
        if (lightController->skipped > lightController->maxSkipped) {
            for (uint8_t i = 0; i < lightController->amountOfLights; i++) {
                Result result = sendAlive(&lightController->lights[i]);
                if (result == SEND_ERROR && wifiConnected) {
                    // If send message didn't go through reconnect with lights (if we still have wifi connection)
                    reconstructLights(lightController);
                    return;
                }
            }
            lightController->skipped = 0;
        }

    }

}

void freeLightController(LightController* lightController) {
    freeController(&lightController->controller);

    for (uint8_t i = 0; i < lightController->amountOfLights; i++) {
        freeLight(&lightController->lights[i]);
    }
    free(lightController->lights);

    free(lightController);
}

// Help functions

void reconstructLights(LightController* lightController) {
    ESP_LOGE(TAG, "Reconnecting with lights...");
    bool isLight = lightController->lights[0].isLight;
    // Free existing Lights
    for (uint8_t i = 0; i < lightController->amountOfLights; i++) {
        freeLight(&lightController->lights[i]);
    }

    for (uint8_t i = 0; i < lightController->amountOfLights; i++) {
        Result result = initLight(&lightController->lights[i], isLight, lightController->lightUris[i]);
        if (result != OK) {
            // Free all previous allocated lights
            for (uint8_t j = 0; j < i; j++) {
                 freeLight(&lightController->lights[j]);
            }
        } else {
            ESP_LOGI(TAG, "Reconnected with lights");
        }
    }
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