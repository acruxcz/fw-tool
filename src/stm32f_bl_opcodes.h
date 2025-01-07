/*
 * stm32f_bl_opcodes.h
 *
 *  Created on: Jul 9, 2019
 *      Author: petr
 */

#ifndef STM32F_BL_OPCODES_H_
#define STM32F_BL_OPCODES_H_


#define STM32BL_INIT	0x7f
#define	STM32BL_ACK		0x79
#define	STM32BL_NACK	0x1F

#define	STM32BL_GET 	0x00
#define	STM32BL_GETV	0x01
#define	STM32BL_GETID	0x02
#define	STM32BL_READM	0x11
#define	STM32BL_GO		0x21
#define	STM32BL_WRITEM	0x31
#define	STM32BL_ERASE	0x43
#define	STM32BL_EERASE	0x44
#define	STM32BL_WP		0x63
#define	STM32BL_UWP		0x73
#define	STM32BL_RP		0x82
#define	STM32BL_URP		0x92

#endif /* STM32F_BL_OPCODES_H_ */
