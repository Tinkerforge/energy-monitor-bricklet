import com.tinkerforge.IPConnection;
import com.tinkerforge.BrickletEnergyMonitor;

public class ExampleCallback {
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

		// Add energy data listener
		em.addEnergyDataListener(new BrickletEnergyMonitor.EnergyDataListener() {
			public void energyData(int voltage, int current, int energy, int realPower,
			                       int apparentPower, int reactivePower, int powerFactor,
			                       int frequency) {
				System.out.println("Voltage: " + voltage/100.0 + " V");
				System.out.println("Current: " + current/100.0 + " A");
				System.out.println("Energy: " + energy/100.0 + " Wh");
				System.out.println("Real Power: " + realPower/100.0 + " h");
				System.out.println("Apparent Power: " + apparentPower/100.0 + " VA");
				System.out.println("Reactive Power: " + reactivePower/100.0 + " var");
				System.out.println("Power Factor: " + powerFactor/1000.0);
				System.out.println("Frequency: " + frequency/100.0 + " Hz");
				System.out.println("");
			}
		});

		// Set period for energy data callback to 1s (1000ms)
		em.setEnergyDataCallbackConfiguration(1000, false);

		System.out.println("Press key to exit"); System.in.read();
		ipcon.disconnect();
	}
}
