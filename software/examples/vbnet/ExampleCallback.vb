Imports System
Imports Tinkerforge

Module ExampleCallback
    Const HOST As String = "localhost"
    Const PORT As Integer = 4223
    Const UID As String = "XYZ" ' Change XYZ to the UID of your Energy Monitor Bricklet

    ' Callback subroutine for Energy Data callback
    Sub EnergyDataCB(ByVal sender As BrickletEnergyMonitor, ByVal voltage As Integer, _
                     ByVal current As Integer, ByVal energy As Integer, _
                     ByVal realPower As Integer, ByVal apparentPower As Integer, _
                     ByVal reactivePower As Integer, ByVal powerFactor As Integer, _
                     ByVal frequency As Integer)
        Console.WriteLine("Voltage: " + (voltage/100.0).ToString() + " V")
        Console.WriteLine("Current: " + (current/100.0).ToString() + " A")
        Console.WriteLine("Energy: " + (energy/100.0).ToString() + " Wh")
        Console.WriteLine("Real Power: " + (realPower/100.0).ToString() + " h")
        Console.WriteLine("Apparent Power: " + (apparentPower/100.0).ToString() + " VA")
        Console.WriteLine("Reactive Power: " + (reactivePower/100.0).ToString() + " VAR")
        Console.WriteLine("Power Factor: " + (powerFactor/1000.0).ToString())
        Console.WriteLine("Frequency: " + (frequency/100.0).ToString() + " Hz")
        Console.WriteLine("")
    End Sub

    Sub Main()
        Dim ipcon As New IPConnection() ' Create IP connection
        Dim em As New BrickletEnergyMonitor(UID, ipcon) ' Create device object

        ipcon.Connect(HOST, PORT) ' Connect to brickd
        ' Don't use device before ipcon is connected

        ' Register Energy Data callback to subroutine EnergyDataCB
        AddHandler em.EnergyDataCallback, AddressOf EnergyDataCB

        ' Set period for Energy Data callback to 1s (1000ms)
        em.SetEnergyDataCallbackConfiguration(1000, False)

        Console.WriteLine("Press key to exit")
        Console.ReadLine()
        ipcon.Disconnect()
    End Sub
End Module
