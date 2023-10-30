#ifndef TYPES_COAP_H
#define TYPES_COAP_H

#define COAP_URI            "coaps://10.0.0.9:5684/"              // Base uri for the lights
#define COAP_PSK_KEY        "RkXN9tg54IbYpwdb"                    // Password
#define COAP_PSK_IDENTITY   "da02bc820efd415697f44330fbba17e1"    // Identity
#define COAP_TIME_SEC       5                                     // Amount of time to wait for an answer
#define COAP_LOG_INFO       2


typedef enum Url {
    DEVICES                         = 15001,    // Lights are located in here
    GROUPS                          = 15004,
    SCENES                          = 15005,
    NOTIFICATIONS                   = 15006,
    SMART_TASKS                     = 15010,
    ACTIONS                         = 15011
} Url;

typedef enum Groups {
    SUPERGROUP                      = 131073,
    KELDER                          = 131077,
    LED                             = 131080
} Groups;

// Official names for the lights
typedef enum _Lights {
    _GROTE_TAFEL_RAAM               = 65546,
    _GROTE_TAFEL_MIDDEN             = 65547,
    _TUSSEN_KLEINE_TAFEL_EN_ZETEL   = 65551,
    _KLEINE_TAFEL                   = 65552,
    _GROTE_TAFEL_INGANG_KANT        = 65556,
    _OPEN_KAST_EN_BESTUURSKAST      = 65560,
    _DEUR                           = 65561,
    _SERVER                         = 65567,
    _POSTERHOEK                     = 65575
} _Lights;

// Current position of each light
typedef enum Lights {
    ZETEL_RAAM                      = _GROTE_TAFEL_RAAM,
    ZETEL_DEUR                      = _GROTE_TAFEL_MIDDEN,
    KLEINE_TAFEL                    = _TUSSEN_KLEINE_TAFEL_EN_ZETEL,
    GROTE_TAFEL_SERVER              = _KLEINE_TAFEL,
    MINI_TAFEL                      = _GROTE_TAFEL_INGANG_KANT,
    GROTE_TAFEL                     = _OPEN_KAST_EN_BESTUURSKAST,
    DEUR                            = _DEUR,
    VOORRAAD                        = _SERVER,
    KASTENHOEK                      = _POSTERHOEK
} Lights;

typedef enum Actions {
    REBOOT                          = 9030,
    RESET                           = 9031,
    UPDATE                          = 9034,
    DETAILS                         = 15012
} Actions;

// Actions that can be used for groups and lights
typedef enum ActionsGeneral {
    ON_OFF                          = 5850,
    BRIGHTNESS                      = 5851,
} ActionsGeneral;

// Actions that can be used for groups
typedef enum ActionsGroup {
    NAME                            = 9001,
    CREATION                        = 9002,
    INSTANCE                        = 9003,
    LIST_ITEMS                      = 9018
} ActionsGroup;

// Actions that can be used for lights
typedef enum ActionsLight {
    CONFIG                          = 3311,
    COLOR_HEX                       = 5706,
    HUE                             = 5707,
    SATURATION                      = 5708,
    COLOR_X                         = 5709,
    COLOR_Y                         = 5710,
    COLOR_TEMPERATURE               = 5711,
    TRANSITION_TIME                 = 5712,
} ActionsLight;

typedef enum ColorTemperatures {
    WARM                            = 447,
    MEDIUM                          = 367,
    COLD                            = 251,
} ColorTemperatures;

#endif
