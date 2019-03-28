/* energy-monitor-bricklet
 * Copyright (C) 2019 Olaf Lüke <olaf@tinkerforge.com>
 *
 * communication.h: TFP protocol message handling
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

#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <stdint.h>
#include <stdbool.h>

#include "bricklib2/protocols/tfp/tfp.h"
#include "bricklib2/bootloader/bootloader.h"

// Default functions
BootloaderHandleMessageResponse handle_message(const void *data, void *response);
void communication_tick(void);
void communication_init(void);

// Constants
#define ENERGY_MONITOR_BOOTLOADER_MODE_BOOTLOADER 0
#define ENERGY_MONITOR_BOOTLOADER_MODE_FIRMWARE 1
#define ENERGY_MONITOR_BOOTLOADER_MODE_BOOTLOADER_WAIT_FOR_REBOOT 2
#define ENERGY_MONITOR_BOOTLOADER_MODE_FIRMWARE_WAIT_FOR_REBOOT 3
#define ENERGY_MONITOR_BOOTLOADER_MODE_FIRMWARE_WAIT_FOR_ERASE_AND_REBOOT 4

#define ENERGY_MONITOR_BOOTLOADER_STATUS_OK 0
#define ENERGY_MONITOR_BOOTLOADER_STATUS_INVALID_MODE 1
#define ENERGY_MONITOR_BOOTLOADER_STATUS_NO_CHANGE 2
#define ENERGY_MONITOR_BOOTLOADER_STATUS_ENTRY_FUNCTION_NOT_PRESENT 3
#define ENERGY_MONITOR_BOOTLOADER_STATUS_DEVICE_IDENTIFIER_INCORRECT 4
#define ENERGY_MONITOR_BOOTLOADER_STATUS_CRC_MISMATCH 5

#define ENERGY_MONITOR_STATUS_LED_CONFIG_OFF 0
#define ENERGY_MONITOR_STATUS_LED_CONFIG_ON 1
#define ENERGY_MONITOR_STATUS_LED_CONFIG_SHOW_HEARTBEAT 2
#define ENERGY_MONITOR_STATUS_LED_CONFIG_SHOW_STATUS 3

// Function and callback IDs and structs
#define FID_GET_ENERGY_DATA 1
#define FID_RESET_ENERGY 2
#define FID_GET_WAVEFORM_LOW_LEVEL 3
#define FID_SET_TRANSFORMER_CALIBRATION 4
#define FID_GET_TRANSFORMER_CALIBRATION 5
#define FID_CALIBRATE_OFFSET 6
#define FID_SET_ENERGY_DATA_CALLBACK_CONFIGURATION 7
#define FID_GET_ENERGY_DATA_CALLBACK_CONFIGURATION 8

#define FID_CALLBACK_ENERGY_DATA 9

typedef struct {
	TFPMessageHeader header;
} __attribute__((__packed__)) GetEnergyData;

typedef struct {
	TFPMessageHeader header;
	int32_t voltage;
	int32_t current;
	int32_t energy;
	int32_t real_power;
	int32_t apparent_power;
	int32_t reactive_power;
	uint16_t power_factor;
	uint16_t frequency;
} __attribute__((__packed__)) GetEnergyData_Response;

typedef struct {
	TFPMessageHeader header;
} __attribute__((__packed__)) ResetEnergy;

typedef struct {
	TFPMessageHeader header;
} __attribute__((__packed__)) GetWaveformLowLevel;

typedef struct {
	TFPMessageHeader header;
	uint16_t waveform_chunk_offset;
	int16_t waveform_chunk_data[30];
} __attribute__((__packed__)) GetWaveformLowLevel_Response;

typedef struct {
	TFPMessageHeader header;
	uint16_t voltage_ratio;
	uint16_t current_ratio;
	int16_t phase_shift;
} __attribute__((__packed__)) SetTransformerCalibration;

typedef struct {
	TFPMessageHeader header;
} __attribute__((__packed__)) GetTransformerCalibration;

typedef struct {
	TFPMessageHeader header;
	uint16_t voltage_ratio;
	uint16_t current_ratio;
	int16_t phase_shift;
} __attribute__((__packed__)) GetTransformerCalibration_Response;

typedef struct {
	TFPMessageHeader header;
} __attribute__((__packed__)) CalibrateOffset;

typedef struct {
	TFPMessageHeader header;
	uint32_t period;
	bool value_has_to_change;
} __attribute__((__packed__)) SetEnergyDataCallbackConfiguration;

typedef struct {
	TFPMessageHeader header;
} __attribute__((__packed__)) GetEnergyDataCallbackConfiguration;

typedef struct {
	TFPMessageHeader header;
	uint32_t period;
	bool value_has_to_change;
} __attribute__((__packed__)) GetEnergyDataCallbackConfiguration_Response;

typedef struct {
	TFPMessageHeader header;
	int32_t voltage;
	int32_t current;
	int32_t energy;
	int32_t real_power;
	int32_t apparent_power;
	int32_t reactive_power;
	uint16_t power_factor;
	uint16_t frequency;
} __attribute__((__packed__)) EnergyData_Callback;


// Function prototypes
BootloaderHandleMessageResponse get_energy_data(const GetEnergyData *data, GetEnergyData_Response *response);
BootloaderHandleMessageResponse reset_energy(const ResetEnergy *data);
BootloaderHandleMessageResponse get_waveform_low_level(const GetWaveformLowLevel *data, GetWaveformLowLevel_Response *response);
BootloaderHandleMessageResponse set_transformer_calibration(const SetTransformerCalibration *data);
BootloaderHandleMessageResponse get_transformer_calibration(const GetTransformerCalibration *data, GetTransformerCalibration_Response *response);
BootloaderHandleMessageResponse calibrate_offset(const CalibrateOffset *data);
BootloaderHandleMessageResponse set_energy_data_callback_configuration(const SetEnergyDataCallbackConfiguration *data);
BootloaderHandleMessageResponse get_energy_data_callback_configuration(const GetEnergyDataCallbackConfiguration *data, GetEnergyDataCallbackConfiguration_Response *response);

// Callbacks
bool handle_energy_data_callback(void);

#define COMMUNICATION_CALLBACK_TICK_WAIT_MS 1
#define COMMUNICATION_CALLBACK_HANDLER_NUM 1
#define COMMUNICATION_CALLBACK_LIST_INIT \
	handle_energy_data_callback, \


#endif
