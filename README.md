# 114 學年度 — 第二站 USB 介面卡製作與控制（整合版 README）
Last Update: 2025-12-02

本 README 針對本工作區的練習與教學，整合 `114_PCRepair_Station2_Analysis.md` 與 `MCU_FunctionSpec.md` 的重點，提供快速導覽與實作指南。

## 任務概要
- 目標：完成 PC 應用程式 + MCU（Arduino）之 USB 介面卡整合控制。
- 重點：TFT 顯示、WS2812 LED、按鍵行為、EEPROM 狀態保存、PC 端 Open/Close 與時間同步、CPU/RAM 顯示。

## 硬體與連線
- MCU：ATmega328P（Nano/UNO 類）
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
