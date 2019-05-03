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

	em.RegisterEnergyDataCallback(func(voltage int32, current int32, energy int32, realPower int32, apparentPower int32, reactivePower int32, powerFactor uint16, frequecy uint16) {
		fmt.Printf("Voltage: %f V\n", float64(voltage)/100.0)
		fmt.Printf("Current: %f A\n", float64(current)/100.0)
		fmt.Printf("Energy: %f Wh\n", float64(energy)/100.0)
		fmt.Printf("Real Power: %f h\n", float64(realPower)/100.0)
		fmt.Printf("Apparent Power: %f VA\n", float64(apparentPower)/100.0)
		fmt.Printf("Reactive Power: %f VAR\n", float64(reactivePower)/100.0)
		fmt.Printf("Power Factor: %f\n", float64(powerFactor)/1000.0)
		fmt.Printf("Frequency: %f Hz\n", float64(frequecy)/100.0)
		fmt.Println()
	})

	// Set period for Energy Data callback to 1s (1000ms).
	em.SetEnergyDataCallbackConfiguration(1000, false)

	fmt.Print("Press enter to exit.")
	fmt.Scanln()
}
