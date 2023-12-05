#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "esp_log.h"
#include "esp_http_client.h"

#include "globals.h"
#include "light.h"

#define DEFAULT_SIZE 128

Result initLight(
    Light* light, 
    uint32_t* ids, 
    esp_http_client_handle_t* httpClient
    ) {
    char* data = malloc(sizeof(char) * DEFAULT_SIZE);

    if (data == NULL) {
        return MEMORY_ISSUE;
    }

    light->payloadOffset = 0;

    if (ids[0] < 100000) {
        light->payloadOffset += snprintf(data + light->payloadOffset, DEFAULT_SIZE - light->payloadOffset, "{\"lights\": [%lu", ids[0]);
        uint8_t i = 1;

        while (i < AMOUNT_OF_LIGHTS && ids[i] != 0) {
            light->payloadOffset += snprintf(data + light->payloadOffset, DEFAULT_SIZE - light->payloadOffset, ",%lu", ids[i]);
            i++;
        }
        light->payloadOffset += snprintf(data + light->payloadOffset, DEFAULT_SIZE - light->payloadOffset, "], \"brightness\": ");
    } else {
        light->payloadOffset += snprintf(data + light->payloadOffset, DEFAULT_SIZE - light->payloadOffset, "{\"group\": %ld, \"brightness\": ", ids[0]);
    }

    light->payload = data;
    light->httpClient = httpClient;

    return OK;
}

Result updateBrightness(Light* light, uint8_t brightness) {
    char data[DEFAULT_SIZE];
    strcpy(data, light->payload);

    int snprintf_result;

    snprintf_result = snprintf(data + light->payloadOffset, DEFAULT_SIZE - light->payloadOffset, "%hd}", brightness);

    if (snprintf_result == -1) {
        ESP_LOGE(TAG, "Error constructing http message");
        return BRIGHTNESS_FAIL;
    }

    esp_err_t err = esp_http_client_set_post_field(*light->httpClient, data, DEFAULT_SIZE);

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error adding post field");
        return HTTP_FAIL;
    }

    err = esp_http_client_perform(*light->httpClient);

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error sending http request");
        return HTTP_SEND_FAIL;
    }

    return OK;
}

Result makeHttpClient(esp_http_client_handle_t* client, char* url) {
    ESP_LOGI(TAG, "Making http connection...");

    esp_http_client_config_t config = {
        .url = url,
    };
    *client = esp_http_client_init(&config);

    if (client == NULL) {
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

    ESP_LOGI(TAG, "Successful http connection");

    return OK;
}

void freeLight(Light* light) {
    free(light);
}
