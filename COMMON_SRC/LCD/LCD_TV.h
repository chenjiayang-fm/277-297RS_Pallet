/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		LCD_TV.h
	\brief		LCD TV Funcation Header
	\author		Pierce
	\version	0.2
	\date		2020/06/01
	\copyright	Copyright(C) 2020 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _LCD_TV_H_
#define _LCD_TV_H_
#include <stdint.h>
#include "_510PF.h"
//------------------------------------------------------------------------------	
/*!
	\brief 		TV Output Initial Function
	\param[in]	ubMode is the TV output type (NTSC / NTSC_443 / PAL / PAL_M)
	\param[in]	ubProg is the TV output type (INTERLACE / PROGRESSIVE)
					  
	\par [Example]
	\code    	
		  LCD_TV_Init(NTSC, INTERLACE);
	\endcode
*/
void LCD_TV_Init (uint8_t ubMode, uint8_t ubProg);
#endif
