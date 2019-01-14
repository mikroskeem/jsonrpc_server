#pragma once

#include "jsonrpc.h"
#include "generic_errors.h"
#include <string.h>

typedef struct jsonrpc_req_ctx_s {
    json_t *request;
    json_t *id;
    json_t *response;
} jsonrpc_req_ctx;

int handle_single_request(jsonrpc_ctx *ctx, jsonrpc_req_ctx *req_ctx);
