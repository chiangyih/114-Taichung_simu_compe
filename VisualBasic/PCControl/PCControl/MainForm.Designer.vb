Namespace PCControl
    <Global.Microsoft.VisualBasic.CompilerServices.DesignerGenerated()> _
    Partial Class MainForm
        Inherits System.Windows.Forms.Form

        Private components As System.ComponentModel.IContainer
        Private WithEvents tmrUpdate As System.Windows.Forms.Timer
        Private lblCurrentTime As System.Windows.Forms.Label
        Private btnTimeSync As System.Windows.Forms.Button
        Private cmbMode As System.Windows.Forms.ComboBox
        Private grpCardStatus As System.Windows.Forms.GroupBox
        Private cmbPorts As System.Windows.Forms.ComboBox
        Private btnOpen As System.Windows.Forms.Button
        Private btnClose As System.Windows.Forms.Button
        Private lblDeviceStatus As System.Windows.Forms.Label
        Private lblDeviceStatusValue As System.Windows.Forms.Label
        Private btnExit As System.Windows.Forms.Button
        Private grpSystemInfo As System.Windows.Forms.GroupBox
        Private lblCpuUsage As System.Windows.Forms.Label
        Private lblRamUsage As System.Windows.Forms.Label
        Private grpLedControl As System.Windows.Forms.GroupBox
        Private txtLedValue As System.Windows.Forms.TextBox
        Private btnWrite As System.Windows.Forms.Button
        Private btnRead As System.Windows.Forms.Button

        <System.Diagnostics.DebuggerStepThrough()> _
        Private Sub InitializeComponent()
            Me.components = New System.ComponentModel.Container()
            Me.tmrUpdate = New System.Windows.Forms.Timer(Me.components)
            Me.lblCurrentTime = New System.Windows.Forms.Label()
            Me.btnTimeSync = New System.Windows.Forms.Button()
            Me.cmbMode = New System.Windows.Forms.ComboBox()
            Me.grpCardStatus = New System.Windows.Forms.GroupBox()
            Me.cmbPorts = New System.Windows.Forms.ComboBox()
            Me.btnOpen = New System.Windows.Forms.Button()
            Me.btnClose = New System.Windows.Forms.Button()
            Me.lblDeviceStatus = New System.Windows.Forms.Label()
            Me.lblDeviceStatusValue = New System.Windows.Forms.Label()
            Me.btnExit = New System.Windows.Forms.Button()
            Me.grpSystemInfo = New System.Windows.Forms.GroupBox()
            Me.lblCpuUsage = New System.Windows.Forms.Label()
            Me.lblRamUsage = New System.Windows.Forms.Label()
            Me.grpLedControl = New System.Windows.Forms.GroupBox()
            Me.txtLedValue = New System.Windows.Forms.TextBox()
            Me.btnWrite = New System.Windows.Forms.Button()
            Me.btnRead = New System.Windows.Forms.Button()
            Me.grpCardStatus.SuspendLayout()
            Me.grpSystemInfo.SuspendLayout()
            Me.grpLedControl.SuspendLayout()
            Me.SuspendLayout()
            '
            'tmrUpdate
            Me.tmrUpdate.Interval = 1000
            '
            'lblCurrentTime
            Me.lblCurrentTime.AutoSize = True
            Me.lblCurrentTime.Font = New System.Drawing.Font("新細明體", 12.0!, System.Drawing.FontStyle.Regular)
            Me.lblCurrentTime.Location = New System.Drawing.Point(15, 15)
            Me.lblCurrentTime.Size = New System.Drawing.Size(280, 16)
            Me.lblCurrentTime.Text = "Current Time: 2024/10/23 13:20:35"
            '
            'btnTimeSync
            Me.btnTimeSync.Location = New System.Drawing.Point(310, 10)
            Me.btnTimeSync.Size = New System.Drawing.Size(90, 30)
            Me.btnTimeSync.Text = "Time Sync"
            '
            'cmbMode
            Me.cmbMode.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList
            Me.cmbMode.Font = New System.Drawing.Font("新細明體", 11.0!)
            Me.cmbMode.Location = New System.Drawing.Point(530, 10)
            Me.cmbMode.Size = New System.Drawing.Size(200, 26)
            Me.cmbMode.Items.AddRange(New Object() {"模式選擇", "1-鎖定螢幕"})
            Me.cmbMode.SelectedIndex = 0
            '
            'grpCardStatus
            Me.grpCardStatus.Location = New System.Drawing.Point(15, 60)
            Me.grpCardStatus.Size = New System.Drawing.Size(370, 240)
            Me.grpCardStatus.Text = "介面卡狀態"
            Me.grpCardStatus.Font = New System.Drawing.Font("新細明體", 11.0!)
            Me.grpCardStatus.Controls.Add(Me.cmbPorts)
            Me.grpCardStatus.Controls.Add(Me.btnOpen)
            Me.grpCardStatus.Controls.Add(Me.btnClose)
            Me.grpCardStatus.Controls.Add(Me.lblDeviceStatus)
            Me.grpCardStatus.Controls.Add(Me.lblDeviceStatusValue)
            Me.grpCardStatus.Controls.Add(Me.btnExit)
            '
            'cmbPorts
            Me.cmbPorts.DropDownStyle = System.Windows.Forms.ComboBoxStyle.DropDownList
            Me.cmbPorts.Font = New System.Drawing.Font("新細明體", 12.0!)
            Me.cmbPorts.Location = New System.Drawing.Point(20, 30)
            Me.cmbPorts.Size = New System.Drawing.Size(140, 28)
            '
            'btnOpen
            Me.btnOpen.Font = New System.Drawing.Font("新細明體", 11.0!)
            Me.btnOpen.Location = New System.Drawing.Point(180, 28)
            Me.btnOpen.Size = New System.Drawing.Size(75, 30)
            Me.btnOpen.Text = "Open"
            '
            'btnClose
            Me.btnClose.Font = New System.Drawing.Font("新細明體", 11.0!)
            Me.btnClose.Location = New System.Drawing.Point(270, 28)
            Me.btnClose.Size = New System.Drawing.Size(75, 30)
            Me.btnClose.Text = "Close"
            '
            'lblDeviceStatus
            Me.lblDeviceStatus.AutoSize = True
            Me.lblDeviceStatus.Font = New System.Drawing.Font("新細明體", 11.0!)
            Me.lblDeviceStatus.Location = New System.Drawing.Point(20, 80)
            Me.lblDeviceStatus.Text = "Device Status："
            '
            'lblDeviceStatusValue
            Me.lblDeviceStatusValue.BackColor = System.Drawing.Color.Green
            Me.lblDeviceStatusValue.Font = New System.Drawing.Font("新細明體", 12.0!, System.Drawing.FontStyle.Bold)
            Me.lblDeviceStatusValue.ForeColor = System.Drawing.Color.White
            Me.lblDeviceStatusValue.Location = New System.Drawing.Point(180, 75)
            Me.lblDeviceStatusValue.Size = New System.Drawing.Size(150, 28)
            Me.lblDeviceStatusValue.Text = "Device Online"
            Me.lblDeviceStatusValue.TextAlign = System.Drawing.ContentAlignment.MiddleCenter
            '
            'btnExit
            Me.btnExit.Font = New System.Drawing.Font("新細明體", 14.0!, System.Drawing.FontStyle.Bold)
            Me.btnExit.Location = New System.Drawing.Point(70, 155)
            Me.btnExit.Size = New System.Drawing.Size(230, 60)
            Me.btnExit.Text = "EXIT"
            '
            'grpSystemInfo
            Me.grpSystemInfo.Location = New System.Drawing.Point(400, 60)
            Me.grpSystemInfo.Size = New System.Drawing.Size(330, 145)
            Me.grpSystemInfo.Text = "系統資訊"
            Me.grpSystemInfo.Font = New System.Drawing.Font("新細明體", 11.0!)
            Me.grpSystemInfo.Controls.Add(Me.lblCpuUsage)
            Me.grpSystemInfo.Controls.Add(Me.lblRamUsage)
            '
            'lblCpuUsage
            Me.lblCpuUsage.AutoSize = True
            Me.lblCpuUsage.Font = New System.Drawing.Font("新細明體", 11.0!)
            Me.lblCpuUsage.Location = New System.Drawing.Point(20, 35)
            Me.lblCpuUsage.Text = "CPU 使用率：58%"
            '
            'lblRamUsage
            Me.lblRamUsage.AutoSize = True
            Me.lblRamUsage.Font = New System.Drawing.Font("新細明體", 11.0!)
            Me.lblRamUsage.Location = New System.Drawing.Point(20, 70)
            Me.lblRamUsage.Text = "RAM 使用率：8.3GB/ 15.9GB"
            '
            'grpLedControl
            Me.grpLedControl.Location = New System.Drawing.Point(400, 215)
            Me.grpLedControl.Size = New System.Drawing.Size(330, 85)
            Me.grpLedControl.Text = "LED腳位讀寫"
            Me.grpLedControl.Font = New System.Drawing.Font("新細明體", 11.0!)
            Me.grpLedControl.Controls.Add(Me.txtLedValue)
            Me.grpLedControl.Controls.Add(Me.btnWrite)
            Me.grpLedControl.Controls.Add(Me.btnRead)
            '
            'txtLedValue
            Me.txtLedValue.Font = New System.Drawing.Font("新細明體", 11.0!)
            Me.txtLedValue.Location = New System.Drawing.Point(20, 35)
            Me.txtLedValue.Size = New System.Drawing.Size(140, 25)
            Me.txtLedValue.Text = ""
            '
            'btnWrite
            Me.btnWrite.Font = New System.Drawing.Font("新細明體", 11.0!)
            Me.btnWrite.Location = New System.Drawing.Point(175, 32)
            Me.btnWrite.Size = New System.Drawing.Size(65, 30)
            Me.btnWrite.Text = "Write"
            '
            'btnRead
            Me.btnRead.Font = New System.Drawing.Font("新細明體", 11.0!)
            Me.btnRead.Location = New System.Drawing.Point(250, 32)
            Me.btnRead.Size = New System.Drawing.Size(65, 30)
            Me.btnRead.Text = "Read"
            '
            'MainForm
            Me.AutoScaleDimensions = New System.Drawing.SizeF(6.0!, 12.0!)
            Me.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font
            Me.ClientSize = New System.Drawing.Size(750, 320)
            Me.Controls.Add(Me.lblCurrentTime)
            Me.Controls.Add(Me.btnTimeSync)
            Me.Controls.Add(Me.cmbMode)
            Me.Controls.Add(Me.grpCardStatus)
            Me.Controls.Add(Me.grpSystemInfo)
            Me.Controls.Add(Me.grpLedControl)
            Me.FormBorderStyle = System.Windows.Forms.FormBorderStyle.FixedDialog
            Me.MaximizeBox = False
            Me.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen
            Me.Text = "113學年度工業類學生技藝競賽檢修站第二站開發環境： XX"
            Me.grpCardStatus.ResumeLayout(False)
            Me.grpCardStatus.PerformLayout()
            Me.grpSystemInfo.ResumeLayout(False)
            Me.grpSystemInfo.PerformLayout()
            Me.grpLedControl.ResumeLayout(False)
            Me.grpLedControl.PerformLayout()
            Me.ResumeLayout(False)
            Me.PerformLayout()
        End Sub

    End Class
End Namespace
