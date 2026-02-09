#include "bmctool/ipmi.h"
#include <string.h>
#include <arpa/inet.h>  // for htonl

/**
 * 建構 IPMI request 封包
 * 
 * @param msg - IPMI 訊息（包含 netfn, cmd, data）
 * @param buffer - 輸出緩衝區
 * @param len - 輸入時為緩衝區大小，輸出時為實際封包長度
 * @return 0 成功，負數失敗
 */
int ipmi_build_request(const ipmi_msg_t* msg, uint8_t* buffer, size_t* len) {
    if (!msg || !buffer || !len) {
        return BMC_ERROR_INVALID_PARAM;
    }
    
    // 檢查緩衝區是否足夠
    size_t required_size = sizeof(rmcp_header_t) + 
                          sizeof(ipmi_session_header_t) +
                          1 +  // msg_len
                          sizeof(ipmi_msg_header_t) +
                          msg->data_len +
                          1;   // data_checksum
    
    if (*len < required_size) {
        return BMC_ERROR_MEMORY;
    }
    
    uint8_t* ptr = buffer;
    
    /* 1. RMCP Header */
    rmcp_header_t* rmcp = (rmcp_header_t*)ptr;
    rmcp->version = RMCP_VERSION_1_0;
    rmcp->reserved = 0x00;
    rmcp->sequence = RMCP_SEQUENCE_NO_ACK;
    rmcp->class = RMCP_CLASS_IPMI;
    ptr += sizeof(rmcp_header_t);
    
    /* 2. Session Header (無認證) */
    ipmi_session_header_t* session = (ipmi_session_header_t*)ptr;
    session->auth_type = IPMI_AUTH_TYPE_NONE;
    session->sequence = 0;  // 暫時用 0，實際應該遞增
    session->id = 0;        // Session ID = 0 表示無 session
    ptr += sizeof(ipmi_session_header_t);
    
    /* 3. Message Length */
    size_t msg_body_len = sizeof(ipmi_msg_header_t) + msg->data_len + 1;
    *ptr++ = (uint8_t)msg_body_len;
    
    /* 4. IPMI Message Header */
    ipmi_msg_header_t* msg_hdr = (ipmi_msg_header_t*)ptr;
    msg_hdr->target_addr = IPMI_BMC_SLAVE_ADDR;
    msg_hdr->target_lun = (msg->netfn << 2);  // NetFn in upper 6 bits, LUN=0
    
    // 計算 header checksum
    uint8_t header_data[2];
    header_data[0] = msg_hdr->target_addr;
    header_data[1] = msg_hdr->target_lun;
    msg_hdr->header_checksum = ipmi_checksum(header_data, 2);
    
    msg_hdr->source_addr = IPMI_REMOTE_SWID;
    msg_hdr->source_lun = (msg->seq << 2);    // Seq in upper 6 bits, LUN=0
    msg_hdr->cmd = msg->cmd;
    ptr += sizeof(ipmi_msg_header_t);
    
    /* 5. Data */
    if (msg->data_len > 0) {
        memcpy(ptr, msg->data, msg->data_len);
        ptr += msg->data_len;
    }
    
    /* 6. Data Checksum */
    // Checksum 包含：source_addr, source_lun, cmd, data
    size_t checksum_data_len = 3 + msg->data_len;  // 3 = source_addr + source_lun + cmd
    uint8_t* checksum_start = (uint8_t*)&msg_hdr->source_addr;
    *ptr++ = ipmi_checksum(checksum_start, checksum_data_len);
    
    /* 更新實際長度 */
    *len = ptr - buffer;
    
    return BMC_SUCCESS;
}

/**
 * 解析 IPMI response 封包
 * 
 * @param buffer - 接收到的封包
 * @param len - 封包長度
 * @param msg - 輸出的 IPMI 訊息
 * @return 0 成功，負數失敗
 */
int ipmi_parse_response(const uint8_t* buffer, size_t len, ipmi_msg_t* msg) {
    if (!buffer || !msg || len < 20) {  // 最小封包大小
        return BMC_ERROR_INVALID_PARAM;
    }
    
    const uint8_t* ptr = buffer;
    
    /* 1. 驗證 RMCP Header */
    const rmcp_header_t* rmcp = (const rmcp_header_t*)ptr;
    if (rmcp->version != RMCP_VERSION_1_0 || rmcp->class != RMCP_CLASS_IPMI) {
        bmc_log(LOG_LEVEL_ERROR, "Invalid RMCP header");
        return BMC_ERROR_PROTOCOL;
    }
    ptr += sizeof(rmcp_header_t);
    
    /* 2. Session Header */
    const ipmi_session_header_t* session = (const ipmi_session_header_t*)ptr;
    if (session->auth_type != IPMI_AUTH_TYPE_NONE) {
        bmc_log(LOG_LEVEL_WARN, "Authentication not supported yet");
    }
    ptr += sizeof(ipmi_session_header_t);
    
    /* 3. Message Length */
    uint8_t msg_len = *ptr++;
    if (ptr + msg_len > buffer + len) {
        bmc_log(LOG_LEVEL_ERROR, "Invalid message length");
        return BMC_ERROR_PROTOCOL;
    }
    
    /* 4. IPMI Message Header */
    const ipmi_msg_header_t* msg_hdr = (const ipmi_msg_header_t*)ptr;
    
    // 驗證 header checksum
    uint8_t header_data[2];
    header_data[0] = msg_hdr->target_addr;
    header_data[1] = msg_hdr->target_lun;
    uint8_t expected_header_checksum = ipmi_checksum(header_data, 2);
    if (msg_hdr->header_checksum != expected_header_checksum) {
        bmc_log(LOG_LEVEL_ERROR, "Header checksum mismatch: expected 0x%02x, got 0x%02x",
                expected_header_checksum, msg_hdr->header_checksum);
        return BMC_ERROR_PROTOCOL;
    }
    
    ptr += sizeof(ipmi_msg_header_t);
    
    /* 5. 提取資料 */
    msg->netfn = (msg_hdr->target_lun >> 2);  // Response 的 NetFn = Request NetFn | 1
    msg->cmd = msg_hdr->cmd;
    msg->seq = (msg_hdr->source_lun >> 2);
    
    // Data length = msg_len - header - data_checksum
    msg->data_len = msg_len - sizeof(ipmi_msg_header_t) - 1;
    
    if (msg->data_len > 0) {
        if (msg->data_len > IPMI_MAX_DATA_SIZE) {
            bmc_log(LOG_LEVEL_ERROR, "Data too large: %zu bytes", msg->data_len);
            return BMC_ERROR_PROTOCOL;
        }
        memcpy(msg->data, ptr, msg->data_len);
        ptr += msg->data_len;
    }
    
    /* 6. 驗證 data checksum */
    uint8_t data_checksum = *ptr;
    size_t checksum_data_len = 3 + msg->data_len;
    const uint8_t* checksum_start = (const uint8_t*)&msg_hdr->source_addr;
    uint8_t expected_data_checksum = ipmi_checksum(checksum_start, checksum_data_len);
    if (data_checksum != expected_data_checksum) {
        bmc_log(LOG_LEVEL_ERROR, "Data checksum mismatch: expected 0x%02x, got 0x%02x",
                expected_data_checksum, data_checksum);
        return BMC_ERROR_PROTOCOL;
    }
    
    return BMC_SUCCESS;
}
