#include "bmctool/ipmi_context.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <sys/time.h>

ipmi_ctx_t* ipmi_ctx_create(void) {
    ipmi_ctx_t* ctx = calloc(1, sizeof(ipmi_ctx_t));
    if (!ctx) {
        return NULL;
    }
    
    ctx->port = IPMI_DEFAULT_PORT;
    ctx->sockfd = -1;
    ctx->seq = 1;
    ctx->timeout_ms = 5000;  // 5 秒
    ctx->retries = 3;
    
    return ctx;
}

void ipmi_ctx_destroy(ipmi_ctx_t* ctx) {
    if (!ctx) {
        return;
    }
    
    if (ctx->sockfd >= 0) {
        close(ctx->sockfd);
    }
    
    free(ctx);
}

int ipmi_ctx_set_target(ipmi_ctx_t* ctx, const char* host, uint16_t port) {
    if (!ctx || !host) {
        return BMC_ERROR_INVALID_PARAM;
    }
    
    strncpy(ctx->host, host, sizeof(ctx->host) - 1);
    ctx->host[sizeof(ctx->host) - 1] = '\0';
    
    if (port > 0) {
        ctx->port = port;
    }
    
    return BMC_SUCCESS;
}

int ipmi_ctx_set_timeout(ipmi_ctx_t* ctx, int timeout_ms) {
    if (!ctx || timeout_ms <= 0) {
        return BMC_ERROR_INVALID_PARAM;
    }
    
    ctx->timeout_ms = timeout_ms;
    return BMC_SUCCESS;
}

int ipmi_ctx_open(ipmi_ctx_t* ctx) {
    if (!ctx) {
        return BMC_ERROR_INVALID_PARAM;
    }
    
    if (ctx->sockfd >= 0) {
        bmc_log(LOG_LEVEL_WARN, "Socket already open");
        return BMC_SUCCESS;
    }
    
    // 建立 UDP socket
    ctx->sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (ctx->sockfd < 0) {
        bmc_log(LOG_LEVEL_ERROR, "socket() failed: %s", strerror(errno));
        return BMC_ERROR_NETWORK;
    }
    
    // 設定 timeout
    struct timeval tv;
    tv.tv_sec = ctx->timeout_ms / 1000;
    tv.tv_usec = (ctx->timeout_ms % 1000) * 1000;
    
    if (setsockopt(ctx->sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        bmc_log(LOG_LEVEL_WARN, "setsockopt() failed: %s", strerror(errno));
    }
    
    bmc_log(LOG_LEVEL_DEBUG, "Socket opened: fd=%d", ctx->sockfd);
    return BMC_SUCCESS;
}

void ipmi_ctx_close(ipmi_ctx_t* ctx) {
    if (!ctx || ctx->sockfd < 0) {
        return;
    }
    
    close(ctx->sockfd);
    ctx->sockfd = -1;
    
    bmc_log(LOG_LEVEL_DEBUG, "Socket closed");
}
