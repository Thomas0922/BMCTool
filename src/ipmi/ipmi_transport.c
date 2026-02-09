#include "bmctool/ipmi_context.h"
#include "bmctool/ipmi.h"
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>

// 解析 hostname 或 IP
static int resolve_host(const char* host, struct in_addr* addr) {
    // 先試試是不是 IP address
    if (inet_pton(AF_INET, host, addr) == 1) {
        return 0;
    }
    
    // 不是 IP，試試 DNS lookup
    struct hostent* he = gethostbyname(host);
    if (!he) {
        bmc_log(LOG_LEVEL_ERROR, "gethostbyname(%s) failed", host);
        return -1;
    }
    
    memcpy(addr, he->h_addr_list[0], sizeof(struct in_addr));
    return 0;
}

int ipmi_send_recv(ipmi_ctx_t* ctx, const ipmi_msg_t* req, ipmi_msg_t* rsp) {
    if (!ctx || !req || !rsp) {
        return BMC_ERROR_INVALID_PARAM;
    }
    
    if (ctx->sockfd < 0) {
        bmc_log(LOG_LEVEL_ERROR, "Socket not open");
        return BMC_ERROR_NETWORK;
    }
    
    // 準備目標位址
    struct sockaddr_in dest_addr;
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(ctx->port);
    
    if (resolve_host(ctx->host, &dest_addr.sin_addr) != 0) {
        return BMC_ERROR_NETWORK;
    }
    
    // 建構 request 封包
    ipmi_msg_t req_copy = *req;
    req_copy.seq = ctx->seq++;  // 用 ctx 的 seq
    
    uint8_t send_buf[512];
    size_t send_len = sizeof(send_buf);
    
    int ret = ipmi_build_request(&req_copy, send_buf, &send_len);
    if (ret != BMC_SUCCESS) {
        bmc_log(LOG_LEVEL_ERROR, "Build request failed: %s", bmc_error_str(ret));
        return ret;
    }
    
    bmc_log(LOG_LEVEL_DEBUG, "Sending %zu bytes to %s:%d", 
            send_len, ctx->host, ctx->port);
    
    if (g_log_level == LOG_LEVEL_DEBUG) {
        ipmi_dump_packet(send_buf, send_len);
    }
    
    // 發送
    ssize_t sent = sendto(ctx->sockfd, send_buf, send_len, 0,
                          (struct sockaddr*)&dest_addr, sizeof(dest_addr));
    if (sent < 0) {
        bmc_log(LOG_LEVEL_ERROR, "sendto() failed: %s", strerror(errno));
        return BMC_ERROR_NETWORK;
    }
    
    if ((size_t)sent != send_len) {
        bmc_log(LOG_LEVEL_ERROR, "Partial send: %zd/%zu bytes", sent, send_len);
        return BMC_ERROR_NETWORK;
    }
    
    // 接收（帶 timeout）
    uint8_t recv_buf[512];
    struct sockaddr_in src_addr;
    socklen_t src_len = sizeof(src_addr);
    
    ssize_t received = recvfrom(ctx->sockfd, recv_buf, sizeof(recv_buf), 0,
                                (struct sockaddr*)&src_addr, &src_len);
    
    if (received < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            bmc_log(LOG_LEVEL_ERROR, "Timeout waiting for response");
            return BMC_ERROR_TIMEOUT;
        }
        bmc_log(LOG_LEVEL_ERROR, "recvfrom() failed: %s", strerror(errno));
        return BMC_ERROR_NETWORK;
    }
    
    bmc_log(LOG_LEVEL_DEBUG, "Received %zd bytes", received);
    
    if (g_log_level == LOG_LEVEL_DEBUG) {
        ipmi_dump_packet(recv_buf, received);
    }
    
    // 解析 response
    ret = ipmi_parse_response(recv_buf, received, rsp);
    if (ret != BMC_SUCCESS) {
        bmc_log(LOG_LEVEL_ERROR, "Parse response failed: %s", bmc_error_str(ret));
        return ret;
    }
    
    // 檢查 sequence number
    if (rsp->seq != req_copy.seq) {
        bmc_log(LOG_LEVEL_WARN, "Sequence mismatch: req=%d, rsp=%d", 
                req_copy.seq, rsp->seq);
    }
    
    return BMC_SUCCESS;
}
