#include <stdlib.h>

#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"

#include "types.h"
#include "controller.h"


Result initController(Controller* controller, const adc_channel_t channel, const adc_unit_t unit) {
    // ADC init
    adc_oneshot_unit_handle_t   handle;
    adc_oneshot_unit_init_cfg_t init_cfg = {
        .unit_id = unit
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_cfg, &handle));

    // ADC config
    adc_oneshot_chan_cfg_t cfg = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_11,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(handle, channel, &cfg));    
    
    controller->handle   = handle;
    controller->channel  = channel;
    controller->adc_raw  = 0;
    
    return OK;
}

void getNewAdc(Controller* controller) {
    int previousAdc  = controller->adc_raw;
    esp_err_t result = adc_oneshot_read(controller->handle, controller->channel, &controller->adc_raw);
    if (result != ESP_OK) {
        ESP_LOGE(TAG, "Error getting ADC reading");
        controller->adc_raw = previousAdc;
    }
}

void freeController(Controller* controller) {
    ESP_ERROR_CHECK(adc_oneshot_del_unit(controller->handle));
}
