using System;
using Tinkerforge;

class Example
{
	private static string HOST = "localhost";
	private static int PORT = 4223;
	private static string UID = "XYZ"; // Change XYZ to the UID of your Energy Monitor Bricklet

	// Callback function for energy data callback
	static void EnergyDataCB(BrickletEnergyMonitor sender, int voltage, int current,
	                         int energy, int realPower, int apparentPower,
	                         int reactivePower, int powerFactor, int frequency)
	{
		Console.WriteLine("Voltage: " + voltage/100.0 + " V");
		Console.WriteLine("Current: " + current/100.0 + " A");
		Console.WriteLine("Energy: " + energy/100.0 + " Wh");
		Console.WriteLine("Real Power: " + realPower/100.0 + " h");
		Console.WriteLine("Apparent Power: " + apparentPower/100.0 + " VA");
		Console.WriteLine("Reactive Power: " + reactivePower/100.0 + " var");
		Console.WriteLine("Power Factor: " + powerFactor/1000.0);
		Console.WriteLine("Frequency: " + frequency/100.0 + " Hz");
		Console.WriteLine("");
	}

	static void Main()
	{
		IPConnection ipcon = new IPConnection(); // Create IP connection
		BrickletEnergyMonitor em = new BrickletEnergyMonitor(UID, ipcon); // Create device object

		ipcon.Connect(HOST, PORT); // Connect to brickd
		// Don't use device before ipcon is connected

		// Register energy data callback to function EnergyDataCB
		em.EnergyDataCallback += EnergyDataCB;

		// Set period for energy data callback to 1s (1000ms)
		em.SetEnergyDataCallbackConfiguration(1000, false);

		Console.WriteLine("Press enter to exit");
		Console.ReadLine();
		ipcon.Disconnect();
	}
}
