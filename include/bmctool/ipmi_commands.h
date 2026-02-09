#ifndef BMCTOOL_IPMI_COMMANDS_H
#define BMCTOOL_IPMI_COMMANDS_H

#include "bmctool/ipmi_context.h"

// Get Device ID response
typedef struct {
    uint8_t device_id;
    uint8_t device_revision;
    uint8_t firmware_rev1;
    uint8_t firmware_rev2;
    uint8_t ipmi_version;
    uint8_t additional_support;
    uint8_t manufacturer_id[3];
    uint8_t product_id[2];
    uint8_t aux_firmware_rev[4];
} ipmi_device_id_t;

// 命令函式
int ipmi_cmd_get_device_id(ipmi_ctx_t* ctx, ipmi_device_id_t* device_id);

#endif

// Chassis Status response
typedef struct {
    uint8_t current_power_state;
    uint8_t last_power_event;
    uint8_t misc_chassis_state;
    uint8_t front_panel_button;
} ipmi_chassis_status_t;

// Chassis 命令
int ipmi_cmd_get_chassis_status(ipmi_ctx_t* ctx, ipmi_chassis_status_t* status);
