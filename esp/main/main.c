#include <stdbool.h>

#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/param.h>

#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_http_client.h"

#include "nvs_flash.h"

#include "globals.h"
#include "lightController.h"


void wifiEventHandler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
void wifiConnect();

bool wifiConnected;

void app_main(void) {
    // Required
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Connect to wifi
    wifiConnected = false;
    wifiConnect();

    while (! wifiConnected) {
        sleep(1);
    }

    // Light Controller
    LightController* lightControllers = malloc(sizeof(LightController) * AMOUNT_OF_LIGHT_CONTROLLERS);

    if (lightControllers == NULL) {
        ESP_LOGE(TAG, "Error allocating memory for lightControllers");
        abort();
    }

    // Controller
    adc_oneshot_unit_handle_t   handle;
    adc_oneshot_unit_init_cfg_t init_cfg = {
        .unit_id = ADC_UNIT_1
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_cfg, &handle));

    // Light
    esp_http_client_handle_t httpClientLight;
    Result result = makeHttpClient(&httpClientLight, LIGHT_URL);

    if (result != OK) {
        abort();
    }

    esp_http_client_handle_t httpClientGroup;
    result = makeHttpClient(&httpClientGroup, GROUP_URL);

    if (result != OK) {
        abort();
    }

    uint32_t lightOne[] = {65546};
    uint32_t lightTwo[] = {65547};
    uint32_t lightThree[] = {65556};
    uint32_t lightFour[] = {131077};

    // 1
    initLightController(
        &lightControllers[0],
        ADC_CHANNEL_0,
        &handle,
        1,
        lightOne,
        true,
        httpClientLight
    );

    // 2
    initLightController(
        &lightControllers[1],
        ADC_CHANNEL_1,
        &handle,
        1,
        lightTwo,
        true,
        httpClientLight
    );

    // 3
    initLightController(
        &lightControllers[2],
        ADC_CHANNEL_2,
        &handle,
        1,
        lightThree,
        true,
        httpClientLight
    );

    // 4
    initLightController(
        &lightControllers[3],
        ADC_CHANNEL_3,
        &handle,
        1,
        lightFour,
        false,
        httpClientGroup
    );

    while (1) {
        for (uint8_t i = 0; i < AMOUNT_OF_LIGHT_CONTROLLERS; i++) {
            updateLightController(&(lightControllers[i]));
        }

        vTaskDelay(pdMS_TO_TICKS(WAIT_MS));
    }

    // Clean everything up
    for (uint8_t i = 0; i < AMOUNT_OF_LIGHT_CONTROLLERS; i++) {
        freeLightController(&(lightControllers[i]));
    }

    esp_http_client_cleanup(httpClientLight);
    esp_http_client_cleanup(httpClientGroup);
}

void wifiEventHandler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id,void *event_data) {
    switch (event_id) {
        case WIFI_EVENT_STA_START:
            ESP_LOGI(TAG, "Connecting to wifi...");
            break;
        case WIFI_EVENT_STA_CONNECTED:
            ESP_LOGI(TAG, "Connected to wifi");
            wifiConnected = true;
            break;
        case WIFI_EVENT_STA_DISCONNECTED:
            ESP_LOGE(TAG, "Lost wifi connection");
            wifiConnected = false;
            wifiConnect();
            break;
        default:
            break;
    }
}

void wifiConnect() {
    // Configuration
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t wifi_initiation = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&wifi_initiation); //     
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifiEventHandler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifiEventHandler, NULL);
    wifi_config_t wifi_configuration = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASSWORD,
            
        }
    };
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_configuration);
    // Start
    esp_wifi_start();
    esp_wifi_set_mode(WIFI_MODE_STA);
    // Connect
    esp_wifi_connect();    
}