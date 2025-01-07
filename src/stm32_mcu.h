/*
 * stm32_mcu.h
 *
 *  Created on: Jul 9, 2019
 *      Author: petr
 */

#ifndef STM32_MCU_H_
#define STM32_MCU_H_

#include <stdint.h>

typedef struct{
	uint16_t	id;
	char		*name;
	uint32_t	flash_start;
	uint32_t	flash_end;
	uint32_t	flash_size_reg;
	uint32_t	page_size;
	uint32_t	num_of_pages;
	uint32_t	flash_size;

}stm32_mcu_t;


stm32_mcu_t * stm32_get_config(uint16_t id);

#endif /* STM32_MCU_H_ */
