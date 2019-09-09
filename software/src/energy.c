/* energy-monitor-bricklet
 * Copyright (C) 2019 Olaf Lüke <olaf@tinkerforge.com>
 *
 * energy.c: ADC read-out and voltage/ampere/energy calculation
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include "energy.h"

#include "configs/config_energy.h"
#include "bricklib2/hal/system_timer/system_timer.h"
#include "bricklib2/bootloader/bootloader.h"
#include "bricklib2/logging/logging.h"
#include "bricklib2/utility/util_definitions.h"

#include "math.h"

#include "xmc_vadc.h"
#include "xmc_scu.h"
#include "xmc_wdt.h"
#include "xmc_math.h"

#define ENERGY_RATIO_CURRENT_DEFAULT 3000
#define ENERGY_RATIO_VOLTAGE_DEFAULT 1923

#define ENERGY_CROSSINGS_PER_CALCULATON 10
#define ENERGY_NO_WAVEFORM_THRESHOLD    3000

#define energy_adc_irq_handler IRQ_Hdlr_15

Energy energy;


uint32_t *const energy_samples_start = energy.samples;
uint32_t *const energy_samples_end   = energy.samples + ENERGY_SAMPLES_NUM - 1;
uint32_t *energy_samples_start_cur   = energy.samples;
uint32_t *energy_samples_end_cur     = energy.samples;

// 1 sample per 71.85us 
// Interrupt takes 0.874us (measured with logic analyzer)
void __attribute__((optimize("-O3"))) __attribute__ ((section (".ram_code"))) energy_adc_irq_handler(void) {
	const uint16_t v = XMC_VADC_GROUP_GetDetailedResult(ENERGY_V_ADC_GROUP, ENERGY_V_ADC_RESULT_REG);
	const uint16_t a = XMC_VADC_GROUP_GetDetailedResult(ENERGY_A_ADC_GROUP, ENERGY_A_ADC_RESULT_REG);

	if(energy_samples_end_cur == energy_samples_end) {
		energy_samples_end_cur = energy_samples_start;
	} else {
		energy_samples_end_cur++;
	}
	*energy_samples_end_cur = ((uint32_t)a) | (((uint32_t)v) << 16);
}

void energy_write_offset_calibration(uint16_t a, uint16_t v) {
	uint32_t page[EEPROM_PAGE_SIZE/sizeof(uint32_t)];
	page[ENERGY_CALIBRATION_MAGIC_POS]    = ENERGY_CALIBRATION_MAGIC;
	page[ENERGY_OFFSET_CALIBRATION_A_POS] = a;
	page[ENERGY_OFFSET_CALIBRATION_V_POS] = v;

	if(!bootloader_write_eeprom_page(ENERGY_OFFSET_CALIBRATION_PAGE, page)) {
		// TODO: Error handling?
	}
	
	logd("Write offset calibration values: a %d, v %d\n\r", a, v);
}


void energy_read_offset_calibration(void) {
	uint32_t page[EEPROM_PAGE_SIZE/sizeof(uint32_t)];

	bootloader_read_eeprom_page(ENERGY_OFFSET_CALIBRATION_PAGE, page);

	// The magic number is not where it is supposed to be.
	// This is either our first startup or something went wrong.
	// We initialize the calibration data with sane default values.
	if(page[ENERGY_CALIBRATION_MAGIC_POS] != ENERGY_CALIBRATION_MAGIC) {
		energy.offset_current = 8167; 
		energy.offset_voltage = 7962;

		energy_write_offset_calibration(energy.offset_current, energy.offset_voltage);

		return;
	}

	energy.offset_current = page[ENERGY_OFFSET_CALIBRATION_A_POS];
	energy.offset_voltage = page[ENERGY_OFFSET_CALIBRATION_V_POS];

	logd("Read offset calibration values: a %d, v %d\n\r", energy.offset_current, energy.offset_voltage);
}

void energy_calibrate_offset(void) {
	uint32_t a_sum = 0;
	uint32_t v_sum = 0;

	for(uint16_t i = 0; i < ENERGY_SAMPLES_NUM; i++) {
		const uint32_t sample = energy.samples[i];
		const uint16_t a = (sample >>  0) & 0xFFFF;
		const uint16_t v = (sample >> 16) & 0xFFFF;

		a_sum += a;
		v_sum += v;
	}

	energy.offset_current = a_sum / ENERGY_SAMPLES_NUM;
	energy.offset_voltage = v_sum / ENERGY_SAMPLES_NUM;

	energy_write_offset_calibration(energy.offset_current, energy.offset_voltage);
}


void energy_write_ratio_calibration(uint16_t a, uint16_t v) {
	uint32_t page[EEPROM_PAGE_SIZE/sizeof(uint32_t)];
	page[ENERGY_CALIBRATION_MAGIC_POS]    = ENERGY_CALIBRATION_MAGIC;
	page[ENERGY_RATIO_CALIBRATION_A_POS] = a;
	page[ENERGY_RATIO_CALIBRATION_V_POS] = v;

	if(!bootloader_write_eeprom_page(ENERGY_RATIO_CALIBRATION_PAGE, page)) {
		// TODO: Error handling?
	}
	
	logd("Write ratio calibration values: a %d, v %d\n\r", a, v);
}


void energy_read_ratio_calibration(void) {
	uint32_t page[EEPROM_PAGE_SIZE/sizeof(uint32_t)];

	bootloader_read_eeprom_page(ENERGY_RATIO_CALIBRATION_PAGE, page);

	// The magic number is not where it is supposed to be.
	// This is either our first startup or something went wrong.
	// We initialize the calibration data with sane default values.
	if(page[ENERGY_CALIBRATION_MAGIC_POS] != ENERGY_CALIBRATION_MAGIC) {
		energy.ratio_current = ENERGY_RATIO_CURRENT_DEFAULT;
		energy.ratio_voltage = ENERGY_RATIO_VOLTAGE_DEFAULT;

		energy_write_ratio_calibration(energy.ratio_current, energy.ratio_voltage);

		return;
	}

	energy.ratio_current = page[ENERGY_RATIO_CALIBRATION_A_POS];
	energy.ratio_voltage = page[ENERGY_RATIO_CALIBRATION_V_POS];

	logd("Read ratio calibration values: a %d, v %d\n\r", energy.ratio_current, energy.ratio_voltage);
}

void energy_tick_voltage_and_current(const int32_t v_adc_ac, const int32_t a_adc_ac) {
	// Integrate voltage/current squares (for rms calculation) and V*A for real power calculation
	energy.adc_v_squared_sum += v_adc_ac*v_adc_ac;
	energy.adc_a_squared_sum += a_adc_ac*a_adc_ac;
	energy.adc_w_sum         += v_adc_ac*a_adc_ac;
	energy.sum_count++;

	// We assume that we cross the zero point if the last measured value was negative and the new one is positive.
	// Additionally we wait for at least 50 measurements, to make sure that we don't have false-positives because
	// of noise in the data.
	energy.crossings_count++;
	if((energy.crossings_count > 50) && (energy.last_v_adc) < 0 && (v_adc_ac >= 0)) {
		energy.crossings_count = 0;
		energy.crossings++;
		energy.crossings_frequency++;
	}
	energy.last_v_adc = v_adc_ac;
	energy.last_a_adc = a_adc_ac;

	// Recalculate frequency every 300 crossings
	// This is every 6 seconds at 50Hz net frequency, which gives a resolution of 0.01Hz.
	if(energy.crossings_frequency >= 300) {
		const uint32_t new_time = system_timer_get_ms();
		if(energy.crossings_frequency_time != 0) {
			// Calculate frequency with 0.01Hz resolution
			const uint32_t diff = new_time - energy.crossings_frequency_time;
			if(diff != 0) {
				energy.frequency = 1000*100*300/diff;
			}
		}

		energy.crossings_frequency_time = new_time;
		energy.crossings_frequency = 0;
	}

	// We calculate new values every ENERGY_CROSSINGS_PER_CALCULATON crossings.
	// A crossing here is the low to high zero-crossing of the sinusoidal voltage
	if(energy.crossings >= ENERGY_CROSSINGS_PER_CALCULATON) {
		if(energy.sum_count == 0) {
			energy.voltage    = 0;
			energy.current    = 0;
			energy.real_power = 0;
		} else {
			// Calculate voltage RMS
			// 330*750*ratio_v/(4*4095*56*100) (mV) => ratio_v*55/20384
			energy.voltage    = ((int64_t)(energy.ratio_voltage*55))*((int64_t)sqrt(energy.adc_v_squared_sum/energy.sum_count))/((int64_t)20384);

			// Calculate current RMS
			// 330*68*ratio_a/(4*4095*91*100) (mA)  => ratio_a*187/1242150
			energy.current    = ((int64_t)(energy.ratio_current*187))*((int64_t)sqrt(energy.adc_a_squared_sum/energy.sum_count))/((int64_t)1242150);

			// Calculate real power
			// ((ratio_v*55/20384) * (ratio_a*187/1242150)) / 100  => ratio_v*ratio_a*2057/506399712000 ~ ratio_v*ratio_a/246423218
			energy.real_power = (((int64_t)(energy.ratio_voltage*energy.ratio_current))*energy.adc_w_sum)/(((int64_t)energy.sum_count)*((int64_t)246423218));
		}

		// Calculate apparent and reactive power from V/A RMS and real power
		energy.apparent_power = (((int64_t)energy.voltage) * ((int64_t)energy.current)) / 100; // 0.01mV * 0.01mA = 0.0001mW => /100 = 0.1mW
		int64_t power_square_diff = ((int64_t)energy.apparent_power)*((int64_t)energy.apparent_power) - ((int64_t)energy.real_power)*((int64_t)energy.real_power);
		if(power_square_diff > 0) {
			energy.reactive_power = sqrt(power_square_diff);
		} else {
			energy.reactive_power = 0;

		}

		// Calculate power factor (W/VA)
		if(energy.apparent_power == 0) {
			energy.power_factor = 0;
		} else {
			energy.power_factor = (((int64_t)ABS(energy.real_power))*((int64_t)1000))/((int64_t)ABS(energy.apparent_power));
		}

		// The real power is the only unit with the correct sign,
		// we adjust the current and the other powers accordingly.
		if(energy.real_power < 0) {
			energy.apparent_power *= -1;
			energy.reactive_power *= -1;
			energy.current        *= -1;
		}

		// Calculate Wh
		uint32_t new_time  = system_timer_get_ms();
		uint32_t time_diff = ((uint32_t)(new_time - energy.wh_sum_last));
		energy.wh_sum_last = new_time;
		// Internally we use Wms
		energy.wh_sum      = energy.wh_sum + ((int64_t)energy.real_power)*((int64_t)time_diff);
		// Externally for the API we calculate 0.01Wh
		energy.energy      = energy.wh_sum/(1000*60*60);

		// Set all sums back to 0 for next round of calculations
		energy.adc_v_squared_sum = 0;
		energy.adc_a_squared_sum = 0;
		energy.adc_w_sum = 0;
		energy.sum_count = 0;
		energy.crossings = 0;
	}

	// If we don't see any crossings for a long time (3000 crossings_count is about 1/4 of a second)
	// we assume that there is no current flowing at all and set everything to 0.
	if(energy.crossings_count > ENERGY_NO_WAVEFORM_THRESHOLD) {
		energy.voltage        = 0;
		energy.current        = 0;
		energy.real_power     = 0;
		energy.apparent_power = 0;
		energy.reactive_power = 0;
		energy.power_factor   = 0;
		energy.energy         = 0;
		energy.frequency      = 0;
	}
}

void energy_tick_voltage(const int32_t v_adc_ac) {
	// Integrate voltage squares (for rms calculation)
	energy.adc_v_squared_sum += v_adc_ac*v_adc_ac;
	energy.sum_count++;

	// We assume that we cross the zero point if the last measured value was negative and the new one is positive.
	// Additionally we wait for at least 50 measurements, to make sure that we don't have false-positives because
	// of noise in the data.
	energy.crossings_count++;
	if((energy.crossings_count > 50) && (energy.last_v_adc) < 0 && (v_adc_ac >= 0)) {
		energy.crossings_count = 0;
		energy.crossings++;
		energy.crossings_frequency++;
	}
	energy.last_v_adc = v_adc_ac;

	// Recalculate frequency every 300 crossings
	// This is every 6 seconds at 50Hz net frequency, which gives a resolution of 0.01Hz.
	if(energy.crossings_frequency >= 300) {
		const uint32_t new_time = system_timer_get_ms();
		if(energy.crossings_frequency_time != 0) {
			// Calculate frequency with 0.01Hz resolution
			const uint32_t diff = new_time - energy.crossings_frequency_time;
			if(diff != 0) {
				energy.frequency = 1000*100*300/diff;
			}
		}
		energy.crossings_frequency_time = new_time;
		energy.crossings_frequency = 0;
	}

	// We calculate new values every ENERGY_CROSSINGS_PER_CALCULATON crossings.
	// A crossing here is the low to high zero-crossing of the sinusoidal voltage
	if(energy.crossings >= ENERGY_CROSSINGS_PER_CALCULATON) {
		// Calculate voltage RMS
		// 330*750*ratio_v/(4*4095*56*100) (mV) => ratio_v*55/20384
		if(energy.sum_count == 0) {
			energy.voltage = 0;
		} else {
			energy.voltage = ((int64_t)(energy.ratio_voltage*55))*((int64_t)sqrt(energy.adc_v_squared_sum/energy.sum_count))/((int64_t)20384);
		}

		// We only have voltage here, we can't calculate any of the other values
		energy.current        = 0;
		energy.real_power     = 0;
		energy.apparent_power = 0;
		energy.reactive_power = 0;
		energy.power_factor   = 0;
		energy.energy         = 0;

		// Set all sums back to 0 for next round of calculations
		energy.adc_v_squared_sum = 0;
		energy.adc_a_squared_sum = 0;
		energy.adc_w_sum = 0;
		energy.sum_count = 0;
		energy.crossings = 0;
	}

	// If we don't see any crossings for a long time (3000 crossings_count is about 1/4 of a second)
	// we assume that there is no current flowing at all and set everything to 0.
	if(energy.crossings_count > ENERGY_NO_WAVEFORM_THRESHOLD) {
		energy.voltage   = 0;
		energy.frequency = 0;
	}
}

void energy_tick_current(const int32_t a_adc_ac) {
	// Integrate voltage/current squares (for rms calculation) and V*A for real power calculation
	energy.adc_a_squared_sum += a_adc_ac*a_adc_ac;
	energy.sum_count++;

	// We assume that we cross the zero point if the last measured value was negative and the new one is positive.
	// Additionally we wait for at least 50 measurements, to make sure that we don't have false-positives because
	// of noise in the data.
	energy.crossings_count++;
	if((energy.crossings_count > 50) && (energy.last_a_adc) < 0 && (a_adc_ac >= 0)) {
		energy.crossings_count = 0;
		energy.crossings++;
		energy.crossings_frequency++;
	}
	energy.last_a_adc = a_adc_ac;

	// Recalculate frequency every 300 crossings
	// This is every 6 seconds at 50Hz net frequency, which gives a resolution of 0.01Hz.
	if(energy.crossings_frequency >= 300) {
		const uint32_t new_time = system_timer_get_ms();
		if(energy.crossings_frequency_time != 0) {
			// Calculate frequency with 0.01Hz resolution
			const uint32_t diff = new_time - energy.crossings_frequency_time;
			if(diff != 0) {
				energy.frequency = 1000*100*300/diff;
			}
		}

		energy.crossings_frequency_time = new_time;
		energy.crossings_frequency = 0;
	}

	// We calculate new values every ENERGY_CROSSINGS_PER_CALCULATON crossings.
	// A crossing here is the low to high zero-crossing of the sinusoidal voltage
	if(energy.crossings >= ENERGY_CROSSINGS_PER_CALCULATON) {
		// Calculate current RMS
		// 330*68*ratio_a/(4*4095*91*100) (mA)  => ratio_a*187/1242150
		if(energy.sum_count == 0) {
			energy.current = 0;
		} else {
			energy.current = ((int64_t)(energy.ratio_current*187))*((int64_t)sqrt(energy.adc_a_squared_sum/energy.sum_count))/((int64_t)1242150);
		}

		// We only have current here, we can't calculate any of the other values
		energy.voltage        = 0;
		energy.real_power     = 0;
		energy.apparent_power = 0;
		energy.reactive_power = 0;
		energy.power_factor   = 0;
		energy.energy         = 0;

		// Set all sums back to 0 for next round of calculations
		energy.adc_v_squared_sum = 0;
		energy.adc_a_squared_sum = 0;
		energy.adc_w_sum = 0;
		energy.sum_count = 0;
		energy.crossings = 0;
	}

	// If we don't see any crossings for a long time (3000 crossings_count is about 1/4 of a second)
	// we assume that there is no current flowing at all and set everything to 0.
	if(energy.crossings_count > ENERGY_NO_WAVEFORM_THRESHOLD) {
		energy.current   = 0;
		energy.frequency = 0;
	}
}

void energy_tick(void) {
	if(energy.calibrate_offset_new) {
		energy.calibrate_offset_new = false;
		energy_calibrate_offset();
	}

	if(energy.calibrate_ratio_new) {
		energy.calibrate_ratio_new = false;
		energy_write_ratio_calibration(energy.ratio_current, energy.ratio_voltage);
	}

	while(energy_samples_start_cur != energy_samples_end_cur) {
		// Check if transformers are connected. 
		// If a plug is disconnected we wait for it to be connected for at least two loops in a row
		// to make sure that we don't have use the measured values during the plugging of the connector.
		const bool new_voltage_transformer_connected = XMC_GPIO_GetInput(ENERGY_V_PLUG_PIN);
		const bool new_current_transformer_connected = XMC_GPIO_GetInput(ENERGY_A_PLUG_PIN);
		const bool voltage_transformer_connected = new_voltage_transformer_connected && energy.last_voltage_transformer_connected;
		const bool current_transformer_connected = new_current_transformer_connected && energy.last_current_transformer_connected;
		energy.last_voltage_transformer_connected = new_voltage_transformer_connected;
		energy.last_current_transformer_connected = new_current_transformer_connected;

		const uint16_t index = energy_samples_start_cur - energy_samples_start;

		// Get oldest ADC sample and split it in voltage and current values
		const uint32_t sample = *energy_samples_start_cur;
		if(energy_samples_start_cur == energy_samples_end) {
			energy_samples_start_cur = energy_samples_start;
		} else {
			energy_samples_start_cur++;
		}

		const uint16_t a_adc_dc = (sample >>  0) & 0xFFFF;
		const uint16_t v_adc_dc = (sample >> 16) & 0xFFFF;

		// Remove offset from ADC value. Now ADC value 0 is the 0 value of the sinus.
		const int32_t a_adc_ac = a_adc_dc - energy.offset_current;
		const int32_t v_adc_ac = v_adc_dc - energy.offset_voltage;

		// Save 16bit value of the current for waveform drawing
		// ratio_a*187/1242150   ~ ratio_a/6643
		energy.waveform_a[index] = current_transformer_connected ? (energy.ratio_current*a_adc_ac) / 6643 : 0; // 1/100 A

		// Save 16bit value of the voltage for waveform drawing
		// ratio_v*55/(20384*10) ~ ratio_v/3706
		energy.waveform_v[index] = voltage_transformer_connected ? (energy.ratio_voltage*v_adc_ac) / 3706 : 0; // 1/10 V
		energy.waveform_last_index = index;

		if(voltage_transformer_connected && current_transformer_connected) {
			energy_tick_voltage_and_current(v_adc_ac, a_adc_ac);
		} else if(voltage_transformer_connected) {
			energy_tick_voltage(v_adc_ac);
		} else if(current_transformer_connected) {
			energy_tick_current(a_adc_ac);
		} else {
			// If no transformer is connected we set everything to 0.
			energy.voltage        = 0;
			energy.current        = 0;
			energy.real_power     = 0;
			energy.apparent_power = 0;
			energy.reactive_power = 0;
			energy.power_factor   = 0;
			energy.energy         = 0;
			energy.frequency      = 0;
		}
	}
}


void energy_init_adc(void) {
	// Check if the arbiter is already enabled. Before checking this ensures that clock and reset states are correct
	bool arbitration_status = false;
	bool clock_reset_check = !XMC_SCU_CLOCK_IsPeripheralClockGated(XMC_SCU_PERIPHERAL_CLOCK_VADC);
	if(clock_reset_check) {
		arbitration_status = XMC_VADC_GROUP_ScanIsArbitrationSlotEnabled(ENERGY_ADC_MASTER_GROUP);
	}

	// Global ADC initialization
	const XMC_VADC_GLOBAL_CONFIG_t adc_global_config = {
		.boundary0 = (uint32_t) 0, // Lower boundary value for Normal comparison mode
		.boundary1 = (uint32_t) 0, // Upper boundary value for Normal comparison mode

		.class0 = {
			.sample_time_std_conv     = 31,                      // The Sample time is (2*tadci)
			.conversion_mode_standard = XMC_VADC_CONVMODE_12BIT, // 12bit conversion Selected

		},
		.class1 = {
			.sample_time_std_conv     = 31,                      // The Sample time is (2*tadci)
			.conversion_mode_standard = XMC_VADC_CONVMODE_12BIT, // 12bit conversion Selected

		},

		.data_reduction_control         = 0b11, // Accumulate 4 result values
		.wait_for_read_mode             = 1, // GLOBRES Register will not be overwritten until the previous value is read
		.event_gen_enable               = 1, // Result Event from GLOBRES is disabled
		.disable_sleep_mode_control     = 0  // Sleep mode is enabled
	};
	XMC_VADC_GLOBAL_Init(VADC, &adc_global_config);

	const XMC_VADC_GROUP_CONFIG_t group_init_handle = {
		.emux_config = {
			.stce_usage                  = 0, 					           // Use STCE when the setting changes
			.emux_mode                   = XMC_VADC_GROUP_EMUXMODE_SWCTRL, // Mode for Emux conversion
			.emux_coding                 = XMC_VADC_GROUP_EMUXCODE_BINARY, // Channel progression - binary format
			.starting_external_channel   = 0,                              // Channel starts at 0 for EMUX
			.connected_channel           = 0                               // Channel connected to EMUX
		},
		.class0 = {
			.sample_time_std_conv        = 31,                             // The Sample time is (2*tadci)
			.conversion_mode_standard    = XMC_VADC_CONVMODE_12BIT,        // 12bit conversion Selected
			.sampling_phase_emux_channel = 0,                              // The Sample time is (2*tadci)
			.conversion_mode_emux        = XMC_VADC_CONVMODE_12BIT         // 12bit conversion Selected
		},
		.class1 = {
			.sample_time_std_conv        = 31,                             // The Sample time is (2*tadci)
			.conversion_mode_standard    = XMC_VADC_CONVMODE_12BIT,        // 12bit conversion Selected
			.sampling_phase_emux_channel = 0,                              // The Sample time is (2*tadci)
			.conversion_mode_emux        = XMC_VADC_CONVMODE_12BIT         // 12bit conversion Selected
		},

		.boundary0                       = 0,                              // Lower boundary value for Normal comparison mode
		.boundary1	                     = 0,                              // Upper boundary value for Normal comparison mode
		.arbitration_round_length        = 0,                              // 4 arbitration slots per round selected (tarb = 4*tadcd) */
		.arbiter_mode                    = XMC_VADC_GROUP_ARBMODE_ALWAYS,  // Determines when the arbiter should run.
	};

    XMC_VADC_GROUP_Init(VADC_G0, &group_init_handle);
    XMC_VADC_GROUP_Init(VADC_G1, &group_init_handle);
    XMC_VADC_GROUP_SetPowerMode(VADC_G0, XMC_VADC_GROUP_POWERMODE_NORMAL);
    XMC_VADC_GROUP_SetPowerMode(VADC_G1, XMC_VADC_GROUP_POWERMODE_NORMAL);
    XMC_VADC_GLOBAL_SHS_EnableAcceleratedMode(SHS0, ENERGY_A_ADC_GROUP_INDEX);
    XMC_VADC_GLOBAL_SHS_EnableAcceleratedMode(SHS0, ENERGY_V_ADC_GROUP_INDEX);
	XMC_VADC_GLOBAL_SHS_SetClockDivider(SHS0, 0);
    XMC_VADC_GLOBAL_SHS_SetAnalogReference(SHS0, XMC_VADC_GLOBAL_SHS_AREF_EXTERNAL_VDD_UPPER_RANGE);

	XMC_VADC_GLOBAL_StartupCalibration(VADC);


	const XMC_VADC_GROUP_CLASS_t iclass_config = {
		.sample_time_std_conv        = 31,
		.conversion_mode_standard    = XMC_VADC_CONVMODE_12BIT,
		.sampling_phase_emux_channel = 0,
		.conversion_mode_emux        = XMC_VADC_CONVMODE_12BIT
	};
	XMC_VADC_GROUP_InputClassInit(ENERGY_ADC_MASTER_GROUP, iclass_config, XMC_VADC_GROUP_CONV_STD, 0);


	// Initialization of scan request source
	const XMC_VADC_SCAN_CONFIG_t scan_config = {
		.conv_start_mode   = XMC_VADC_STARTMODE_WFS,       // Conversion start mode WFS/CIR/CNR
		.req_src_priority  = XMC_VADC_GROUP_RS_PRIORITY_0, // The scan request source priority
		.trigger_signal    = XMC_VADC_REQ_TR_A,            // If trigger needed the signal input
		.trigger_edge      = XMC_VADC_TRIGGER_EDGE_NONE,   // Trigger edge needed if trigger enabled
		.gate_signal       = XMC_VADC_REQ_GT_A,            // If gating needed the signal input
		.timer_mode        = 0,                            // Disabled equidistant sampling
		.external_trigger  = false,                        // External trigger Enabled/Disabled
		.req_src_interrupt = true,                         // Request source event Enabled/disabled
		.enable_auto_scan  = true,                         // Autoscan enabled/disabled
		.load_mode         = XMC_VADC_SCAN_LOAD_COMBINE,   // Response from SCAN when a Load event occours.
	};
	XMC_VADC_GROUP_ScanInit(ENERGY_ADC_MASTER_GROUP, &scan_config);

	// Configure the gating mode for Scan
	XMC_VADC_GROUP_ScanSetGatingMode(ENERGY_ADC_MASTER_GROUP, XMC_VADC_GATEMODE_IGNORE);


	// Disable the Arbitration if no other instance has enabled it
	if(!arbitration_status) {
		XMC_VADC_GROUP_ScanDisableArbitrationSlot(ENERGY_ADC_MASTER_GROUP);
	}


	// Configure the Sync conversion operation
	XMC_VADC_GROUP_SetPowerMode(ENERGY_ADC_MASTER_GROUP, XMC_VADC_GROUP_POWERMODE_OFF);
	XMC_VADC_GROUP_SetPowerMode(ENERGY_ADC_SLAVE_GROUP, XMC_VADC_GROUP_POWERMODE_OFF);

	XMC_VADC_GROUP_SetSyncSlave(ENERGY_ADC_SLAVE_GROUP, ENERGY_ADC_MASTER_GROUP_INDEX, ENERGY_ADC_SLAVE_GROUP_INDEX);
	XMC_VADC_GROUP_CheckSlaveReadiness(ENERGY_ADC_MASTER_GROUP, ENERGY_ADC_SLAVE_GROUP_INDEX);

	XMC_VADC_GROUP_SetSyncSlaveReadySignal(ENERGY_ADC_SLAVE_GROUP, ENERGY_ADC_SLAVE_GROUP_INDEX, ENERGY_ADC_MASTER_GROUP_INDEX);


	// Configure the iclass settings needed for the sync slaves
	XMC_VADC_GROUP_CLASS_t conv_class = XMC_VADC_GROUP_GetInputClass(ENERGY_ADC_MASTER_GROUP, 0);
	XMC_VADC_GLOBAL_CLASS_t conv_class_global;
	conv_class_global.globiclass = conv_class.g_iclass0;
	XMC_VADC_GLOBAL_InputClassInit(VADC, conv_class_global, XMC_VADC_GROUP_CONV_STD, 1);
	XMC_VADC_GROUP_SetSyncMaster(ENERGY_ADC_MASTER_GROUP);
	XMC_VADC_GROUP_SetPowerMode(ENERGY_ADC_MASTER_GROUP, XMC_VADC_GROUP_POWERMODE_NORMAL);


	// Channel and Result Config Master
	XMC_VADC_CHANNEL_CONFIG_t channel_config = {
		.input_class                = XMC_VADC_CHANNEL_CONV_GROUP_CLASS0,
		.lower_boundary_select      = XMC_VADC_CHANNEL_BOUNDARY_GROUP_BOUND0,
		.upper_boundary_select      = XMC_VADC_CHANNEL_BOUNDARY_GROUP_BOUND0,
		.event_gen_criteria         = XMC_VADC_CHANNEL_EVGEN_NEVER,
		.sync_conversion            = true,                         // Sync feature disabled
		.alternate_reference        = XMC_VADC_CHANNEL_REF_INTREF,  // Internal reference selected
		.result_reg_number          = ENERGY_A_ADC_RESULT_REG,      // GxRES[10] selected
		.use_global_result          = 0,                            // Use Group result register
		.result_alignment           = XMC_VADC_RESULT_ALIGN_RIGHT,  // Result alignment - Right Aligned
		.broken_wire_detect_channel = XMC_VADC_CHANNEL_BWDCH_VAGND, // No Broken wire mode select
		.broken_wire_detect         = false,                        // No Broken wire detection
		.bfl                        = 0,                            // No Boundary flag
		.channel_priority           = 1,                            // Highest Priority: 1 selected
		.alias_channel              = ENERGY_A_ADC_ALIAS            // Channel is Aliased
	};

	XMC_VADC_RESULT_CONFIG_t result_config = {
		.data_reduction_control  = 0b11,
		.post_processing_mode    = XMC_VADC_DMM_REDUCTION_MODE,
		.wait_for_read_mode      = true,
		.part_of_fifo            = false , // No FIFO
		.event_gen_enable        = true    // Result event Enabled
	};

	XMC_VADC_GROUP_ChannelInit(ENERGY_A_ADC_GROUP, 0, &channel_config);
	XMC_VADC_GROUP_ResultInit(ENERGY_A_ADC_GROUP, ENERGY_A_ADC_RESULT_REG, &result_config);

	// Channel and Result Config Slave
	channel_config.input_class       = XMC_VADC_CHANNEL_CONV_GLOBAL_CLASS1;
	channel_config.sync_conversion   = false;
	channel_config.result_reg_number = ENERGY_V_ADC_RESULT_REG;
	channel_config.alias_channel     = ENERGY_V_ADC_ALIAS;
	result_config.event_gen_enable   = false;

	XMC_VADC_GROUP_ChannelInit(ENERGY_V_ADC_GROUP, 0, &channel_config);
	XMC_VADC_GROUP_ResultInit(ENERGY_V_ADC_GROUP, ENERGY_V_ADC_RESULT_REG, &result_config);


	// Interrupt Configuration
	XMC_VADC_GROUP_SetResultInterruptNode(ENERGY_A_ADC_GROUP, ENERGY_A_ADC_RESULT_REG, XMC_VADC_SR_SHARED_SR0);
	NVIC_SetPriority(ENERGY_ADC_IRQ_NODE_ID, ENERGY_ADC_IRQ_PRIO);
	NVIC_EnableIRQ(ENERGY_ADC_IRQ_NODE_ID);

	// Load the queue/scan entries into the hardware 
	XMC_VADC_GROUP_ScanAddMultipleChannels(ENERGY_ADC_MASTER_GROUP, 1);

	// Start the arbiter of the ADC request source after the initialization.
	XMC_VADC_GROUP_ScanEnableArbitrationSlot(ENERGY_ADC_MASTER_GROUP);

	// Start conversion
	XMC_VADC_GROUP_ScanTriggerConversion(ENERGY_ADC_MASTER_GROUP);
}

void energy_init(void) {
	memset(&energy, 0, sizeof(Energy));
	energy.frequency = 5000; // Set frequency to 50Hz on startup, so it shows something reasonable during the first seconds.

	XMC_GPIO_CONFIG_t input = {
		.mode = XMC_GPIO_MODE_INPUT_TRISTATE,
		.input_hysteresis = XMC_GPIO_INPUT_HYSTERESIS_STANDARD
	};

	XMC_GPIO_CONFIG_t pullup = {
		.mode = XMC_GPIO_MODE_INPUT_PULL_UP,
		.input_hysteresis = XMC_GPIO_INPUT_HYSTERESIS_STANDARD
	};

	XMC_GPIO_Init(ENERGY_A_ADC_PIN, &input);
	XMC_GPIO_Init(ENERGY_V_ADC_PIN, &input);

	XMC_GPIO_Init(ENERGY_A_PLUG_PIN, &pullup);
	XMC_GPIO_Init(ENERGY_V_PLUG_PIN, &pullup);

	energy_read_offset_calibration();
	energy_read_ratio_calibration();

	energy_init_adc();
}