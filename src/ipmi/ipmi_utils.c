#include "bmctool/ipmi.h"
#include <stdio.h>

// NetFn 轉字串
const char* ipmi_netfn_str(uint8_t netfn) {
    switch (netfn) {
        case IPMI_NETFN_CHASSIS:        return "Chassis";
        case IPMI_NETFN_BRIDGE:         return "Bridge";
        case IPMI_NETFN_SENSOR:         return "Sensor/Event";
        case IPMI_NETFN_APP:            return "App";
        case IPMI_NETFN_FIRMWARE:       return "Firmware";
        case IPMI_NETFN_STORAGE:        return "Storage";
        case IPMI_NETFN_TRANSPORT:      return "Transport";
        default:                        return "Unknown";
    }
}

// Command 轉字串
const char* ipmi_cmd_str(uint8_t netfn, uint8_t cmd) {
    if (netfn == IPMI_NETFN_APP && cmd == IPMI_CMD_GET_DEVICE_ID) {
        return "Get Device ID";
    }
    return "Unknown";
}

// 詳細顯示封包內容
void ipmi_dump_packet(const uint8_t* data, size_t len) {
    if (!data || len < 14) {
        printf("Invalid packet (too short)\n");
        return;
    }
    
    printf("=== IPMI Packet Analysis ===\n");
    
    // RMCP Header (4 bytes)
    printf("[RMCP Header]\n");
    printf("  Version: 0x%02x", data[0]);
    if (data[0] == 0x06) printf(" (RMCP v1.0)");
    printf("\n");
    printf("  Reserved: 0x%02x\n", data[1]);
    printf("  Sequence: 0x%02x", data[2]);
    if (data[2] == 0xFF) printf(" (No ACK)");
    printf("\n");
    printf("  Class: 0x%02x", data[3]);
    if (data[3] == 0x07) printf(" (IPMI)");
    printf("\n");
    
    // Session Header (9 bytes for no-auth)
    printf("[Session Header]\n");
    printf("  Auth Type: 0x%02x", data[4]);
    if (data[4] == 0x00) printf(" (None)");
    printf("\n");
    
    uint32_t seq = (data[5]) | (data[6] << 8) | (data[7] << 16) | (data[8] << 24);
    printf("  Sequence: 0x%08x\n", seq);
    
    uint32_t session_id = (data[9]) | (data[10] << 8) | (data[11] << 16) | (data[12] << 24);
    printf("  Session ID: 0x%08x\n", session_id);
    
    // Message length
    uint8_t msg_len = data[13];
    printf("[IPMI Message] (Length: %d bytes)\n", msg_len);
    
    if (len < (size_t)(14 + msg_len)) {
        printf("  ERROR: Packet truncated\n");
        return;
    }
    
    // Message header
    uint8_t target_addr = data[14];
    uint8_t netfn_lun = data[15];
    uint8_t netfn = (netfn_lun >> 2) & 0x3F;
    uint8_t lun = netfn_lun & 0x03;
    uint8_t header_checksum = data[16];
    
    printf("  Target Addr: 0x%02x", target_addr);
    if (target_addr == IPMI_BMC_SLAVE_ADDR) printf(" (BMC)");
    printf("\n");
    
    printf("  NetFn: 0x%02x (%s)\n", netfn, ipmi_netfn_str(netfn));
    printf("  LUN: 0x%02x\n", lun);
    printf("  Header Checksum: 0x%02x\n", header_checksum);
    
    uint8_t source_addr = data[17];
    uint8_t seq_lun = data[18];
    uint8_t req_seq = (seq_lun >> 2) & 0x3F;
    uint8_t req_lun = seq_lun & 0x03;
    uint8_t cmd = data[19];
    
    printf("  Source Addr: 0x%02x\n", source_addr);
    printf("  Request Seq: 0x%02x\n", req_seq);
    printf("  Request LUN: 0x%02x\n", req_lun);
    printf("  Command: 0x%02x (%s)\n", cmd, ipmi_cmd_str(netfn, cmd));
    
    // Data (if any)
    size_t data_len = msg_len - 7;  // 扣掉 header (6 bytes) 和 checksum (1 byte)
    if (data_len > 0) {
        printf("[Data] (%zu bytes)\n  ", data_len);
        for (size_t i = 0; i < data_len; i++) {
            printf("%02x ", data[20 + i]);
        }
        printf("\n");
    }
    
    // Data checksum
    if (len >= (size_t)(14 + msg_len)) {
        uint8_t data_checksum = data[14 + msg_len - 1];
        printf("[Data Checksum]\n");
        printf("  Checksum: 0x%02x\n", data_checksum);
    }
    
    // Raw hex dump
    printf("=== Raw Hex Dump ===\n");
    printf("Full Packet (%zu bytes):\n", len);
    bmc_hexdump(data, len);
}
