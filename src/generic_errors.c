#include "generic_errors.h"

// Helper macro
#define return_error(id, code, message) \
    json_t *error = json_object(); \
    json_object_set_new(error, "code", json_integer((code))); \
    json_object_set_new(error, "message", json_string((message))); \
    return _error(id, error); \

// Helper function
static json_t *_error(json_t *id, json_t *error) {
    json_t *response = json_object();
    json_object_set_new(response, "jsonrpc", json_string("2.0"));
    json_object_set_new(response, "error", error);

    if(id != NULL) {
        json_object_set(response, "id", id);
    } else {
        json_object_set_new(response, "id", json_null());
    }

    return response;
}

json_t *generate_invalid_json() {
    return_error(NULL, -32700, "Parse error");
}

json_t *generate_invalid_request(json_t *id) {
    return_error(id, -32600, "Invalid Request");
}

json_t *generate_method_not_found(json_t *id) {
    return_error(id, -32601, "Method not found");
}

json_t *generate_invalid_params(json_t *id) {
    return_error(id, -32602, "Invalid params");
}

json_t *generate_internal_error(json_t *id) {
    return_error(id, -32603, "Internal error");
}

json_t *generate_server_error(int code, json_t *id) {
    return_error(id, -32603, "Server error");
}
