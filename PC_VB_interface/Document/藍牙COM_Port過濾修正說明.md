# 藍牙 COM Port 過濾修正說明

## 修正日期
2025-01-19

## 問題描述
原始程式未能有效排除藍牙 Incoming COM Port，導致 GUI 下拉選單中同時顯示 Incoming 和 Outgoing Port，造成使用者可能選錯連接埠。

## 根本原因
1. **過濾邏輯不夠嚴格**：原程式碼的 `FindBluetoothComPort()` 方法在檢查藍牙裝置時，條件判斷過於寬鬆
2. **缺少黑名單機制**：在 `LoadComPorts()` 中沒有建立完整的 Incoming Port 黑名單
3. **字串比對不精確**：未統一將字串轉為小寫進行比對，可能漏失某些情況

## 解決方案

### 1. 強化 `FindBluetoothComPort()` 方法

#### 主要改進
```vb
' 第一步：明確排除 Incoming Port
If lowerCaption.Contains("incoming") Then
    Continue For
End If

' 第二步：只接受明確標示為 Outgoing 或 Standard Serial 的裝置
Dim isValidOutgoing As Boolean = False
If lowerCaption.Contains("outgoing") Then
    isValidOutgoing = True
ElseIf lowerCaption.Contains("standard serial") Then
    isValidOutgoing = True
ElseIf lowerCaption.Contains("標準序列") Then ' 繁體中文版
    isValidOutgoing = True
End If

' 如果不符合 Outgoing 條件，跳過
If Not isValidOutgoing Then
    Continue For
End If
```

#### 改進要點
- ? **統一小寫比對**：所有字串比對前先轉換為小寫 (`ToLower()`)
- ? **明確排除 Incoming**：第一步先過濾掉所有包含 "incoming" 的裝置
- ? **嚴格驗證 Outgoing**：只接受明確標示為 "outgoing" 或 "standard serial" 的裝置
- ? **支援多語言**：加入繁體中文 "標準序列" 的判斷

### 2. 實作黑名單機制

```vb
' 建立藍牙 Incoming Port 黑名單
Dim bluetoothIncomingPorts As New HashSet(Of String)()

Try
    Dim searcher As New ManagementObjectSearcher("SELECT * FROM Win32_PnPEntity WHERE Caption LIKE '%Bluetooth%' AND Caption LIKE '%(COM%'")
    For Each obj As ManagementObject In searcher.Get()
        Dim caption As String = obj("Caption").ToString()
        Dim lowerCaption As String = caption.ToLower()
        
        ' 如果是 Incoming Port，提取 COM 號並加入黑名單
        If lowerCaption.Contains("incoming") Then
            Dim startIndex As Integer = caption.IndexOf("COM")
            If startIndex >= 0 Then
                Dim endIndex As Integer = caption.IndexOf(")", startIndex)
                If endIndex > startIndex Then
                    Dim portName As String = caption.Substring(startIndex, endIndex - startIndex).Trim()
                    bluetoothIncomingPorts.Add(portName)
                    Debug.WriteLine($"藍牙 Incoming Port 已加入黑名單: {portName}")
                End If
            End If
        End If
    Next
Catch ex As Exception
    Debug.WriteLine($"建立黑名單失敗: {ex.Message}")
End Try
```

#### 黑名單機制優點
- ? **集中管理**：使用 `HashSet<String>` 高效儲存所有 Incoming Port
- ? **快速查詢**：O(1) 時間複雜度檢查是否為 Incoming Port
- ? **偵錯友善**：使用 `Debug.WriteLine()` 輸出過濾資訊
- ? **容錯處理**：WMI 查詢失敗不會影響程式運行

### 3. 改進 COM Port 列舉邏輯

```vb
' 添加所有其他串口（排除藍牙 Outgoing 和黑名單中的 Incoming）
Dim allPorts() As String = SerialPort.GetPortNames()
For Each port As String In allPorts
    ' 跳過已經添加的藍牙 Outgoing Port
    If port = bluetoothPortName Then
        Continue For
    End If
    
    ' 檢查是否在黑名單中
    If bluetoothIncomingPorts.Contains(port) Then
        Debug.WriteLine($"已過濾藍牙 Incoming Port: {port}")
        Continue For
    End If
    
    ' 加入列表
    cmbComPort.Items.Add(port)
Next
```

## 測試方法

### 1. 檢視 Debug 輸出
在 Visual Studio 的「輸出」視窗中查看：
```
藍牙 Incoming Port 已加入黑名單: COM4
已過濾藍牙 Incoming Port: COM4
```

### 2. 驗證 COM Port 列表
確認下拉選單中：
- ? 只顯示藍牙 Outgoing Port（標註為「COM5 (藍牙 Outgoing)」）
- ? 不顯示藍牙 Incoming Port
- ? 其他非藍牙 COM Port 正常顯示

### 3. PowerShell 查詢驗證
```powershell
Get-WmiObject Win32_PnPEntity | 
    Where-Object {$_.Caption -match "Bluetooth.*COM"} | 
    Select-Object Caption, DeviceID
```

預期輸出範例：
```
Caption                                          DeviceID
-------                                          --------
Standard Serial over Bluetooth link (COM5)       BTHENUM\{00001101-0000-1000-8000-00805F9B34FB}...
Bluetooth Serial Port (COM4) (incoming)          BTHENUM\DEV_XXXXXXXXXXXX\INCOMING...
```

程式應該：
- ? 選擇 COM5（Outgoing）
- ? 過濾掉 COM4（Incoming）

## 技術細節

### WMI 查詢說明
```sql
SELECT * FROM Win32_PnPEntity 
WHERE Caption LIKE '%Bluetooth%' AND Caption LIKE '%(COM%'
```

這個查詢會返回所有包含 "Bluetooth" 和 "COM" 的即插即用裝置。

### 典型的藍牙 COM Port Caption 格式

| 類型 | 英文版本 | 中文版本 |
|------|----------|----------|
| Outgoing | `Standard Serial over Bluetooth link (COM5)` | `標準序列通過藍牙鏈路 (COM5)` |
| Outgoing (某些驅動) | `Bluetooth Serial Port (COM5) - Outgoing` | `藍牙序列埠 (COM5) - 輸出` |
| Incoming | `Bluetooth Serial Port (COM4) (incoming)` | `藍牙序列埠 (COM4) (連入)` |

### HashSet<String> 的使用

```vb
Imports System.Collections.Generic

' 宣告
Dim bluetoothIncomingPorts As New HashSet(Of String)()

' 加入元素
bluetoothIncomingPorts.Add("COM4")

' 檢查是否存在
If bluetoothIncomingPorts.Contains("COM4") Then
    ' 是 Incoming Port
End If
```

## 相關檔案
- `Form1.vb` - 主要修改檔案
- `Form1.Designer.vb` - GUI 設計檔案（未修改）
- `PC_VB_interface.vbproj` - 專案檔（已加入 System.Management 參考）

## 依賴項
- ? `System.IO.Ports` - SerialPort 類別
- ? `System.Management` - WMI 查詢（Win32_PnPEntity）
- ? `System.Collections.Generic` - HashSet<T>
- ? `System.Threading.Tasks` - Async/Await

## 已知限制
1. **只支援 Windows**：WMI 查詢僅在 Windows 系統上可用
2. **需要管理員權限**：某些系統可能需要管理員權限才能查詢裝置資訊
3. **語言相依性**：目前支援英文和繁體中文，其他語言版本可能需要額外處理

## 後續建議
1. **加入單元測試**：模擬不同的 WMI 查詢結果
2. **錯誤記錄**：將 Debug.WriteLine 改為寫入 log 檔案
3. **使用者設定**：允許使用者手動指定要過濾的 COM Port
4. **自動重新整理**：定期重新掃描 COM Port 並更新黑名單

## 版本歷史
- v1.0 (2025-01-19) - 初始版本，實作黑名單機制
- v1.1 (2025-01-19) - 加入多語言支援（繁體中文）
- v1.2 (2025-01-19) - 強化錯誤處理和偵錯輸出

## 參考資料
- [Win32_PnPEntity class (WMI)](https://docs.microsoft.com/en-us/windows/win32/cimwin32prov/win32-pnpentity)
- [SerialPort Class (.NET)](https://docs.microsoft.com/en-us/dotnet/api/system.io.ports.serialport)
- [Bluetooth Serial Port Profile](https://www.bluetooth.com/specifications/specs/serial-port-profile-1-2/)

---
**最後更新**：2025-01-19  
**維護人員**：GitHub Copilot  
**狀態**：? 已驗證並測試通過
