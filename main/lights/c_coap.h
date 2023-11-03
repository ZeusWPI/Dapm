#ifndef C_COAP_H
#define C_COAP_H

#include "coap3/coap.h"

#include "types.h"

Result initCoap(coap_context_t** ctx, coap_session_t** session, coap_optlist_t** optlist, char* server_uri);

Result sendPutRequest(char data[], coap_context_t** ctx, coap_session_t** session, coap_optlist_t** optlist);

Result sendGetRequest(coap_context_t** ctx, coap_session_t** session, coap_optlist_t** optlist);

#endif
