#ifndef GLOBALS_H
#define GLOBALS_H

#define WAIT_MS                     100
#define POTENTIOMETERS              8
#define AMOUNT_OF_LIGHTS            9

#define WIFI_SSID                   "Zeus WPI"
#define WIFI_PASSWORD               "zeusisdemax"
#define LIGHT_URL                   "http://10.0.0.153:8080/led"
#define GROUP_URL                   "http://10.0.0.153:8080/group"

#define ADC_MAX                     8191    // Maximum ADC value
#define ADC_MIN                     0       // Minimum ADC value
#define LIGHT_MAX                   254     // Maximum light value
#define LIGHT_MIN                   0       // Minimum light value
#define LIGHT_CHANGE                10      // Amount that the light value needs to change before sending it

extern bool wifiConnected;

typedef enum Result {
    OK,
    MEMORY_ISSUE,
    HTTP_FAIL,
    HTTP_SEND_FAIL,
    BRIGHTNESS_FAIL,
    NO_WIFI
} Result;

#endif
