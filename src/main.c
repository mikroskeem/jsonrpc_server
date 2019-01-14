#include "jsonrpc.h"
#include <string.h>
#include <zlib.h>

static RPC_HANDLER(version);
static RPC_HANDLER(hello);
static RPC_HANDLER(do_crc32);

const struct jsonrpc_handler handlers[] = {
    RPC_ADD_HANDLER(version),
    RPC_ADD_HANDLER(hello),
    RPC_ADD_HANDLER(do_crc32),
    RPC_HANDLERS_END
};

int main(void) {
    jsonrpc_ctx ctx = {0};
    ctx.handlers = handlers;
    jsonrpc_ctx_init(&ctx);

    const char *req0 = "{\"jsonrpc\":\"2.0\",\"id\":3.0,\"method\":\"version\",\"params\":[]}";
    const char *req1 = "{\"jsonrpc\":\"2.0\",\"id\":2.0,\"method\":\"version\"}";
    const char *req2 = "{\"jsonrpc\":\"2.0\",\"id\":1.0,\"method\":\"hello\"}";
    const char *req3 = "{\"jsonrpc\":\"2.0\",\"id\":0.0,\"method\":\"do_crc32\",\"params\":[\"hi\"]}";
    const char *req4 = "[{\"jsonrpc\":\"2.0\",\"id\":1.0,\"method\":\"hello\"}]";
    const char *req5 = "[{\"jsonrpc\":\"2.0\",\"method\":\"hello\"}]";
    const char *req6 = "[]";
    const char *req7 = "{\"jsonrpc\":\"2.0\",\"id\":0.0,\"method\":\"do_crc32\",\"params\":[\"hello world jsonrpc\"]}";

    char *response = malloc(2048);
    (void) jsonrpc_handle_request_simple(&ctx, req0, strlen(req0), &response, 2048); printf("%s\n", response);
    (void) jsonrpc_handle_request_simple(&ctx, req1, strlen(req1), &response, 2048); printf("%s\n", response);
    (void) jsonrpc_handle_request_simple(&ctx, req2, strlen(req2), &response, 2048); printf("%s\n", response);
    (void) jsonrpc_handle_request_simple(&ctx, req3, strlen(req3), &response, 2048); printf("%s\n", response);
    (void) jsonrpc_handle_request_simple(&ctx, req4, strlen(req4), &response, 2048); printf("%s\n", response);
    (void) jsonrpc_handle_request_simple(&ctx, req5, strlen(req5), &response, 2048); printf("%s\n", response);
    (void) jsonrpc_handle_request_simple(&ctx, req6, strlen(req6), &response, 2048); printf("%s\n", response);
    (void) jsonrpc_handle_request_simple(&ctx, req7, strlen(req7), &response, 2048); printf("%s\n", response);
    free(response);

    jsonrpc_ctx_destroy(&ctx);
    return 0;
}

static RPC_HANDLER(version) {
    // Don't even bother
    IF_RPC_FLAG(FLAG_IS_NOTIF)
        return ERR_NOTIF;

    *response = json_string("idk");
    return ERR_NONE;
}

static RPC_HANDLER(hello) {
    IF_RPC_FLAG(FLAG_IS_NOTIF)
        return ERR_NOTIF;

    *response = json_string("world!");
    return ERR_NONE;
}

static RPC_HANDLER(do_crc32) {
    IF_RPC_FLAG(FLAG_IS_NOTIF)
        return ERR_NOTIF;

    json_t *_text = NULL;
    IF_RPC_FLAG(FLAG_ARRAY_PARAMS) {
        _text = json_array_get(parameters, 0);
    } else IF_RPC_FLAG(FLAG_KV_PARAMS) {
        _text = json_object_get(parameters, "text");
    }

    if(_text == NULL)
        return ERR_INVALID;

    const char *text = json_string_value(_text);

    // Do CRC-32
    unsigned long crc = crc32(0L, (const unsigned char *) text, strlen(text));
    char *hexstr = malloc(32);
    snprintf(hexstr, 31, "0x%lx", crc);
    *response = json_string(hexstr);
    free(hexstr);


    return ERR_NONE;
}
