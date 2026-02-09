# BMC Protocol Toolkit

從零開始用 C 語言實作 IPMI 和 Redfish 協議

## 專案起源

想真正理解 BMC 怎麼運作，但發現光用 ipmitool 不夠。所以決定自己寫一個，過程中把 IPMI 封包格式、checksum 計算、UDP socket 這些都自己做一遍。雖然花了不少時間除錯，但確實把協議的每個細節都搞懂了。

## 功能

### IPMI 部分
- 完整的 RMCP 封包處理（建構和解析）
- Two's complement checksum（這個花了點時間才搞對）
- UDP socket 傳輸層，有處理 timeout
- 基本的 session 管理
- 實作了兩個命令：Get Device ID 和 Get Chassis Status

### Redfish 部分
- HTTP/HTTPS 客戶端（用 libcurl）
- JSON 解析（用 json-c）
- Basic 認證
- 實作了 System Info 和 Thermal 兩個端點

### CLI 工具
- 可以用同一個指令操作 IPMI 和 Redfish
- 輸出有美化，用了 Unicode 畫框
- 有表格模式可以選
- Verbose 模式會顯示完整封包分析
- 用 Valgrind 驗證過，沒有記憶體洩漏

## 編譯
```bash
make
```

需要的套件：
```bash
sudo apt install libcurl4-openssl-dev libjson-c-dev
```

## 使用方式

### IPMI
```bash
# 查詢 BMC 裝置資訊
./bmctool -H 192.168.1.100 ipmi get-device-id

# 查詢機箱電源狀態
./bmctool -H 192.168.1.100 ipmi chassis-status

# 顯示詳細封包內容
./bmctool -H 192.168.1.100 -v ipmi get-device-id

# 表格輸出
./bmctool -H 192.168.1.100 -f table ipmi get-device-id
```

### Redfish
```bash
# 查詢系統資訊
./bmctool -H https://192.168.1.100 -U admin -P password redfish system 1

# 查詢溫度資訊
./bmctool -H https://192.168.1.100 -U admin -P password redfish thermal 1
```

## 測試環境

沒有實體 BMC 的話可以用 mock server 測試：
```bash
# Terminal 1: 啟動 IPMI responder
python3 tests/ipmi_responder.py

# Terminal 2: 啟動 Redfish mock server
python3 tests/redfish_mock_server.py

# Terminal 3: 測試
./bmctool -H 127.0.0.1 -p 9623 ipmi get-device-id
./bmctool -H http://127.0.0.1:8000 -U admin -P password redfish system 1
```

整合測試：
```bash
./tests/integration_test.sh
```

記憶體檢查：
```bash
valgrind --leak-check=full ./bmctool -H 127.0.0.1 -p 9623 ipmi get-device-id
```

## 專案結構
```
src/
  common/          日誌、錯誤處理、輸出格式化
  ipmi/           IPMI 協議實作
    ├── checksum   Two's complement checksum
    ├── packet     封包建構和解析
    ├── transport  UDP socket 傳輸層
    └── commands   IPMI 命令
  redfish/        Redfish 協議實作
    ├── client     HTTP 客戶端 (libcurl)
    ├── json       JSON 解析 (json-c)
    └── api        Redfish API
  cli/            命令列介面
```

## 實作重點

### IPMI 封包格式
```
UDP 封包 (21 bytes)
├── RMCP Header (4)      版本、序號、類別
├── Session Header (9)   認證類型、Session ID
├── Message Length (1)   
└── IPMI Message (7)
    ├── Header (6)       目標位址、NetFn、Checksum
    └── Checksum (1)     Data checksum
```

### Checksum 計算

這個花了點時間才搞懂，IPMI 用的是 two's complement：
```c
uint8_t ipmi_checksum(const uint8_t* data, size_t len) {
    uint8_t sum = 0;
    for (size_t i = 0; i < len; i++) {
        sum += data[i];
    }
    return (0x100 - sum) & 0xFF;
}
```

重點是所有相關 bytes 加起來（包括 checksum）低 8 位元要是 0。

### Struct Packing

封包處理需要確保 struct 不會被 padding：
```c
typedef struct __attribute__((packed)) {
    uint8_t version;
    uint8_t reserved;
    uint8_t sequence;
    uint8_t class;
} rmcp_header_t;
```

### 網路位元組序

需要處理 endianness：
```c
session_id = ntohl(session_id);  // network to host (32-bit)
```

## 踩過的坑

1. **Checksum 一直算錯**  
   一開始沒搞懂 two's complement 的概念，後來對照規格書和用 Wireshark 抓封包才找到問題

2. **Struct packing 問題**  
   編譯器會自動對齊 struct，導致封包格式錯誤。要加 `__attribute__((packed))`

3. **Endianness 轉換遺漏**  
   網路傳輸是 big-endian，要記得用 htonl/ntohl 轉換

4. **VirtualBMC 只支援 RMCP+**  
   原本想用 VirtualBMC 測試，結果發現它要求認證。最後改用自己寫的 Python responder

## 學到什麼

**技術面：**
- 如何讀懂並實作二進位網路協議
- C 語言的記憶體管理和指標操作
- Socket 程式設計和網路除錯
- 從規格書到實作的完整流程

**除錯技巧：**
- 用 Wireshark 抓包對照規格書
- Hex dump 逐 byte 比對
- Valgrind 檢查記憶體問題
- GDB 追蹤程式執行

**心得：**
協議細節很多，需要很有耐心。但親手實作過一次之後，對 BMC 的運作方式就很清楚了。比只會用 ipmitool 的理解深入很多。

## 與 ipmitool 的差異

| 項目 | ipmitool | 這個專案 |
|------|----------|---------|
| 程式碼量 | 100k+ 行 | ~4k 行 |
| 用途 | 生產環境使用 | 學習/研究用 |
| 封包分析 | 基本功能 | 詳細分析（verbose 模式）|
| 程式碼可讀性 | 較複雜 | 清楚的模組化設計 |

這不是要取代 ipmitool，而是透過實作來學習。如果要在生產環境用，還是建議用 ipmitool 或 freeipmi 這類成熟工具。

## 統計資料

- 程式碼：約 4000 行 C
- 協議：IPMI 2.0 + Redfish
- 命令：4 個（2 IPMI + 2 Redfish）
- 記憶體洩漏：0 bytes
- 測試覆蓋：核心功能都有測試

## 未來可以加的功能

- SEL (System Event Log) 操作
- Sensor reading 命令
- FRU information 讀取
- RMCP+ 認證支援
- 更多 Redfish 端點

如果有時間的話可能會加，但目前功能已經夠展示對協議的理解了。

## License

MIT

---
