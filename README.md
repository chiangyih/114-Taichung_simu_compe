# 114 學年度 — 第二站 USB 介面卡製作與控制（整合版 README）
Last Update: 2025-12-02

本 README 針對本工作區的練習與教學，整合 `114_PCRepair_Station2_Analysis.md` 與 `MCU_FunctionSpec.md` 的重點，提供快速導覽與實作指南。

## 任務概要
# 114-Taichung_simu_compe
本倉庫包含 MCU 介面卡程式、PC 端 Visual Basic 控制程式與相關文件。以下為最新功能與使用方式總覽。

## MCU（Arduino）最新變更摘要
- LED 啟動條件：改為僅在收到 `LED_OPEN` 指令後才允許點亮 LED；`LED_CLOSE` 指令關閉允許並強制全滅。
- 雙鍵 RGB 顯示：同時按住 `S1` 與 `S2` ≥ 0.5 秒，畫面中央顯示 `R/G/B` 數值，且全部 8 顆 WS2812 LED 同步顯示該顏色；數值每 2 秒隨機變化一次；放開即刻回復原狀（畫面中間顯示崗位號碼，LED 依狀態全滅或關閉）。
- 顯示模式：僅顯示數字模式 `1/2/3/5`，時間僅顯示 `HH:MM:SS`，時間下方有分隔線；開機預設模式保留為 C1/C2（依 EEPROM），但不會自動解鎖。
- EEPROM：`EEPROM_ADDR_MODE` 保存模式、`EEPROM_ADDR_LEDVAL` 保存 LED 位元值；在雙鍵 RGB 模式下不參考位元值，所有 LED 一律同步顯示顏色。
- 相依套件：`Adafruit GFX`、`Adafruit ST7735/ST7789`、`Adafruit NeoPixel`。

## PC 端（Visual Basic）控制程式
- 專案路徑：`VisualBasic/PCControl/PCControl.sln`
- 功能：
	- 串列埠連線（Open/Close）、顯示 `Device Status` 狀態
	- `LED_OPEN` / `LED_CLOSE` 控制 MCU LED 啟動/關閉
	- `Time Sync`：送出 `Tyyyy/MM/dd HH:mm:ss`
	- 模式選擇：`M1` / `M2`（可依需求擴充）
	- 系統資訊：每秒送出 `Scpu=<int>;ram=<used>/<total>` 至 MCU（畫面顯示 CPU/RAM 使用）
	- LED 腳位讀寫：`R` 讀取當前 LED 值、`L<n>` 寫入（1–254）
- 畫面：依 113 學年度工科賽 PC 操作畫面重新設計，包含 Current Time、介面卡狀態、系統資訊、LED 腳位讀寫與 EXIT 按鈕。

## 指令協定（最新）
- `LED_OPEN`：MCU 設定 `connected=true`，允許 LED 顯示（回 `OK`）。
- `LED_CLOSE`：MCU 設定 `connected=false`，LED 全滅（回 `OK`）。
- `Tyyyy/MM/dd HH:mm:ss`：時間同步（回 `OK`）。
- `Scpu=<int>;ram=<used>/<total>`：CPU/RAM 更新，MCU 切至模式 3 顯示（回 `OK`）。
- `M1` / `M2`：模式切換（回 `OK`）。
- `R`：回傳 `VAL=<n>`（LED 位元值）。
- `L<n>`：寫入 LED 位元值（1–254），回 `WRITE OK` 或 `ERR`。

## 建置與執行
- MCU：使用 PlatformIO
	- 編譯與上傳：
		```pwsh
		pio run --target upload
		```
- PC（VB.NET）：使用 Visual Studio 2019/2022
	- 開啟 `VisualBasic/PCControl/PCControl.sln`，編譯並執行。
	- 選擇 Arduino COM 埠後按 `Open`，程式將自動送出 `LED_OPEN`。

## 檔案結構重點
- `include/` 與 `src/`：模組化程式（display、led、buttons、serial_cmd、config、state）
- `src/serial_cmd.cpp`：新增 `LED_OPEN/LED_CLOSE` 解析，移除 `T/OPEN/CLOSE` 對 LED 連線狀態的影響。
- `src/led.cpp`：連線檢查，未連線則 `ledAllOff()`；雙鍵 RGB 模式一律 `ledSetAll()` 全亮同步色；`RGB_UPDATE_MS = 2000`。
- `VisualBasic/PCControl/`：PC 端 WinForms 專案與 UI（依考題版面）。
- 顯示：TFT LCD（SPI）
- LED：WS2812 × 8 顆
- 按鍵：S1、S2、S3、S4
- 通訊：USB-Serial（CH340/FT232），藍牙 HC-05/06（名稱 `BT-XX`）
- EEPROM：地址 0x00（顯示模式）、0x01（LED 腳位值：1–254，255 預設）

## 功能清單（簡要）
- 單鍵顯示：S1〜S4 顯示 `btn-x`，0.5 秒自動還原。
- 雙鍵 RGB：S1+S2 >0.5 秒隨機 RGB；EEPROM=255 全亮，否則依 bit 控制亮滅。
- 顯示模式：C1（日期+時間）、C2（時間）、C3（CPU/RAM）、5（Locked）。
- PC 端：Open/Close 流程、COM 自動更新、時間同步、CPU/RAM 計算並送 MCU。
- EEPROM：模式與 LED 腳位值斷電保留；Read 初始回 255、Write 成功顯示 `5` + `Locked`。

## 通訊協定（示例）
- PC→MCU：`Tyyyy/MM/dd HH:mm:ss`、`Scpu=58;ram=8.3/15.9`、`R`、`L230`、`M1/M2`
- MCU→PC：`VAL=255`、`VAL=<n>`、`WRITE OK`/`ERR`、`OK`

## 快速測試流程
1. 開機 → TFT 初始畫面正確（時間/崗位/模式）。
2. 單鍵 → `btn-x` 顯示 0.5 秒後還原。
3. 雙鍵 → 隨機 RGB、LED 同步，每秒更新；放開自動復原。
4. PC Open → 連線符號變化，開始時間同步與狀態更新。
5. 模式 C3 → PC 推送 CPU/RAM；MCU 顯示為數值/進度條。
6. LED 腳位 Write → 合法值 1–254；TFT 顯示 `5` 與 `Locked` 閃爍。
7. 斷電重上電 → 模式/LED 值保持；PC 再 Read 應回 EEPROM 實值。

## 文件索引
- 題目分析：`114_PCRepair_Station2_Analysis.md`
- MCU 規格：`MCU_FunctionSpec.md`
- PC 規格：`PC_FunctionSpec.md`
- 快速參考：`腳位配置參考卡.md`、`藍牙資料格式快速參考.md`

## 待辦建議
- 以本規格更新 `src/main.cpp` 程式碼（模式、封包、EEPROM）。
- 撰寫 PC 端小工具：COM 選擇、時間同步、CPU/RAM、LED Read/Write。
- 製作測試清單：依評分表逐項驗證（按鍵/雙鍵/連線/模式/EEPROM/斷電）。

---

若需我將上述規格直接落地到 `src/main.cpp` 或建立 PC 端測試工具，請告訴我你偏好的語言與框架（VB.NET/C# WinForm 皆可）。
