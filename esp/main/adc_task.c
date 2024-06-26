#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"
#include "esp_event.h"


#include <stdatomic.h>


static const char* TAG = "adc_task";

#define POTENTIOMETERS (8)
#define WAIT_MS                     10      // ms between ADC readings
#define ADC_MAX                     8191    // Maximum ADC value
#define ADC_MIN                     0       // Minimum ADC value
#define LIGHT_MAX                   254     // Maximum light value
#define LIGHT_MIN                   0       // Minimum light value
#define MIN_MAX_SNAPPING            10

#define LIGHT_CHANGE                3      // Amount that the light value needs to change before sending it


adc_channel_t channels[POTENTIOMETERS] = {
    ADC_CHANNEL_4, // MIC 1
    ADC_CHANNEL_5, // MIC 2
    ADC_CHANNEL_9, // MIC 3
    ADC_CHANNEL_6, // MIC 4
    ADC_CHANNEL_8, // LINE
    ADC_CHANNEL_3, // BASS
    ADC_CHANNEL_7, // TREBLE
    ADC_CHANNEL_2, // MASTER
};

atomic_bool value_update[8] = {false};
atomic_uchar brightness[8] = {0};

uint8_t convertAdcToLight(int adc, uint8_t previous) {
    uint8_t output = ((adc - ADC_MIN) * (LIGHT_MAX - LIGHT_MIN)) / (ADC_MAX - ADC_MIN) + LIGHT_MIN;

    // Make sure off and max value can be reached
    if (output < LIGHT_MIN + MIN_MAX_SNAPPING) {
        output = LIGHT_MIN;
    } else if (output > LIGHT_MAX - MIN_MAX_SNAPPING) {
        output = LIGHT_MAX;
    }

    // hysteresis
    if (output == LIGHT_MIN || output == LIGHT_MIN) {
        return output;
    }

    uint8_t difference = output > previous ? output - previous : previous - output;
    if (difference < LIGHT_CHANGE) {
        return previous;
    }

    return output;
}

void adc_task(void * pvParameters) {
    adc_oneshot_unit_handle_t handle;
    adc_oneshot_unit_init_cfg_t init_cfg = {
        .unit_id = ADC_UNIT_1
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_cfg, &handle));

    adc_oneshot_chan_cfg_t cfg = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_11,
    };
    for (int i = 0; i < POTENTIOMETERS; i++) {
        adc_oneshot_config_channel(handle, channels[i], &cfg);
    }


    // init done, now continuously measure handles
    while (true) {
        for (int i = 0; i < POTENTIOMETERS; i++) {
            int adc_raw = 0;
            esp_err_t result = adc_oneshot_read(handle, channels[i], &adc_raw);
            if (result != ESP_OK) {
                ESP_LOGE(TAG, "Error getting ADC reading");
            }
            uint8_t current_brightness = convertAdcToLight(adc_raw, brightness[i]);
            if (brightness[i] != current_brightness) {
                ESP_LOGI(TAG, "channel %d changed from %d to %d", i, brightness[i], current_brightness);
                brightness[i] = current_brightness;
                value_update[i] = true;
            }   
        }
        vTaskDelay(pdMS_TO_TICKS(WAIT_MS));
    }   
}