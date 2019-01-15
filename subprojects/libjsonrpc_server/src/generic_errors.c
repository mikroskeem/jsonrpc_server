/*
 * This file is part of project jsonrpc_server, licensed under the MIT License (MIT).
 *
 * Copyright (c) 2019 Mark Vainomaa <mikroskeem@mikroskeem.eu>
 * Copyright (c) Contributors
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

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
