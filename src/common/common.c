#include "bmctool/common.h"
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <ctype.h>

log_level_t g_log_level = LOG_LEVEL_INFO;
output_format_t g_output_format = OUTPUT_FORMAT_NORMAL;

// 錯誤碼轉字串
const char* bmc_error_str(int error_code) {
    switch (error_code) {
        case BMC_SUCCESS:              return "Success";
        case BMC_ERROR_INVALID_PARAM:  return "Invalid parameter";
        case BMC_ERROR_MEMORY:         return "Memory error";
        case BMC_ERROR_NETWORK:        return "Network error";
        case BMC_ERROR_TIMEOUT:        return "Timeout";
        case BMC_ERROR_PROTOCOL:       return "Protocol error";
        default:                       return "Unknown error";
    }
}

// 日誌系統
void bmc_log(log_level_t level, const char* fmt, ...) {
    if (level > g_log_level) {
        return;
    }
    
    const char* level_str[] = {"ERROR", "WARN", "INFO", "DEBUG"};
    
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    char time_str[32];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);
    
    fprintf(stderr, "[%s] [%s] ", time_str, level_str[level]);
    
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    
    fprintf(stderr, "\n");
}

// Hex dump
void bmc_hexdump(const uint8_t* data, size_t len) {
    for (size_t i = 0; i < len; i += 16) {
        printf("%04zx: ", i);
        
        // Hex 部分
        for (size_t j = 0; j < 16; j++) {
            if (i + j < len) {
                printf("%02x ", data[i + j]);
            } else {
                printf("   ");
            }
            if (j == 7) printf(" ");
        }
        
        printf(" |");
        
        // ASCII 部分
        for (size_t j = 0; j < 16 && i + j < len; j++) {
            uint8_t c = data[i + j];
            printf("%c", isprint(c) ? c : '.');
        }
        
        printf("|\n");
    }
}
