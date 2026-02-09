#include "bmctool/redfish.h"

// 宣告內部函式
extern int http_get(redfish_ctx_t* ctx, const char* path, char** response_out);
extern int redfish_parse_system(const char* json_str, redfish_system_t* system);

int redfish_get_system(redfish_ctx_t* ctx, const char* system_id, redfish_system_t* system) {
    if (!ctx || !system_id || !system) {
        return BMC_ERROR_INVALID_PARAM;
    }
    
    // 建構 path
    char path[256];
    snprintf(path, sizeof(path), "/redfish/v1/Systems/%s", system_id);
    
    // 執行 HTTP GET
    char* response = NULL;
    int ret = http_get(ctx, path, &response);
    if (ret != BMC_SUCCESS) {
        return ret;
    }
    
    // 解析 JSON
    ret = redfish_parse_system(response, system);
    
    free(response);
    return ret;
}

int redfish_get_thermal(redfish_ctx_t* ctx, const char* chassis_id) {
    if (!ctx || !chassis_id) {
        return BMC_ERROR_INVALID_PARAM;
    }
    
    char path[256];
    snprintf(path, sizeof(path), "/redfish/v1/Chassis/%s/Thermal", chassis_id);
    
    char* response = NULL;
    int ret = http_get(ctx, path, &response);
    
    if (ret == BMC_SUCCESS && response) {
        // 簡單印出 JSON（之後可以詳細解析）
        printf("%s\n", response);
        free(response);
    }
    
    return ret;
}
