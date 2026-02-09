#include "bmctool/common.h"
#include <stdio.h>
#include <string.h>

#define MAX_COLS 10
#define MAX_COL_WIDTH 50

typedef struct {
    int num_cols;
    char headers[MAX_COLS][MAX_COL_WIDTH];
    int col_widths[MAX_COLS];
} table_t;

static table_t g_table = {0};

void table_init(int num_cols, const char* headers[]) {
    g_table.num_cols = num_cols;
    
    for (int i = 0; i < num_cols && i < MAX_COLS; i++) {
        strncpy(g_table.headers[i], headers[i], MAX_COL_WIDTH - 1);
        g_table.headers[i][MAX_COL_WIDTH - 1] = '\0';
        
        // 計算欄位寬度（至少要能放下 header）
        g_table.col_widths[i] = strlen(headers[i]);
    }
}

void table_print_header(void) {
    // 上框線
    printf("┌");
    for (int i = 0; i < g_table.num_cols; i++) {
        for (int j = 0; j < g_table.col_widths[i] + 2; j++) {
            printf("─");
        }
        if (i < g_table.num_cols - 1) {
            printf("┬");
        }
    }
    printf("┐\n");
    
    // Header
    printf("│");
    for (int i = 0; i < g_table.num_cols; i++) {
        printf(" %-*s │", g_table.col_widths[i], g_table.headers[i]);
    }
    printf("\n");
    
    // 分隔線
    printf("├");
    for (int i = 0; i < g_table.num_cols; i++) {
        for (int j = 0; j < g_table.col_widths[i] + 2; j++) {
            printf("─");
        }
        if (i < g_table.num_cols - 1) {
            printf("┼");
        }
    }
    printf("┤\n");
}

void table_print_row(const char* cols[]) {
    printf("│");
    for (int i = 0; i < g_table.num_cols; i++) {
        printf(" %-*s │", g_table.col_widths[i], cols[i] ? cols[i] : "");
    }
    printf("\n");
}

void table_print_footer(void) {
    printf("└");
    for (int i = 0; i < g_table.num_cols; i++) {
        for (int j = 0; j < g_table.col_widths[i] + 2; j++) {
            printf("─");
        }
        if (i < g_table.num_cols - 1) {
            printf("┴");
        }
    }
    printf("┘\n");
}

// 簡單的 key-value 顯示
void print_kv(const char* key, const char* value) {
    printf("%-20s : %s\n", key, value);
}

void print_section_header(const char* title) {
    int len = strlen(title);
    printf("\n");
    printf("╔");
    for (int i = 0; i < len + 2; i++) printf("═");
    printf("╗\n");
    printf("║ %s ║\n", title);
    printf("╚");
    for (int i = 0; i < len + 2; i++) printf("═");
    printf("╝\n");
}
uchen
