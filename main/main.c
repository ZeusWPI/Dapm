// No touching the includes :O !!!!!!!
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/param.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "protocol_examples_common.h"

#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"

#include "nvs_flash.h"

#include "coap3/coap.h"

#include "types.h"
#include "types_coap.h"
#include "light_controller.h"


void wifiEventHandler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
void wifiConnect();

void app_main(void) {
    // Required
    ESP_ERROR_CHECK(nvs_flash_init() );
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Connect to wifi
    // wifiConnected = false;
    // wifiConnect();

    // while (! wifiConnected) {
    //     sleep(1);
    // }

    ESP_ERROR_CHECK(example_connect());
    wifiConnected = true;

    // esp_wifi_init(WIFI_INIT_CONFIG_DEFAULT);

    // Construct all light controllers
    LightController* lightControllers = malloc(sizeof(LightController) * AMOUNT_OF_LIGHT_CONTROLLERS);

    if (lightControllers == NULL) {
        ESP_LOGE(TAG, "Error allocating memory for lightControllers");
        abort();
    }

    // 1
    char** uris = malloc(sizeof(char*) * 2);
    uris[0] = malloc(sizeof(char) * 40);
    snprintf(uris[0], 40, "%s%d/%d", COAP_URI, DEVICES, VOORRAAD);
    uris[1] = malloc(sizeof(char) * 40);
    snprintf(uris[1], 40, "%s%d/%d", COAP_URI, DEVICES, DEUR);
    initLightController(
        &lightControllers[0],
        ADC_CHANNEL_0,
        ADC_UNIT_1,
        2,
        true,
        uris
    );

    // 2
    // char** uris2 = malloc(sizeof(char*) * 1);
    // uris2[0] = malloc(sizeof(char) * 40);
    // snprintf(uris2[0], 40, "%s%d/%d", COAP_URI, DEVICES, GROTE_TAFEL);
    // initLightController(
    //     &lightControllers[1],
    //     ADC_CHANNEL_7,
    //     ADC_UNIT_1,
    //     1,
    //     true,
    //     uris2
    // );

    // 3

    // 4

    // 5

    // 6

    // 7

    // 8


    // Infinite loop to get rotating thing positions and adjust light bulbs
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

