#ifndef TYPES_H
#define TYPES_H

// Don't change the includes or be doomed
#include <stdint.h>
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

// Controllers
#include "esp_adc/adc_oneshot.h"

// Lights
#include "coap3/coap.h"

#define TAG                         "Lights"    // Tag used in log messages
#define AMOUNT_OF_LIGHT_CONTROLLERS 2       // Amount of LightController made in main.c
#define WAIT_MS                     100     // Amount of miliseconds to wait in main.c loop
#define SEND_ALIVE_S                30      // Send a keep alive message every x seconds if no other data was send
#define LIGHT_CHANGE                10      // Amount that the light value needs to change before sending it

#define WIFI_SSID                   "Zeus WPI"
#define WIFI_PASSWORD               "zeusisdemax"

#define ADC_MAX                     8191    // Maximum ADC value
#define ADC_MIN                     0       // Minimum ADC value
#define LIGHT_MAX                   255     // Maximum light value
#define LIGHT_MIN                   0       // Minimum light value

extern bool wifiConnected;

// Possible function result
typedef enum Result {
    OK,
    MEMORY_ISSUE,
    COAP_ERROR,
    BRIGHTNESS_ERROR,
    SEND_ERROR
} Result;

// Struct for the data regarding the rotating things
typedef struct Controller {
    adc_oneshot_unit_handle_t handle;
    adc_channel_t channel;
    int adc_raw;
} Controller;

// Struct for the lights
typedef struct Light {
    bool isLight;               // Light or group
    coap_context_t* ctx;
    coap_session_t* session;
    coap_optlist_t* optlist;
    uint8_t id;
} Light;

#endif
