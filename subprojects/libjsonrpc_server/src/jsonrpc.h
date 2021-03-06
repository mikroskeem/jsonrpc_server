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

#pragma once

#include <jansson.h>

typedef struct jsonrpc_ctx_s jsonrpc_ctx;

#define ERR_NONE      (0)
#define ERR_NOMETHOD  (1)
#define ERR_PARSE     (2)
#define ERR_INVALID   (3)
#define ERR_NOTIF     (4) // Not actually an error, but says that it's notification and no response is generated.

#define FLAG_ARRAY_PARAMS (1)
#define FLAG_KV_PARAMS    (1 << 2)
#define FLAG_IS_NOTIF     (1 << 3)  // In other words, "do not bother generating response"

/**
 * Convenience macros to create RPC method handlers
 */
#define RPC_HANDLER_SIGNATURE jsonrpc_ctx *ctx, int flags, json_t *parameters, json_t **response // object, flags array/object (see jsonrpc 2.0 spec), response ptr
#define RPC_HANDLER(name) int name(RPC_HANDLER_SIGNATURE)
#define IF_RPC_FLAG(flag) if((flags & (flag)) != 0)

/**
 * JSON-RPC method handler info structure
 */
struct jsonrpc_handler {
    char *name;
    int (*handler)(RPC_HANDLER_SIGNATURE);
};

/**
 * Convenience macro to add method handler to RPC method handlers list
 */
#define RPC_ADD_HANDLER(name) { #name, name }

/**
 * Convenience macro to end RPC method handlers list
 */
#define RPC_HANDLERS_END { NULL, NULL }

typedef struct jsonrpc_ctx_s {
    // JSON-RPC methods
    const struct jsonrpc_handler *handlers;

    // Response transformer
    json_t *(*response_transformer)(jsonrpc_ctx *ctx, const char *method, json_t *original);

    // Context data
    void *data;
} jsonrpc_ctx;

int jsonrpc_ctx_init(jsonrpc_ctx *ctx);
int jsonrpc_ctx_destroy(jsonrpc_ctx *ctx);

// Handles request
int jsonrpc_handle_request(jsonrpc_ctx *ctx, json_t *json, json_t **response);

// Simple request handler, reading request from string and writing string
int jsonrpc_handle_request_simple(jsonrpc_ctx *ctx, const char *json_body, size_t body_len, char **response, size_t response_len, json_error_t *err);
