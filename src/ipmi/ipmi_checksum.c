#include "bmctool/ipmi.h"

/**
 * 計算 IPMI checksum (Two's Complement)
 * 
 * 算法：
 * 1. 加總所有 bytes
 * 2. 取最低 8 bits
 * 3. Two's complement: (0x100 - sum) & 0xFF
 * 
 * 這樣設計是為了讓所有 bytes（包含 checksum）相加後
 * 最低 8 bits 為 0
 */
uint8_t ipmi_checksum(const uint8_t* data, size_t len) {
    if (!data || len == 0) {
        return 0;
    }
    
    uint32_t sum = 0;
    for (size_t i = 0; i < len; i++) {
        sum += data[i];
    }
    
    // Two's complement
    uint8_t checksum = (0x100 - (sum & 0xFF)) & 0xFF;
    
    return checksum;
}

/**
 * 驗證 checksum 是否正確
 * 
 * 正確的 checksum：所有 bytes + checksum 的和
 * 最低 8 bits 應該等於 0
 */
int ipmi_verify_checksum(const uint8_t* data, size_t len, uint8_t checksum) {
    if (!data || len == 0) {
        return -1;
    }
    
    uint32_t sum = 0;
    for (size_t i = 0; i < len; i++) {
        sum += data[i];
    }
    sum += checksum;
    
    // 檢查最低 8 bits 是否為 0
    return ((sum & 0xFF) == 0) ? 0 : -1;
}
