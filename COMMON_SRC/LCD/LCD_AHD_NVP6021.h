/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		LCD_AHD_NVP6021.h
	\brief		NVP6021 AHD encoder start-up Driver Header File
	\author		futao_jiang
	\version	1.1
	\date		2022/03/21
	\copyright	Copyright(C) 2022 SONiX Technology Co.,Ltd. All rights reserved.
*/
#ifndef __LCD_AHD_NVP6021_H
#define __LCD_AHD_NVP6021_H
//------------------------------------------------------------------------------
#include <stdint.h>

typedef enum {
	NVP6021_AHD_720P25		= 0,	//! 16bit data bus width (Not supported under LT9211+NVP6021 hardware environment)
	NVP6021_AHD_720P30		= 1,	//! 16bit data bus width
	NVP6021_AHD_720P50		= 2,	//! 16bit data bus width
	NVP6021_AHD_720P60		= 3,	//! 16bit data bus width
	NVP6021_AHD_1080P25		= 4,	//! 16bit data bus width
	NVP6021_AHD_1080P30		= 5,	//! 16bit data bus width
	NVP6021_AHD_FMT_MAX
} NVP6021_VDO_MODE_e;

typedef struct {
	uint16_t hs;
	uint16_t hbp;
	uint16_t hfp;
	uint16_t hact;
	uint16_t htotal;
	uint16_t vs;
	uint16_t vbp;
	uint16_t vfp;
	uint16_t vact;
	uint16_t vtotal;
	uint32_t pclk_kHz;
} Video_Timing_t;

void LCD_NVP6021_Init( void );
void AHD_NVP6021_Start( void );
void AHD_NVP6021_DacSuspend( void );
void AHD_NVP6021_DacResume( void );
void LCD_PixelPllSetting( void );

#endif //! #ifndef __LCD_AHD_NVP6021_H
