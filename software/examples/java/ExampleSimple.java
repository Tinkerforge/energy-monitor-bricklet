import com.tinkerforge.IPConnection;
import com.tinkerforge.BrickletEnergyMonitor;
import com.tinkerforge.BrickletEnergyMonitor.EnergyData;

public class ExampleSimple {
	private static final String HOST = "localhost";
	private static final int PORT = 4223;

	// Change XYZ to the UID of your Energy Monitor Bricklet
	private static final String UID = "XYZ";

	// Note: To make the example code cleaner we do not handle exceptions. Exceptions
	//       you might normally want to catch are described in the documentation
	public static void main(String args[]) throws Exception {
		IPConnection ipcon = new IPConnection(); // Create IP connection
		BrickletEnergyMonitor em = new BrickletEnergyMonitor(UID, ipcon); // Create device object

		ipcon.connect(HOST, PORT); // Connect to brickd
		// Don't use device before ipcon is connected

		// Get current energy data
		EnergyData energyData = em.getEnergyData(); // Can throw com.tinkerforge.TimeoutException

		System.out.println("Voltage: " + energyData.voltage/100.0 + " V");
		System.out.println("Current: " + energyData.current/100.0 + " A");
		System.out.println("Energy: " + energyData.energy/100.0 + " Wh");
		System.out.println("Real Power: " + energyData.realPower/100.0 + " h");
		System.out.println("Apparent Power: " + energyData.apparentPower/100.0 + " VA");
		System.out.println("Reactive Power: " + energyData.reactivePower/100.0 + " var");
		System.out.println("Power Factor: " + energyData.powerFactor/1000.0);
		System.out.println("Frequency: " + energyData.frequency/100.0 + " Hz");

		System.out.println("Press key to exit"); System.in.read();
		ipcon.disconnect();
	}
}
