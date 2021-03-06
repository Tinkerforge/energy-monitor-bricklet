/* energy-monitor-bricklet
 * Copyright (C) 2019 Olaf Lüke <olaf@tinkerforge.com>
 *
 * communication.c: TFP protocol message handling
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

#include "communication.h"

#include "bricklib2/utility/communication_callback.h"
#include "bricklib2/utility/util_definitions.h"
#include "bricklib2/protocols/tfp/tfp.h"
#include "bricklib2/hal/system_timer/system_timer.h"

#include "xmc_gpio.h"

#include "energy.h"
#include "configs/config_energy.h"

BootloaderHandleMessageResponse handle_message(const void *message, void *response) {
	switch(tfp_get_fid_from_message(message)) {
		case FID_GET_ENERGY_DATA: return get_energy_data(message, response);
		case FID_RESET_ENERGY: return reset_energy(message);
		case FID_GET_WAVEFORM_LOW_LEVEL: return get_waveform_low_level(message, response);
		case FID_GET_TRANSFORMER_STATUS: return get_transformer_status(message, response);
		case FID_SET_TRANSFORMER_CALIBRATION: return set_transformer_calibration(message);
		case FID_GET_TRANSFORMER_CALIBRATION: return get_transformer_calibration(message, response);
		case FID_CALIBRATE_OFFSET: return calibrate_offset(message);
		case FID_SET_ENERGY_DATA_CALLBACK_CONFIGURATION: return set_energy_data_callback_configuration(message);
		case FID_GET_ENERGY_DATA_CALLBACK_CONFIGURATION: return get_energy_data_callback_configuration(message, response);
		default: return HANDLE_MESSAGE_RESPONSE_NOT_SUPPORTED;
	}
}


BootloaderHandleMessageResponse get_energy_data(const GetEnergyData *data, GetEnergyData_Response *response) {
	response->header.length  = sizeof(GetEnergyData_Response);
	response->voltage        = energy.voltage;
	response->current        = energy.current;
	response->energy         = energy.energy;
	response->real_power     = energy.real_power;
	response->apparent_power = energy.apparent_power;
	response->reactive_power = energy.reactive_power;
	response->power_factor   = energy.power_factor;
	response->frequency      = energy.frequency;

	return HANDLE_MESSAGE_RESPONSE_NEW_MESSAGE;
}

BootloaderHandleMessageResponse reset_energy(const ResetEnergy *data) {
	energy.energy = 0;
	energy.wh_sum = 0;

	return HANDLE_MESSAGE_RESPONSE_EMPTY;
}

BootloaderHandleMessageResponse get_waveform_low_level(const GetWaveformLowLevel *data, GetWaveformLowLevel_Response *response) {
	response->header.length         = sizeof(GetWaveformLowLevel_Response);
	response->waveform_chunk_offset = energy.waveform_api_index*2;

	if(energy.waveform_api_index == 0) {
		// Find zero-crossing
		int16_t before = 0;
		uint16_t start_index = energy.waveform_last_index + 1;
		uint16_t count;
		for(count = 0; count < 256; count++) {
			int16_t new = energy.waveform_v[start_index];
			if((before < 0) && (new >= 0)) {
				break;
			} else {
				before = new;
			}

			start_index++;
			if(start_index >= ENERGY_SAMPLES_NUM) {
				start_index = 0;
			}
		}

		if(count >= 255) {
			start_index = energy.waveform_last_index + 1;
		}

		// Transfer waveform starting from zero crossing
		uint16_t waveform_index = start_index;
		for(uint16_t api_index = 0; api_index < ENERGY_API_SAMPLES_NUM; api_index++) {
			energy.waveform_v_api[api_index] = energy.waveform_v[waveform_index];
			energy.waveform_a_api[api_index] = energy.waveform_a[waveform_index];
			waveform_index++;
			if(waveform_index >= ENERGY_SAMPLES_NUM) {
				waveform_index = 0;
			}
		}
	}

	const uint8_t length = MIN(15, ENERGY_API_SAMPLES_NUM - energy.waveform_api_index);
	for(uint8_t i = 0; i < length; i++) {
		response->waveform_chunk_data[i*2 + 0] = energy.waveform_v_api[energy.waveform_api_index + i];
		response->waveform_chunk_data[i*2 + 1] = energy.waveform_a_api[energy.waveform_api_index + i];
	}

	energy.waveform_api_index += length;

	if(energy.waveform_api_index >= ENERGY_API_SAMPLES_NUM) {
		energy.waveform_api_index = 0;
	}

	return HANDLE_MESSAGE_RESPONSE_NEW_MESSAGE;
}

BootloaderHandleMessageResponse get_transformer_status(const GetTransformerStatus *data, GetTransformerStatus_Response *response) {
	response->header.length                 = sizeof(GetTransformerStatus_Response);
	response->voltage_transformer_connected = XMC_GPIO_GetInput(ENERGY_V_PLUG_PIN);
	response->current_transformer_connected = XMC_GPIO_GetInput(ENERGY_A_PLUG_PIN);

	return HANDLE_MESSAGE_RESPONSE_NEW_MESSAGE;
}

BootloaderHandleMessageResponse set_transformer_calibration(const SetTransformerCalibration *data) {
	energy.ratio_voltage = data->voltage_ratio;
	energy.ratio_current = data->current_ratio;
	// We ignore phase shift for now.
	energy.calibrate_ratio_new = true;

	return HANDLE_MESSAGE_RESPONSE_EMPTY;
}

BootloaderHandleMessageResponse get_transformer_calibration(const GetTransformerCalibration *data, GetTransformerCalibration_Response *response) {
	response->header.length = sizeof(GetTransformerCalibration_Response);
	response->voltage_ratio = energy.ratio_voltage;
	response->current_ratio = energy.ratio_current;
	response->phase_shift   = 0;

	return HANDLE_MESSAGE_RESPONSE_NEW_MESSAGE;
}

BootloaderHandleMessageResponse calibrate_offset(const CalibrateOffset *data) {
	energy.calibrate_offset_new = true;

	return HANDLE_MESSAGE_RESPONSE_EMPTY;
}

BootloaderHandleMessageResponse set_energy_data_callback_configuration(const SetEnergyDataCallbackConfiguration *data) {
	energy.energy_cb_period              = data->period;
	energy.energy_cb_value_has_to_change = data->value_has_to_change;

	return HANDLE_MESSAGE_RESPONSE_EMPTY;
}

BootloaderHandleMessageResponse get_energy_data_callback_configuration(const GetEnergyDataCallbackConfiguration *data, GetEnergyDataCallbackConfiguration_Response *response) {
	response->header.length       = sizeof(GetEnergyDataCallbackConfiguration_Response);
	response->period              = energy.energy_cb_period;
	response->value_has_to_change = energy.energy_cb_value_has_to_change;

	return HANDLE_MESSAGE_RESPONSE_NEW_MESSAGE;
}


bool handle_energy_data_callback(void) {
	static bool is_buffered = false;
	static EnergyData_Callback cb;
	static int32_t  last_voltage        = 0;
	static int32_t  last_current        = 0;
	static int32_t  last_energy         = 0;
	static int32_t  last_real_power     = 0;
	static int32_t  last_apparent_power = 0;
	static int32_t  last_reactive_power = 0;
	static uint16_t last_power_factor   = 0;
	static uint16_t last_frequency      = 0;
	static uint32_t last_time           = 0;

	if(!is_buffered) {
		if(energy.energy_cb_period == 0 ||
		    !system_timer_is_time_elapsed_ms(last_time, energy.energy_cb_period)) {
			return false;
		}

		if(energy.energy_cb_value_has_to_change &&
		   (energy.voltage        == last_voltage) &&
		   (energy.current        == last_current) &&
		   (energy.energy         == last_energy) &&
		   (energy.real_power     == last_real_power) &&
		   (energy.apparent_power == last_apparent_power) &&
		   (energy.reactive_power == last_reactive_power) &&
		   (energy.power_factor   == last_power_factor) &&
		   (energy.frequency      == last_frequency)) {
			return false;
		}

		tfp_make_default_header(&cb.header, bootloader_get_uid(), sizeof(EnergyData_Callback), FID_CALLBACK_ENERGY_DATA);
		cb.voltage          = energy.voltage;
		cb.current          = energy.current;
		cb.energy           = energy.energy;
		cb.real_power       = energy.real_power;
		cb.apparent_power   = energy.apparent_power;
		cb.reactive_power   = energy.reactive_power;
		cb.power_factor     = energy.power_factor;
		cb.frequency        = energy.frequency;

		last_voltage        = cb.voltage;
		last_current        = cb.current;
		last_energy         = cb.energy;
		last_real_power     = cb.real_power;
		last_apparent_power = cb.apparent_power;
		last_reactive_power = cb.reactive_power;
		last_power_factor   = cb.power_factor;
		last_frequency      = cb.frequency;

		last_time           = system_timer_get_ms();
	}

	if(bootloader_spitfp_is_send_possible(&bootloader_status.st)) {
		bootloader_spitfp_send_ack_and_message(&bootloader_status, (uint8_t*)&cb, sizeof(EnergyData_Callback));
		is_buffered = false;
		return true;
	} else {
		is_buffered = true;
	}

	return false;
}

void communication_tick(void) {
	communication_callback_tick();
}

void communication_init(void) {
	communication_callback_init();
}
