#ifndef LIGHT_H
#define LIGHT_H

#include <stdint.h>
#include <stdbool.h>

#include "esp_http_client.h"

#include "globals.h"

typedef struct Light {
    char* payload;
    uint8_t payloadOffset;
    esp_http_client_handle_t* httpClient;
} Light;

Result initLight(
    Light* light,
    uint32_t* ids,
    esp_http_client_handle_t* httpClient
);

Result updateBrightness(Light* light, uint8_t brightness);

Result makeHttpClient(esp_http_client_handle_t* client, char* url);

void freeLight(Light* light);

#endif
