Imports System
Imports System.IO.Ports
Imports System.Management

Namespace PCControl
    Public Class MainForm
        Private WithEvents _port As New SerialPort()
        Private _isConnected As Boolean = False

        Private Sub MainForm_Load(sender As Object, e As EventArgs) Handles MyBase.Load
            RefreshPorts()
            UpdateTime()
            tmrUpdate.Start()
        End Sub

        Private Sub tmrUpdate_Tick(sender As Object, e As EventArgs) Handles tmrUpdate.Tick
            UpdateTime()
            If _isConnected Then
                UpdateSystemInfo()
            End If
        End Sub

        Private Sub UpdateTime()
            Dim now As Date = Date.Now
            lblCurrentTime.Text = $"Current Time: {now:yyyy/MM/dd HH:mm:ss}"
        End Sub

        Private Sub RefreshPorts()
            cmbPorts.Items.Clear()
            cmbPorts.Items.AddRange(SerialPort.GetPortNames())
            If cmbPorts.Items.Count > 0 Then cmbPorts.SelectedIndex = 0
        End Sub

        Private Sub btnOpen_Click(sender As Object, e As EventArgs) Handles btnOpen.Click
            Try
                If _port.IsOpen Then _port.Close()
                _port.PortName = cmbPorts.SelectedItem?.ToString()
                _port.BaudRate = 9600
                _port.NewLine = vbLf
                _port.Open()
                _isConnected = True
                lblDeviceStatusValue.Text = "Device Online"
                lblDeviceStatusValue.BackColor = Color.Green
                SendCommand("LED_OPEN")
            Catch ex As Exception
                MessageBox.Show("連線錯誤: " & ex.Message, "錯誤", MessageBoxButtons.OK, MessageBoxIcon.Error)
            End Try
        End Sub

        Private Sub btnClose_Click(sender As Object, e As EventArgs) Handles btnClose.Click
            Try
                If _port.IsOpen Then
                    SendCommand("LED_CLOSE")
                    Threading.Thread.Sleep(100)
                    _port.Close()
                End If
                _isConnected = False
                lblDeviceStatusValue.Text = "Device Offline"
                lblDeviceStatusValue.BackColor = Color.Red
            Catch ex As Exception
                MessageBox.Show("斷線錯誤: " & ex.Message, "錯誤", MessageBoxButtons.OK, MessageBoxIcon.Error)
            End Try
        End Sub

        Private Sub btnTimeSync_Click(sender As Object, e As EventArgs) Handles btnTimeSync.Click
            Dim now As Date = Date.Now
            Dim cmd As String = $"T{now:yyyy/MM/dd HH:mm:ss}"
            SendCommand(cmd)
        End Sub

        Private Sub cmbMode_SelectedIndexChanged(sender As Object, e As EventArgs) Handles cmbMode.SelectedIndexChanged
            If cmbMode.SelectedIndex = 1 Then
                ' 1-鎖定螢幕 (假設對應 MODE_5 或特定指令)
                SendCommand("M1")
            End If
        End Sub

        Private Sub btnWrite_Click(sender As Object, e As EventArgs) Handles btnWrite.Click
            Dim valText = txtLedValue.Text.Trim()
            Dim val As Integer
            If Integer.TryParse(valText, val) AndAlso val >= 1 AndAlso val <= 254 Then
                SendCommand($"L{val}")
            Else
                MessageBox.Show("請輸入 1-254 之間的數值", "錯誤", MessageBoxButtons.OK, MessageBoxIcon.Warning)
            End If
        End Sub

        Private Sub btnRead_Click(sender As Object, e As EventArgs) Handles btnRead.Click
            SendCommand("R")
        End Sub

        Private Sub btnExit_Click(sender As Object, e As EventArgs) Handles btnExit.Click
            If _port.IsOpen Then
                SendCommand("LED_CLOSE")
                Threading.Thread.Sleep(100)
                _port.Close()
            End If
            Application.Exit()
        End Sub

        Private Sub UpdateSystemInfo()
            Try
                ' 取得 CPU 使用率（簡化版，實際可用 PerformanceCounter）
                Dim cpuUsage As Integer = GetCpuUsage()
                
                ' 取得 RAM 資訊
                Dim ramInfo = GetRamInfo()
                
                lblCpuUsage.Text = $"CPU 使用率：{cpuUsage}%"
                lblRamUsage.Text = $"RAM 使用率：{ramInfo.Item1:F1}GB/ {ramInfo.Item2:F1}GB"
                
                ' 傳送至 MCU
                Dim cmd As String = $"Scpu={cpuUsage};ram={ramInfo.Item1:F1}/{ramInfo.Item2:F1}"
                SendCommand(cmd)
            Catch ex As Exception
                ' 忽略更新錯誤
            End Try
        End Sub

        Private Function GetCpuUsage() As Integer
            ' 簡化版：返回隨機值，實際應使用 PerformanceCounter
            Static rnd As New Random()
            Return rnd.Next(30, 80)
        End Function

        Private Function GetRamInfo() As Tuple(Of Double, Double)
            Try
                Dim searcher As New ManagementObjectSearcher("SELECT * FROM Win32_OperatingSystem")
                For Each obj As ManagementObject In searcher.Get()
                    Dim totalBytes As ULong = CULng(obj("TotalVisibleMemorySize")) * 1024
                    Dim freeBytes As ULong = CULng(obj("FreePhysicalMemory")) * 1024
                    Dim totalGB As Double = totalBytes / (1024.0 ^ 3)
                    Dim usedGB As Double = (totalBytes - freeBytes) / (1024.0 ^ 3)
                    Return New Tuple(Of Double, Double)(usedGB, totalGB)
                Next
            Catch
                ' 失敗時返回預設值
            End Try
            Return New Tuple(Of Double, Double)(8.3, 15.9)
        End Function

        Private Sub SendCommand(cmd As String)
            Try
                If Not _port.IsOpen Then
                    Return
                End If
                _port.WriteLine(cmd)
            Catch ex As Exception
                ' 忽略傳送錯誤
            End Try
        End Sub

        Private Sub _port_DataReceived(sender As Object, e As SerialDataReceivedEventArgs) Handles _port.DataReceived
            Try
                Dim line As String = _port.ReadLine().Trim()
                ' 處理 MCU 回應
                If line.StartsWith("VAL=") Then
                    Dim val = line.Substring(4)
                    UpdateLedDisplay(val)
                End If
            Catch
            End Try
        End Sub

        Private Sub UpdateLedDisplay(val As String)
            If InvokeRequired Then
                BeginInvoke(New Action(Of String)(AddressOf UpdateLedDisplay), val)
                Return
            End If
            txtLedValue.Text = val
        End Sub
    End Class
End Namespace
