// This example is not self-contained.
// It requres usage of the example driver specific to your platform.
// See the HAL documentation.

#include "bindings/hal_common.h"
#include "bindings/bricklet_energy_monitor.h"

#define UID "XYZ" // Change XYZ to the UID of your Energy Monitor Bricklet

void check(int rc, const char* msg);

void example_setup(TF_HAL *hal);
void example_loop(TF_HAL *hal);


// Callback function for energy data callback
static void energy_data_handler(TF_EnergyMonitor *device, int32_t voltage,
                                int32_t current, int32_t energy, int32_t real_power,
                                int32_t apparent_power, int32_t reactive_power,
                                uint16_t power_factor, uint16_t frequency,
                                void *user_data) {
	(void)device; (void)user_data; // avoid unused parameter warning

	tf_hal_printf("Voltage: %d 1/%d V\n", voltage, 100);
	tf_hal_printf("Current: %d 1/%d A\n", current, 100);
	tf_hal_printf("Energy: %d 1/%d Wh\n", energy, 100);
	tf_hal_printf("Real Power: %d 1/%d h\n", real_power, 100);
	tf_hal_printf("Apparent Power: %d 1/%d VA\n", apparent_power, 100);
	tf_hal_printf("Reactive Power: %d 1/%d var\n", reactive_power, 100);
	tf_hal_printf("Power Factor: %d 1/%d\n", power_factor, 1000);
	tf_hal_printf("Frequency: %d 1/%d Hz\n", frequency, 100);
	tf_hal_printf("\n");
}

static TF_EnergyMonitor em;

void example_setup(TF_HAL *hal) {
	// Create device object
	check(tf_energy_monitor_create(&em, UID, hal), "create device object");

	// Register energy data callback to function energy_data_handler
	tf_energy_monitor_register_energy_data_callback(&em,
	                                                energy_data_handler,
	                                                NULL);

	// Set period for energy data callback to 1s (1000ms)
	tf_energy_monitor_set_energy_data_callback_configuration(&em, 1000, false);
}

void example_loop(TF_HAL *hal) {
	// Poll for callbacks
	tf_hal_callback_tick(hal, 0);
}
