package main

import (
	"fmt"
	"github.com/Tinkerforge/go-api-bindings/energy_monitor_bricklet"
	"github.com/Tinkerforge/go-api-bindings/ipconnection"
)

const ADDR string = "localhost:4223"
const UID string = "XYZ" // Change XYZ to the UID of your Energy Monitor Bricklet.

func main() {
	ipcon := ipconnection.New()
	defer ipcon.Close()
	em, _ := energy_monitor_bricklet.New(UID, &ipcon) // Create device object.

	ipcon.Connect(ADDR) // Connect to brickd.
	defer ipcon.Disconnect()
	// Don't use device before ipcon is connected.

	// Get current Energy Data.
	voltage, current, energy, realPower, apparentPower, reactivePower, powerFactor, frequecy, _ := em.GetEnergyData()

	fmt.Printf("Voltage: %f V\n", float64(voltage)/100.0)
	fmt.Printf("Current: %f A\n", float64(current)/100.0)
	fmt.Printf("Energy: %f Wh\n", float64(energy)/100.0)
	fmt.Printf("Real Power: %f h\n", float64(realPower)/100.0)
	fmt.Printf("Apparent Power: %f VA\n", float64(apparentPower)/100.0)
	fmt.Printf("Reactive Power: %f VAR\n", float64(reactivePower)/100.0)
	fmt.Printf("Power Factor: %f\n", float64(powerFactor)/1000.0)
	fmt.Printf("Frequency: %f Hz\n", float64(frequecy)/100.0)

	fmt.Print("Press enter to exit.")
	fmt.Scanln()
}
