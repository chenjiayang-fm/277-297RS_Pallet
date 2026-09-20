/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		LCD_SSD2828.h
	\brief		LCD SSD2828 Funcation Header
	\author		Pierce
	\version	1.6
	\date		2020/06/01
	\copyright	Copyright(C) 2020 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _LCD_SSD2828_H_
#define _LCD_SSD2828_H_
#include <stdbool.h>
#include "LCD_TYPE.H"
//------------------------------------------------------------------------------
#if (LCD_PANEL == LCD_SSD2828_ || LCD_PANEL == LCD_SSD2828_Y50019N00N || \
	 LCD_PANEL == LCD_SSD2828_YN550FH002N)
//! The command pins define for SSD2828
#if defined(BSP_DVR_SDK)
#define SSD2828_CSIO				GPIO->GPIO_OE3
#define SSD2828_SCKIO				GPIO->GPIO_OE2
#define SSD2828_SDIIO				GPIO->GPIO_OE5
#define SSD2828_SDOIO				GPIO->GPIO_OE4
#define SSD2828_CS					GPIO->GPIO_O3
#define SSD2828_SCK					GPIO->GPIO_O2
#define SSD2828_SDI					GPIO->GPIO_I5
#define SSD2828_SDO					GPIO->GPIO_O4
#else
#define SSD2828_CSIO				GPIO->GPIO_OE6
#define SSD2828_SCKIO				GPIO->GPIO_OE5
#define SSD2828_SDIIO				GPIO->GPIO_OE7
#define SSD2828_SDOIO				GPIO->GPIO_OE8
#define SSD2828_CS					GPIO->GPIO_O6
#define SSD2828_SCK					GPIO->GPIO_O5
#define SSD2828_SDI					GPIO->GPIO_I7
#define SSD2828_SDO					GPIO->GPIO_O8
#endif
//! The macro define for SSD2828
#define LCD_SSD2828_CMD_WR			(0x700000)
#define LCD_SSD2828_DAT_WR			(0x720000)
#define LCD_SSD2828_DAT_RD			(0x730000)

#define LCD_SSD2828_CMD_MAX			(8)
#define LCD_SSD2828_PK_MAX			(24)
//------------------------------------------------------------------------------
/*!
	\brief 		MIPI Bridge SSD2828 and MIPI LCD Initial Function	
	\return		true is MIPI bridge SSD2828 and MIPI LCD initial OK!
	\return		false is MIPI bridge SSD2828 and MIPI LCD initial Fail!
	\par [Example]
	\code    
		 bLCD_MIPI_SSD2828_Init();
	\ndcode
*/
bool bLCD_MIPI_SSD2828_Init (void);
//------------------------------------------------------------------------------
/*!
	\brief 		MIPI Bridge SSD2828 and MIPI LCD Start Display Function		
	\par [Example]
	\code    
		 LCD_MIPI_SSD2828_Start();
	\ndcode
*/
void LCD_MIPI_SSD2828_Start (void);
//------------------------------------------------------------------------------
/*!
	\brief 		LCD Initial Timing (RGB 24-Bit Data Enable) Function		
	\par [Example]
	\code    
		 LCD_MIPI_SSD2828();
	\ndcode
*/
void LCD_MIPI_SSD2828 (void);
//------------------------------------------------------------------------------
/*!
	\brief 		Write SSD2828 Register Function		
	\param[in]	ubReg is the register of the bridge SSD2828
	\param[in]	uwData is the writing data into the register of the bridge SSD2828
	\par [Example]
	\code    
		 LCD_SSD2828_RegWrite(0xB7,0x0349);
	\ndcode
*/
void LCD_SSD2828_RegWr (uint8_t ubReg, uint16_t uwData);
//------------------------------------------------------------------------------
/*!
	\brief 		Write SSD2828 Register Data Function		
	\param[in]	uwData is the writing data into the register of the bridge SSD2828
	\par [Example]
	\code    
		 LCD_SSD2828_DatWr(0x001C);	
	\ndcode
*/
void LCD_SSD2828_DatWr (uint16_t uwData);
//------------------------------------------------------------------------------
/*!
	\brief 		MIPI Bridge SSD2828 and MIPI LCD to Wake Up from Suspend Function		
	\par [Example]
	\code    
		 LCD_MIPI_SSD2828_Wakeup();
	\ndcode
*/
void LCD_MIPI_SSD2828_Wakeup (void);
//------------------------------------------------------------------------------
/*!
	\brief 		MIPI Bridge SSD2828 and MIPI LCD to Suspend from Displaying Function		
	\par [Example]
	\code    
		 LCD_MIPI_SSD2828_Sleep();
	\ndcode
*/
void LCD_MIPI_SSD2828_Sleep (void);

#endif
#endif
