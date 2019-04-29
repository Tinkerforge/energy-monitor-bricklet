/* energy-monitor-bricklet
 * Copyright (C) 2019 Olaf Lüke <olaf@tinkerforge.com>
 *
 * config_energy.h: Config for the ADC and plug inputs
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

#ifndef CONFIG_ENERGY_H
#define CONFIG_ENERGY_H

#include "xmc_gpio.h"
#include "xmc_vadc.h"

#define ENERGY_ADC_IRQ_NODE_ID        15
#define ENERGY_ADC_IRQ_PRIO           0
#define ENERGY_ADC_MASTER_GROUP       VADC_G0
#define ENERGY_ADC_MASTER_GROUP_INDEX 0
#define ENERGY_ADC_MASTER_SYNC_SLAVES 0b0010

#define ENERGY_ADC_SLAVE_GROUP        VADC_G1
#define ENERGY_ADC_SLAVE_GROUP_INDEX  1

#define ENERGY_A_ADC_PIN              P2_1
#define ENERGY_A_ADC_CHANNEL          0
#define ENERGY_A_ADC_RESULT_REG       10
#define ENERGY_A_ADC_ALIAS            XMC_VADC_CHANNEL_ALIAS_CH6
#define ENERGY_A_ADC_GROUP            VADC_G0
#define ENERGY_A_ADC_GROUP_INDEX      0

#define ENERGY_V_ADC_PIN              P2_7
#define ENERGY_V_ADC_CHANNEL          0
#define ENERGY_V_ADC_RESULT_REG       10
#define ENERGY_V_ADC_ALIAS            XMC_VADC_CHANNEL_ALIAS_CH1
#define ENERGY_V_ADC_GROUP            VADC_G1
#define ENERGY_V_ADC_GROUP_INDEX      1




#define ENERGY_V_PLUG_PIN             P0_5
#define ENERGY_A_PLUG_PIN             P0_7

#endif