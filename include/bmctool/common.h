#ifndef BMCTOOL_COMMON_H
#define BMCTOOL_COMMON_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/* 版本資訊 */
#define BMCTOOL_VERSION_MAJOR 0
#define BMCTOOL_VERSION_MINOR 1
#define BMCTOOL_VERSION_PATCH 0

/* 錯誤碼 */
typedef enum {
    BMC_SUCCESS = 0,
    BMC_ERROR_INVALID_PARAM = -1,
    BMC_ERROR_MEMORY = -2,
    BMC_ERROR_NETWORK = -3,
    BMC_ERROR_TIMEOUT = -4,
    BMC_ERROR_AUTH = -5,
    BMC_ERROR_PROTOCOL = -6,
    BMC_ERROR_UNKNOWN = -99
} bmc_error_t;

/* 日誌等級 */
typedef enum {
    LOG_LEVEL_DEBUG = 0,
    LOG_LEVEL_INFO = 1,
    LOG_LEVEL_WARN = 2,
    LOG_LEVEL_ERROR = 3
} log_level_t;

/* 全域設定 */
extern log_level_t g_log_level;

/* 工具函式宣告 */
const char* bmc_error_str(bmc_error_t err);
void bmc_log(log_level_t level, const char* fmt, ...);
void bmc_hexdump(const uint8_t* data, size_t len, const char* prefix);

#endif /* BMCTOOL_COMMON_H */
