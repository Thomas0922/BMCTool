#ifndef BMCTOOL_IPMI_CONTEXT_H
#define BMCTOOL_IPMI_CONTEXT_H

#include "bmctool/common.h"
#include "bmctool/ipmi.h"
#include <stdint.h>

// IPMI 連線設定
typedef struct {
    char host[256];          // BMC IP 或 hostname
    uint16_t port;           // 預設 623
    char username[32];       // 使用者名稱（目前未用）
    char password[32];       // 密碼（目前未用）
    
    int sockfd;              // UDP socket fd
    uint8_t seq;             // Request sequence number
    
    int timeout_ms;          // Timeout（毫秒）
    int retries;             // 重試次數
} ipmi_ctx_t;

// Context 操作
ipmi_ctx_t* ipmi_ctx_create(void);
void ipmi_ctx_destroy(ipmi_ctx_t* ctx);

int ipmi_ctx_set_target(ipmi_ctx_t* ctx, const char* host, uint16_t port);
int ipmi_ctx_set_timeout(ipmi_ctx_t* ctx, int timeout_ms);

// 連線管理
int ipmi_ctx_open(ipmi_ctx_t* ctx);
void ipmi_ctx_close(ipmi_ctx_t* ctx);

// 收發封包
int ipmi_send_recv(ipmi_ctx_t* ctx, const ipmi_msg_t* req, ipmi_msg_t* rsp);

#endif
