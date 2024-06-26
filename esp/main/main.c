#include <stdbool.h>
#include <unistd.h>

#include <string.h>
#include <sys/param.h>

#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"

#include "nvs_flash.h"

#include "globals.h"

const static char* TAG = "main";

void wifiEventHandler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
void wifiConnect();

bool wifiConnected = false;

void adc_task(void * pvParameters);
void http_task(void * pvParameters);

void app_main(void) {
    ESP_LOGI(TAG, "Starting up...");

    // Required
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Connect to wifi
    wifiConnect();

    while (! wifiConnected) {
        vTaskDelay(10);
    }
    ESP_LOGI(TAG, "connected to wifi");

    xTaskCreate(
        adc_task,        // Function that should be called
        "adc",           // Name of the task (for debugging)
        4096,            // Stack size (bytes)
        NULL,            // Parameter to pass
        1,               // Task priority
        NULL             // Task handle
    );
    xTaskCreate(
        http_task,       // Function that should be called
        "http",          // Name of the task (for debugging)
        4096,            // Stack size (bytes)
        NULL,            // Parameter to pass
        1,               // Task priority
        NULL             // Task handle
    );
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