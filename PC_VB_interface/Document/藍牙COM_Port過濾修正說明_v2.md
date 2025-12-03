# 藍牙 COM Port 過濾問題修正 v2.0

## 修正日期
2025-01-19 (第二版)

## 問題回報
使用者回報即使套用了 v1.0 的修正，GUI 中仍然出現重複的 COM Port（如截圖中的 COM12 出現兩次），表示 Incoming Port 未被正確過濾。

## 根本原因分析

### v1.0 版本的問題
1. **只檢查 Caption 屬性**：原始程式碼只檢查 `Win32_PnPEntity` 的 `Caption` 屬性
2. **某些驅動的 Caption 不完整**：某些藍牙驅動程式的 `Caption` 屬性可能不包含 "incoming" 標記
3. **需要檢查多個屬性**：應該同時檢查 `Caption`、`Name` 和 `Description` 屬性

### 實際案例
從使用者的截圖可以看到：
```
COM12  (出現兩次)
COM12
COM11
COM7
```

這表示有一個 COM12 是 Incoming Port，但沒有被過濾掉。

## v2.0 解決方案

### 1. 多屬性檢查

#### 修改前（v1.0）
```vb
Dim caption As String = obj("Caption").ToString()
Dim lowerCaption As String = caption.ToLower()

If lowerCaption.Contains("incoming") Then
    Continue For
End If
```

#### 修改後（v2.0）
```vb
Dim caption As String = If(obj("Caption") IsNot Nothing, obj("Caption").ToString(), "")
Dim name As String = If(obj("Name") IsNot Nothing, obj("Name").ToString(), "")
Dim description As String = If(obj("Description") IsNot Nothing, obj("Description").ToString(), "")

Dim lowerCaption As String = caption.ToLower()
Dim lowerName As String = name.ToLower()
Dim lowerDesc As String = description.ToLower()

' 檢查所有屬性
If lowerCaption.Contains("incoming") OrElse 
   lowerName.Contains("incoming") OrElse 
   lowerDesc.Contains("incoming") Then
    Debug.WriteLine($"排除 Incoming Port: Caption={caption}, Name={name}")
    Continue For
End If
```

### 2. 使用正則表達式提取 COM 號

#### 問題
原始方法使用 `IndexOf()` 和 `Substring()` 提取 COM 號，容易出錯：
```vb
Dim startIndex As Integer = caption.IndexOf("COM")
Dim endIndex As Integer = caption.IndexOf(")", startIndex)
Dim portName As String = caption.Substring(startIndex, endIndex - startIndex).Trim()
```

#### 改進
使用正則表達式更可靠：
```vb
Dim comMatch As System.Text.RegularExpressions.Match = 
    System.Text.RegularExpressions.Regex.Match(caption, "COM(\d+)")

If comMatch.Success Then
    Dim portName As String = comMatch.Value  ' 例如: "COM12"
    ' ...
End If
```

### 3. 完整的黑名單機制

```vb
' 建立完整的黑名單：包括藍牙 Incoming Port 和已添加的 Outgoing Port
Dim blacklistedPorts As New HashSet(Of String)()

' 將已選擇的藍牙 Outgoing Port 加入黑名單，避免重複
If bluetoothPortName IsNot Nothing Then
    blacklistedPorts.Add(bluetoothPortName)
End If

' 掃描所有藍牙設備
Try
    Dim searcher As New ManagementObjectSearcher("SELECT * FROM Win32_PnPEntity WHERE Caption LIKE '%(COM%'")
    For Each obj As ManagementObject In searcher.Get()
        ' 獲取所有相關屬性
        Dim caption As String = If(obj("Caption") IsNot Nothing, obj("Caption").ToString(), "")
        Dim name As String = If(obj("Name") IsNot Nothing, obj("Name").ToString(), "")
        Dim description As String = If(obj("Description") IsNot Nothing, obj("Description").ToString(), "")
        
        ' 檢查是否為藍牙設備（檢查所有屬性）
        Dim isBluetooth As Boolean = 
            caption.ToLower().Contains("bluetooth") OrElse 
            name.ToLower().Contains("bluetooth") OrElse 
            description.ToLower().Contains("bluetooth")
        
        If Not isBluetooth Then
            Continue For
        End If
        
        ' 檢查是否為 Incoming Port（檢查所有屬性）
        Dim isIncoming As Boolean = 
            caption.ToLower().Contains("incoming") OrElse 
            name.ToLower().Contains("incoming") OrElse 
            description.ToLower().Contains("incoming")
        
        If isIncoming Then
            ' 使用正則表達式提取 COM 號
            Dim comMatch As System.Text.RegularExpressions.Match = 
                System.Text.RegularExpressions.Regex.Match(caption, "COM(\d+)")
            
            If comMatch.Success Then
                Dim portName As String = comMatch.Value
                blacklistedPorts.Add(portName)
                Debug.WriteLine($"藍牙 Incoming Port 已加入黑名單: {portName}")
            End If
        End If
    Next
Catch ex As Exception
    Debug.WriteLine($"建立黑名單失敗: {ex.Message}")
End Try
```

### 4. 詳細的 Debug 輸出

```vb
Debug.WriteLine("=== 開始載入 COM Ports ===")
Debug.WriteLine($"系統中所有 COM Ports: {String.Join(", ", allPorts)}")
Debug.WriteLine($"黑名單中的 Ports: {String.Join(", ", blacklistedPorts)}")

For Each port As String In allPorts
    If blacklistedPorts.Contains(port) Then
        Debug.WriteLine($"已過濾 Port: {port}")
        Continue For
    End If
    
    cmbComPort.Items.Add(port)
    Debug.WriteLine($"已添加 Port: {port}")
Next

Debug.WriteLine($"=== 載入完成，共 {cmbComPort.Items.Count} 個 Ports ===")
```

## 如何使用 Debug 輸出進行診斷

### 1. 在 Visual Studio 中查看輸出視窗

1. **執行程式**（F5 或 Debug > Start Debugging）
2. **開啟輸出視窗**：
   - 方法 1：`View` > `Output` (或按 `Ctrl+W, O`)
   - 方法 2：在下方面板選擇「輸出」分頁
3. **查看 Debug 訊息**：
   - 在下拉選單中選擇「偵錯」或「Debug」

### 2. 預期的 Debug 輸出範例

#### 正常情況（成功過濾）
```
=== 開始載入 COM Ports ===
找到 Standard Serial Port: Standard Serial over Bluetooth link (COM5)
確認找到有效的藍牙 Outgoing Port: COM5
已添加藍牙 Outgoing Port: COM5
藍牙 Incoming Port 已加入黑名單: COM4 (來源: Bluetooth Serial Port (COM4) (incoming))
系統中所有 COM Ports: COM4, COM5, COM7, COM11, COM12
黑名單中的 Ports: COM4, COM5
已過濾 Port: COM4
已過濾 Port: COM5
已添加 Port: COM7
已添加 Port: COM11
已添加 Port: COM12
=== 載入完成，共 4 個 Ports ===
```

#### 問題情況（未成功過濾）
```
=== 開始載入 COM Ports ===
找到 Standard Serial Port: Standard Serial over Bluetooth link (COM12)
確認找到有效的藍牙 Outgoing Port: COM12
已添加藍牙 Outgoing Port: COM12
系統中所有 COM Ports: COM7, COM11, COM12, COM12
黑名單中的 Ports: COM12
已過濾 Port: COM12
已添加 Port: COM7
已添加 Port: COM11
已添加 Port: COM12  ← 這個 COM12 應該被過濾但沒有！
=== 載入完成，共 4 個 Ports ===
```

### 3. 如果仍然出現重複，請檢查：

1. **確認 Debug 輸出中的黑名單**：
   ```
   黑名單中的 Ports: COM4, COM5
   ```
   是否包含所有 Incoming Port？

2. **檢查是否有兩個不同的 COM12**：
   - Windows 可能顯示兩個不同的 COM12 裝置
   - 使用 PowerShell 確認：
   ```powershell
   Get-WmiObject Win32_PnPEntity | 
       Where-Object {$_.Caption -match "COM12"} | 
       Select-Object Caption, Name, Description, DeviceID
   ```

3. **檢查 WMI 屬性**：
   ```powershell
   Get-WmiObject Win32_PnPEntity | 
       Where-Object {$_.Caption -match "Bluetooth.*COM"} | 
       Select-Object Caption, Name, Description | 
       Format-List
   ```

## 新增的命名空間

```vb
Imports System.Text.RegularExpressions
```

這是用於正則表達式的命名空間。

## 完整的命名空間清單

```vb
Imports System.IO.Ports
Imports Microsoft.VisualBasic.Devices
Imports System.Threading.Tasks
Imports System.Management
Imports System.Collections.Generic
Imports System.Text.RegularExpressions
```

## 測試步驟

### 步驟 1：清除舊的執行檔
```powershell
# 在專案目錄執行
Remove-Item -Path "bin\*" -Recurse -Force
Remove-Item -Path "obj\*" -Recurse -Force
```

### 步驟 2：重新建置
在 Visual Studio 中：
1. `Build` > `Clean Solution`
2. `Build` > `Rebuild Solution`

### 步驟 3：執行並檢查 Debug 輸出
1. 按 F5 執行程式
2. 開啟輸出視窗 (`Ctrl+W, O`)
3. 檢查 Debug 訊息

### 步驟 4：驗證 GUI
檢查 COM Port 下拉選單中：
- ? 只有一個藍牙 Outgoing Port（標註為「(藍牙 Outgoing)」）
- ? 沒有重複的 COM Port
- ? 沒有藍牙 Incoming Port

## 如果問題仍然存在

請執行以下 PowerShell 命令並提供輸出：

```powershell
# 查詢所有藍牙 COM Port
Get-WmiObject Win32_PnPEntity | 
    Where-Object {$_.Caption -match "Bluetooth.*COM"} | 
    Select-Object @{N='Caption';E={$_.Caption}}, 
                  @{N='Name';E={$_.Name}}, 
                  @{N='Description';E={$_.Description}}, 
                  @{N='DeviceID';E={$_.DeviceID}} | 
    Format-Table -AutoSize

# 查詢所有包含 COM 的裝置
Get-WmiObject Win32_PnPEntity | 
    Where-Object {$_.Caption -match "\(COM\d+\)"} | 
    Select-Object Caption, Name, Description | 
    Format-Table -Wrap
```

## 版本歷史

| 版本 | 日期 | 變更內容 |
|------|------|----------|
| v1.0 | 2025-01-19 | 初始版本，基本過濾機制 |
| v1.1 | 2025-01-19 | 加入多語言支援 |
| v2.0 | 2025-01-19 | **重大更新**：多屬性檢查、正則表達式、完整 Debug 輸出 |

## 技術摘要

### 關鍵改進
1. ? 檢查 `Caption`、`Name`、`Description` 三個屬性
2. ? 使用正則表達式 `COM(\d+)` 提取 COM 號
3. ? 完整的黑名單機制（包括 Outgoing Port 避免重複）
4. ? 詳細的 Debug 輸出供診斷

### 適用場景
- Windows 10/11
- HC-05/HC-06 藍牙模組
- 各種藍牙驅動程式（Broadcom、Intel、Realtek 等）

---

**最後更新**：2025-01-19 v2.0  
**維護人員**：GitHub Copilot  
**狀態**：? 已強化並增加診斷功能
