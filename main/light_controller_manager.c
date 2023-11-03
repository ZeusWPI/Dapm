#include <stdint.h>

#include "esp_adc/adc_oneshot.h"

#include "globals.h"
#include "light_controller_manager.h"
#include "light_controller.h"


Result initLightControllerManager(LightControllerManager* manager, uint8_t totalAmountOfLightControllers) {
    manager->lightControllers = malloc(sizeof(LightController) * totalAmountOfLightControllers);

    if (manager->lightControllers == NULL) {
        return MEMORY_ISSUE;
    }

    // Initiate adc handle
    adc_oneshot_unit_handle_t   handle;
    adc_oneshot_unit_init_cfg_t init_cfg = {
        .unit_id = ADC_UNIT_1
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_cfg, &handle));

    manager->handle = handle;
    manager->amountOfLightControllers = 0;
    manager->currentLightId = 0;

    return OK;
}

Result addLightController(LightControllerManager* manager, const adc_channel_t channel, const bool isLight, const uint8_t amountOfUris, const char** serverUris) {
    Result result = initLightController(
        manager->lightControllers[manager->amountOfLightControllers],
        channel,
        &manager->handle,
        isLight,
        amountOfUris,
        serverUris
    );

    if (result != OK) {
        return result;
    }
    
    manager->amountOfLightControllers++;

    return OK;
}

void UpdateLightControllers(LightControllerManager* manager) {
    for (uint8_t i = 0; i < manager->amountOfLightControllers; i++) {
        updateLightController(manager, manager->lightControllers[i]);
    }
}

void freeLightController(LightControllerManager* manager) {
    for (uint8_t i = 0; i < manager->amountOfLightControllers; i++) {
        freeLightController(manager->lightControllers[i]);
    }
    free(manager->lightControllers);

    adc_oneshot_del_unit(manager->handle);

    free(manager);
}
