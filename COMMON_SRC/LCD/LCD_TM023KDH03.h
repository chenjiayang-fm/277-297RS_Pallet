/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		LCD_TM023KDH03.h
	\brief		LCD TM023KDH03 Header
	\author		Pierce
	\version	0.2
	\date		2020/06/01
	\copyright	Copyright(C) 2020 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _LCD_TM023KDH03_H_
#define _LCD_TM023KDH03_H_
#include "_510PF.h"
//------------------------------------------------------------------------------
//! The command pins define for TM023KDH03
#define TM023KDH03_RSTIO			GPIO->GPIO_OE0
#define TM023KDH03_CSIO				GPIO->GPIO_OE1
#define TM023KDH03_RSTO				GPIO->GPIO_O0
#define TM023KDH03_CSO				GPIO->GPIO_O1
//! The macro define for TM023KDH03
#define TM023KDH03_CMD(Cmd)			LCD->LCD_CMD = Cmd;		
#define TM023KDH03_DATA(Data)		LCD->LCD_DATA = Data;
//------------------------------------------------------------------------------
/*!
	\brief 		LCD TM023KDH03 Initial Function	
	\param[in]	ubValue is the setting value for TM023KDH03
	\par [Example]
	\code    
		 LCD_TM024HDH03_Init(0x55, 0);
	\ndcode
*/
void LCD_TM023KDH03_Init(uint8_t ubValue);
//------------------------------------------------------------------------------
/*!
	\brief 		LCD Initial Timing (8080) Function
	\param[in]	ubMode is the 8080 mode type (LCD_8080_8_2BYTE / LCD_8080_16 / LCD_8080_8_3BYTE / LCD_8080_9)
	\par [Example]
	\code    
		 LCD_TM023KDH03_CPU(LCD_8080_8_3BYTE);
	\ndcode
*/
void LCD_TM023KDH03_CPU(uint8_t ubMode);
#endif
