/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		LCD_TC358778_Y50019N00N.c
	\brief		LCD TC358778 Funcation
	\author		Pierce
	\version	0.2
	\date		2020/06/01
	\copyright	Copyright(C) 2020 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#include <stdio.h>
#include <stdint.h>
#include "LCD.h"
#include "TIMER.h"
#include "SPI.h"
//------------------------------------------------------------------------------
#if (LCD_PANEL == LCD_TC358778_Y50019N00N)
#if !defined(BSP_DVR_SDK)
uint32_t* plLCD_SpiData;
//------------------------------------------------------------------------------
uint16_t LCD_TC358778_RegRd (uint16_t uwReg)
{
    *plLCD_SpiData = (uwReg << 16) | LCD_TC358778_CMD_RD;
    SPI_DmaRW((uint8_t*)plLCD_SpiData, (uint8_t*)plLCD_SpiData, 1, 32, SPI_WaitReady);
	*plLCD_SpiData = (uwReg << 16) | LCD_TC358778_CMD_RD;
	SPI_DmaRW((uint8_t*)plLCD_SpiData, (uint8_t*)plLCD_SpiData, 1, 32, SPI_WaitReady);
    return (uint16_t)(*plLCD_SpiData);
}
//------------------------------------------------------------------------------
void LCD_TC358778_RegWr (uint16_t uwReg, uint16_t uwData)
{
	uint16_t *puw = (uint16_t*)plLCD_SpiData;
	puw[0] = uwData;
	puw[1] = uwReg;
	SPI_DmaRW((uint8_t*)plLCD_SpiData, (uint8_t*)plLCD_SpiData, 1, 32, SPI_WaitReady);
}
//------------------------------------------------------------------------------
#else
uint32_t* plLCD_SpiData;
void LCD_TC358778_Wr (uint32_t ulValue)
{
	uint8_t ubi;
	
	TC358778_SDO = 1;
	TC358778_SCK = 1;
	TC358778_CS = 1;	
	TC358778_SDOIO = 1;
	TC358778_SCKIO = 1;
	TC358778_CSIO = 1;
	TIMER_Delay_us(1);
	
	TC358778_CS = 0;
	TIMER_Delay_us(1);
	for (ubi=0; ubi<LCD_TC358778_PK_MAX; ++ubi)
	{
		TC358778_SCK = 0;
		TC358778_SDO = (ulValue & (1 << (LCD_TC358778_PK_MAX - ubi -1)))?1:0;
		TIMER_Delay_us(1);
		TC358778_SCK = 1;
		TIMER_Delay_us(1);
	}
	TIMER_Delay_us(1);
	TC358778_CS = 1;
}
//------------------------------------------------------------------------------
uint16_t LCD_TC358778_Rd (uint32_t ulValue)
{
	uint16_t uwData=0;
	uint8_t ubi;
	
	TC358778_SDO = 1;
	TC358778_SCK = 1;
	TC358778_CS = 1;	
	TC358778_SDIIO = 0;
	TC358778_SDOIO = 1;
	TC358778_SCKIO = 1;
	TC358778_CSIO = 1;
	TIMER_Delay_us(1);
	
	TC358778_CS = 0;
	TIMER_Delay_us(1);
	for (ubi=0; ubi<LCD_TC358778_PK_MAX; ++ubi)
	{
		TC358778_SCK = 0;
		if (LCD_TC358778_CMD_MAX > ubi)
			TC358778_SDO = (ulValue & (1 << (LCD_TC358778_PK_MAX - ubi -1)))?1:0;
		TIMER_Delay_us(1);
		TC358778_SCK = 1;
		if (LCD_TC358778_CMD_MAX >= ubi && TC358778_SDI)
			uwData |= (1 << (LCD_TC358778_PK_MAX - ubi -1));
		TIMER_Delay_us(1);
	}
	TIMER_Delay_us(1);
	TC358778_CS = 1;
	return uwData;
}
//------------------------------------------------------------------------------
uint16_t LCD_TC358778_RegRd (uint16_t uwReg)
{
	uint32_t ulValue = (uwReg << 16) | LCD_TC358778_CMD_RD;
	uint16_t uwValue = LCD_TC358778_Rd(ulValue);	
	return uwValue;
}
//------------------------------------------------------------------------------
void LCD_TC358778_RegWr (uint16_t uwReg, uint16_t uwData)
{
	uint32_t ulValue;
	uint16_t *puw = (uint16_t*)&ulValue;
	puw[0] = uwData;
	puw[1] = uwReg;
	LCD_TC358778_Wr (ulValue);
}
#endif
//------------------------------------------------------------------------------
#define SPI_HW 		0
#define SPI_FW 		1
#define SPI_SEL		SPI_FW
//#define SPI_SEL		SPI_HW
bool bLCD_MIPI_TC358778_Init (void)
{
	uint16_t uwId;
    SPI_Setup_t spi_setup;

	printd(DBG_Debug3Lvl,"MIPI TC358778 Init\n");
#if defined(BSP_DVR_SDK)
	if(SPI_SEL  == SPI_HW)
#endif
	{
		spi_setup.ubSPI_CPOL    = 0;
		spi_setup.ubSPI_CPHA    = 0;
		spi_setup.tSPI_Mode     = SPI_MASTER;
		spi_setup.uwClkDiv      = ((float)9.6 / GLB->APBC_RATE) + 0.99 - 1;         // SPI Clock <= 1.5MHz
		if (!spi_setup.uwClkDiv)
			spi_setup.uwClkDiv = 1;
	}
    
    plLCD_SpiData = osUncachedMalloc(4);
    if(plLCD_SpiData == NULL)
    {
        printd(DBG_CriticalLvl, "LCD: osUncachedMalloc fail!!\n");
        while(1);
    }
#if defined(BSP_DVR_SDK)
	if(SPI_SEL  == SPI_HW)
#endif
	{
		SPI_Init(&spi_setup);
	}
	TIMER_Delay_ms(20);
	if (0x4401 != (uwId = LCD_TC358778_RegRd(0)))
	{
		printd(DBG_ErrorLvl, "LCD: TC358778 Fail %X\n", uwId);
#if !defined(BSP_DVR_SDK)
		return false;
#endif
	}

	LCD->LCD_MODE = LCD_DE;
	LCD->TV_LCD_EN = 1;
	//! SW Reset           
	LCD_TC358778_RegWr(0x0002, 0x0001);   
	TIMER_Delay_us(1);
	LCD_TC358778_RegWr(0x0002, 0x0000);   
	TIMER_Delay_ms(1);
	
	//! PLL
	LCD_TC358778_RegWr(0x0016, 0x108D);
	LCD_TC358778_RegWr(0x0018, 0x0603);	
	TIMER_Delay_ms(1);
	LCD_TC358778_RegWr(0x0018, 0x0613);	
	TIMER_Delay_ms(1);
	
	//! VSDly
	LCD_TC358778_RegWr(0x0006, 0x0050);
	
	//! D-PHY
	LCD_TC358778_RegWr(0x0140, 0x0000);	//!< Clock lane Enable
	LCD_TC358778_RegWr(0x0142, 0x0000);	
	
	LCD_TC358778_RegWr(0x0144, 0x0000);	//!< Data lane0 Enable
	LCD_TC358778_RegWr(0x0146, 0x0000);	
	
	LCD_TC358778_RegWr(0x0148, 0x0000);	//!< Data lane1 Enable
	LCD_TC358778_RegWr(0x014A, 0x0000);	
	
	LCD_TC358778_RegWr(0x014C, 0x0000);	//!< Data lane2 Enable
	LCD_TC358778_RegWr(0x014E, 0x0000);	
	
	LCD_TC358778_RegWr(0x0150, 0x0000);	//!< Data lane3 Enable
	LCD_TC358778_RegWr(0x0152, 0x0000);
	
	LCD_TC358778_RegWr(0x0100, 0x0002);	//!< Clock lane Ctrl
	LCD_TC358778_RegWr(0x0102, 0x0000);	
	
	LCD_TC358778_RegWr(0x0104, 0x0002);	//!< Data lane0 Ctrl
	LCD_TC358778_RegWr(0x0106, 0x0000);	
	
	LCD_TC358778_RegWr(0x0108, 0x0002);	//!< Data lane1 Ctrl
	LCD_TC358778_RegWr(0x010A, 0x0000);	
	
	LCD_TC358778_RegWr(0x010C, 0x0002);	//!< Data lane2 Ctrl
	LCD_TC358778_RegWr(0x010E, 0x0000);	
	
	LCD_TC358778_RegWr(0x0110, 0x0002);	//!< Data lane3 Ctrl
	LCD_TC358778_RegWr(0x0112, 0x0000);
	
	//! DSI-TX PPI Ctrl
	LCD_TC358778_RegWr(0x0210, 0x0A8C);	//!< Line Initialization Counter
	LCD_TC358778_RegWr(0x0212, 0x0000);
	
	LCD_TC358778_RegWr(0x0214, 0x0002);	//!< SYSLPTX Timing Counter
	LCD_TC358778_RegWr(0x0216, 0x0000);
	
	LCD_TC358778_RegWr(0x0218, 0x0E02);	//!< Tick Header Counter
	LCD_TC358778_RegWr(0x021A, 0x0000);
	
	LCD_TC358778_RegWr(0x021C, 0x0000);	//!< Tick Header Counter
	LCD_TC358778_RegWr(0x021E, 0x0000);
	
	LCD_TC358778_RegWr(0x0220, 0x0002);	//!< THS Header Counter
	LCD_TC358778_RegWr(0x0222, 0x0000);
	
	LCD_TC358778_RegWr(0x0224, 0x4650);	//!< TWAKEUP 
	LCD_TC358778_RegWr(0x0226, 0x0000);
	
	LCD_TC358778_RegWr(0x0228, 0x0006);	//!< Tick Post Counter
	LCD_TC358778_RegWr(0x022A, 0x0000);
	
	LCD_TC358778_RegWr(0x022C, 0x0001);	//!< THS Trail Counter
	LCD_TC358778_RegWr(0x022E, 0x0000);
	
	LCD_TC358778_RegWr(0x0230, 0x0005);	//!< HSTXV Regulator Counter
	LCD_TC358778_RegWr(0x0232, 0x0000);
	
	LCD_TC358778_RegWr(0x0234, 0x001F);	//!< HSTXV Regulator Ebanle
	LCD_TC358778_RegWr(0x0236, 0x0000);
	
	LCD_TC358778_RegWr(0x0238, 0x0001);	//!< TX Option Ctrl 
	LCD_TC358778_RegWr(0x023A, 0x0000);
	
	LCD_TC358778_RegWr(0x023C, 0x0001);	//!< BTA Ctrl1 
	LCD_TC358778_RegWr(0x023E, 0x0002);
	
	LCD_TC358778_RegWr(0x0204, 0x0001);	//!< Start Ctrl 
	LCD_TC358778_RegWr(0x0206, 0x0000);
	
	//! DSI-Tx Timing Ctrl
	LCD_TC358778_RegWr(0x0620, 0x0001);	//!< DSI Hsync Event
	LCD_TC358778_RegWr(0x0622, 0x0010);	//!< DSI Vsync Blank
	LCD_TC358778_RegWr(0x0624, 0x000C);	//!< DSI Vsync BP
	LCD_TC358778_RegWr(0x0626, 0x0500);	//!< DSI V Size
	LCD_TC358778_RegWr(0x0628, 0x008A);	//!< DSI Hsync Blank Counter
	LCD_TC358778_RegWr(0x062A, 0x0045);	//!< DSI Hsync BP Counter
	LCD_TC358778_RegWr(0x062C, 0x0870);	//!< DSI H Size Counter
	
	LCD_TC358778_RegWr(0x0518, 0x0001);	//!< DSI Start
	LCD_TC358778_RegWr(0x051A, 0x0000);	
	
	//! LCD Initial
	//! u8 lcd_init_B9[]={0xB9,0xFF,0x83,0x94};
	LCD_TC358778_RegWr(0x0602, 0x4039);
	LCD_TC358778_RegWr(0x0604, 0x0004);	
	LCD_TC358778_RegWr(0x0610, 0xFFB9);
	LCD_TC358778_RegWr(0x0612, 0x9483);
	LCD_TC358778_RegWr(0x0600, 0x0001);
	TIMER_Delay_ms(1);
	
	//! u8 lcd_init_BA[]={0xBA,0x73,0x83};
	LCD_TC358778_RegWr(0x0602, 0x4039);
	LCD_TC358778_RegWr(0x0604, 0x0003);	
	LCD_TC358778_RegWr(0x0610, 0x73BA);
	LCD_TC358778_RegWr(0x0612, 0x0083);
	LCD_TC358778_RegWr(0x0600, 0x0001);
	TIMER_Delay_ms(1);
	
	//! u8 lcd_init_B1[]={0xB1,0x6C,0x12,0x12,
	//! 				  0x26,0x04,0x11,0xF1,
	//!                   0x81,0x3A,0x54,0x23,
	//!					  0x80,0xC0,0xD2,0x58};	
	LCD_TC358778_RegWr(0x0008, 0x0001);
	LCD_TC358778_RegWr(0x0050, 0x0039);	
	LCD_TC358778_RegWr(0x0022, 0x0010);
	LCD_TC358778_RegWr(0x00E0, 0x8000);
	LCD_TC358778_RegWr(0x00E8, 0x6CB1);
	LCD_TC358778_RegWr(0x00E8, 0x1212);
	LCD_TC358778_RegWr(0x00E8, 0x0426);
	LCD_TC358778_RegWr(0x00E8, 0xF111);
	LCD_TC358778_RegWr(0x00E8, 0x3A81);
	LCD_TC358778_RegWr(0x00E8, 0x2354);
	LCD_TC358778_RegWr(0x00E8, 0xC080);
	LCD_TC358778_RegWr(0x00E8, 0x58D2);
	LCD_TC358778_RegWr(0x00E0, 0xE000);
	TIMER_Delay_ms(1);
	LCD_TC358778_RegWr(0x00E0, 0x2000);
	LCD_TC358778_RegWr(0x00E0, 0x0000);
	TIMER_Delay_ms(1);
	
	//! u8 lcd_init_B2[]={0xB2,0x00,0x64,0x0E,
	//!                   0x0D,0x22,0x1C,0x08,
	//!					  0x08,0x1C,0x4D,0x00};	
	LCD_TC358778_RegWr(0x0008, 0x0001);
	LCD_TC358778_RegWr(0x0050, 0x0039);	
	LCD_TC358778_RegWr(0x0022, 0x000C);
	LCD_TC358778_RegWr(0x00E0, 0x8000);
	LCD_TC358778_RegWr(0x00E8, 0x00B2);
	LCD_TC358778_RegWr(0x00E8, 0x0E64);
	LCD_TC358778_RegWr(0x00E8, 0x220D);
	LCD_TC358778_RegWr(0x00E8, 0x081C);
	LCD_TC358778_RegWr(0x00E8, 0x1C08);
	LCD_TC358778_RegWr(0x00E8, 0x004D);
	LCD_TC358778_RegWr(0x00E0, 0xE000);
	TIMER_Delay_ms(1);
	LCD_TC358778_RegWr(0x00E0, 0x2000);
	LCD_TC358778_RegWr(0x00E0, 0x0000);
	TIMER_Delay_ms(1);
	
	//! u8 lcd_init_B4[]={0xB4,0x00,0xFF,0x51,
	//!					  0x5A,0x59,0x5A,0x03,
	//!					  0x5A,0x01,0x70,0x20,0x70};	
	LCD_TC358778_RegWr(0x0008, 0x0001);
	LCD_TC358778_RegWr(0x0050, 0x0039);	
	LCD_TC358778_RegWr(0x0022, 0x000D);
	LCD_TC358778_RegWr(0x00E0, 0x8000);
	LCD_TC358778_RegWr(0x00E8, 0x00B4);
	LCD_TC358778_RegWr(0x00E8, 0x51FF);
	LCD_TC358778_RegWr(0x00E8, 0x595A);
	LCD_TC358778_RegWr(0x00E8, 0x035A);
	LCD_TC358778_RegWr(0x00E8, 0x015A);
	LCD_TC358778_RegWr(0x00E8, 0x2070);
	LCD_TC358778_RegWr(0x00E8, 0x0070);
	LCD_TC358778_RegWr(0x00E0, 0xE000);
	TIMER_Delay_ms(1);
	LCD_TC358778_RegWr(0x00E0, 0x2000);
	LCD_TC358778_RegWr(0x00E0, 0x0000);
	TIMER_Delay_ms(1);

	//! u8 lcd_init_BC[]={0xBC,0x07};	
	LCD_TC358778_RegWr(0x0602, 0x1015);
	LCD_TC358778_RegWr(0x0604, 0x0000);	
	LCD_TC358778_RegWr(0x0610, 0x07BC);
	LCD_TC358778_RegWr(0x0600, 0x0001);
	TIMER_Delay_ms(1);
	

	//! u8 lcd_init_BF[]={0xBF,0x41,0x0E,0x01};	
	LCD_TC358778_RegWr(0x0602, 0x4039);
	LCD_TC358778_RegWr(0x0604, 0x0004);	
	LCD_TC358778_RegWr(0x0610, 0x41BF);
	LCD_TC358778_RegWr(0x0612, 0x010E);
	LCD_TC358778_RegWr(0x0600, 0x0001);
	TIMER_Delay_ms(1);
	
	//!u8 lcd_init_D3[]={0xD3,0x00,0x0F,0x00,
	//!					 0x40,0x07,0x10,0x00,
	//!					 0x08,0x10,0x08,0x00,
	//!					 0x08,0x54,0x15,0x0E,
	//!					 0x05,0x0E,0x02,0x15,
	//!					 0x06,0x05,0x06,0x47,
	//!					 0x44,0x0A,0x0A,0x4B,
	//!					 0x10,0x07,0x07};	
	LCD_TC358778_RegWr(0x0008, 0x0001);
	LCD_TC358778_RegWr(0x0050, 0x0039);	
	LCD_TC358778_RegWr(0x0022, 0x001F);
	LCD_TC358778_RegWr(0x00E0, 0x8000);
	LCD_TC358778_RegWr(0x00E8, 0x00D3);
	LCD_TC358778_RegWr(0x00E8, 0x000F);
	LCD_TC358778_RegWr(0x00E8, 0x0740);
	LCD_TC358778_RegWr(0x00E8, 0x0010);
	LCD_TC358778_RegWr(0x00E8, 0x1008);
	LCD_TC358778_RegWr(0x00E8, 0x0008);
	LCD_TC358778_RegWr(0x00E8, 0x5408);
	LCD_TC358778_RegWr(0x00E8, 0x0E15);	
	LCD_TC358778_RegWr(0x00E8, 0x0E05);
	LCD_TC358778_RegWr(0x00E8, 0x1502);
	LCD_TC358778_RegWr(0x00E8, 0x0506);
	LCD_TC358778_RegWr(0x00E8, 0x4706);
	LCD_TC358778_RegWr(0x00E8, 0x0A44);
	LCD_TC358778_RegWr(0x00E8, 0x4B0A);
	LCD_TC358778_RegWr(0x00E8, 0x0710);
	LCD_TC358778_RegWr(0x00E8, 0x0007);
	LCD_TC358778_RegWr(0x00E0, 0xE000);
	TIMER_Delay_ms(1);
	LCD_TC358778_RegWr(0x00E0, 0x2000);
	LCD_TC358778_RegWr(0x00E0, 0x0000);
	TIMER_Delay_ms(1);
	
	//! u8 lcd_init_D5[]={0xD5,0x1A,0x1A,0x1B,
	//! 				  0x1B,0x00,0x01,0x02,
	//!					  0x03,0x04,0x05,0x06,
	//!					  0x07,0x08,0x09,0x0A,
	//!					  0x0B,0x24,0x25,0x18,
	//!					  0x18,0x26,0x27,0x18,
	//!					  0x18,0x18,0x18,0x18,
	//!					  0x18,0x18,0x18,0x18,
	//!					  0x18,0x18,0x18,0x18,
	//!					  0x18,0x18,0x18,0x20,
	//!					  0x21,0x18,0x18,0x18,0x18};
	LCD_TC358778_RegWr(0x0008, 0x0001);
	LCD_TC358778_RegWr(0x0050, 0x0039);	
	LCD_TC358778_RegWr(0x0022, 0x002D);
	LCD_TC358778_RegWr(0x00E0, 0x8000);
	LCD_TC358778_RegWr(0x00E8, 0x1AD5);
	LCD_TC358778_RegWr(0x00E8, 0x1B1A);
	LCD_TC358778_RegWr(0x00E8, 0x001B);
	LCD_TC358778_RegWr(0x00E8, 0x0201);
	LCD_TC358778_RegWr(0x00E8, 0x0403);
	LCD_TC358778_RegWr(0x00E8, 0x0605);
	LCD_TC358778_RegWr(0x00E8, 0x0807);
	LCD_TC358778_RegWr(0x00E8, 0x0A09);	
	LCD_TC358778_RegWr(0x00E8, 0x240B);
	LCD_TC358778_RegWr(0x00E8, 0x1825);
	LCD_TC358778_RegWr(0x00E8, 0x2618);
	LCD_TC358778_RegWr(0x00E8, 0x1827);
	LCD_TC358778_RegWr(0x00E8, 0x1818);
	LCD_TC358778_RegWr(0x00E8, 0x1818);
	LCD_TC358778_RegWr(0x00E8, 0x1818);
	LCD_TC358778_RegWr(0x00E8, 0x1818);	
	LCD_TC358778_RegWr(0x00E8, 0x1818);
	LCD_TC358778_RegWr(0x00E8, 0x1818);
	LCD_TC358778_RegWr(0x00E8, 0x1818);
	LCD_TC358778_RegWr(0x00E8, 0x2018);
	LCD_TC358778_RegWr(0x00E8, 0x1821);
	LCD_TC358778_RegWr(0x00E8, 0x1818);
	LCD_TC358778_RegWr(0x00E8, 0x0018);
	LCD_TC358778_RegWr(0x00E0, 0xE000);
	TIMER_Delay_ms(1);
	LCD_TC358778_RegWr(0x00E0, 0x2000);
	LCD_TC358778_RegWr(0x00E0, 0x0000);
	TIMER_Delay_ms(1);
	

	//!u8 lcd_init_D6[]={0xD6,0x1A,0x1A,0x1B,
	//!					 0x1B,0x0B,0x0A,0x09,
	//!					 0x08,0x07,0x06,0x05,
	//!					 0x04,0x03,0x02,0x01,
	//!					 0x00,0x21,0x20,0x58,
	//!					 0x58,0x27,0x26,0x18,
	//!					 0x18,0x18,0x18,0x18,
	//!					 0x18,0x18,0x18,0x18,
	//!					 0x18,0x18,0x18,0x18,
	//!					 0x18,0x18,0x18,0x25,
	//!					 0x24,0x18,0x18,0x18,0x18};
	LCD_TC358778_RegWr(0x0008, 0x0001);
	LCD_TC358778_RegWr(0x0050, 0x0039);	
	LCD_TC358778_RegWr(0x0022, 0x002D);
	LCD_TC358778_RegWr(0x00E0, 0x8000);
	LCD_TC358778_RegWr(0x00E8, 0x1AD6);
	LCD_TC358778_RegWr(0x00E8, 0x1B1A);
	LCD_TC358778_RegWr(0x00E8, 0x0B1B);
	LCD_TC358778_RegWr(0x00E8, 0x090A);
	LCD_TC358778_RegWr(0x00E8, 0x0708);
	LCD_TC358778_RegWr(0x00E8, 0x0506);
	LCD_TC358778_RegWr(0x00E8, 0x0304);
	LCD_TC358778_RegWr(0x00E8, 0x0102);	
	LCD_TC358778_RegWr(0x00E8, 0x2100);	
	LCD_TC358778_RegWr(0x00E8, 0x5820);
	LCD_TC358778_RegWr(0x00E8, 0x2758);
	LCD_TC358778_RegWr(0x00E8, 0x1826);
	LCD_TC358778_RegWr(0x00E8, 0x1818);
	LCD_TC358778_RegWr(0x00E8, 0x1818);
	LCD_TC358778_RegWr(0x00E8, 0x1818);
	LCD_TC358778_RegWr(0x00E8, 0x1818);	
	LCD_TC358778_RegWr(0x00E8, 0x1818);	
	LCD_TC358778_RegWr(0x00E8, 0x1818);
	LCD_TC358778_RegWr(0x00E8, 0x1818);
	LCD_TC358778_RegWr(0x00E8, 0x2518);
	LCD_TC358778_RegWr(0x00E8, 0x1824);
	LCD_TC358778_RegWr(0x00E8, 0x1818);
	LCD_TC358778_RegWr(0x00E8, 0x0018);
	LCD_TC358778_RegWr(0x00E0, 0xE000);
	TIMER_Delay_ms(1);
	LCD_TC358778_RegWr(0x00E0, 0x2000);
	LCD_TC358778_RegWr(0x00E0, 0x0000);
	TIMER_Delay_ms(1);

	//! u8 lcd_init_C6[]={0xC6,0xBD};	
	LCD_TC358778_RegWr(0x0602, 0x1015);
	LCD_TC358778_RegWr(0x0604, 0x0000);	
	LCD_TC358778_RegWr(0x0610, 0xBDC6);	
	LCD_TC358778_RegWr(0x0600, 0x0001);
	TIMER_Delay_ms(1);

	//! u8 lcd_init_BD[]={0xBD,0x02};
	LCD_TC358778_RegWr(0x0602, 0x1015);
	LCD_TC358778_RegWr(0x0604, 0x0000);	
	LCD_TC358778_RegWr(0x0610, 0x02BD);	
	LCD_TC358778_RegWr(0x0600, 0x0001);
	TIMER_Delay_ms(1);
	
	//! u8 lcd_init_D8[]={0xD8,0xFF,0xFF,0xEE,
	//!    				  0xEB,0xFB,0xA0,0xFF,
	//!					  0xFF,0xEE,0xEB,0xFB,0xA0};
	LCD_TC358778_RegWr(0x0008, 0x0001);
	LCD_TC358778_RegWr(0x0050, 0x0039);	
	LCD_TC358778_RegWr(0x0022, 0x000D);
	LCD_TC358778_RegWr(0x00E0, 0x8000);
	LCD_TC358778_RegWr(0x00E8, 0xFFD8);
	LCD_TC358778_RegWr(0x00E8, 0xEEFF);
	LCD_TC358778_RegWr(0x00E8, 0xFBEB);
	LCD_TC358778_RegWr(0x00E8, 0xFFA0);
	LCD_TC358778_RegWr(0x00E8, 0xEEFF);
	LCD_TC358778_RegWr(0x00E8, 0xFBEB);
	LCD_TC358778_RegWr(0x00E8, 0x00A0);
	LCD_TC358778_RegWr(0x00E0, 0xE000);
	TIMER_Delay_ms(1);
	LCD_TC358778_RegWr(0x00E0, 0x2000);
	LCD_TC358778_RegWr(0x00E0, 0x0000);
	TIMER_Delay_ms(1);

	//! u8 lcd_init_CC[]={0xCC,0x01};	
	LCD_TC358778_RegWr(0x0602, 0x1015);
	LCD_TC358778_RegWr(0x0604, 0x0000);	
	LCD_TC358778_RegWr(0x0610, 0x05CC);	
	LCD_TC358778_RegWr(0x0600, 0x0001);
	TIMER_Delay_ms(1);
	

	//! u8 lcd_init_C0[]={0xC0,0x30,0x14};	
	LCD_TC358778_RegWr(0x0602, 0x4039);
	LCD_TC358778_RegWr(0x0604, 0x0003);	
	LCD_TC358778_RegWr(0x0610, 0x30c0);
	LCD_TC358778_RegWr(0x0612, 0x0014);
	LCD_TC358778_RegWr(0x0600, 0x0001);
	TIMER_Delay_ms(1);
	
	//! u8 lcd_init_C7[]={0xC7,0x00,0xC0,0x40,0xC0};
	LCD_TC358778_RegWr(0x0602, 0x4039);
	LCD_TC358778_RegWr(0x0604, 0x0005);	
	LCD_TC358778_RegWr(0x0610, 0x00c7);
	LCD_TC358778_RegWr(0x0612, 0x40c0);
	LCD_TC358778_RegWr(0x0614, 0x00c0);
	LCD_TC358778_RegWr(0x0600, 0x0001);
	TIMER_Delay_ms(1);
	
	//! u8 lcd_init_B6[]={0xB6,0x64,0x64};
	LCD_TC358778_RegWr(0x0602, 0x4039);
	LCD_TC358778_RegWr(0x0604, 0x0003);	
	LCD_TC358778_RegWr(0x0610, 0x64B6);
	LCD_TC358778_RegWr(0x0612, 0x0064);
	LCD_TC358778_RegWr(0x0600, 0x0001);
	TIMER_Delay_ms(1);
	
	//! u8 lcd_init_E0[]={0xE0,0x00,0x12,0x19,
	//!					  0x23,0x2A,0x3f,0x25,
	//!					  0x42,0x08,0x0A,0x0D,
	//!					  0x18,0x0E,0x10,0x12,
	//!					  0x10,0x12,0x0C,0x16,
	//!					  0x18,0x1C,0x00,0x12,
	//!					  0x19,0x23,0x2A,0x3f,
	//!					  0x25,0x42,0x08,0x0A,
	//!					  0x0D,0x18,0x0E,0x10,
	//!					  0x12,0x10,0x12,0x0C,
	//!					  0x16,0x18,0x1C};
	LCD_TC358778_RegWr(0x0008, 0x0001);
	LCD_TC358778_RegWr(0x0050, 0x0039);	
	LCD_TC358778_RegWr(0x0022, 0x002B);
	LCD_TC358778_RegWr(0x00E0, 0x8000);
	LCD_TC358778_RegWr(0x00E8, 0x00E0);
	LCD_TC358778_RegWr(0x00E8, 0x1912);
	LCD_TC358778_RegWr(0x00E8, 0x2A23);
	LCD_TC358778_RegWr(0x00E8, 0x253F);
	LCD_TC358778_RegWr(0x00E8, 0x0842);
	LCD_TC358778_RegWr(0x00E8, 0x0D0A);
	LCD_TC358778_RegWr(0x00E8, 0x0E18);
	LCD_TC358778_RegWr(0x00E8, 0x1210);	
	LCD_TC358778_RegWr(0x00E8, 0x1210);
	LCD_TC358778_RegWr(0x00E8, 0x160C);
	LCD_TC358778_RegWr(0x00E8, 0x1C18);
	LCD_TC358778_RegWr(0x00E8, 0x1200);
	LCD_TC358778_RegWr(0x00E8, 0x2319);
	LCD_TC358778_RegWr(0x00E8, 0x3F2A);
	LCD_TC358778_RegWr(0x00E8, 0x4225);
	LCD_TC358778_RegWr(0x00E8, 0x0A08);	
	LCD_TC358778_RegWr(0x00E8, 0x180D);
	LCD_TC358778_RegWr(0x00E8, 0x100E);
	LCD_TC358778_RegWr(0x00E8, 0x1012);
	LCD_TC358778_RegWr(0x00E8, 0x0C12);
	LCD_TC358778_RegWr(0x00E8, 0x1816);
	LCD_TC358778_RegWr(0x00E8, 0x001C);
	LCD_TC358778_RegWr(0x00E0, 0xE000);
	TIMER_Delay_ms(1);
	LCD_TC358778_RegWr(0x00E0, 0x2000);
	LCD_TC358778_RegWr(0x00E0, 0x0000);
	TIMER_Delay_ms(1);
	
	//! mipi_0data(0x11);	
	//! Leave sleep mode for LCD
	LCD_TC358778_RegWr(0x0602, 0x1005);
	LCD_TC358778_RegWr(0x0604, 0x0000);
	LCD_TC358778_RegWr(0x0610, 0x0011);
	LCD_TC358778_RegWr(0x0600, 0x0001);
	TIMER_Delay_ms(120);
	
	//! mipi_0data(0x29);	
	//! Display on for LCD
	LCD_TC358778_RegWr(0x0602, 0x1005);
	LCD_TC358778_RegWr(0x0604, 0x0000);
	LCD_TC358778_RegWr(0x0610, 0x0029);
	LCD_TC358778_RegWr(0x0600, 0x0001);
	TIMER_Delay_ms(1);	
	LCD->TV_LCD_EN = 0;
	return true;
}
//------------------------------------------------------------------------------
void LCD_MIPI_TC358778_Sleep (void)
{
	//! mipi_0data(0x10);
	//! Enter sleep for LCD
	LCD_TC358778_RegWr(0x0602, 0x1005);
	LCD_TC358778_RegWr(0x0604, 0x0000);
	LCD_TC358778_RegWr(0x0610, 0x0010);
	LCD_TC358778_RegWr(0x0600, 0x0001);
	TIMER_Delay_us(10);
	
	LCD_TC358778_RegWr(0x0032, 0x8000);	//!< Frame Stop for TC358778
	TIMER_Delay_ms(40);
	LCD_TC358778_RegWr(0x0004, 0x0004); //!< Parallel port disable
	LCD_TC358778_RegWr(0x0032, 0xC000);	//!< Reset Frame for TC358778
	
	LCD_TC358778_RegWr(0x0238, 0x0000);	//!< Stop DSI clock
	LCD_TC358778_RegWr(0x023A, 0x0000);
	
	LCD_TC358778_RegWr(0x0500, 0x0080);	//! Set LP mode
	LCD_TC358778_RegWr(0x0502, 0xC300);
	LCD_TC358778_RegWr(0x0500, 0x8000);	//! Set LP mode
	LCD_TC358778_RegWr(0x0502, 0xA300);
	
	//! D-PHY
	LCD_TC358778_RegWr(0x0140, 0x0001);	//!< Clock lane Disable
	LCD_TC358778_RegWr(0x0142, 0x0000);	
	
	LCD_TC358778_RegWr(0x0144, 0x0001);	//!< Data lane0 Disable
	LCD_TC358778_RegWr(0x0146, 0x0000);	
	
	LCD_TC358778_RegWr(0x0148, 0x0001);	//!< Data lane1 Disable
	LCD_TC358778_RegWr(0x014A, 0x0000);	
	
	LCD_TC358778_RegWr(0x014C, 0x0001);	//!< Data lane2 Disable
	LCD_TC358778_RegWr(0x014E, 0x0000);	
	
	LCD_TC358778_RegWr(0x0150, 0x0001);	//!< Data lane3 Disable
	LCD_TC358778_RegWr(0x0152, 0x0000);
	
	LCD_TC358778_RegWr(0x0518, 0x0000);	//!< DSI Start
	LCD_TC358778_RegWr(0x051A, 0x0000);	
	
	LCD_TC358778_RegWr(0x0008, 0x0034);	//!< DSI Tx
	LCD_TC358778_RegWr(0x0018, 0x0600);	
}
//------------------------------------------------------------------------------
void LCD_MIPI_TC358778_Wakeup (void)
{
	LCD_TC358778_RegWr(0x0018, 0x0603);	
	TIMER_Delay_ms(1);
	LCD_TC358778_RegWr(0x0018, 0x0613);	
	TIMER_Delay_ms(1);
	//! D-PHY
	LCD_TC358778_RegWr(0x0140, 0x0000);	//!< Clock lane Enable
	LCD_TC358778_RegWr(0x0142, 0x0000);	
	
	LCD_TC358778_RegWr(0x0144, 0x0000);	//!< Data lane0 Enable
	LCD_TC358778_RegWr(0x0146, 0x0000);	
	
	LCD_TC358778_RegWr(0x0148, 0x0000);	//!< Data lane1 Enable
	LCD_TC358778_RegWr(0x014A, 0x0000);	
	
	LCD_TC358778_RegWr(0x014C, 0x0000);	//!< Data lane2 Enable
	LCD_TC358778_RegWr(0x014E, 0x0000);	
	
	LCD_TC358778_RegWr(0x0150, 0x0000);	//!< Data lane3 Enable
	LCD_TC358778_RegWr(0x0152, 0x0000);
	
	LCD_TC358778_RegWr(0x0238, 0x0001);	//!< TX Option Ctrl 
	LCD_TC358778_RegWr(0x023A, 0x0000);
	
	LCD_TC358778_RegWr(0x0518, 0x0001);	//!< DSI Start
	LCD_TC358778_RegWr(0x051A, 0x0000);	
	
	LCD_MIPI_TC358778_Start();
	
	//! mipi_0data(0x11);
	//! Leave sleep mode for LCD
	LCD_TC358778_RegWr(0x0602, 0x1005);
	LCD_TC358778_RegWr(0x0604, 0x0000);
	LCD_TC358778_RegWr(0x0610, 0x0011);
	LCD_TC358778_RegWr(0x0600, 0x0001);
}
//------------------------------------------------------------------------------
void LCD_MIPI_TC358778_Start (void)
{
	LCD_TC358778_RegWr(0x0500, 0x0086);	//!< Set to HS mode 
	LCD_TC358778_RegWr(0x0502, 0xA300);
	LCD_TC358778_RegWr(0x0500, 0x8000);	//! Set LP mode
	LCD_TC358778_RegWr(0x0502, 0xC300);	
	
	LCD_TC358778_RegWr(0x0008, 0x0037);	//!< DSI Tx
	LCD_TC358778_RegWr(0x0050, 0x003E);
	LCD_TC358778_RegWr(0x0032, 0x0000);	
	LCD_TC358778_RegWr(0x0004, 0x0044); //!< Parallel port enable
}
//------------------------------------------------------------------------------
void LCD_MIPI_TC358778 (void)
{
	//! MIPI TC358778
	printd(DBG_Debug3Lvl, "LCD MIPI TC358778\n");
	printd(DBG_Debug3Lvl, "DE Mode\n");
	printd(DBG_Debug3Lvl, "Screen Size 720 x 1280\n");
	//! LCD Setting
	LCD->LCD_MODE = LCD_DE;
	LCD->SEL_TV = 0;		

	LCD->LCD_HO_SIZE = 720;
	LCD->LCD_VO_SIZE = 1280;	
	//! Timing
	LCD->LCD_HT_DM_SIZE = 176;
	LCD->LCD_VT_DM_SIZE = 16;
	LCD->LCD_HT_START = 48;
	LCD->LCD_VT_START = 16;
	
	LCD->LCD_HS_WIDTH = 24;
	LCD->LCD_VS_WIDTH = 4; 
			
	LCD->LCD_RGB_REVERSE = 0;
	LCD->LCD_EVEN_RGB = 1;
	LCD->LCD_ODD_RGB = 1;
	LCD->LCD_HSYNC_HIGH = 0;
	LCD->LCD_VSYNC_HIGH = 0;
	LCD->LCD_DE_HIGH = 1;
	LCD->LCD_PCK_RIS = 1;//0;	
}
//------------------------------------------------------------------------------
void LCD_PixelPllSetting(void)
{
	float fPixelClock;
	uint8_t ubFps = 60;
	float fFVCO;

	//! Calculation Pixel Clock
	switch (LCD->LCD_MODE)
	{
		case LCD_AU_UPS051_8:
		case LCD_AU_UPS051_6:
		case LCD_DE:
			fPixelClock = ((float)ubFps) * (LCD->LCD_HO_SIZE + LCD->LCD_HT_DM_SIZE + LCD->LCD_HT_START) *
								   (LCD->LCD_VO_SIZE + LCD->LCD_VT_DM_SIZE + LCD->LCD_VT_START) / 1000000;
			break;
		case LCD_RGB_DUMMY:
			fPixelClock = ((float)ubFps) * ((LCD->LCD_HO_SIZE << 2) + LCD->LCD_HT_DM_SIZE + LCD->LCD_HT_START) *
								            (LCD->LCD_VO_SIZE + LCD->LCD_VT_DM_SIZE + LCD->LCD_VT_START) / 1000000;
			break;
		case LCD_AU:
			fPixelClock = ((float)ubFps) * (LCD->LCD_HO_SIZE * 3 + LCD->LCD_HT_DM_SIZE + LCD->LCD_HT_START) *
											(LCD->LCD_VO_SIZE + LCD->LCD_VT_DM_SIZE + LCD->LCD_VT_START) / 1000000;
			break;
		case LCD_YUV422:
			fPixelClock = ((float)ubFps) * ((LCD->LCD_HO_SIZE << 1) + LCD->LCD_HT_DM_SIZE + LCD->LCD_HT_START) *
											(LCD->LCD_VO_SIZE + LCD->LCD_VT_DM_SIZE + LCD->LCD_VT_START) / 1000000;
			break;
		case LCD_BT656_BT601:
			fPixelClock = ((float)ubFps) * LCD->LCD_VO_SIZE * ((LCD->LCD_HO_SIZE << 1) + 
								           LCD->LCD_HT_START + (LCD->LCD_HS_WIDTH << 1) + 8) / 1000000;
			break;
		default:
			return;
	}
	
	printd(DBG_InfoLvl, "LCD Pixel Clock = %f MHz\n", fPixelClock);					

	LCD->LCD_PCK_SPEED = 1;
	GLB->LCDPLL_CK_SEL = 1;
	//! LCD PLL
	if (!GLB->LCDPLL_PD_N)
	{
		GLB->LCDPLL_LDO_EN = 1;
		GLB->LCDPLL_PD_N = 1;
		TIMER_Delay_us(400);
		GLB->LCDPLL_SDM_EN = 1;
		TIMER_Delay_us(1);
	}	
	GLB->LCDPLL_INT = 12;
	GLB->LCDPLL_FRA = 0.385 * (1 << 20);
	TIMER_Delay_us(1);
	GLB->LCDPLL_INT_FRA_VLD = 1;
	TIMER_Delay_us(1);	
	//! LCD Controller rate
	GLB->LCD_RATE = 1;

	if (GLB->LCDPLL_CK_SEL == 0)
		fFVCO = fPixelClock * LCD->LCD_PCK_SPEED * 6;
	else if (GLB->LCDPLL_CK_SEL == 1)
		fFVCO = fPixelClock * LCD->LCD_PCK_SPEED * 2;
	else
		fFVCO = fPixelClock * LCD->LCD_PCK_SPEED * 1;
	
	if ((fFVCO < 148.3) || (fFVCO > 165)) { //range:148.3M ~ 165M
		printd(DBG_ErrorLvl, "fFVCO=%f MHz,Change PLL pls!\n", fFVCO);
		while(1);
	}
}
#endif
