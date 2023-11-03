#include <stdio.h>
#include <ctype.h>
#include <sys/param.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "esp_log.h"

#include "coap3/coap.h"

#include "types.h"
#include "types_coap.h"
#include "c_coap.h"


// Log handler
void coap_log_handler (coap_log_t level, const char *message);
// Processed uri
int coap_get_address(coap_uri_t *uri, coap_address_t* dst_addr);
// Builds the opt list
void coap_build_optlist(coap_uri_t *uri, coap_optlist_t **optlist);
// Starts a new session with DTLS
coap_session_t* coap_start_psk_session(coap_context_t* ctx, coap_address_t* dest, coap_uri_t* uri);
// Handles server response
coap_response_t message_handler(coap_session_t* session, const coap_pdu_t* sent, const coap_pdu_t* received, const coap_mid_t mid);


Result initCoap(coap_context_t** ctx, coap_session_t** session, coap_optlist_t** optlist, char* server_uri) {
    coap_uri_t uri;             // Uri
    coap_address_t dst_addr;    // Destination address

    // Start everything
    coap_startup();

    // Logging level
    coap_set_log_handler(coap_log_handler);
    coap_set_log_level(COAP_LOG_INFO);

    // Create context
    if (!(*ctx = coap_new_context(NULL))) {
        ESP_LOGE(TAG, "Error creating new context");
        return COAP_ERROR;
    }

    // Support large responses
    coap_context_set_block_mode(*ctx, COAP_BLOCK_USE_LIBCOAP);

    // Set the response handler
    coap_register_response_handler(*ctx, message_handler);

    // Resolve destination address
    if (coap_split_uri((const uint8_t *)server_uri, strlen(server_uri), &uri) == -1) {
        ESP_LOGE(TAG, "Error spliting uri");
        coap_free_context(*ctx);
        return COAP_ERROR;
    }

    // Create options list
    coap_build_optlist(&uri, optlist);
    if (optlist == NULL) {
        ESP_LOGE(TAG, "Error building optlist");
        coap_free_context(*ctx);
        return COAP_ERROR;
    }

    // Get destination address
    if (!(coap_get_address(&uri, &dst_addr))) {
        ESP_LOGE(TAG, "Error getting address");
        coap_delete_optlist(*optlist);
        coap_free_context(*ctx);
        return COAP_ERROR;
    }

    // Function to create psk session
    if (!((*session) = coap_start_psk_session(*ctx, &dst_addr, &uri))) {
        ESP_LOGE(TAG, "Error starting session");
        coap_delete_optlist(*optlist);
        coap_session_release(*session);
        coap_free_context(*ctx);
        return COAP_ERROR;
    }

    return OK;
}

Result sendPutRequest(char data[], coap_context_t** ctx, coap_session_t** session,  coap_optlist_t** optlist) {
    coap_pdu_t*     pdu;        // PDU
    unsigned char token[8];     // Token
    size_t tokenLength;         // Token length

    // Construct coap message
    if (!(pdu = coap_new_pdu(COAP_MESSAGE_CON, COAP_REQUEST_CODE_PUT, *session))) {
        ESP_LOGE(TAG, "Error creating new pdu for PUT request");
        return SEND_ERROR;
    }

    // Add unique token
    coap_session_new_token(*session, &tokenLength, token);
    coap_add_token(pdu, tokenLength, token);

    unsigned char buf[4];

    coap_insert_optlist(optlist,
                        coap_new_optlist(COAP_OPTION_CONTENT_FORMAT,
                                         coap_encode_var_safe(buf, sizeof(buf), COAP_MEDIATYPE_APPLICATION_JSON), 
                                         buf));

    // Add opt list
    coap_add_optlist_pdu(pdu, optlist);

    ESP_LOGI(TAG, "Message: %s", data);

    if (!coap_add_data_large_request(*session, pdu, strlen(data), (unsigned char*) data, NULL, NULL)) {
        ESP_LOGE(TAG, "Error add large data request");
        coap_delete_pdu(pdu);
        return SEND_ERROR;
    }

    // Send message
    if (coap_send(*session, pdu) == COAP_INVALID_MID) {
        ESP_LOGE(TAG, "Error sending PUT message");
        coap_delete_pdu(pdu);
        return SEND_ERROR;
    }

    return OK;
}

Result sendGetRequest(coap_context_t** ctx, coap_session_t** session,  coap_optlist_t** optlist) {
    coap_pdu_t*     pdu;        // PDU
    unsigned char token[8];     // Token
    size_t tokenLength;         // Token length

    // Construct coap message
    if (!(pdu = coap_new_pdu(COAP_MESSAGE_CON, COAP_REQUEST_CODE_GET, *session))) {
        ESP_LOGE(TAG, "Error creating new pdu for GET request");
        return SEND_ERROR;
    }

    // Add unique token
    coap_session_new_token(*session, &tokenLength, token);
    coap_add_token(pdu, tokenLength, token);

    // Add opt list
    coap_add_optlist_pdu(pdu, optlist);

    // Send message
    if (coap_send(*session, pdu) == COAP_INVALID_MID) {
        ESP_LOGE(TAG, "Error sending GET request");
        coap_delete_pdu(pdu);
        return SEND_ERROR;
    }

    coap_io_process(*ctx, COAP_IO_NO_WAIT);

    return OK;
}

// Loggggggs
void coap_log_handler (coap_log_t level, const char *message) {
    printf(message);
    // Don't need logs if your code works :O
    return;
}

// Build opt list
void coap_build_optlist(coap_uri_t *uri, coap_optlist_t **optlist) {
    // Variables
    #define BUFSIZE 40
    unsigned char _buf[BUFSIZE];
    unsigned char *buf;
    size_t buflen;
    int res;

    // Reset optlist
    (*optlist) = NULL;

    // If uri has a path add all path stuff to optlist
    if (uri->path.length) {
        buflen = BUFSIZE;
        buf = _buf;
        res = coap_split_path(uri->path.s, uri->path.length, buf, &buflen);

        while (res--) {
            coap_insert_optlist(optlist,
                                coap_new_optlist(COAP_OPTION_URI_PATH,
                                                 coap_opt_length(buf),
                                                 coap_opt_value(buf)));

            buf += coap_opt_size(buf);
        }
    }

    // If uri has a query add all query stuff to optlist
    if (uri->query.length) {
        buflen = BUFSIZE;
        buf = _buf;
        res = coap_split_query(uri->query.s, uri->query.length, buf, &buflen);

        while (res--) {
            coap_insert_optlist(optlist,
                                coap_new_optlist(COAP_OPTION_URI_QUERY,
                                                 coap_opt_length(buf),
                                                 coap_opt_value(buf)));

            buf += coap_opt_size(buf);
        }
    }
}

// Make fancy session with dtls
coap_session_t* coap_start_psk_session(coap_context_t* ctx, coap_address_t* dest, coap_uri_t* uri) {
    // Variables
    coap_dtls_cpsk_t dtls_psk;  // DTLS

    // Set a bunch of dtls_psk variables
    dtls_psk.version = COAP_DTLS_CPSK_SETUP_VERSION;
    dtls_psk.validate_ih_call_back = NULL;
    dtls_psk.ih_call_back_arg = NULL;

    // Address
    char client_sni[256];
    memcpy(client_sni, uri->host.s, MIN(uri->host.length, sizeof(client_sni) - 1));
    dtls_psk.client_sni = client_sni;

    // More variables yay
    dtls_psk.psk_info.identity.s = (const uint8_t *)COAP_PSK_IDENTITY;
    dtls_psk.psk_info.identity.length = strlen(COAP_PSK_IDENTITY);
    dtls_psk.psk_info.key.s = (const uint8_t *)COAP_PSK_KEY;
    dtls_psk.psk_info.key.length = strlen(COAP_PSK_KEY);

    return coap_new_client_session_psk2(ctx, NULL, dest, COAP_PROTO_DTLS, &dtls_psk);
}

// Debug function
const char* get_coap_pdu_code_t_string(coap_pdu_code_t type) {
    switch (type) {
        case COAP_EMPTY_CODE: return "No code";

        case COAP_REQUEST_CODE_GET: return "get";
        case COAP_REQUEST_CODE_POST: return "post";
        case COAP_REQUEST_CODE_PUT: return "put";
        case COAP_REQUEST_CODE_DELETE : return "delete";
        case COAP_REQUEST_CODE_FETCH: return "fetch";
        case COAP_REQUEST_CODE_PATCH: return "pathc";
        case COAP_REQUEST_CODE_IPATCH: return "ipathc";

        case COAP_RESPONSE_CODE_CREATED: return "created";
        case COAP_RESPONSE_CODE_DELETED: return "deleted";
        case COAP_RESPONSE_CODE_VALID: return "valid";
        case COAP_RESPONSE_CODE_CHANGED: return "changed";
        case COAP_RESPONSE_CODE_CONTENT: return "content";
        case COAP_RESPONSE_CODE_CONTINUE: return "continue";
        case COAP_RESPONSE_CODE_BAD_REQUEST: return "bad_request";
        case COAP_RESPONSE_CODE_UNAUTHORIZED: return "unautherized";
        case COAP_RESPONSE_CODE_BAD_OPTION: return "bad_otption";
        case COAP_RESPONSE_CODE_FORBIDDEN: return "forbidden";
        case COAP_RESPONSE_CODE_NOT_FOUND: return "not_found";
        case COAP_RESPONSE_CODE_NOT_ALLOWED: return "not_allowed";
        case COAP_RESPONSE_CODE_NOT_ACCEPTABLE: return "not_acceptable";
        case COAP_RESPONSE_CODE_INCOMPLETE: return "incomplete";
        case COAP_RESPONSE_CODE_CONFLICT: return "conflict";
        case COAP_RESPONSE_CODE_PRECONDITION_FAILED: return "precondition_failed";
        case COAP_RESPONSE_CODE_REQUEST_TOO_LARGE: return "too_large";
        case COAP_RESPONSE_CODE_UNSUPPORTED_CONTENT_FORMAT: return "unsupported_content_format";
        case COAP_RESPONSE_CODE_UNPROCESSABLE: return "unprocessable";
        case COAP_RESPONSE_CODE_TOO_MANY_REQUESTS: return "too_many_requests";
        case COAP_RESPONSE_CODE_INTERNAL_ERROR: return "error";
        case COAP_RESPONSE_CODE_NOT_IMPLEMENTED: return "not_implemented";
        case COAP_RESPONSE_CODE_BAD_GATEWAY: return "bad-gateway";
        case COAP_RESPONSE_CODE_SERVICE_UNAVAILABLE: return "service_unavailable";
        case COAP_RESPONSE_CODE_GATEWAY_TIMEOUT: return "gateway_timeout";
        case COAP_RESPONSE_CODE_PROXYING_NOT_SUPPORTED: return "proxying_not_supported";
        case COAP_RESPONSE_CODE_HOP_LIMIT_REACHED: return "hop_limit_reached";

        case COAP_SIGNALING_CODE_CSM: return "csm";
        case COAP_SIGNALING_CODE_PING: return "ping";
        case COAP_SIGNALING_CODE_PONG: return "pong";
        case COAP_SIGNALING_CODE_RELEASE: return "release";
        case COAP_SIGNALING_CODE_ABORT: return "abort";
    }

    return "";
}

// Handle messages returned from server
coap_response_t message_handler(coap_session_t* session, const coap_pdu_t* sent, const coap_pdu_t* received, const coap_mid_t mid) {
    ESP_LOGI(TAG, "Response");

    const unsigned char* data = NULL;
    size_t data_len;
    size_t offset;
    size_t total;
    coap_pdu_code_t rcvd_code = coap_pdu_get_code(received);
    ESP_LOGI(TAG, "Code: %s", get_coap_pdu_code_t_string(rcvd_code));

    if (COAP_RESPONSE_CLASS(rcvd_code) == 2) {
        if (coap_get_data_large(received, &data_len, &data, &offset, &total)) {
            if (data_len != total) {
                ESP_LOGI(TAG, "Unexpected partial data received offset %zu, length %zu", offset, data_len);
            }
            ESP_LOGI(TAG, "Received:\n%.*s", (int)data_len, data);
        }
        return COAP_RESPONSE_OK;
    }
    ESP_LOGI(TAG, "%d.%02d", (rcvd_code >> 5), rcvd_code & 0x1F);
    if (coap_get_data_large(received, &data_len, &data, &offset, &total)) {
        ESP_LOGI(TAG, ": ");
        while(data_len--) {
            ESP_LOGI(TAG, "%c", isprint(*data) ? *data : '.');
            data++;
        }
    }
    return COAP_RESPONSE_OK;
}

// Get address, voodoo stuff
int coap_get_address(coap_uri_t *uri, coap_address_t* dst_addr) {
    char *phostname = NULL;
    struct addrinfo hints;
    struct addrinfo *addrres;
    int error;
    char tmpbuf[INET6_ADDRSTRLEN];
    phostname = (char *)calloc(1, uri->host.length + 1);
    if (phostname == NULL) {
        ESP_LOGE(TAG, "Calloc failed");
        return 0;
    }
    memcpy(phostname, uri->host.s, uri->host.length);

    memset ((char *)&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_family = AF_UNSPEC;

    error = getaddrinfo(phostname, NULL, &hints, &addrres);
    if (error != 0) {
        ESP_LOGE(TAG, "DNS lookup failed for destination address %s. error: %d", phostname, error);
        free(phostname);
        return 0;
    }
    if (addrres == NULL) {
        ESP_LOGE(TAG, "DNS lookup %s did not return any addresses", phostname);
        free(phostname);
        return 0;
    }
    free(phostname);
    coap_address_init(dst_addr);
    switch (addrres->ai_family) {
    case AF_INET:
        memcpy(&dst_addr->addr.sin, addrres->ai_addr, sizeof((*dst_addr).addr.sin));
        (*dst_addr).addr.sin.sin_port        = htons(uri->port);
        inet_ntop(AF_INET, &dst_addr->addr.sin.sin_addr, tmpbuf, sizeof(tmpbuf));
        break;
    case AF_INET6:
        memcpy(&dst_addr->addr.sin6, addrres->ai_addr, sizeof((*dst_addr).addr.sin6));
        (*dst_addr).addr.sin6.sin6_port        = htons(uri->port);
        inet_ntop(AF_INET6, &dst_addr->addr.sin6.sin6_addr, tmpbuf, sizeof(tmpbuf));
        break;
    default:
        ESP_LOGE(TAG, "DNS lookup response failed");
        return 0;
    }
    freeaddrinfo(addrres);

    return 1;
}
