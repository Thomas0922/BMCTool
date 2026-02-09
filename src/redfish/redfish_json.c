#include "bmctool/redfish.h"
#include <json-c/json.h>
#include <string.h>

// 安全地從 JSON 取得字串
static void json_get_string(struct json_object* obj, const char* key, char* dest, size_t dest_size) {
    struct json_object* val;
    if (json_object_object_get_ex(obj, key, &val)) {
        const char* str = json_object_get_string(val);
        if (str) {
            strncpy(dest, str, dest_size - 1);
            dest[dest_size - 1] = '\0';
        }
    }
}

int redfish_parse_system(const char* json_str, redfish_system_t* system) {
    if (!json_str || !system) {
        return BMC_ERROR_INVALID_PARAM;
    }
    
    struct json_object* root = json_tokener_parse(json_str);
    if (!root) {
        bmc_log(LOG_LEVEL_ERROR, "JSON parse error");
        return BMC_ERROR_PROTOCOL;
    }
    
    // 解析各欄位
    json_get_string(root, "Id", system->id, sizeof(system->id));
    json_get_string(root, "Name", system->name, sizeof(system->name));
    json_get_string(root, "Manufacturer", system->manufacturer, sizeof(system->manufacturer));
    json_get_string(root, "Model", system->model, sizeof(system->model));
    json_get_string(root, "SerialNumber", system->serial_number, sizeof(system->serial_number));
    json_get_string(root, "PowerState", system->power_state, sizeof(system->power_state));
    
    // BiosVersion 可能在 Bios 物件內
    struct json_object* bios;
    if (json_object_object_get_ex(root, "BiosVersion", &bios)) {
        json_get_string(root, "BiosVersion", system->bios_version, sizeof(system->bios_version));
    }
    
    json_object_put(root);
    return BMC_SUCCESS;
}
