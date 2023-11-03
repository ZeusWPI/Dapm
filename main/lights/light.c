// Leave includes as they are >:(
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

#include "protocol_examples_common.h"

#include "coap3/coap.h"

#include "types.h"
#include "types_coap.h"
#include "light.h"
#include "c_coap.h"


Result initLight(Light* light, const bool isLight, char* server_uri) {
    coap_context_t* ctx;
    coap_session_t* session;
    coap_optlist_t* optlist = NULL;

    ESP_LOGI(TAG, "Server URI: %s", server_uri);

    // Initialise fields
    Result result = initCoap(
        &ctx,
        &session,
        &optlist,
        server_uri
    );

    if (result != OK) {
        return result;
    }

    light->isLight    = isLight;
    light->ctx        = ctx;
    light->session    = session;
    light->optlist    = optlist;

    return OK;
}

void coapProcesses(Light* light) {
    ESP_LOGI(TAG, "Checking processes....");
    coap_io_process(light->ctx, COAP_IO_NO_WAIT);
}

Result updateBrightness(Light* light, uint8_t brightness) {
    ESP_LOGI(TAG, "Sending.....");
    // Possible if light reconnecting didn't work
    if (light == NULL) {
        return SEND_ERROR;
    }

    char data[50];

    int snprintf_result;
    if (light->isLight) {
        snprintf_result = snprintf(data, sizeof(data), "{ \"%d\": [ { \"%d\": %hd } ] }", CONFIG, BRIGHTNESS, brightness);
    } else {
        snprintf_result = snprintf(data, sizeof(data), "{\"%d\":%hd}", BRIGHTNESS, brightness);
    }

    if (snprintf_result == -1) {
        ESP_LOGE(TAG, "Error constructing brightness message");
        return BRIGHTNESS_ERROR;
    }

    return sendPutRequest(
        data,
        &light->ctx,
        &light->session,
        &light->optlist
    );
}

Result sendAlive(Light* light) {
    // Possible if light reconnecting didn't work
    if (light == NULL) {
        return SEND_ERROR;
    }

    // The keep alive message is just a GET request
    return sendGetRequest(
        &light->ctx,
        &light->session,
        &light->optlist
    );
}

void freeLight(Light* light) {
    if (light == NULL) {
        return;
    }

    coap_delete_optlist(light->optlist);
    coap_session_release(light->session);
    coap_free_context(light->ctx);
}
