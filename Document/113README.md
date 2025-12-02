# 114工科賽參考 — 第二站 USB 介面卡控制（依據 114 題目規格重整）
Last Update: 2025-12-02
## 專案概述

114 學年度工業類科學生技藝競賽  
電腦修護職類 第二站 — USB 介面卡製作與控制（PC + MCU 整合）

本文件已依據 `Document/114_PCRepair_Station2_Analysis.md` 與 `Document/MCU_FunctionSpec.md` 內容，將原 113 說明更新為 114 第二站之功能與測試重點，提供練習與教學參考。

### 硬體配置（依 114 題目）
- **MCU**: ATmega328P（Arduino Nano/UNO 類）
- **通訊**: USB-Serial（CH340/FT232），藍牙 HC-05/06（需命名 `BT-XX`）
- **顯示**: TFT LCD（SPI）
- **RGB**: WS2812 × 8 LEDs
- **按鍵**: S1、S2、S3、S4
- **EEPROM**: 內建（模式/LED 腳位）
- **崗位**: XX（依實際分配）

### 核心功能（114）
- **按鍵顯示**：S1〜S4 顯示 `btn-x`，0.5 秒自動還原。
- **雙鍵 RGB**：S1+S2 >0.5 秒，隨機 RGB 顯示，LED 同步變色；若 EEPROM 已設定 LED 腳位值，依 bit 控制亮滅。
- **藍牙名稱**：設定為 `BT-XX`，密碼可自訂（考前多為前置作業）。
- **顯示模式**：C1（日期+時間）、C2（時間）、C3（CPU/RAM 資訊）、5（Locked 狀態）。
- **PC 連線**：Open/Close 流程、連線符號顯示、COM Port 自動更新。
- **時間同步**：PC 定時送 `Tyyyy/MM/dd HH:mm:ss`，MCU 更新 TFT。
- **CPU/RAM 顯示**：PC 計算後以封包送 MCU，MCU 用數值/進度條顯示。
- **EEPROM**：模式地址 0x00；LED 腳位地址 0x01（1–254，255 預設）。斷電不丟失。
- **LED 腳位 Read/Write**：Read 初始回 255；Write 僅允許 1–254，成功後模式顯示 `5` 並閃爍 `Locked`。

---

## 快速開始

### 1. 環境需求
- PlatformIO（VSCode 擴充套件）
- Arduino Nano/UNO 開發板
- USB-Serial 介面（CH340/FT232 等）
- HC-05/06 藍牙模組（選配，名稱需 `BT-XX`）

### 2. 編譯與上傳
```bash
# 編譯程式碼
pio run

# 上傳到 Arduino
pio run --target upload

# 開啟序列埠監視器 (9600 baud)
pio device monitor
```

### 3. 測試腳本
```bash
# WS2812 顏色測試
python test_ws2812_colors.py

### 3. 基本測試流程（114）
1) 開機：確認 TFT 初始畫面（時間、崗位、模式）。
2) 單鍵：S1〜S4 顯示 `btn-x`，0.5 秒還原。
3) 雙鍵：S1+S2 >0.5 秒，RGB 隨機、LED 同色全亮（EEPROM=255）。
4) Open：PC 端連線，TFT 顯示連線符號，開始時間同步。
5) 模式：切換 C1/C2/C3；C3 顯示 CPU/RAM（PC 計算送 MCU）。
6) LED 腳位：Write 合法值（1–254），TFT 顯示 `5` + `Locked`；再雙鍵依 bit 控制亮滅。
7) 斷電：重上電後，模式/LED 值保持；PC 重新 Open、Read 應回 EEPROM 實值。
### 核心文件
- **[FirmwareSpec.md](FirmwareSpec.md)** - 完整韌體需求規格
- **[Arduino_WS2812_Integration_Guide.md](Arduino_WS2812_Integration_Guide.md)** - WS2812 整合指南

### 快速參考
- **[腳位配置參考卡.md](腳位配置參考卡.md)** - 硬體接線速查表
- **[114_PCRepair_Station2_Analysis.md](114_PCRepair_Station2_Analysis.md)** - 第二站題目功能與規格分析
- **[MCU_FunctionSpec.md](MCU_FunctionSpec.md)** - MCU 端功能規格書（封包/EEPROM/模式）
- **[PC_FunctionSpec.md](PC_FunctionSpec.md)** - PC 端功能規格書（COM/時間/CPU/RAM/流程）
- **[Connect_to_BLE功能快速參考.md](Connect_to_BLE功能快速參考.md)** - BLE 連線功能說明

### 硬體設定
- **[測試指南.md](測試指南.md)** - 系統測試程序
---

- **ST7735/TFT 接線說明**（待補） - 顯示器接線指南
- **測試指南**（待補） - 系統測試程序
### 支援的命令

| 命令 | 格式 | 功能 | 回應 | 容錯 |
## 通訊協定（114）

### 封包方向：PC → MCU
- `Tyyyy/MM/dd HH:mm:ss`：時間同步。
- `Scpu=58;ram=8.3/15.9`：CPU/RAM 資訊同步（數值、總量）。
- `R`：請求回傳 EEPROM LED 值。
- `L230`：寫入 LED 腳位值 230（合法範圍 1–254）。
- `M1/M2`：切換顯示模式（日期時間 / 時間）。

### 封包方向：MCU → PC（例）
- `VAL=255`：EEPROM 未設定。
- `VAL=<n>`：回傳實際 EEPROM 值。
- `WRITE OK` / `ERR`：寫入成功或格式錯誤。
- `OK`：一般確認封包。

### EEPROM 規劃（114）
- `0x00`：顯示模式（0=完整日期時間，1=僅時間）。
- `0x01`：LED 腳位值（1–254，255 表預設）。

### Open/Close 流程要點
- Open 後自動同步時間、更新狀態，TFT 連線符號變化。
- Close 後 UI 灰階不可操作；再次 Open 應自動恢復所有功能。
- COM Port 列表需週期性更新且差異更新。
### 已完成項目
✅ 基本功能實作 (F1-F8)  
✅ WS2812 顏色閾值修正 (符合整合指南)  
✅ 藍牙逾時偵測機制  
✅ TFT 顯示自動更新  
### 舊版（113）內容調整
- 移除 Countdown、LOAD 顏色區間等不屬於 114 題目之項目。
- 藍牙命令改為名稱設定為主，通訊以 USB-Serial 為核心。
- EEPROM 結構改為模式 + LED 腳位值，去除簽名位概念。

### 目前狀態
✅ 依 114 題目完成規格重整（文件）  
⚠️ 程式碼對應與測試腳本仍需依此規格更新（待作）
### TFT 顯示異常
1. 檢查接線 (參考 ST7735接線說明.md)
2. 確認使用 Software SPI 模式 (MOSI=A4, SCK=A5)
3. 嘗試不同初始化參數 (INITR_BLACKTAB/GREENTAB/REDTAB)

### 藍牙連線問題
1. 確認 HC-05 鮑率為 9600 bps
2. 檢查 Serial Monitor 是否顯示 "BLE RX:" 訊息
3. 確認 platformio.ini 設定 `monitor_speed = 9600`

### WS2812 不亮
1. 檢查資料線接在 D5
2. 確認電源供應足夠 (5V, ≥2A)
3. 檢查 CPU Loading 數值範圍 (0-100)

---

**最後更新**: 2025年11月20日 20:06

**最後更新**: 2025年12月2日
- **CONNECT / DISCONNECT / PING**：精確匹配
