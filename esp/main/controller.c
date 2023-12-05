#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"

#include "globals.h"
#include "controller.h"
#include "lightController.h"

void initController(
    Controller* controller,
    const adc_channel_t channel,
    const adc_oneshot_unit_handle_t* handle
) {
    adc_oneshot_chan_cfg_t cfg = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_11,
    };

    ESP_ERROR_CHECK(adc_oneshot_config_channel(*handle, channel, &cfg));    
    
    controller->handle   = *handle;
    controller->channel  = channel;
    controller->adcRaw  = 0;
    controller->previousAdc = 0;
}

void getNewAdc(Controller* controller) {
    esp_err_t result = adc_oneshot_read(controller->handle, controller->channel, &controller->adcRaw);
    if (result != ESP_OK) {
        ESP_LOGE(TAG, "Error getting ADC reading");
        controller->adcRaw = controller->previousAdc;
    }
}

void freeController(Controller* controller) {
    ESP_ERROR_CHECK(adc_oneshot_del_unit(controller->handle));

    free(controller);
}
