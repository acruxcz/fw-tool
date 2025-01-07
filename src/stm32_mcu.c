/*
 * stm32_mcu.c
 *
 *  Created on: Jul 9, 2019
 *      Author: petr
 */

#include <stdio.h>
#include <stdlib.h>
#include "stm32_mcu.h"

stm32_mcu_t mcu_table[] =
{
		{	.id = 0x444, .name = "STM32F03xx4/6", .flash_start =  0x08000000, .flash_end = 0x08008000, .page_size = 1024, .num_of_pages = 64, .flash_size_reg = 0x1FFFF7CC }
};

#define table_size sizeof(mcu_table) / sizeof(stm32_mcu_t)

stm32_mcu_t * stm32_get_config(uint16_t id)
{
	int i;
	for(i=0;i< table_size;i++)
	{
		if(id == mcu_table[i].id) return &mcu_table[i];
	}
	return NULL;
}
