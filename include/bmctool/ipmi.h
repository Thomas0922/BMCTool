#ifndef BMCTOOL_IPMI_H
#define BMCTOOL_IPMI_H

#include "bmctool/common.h"

/* RMCP 常數 */
#define RMCP_VERSION_1_0        0x06
#define RMCP_SEQUENCE_NO_ACK    0xFF
#define RMCP_CLASS_IPMI         0x07
#define RMCP_CLASS_ASF          0x06

/* IPMI 常數 */
#define IPMI_BMC_SLAVE_ADDR     0x20
#define IPMI_REMOTE_SWID        0x81
#define IPMI_MAX_DATA_SIZE      256
#define IPMI_DEFAULT_PORT       623

/* Authentication Type */
#define IPMI_AUTH_TYPE_NONE     0x00
#define IPMI_AUTH_TYPE_MD2      0x01
#define IPMI_AUTH_TYPE_MD5      0x02
#define IPMI_AUTH_TYPE_PASSWORD 0x04

/* NetFn (Network Function) */
#define IPMI_NETFN_CHASSIS      0x00
#define IPMI_NETFN_BRIDGE       0x02
#define IPMI_NETFN_SENSOR       0x04
#define IPMI_NETFN_APP          0x06
#define IPMI_NETFN_FIRMWARE     0x08
#define IPMI_NETFN_STORAGE      0x0A
#define IPMI_NETFN_TRANSPORT    0x0C

/* 常用命令 */
#define IPMI_CMD_GET_DEVICE_ID  0x01
#define IPMI_CMD_COLD_RESET     0x02
#define IPMI_CMD_WARM_RESET     0x03
#define IPMI_CMD_GET_SENSOR_READING  0x2D
#define IPMI_CMD_GET_SEL_INFO   0x40

/* RMCP Header */
typedef struct {
    uint8_t version;        // 0x06
    uint8_t reserved;       // 0x00
    uint8_t sequence;       // 0xFF for IPMI
    uint8_t class;          // 0x07 for IPMI
} __attribute__((packed)) rmcp_header_t;

/* IPMI Session Header (無認證) */
typedef struct {
    uint8_t auth_type;      // 認證類型
    uint32_t sequence;      // Session sequence number
    uint32_t id;            // Session ID
} __attribute__((packed)) ipmi_session_header_t;

/* IPMI Message Header */
typedef struct {
    uint8_t target_addr;    // 目標位址 (通常是 0x20)
    uint8_t target_lun;     // NetFn (6 bits) + LUN (2 bits)
    uint8_t header_checksum;// Header checksum
    uint8_t source_addr;    // 來源位址 (通常是 0x81)
    uint8_t source_lun;     // Sequence (6 bits) + LUN (2 bits)
    uint8_t cmd;            // 命令碼
} __attribute__((packed)) ipmi_msg_header_t;

/* 完整的 IPMI 訊息 */
typedef struct {
    uint8_t netfn;          // Network Function
    uint8_t cmd;            // Command
    uint8_t seq;            // Sequence number
    uint8_t data[IPMI_MAX_DATA_SIZE];  // 資料
    size_t data_len;        // 資料長度
} ipmi_msg_t;

/* IPMI 完整封包（用於網路傳輸） */
typedef struct {
    rmcp_header_t rmcp;
    ipmi_session_header_t session;
    uint8_t msg_len;        // Message length
    ipmi_msg_header_t msg_header;
    uint8_t data[IPMI_MAX_DATA_SIZE];
    uint8_t data_checksum;
} ipmi_packet_t;

/* 函式宣告 */

/* Checksum 計算 */
uint8_t ipmi_checksum(const uint8_t* data, size_t len);

/* 封包建構 */
int ipmi_build_request(const ipmi_msg_t* msg, uint8_t* buffer, size_t* len);

/* 封包解析 */
int ipmi_parse_response(const uint8_t* buffer, size_t len, ipmi_msg_t* msg);

/* 工具函式 */
void ipmi_dump_packet(const uint8_t* data, size_t len);
const char* ipmi_netfn_str(uint8_t netfn);
const char* ipmi_cmd_str(uint8_t netfn, uint8_t cmd);

#endif /* BMCTOOL_IPMI_H */

// Chassis commands
#define IPMI_CMD_GET_CHASSIS_STATUS  0x01
