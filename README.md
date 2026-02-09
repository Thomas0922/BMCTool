# BMC Protocol Toolkit

從零實作 IPMI 2.0 協議解析與測試工具。

## 專案動機

想理解 BMC 怎麼運作，但光用 ipmitool 覺得還不夠。這個專案透過實際撰寫 C 程式碼來實作 IPMI 協議，過程中深入理解了封包格式、checksum 計算、網路傳輸等細節。

## 實作內容

### IPMI 協議處理
- RMCP 封包解析與建構
- Two's complement checksum 驗證
- Session 管理（基礎版）
- 支援的命令：Get Device ID

### 網路傳輸層
- UDP socket 實作
- Timeout 處理（5 秒）
- 錯誤處理與重試機制
- 支援自訂 port

### CLI 工具
- 命令列參數解析（getopt）
- 一般模式與 verbose 模式
- 清晰的錯誤訊息

## 編譯
```bash
make
```

## 測試

### 啟動測試環境
```bash
# Terminal 1: 啟動 IPMI responder
python3 ~/ipmi_responder.py
```

### 執行測試
```bash
# Terminal 2: 測試工具
./bmctool -H 127.0.0.1 -p 9623 get-device-id

# Verbose 模式（顯示完整封包）
./bmctool -H 127.0.0.1 -p 9623 -v get-device-id
```

**輸出範例：**
```
Device ID          : 0x20
Device Revision    : 0
Firmware Version   : 1.0
IPMI Version       : 2.0
Manufacturer ID    : 0x0000b4 (Advantech)
Product ID         : 0x0000
```

## 程式碼品質

### 記憶體檢查
```bash
valgrind --leak-check=full ./bmctool -H 127.0.0.1 -p 9623 get-device-id
```

**結果：**
- 記憶體分配：2 allocs, 2 frees, 1364 bytes
- 洩漏：0 bytes
- 錯誤：0 errors

### 測試覆蓋
```bash
make test
```

包含：
- Common 模組測試（日誌、checksum）
- IPMI 封包解析測試
- 網路傳輸測試

## 專案結構
```
src/
  common/    - 日誌、錯誤處理、hex dump
  ipmi/      - 封包處理、命令實作、網路傳輸
  cli/       - 命令列介面
include/     - 標頭檔
tests/       - 測試程式
```

## 技術重點

### 協議實作
- 嚴格按照 IPMI 2.0 規格書（Intel）
- 處理 struct packing 和 byte ordering
- 實作 two's complement checksum

### 系統程式設計
- UDP socket 管理
- Timeout 與非阻塞 I/O
- 完整的資源清理（零洩漏）

### 程式品質
- 模組化設計
- 清晰的錯誤處理
- Valgrind 驗證

## 開發過程

這個專案讓我學到：
1. 讀懂並實作二進位網路協議
2. C 語言的記憶體管理和指標操作
3. Socket 程式設計和網路除錯
4. 從規格書到實作的完整流程

比較有挑戰的地方：
- IPMI checksum 計算（two's complement）
- Struct packing 對齊問題
- 封包格式的細節很多，需要仔細對照規格

## 與現有工具的差異

ipmitool 是成熟的生產工具（100k+ 行），這個專案是學習導向（~2k 行），重點在理解協議本身。

**差異化功能：**
- 詳細的封包分析工具（verbose 模式）
- 簡潔的程式碼架構，容易理解
- 模組化設計，方便擴展

## License

MIT

---

這是個學習專案。生產環境請用 ipmitool 或其他成熟工具。
EOF

echo "✓ README.md 已更新"
