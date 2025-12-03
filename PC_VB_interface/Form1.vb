Imports System.IO.Ports
Imports Microsoft.VisualBasic.Devices
Imports System.Threading.Tasks
Imports System.Management
Imports System.Collections.Generic
Imports System.Text.RegularExpressions

Public Class Form1
    Private serialPort As SerialPort
    Private cpuCounter As PerformanceCounter
    Private isInitializing As Boolean = True
    Private bluetoothPortName As String = Nothing

    Private Sub Form1_Load(sender As Object, e As EventArgs) Handles MyBase.Load
        Me.Text = "114 學年度工業類科學生技藝競賽電腦修護職種第二站崗位號碼：01"

        LoadComPorts()

        If cmbComPort.Items.Count > 0 Then
            cmbComPort.SelectedIndex = 0
        End If

        cmbMode.SelectedIndex = 0

        ' 初始化 CPU 計數器
        Try
            cpuCounter = New PerformanceCounter("Processor", "% Processor Time", "_Total")
            cpuCounter.NextValue()
        Catch ex As Exception
            cpuCounter = Nothing
        End Try

        ' 顯示初始系統資訊
        UpdateSystemInfo()

        ' 啟動計時器
        timerUpdate.Start()

        ' 標記初始化完成
        isInitializing = False

        ' 設定初始裝置狀態
        lblDeviceStatusValue.Text = "Device Offline"
        lblDeviceStatusValue.BackColor = Color.Red

        ' 延遲啟動串口並同步時間
        Dim startupTimer As New Timer()
        startupTimer.Interval = 500
        AddHandler startupTimer.Tick, Sub(s, args)
                                          startupTimer.Stop()
                                          AutoOpenPortAndSyncTime()
                                      End Sub
        startupTimer.Start()
    End Sub

    Private Function FindBluetoothComPorts() As (selectedPort As String, allPorts As HashSet(Of String))
        Dim allBluetoothPorts As New HashSet(Of String)()
        Dim bluetoothPortNumbers As New List(Of Integer)()
        
        Try
            ' 使用 WMI 查詢所有藍牙 COM Port（只查詢一次）
            Dim searcher As New ManagementObjectSearcher("SELECT * FROM Win32_PnPEntity WHERE Caption LIKE '%(COM%'")

            ' 收集所有藍牙連結序列埠
            For Each obj As ManagementObject In searcher.Get()
                Dim caption As String = If(obj("Caption") IsNot Nothing, obj("Caption").ToString(), "")
                Dim name As String = If(obj("Name") IsNot Nothing, obj("Name").ToString(), "")
                Dim description As String = If(obj("Description") IsNot Nothing, obj("Description").ToString(), "")
                
                Dim lowerCaption As String = caption.ToLower()
                Dim lowerName As String = name.ToLower()
                Dim lowerDesc As String = description.ToLower()
                
                ' 檢查是否為藍牙設備
                If lowerCaption.Contains("bluetooth") OrElse lowerName.Contains("bluetooth") OrElse lowerDesc.Contains("bluetooth") Then
                    ' 提取 COM 埠號
                    Dim comMatch As System.Text.RegularExpressions.Match = 
                        System.Text.RegularExpressions.Regex.Match(caption, "COM(\d+)")
                    
                    If comMatch.Success Then
                        Dim portNumber As Integer = Integer.Parse(comMatch.Groups(1).Value)
                        Dim portName As String = comMatch.Value
                        
                        ' 驗證此 COM 埠是否真的可用
                        If SerialPort.GetPortNames().Contains(portName) Then
                            allBluetoothPorts.Add(portName)
                            bluetoothPortNumbers.Add(portNumber)
                            Debug.WriteLine($"找到藍牙 Port: {portName}")
                        End If
                    End If
                End If
            Next

            ' 選擇埠號碼最小的
            If bluetoothPortNumbers.Count > 0 Then
                bluetoothPortNumbers.Sort()
                Dim selectedPort As String = "COM" & bluetoothPortNumbers(0).ToString()
                Debug.WriteLine($"選擇埠號碼最小的藍牙 Port: {selectedPort}")
                
                If bluetoothPortNumbers.Count > 1 Then
                    Dim excludedPorts = bluetoothPortNumbers.Skip(1).Select(Function(p) "COM" & p.ToString()).ToList()
                    Debug.WriteLine($"排除號碼較大的藍牙 Ports: {String.Join(", ", excludedPorts)}")
                End If
                
                Return (selectedPort, allBluetoothPorts)
            End If
        Catch ex As Exception
            Debug.WriteLine($"FindBluetoothComPorts 失敗: {ex.Message}")
        End Try

        Debug.WriteLine("FindBluetoothComPorts: 未找到藍牙 Port")
        Return (Nothing, allBluetoothPorts)
    End Function

    Private Sub LoadComPorts()
        cmbComPort.Items.Clear()
        Debug.WriteLine("=== 開始載入 COM Ports ===")

        ' 一次性查詢所有藍牙埠，並找出號碼最小的
        Dim bluetoothResult = FindBluetoothComPorts()
        bluetoothPortName = bluetoothResult.selectedPort
        Dim allBluetoothPorts = bluetoothResult.allPorts

        ' 如果找到藍牙串口，優先添加（號碼最小的）
        If bluetoothPortName IsNot Nothing Then
            cmbComPort.Items.Add(bluetoothPortName & " (藍牙)")
            Debug.WriteLine($"已添加號碼最小的藍牙 Port: {bluetoothPortName}")
        End If

        ' 添加所有其他串口（排除所有藍牙 Port）
        Dim allPorts() As String = SerialPort.GetPortNames()
        Debug.WriteLine($"系統中所有 COM Ports: {String.Join(", ", allPorts)}")
        Debug.WriteLine($"所有藍牙 Ports: {String.Join(", ", allBluetoothPorts)}")
        
        For Each port As String In allPorts
            ' 排除所有藍牙 Port（包括號碼大的）
            If allBluetoothPorts.Contains(port) Then
                Debug.WriteLine($"已過濾藍牙 Port: {port}")
                Continue For
            End If
            
            ' 加入非藍牙串口
            cmbComPort.Items.Add(port)
            Debug.WriteLine($"已添加非藍牙 Port: {port}")
        Next

        ' 如果沒有找到任何串口，添加預設值
        If cmbComPort.Items.Count = 0 Then
            cmbComPort.Items.Add("COM3")
            Debug.WriteLine("未找到任何 COM Port，已添加預設值 COM3")
        End If
        
        Debug.WriteLine($"=== 載入完成，共 {cmbComPort.Items.Count} 個 Ports ===")
    End Sub

    Private Async Sub AutoOpenPortAndSyncTime()
        Try
            Debug.WriteLine("=== 開始自動開啟串口 ===")
            
            If cmbComPort.Items.Count > 0 Then
                ' 自動選擇第一個串口（應該是號碼最小的藍牙埠）
                cmbComPort.SelectedIndex = 0
                Debug.WriteLine($"已選擇下拉選單第一個項目: {cmbComPort.SelectedItem}")

                ' 在 UI 執行緒上先取得控制項的值
                Dim selectedPortText As String = cmbComPort.SelectedItem.ToString()
                ' 移除標籌
                Dim selectedPort As String = selectedPortText.Replace(" (藍牙)", "")
                
                Debug.WriteLine($"準備開啟串口: {selectedPort}")
                Debug.WriteLine($"是否為藍牙埠: {If(bluetoothPortName IsNot Nothing AndAlso bluetoothPortName = selectedPort, "是", "否")}")
                Debug.WriteLine($"當前所有 COM Ports: {String.Join(", ", SerialPort.GetPortNames())}")
                Debug.WriteLine($"當前藍牙 Port: {If(bluetoothPortName, "無")}")

                ' 在後台執行緒開啟串口
                Await Task.Run(Sub()
                                   serialPort = New SerialPort()
                                   serialPort.PortName = selectedPort
                                   serialPort.BaudRate = 9600
                                   serialPort.DataBits = 8
                                   serialPort.Parity = Parity.None
                                   serialPort.StopBits = StopBits.One
                                   serialPort.ReadTimeout = 500
                                   serialPort.WriteTimeout = 500

                                   Debug.WriteLine($"正在開啟串口 {selectedPort}...")
                                   serialPort.Open()
                                   Debug.WriteLine($"串口 {selectedPort} 已成功開啟")
                               End Sub)

                ' 等待串口穩定
                Await Task.Delay(100)

                ' 更新 UI
                lblDeviceStatusValue.Text = "Device Online"
                lblDeviceStatusValue.BackColor = Color.LimeGreen

                ' 顯示連線成功訊息
                Dim connectionType As String = If(bluetoothPortName IsNot Nothing AndAlso bluetoothPortName = selectedPort, "藍牙", "串口")
                Debug.WriteLine($"=== 自動開啟串口成功: {selectedPort} ({connectionType}) ===")
                MessageBox.Show($"{connectionType}已開啟: {selectedPort}", "初始化成功", MessageBoxButtons.OK, MessageBoxIcon.Information)
            Else
                Debug.WriteLine("=== 沒有可用的串口 ===")
            End If
        Catch ex As UnauthorizedAccessException
            Debug.WriteLine($"串口被佔用: {ex.Message}")
            MessageBox.Show("串口被其他程式佔用，請關閉其他程式後重試", "錯誤", MessageBoxButtons.OK, MessageBoxIcon.Error)
            lblDeviceStatusValue.Text = "Device Offline"
            lblDeviceStatusValue.BackColor = Color.Red
        Catch ex As Exception
            Debug.WriteLine($"自動開啟串口失敗: {ex.Message}")
            MessageBox.Show("自動開啟串口失敗：" & ex.Message, "錯誤", MessageBoxButtons.OK, MessageBoxIcon.Error)
            lblDeviceStatusValue.Text = "Device Offline"
            lblDeviceStatusValue.BackColor = Color.Red
        End Try
    End Sub

    Private Sub timerUpdate_Tick(sender As Object, e As EventArgs) Handles timerUpdate.Tick
        lblCurrentTime.Text = "Current Time: " & DateTime.Now.ToString("yyyy/MM/dd HH:mm:ss")
        UpdateSystemInfo()
    End Sub

    Private Sub UpdateSystemInfo()
        Try
            ' 使用已初始化的 CPU 計數器
            If cpuCounter IsNot Nothing Then
                Dim cpuUsage As Integer = CInt(cpuCounter.NextValue())
                lblCpuUsage.Text = $"CPU 使用率：{cpuUsage}%"
            Else
                lblCpuUsage.Text = "CPU 使用率：--"
            End If

            Dim ramInfo As New ComputerInfo()
            Dim totalRam As Double = ramInfo.TotalPhysicalMemory / (1024 ^ 3)
            Dim availableRam As Double = ramInfo.AvailablePhysicalMemory / (1024 ^ 3)
            Dim usedRam As Double = totalRam - availableRam
            lblRamUsage.Text = $"RAM 使用率：{usedRam:F1}GB / {totalRam:F1}GB"
        Catch ex As Exception
            lblCpuUsage.Text = "CPU 使用率：--"
            lblRamUsage.Text = "RAM 使用率：-- / --"
        End Try
    End Sub

    Private Async Sub btnOpen_Click(sender As Object, e As EventArgs) Handles btnOpen.Click
        Try
            ' 禁用按鈕防止重複點擊
            btnOpen.Enabled = False

            ' 關閉現有連接
            If serialPort IsNot Nothing AndAlso serialPort.IsOpen Then
                serialPort.Close()
            End If

            ' 在 UI 執行緒上先取得控制項的值
            Dim selectedPortText As String = cmbComPort.SelectedItem.ToString()
            ' 移除標籤
            Dim selectedPort As String = selectedPortText.Replace(" (藍牙)", "")

            ' 在後台執行緒開啟串口
            Await Task.Run(Sub()
                               serialPort = New SerialPort()
                               serialPort.PortName = selectedPort
                               serialPort.BaudRate = 9600
                               serialPort.DataBits = 8
                               serialPort.Parity = Parity.None
                               serialPort.StopBits = StopBits.One
                               serialPort.ReadTimeout = 500
                               serialPort.WriteTimeout = 500

                               serialPort.Open()
                           End Sub)

            ' 等待串口穩定
            Await Task.Delay(100)

            ' 更新 UI
            lblDeviceStatusValue.Text = "Device Online"
            lblDeviceStatusValue.BackColor = Color.LimeGreen

            MessageBox.Show("串口已開啟", "成功", MessageBoxButtons.OK, MessageBoxIcon.Information)

        Catch ex As UnauthorizedAccessException
            MessageBox.Show("串口被其他程式佔用，請關閉其他程式後重試", "錯誤", MessageBoxButtons.OK, MessageBoxIcon.Error)
           	lblDeviceStatusValue.Text = "Device Offline"
            lblDeviceStatusValue.BackColor = Color.Red
        Catch ex As Exception
            MessageBox.Show("無法開啟串口：" & ex.Message, "錯誤", MessageBoxButtons.OK, MessageBoxIcon.Error)
            lblDeviceStatusValue.Text = "Device Offline"
            lblDeviceStatusValue.BackColor = Color.Red
        Finally
            btnOpen.Enabled = True
        End Try
    End Sub

    Private Sub btnClose_Click(sender As Object, e As EventArgs) Handles btnClose.Click
        Try
            If serialPort IsNot Nothing AndAlso serialPort.IsOpen Then
                serialPort.Close()
                lblDeviceStatusValue.Text = "Device Offline"
                lblDeviceStatusValue.BackColor = Color.Red
                MessageBox.Show("串口已關閉", "成功", MessageBoxButtons.OK, MessageBoxIcon.Information)
            End If
        Catch ex As Exception
            MessageBox.Show("關閉串口時發生錯誤：" & ex.Message, "錯誤", MessageBoxButtons.OK, MessageBoxIcon.Error)
        End Try
    End Sub

    Private Sub btnTimeSync_Click(sender As Object, e As EventArgs) Handles btnTimeSync.Click
        lblCurrentTime.Text = "Current Time: " & DateTime.Now.ToString("yyyy/MM/dd HH:mm:ss")

        ' 如果串口已開啟，同步時間到 MCU
        If serialPort IsNot Nothing AndAlso serialPort.IsOpen Then
            Try
                Dim currentTime As String = DateTime.Now.ToString("HH:mm:ss")
                serialPort.WriteLine(currentTime)
                MessageBox.Show($"時間已同步到 MCU：{currentTime}", "成功", MessageBoxButtons.OK, MessageBoxIcon.Information)
            Catch ex As Exception
                MessageBox.Show("同步時間到 MCU 失敗：" & ex.Message, "錯誤", MessageBoxButtons.OK, MessageBoxIcon.Error)
            End Try
        Else
            MessageBox.Show("時間已同步（串口未開啟）", "成功", MessageBoxButtons.OK, MessageBoxIcon.Information)
        End If
    End Sub

    Private Async Sub btnWrite_Click(sender As Object, e As EventArgs) Handles btnWrite.Click
        If serialPort IsNot Nothing AndAlso serialPort.IsOpen Then
            Try
                btnWrite.Enabled = False
                Await Task.Run(Sub()
                                   serialPort.WriteLine("WRITE")
                               End Sub)
                MessageBox.Show("已發送寫入命令", "成功", MessageBoxButtons.OK, MessageBoxIcon.Information)
            Catch ex As Exception
                MessageBox.Show("寫入失敗：" & ex.Message, "錯誤", MessageBoxButtons.OK, MessageBoxIcon.Error)
            Finally
                btnWrite.Enabled = True
            End Try
        Else
            MessageBox.Show("請先開啟串口", "提示", MessageBoxButtons.OK, MessageBoxIcon.Warning)
        End If
    End Sub

    Private Async Sub btnRead_Click(sender As Object, e As EventArgs) Handles btnRead.Click
        If serialPort IsNot Nothing AndAlso serialPort.IsOpen Then
            Try
                btnRead.Enabled = False
                Await Task.Run(Sub()
                                   serialPort.WriteLine("READ")
                               End Sub)
                MessageBox.Show("已發送讀取命令", "成功", MessageBoxButtons.OK, MessageBoxIcon.Information)
            Catch ex As Exception
                MessageBox.Show("讀取失敗：" & ex.Message, "錯誤", MessageBoxButtons.OK, MessageBoxIcon.Error)
            Finally
                btnRead.Enabled = True
            End Try
        Else
            MessageBox.Show("請先開啟串口", "提示", MessageBoxButtons.OK, MessageBoxIcon.Warning)
        End If
    End Sub

    Private Sub cmbMode_SelectedIndexChanged(sender As Object, e As EventArgs) Handles cmbMode.SelectedIndexChanged
        ' 跳過初始化階段的提示
        If isInitializing Then
            Return
        End If

        Select Case cmbMode.SelectedIndex
            Case 0
                MessageBox.Show("已選擇：鎖定螢幕", "模式選擇", MessageBoxButtons.OK, MessageBoxIcon.Information)
            Case 1
                MessageBox.Show("已選擇：顯示設置", "模式選擇", MessageBoxButtons.OK, MessageBoxIcon.Information)
            Case 2
                MessageBox.Show("已選擇：電腦資訊", "模式選擇", MessageBoxButtons.OK, MessageBoxIcon.Information)
        End Select
    End Sub

    Private Sub btnExit_Click(sender As Object, e As EventArgs) Handles btnExit.Click
        If MessageBox.Show("確定要退出程式嗎？", "確認", MessageBoxButtons.YesNo, MessageBoxIcon.Question) = DialogResult.Yes Then
            If serialPort IsNot Nothing AndAlso serialPort.IsOpen Then
                serialPort.Close()
            End If
            If cpuCounter IsNot Nothing Then
                cpuCounter.Dispose()
            End If
            Application.Exit()
        End If
    End Sub

    Private Sub Form1_FormClosing(sender As Object, e As FormClosingEventArgs) Handles MyBase.FormClosing
        If serialPort IsNot Nothing AndAlso serialPort.IsOpen Then
            serialPort.Close()
        End If
        If cpuCounter IsNot Nothing Then
            cpuCounter.Dispose()
        End If
    End Sub

End Class
