#ifndef LIGHT_H
#define LIGHT_H

#include "coap3/coap.h"

#include "types.h"


Result initLight(Light* light, const bool isLight, char* server_uri);

void coapProcesses(Light* light);

// Update the light value
Result updateBrightness(Light* light, uint8_t brightness);

// Send keep alive message
Result sendAlive(Light* light);

void freeLight(Light* light);

#endif
