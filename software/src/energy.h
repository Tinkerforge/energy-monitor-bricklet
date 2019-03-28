/* energy-monitor-bricklet
 * Copyright (C) 2019 Olaf Lüke <olaf@tinkerforge.com>
 *
 * energy.h: ADC read-out and voltage/ampere/energy calculation
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

#ifndef ENERGY_H
#define ENERGY_H

#include <stdint.h>
#include <stdbool.h>

#define ENERGY_CALIBRATION_MAGIC 0x12345679
#define ENERGY_CALIBRATION_MAGIC_POS 0

#define ENERGY_OFFSET_CALIBRATION_PAGE 1
#define ENERGY_OFFSET_CALIBRATION_A_POS 1
#define ENERGY_OFFSET_CALIBRATION_V_POS 2

#define ENERGY_RATIO_CALIBRATION_PAGE 2
#define ENERGY_RATIO_CALIBRATION_A_POS 1
#define ENERGY_RATIO_CALIBRATION_V_POS 2

#define ENERGY_SAMPLES_NUM 1024
#define ENERGY_API_SAMPLES_NUM (1024 - 256)

typedef struct {
	uint32_t samples[ENERGY_SAMPLES_NUM]; // ampere | (voltage << 16)

	int16_t waveform_a[ENERGY_SAMPLES_NUM];
	int16_t waveform_v[ENERGY_SAMPLES_NUM];

	int16_t waveform_a_api[ENERGY_API_SAMPLES_NUM];
	int16_t waveform_v_api[ENERGY_API_SAMPLES_NUM];

	int32_t last_v_adc;
	int32_t last_a_adc;

	bool last_voltage_transformer_connected;
	bool last_current_transformer_connected;

	bool calibrate_offset_new;
	uint16_t offset_voltage;
	uint16_t offset_current;
	bool calibrate_ratio_new;
	uint16_t ratio_voltage;
	uint16_t ratio_current;

	uint32_t energy_cb_period;
	bool energy_cb_value_has_to_change;

	uint16_t waveform_last_index;
	uint16_t waveform_api_index;

	uint32_t crossings_frequency_time;
	uint32_t crossings_frequency;
	uint32_t crossings_count;
	uint32_t crossings;
	int64_t adc_a_squared_sum;
	int64_t adc_v_squared_sum;
	int64_t adc_w_sum;
	uint32_t sum_count;

	uint32_t wh_sum_last;
	uint32_t wh_sum_time;
	int64_t wh_sum;

	int32_t voltage;        // 10mV RMS
	int32_t current;        // 10mA RMS
	int32_t energy;         // 10mWh
	int32_t real_power;     // 10mW
	int32_t apparent_power; // 10mVA
	int32_t reactive_power; // 10mVAR
	uint16_t power_factor;  // 1/1000
	uint16_t frequency;     // 1/100Hz
} Energy;

extern Energy energy;

void energy_tick(void);
void energy_init(void);

#endif