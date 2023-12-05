#include <stdint.h>
#include <stdbool.h>

#include "esp_log.h"
#include "esp_http_client.h"

#include "globals.h"
#include "light.h"

void initLight(
    Light* light, 
    bool isLight, 
    uint32_t id, 
    esp_http_client_handle_t httpClient
    ) {
    light->isLight = isLight;
    light->id = id;
    light->httpClient = httpClient;
}

Result updateBrightness(Light* light, uint8_t brightness) {
    char data[50];

    int snprintf_result;
    ESP_LOGI(TAG, "Is light %hd", light->isLight);
    if (light->isLight) {
        snprintf_result = snprintf(data, sizeof(data), "{\"light\": %ld, \"brightness\": %hd}", light->id, brightness);
    } else {
        snprintf_result = snprintf(data, sizeof(data), "{\"group\": %ld, \"brightness\": %hd}", light->id, brightness);
    }

    if (snprintf_result == -1) {
        ESP_LOGE(TAG, "Error constructing http message");
        return BRIGHTNESS_FAIL;
    }

    esp_err_t err = esp_http_client_set_post_field(light->httpClient, data, sizeof(data));

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error adding post field");
        return HTTP_FAIL;
    }

    err = esp_http_client_perform(light->httpClient);

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error sending http request");
        return HTTP_FAIL;
    }

    return OK;
}

Result makeHttpClient(esp_http_client_handle_t* client, char* url) {
    esp_http_client_config_t config = {
        .url = url,
    };
    *client = esp_http_client_init(&config);

    if (*client == NULL) {
        ESP_LOGE(TAG, "Error creating http client light");
        return HTTP_FAIL;
    }

    esp_err_t errUrl = esp_http_client_set_url(*client, url);
    esp_err_t errMethod = esp_http_client_set_method(*client, HTTP_METHOD_POST);
    esp_err_t errHeader = esp_http_client_set_header(*client, "Content-Type", "application/json");

    if (errUrl != ESP_OK || errMethod != ESP_OK || errHeader != ESP_OK) {
        ESP_LOGE(TAG, "Error setting fields for http client light");
        return HTTP_FAIL;
    }

    return OK;
}

void freeLight(Light* light) {
    free(light);
}
