#include "esp_log.h"
#include "esp_event.h"
#include "esp_http_client.h"
#include "esp_timer.h"
#include "cJSON.h"

#include "types_coap.h"
#include "globals.h"
#include <stdatomic.h>

const static char* TAG = "http_task";

extern atomic_bool value_update[POTENTIOMETERS];
extern atomic_uchar brightness[POTENTIOMETERS];

uint32_t lights[POTENTIOMETERS][AMOUNT_OF_LIGHTS] = {
        {ZETEL_RAAM, 0}, // MIC 1
        {1, 0}, // MIC 2
        {1, 0}, // MIC 3
        {1, 0}, // MIC 4
        {1, 0}, // LINE
        {1, 0}, // BASS
        {1, 0}, // TREBLE
        {KELDER, 0}, // MASTER
    };

void send_http_post(char* post_data, char* url) {
    esp_http_client_config_t config = {
        .url = url,
        .transport_type = HTTP_TRANSPORT_OVER_TCP,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);

    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_post_field(client, post_data, strlen(post_data));
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK) {
        ESP_LOGI(TAG, "HTTP POST Status = %d", esp_http_client_get_status_code(client));
    } else {
        ESP_LOGE(TAG, "HTTP POST request failed: %s", esp_err_to_name(err));
    }
    esp_http_client_cleanup(client);
}

bool updateBrightness(int idx, uint8_t brightness) {
    cJSON* root = cJSON_CreateObject();
    uint32_t* ids = lights[idx];
    if (ids[0] == 0) return false;
    
    // build payload
    char* url = "";
    if (ids[0] < 100000) {
        // consists of lights
        cJSON* array = cJSON_AddArrayToObject(root, "lights");
        for (int i = 0; i < 5; i++) {
            if (ids[i] == 0) break;
            cJSON* element = cJSON_CreateNumber(ids[i]);
	        cJSON_AddItemToArray(array, element);
        }
        url = LIGHT_URL;
    } else {
        // consists of a group
        cJSON_AddNumberToObject(root, "group", ids[0]);
        url = GROUP_URL;
    }
    cJSON_AddNumberToObject(root, "brightness", brightness);

    char* data = cJSON_Print(root);
    cJSON_free(root);

    send_http_post(data, url);

    cJSON_free(data);

    return true;
}

void http_task(void * pvParameters) {
    while (true) {
        for (int i = 0; i < POTENTIOMETERS; i++) {
            if (value_update[i]) {
                ESP_LOGI(TAG, "http request: channel %d to %d", i, brightness[i]);
                value_update[i] = false;
                int64_t begin = esp_timer_get_time();
                updateBrightness(i, brightness[i]);
                int64_t end = esp_timer_get_time();
                int difference = end - begin;
                ESP_LOGW(TAG, "took %d us", difference);
                int diff_ms = difference / 1000;
                if (diff_ms > 50) {
                    ESP_LOGE(TAG, "took too damn long");
                }
            }
        }
       vTaskDelay(pdMS_TO_TICKS(10)); // ms between http requests
    }
}