#include "bmctool/ipmi_commands.h"
#include <string.h>

int ipmi_cmd_get_device_id(ipmi_ctx_t* ctx, ipmi_device_id_t* device_id) {
    if (!ctx || !device_id) {
        return BMC_ERROR_INVALID_PARAM;
    }
    
    // 建構 request
    ipmi_msg_t req = {
        .netfn = IPMI_NETFN_APP,
        .cmd = IPMI_CMD_GET_DEVICE_ID,
        .seq = 0,  // 會被 ipmi_send_recv 填入
        .data_len = 0
    };
    
    ipmi_msg_t rsp;
    
    // 送出並等回應
    int ret = ipmi_send_recv(ctx, &req, &rsp);
    if (ret != BMC_SUCCESS) {
        return ret;
    }
    
    // 檢查 completion code
    if (rsp.data_len < 1) {
        bmc_log(LOG_LEVEL_ERROR, "Response too short");
        return BMC_ERROR_PROTOCOL;
    }
    
    uint8_t cc = rsp.data[0];
    if (cc != 0x00) {
        bmc_log(LOG_LEVEL_ERROR, "Command failed: completion code 0x%02x", cc);
        return BMC_ERROR_PROTOCOL;
    }
    
    // 解析回應（最少需要 12 bytes：cc + 11 bytes data）
    if (rsp.data_len < 12) {
        bmc_log(LOG_LEVEL_ERROR, "Response data too short: %zu bytes", rsp.data_len);
        return BMC_ERROR_PROTOCOL;
    }
    
    // 填入結構
    device_id->device_id = rsp.data[1];
    device_id->device_revision = rsp.data[2] & 0x0F;
    device_id->firmware_rev1 = rsp.data[3] & 0x7F;
    device_id->firmware_rev2 = rsp.data[4];
    device_id->ipmi_version = rsp.data[5];
    device_id->additional_support = rsp.data[6];
    
    memcpy(device_id->manufacturer_id, &rsp.data[7], 3);
    memcpy(device_id->product_id, &rsp.data[10], 2);
    
    // aux firmware rev 是 optional
    if (rsp.data_len >= 16) {
        memcpy(device_id->aux_firmware_rev, &rsp.data[12], 4);
    } else {
        memset(device_id->aux_firmware_rev, 0, 4);
    }
    
    return BMC_SUCCESS;
}

int ipmi_cmd_get_chassis_status(ipmi_ctx_t* ctx, ipmi_chassis_status_t* status) {
    if (!ctx || !status) {
        return BMC_ERROR_INVALID_PARAM;
    }
    
    // 建構 request
    ipmi_msg_t req = {
        .netfn = IPMI_NETFN_CHASSIS,
        .cmd = 0x01,  // Get Chassis Status
        .seq = 0,
        .data_len = 0
    };
    
    ipmi_msg_t rsp;
    
    int ret = ipmi_send_recv(ctx, &req, &rsp);
    if (ret != BMC_SUCCESS) {
        return ret;
    }
    
    // 檢查 completion code
    if (rsp.data_len < 1 || rsp.data[0] != 0x00) {
        bmc_log(LOG_LEVEL_ERROR, "Command failed: completion code 0x%02x", 
                rsp.data_len > 0 ? rsp.data[0] : 0xFF);
        return BMC_ERROR_PROTOCOL;
    }
    
    // 解析回應（至少需要 4 bytes）
    if (rsp.data_len < 4) {
        bmc_log(LOG_LEVEL_ERROR, "Response too short: %zu bytes", rsp.data_len);
        return BMC_ERROR_PROTOCOL;
    }
    
    status->current_power_state = rsp.data[1];
    status->last_power_event = rsp.data[2];
    status->misc_chassis_state = rsp.data[3];
    status->front_panel_button = (rsp.data_len >= 5) ? rsp.data[4] : 0;
    
    return BMC_SUCCESS;
}
