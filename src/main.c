#include "jsonrpc.h"
#include <string.h>
#include <zlib.h>

#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/err.h>

#include <sys/stat.h> // for chmod(3)

static RPC_HANDLER(version);
static RPC_HANDLER(hello);
static RPC_HANDLER(do_crc32);

static json_t *add_signature(jsonrpc_ctx *ctx, const char* method, json_t *original);

typedef struct key_ctx_s {
    EVP_PKEY *pkey;
} key_ctx;

const struct jsonrpc_handler handlers[] = {
    RPC_ADD_HANDLER(version),
    RPC_ADD_HANDLER(hello),
    RPC_ADD_HANDLER(do_crc32),
    RPC_HANDLERS_END
};

int main(void) {
    // Initialize OpenSSL
    ERR_load_crypto_strings();
    OpenSSL_add_all_algorithms();
    OPENSSL_init();

    // Initialize ed25519 context
    key_ctx sign_key = {0};

    char *errbuf = malloc(256);
    FILE *certf = fopen("ed25519.pem", "r");
    if(certf == NULL) {
        // Generate key
        EVP_PKEY_CTX *pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_ED25519, NULL);
        if(EVP_PKEY_keygen_init(pctx) < 1) {
            fprintf(stderr, "Failed to initialize keygen: %s\n", ERR_error_string(ERR_get_error(), errbuf));
            free(errbuf);
            return 1;
        }
        if(EVP_PKEY_keygen(pctx, &sign_key.pkey) < 1) {
            fprintf(stderr, "Failed to generate ed25519 key: %s\n", ERR_error_string(ERR_get_error(), errbuf));
            free(errbuf);
            return 1;
        }
        EVP_PKEY_CTX_free(pctx);

        // Write it to file
        certf = fopen("ed25519.pem", "w");
        if(PEM_write_PrivateKey(certf, sign_key.pkey, NULL, NULL, 0, NULL, NULL) < 1) {
            fprintf(stderr, "Failed to write ed25519 key on disk: %s\n", ERR_error_string(ERR_get_error(), errbuf));
            free(errbuf);
            fclose(certf);
            return 1;
        }

        // Set file access mode to 0600
        if(chmod("ed25519.pem", S_IRUSR | S_IWUSR) < 0) {
            perror("chmod");
        }
    } else {
        // Load private key
        if(PEM_read_PrivateKey(certf, &sign_key.pkey, NULL, NULL) == NULL) {
            fprintf(stderr, "Failed to load ed25519 key from disk: %s\n", ERR_error_string(ERR_get_error(), errbuf));
            free(errbuf);
            fclose(certf);
            return 1;
        }
    }
    fclose(certf);
    free(errbuf);

    // Initialize JSON-RPC handler
    jsonrpc_ctx ctx = {0};
    ctx.handlers = handlers;
    ctx.response_transformer = add_signature;
    ctx.data = &sign_key;
    jsonrpc_ctx_init(&ctx);

    const char *req0 = "{\"jsonrpc\":\"2.0\",\"id\":3.0,\"method\":\"version\",\"params\":[]}";
    const char *req1 = "{\"jsonrpc\":\"2.0\",\"id\":2.0,\"method\":\"version\"}";
    const char *req2 = "{\"jsonrpc\":\"2.0\",\"id\":1.0,\"method\":\"hello\"}";
    const char *req3 = "{\"jsonrpc\":\"2.0\",\"id\":0.0,\"method\":\"do_crc32\",\"params\":[\"hi\"]}";
    const char *req4 = "[{\"jsonrpc\":\"2.0\",\"id\":1.0,\"method\":\"hello\"}]";
    const char *req5 = "[{\"jsonrpc\":\"2.0\",\"method\":\"hello\"}]";
    const char *req6 = "[]";
    const char *req7 = "{\"jsonrpc\":\"2.0\",\"id\":0.0,\"method\":\"do_crc32\",\"params\":[\"hello world jsonrpc\"]}";

    json_error_t err = {0};
    char *response = malloc(2048);
    (void) jsonrpc_handle_request_simple(&ctx, req0, strlen(req0), &response, 2048, &err); printf("%s\n", response);
    (void) jsonrpc_handle_request_simple(&ctx, req1, strlen(req1), &response, 2048, &err); printf("%s\n", response);
    (void) jsonrpc_handle_request_simple(&ctx, req2, strlen(req2), &response, 2048, &err); printf("%s\n", response);
    (void) jsonrpc_handle_request_simple(&ctx, req3, strlen(req3), &response, 2048, &err); printf("%s\n", response);
    (void) jsonrpc_handle_request_simple(&ctx, req4, strlen(req4), &response, 2048, &err); printf("%s\n", response);
    (void) jsonrpc_handle_request_simple(&ctx, req5, strlen(req5), &response, 2048, &err); printf("%s\n", response);
    (void) jsonrpc_handle_request_simple(&ctx, req6, strlen(req6), &response, 2048, &err); printf("%s\n", response);
    (void) jsonrpc_handle_request_simple(&ctx, req7, strlen(req7), &response, 2048, &err); printf("%s\n", response);
    free(response);

    jsonrpc_ctx_destroy(&ctx);

    // Deinitialize OpenSSL
    EVP_PKEY_free(sign_key.pkey);
    EVP_cleanup();
    CRYPTO_cleanup_all_ex_data();
    ERR_free_strings();
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

static json_t *add_signature(jsonrpc_ctx *ctx, const char* method, json_t *original) {
    // Grab key context
    key_ctx *key = (key_ctx *) ctx->data;

    // Initialize signing context
    char *errbuf = malloc(256);
    EVP_MD_CTX *mdctx = NULL;
    if((mdctx = EVP_MD_CTX_create()) == NULL) {
        fprintf(stderr, "ERROR: failed to create signing context! %s\n", ERR_error_string(ERR_get_error(), errbuf));
        free(errbuf);
        return original;
    }

    if(EVP_DigestSignInit(mdctx, NULL, NULL, NULL, key->pkey) < 1) {
        fprintf(stderr, "ERROR: failed to init signing context! %s\n", ERR_error_string(ERR_get_error(), errbuf));
        EVP_MD_CTX_destroy(mdctx);
        free(errbuf);
        return original;
    }

    // Concat method name and response
    char *buffer;
    char *dump = json_dumps(original, 0);

    size_t len = strlen(method) + strlen(dump) + 1;
    buffer = malloc(len);
    snprintf(buffer, len, "%s%s", method, dump);
    free(dump);

    // Initialize signed data buffer
    size_t sign_buffer_size = EVP_PKEY_size(key->pkey);
    unsigned char *sign_buffer = OPENSSL_malloc(sign_buffer_size);

    // Sign data
    if(EVP_DigestSign(mdctx, sign_buffer, &sign_buffer_size, (unsigned char *) buffer, len) < 1) {
        fprintf(stderr, "ERROR: failed to sign data! %s\n", ERR_error_string(ERR_get_error(), errbuf));
        OPENSSL_clear_free((char *) sign_buffer, sign_buffer_size);
        EVP_MD_CTX_destroy(mdctx);
        free(errbuf);
        free(buffer);
        return original;
    }

    // Encode to base64
    BIO *b64 = BIO_new(BIO_f_base64());
    BIO *bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);
    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL); // Don't allow newlines
    BIO_write(bio, sign_buffer, sign_buffer_size);
    BIO_flush(bio);

    // Grab encoded buffer
    BUF_MEM *encoded_ptr = NULL;
    BIO_get_mem_ptr(bio, &encoded_ptr);

    // Add to original
    json_object_set_new(original, "signature", json_stringn(encoded_ptr->data, encoded_ptr->length));

    // Free resources
    BIO_set_close(bio, BIO_CLOSE);
    BIO_free_all(bio);
    free(errbuf);
    free(buffer);
    OPENSSL_clear_free((char *) sign_buffer, sign_buffer_size);
    EVP_MD_CTX_destroy(mdctx);

    return original;
}
