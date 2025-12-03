# 藍牙 COM Port 過濾 - 快速測試指南

## ?? 目標
確認程式正確過濾藍牙 Incoming Port，只顯示 Outgoing Port。

## ?? 測試前準備

### 1. 確認藍牙已配對
- 開啟 Windows 設定 > 裝置 > 藍牙
- 確認 HC-05 或其他藍牙設備已配對

### 2. 查看目前的 COM Port
按 `Win + X`，選擇「裝置管理員」：
1. 展開「連接埠 (COM 和 LPT)」
2. 記錄所有藍牙相關的 COM Port：
   ```
   例如：
   - Standard Serial over Bluetooth link (COM5)  ← Outgoing
   - Bluetooth Serial Port (COM4) (incoming)    ← Incoming
   ```

## ?? 執行測試

### 步驟 1：啟動程式（Debug 模式）
1. 在 Visual Studio 中按 `F5` 執行
2. 開啟輸出視窗：`Ctrl + W, O` 或 `View` > `Output`
3. 確保下拉選單選擇「偵錯」

### 步驟 2：檢查 Debug 輸出
程式啟動時應該會看到類似以下訊息：

```
=== 開始載入 COM Ports ===
找到 Standard Serial Port: Standard Serial over Bluetooth link (COM5)
確認找到有效的藍牙 Outgoing Port: COM5
已添加藍牙 Outgoing Port: COM5
藍牙 Incoming Port 已加入黑名單: COM4 (來源: Bluetooth Serial Port (COM4) (incoming))
系統中所有 COM Ports: COM3, COM4, COM5, COM7
黑名單中的 Ports: COM4, COM5
已過濾 Port: COM4
已過濾 Port: COM5
已添加 Port: COM3
已添加 Port: COM7
=== 載入完成，共 3 個 Ports ===
```

### 步驟 3：檢查 GUI 下拉選單
COM Port 下拉選單應該顯示：

? **正確的結果**：
```
┌─────────────────────────┐
│ COM5 (藍牙 Outgoing)    │ ← 只有這一個藍牙 Port
│ COM3                    │
│ COM7                    │
└─────────────────────────┘
```

? **錯誤的結果**（如果看到這個，表示過濾失敗）：
```
┌─────────────────────────┐
│ COM5 (藍牙 Outgoing)    │
│ COM3                    │
│ COM4                    │ ← 這是 Incoming，不應該出現
│ COM5                    │ ← 重複！
│ COM7                    │
└─────────────────────────┘
```

## ?? 故障排除

### 問題 1：看不到 Debug 輸出
**解決方法**：
1. 確認是用 Debug 模式執行（不是 Release）
2. 在輸出視窗下拉選單選擇「偵錯」
3. 檢查 `Debug.WriteLine()` 的命名空間是否正確

### 問題 2：仍然看到重複的 COM Port
**診斷步驟**：

#### A. 檢查黑名單
在 Debug 輸出中找到這一行：
```
黑名單中的 Ports: COM4, COM5
```

如果黑名單中**沒有**包含重複的 Port，執行以下 PowerShell 命令：

```powershell
Get-WmiObject Win32_PnPEntity | 
    Where-Object {$_.Caption -match "COM"} | 
    Select-Object Caption, Name, Description | 
    Where-Object {$_.Caption -match "COM12" -or $_.Name -match "COM12"} | 
    Format-List
```

將輸出貼到 GitHub Issue 或給我看。

#### B. 檢查是否真的有兩個不同的藍牙設備
```powershell
Get-WmiObject Win32_PnPEntity | 
    Where-Object {$_.Caption -match "Bluetooth.*COM"} | 
    Select-Object Caption, DeviceID | 
    Format-Table -AutoSize
```

### 問題 3：找不到藍牙 Outgoing Port
**可能原因**：
1. 藍牙設備未配對
2. 驅動程式命名不標準

**解決方法**：
執行診斷命令：
```powershell
Get-WmiObject Win32_PnPEntity | 
    Where-Object {$_.Caption -match "Bluetooth"} | 
    Select-Object Caption, Name, Description | 
    Format-Table -Wrap
```

查看輸出中是否有包含 "outgoing"、"standard serial" 或 "標準序列" 的裝置。

## ?? 測試檢查清單

### 基本測試
- [ ] 程式啟動無錯誤
- [ ] 輸出視窗顯示 Debug 訊息
- [ ] 下拉選單中只有一個藍牙 Port（標註為「藍牙 Outgoing」）
- [ ] 沒有重複的 COM Port
- [ ] 沒有藍牙 Incoming Port

### 連線測試
- [ ] 選擇藍牙 Outgoing Port
- [ ] 點擊 Open，成功連線
- [ ] Device Status 顯示 "Device Online"（綠色）
- [ ] 可以與 MCU 通訊

### 邊界測試
- [ ] 拔掉藍牙設備，重新啟動程式
- [ ] 插回藍牙設備，重新啟動程式
- [ ] COM Port 列表正確更新

## ?? 回報問題時請提供

如果問題仍然存在，請提供以下資訊：

### 1. Debug 輸出（完整）
從輸出視窗複製所有 "=== 開始載入 COM Ports ===" 開始的訊息。

### 2. PowerShell 查詢結果
```powershell
# 執行這兩個命令並貼上結果
Get-WmiObject Win32_PnPEntity | 
    Where-Object {$_.Caption -match "Bluetooth.*COM"} | 
    Select-Object Caption, Name, Description, DeviceID | 
    Format-List

Get-WmiObject Win32_SerialPort | 
    Select-Object DeviceID, Caption, Description | 
    Format-Table -AutoSize
```

### 3. 裝置管理員截圖
展開「連接埠 (COM 和 LPT)」的截圖。

### 4. GUI 截圖
COM Port 下拉選單的截圖。

## ?? 進階診斷

### 使用 WMI Explorer
1. 下載並安裝 [WMI Explorer](https://github.com/vinaypamnani/wmie2/releases)
2. 連接到本機：`root\cimv2`
3. 查詢：`SELECT * FROM Win32_PnPEntity WHERE Caption LIKE '%Bluetooth%COM%'`
4. 查看每個結果的 Caption、Name、Description 屬性

### 使用 Serial Port Monitor
1. 下載 [Free Serial Port Monitor](https://freeserialportmonitor.com/)
2. 查看所有 COM Port 的詳細資訊
3. 確認哪些是 Incoming/Outgoing

---

## ? 測試成功的標準

1. **Debug 輸出清晰**：
   ```
   === 開始載入 COM Ports ===
   ...
   藍牙 Incoming Port 已加入黑名單: COM4
   已過濾 Port: COM4
   ...
   === 載入完成，共 X 個 Ports ===
   ```

2. **GUI 正確顯示**：
   - 只有一個藍牙 Port（Outgoing）
   - 沒有重複的 Port
   - 其他 COM Port 正常顯示

3. **連線成功**：
   - 可以選擇藍牙 Outgoing Port 並連線
   - Device Status 變為 Online
   - 可以與 MCU 正常通訊

---

**測試完成後**：
- ? 如果全部通過，恭喜！程式運作正常。
- ? 如果有問題，請依照「回報問題時請提供」的步驟收集資訊。

**版本**：v2.0  
**日期**：2025-01-19
