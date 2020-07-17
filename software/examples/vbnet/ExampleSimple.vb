Imports System
Imports Tinkerforge

Module ExampleSimple
    Const HOST As String = "localhost"
    Const PORT As Integer = 4223
    Const UID As String = "XYZ" ' Change XYZ to the UID of your Energy Monitor Bricklet

    Sub Main()
        Dim ipcon As New IPConnection() ' Create IP connection
        Dim em As New BrickletEnergyMonitor(UID, ipcon) ' Create device object

        ipcon.Connect(HOST, PORT) ' Connect to brickd
        ' Don't use device before ipcon is connected

        ' Get current Energy Data
        Dim voltage, current, energy, realPower, apparentPower, reactivePower,
            powerFactor, frequency As Integer

        em.GetEnergyData(voltage, current, energy, realPower, apparentPower, _
                         reactivePower, powerFactor, frequency)

        Console.WriteLine("Voltage: " + (voltage/100.0).ToString() + " V")
        Console.WriteLine("Current: " + (current/100.0).ToString() + " A")
        Console.WriteLine("Energy: " + (energy/100.0).ToString() + " Wh")
        Console.WriteLine("Real Power: " + (realPower/100.0).ToString() + " h")
        Console.WriteLine("Apparent Power: " + (apparentPower/100.0).ToString() + " VA")
        Console.WriteLine("Reactive Power: " + (reactivePower/100.0).ToString() + " var")
        Console.WriteLine("Power Factor: " + (powerFactor/1000.0).ToString())
        Console.WriteLine("Frequency: " + (frequency/100.0).ToString() + " Hz")

        Console.WriteLine("Press key to exit")
        Console.ReadLine()
        ipcon.Disconnect()
    End Sub
End Module
