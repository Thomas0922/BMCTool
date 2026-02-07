# BMC Protocol Toolkit

這是一個從零開始實作 IPMI 和 Redfish 協議的學習專案。

## 為什麼做這個？

主要是想理解 BMC 到底是怎麼運作的，但光用 ipmitool 是不夠的。這個專案透過實際寫出協議解析和封包處理的過程，來搞懂 BMC 通訊的每個細節。

這不是要做一個新的 ipmitool，也沒打算拿來當生產環境工具。主要目的是學習，順便做出一些好用的除錯功能。

## 功能

目前實作的部分：

**IPMI**
- 基本的 RMCP/RMCP+ 封包處理
- Session 管理
- 幾個常用命令（Get Device ID、Sensor Reading、SEL 相關）
- 詳細的封包分析工具

**Redfish**
- HTTP client 基礎
- JSON 解析
- 基本的 REST endpoint（Systems、Thermal、Power）

**除錯工具**
- 可以印出完整封包內容，包含每個欄位的說明
- 協議驗證（檢查 BMC 回傳的東西是否符合規範）
- 簡單的效能測試

## 編譯

需要的套件：
```bash
sudo apt install build-essential libcurl4-openssl-dev libjson-c-dev
```

編譯：
```bash
make
```

測試：
```bash
make test
```

## 使用

連 BMC 取得資訊：
```bash
# IPMI
./bmctool ipmi -H 192.168.1.100 -U admin -P password get-device-id

# Redfish  
./bmctool redfish -H https://192.168.1.100 -U admin -P password system info
```

看詳細的封包內容：
```bash
./bmctool --verbose ipmi -H 192.168.1.100 get-device-id
```

## 專案結構
```
src/
  ipmi/      - IPMI 協議實作
  redfish/   - Redfish 客戶端
  common/    - 共用的工具函式
  cli/       - 命令列介面

include/     - 標頭檔
tests/       - 測試程式
docs/        - 文件
```

## 跟現有工具的差異

ipmitool 已經很成熟了，功能也完整。這個專案主要的差異是：

1. 程式碼比較簡潔，大約 5000 行，容易看懂
2. 有詳細的封包分析功能，對學習和除錯很有幫助
3. 加入了協議驗證工具
4. 當作學習教材設計的，不是生產工具


## 測試

單元測試：
```bash
make test
```

檢查記憶體洩漏：
```bash
make memcheck
```

除錯模式編譯：
```bash
make DEBUG=1
```


## License

MIT

---

這是個學習專案。如果你要管理實際的 BMC，請用 ipmitool 或其他成熟的工具。
