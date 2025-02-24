/***************************************************************************//**
 * @file adc_demo.c
 * @brief Implementation of ADC Demo Driver.
 * @author RNechita (ramona.nechita@analog.com)
 ********************************************************************************
 * Copyright 2021(c) Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * - Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in
 * the documentation and/or other materials provided with the
 * distribution.
 * - Neither the name of Analog Devices, Inc. nor the names of its
 * contributors may be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * - The use of this software may or may not infringe the patent rights
 * of one or more patent holders. This license does not release you
 * from the requirement that you obtain separate licenses from these
 * patent holders to use this software.
 * - Use of the software either in source or binary form, must be run
 * on or directly connected to an Analog Devices Inc. component.
 *
 * THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, NON-INFRINGEMENT,
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL ANALOG DEVICES BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, INTELLECTUAL PROPERTY RIGHTS, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "adc_demo.h"
#include "no_os_error.h"
#include "no_os_util.h"

/* default sine lookup table to be used if ext_buff is not available */
const uint16_t sine_lut[128] = {
	0x000, 0x064, 0x0C8, 0x12C, 0x18F, 0x1F1, 0x252, 0x2B1,
	0x30F, 0x36B, 0x3C5, 0x41C, 0x471, 0x4C3, 0x512, 0x55F,
	0x5A7, 0x5ED, 0x62E, 0x66C, 0x6A6, 0x6DC, 0x70D, 0x73A,
	0x763, 0x787, 0x7A7, 0x7C2, 0x7D8, 0x7E9, 0x7F5, 0x7FD,
	0x7FF, 0x7FD, 0x7F5, 0x7E9, 0x7D8, 0x7C2, 0x7A7, 0x787,
	0x763, 0x73A, 0x70D, 0x6DC, 0x6A6, 0x66C, 0x62E, 0x5ED,
	0x5A7, 0x55F, 0x512, 0x4C3, 0x471, 0x41C, 0x3C5, 0x36B,
	0x30F, 0x2B1, 0x252, 0x1F1, 0x18F, 0x12C, 0xC8,  0x64,
	0x000, 0xF9B, 0xF37, 0xED3, 0xE70, 0xE0E, 0xDAD, 0xD4E,
	0xCF0, 0xC94, 0xC3A, 0xBE3, 0xB8E, 0xB3C, 0xAED, 0xAA0,
	0xA58, 0xA12, 0x9D1, 0x993, 0x959, 0x923, 0x8F2, 0x8C5,
	0x89C, 0x878, 0x858, 0x83D, 0x827, 0x816, 0x80A, 0x802,
	0x800, 0x802, 0x80A, 0x816, 0x827, 0x83D, 0x858, 0x878,
	0x89C, 0x8C5, 0x8F2, 0x923, 0x959, 0x993, 0x9D1, 0xA12,
	0xA58, 0xAA0, 0xAED, 0xB3C, 0xB8E, 0xBE3, 0xC3A, 0xC94,
	0xCF0, 0xD4E, 0xDAD, 0xE0E, 0xE70, 0xED3, 0xF37, 0xF9B
};

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

/**
 * @brief init function for the adc demo driver
 * @param desc - descriptor for the adc
 * @param param - initialization param for adc
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t adc_demo_init(struct adc_demo_desc **desc,
		      struct adc_demo_init_param *param)
{
	struct adc_demo_desc *adesc;
	adesc = (struct adc_demo_desc*)calloc(1, sizeof(*adesc));

	if(!adesc)
		return -ENOMEM;

	adesc->ext_buff = param->ext_buff;
	adesc->ext_buff_len = param->ext_buff_len;
	for(int i = 0; i < TOTAL_ADC_CHANNELS; i++)
		adesc->adc_ch_attr[i] = param->dev_ch_attr[i];
	adesc->adc_global_attr = param->dev_global_attr;
	*desc = adesc;

	return 0;
}

/**
 * @brief free allocated resources
 * @param desc - descriptor for the adc
 * @return 0 in case of success, -1 otherwise.
 */
int32_t adc_demo_remove(struct adc_demo_desc *desc)
{
	if(!desc)
		return -EINVAL;

	free(desc);

	return 0;
}

/**
 * @brief active adc channels
 * @param dev - descriptor for the adc
 * @param mask - active channels mask
 * @return 0 in case of success, -1 otherwise.
 */
int32_t update_adc_channels(void *dev, uint32_t mask)
{
	struct adc_demo_desc *desc;

	if(!dev)
		return -ENODEV;

	desc = dev;

	desc->active_ch = mask;
	/* If a real device. Here needs to be selected the channels to be read*/

	return 0;
}

/**
 * @brief close all channels
 * @param dev - physical instance of an adc device
 * @return 0 in case of success.
 */
int32_t close_adc_channels(void* dev)
{
	struct adc_demo_desc *desc;

	if(!dev)
		return -ENODEV;

	desc = dev;

	desc->active_ch = 0;

	return 0;
}

/**
 * @brief utility function for computing next upcoming channel
 * @param ch_mask - active channels .
 * @param last_idx -  previous index.
 * @param new_idx - upcoming channel index, return param.
 * @return 1 if there are more channels, 0 if done.
 */
static bool get_next_ch_idx(uint32_t ch_mask, uint32_t last_idx,
			    uint32_t *new_idx)
{
	last_idx++;
	ch_mask >>= last_idx;
	if (!ch_mask) {
		*new_idx = -1;
		return 0;
	}
	while (!(ch_mask & 1)) {
		last_idx++;
		ch_mask >>= 1;
	}
	*new_idx = last_idx;

	return 1;
}

/**
 * @brief function for reading samples
 * @param dev - physical instance of adc device
 * @param buff - buffer for reading samples
 * @param samples - number of samples to receive
 * @return the number of samples.
 */
int32_t adc_read_samples(void* dev, uint16_t* buff, uint32_t samples)
{
	struct adc_demo_desc *desc;
	uint32_t k = 0;
	uint32_t ch = -1;

	if(!dev)
		return -ENODEV;

	desc = dev;

	if(desc->ext_buff == NULL) {
		//default sine lookup table
		int offset_per_ch = NO_OS_ARRAY_SIZE(sine_lut) / TOTAL_ADC_CHANNELS;
		for(uint32_t i = 0; i < samples; i++) {
			while(get_next_ch_idx(desc->active_ch, ch, &ch))
				buff[k++] = sine_lut[(i + ch * offset_per_ch ) % NO_OS_ARRAY_SIZE(sine_lut)];
		}

		return samples;
	}

	for(uint32_t i = 0; i < samples; i++) {
		while(get_next_ch_idx(desc->active_ch, ch, &ch))
			buff[k++] = ((uint16_t (*)[desc->ext_buff_len])(desc->ext_buff))[ch]
				    [i % desc->ext_buff_len];
	}
	return samples;
}

/**
 * @brief read function for the adc demo driver
 * @param desc - descriptor for the adc
 * @param reg_index - the address at which we want to read
 * @param readval- the value read from register
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t adc_demo_reg_read(struct adc_demo_desc *desc, uint8_t reg_index,
			  uint8_t *readval)
{
	if(!desc || reg_index >= NO_OS_ARRAY_SIZE(desc->reg))
		return -EINVAL;

	*readval = desc->reg[reg_index];

	return 0;
}

/**
 * @brief write function for the adc demo driver
 * @param desc - descriptor for the adc
 * @param reg_index - the address at which we want to write
 * @param writeval - the value to be written
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t adc_demo_reg_write(struct adc_demo_desc *desc, uint8_t reg_index,
			   uint8_t writeval)
{

	if(!desc || reg_index >= NO_OS_ARRAY_SIZE(desc->reg))
		return -EINVAL;

	desc->reg[reg_index] = writeval;

	return 0;
}
