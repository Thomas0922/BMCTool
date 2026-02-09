#ifndef BMCTOOL_REDFISH_H
#define BMCTOOL_REDFISH_H

#include "bmctool/common.h"

// Redfish context
typedef struct {
    char base_url[256];      // 例如 https://192.168.1.100
    char username[64];
    char password[64];
    int use_https;
    int verify_ssl;
} redfish_ctx_t;

// Redfish System 資訊
typedef struct {
    char id[64];
    char name[128];
    char manufacturer[128];
    char model[128];
    char serial_number[128];
    char power_state[32];     // "On", "Off", "PoweringOn", etc.
    char bios_version[64];
} redfish_system_t;

// Context 操作
redfish_ctx_t* redfish_ctx_create(void);
void redfish_ctx_destroy(redfish_ctx_t* ctx);

int redfish_ctx_set_endpoint(redfish_ctx_t* ctx, const char* url);
int redfish_ctx_set_auth(redfish_ctx_t* ctx, const char* username, const char* password);

// API 呼叫
int redfish_get_system(redfish_ctx_t* ctx, const char* system_id, redfish_system_t* system);
int redfish_get_thermal(redfish_ctx_t* ctx, const char* chassis_id);

#endif
