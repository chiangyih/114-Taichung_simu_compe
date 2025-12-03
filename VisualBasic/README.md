# Visual Basic 專案使用指南

- 專案路徑: `VisualBasic/PCControl/PCControl.sln`
- 專案類型: VB.NET Windows Forms (.NET Framework 4.7.2)

## 功能
- 串列埠連線/斷線、重新整理可用 COM 連接埠
- 指令按鈕:
  - `LED_OPEN` / `LED_CLOSE`
  - `Tyyyy/MM/dd HH:mm:ss`
  - `Scpu=<int>;ram=<used>/<total>`
  - `M1` / `M2`
  - `R` / `L<n>`
- 記錄 MCU 回應 (OK/ERR/VAL/WRITE OK 等)

## 建置與執行
1. 以 Visual Studio 2019/2022 開啟 `PCControl.sln`
2. 編譯並執行，選擇對應的 `COM` 連接埠
3. `連線` 後，按 `LED_OPEN` 開始使 MCU 允許 LED 顯示

## 注意事項
- MCU 端序列鮑率須為 `9600`
- `LED_OPEN` 為 LED 啟動依據，僅時間指令 `T...` 不會自動開啟 LED
