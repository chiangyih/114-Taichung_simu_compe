# 電腦修護第二站 — PC 端功能規格書（Functional Specification — PC Side）

## 版本資訊
- 文件版本：v1.1
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
| PC-F7 | 功能切換 | 模式切換 | 切換 MCU 顯示模式：鎖定螢幕/顯示設置/電腦資訊 |
| PC-F8 | 系統資訊 | CPU / RAM | 顯示 CPU%, RAM(GB) 並傳送至 MCU |
| PC-F9 | EEPROM | Read | 讀取 EEPROM LED 腳位值 |
| PC-F10 | EEPROM | Write | 寫入 1–254 值並驗證 |
| PC-F11 | EEPROM | 再次讀取 | 重開程式後必須讀到 EEPROM 真實值 |
| PC-F12 | 流程控制 | 自動恢復 | Close 後再 Open 必須自動恢復所有 UI 與 MCU 狀態 |

---

# 3. UI 規格

## 3.1 主視窗內容

必須包含以下元件：
- **標題列**（含崗位號碼）
- **Current Time 區塊**（每秒更新）
- **COM Port 下拉式選單**（自動更新可用埠）
- **Open / Close 按鈕**
- **Device Status 指示**（Online/Offline）
- **CPU 使用率** 顯示（百分比）
- **RAM 使用量** 顯示（已用 / 總量，GB）
- **LED 腳位 Read / Write 欄位**
- **模式切換下拉選單**（1=鎖定螢幕、2=顯示設置、3=電腦資訊）

---

# 4. 自動連線藍芽序列埠

程式啟動時自動執行以下步驟：
1. 掃描所有可用 COM 埠
2. 檢查每個 COM 埠的裝置名稱
3. 排除藍芽序列埠號碼大的
4. 自動連線藍芽序列埠號碼小的

---

# 5. Open / Close 流程規格

## 5.1 Open 按鈕行為

1. 打開 Serial Port（9600 baud）
2. 傳送初始化封包：`HELLO`
3. MCU 回應後：
   - Device Status → **Online**（指示燈顯示）
   - MCU TFT 右上角顯示連線符號
4. 啟動 Timer（1秒間隔）：
   - 每秒傳送 Time Sync
   - 更新 CPU / RAM 資訊

## 5.2 Close 按鈕行為

- 關閉 Serial Port
- 所有控制項設定為 Disable
- Device Status → **Offline**（指示燈顯示）
- 再按 Open 時必須：
  - 自動恢復全部 UI 狀態
  - 自動進行時間同步
  - 無須使用者做任何手動操作  

---

# 6. 模式切換功能規格

下拉選單選項：1=鎖定螢幕、2=顯示設置、3=電腦資訊

### 6.1 模式 1 - 鎖定螢幕

- 傳送指令至 MCU：`M1`
- MCU 切換回開機初始狀態

### 6.2 模式 2 - 顯示設置

- 傳送指令至 MCU：`M2`
- MCU 切換至顯示設置模式
- 同時發送系統時間同步資料（格式：`Tyyyy/MM/dd HH:mm:ss`）

### 6.3 模式 3 - 電腦資訊

- 傳送指令至 MCU：`M3`（或其他設定）
- MCU 切換至電腦資訊模式
- 發送 CPU 使用率與 RAM 使用量（格式：`Scpu=XX;ram=X.X/XX.X`）

---

# 7. LED 腳位 Read / Write 流程規格

### 7.1 Read 按鈕行為

- **第一次按下 Read**：讀出預設值 **255**
- **後續按下 Read**：讀出 MCU 儲存的 EEPROM 真實值

操作流程：
1. PC 發送：`R`
2. MCU 回傳：`VAL=xxx`
3. PC 顯示於 LED 腳位欄位

### 7.2 Write 按鈕行為

寫入值限定在 **1～254** 之間。

操作流程：
1. PC 驗證輸入值合法性（1–254 範圍內）
2. PC 發送：`Lxxx`（例如 `L230`）
3. MCU 寫入 EEPROM
4. MCU 回應：`WRITE OK`
5. PC 更新 LED 腳位欄位顯示

---

# 8. 通訊協定規格

## 8.1 協定格式

| 功能 | PC → MCU | MCU → PC |
|------|-----------|-----------|
| 初始化 | `HELLO` | `OK` |
| 時間同步 | `Tyyyy/MM/dd HH:mm:ss` | `OK` |
| CPU/RAM 更新 | `Scpu=XX;ram=X.X/XX.X` | `OK` |
| 模式 1 切換 | `M1` | `OK` |
| 模式 2 切換 | `M2` | `OK` |
| 模式 3 切換 | `M3` | `OK` |
| 讀取 EEPROM | `R` | `VAL=xxx` |
| 寫入 EEPROM | `Lxxx` | `WRITE OK` |

---

# 9. CPU / RAM 顯示規格

### 9.1 CPU 使用率

可使用以下方法獲取：
- `PerformanceCounter`
- WMI：`Win32_PerfFormattedData_PerfOS_Processor`

顯示格式：百分比（例如：58%）

### 9.2 RAM 使用量

可使用以下方法獲取：
- WMI：`Win32_OperatingSystem`
- VB.NET：`My.Computer.Info.TotalPhysicalMemory` 和 `My.Computer.Info.AvailablePhysicalMemory`

顯示格式：已用 / 總量（GB）（例如：8.3 / 15.9）

並實時傳送至 MCU 更新 TFT 顯示。

---

# 10. 錯誤處理

需妥善處理以下異常情況：

- **COM Port 被占用**：提示使用者選擇其他埠或關閉佔用程式
- **MCU 無回應 timeout**：設定合理的等待時間（建議 2～3 秒），超時後提示連線異常
- **非法輸入**：驗證使用者輸入，拒絕非數字或超出範圍（1～254）的值
- **SerialPort 斷線**：監測連線狀態，斷線時自動更新 Device Status 為 Offline

---

# 11. 測試案例（建議）

| 測試項目 | 預期結果 |
|---------|----------|
| 程式啟動時自動連線 | ✔ 自動掃描並連線藍芽埠 |
| Open 按鈕動作 | ✔ Device Status 變為 Online，MCU 顯示連線符號 |
| 每秒時間同步 | ✔ MCU TFT 時間與 PC 同步 |
| CPU / RAM 實時更新 | ✔ TFT 正確顯示數值 |
| 模式切換 | ✔ MCU 畫面根據選項正確切換 |
| Close 後再 Open | ✔ 自動恢復 UI 狀態與 MCU 狀態，無須手動操作 |
| Read EEPROM（首次） | ✔ 顯示預設值 255 |
| Write EEPROM | ✔ 寫入指定值，MCU 回應 WRITE OK |
| Read EEPROM（再次） | ✔ 顯示寫入的真實值 |
| SerialPort 斷線 | ✔ Device Status 變為 Offline，提示使用者 |

---

# 12. 附註

- 所有通訊協定採用 **ASCII 格式**，以 **LF（\n）** 或 **CRLF（\r\n）** 作為行尾
- SerialPort 波特率：**9600 baud**
- 建議每個命令設定 **2～3 秒超時機制**，避免程式無限等待
- UI 應提供清晰的狀態指示，讓使用者了解當前連線狀態
- 記錄通訊過程（建議開發階段啟用 Debug Log）便於故障排查

---

**文件完**
