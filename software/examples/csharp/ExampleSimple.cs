using System;
using Tinkerforge;

class Example
{
	private static string HOST = "localhost";
	private static int PORT = 4223;
	private static string UID = "XYZ"; // Change XYZ to the UID of your Energy Monitor Bricklet

	static void Main()
	{
		IPConnection ipcon = new IPConnection(); // Create IP connection
		BrickletEnergyMonitor em = new BrickletEnergyMonitor(UID, ipcon); // Create device object

		ipcon.Connect(HOST, PORT); // Connect to brickd
		// Don't use device before ipcon is connected

		// Get current Energy Data
		int voltage, current, energy, realPower, apparentPower, reactivePower,
		    powerFactor, frequency;
		em.GetEnergyData(out voltage, out current, out energy, out realPower,
		                 out apparentPower, out reactivePower, out powerFactor,
		                 out frequency);

		Console.WriteLine("Voltage: " + voltage/100.0 + " V");
		Console.WriteLine("Current: " + current/100.0 + " A");
		Console.WriteLine("Energy: " + energy/100.0 + " Wh");
		Console.WriteLine("Real Power: " + realPower/100.0 + " h");
		Console.WriteLine("Apparent Power: " + apparentPower/100.0 + " VA");
		Console.WriteLine("Reactive Power: " + reactivePower/100.0 + " VAR");
		Console.WriteLine("Power Factor: " + powerFactor/1000.0);
		Console.WriteLine("Frequency: " + frequency/100.0 + " Hz");

		Console.WriteLine("Press enter to exit");
		Console.ReadLine();
		ipcon.Disconnect();
	}
}
