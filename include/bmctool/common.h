#ifndef BMCTOOL_COMMON_H
#define BMCTOOL_COMMON_H

#include <stdint.h>
#include <stddef.h>

// Error codes
#define BMC_SUCCESS              0
#define BMC_ERROR_INVALID_PARAM  -1
#define BMC_ERROR_MEMORY         -2
#define BMC_ERROR_NETWORK        -3
#define BMC_ERROR_TIMEOUT        -4
#define BMC_ERROR_PROTOCOL       -5

const char* bmc_error_str(int error_code);

// Log levels
typedef enum {
    LOG_LEVEL_ERROR = 0,
    LOG_LEVEL_WARN  = 1,
    LOG_LEVEL_INFO  = 2,
    LOG_LEVEL_DEBUG = 3
} log_level_t;

extern log_level_t g_log_level;

void bmc_log(log_level_t level, const char* fmt, ...);
void bmc_hexdump(const uint8_t* data, size_t len);

// Output format
typedef enum {
    OUTPUT_FORMAT_NORMAL,
    OUTPUT_FORMAT_JSON,
    OUTPUT_FORMAT_TABLE
} output_format_t;

extern output_format_t g_output_format;

// Table functions
void table_init(int num_cols, const char* headers[]);
void table_print_header(void);
void table_print_row(const char* cols[]);
void table_print_footer(void);

// Simple formatting
void print_kv(const char* key, const char* value);
void print_section_header(const char* title);

#endif
