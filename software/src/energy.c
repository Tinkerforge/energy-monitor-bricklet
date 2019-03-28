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

#if 0
#define XMC_MATH_SIGNED_DIVISION                      ((uint32_t) 0 << MATH_DIVCON_USIGN_Pos)
/* Unsigned division is selected */
#define XMC_MATH_UNSIGNED_DIVISION                    ((uint32_t) 1 << MATH_DIVCON_USIGN_Pos)
		MATH->DIVCON  = XMC_MATH_SIGNED_DIVISION;
  		MATH->DVD     = mains_ma;
  		MATH->DVS     = 100;
  		energy.waveform_a[index] =  MATH->QUOT;

		MATH->DIVCON  = XMC_MATH_SIGNED_DIVISION;
  		MATH->DVD     = mains_mv;
  		MATH->DVS     = 100;
  		energy.waveform_v[index] =  MATH->QUOT;
#endif

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

void energy_write_calibration(uint16_t a, uint16_t v) {
	uint32_t page[EEPROM_PAGE_SIZE/sizeof(uint32_t)];
	page[ENERGY_CALIBRATION_MAGIC_POS] = ENERGY_CALIBRATION_MAGIC;
	page[ENERGY_CALIBRATION_A_POS]     = a;
	page[ENERGY_CALIBRATION_V_POS]     = v;

	if(!bootloader_write_eeprom_page(ENERGY_CALIBRATION_PAGE, page)) {
		// TODO: Error handling?
	}
	
	logd("Write calibration values: a %d, v %d\n\r", energy.a_mid, energy.v_mid);
}


void energy_read_calibration(void) {
	uint32_t page[EEPROM_PAGE_SIZE/sizeof(uint32_t)];

	bootloader_read_eeprom_page(ENERGY_CALIBRATION_PAGE, page);

	// The magic number is not where it is supposed to be.
	// This is either our first startup or something went wrong.
	// We initialize the calibration data with sane default values.
	if(page[ENERGY_CALIBRATION_MAGIC_POS] != ENERGY_CALIBRATION_MAGIC) {
		// TODO: Use sane default values
		energy.a_mid = 1; 
		energy.v_mid = 1;

		energy_write_calibration(1, 1);

		return;
	}

	energy.a_mid = page[ENERGY_CALIBRATION_A_POS];
	energy.v_mid = page[ENERGY_CALIBRATION_V_POS];

	logd("Read calibration values: a %d, v %d\n\r", energy.a_mid, energy.v_mid);
}

void energy_calibrate(void) {
	uint32_t a_sum = 0;
	uint32_t v_sum = 0;

	for(uint16_t i = 0; i < ENERGY_SAMPLES_NUM; i++) {
		const uint32_t sample = energy.samples[i];
		const uint16_t a = (sample >>  0) & 0xFFFF;
		const uint16_t v = (sample >> 16) & 0xFFFF;

		a_sum += a;
		v_sum += v;
	}

	energy.a_mid = a_sum / ENERGY_SAMPLES_NUM;
	energy.v_mid = v_sum / ENERGY_SAMPLES_NUM;

	energy_write_calibration(energy.a_mid, energy.v_mid);
}

void energy_tick(void) {
	static uint32_t t = 0;

	if(energy.calibrate) {
		energy.calibrate = false;
		energy_calibrate();
	}

	while(energy_samples_start_cur != energy_samples_end_cur) {
		const uint16_t index = energy_samples_start_cur - energy_samples_start;

		const uint32_t sample = *energy_samples_start_cur;
		const uint16_t a_adc_dc = (sample >>  0) & 0xFFFF;
		const uint16_t v_adc_dc = (sample >> 16) & 0xFFFF;

		const int32_t a_adc_ac = a_adc_dc - energy.a_mid;
		const int32_t v_adc_ac = v_adc_dc - energy.v_mid;

#if 0
		const int32_t a_voltage_ac = 3300*a_adc_ac/(4*4095);
		const int32_t v_voltage_ac = 3300*v_adc_ac/(4*4095);

		const int32_t a_input_voltage_ac = a_voltage_ac*68/91;    // 91k/68k
		const int32_t v_input_voltage_ac = v_voltage_ac*750/56;   // 5.6k/75k

		const int32_t mains_ma = a_input_voltage_ac*30;           // 1V = 30A, x windings => 1V = 30/xA
		const int32_t mains_mv = v_input_voltage_ac*18;           // 230/12.8 = 17.96875 ~ 18
#endif

		energy.waveform_a[index] = (3740*a_adc_ac)  / (8281);    // 1/100 A
		energy.waveform_v[index] = (12375*v_adc_ac) / (2548*10); // 1/10 V
		energy.waveform_last_index = index;

		if(energy_samples_start_cur == energy_samples_end) {
			energy_samples_start_cur = energy_samples_start;
		} else {
			energy_samples_start_cur++;
		}

		energy.adc_v_squared_sum += v_adc_ac*v_adc_ac;
		energy.adc_a_squared_sum += a_adc_ac*a_adc_ac;
		energy.adc_w_sum         += v_adc_ac*a_adc_ac;
		energy.sum_count++;

		energy.crossings_count++;
		// We assume that we cross the zero point if the last measrued value was negative and the new one is positive.
		// Additionally we wait for at least 50 measurements, to make sure that we don't have false-positives because
		// of noise in the data.
		if((energy.crossings_count > 50) && (energy.last_v_adc) < 0 && (v_adc_ac >= 0)) {
			energy.crossings_count = 0;
			energy.crossings++;
			energy.crossings_frequency++;
		}
		energy.last_v_adc = v_adc_ac;

		if(energy.crossings_frequency >= 300) {
			uint32_t new_time = system_timer_get_ms();
			if(energy.crossings_frequency_time != 0) {
				energy.frequency = 1000*100*300/((uint32_t)(new_time - energy.crossings_frequency_time));
			}

			energy.crossings_frequency_time = new_time;
			energy.crossings_frequency = 0;
		}

		if(energy.crossings >= 10) {
			// 330*750*18/(4*4095*56) (mV)  => 12375/2548
			energy.voltage = 12375*sqrt(energy.adc_v_squared_sum/energy.sum_count)/2548;
			// 330*68*30/(4*4095*91) (mA)   => 3740/8281
			energy.current = 3740*sqrt(energy.adc_a_squared_sum/energy.sum_count)/8281;
			// ((12375/2548) * (3740/8281)) / 100 => 462825/21099988
			energy.real_power = (((int64_t)462825)*energy.adc_w_sum)/(((int64_t)energy.sum_count)*((int64_t)21099988));

			// 0.01mV * 0.01mA = 0.0001mW => /100 = 0.1mW
			energy.apparent_power = (((int64_t)energy.voltage) * ((int64_t)energy.current)) / 100;
			energy.reactive_power = sqrt(((int64_t)energy.apparent_power)*((int64_t)energy.apparent_power) - ((int64_t)energy.real_power)*((int64_t)energy.real_power));
			energy.power_factor = (((int64_t)ABS(energy.real_power))*((int64_t)1000))/((int64_t)ABS(energy.apparent_power));

			if(energy.real_power < 0) {
				energy.apparent_power *= -1;
				energy.reactive_power *= -1;
				energy.current        *= -1;
			}

			uint32_t new_time  = system_timer_get_ms();
			uint32_t time_diff = ((uint32_t)(new_time - energy.wh_sum_last));
			energy.wh_sum_last = new_time;
			energy.wh_sum      = energy.wh_sum + ((int64_t)energy.real_power)*((int64_t)time_diff);
			energy.energy      = energy.wh_sum/(1000*60*60);

			energy.adc_v_squared_sum = 0;
			energy.adc_a_squared_sum = 0;
			energy.adc_w_sum = 0;
			energy.sum_count = 0;
			energy.crossings = 0;
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

	XMC_GPIO_CONFIG_t output = {
		.mode = XMC_GPIO_MODE_OUTPUT_PUSH_PULL,
		.output_level = XMC_GPIO_OUTPUT_LEVEL_HIGH
	};
	XMC_GPIO_Init(P1_1, &output);

	XMC_GPIO_Init(ENERGY_A_ADC_PIN, &input);
	XMC_GPIO_Init(ENERGY_V_ADC_PIN, &input);

	energy_read_calibration();

	energy_init_adc();
}