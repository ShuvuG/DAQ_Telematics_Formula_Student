Public Class Raspberry_PI_Configurator
  Private Sub Raspberry_PI_Configurator_Load(sender As Object, e As EventArgs) Handles MyBase.Load 
End Sub

Private Sub update_conf_Click(sender As Object, e As EventArgs) Handles update_conf.Click 'CC = Configure CANBus ("CC" + NodeIDCmbBox.SelectedItem + SerialBRCmbBox.SelectedItem +mCanBRCmbBox.SelectedItem + HrtMTCmbBox.SelectedItem +ErrorRTCmbBox.SelectedItem + IntrptPinCmbBox.SelectedItem + ChipSlectCmbBox.SelectedItem + NodeStautsCmbBox.SelectedItem + OpModeCmbBox.SelectedItem + FilterCmbBox.SelectedItem + BusSchCmbBox.SelectedItem + GlobalTimerCmbBox.SelectedItem + Environment.NewLine)'

If NodeIDCmbBox.SelectedItem = 1 Then
  SerialBR1.Text = SerialBRCmbBox.SelectedItem 
  CanBR1.Text = CanBRCmbBox.SelectedItem 
  HrtMT1.Text = HrtMTCmbBox.SelectedItem 
  ErrorRT1.Text = ErrorRTCmbBox.SelectedItem 
  IntrptPin1.Text = IntrptPinCmbBox.SelectedItem 
  ChipSlect1.Text = ChipSlectCmbBox.SelectedItem 
  NodeStauts1.Text = NodeStautsCmbBox.SelectedItem 
  OpMode1.Text = OpModeCmbBox.SelectedItem 
  Filter1.Text = FilterCmbBox.SelectedItem
  BusSch1.Text = BusSchCmbBox.SelectedItem 
  GlobalTimer1.Text = GlobalTimerCmbBox.SelectedItem
End If

If NodeIDCmbBox.SelectedItem = 2 Then
  SerialBR2.Text = SerialBRCmbBox.SelectedItem 
  CanBR2.Text = CanBRCmbBox.SelectedItem 
  HrtMT2.Text = HrtMTCmbBox.SelectedItem 
  ErrorRT2.Text = ErrorRTCmbBox.SelectedItem 
  IntrptPin2.Text = IntrptPinCmbBox.SelectedItem 
  ChipSlect2.Text = ChipSlectCmbBox.SelectedItem 
  NodeStauts2.Text = NodeStautsCmbBox.SelectedItem 
  OpMode2.Text = OpModeCmbBox.SelectedItem 
  Filter2.Text = FilterCmbBox.SelectedItem
  BusSch2.Text = BusSchCmbBox.SelectedItem 
  GlobalTimer2.Text = GlobalTimerCmbBox.SelectedItem
End If

If NodeIDCmbBox.SelectedItem = 3 Then
  SerialBR3.Text = SerialBRCmbBox.SelectedItem 
  CanBR3.Text = CanBRCmbBox.SelectedItem 
  HrtMT3.Text = HrtMTCmbBox.SelectedItem 
  ErrorRT3.Text = ErrorRTCmbBox.SelectedItem 
  IntrptPin3.Text = IntrptPinCmbBox.SelectedItem 
  ChipSlect3.Text = ChipSlectCmbBox.SelectedItem 
  NodeStauts3.Text = NodeStautsCmbBox.SelectedItem 
  OpMode3.Text = OpModeCmbBox.SelectedItem 
  Filter3.Text = FilterCmbBox.SelectedItem
  BusSch3.Text = BusSchCmbBox.SelectedItem 
  GlobalTimer3.Text = GlobalTimerCmbBox.SelectedItem
End If 
End Sub
End Class
