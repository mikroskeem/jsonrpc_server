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

#include "jsonrpc_internal.h"

JSONRPC_EXPORT
int jsonrpc_ctx_init(jsonrpc_ctx *ctx) {
    return 0;
}

JSONRPC_EXPORT
int jsonrpc_ctx_destroy(jsonrpc_ctx *ctx) {
    return 0;
}

int handle_request_single(jsonrpc_ctx *ctx, json_t *request, json_t **response) {
    int flags = 0;

    // Verify JSON-RPC version
    json_t *_version = json_object_get(request, "jsonrpc");
    if(json_is_string(_version)) {
        const char *ver = json_string_value(_version);
        if(strncmp("2.0", ver, 3) != 0) {
            *response = generate_invalid_request(NULL);
            return ERR_INVALID;
        }
    } else {
        *response = generate_invalid_request(NULL);
        return ERR_INVALID;
    }

    // If id is present...
    json_t *_id = json_object_get(request, "id");
    if(_id != NULL) {
        if(!(json_is_string(_id) || json_is_integer(_id) || json_is_real(_id) || json_is_null(_id))) {
            *response = generate_invalid_request(NULL);
            return ERR_PARSE;
        }
    } else {
        // *sigh*
        flags |= FLAG_IS_NOTIF;
    }

    // Check if given JSON-RPC method exists
    struct jsonrpc_handler *found_handler = NULL;
    json_t *_method = json_object_get(request, "method");
    const char *mname;
    if(json_is_string(_method)) {
        mname = json_string_value(_method);
        size_t len = strlen(mname);

        for(struct jsonrpc_handler *h = (struct jsonrpc_handler *) ctx->handlers; h->name != NULL; h++) {
            if(strlen(h->name) == len && strncmp(h->name, mname, len) == 0) {
                found_handler = h;
            }
        }

        if(found_handler == NULL) {
            *response = generate_method_not_found(_id);
            return ERR_NOMETHOD;
        }
    } else {
        *response = generate_invalid_request(_id);
        return ERR_INVALID;
    }

    // Check params
    json_t *_params = json_object_get(request, "params");
    if(_params != NULL) {
        if(json_is_array(_params)) {
            flags |= FLAG_ARRAY_PARAMS;
        } else if(json_is_object(_params)) {
            flags |= FLAG_KV_PARAMS;
        } else {
            *response = generate_invalid_request(_id);
            return ERR_INVALID;
        }
    }

    // Run handler
    json_t *_response = NULL;
    int r = found_handler->handler(ctx, flags, _params, &_response);

    switch(r) {
        case ERR_NONE:
            break;
        case ERR_NOMETHOD: {
            *response = generate_method_not_found(_id);
            return r;
        }
        case ERR_INVALID: {
            *response = generate_invalid_request(_id);
            return r;
        }
        case ERR_NOTIF:
            return r;
    }

    // Do nothing when it's a notification
    if((flags & FLAG_IS_NOTIF) != 0) {
        return ERR_NOTIF;
    }

    // Wrap response
    *response = json_object();
    json_object_set_new(*response, "result", _response);
    json_object_set(*response, "jsonrpc", _version);
    json_object_set(*response, "id", _id);

    // Transform response if transformer function is set
    if(ctx->response_transformer != NULL) {
        *response = ctx->response_transformer(ctx, mname, *response);
    }

    return ERR_NONE;
}

JSONRPC_EXPORT
int jsonrpc_handle_request(jsonrpc_ctx *ctx, json_t *request, json_t **response) {
    json_incref(request);

    if(json_is_array(request)) {
        if(json_array_size(request) < 1) {
            *response = generate_invalid_request(NULL);
            json_decref(request);
            return ERR_INVALID;
        }

        *response = json_array();

        // Iterate over requests
        size_t ind;
        json_t *child_request;
        json_array_foreach(request, ind, child_request) {
            json_t *child_resp = NULL;
            json_incref(child_request);
            int r = handle_request_single(ctx, child_request, &child_resp);

            if(r == ERR_NOTIF) {
                if(child_resp != NULL)
                    json_decref(child_resp);
            } else if(r > 0) {
                // TODO: what did I have to do here again?
            } else {
                json_array_append_new(*response, child_resp);
            }
            json_decref(child_request);
        }
        json_decref(request);
        return ERR_NONE;
    } else if(json_is_object(request)) {
        int r = handle_request_single(ctx, request, response);
        json_decref(request);
        return r;
    } else {
        *response = generate_invalid_request(NULL);
        return ERR_INVALID;
    }
}

JSONRPC_EXPORT
int jsonrpc_handle_request_simple(jsonrpc_ctx *ctx,
                                  const char *json_body, size_t body_len,
                                  char **response, size_t response_len,
                                  json_error_t *err) {
    int r = -1;

    // Make sure that response buffer pointer is set
    if(response == NULL) {
        return r;
    }

    json_t *resp = NULL;
    json_t *base = json_loads(json_body, 0, err);

    if(base != NULL) {
        r = jsonrpc_handle_request(ctx, base, &resp);
        json_decref(base);
    } else {
        // Parser error woo
        base = generate_invalid_json();
    }

    // Serialize
    char *serialized = json_dumps(resp, 0);
    json_decref(resp);

    // If buffer is null, allocate one
    if(*response == NULL) {
        *response = malloc(response_len);
    }

    // Copy
    strncpy(*response, serialized, response_len - 1);
    free(serialized);

    return r;
}
