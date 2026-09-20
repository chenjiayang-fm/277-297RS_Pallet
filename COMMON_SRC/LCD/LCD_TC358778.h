/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		LCD_TC358778.h
	\brief		LCD TC358778 Funcation Header
	\author		Pierce
	\version	1.6
	\date		2020/06/01
	\copyright	Copyright(C) 2020 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _LCD_TC358778_H_
#define _LCD_TC358778_H_
#include <stdbool.h>
#include "LCD_TYPE.H"
//------------------------------------------------------------------------------
#if (LCD_PANEL == LCD_TC358778_Y50019N00N)
//! The command pins define for TC358778
#if defined(BSP_DVR_SDK)
#define TC358778_CSIO				GPIO->GPIO_OE3
#define TC358778_SCKIO				GPIO->GPIO_OE2
#define TC358778_SDIIO				GPIO->GPIO_OE5
#define TC358778_SDOIO				GPIO->GPIO_OE4
#define TC358778_CS					GPIO->GPIO_O3
#define TC358778_SCK				GPIO->GPIO_O2
#define TC358778_SDI				GPIO->GPIO_I5
#define TC358778_SDO				GPIO->GPIO_O4
#else
#define TC358778_CSIO				GPIO->GPIO_OE6
#define TC358778_SCKIO				GPIO->GPIO_OE5
#define TC358778_SDIIO				GPIO->GPIO_OE7
#define TC358778_SDOIO				GPIO->GPIO_OE8
#define TC358778_CS					GPIO->GPIO_O6
#define TC358778_SCK					GPIO->GPIO_O5
#define TC358778_SDI					GPIO->GPIO_I7
#define TC358778_SDO					GPIO->GPIO_O8
#endif
//------------------------------------------------------------------------------
//! The macro define for TC358778
#define LCD_TC358778_CMD_WR			(0)
#define LCD_TC358778_CMD_RD			(0x10000)
//------------------------------------------------------------------------------
#define LCD_TC358778_CMD_MAX		(16)
#define LCD_TC358778_PK_MAX			(32)
//------------------------------------------------------------------------------
/*!
	\brief 		MIPI Bridge TC358778 and MIPI LCD Initial Function	
	\return		true is MIPI bridge TC358778 and MIPI LCD initial OK!
	\return		false is MIPI bridge TC358778 and MIPI LCD initial Fail!
	\par [Example]
	\code    
		 bLCD_MIPI_TC358778_Init();
	\ndcode
*/
bool bLCD_MIPI_TC358778_Init (void);
//------------------------------------------------------------------------------
/*!
	\brief 		MIPI Bridge TC358778 and MIPI LCD Start Display Function		
	\par [Example]
	\code    
		 LCD_MIPI_TC358778_Start();
	\ndcode
*/
void LCD_MIPI_TC358778_Start (void);
//------------------------------------------------------------------------------
/*!
	\brief 		LCD Initial Timing (RGB 24-Bit Data Enable) Function		
	\par [Example]
	\code    
		 LCD_MIPI_TC358778();
	\ndcode
*/
void LCD_MIPI_TC358778 (void);
//------------------------------------------------------------------------------
/*!
	\brief 		Write TC358778 Register Function		
	\param[in]	uwReg is the register of the bridge TC358778
	\param[in]	uwData is the writing data into the register of the bridge TC358778
	\par [Example]
	\code    
		 LCD_TC358778_RegWr(0x0002, 0x0001); 
	\ndcode
*/
void LCD_TC358778_RegWr (uint16_t uwReg, uint16_t uwData);
//------------------------------------------------------------------------------
/*!
	\brief 		Write TC358778 Register Data Function		
	\param[in]	uwData is the writing data into the register of the bridge TC358778
	\par [Example]
	\code    
		 LCD_MIPI_TC358778_Wakeup();	
	\ndcode
*/
void LCD_MIPI_TC358778_Wakeup (void);
//------------------------------------------------------------------------------
/*!
	\brief 		MIPI Bridge TC358778 and MIPI LCD to Wake Up from Suspend Function		
	\par [Example]
	\code    
		 LCD_MIPI_TC358778_Sleep();
	\ndcode
*/
void LCD_MIPI_TC358778_Sleep (void);

#endif
#endif
