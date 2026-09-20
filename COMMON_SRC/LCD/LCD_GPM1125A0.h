/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		LCD_GPM1125A0.h
	\brief		LCD GPM1125A0 Header
	\author		Pierce
	\version	0.2
	\date		2020/06/01
	\copyright	Copyright(C) 2020 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _LCD_GPM1125A0_H_
#define _LCD_GPM1125A0_H_
#include <stdbool.h>
#include "_510PF.h"
//------------------------------------------------------------------------------
//! The command pins define for GPM1125A0
#define GPM1125A0_CSIO				LCD->LCD_GPIO_OE11
#define GPM1125A0_SDAIO				LCD->LCD_GPIO_OE12
#define GPM1125A0_SCLIO				LCD->LCD_GPIO_OE13
#define GPM1125A0_CS				LCD->LCD_GPIO_O11
#define GPM1125A0_SDAO				LCD->LCD_GPIO_O12
#define GPM1125A0_SCL				LCD->LCD_GPIO_O13
#define GPM1125A0_SDAI				LCD->LCD_GPIO_I12

//! The macro define for GPM1125A0
#define LCD_GPM1125A0_ADDR_SFT		(3)
#define LCD_GPM1125A0_ADDR_MAX		(4)
#define LCD_GPM1125A0_ADDR_SB		(12)
#define LCD_GPM1125A0_DATA_SFT		(10)
#define LCD_GPM1125A0_DATA_MAX		(11)
#define LCD_GPM1125A0_DATA_MASK		(0x7FF)
#define LCD_GPM1125A0_SETTING_SFT	(15)
#define LCD_GPM1125A0_SETTING_MAX	(16)
//------------------------------------------------------------------------------
/*!
	\brief 		GPM1125A0 Serial RGB Data Mode Initial Function	
	\return		true is GPM1125A0 serial RGB data mode initial OK!
	\return		false is GPM1125A0 serial RGB data mode initial Fail!
	\par [Example]
	\code    
		 bLCD_GPM1125A0_SerialRgbData();
	\ndcode
*/
bool bLCD_GPM1125A0_SerialRgbData (void);
//------------------------------------------------------------------------------
/*!
	\brief 		GPM1125A0 RGB Dummy Mode Initial Function	
	\return		true is GPM1125A0 RGB dummy mode initial OK!
	\return		false is GPM1125A0 RGB dummy mode initial Fail!
	\par [Example]
	\code    
		 bLCD_GPM1125A0_RgbDummy();
	\ndcode
*/
bool bLCD_GPM1125A0_RgbDummy (void);
//------------------------------------------------------------------------------
/*!
	\brief 		GPM1125A0 BT601 Mode Initial Function	
	\return		true is GPM1125A0 BT601 mode initial OK!
	\return		false is GPM1125A0 BT601 mode initial Fail!
	\par [Example]
	\code    
		 bLCD_GPM1125A0_CCIR601();
	\ndcode
*/
bool bLCD_GPM1125A0_CCIR601 (void);
//------------------------------------------------------------------------------
/*!
	\brief 		GPM1125A0 BT656 Mode Initial Function	
	\return		true is GPM1125A0 BT656 mode initial OK!
	\return		false is GPM1125A0 BT656 mode initial Fail!
	\par [Example]
	\code    
		 bLCD_GPM1125A0_CCIR656();
	\ndcode
*/
bool bLCD_GPM1125A0_CCIR656 (void);
#endif
