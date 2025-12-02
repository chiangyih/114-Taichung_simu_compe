
# 電腦修護第二站 — PC 端功能規格書（Functional Specification — PC Side）

## 版本資訊
- 文件版本：v1.0  
- 適用競賽：114 學年度工業類科學生技藝競賽 — 電腦修護 第二站  
- 範圍：**PC 端控制程式（C# / VB.NET）**

---

# 1. 系統架構定位
PC 程式負責：
- 與 USB 介面卡進行序列通訊  
- 顯示系統時間、CPU、RAM 使用率  
- 控制 TFT 顯示模式  
- 讀寫 MCU EEPROM（LED 腳位設定值）  
- 維持 Open / Close 連線流程  

---

# 2. PC 端功能需求總表

| 編號 | 功能分類 | 功能名稱 | 描述 |
|------|----------|-----------|--------|
| PC-F1 | UI 顯示 | 標題列格式 | 顯示崗位號碼：`114學年度…崗位號碼：XX` |
| PC-F2 | UI 顯示 | 系統時間 | 每秒更新 yyyy/MM/dd HH:mm:ss |
| PC-F3 | UART | COM Port 自動更新 | 執行期間動態刷新可用連接埠 |
| PC-F4 | UART | Open / Close | 控制 SerialPort，並具備完整流程控制 |
| PC-F5 | UART | Device Status | 顯示 Online / Offline |
| PC-F6 | 時間同步 | Time Sync | 每秒送出系統時間給 MCU |
| PC-F7 | 功能切換 | 顯示格式設定 | 切換 MCU 顯示模式：完整時間/簡易時間 |
| PC-F8 | 系統資訊 | CPU / RAM | 顯示 CPU%, RAM(GB) 並傳送至 MCU |
| PC-F9 | EEPROM | Read | 讀取 EEPROM LED 腳位值 |
| PC-F10 | EEPROM | Write | 寫入 1–254 值並驗證 |
| PC-F11 | EEPROM | 再次讀取 | 重開程式後必須讀到 EEPROM 真實值 |
| PC-F12 | 流程控制 | 自動恢復 | Close 後再 Open 必須自動恢復所有 UI 與 MCU 狀態 |

---

# 3. UI 規格

## 3.1 主視窗內容
必須包含：
- 標題列（含崗位號碼）
- Current Time 區塊
- COM Port 下拉式選單（自動更新）
- Open / Close 按鈕
- Device Status 指示
- CPU 使用率 %
- RAM 使用量（GB 已用 / 總容量）
- LED 腳位 Read / Write 欄位
- 模式切換下拉選單（顯示設置 / 電腦資訊）

---

# 4. Open / Close 流程規格

## 4.1 Open 按鈕行為
1. 打開 Serial Port  
2. 傳送初始化封包：  
   ```
   HELLO
   ```
3. MCU 回應後：  
   -  Device Status → Online  
   -  MCU TFT 右上角顯示連線符號  
4. 啟動 Timer：  
   - 每秒傳送 Time Sync  
   - CPU / RAM 資訊更新  

## 4.2 Close 按鈕行為
- 關閉 Serial Port  
- 所有控制項 Disable  
- Device Status → Offline  
- 再按 Open 必須：  
  - 自動恢復全部 UI  
  - 自動時間同步  
  - 無須使用者做任何手動操作  

---

# 5. 通訊協定（建議格式）

| 功能 | PC → MCU | MCU → PC |
|------|-----------|-----------|
| 時間同步 | `T2024/10/23 13:20:30` | `OK` |
| CPU/RAM 更新 | `Scpu=58;ram=8.3/15.9` | `OK` |
| 要求讀取 EEPROM | `R` | `VAL=255` 或 `VAL=230` |
| 寫入 EEPROM | `L230` | `WRITE OK` |
| 顯示模式切換 | `M1` 或 `M2` | `OK` |

---

# 6. CPU / RAM 顯示規格

### CPU %
可使用：
- PerformanceCounter  
- WMI `Win32_PerfFormattedData_PerfOS_Processor`

### RAM
- WMI `Win32_OperatingSystem`  
- 顯示：已用 / 總量（GB）

並傳到 MCU 更新 TFT 顯示。

---

# 7. EEPROM 操作流程

## 7.1 Read 流程
1. PC 發送：`R`  
2. MCU 回傳：`VAL=xxx`  
3. PC 顯示於 LED 腳位欄位  

## 7.2 Write 流程
1. PC 驗證輸入（1–254）  
2. 發送：`Lxxx`  
3. MCU 寫入 EEPROM  
4. MCU 回應 `WRITE OK`  
5. LED 腳位欄位更新  

---

# 8. 錯誤處理

需處理：
- COM Port 被占用  
- MCU 無回應 timeout  
- 非法輸入（非數字、超出範圍）  
- SerialPort 斷線  

---

# 9. 測試案例（建議）

| 測試項目 | 預期結果 |
|---------|----------|
| Open 後 Device Online | ✔ MCU 接收到初始化封包並顯示連線符號 |
| 每秒時間同步 | ✔ MCU 時間與 PC 同步 |
| CPU / RAM 更新 | ✔ TFT 正確顯示 |
| 關閉再開啟 | ✔ 自動恢復 UI 與 MCU |
| Read EEPROM | ✔ 初次 255，再次為寫入值 |
| Write EEPROM | ✔ LED 腳位寫入成功，TFT 顯示 Locked |

---

# PC 端功能規格書 — 完
