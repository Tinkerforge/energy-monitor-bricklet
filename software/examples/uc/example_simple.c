// This example is not self-contained.
// It requres usage of the example driver specific to your platform.
// See the HAL documentation.

#include "bindings/hal_common.h"
#include "bindings/bricklet_energy_monitor.h"

#define UID "XYZ" // Change XYZ to the UID of your Energy Monitor Bricklet

void check(int rc, const char* msg);

void example_setup(TF_HalContext *hal);
void example_loop(TF_HalContext *hal);


static TF_EnergyMonitor em;

void example_setup(TF_HalContext *hal) {
	// Create device object
	check(tf_energy_monitor_create(&em, UID, hal), "create device object");

	// Get current energy data
	int32_t voltage, current, energy, real_power, apparent_power, reactive_power;
	uint16_t power_factor, frequency;
	check(tf_energy_monitor_get_energy_data(&em, &voltage, &current, &energy, &real_power,
	                                        &apparent_power, &reactive_power,
	                                        &power_factor,
	                                        &frequency), "get energy data");

	tf_hal_printf("Voltage: %d 1/%d V\n", voltage, 100);
	tf_hal_printf("Current: %d 1/%d A\n", current, 100);
	tf_hal_printf("Energy: %d 1/%d Wh\n", energy, 100);
	tf_hal_printf("Real Power: %d 1/%d h\n", real_power, 100);
	tf_hal_printf("Apparent Power: %d 1/%d VA\n", apparent_power, 100);
	tf_hal_printf("Reactive Power: %d 1/%d var\n", reactive_power, 100);
	tf_hal_printf("Power Factor: %d 1/%d\n", power_factor, 1000);
	tf_hal_printf("Frequency: %d 1/%d Hz\n", frequency, 100);
}

void example_loop(TF_HalContext *hal) {
	// Poll for callbacks
	tf_hal_callback_tick(hal, 0);
}
