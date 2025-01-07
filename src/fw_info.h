/*
 * fw_info.h
 *
 *  Created on: Jul 10, 2019
 *      Author: petr
 */

#ifndef FW_INFO_H_
#define FW_INFO_H_

#include "stdint.h"

#define FW_HEADER_MAGIC 	0xAE51F84D
#define MFG_ID_CLEARTEX		0x0
#define MFG_ID_METRALIGTH	0x1
#define MFG_ID_PPA			0x2
#define MFG_ID_TRONEX		0x3

#define MFG_PRODUCT_ID_

#define	FW_PLATFOM_STM32		0x10
#define	FW_PLATFOM_HT32			0x11
#define	FW_PLATFOM_ZYNQ			0x11
#define	FW_PLATFOM_ZYNQ_MPSOC	0x12

typedef struct __attribute__((packed))
{
	uint32_t	MAGIC;				//4B
	uint8_t		MFG_ID;				//5B
	uint8_t		FW_MAJOR_VER;		//6B
	uint8_t		FW_MINOR_VER;		//7B
	uint8_t		PATLORM;			//8B
	uint32_t	PRODUCT_ID;			//12B
	uint16_t	BUILD_NUMBER;		//14B
	uint8_t		resv0[14];			//28B
	uint8_t		FW_STRING[32];		//60B
	uint32_t	CRC;				//64B
}FW_INFO_t;



#endif /* FW_INFO_H_ */
