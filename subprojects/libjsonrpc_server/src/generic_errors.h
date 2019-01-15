#pragma once

#include <jansson.h>

// See: https://www.jsonrpc.org/specification

json_t *generate_invalid_json();                       // -32700
json_t *generate_invalid_request(json_t *id);          // -32600
json_t *generate_method_not_found(json_t *id);         // -32601
json_t *generate_invalid_params(json_t *id);           // -32602
json_t *generate_internal_error(json_t *id);           // -32603
json_t *generate_server_error(int code, json_t *id);   // [-32000, -32099]
