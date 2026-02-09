#include "bmctool/redfish.h"
#include <stdlib.h>
#include <string.h>

redfish_ctx_t* redfish_ctx_create(void) {
    redfish_ctx_t* ctx = calloc(1, sizeof(redfish_ctx_t));
    if (!ctx) {
        return NULL;
    }
    
    ctx->use_https = 1;      // 預設用 HTTPS
    ctx->verify_ssl = 0;     // 測試時不驗證 SSL
    
    return ctx;
}

void redfish_ctx_destroy(redfish_ctx_t* ctx) {
    if (!ctx) {
        return;
    }
    free(ctx);
}

int redfish_ctx_set_endpoint(redfish_ctx_t* ctx, const char* url) {
    if (!ctx || !url) {
        return BMC_ERROR_INVALID_PARAM;
    }
    
    strncpy(ctx->base_url, url, sizeof(ctx->base_url) - 1);
    ctx->base_url[sizeof(ctx->base_url) - 1] = '\0';
    
    // 檢查是 http 還是 https
    if (strncmp(url, "http://", 7) == 0) {
        ctx->use_https = 0;
    } else if (strncmp(url, "https://", 8) == 0) {
        ctx->use_https = 1;
    }
    
    return BMC_SUCCESS;
}

int redfish_ctx_set_auth(redfish_ctx_t* ctx, const char* username, const char* password) {
    if (!ctx) {
        return BMC_ERROR_INVALID_PARAM;
    }
    
    if (username) {
        strncpy(ctx->username, username, sizeof(ctx->username) - 1);
        ctx->username[sizeof(ctx->username) - 1] = '\0';
    }
    
    if (password) {
        strncpy(ctx->password, password, sizeof(ctx->password) - 1);
        ctx->password[sizeof(ctx->password) - 1] = '\0';
    }
    
    return BMC_SUCCESS;
}
