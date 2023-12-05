#ifndef LIGHT_H
#define LIGHT_H

#include <stdint.h>
#include <stdbool.h>

#include "esp_http_client.h"

#include "globals.h"

typedef struct Light {
    bool isLight;
    uint32_t id;
    esp_http_client_handle_t httpClient;
} Light;

void initLight(
    Light* light,
    bool isLight,
    uint32_t id,
    esp_http_client_handle_t httpClient
);

Result updateBrightness(Light* light, uint8_t brightness);

void freeLight(Light* light);

#endif
