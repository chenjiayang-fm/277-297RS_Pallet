/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		LCD_SSD2828.c
	\brief		LCD SSD2828 Funcation
	\author		Pierce
	\version	0.5
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
#if (LCD_PANEL == LCD_SSD2828_Y50019N00N)
#if !defined(BSP_DVR_SDK)
uint32_t* plLCD_SpiData;
//------------------------------------------------------------------------------
uint16_t LCD_SSD2828_RegRd (uint8_t ubReg)
{
    *plLCD_SpiData = (LCD_SSD2828_CMD_WR | ubReg) << 8;
    SPI_DmaRW((uint8_t*)plLCD_SpiData, (uint8_t*)plLCD_SpiData, 1, 32, SPI_WaitReady);

    *plLCD_SpiData = LCD_SSD2828_DAT_RD << 8;
    SPI_DmaRW((uint8_t*)plLCD_SpiData, (uint8_t*)plLCD_SpiData, 1, 32, SPI_WaitReady);

    return (uint16_t)(*plLCD_SpiData >> 8);
}
//------------------------------------------------------------------------------
void LCD_SSD2828_CmdWr (uint8_t ubReg)
{
    *plLCD_SpiData = (LCD_SSD2828_CMD_WR | ubReg) << 8;
    SPI_DmaRW((uint8_t*)plLCD_SpiData, (uint8_t*)plLCD_SpiData, 1, 32, SPI_WaitReady);
}
//------------------------------------------------------------------------------
void LCD_SSD2828_DatWr (uint16_t uwData)
{
    *plLCD_SpiData = (LCD_SSD2828_DAT_WR | uwData) << 8;
    SPI_DmaRW((uint8_t*)plLCD_SpiData, (uint8_t*)plLCD_SpiData, 1, 32, SPI_WaitReady);
	TIMER_Delay_us(1);
}
//------------------------------------------------------------------------------
void LCD_SSD2828_RegWr (uint8_t ubReg, uint16_t uwData)
{
	LCD_SSD2828_CmdWr (ubReg);
	LCD_SSD2828_DatWr (uwData);
}

void LCD_SSD2828_DriverICRd (uint8_t ubReg, uint16_t uwNum)
{
    uint16_t uwData;
    uint16_t uwBuf[uwNum];
    uint8_t i =0;
    LCD_SSD2828_RegWr(0xB7, 0x0382);
    uwData = LCD_SSD2828_RegRd(0xC6);
    if((uwData & 0x0001)==0)
    {  
        LCD_SSD2828_RegWr(0xC1, uwNum);//Num
        LCD_SSD2828_RegWr(0xC0, 0x0001);
        LCD_SSD2828_RegWr(0xC4, 0x0001);
        LCD_SSD2828_RegWr(0xBC, 0x0001);
        LCD_SSD2828_RegWr(0xBF, (uint16_t)ubReg);//Reg
        TIMER_Delay_ms(20);
        while((uwData=LCD_SSD2828_RegRd(0xC6))& 0x0001)
        {
            uwBuf[i] = LCD_SSD2828_RegRd(0xFF);                    
            printf("buf %x =%x,%x\n",i,uwBuf[i],uwData);
            i++;
        }
        printf("Rd Finish\n");
    }
    else
    {
        printf("Rd Err\n");
    }
}

//------------------------------------------------------------------------------
#else
uint32_t* plLCD_SpiData;
void LCD_SSD2828_Wr (uint32_t ulValue)
{
	uint8_t ubi;
	
	SSD2828_SDO = 1;
	SSD2828_SCK = 1;
	SSD2828_CS = 1;	
	SSD2828_SDOIO = 1;
	SSD2828_SCKIO = 1;
	SSD2828_CSIO = 1;
	TIMER_Delay_us(1);
	
	SSD2828_CS = 0;
	TIMER_Delay_us(1);
	for (ubi=0; ubi<LCD_SSD2828_PK_MAX; ++ubi)
	{
		SSD2828_SCK = 0;
		SSD2828_SDO = (ulValue & (1 << (LCD_SSD2828_PK_MAX - ubi -1)))?1:0;
		TIMER_Delay_us(1);
		SSD2828_SCK = 1;
		TIMER_Delay_us(1);
	}
	TIMER_Delay_us(1);
	SSD2828_CS = 1;
}
//------------------------------------------------------------------------------
uint16_t LCD_SSD2828_Rd (uint32_t ulValue)
{
	uint16_t uwData=0;
	uint8_t ubi;
	
	SSD2828_SDO = 1;
	SSD2828_SCK = 1;
	SSD2828_CS = 1;	
	SSD2828_SDIIO = 0;
	SSD2828_SDOIO = 1;
	SSD2828_SCKIO = 1;
	SSD2828_CSIO = 1;
	TIMER_Delay_us(1);
	
	SSD2828_CS = 0;
	TIMER_Delay_us(1);
	for (ubi=0; ubi<LCD_SSD2828_PK_MAX; ++ubi)
	{
		SSD2828_SCK = 0;
		if (LCD_SSD2828_CMD_MAX > ubi)
			SSD2828_SDO = (ulValue & (1 << (LCD_SSD2828_PK_MAX - ubi -1)))?1:0;
		TIMER_Delay_us(1);
		SSD2828_SCK = 1;
		if (LCD_SSD2828_CMD_MAX >= ubi && SSD2828_SDI)
			uwData |= (1 << (LCD_SSD2828_PK_MAX - ubi -1));
		TIMER_Delay_us(1);
	}
	TIMER_Delay_us(1);
	SSD2828_CS = 1;
	return uwData;
}
//------------------------------------------------------------------------------
uint16_t LCD_SSD2828_RegRd (uint8_t ubReg)
{
	uint16_t uwValue;
	
	LCD_SSD2828_Wr(LCD_SSD2828_CMD_WR | ubReg);
	//TIMER_Delay_us(3);
	TIMER_Delay_ms(1);
	uwValue = LCD_SSD2828_Rd(LCD_SSD2828_DAT_RD);	
	TIMER_Delay_us(1);
	return uwValue;
}
//------------------------------------------------------------------------------
void LCD_SSD2828_CmdWr (uint8_t ubReg)
{
	LCD_SSD2828_Wr(LCD_SSD2828_CMD_WR | ubReg);
	TIMER_Delay_us(1);
}
//------------------------------------------------------------------------------
void LCD_SSD2828_DatWr (uint16_t uwData)
{
	LCD_SSD2828_Wr(LCD_SSD2828_DAT_WR | uwData);
	TIMER_Delay_us(1);
}
//------------------------------------------------------------------------------
void LCD_SSD2828_RegWr (uint8_t ubReg, uint16_t uwData)
{
	LCD_SSD2828_CmdWr (ubReg);
	LCD_SSD2828_DatWr (uwData);
}
#endif
//------------------------------------------------------------------------------
#define SPI_HW 		0
#define SPI_FW 		1
#define SPI_SEL		SPI_FW
//#define SPI_SEL		SPI_HW

bool bLCD_MIPI_SSD2828_Init (void)
{
	uint16_t uwId;
    SPI_Setup_t spi_setup;

	printd(DBG_Debug3Lvl,"MIPI SSD2828 Init\n");
#if defined(BSP_DVR_SDK)
	if(SPI_SEL  == SPI_HW)
#endif
	{
		spi_setup.ubSPI_CPOL    = 0;
		spi_setup.ubSPI_CPHA    = 0;
		spi_setup.tSPI_Mode     = SPI_MASTER;
		spi_setup.uwClkDiv      = ((float)160 / GLB->APBC_RATE) + 0.99 - 1;         // SPI Clock <= 1.5MHz
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
	if (0x2828 != (uwId = LCD_SSD2828_RegRd(0xB0)))
	{
		printd(DBG_ErrorLvl, "LCD: SSD2828 Fail %X\n", uwId);
#if !defined(BSP_DVR_SDK)
		return false;
#endif
	}	
	
	LCD_SSD2828_RegWr(0xB1, 0x0418);   // VSA = 2,HSA = 2           
	LCD_SSD2828_RegWr(0xB2, 0x1030);   // VBP = 14,VBP = 42         
	LCD_SSD2828_RegWr(0xB3, 0x14AC);   // VFP = 16,HFP = 44         
	LCD_SSD2828_RegWr(0xB4, 0x02D0);   // HACT = 720                
	LCD_SSD2828_RegWr(0xB5, 0x0500);   // VACT = 1280               
	LCD_SSD2828_RegWr(0xB6, 0x003B);   // Vsync Pulse is active low,
									   // Hsync Pulse is active low,Data is launch at falling edge,SSD2828 latch data at rising edge
									   // Video with blanking packet.
									   // Non video data will be transmitted during any BLLP period.
									   // Non video data will be transmitted using HS mode.
									   // LP mode will be used during BLLP period.
									   // The clock lane enters LP mode when there is no data to transmit.
									   // Burst mode
									   // 24bpp
	LCD_SSD2828_RegWr(0xB8,0x0000);
	LCD_SSD2828_RegWr(0xB9,0x0000);	  // Divide by 1,Enable Sys_clk output,PLL power down                                                
	LCD_SSD2828_RegWr(0xBA,0xC02A);   // 10-251<Fout<500,MS=1,NS=22                                                                    
	LCD_SSD2828_RegWr(0xBB,0x0006);   // LP mode clock, Divide by 6                                                                    
	LCD_SSD2828_RegWr(0xD5,0x1860);                                                                                                      
	LCD_SSD2828_RegWr(0xC9,0x1504);   // HS Zero Delay = HZD * nibble_clk ; HS Prepare Delay =  4 nibble_clk + HPD * nibble_clk        
	LCD_SSD2828_RegWr(0xCA,0x2203);   // CLK Zero Delay = CZD * nibble_clk ; CLK Prepare Delay =  3 nibble_clk + CPD * nibble_clk      
	LCD_SSD2828_RegWr(0xCB,0x0524);   // CLK Pre Delay = CPED * nibble_clk + 0-1 * lp_clk(min,max) ; CLK Post Delay = CPTD * nibble_clk
	LCD_SSD2828_RegWr(0xCC,0x090B);   // CLK Trail Delay = CTD * nibble_clk ; HS Trail Delay = HTD * nibble_clk                        
	LCD_SSD2828_RegWr(0xDE,0x0003);   // 4 lane mode                                                                                   
	LCD_SSD2828_RegWr(0xB9,0x0001);   // Divide by 1,Enable Sys_clk output,PLL enable                                                  	
	LCD_SSD2828_RegWr(0xD6,0x0005);	  // RGB,R is in the higher portion of the pixel.	
	LCD_SSD2828_RegWr(0xB8,0x0000);

	//! Driver IC Initial Code
	TIMER_Delay_ms(5);

	LCD_SSD2828_RegWr(0xB7,0x0342); //!< TXD[11]: Transmit on
									//!< LPE[10]: Short Packet
									//!< EOT[9]: Send EOT Packet at the end of HS transmission
									//!< ECD[8]: Disable ECC CRC Check
									//!< REN[7]: Write operation
									//!< DCS[6]: Generic packet
									//!< CSS[5]: The clock source is tx_clk
									//!< HCLK[4]: HS clock is enabled
									//!< VEN[3]: Video mode is disabled
									//!< SLP[2]: Sleep mode is disabled
									//!< CKE[1]: Clock Lane Enable
									//!< HS[0]: LP mode 
	//! u8 lcd_init_B9[]={0xB9,0xFF,0x83,0x94};
	LCD_SSD2828_RegWr(0xBC,0x0004);	//!< Transmit Data Count
	LCD_SSD2828_RegWr(0xBF,0xFFB9);	//!< Data
	LCD_SSD2828_DatWr(0x9483);		//!< Data
	TIMER_Delay_ms(5);
	
	//! u8 lcd_init_BA[]={0xBA,0x73,0x83};	
	LCD_SSD2828_RegWr(0xBC,0x0003);
	LCD_SSD2828_RegWr(0xBF,0x73BA);
	LCD_SSD2828_DatWr(0x0083);
	TIMER_Delay_ms(5);
	
	//! u8 lcd_init_B1[]={0xB1,0x6C,0x12,0x12,
	//! 				  0x26,0x04,0x11,0xF1,
	//!                   0x81,0x3A,0x54,0x23,
	//!					  0x80,0xC0,0xD2,0x58};	
	LCD_SSD2828_RegWr(0xBC,0x0010);
	LCD_SSD2828_RegWr(0xBF,0x6CB1);
	LCD_SSD2828_DatWr(0x1212);
	LCD_SSD2828_DatWr(0x0426);
	LCD_SSD2828_DatWr(0xF111);
	LCD_SSD2828_DatWr(0x3A81);
	LCD_SSD2828_DatWr(0x2354);
	LCD_SSD2828_DatWr(0xC080);
	LCD_SSD2828_DatWr(0x58D2);
	TIMER_Delay_ms(5);
	
	//! u8 lcd_init_B2[]={0xB2,0x00,0x64,0x0E,
	//!                   0x0D,0x22,0x1C,0x08,
	//!					  0x08,0x1C,0x4D,0x00};	
	LCD_SSD2828_RegWr(0xBC,0x000C);
	LCD_SSD2828_RegWr(0xBF,0x00B2);
	LCD_SSD2828_DatWr(0x0E64);
	LCD_SSD2828_DatWr(0x220D);
	LCD_SSD2828_DatWr(0x081C);	//0x91
	LCD_SSD2828_DatWr(0x1C08);
	LCD_SSD2828_DatWr(0x004D);
	TIMER_Delay_ms(5);
	
	//! u8 lcd_init_B4[]={0xB4,0x00,0xFF,0x51,
	//!					  0x5A,0x59,0x5A,0x03,
	//!					  0x5A,0x01,0x70,0x20,0x70};	
	LCD_SSD2828_RegWr(0xBC,0x000D);
	LCD_SSD2828_RegWr(0xBF,0x00B4);
	LCD_SSD2828_DatWr(0x51FF);
	LCD_SSD2828_DatWr(0x595A);
	LCD_SSD2828_DatWr(0x035A);
	LCD_SSD2828_DatWr(0x015A);
	LCD_SSD2828_DatWr(0x2070);
	LCD_SSD2828_DatWr(0x0070);
	TIMER_Delay_ms(5);

	//! u8 lcd_init_BC[]={0xBC,0x07};	
	LCD_SSD2828_RegWr(0xBC,0x0002);
	LCD_SSD2828_RegWr(0xBF,0x07BC);
	TIMER_Delay_ms(5);
	
	//! u8 lcd_init_BF[]={0xBF,0x41,0x0E,0x01};	
	LCD_SSD2828_RegWr(0xBC,0x0004);
	LCD_SSD2828_RegWr(0xBF,0x41BF);
	LCD_SSD2828_DatWr(0x010E);
	TIMER_Delay_ms(5);
	
	//!u8 lcd_init_D3[]={0xD3,0x00,0x0F,0x00,
	//!					 0x40,0x07,0x10,0x00,
	//!					 0x08,0x10,0x08,0x00,
	//!					 0x08,0x54,0x15,0x0E,
	//!					 0x05,0x0E,0x02,0x15,
	//!					 0x06,0x05,0x06,0x47,
	//!					 0x44,0x0A,0x0A,0x4B,
	//!					 0x10,0x07,0x07};	
	LCD_SSD2828_RegWr(0xBC,0x001F);
	LCD_SSD2828_RegWr(0xBF,0x00D3);
	LCD_SSD2828_DatWr(0x000F);
	LCD_SSD2828_DatWr(0x0740);
	LCD_SSD2828_DatWr(0x0010);
	LCD_SSD2828_DatWr(0x1008);
	LCD_SSD2828_DatWr(0x0008);
	LCD_SSD2828_DatWr(0x5408);
	LCD_SSD2828_DatWr(0x0E15);
	LCD_SSD2828_DatWr(0x0E05);
	LCD_SSD2828_DatWr(0x1502);
	LCD_SSD2828_DatWr(0x0506);
	LCD_SSD2828_DatWr(0x4706);
	LCD_SSD2828_DatWr(0x0A44);
	LCD_SSD2828_DatWr(0x4B0A);
	LCD_SSD2828_DatWr(0x0710);
	LCD_SSD2828_DatWr(0x0007);
	TIMER_Delay_ms(5);
	
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
	LCD_SSD2828_RegWr(0xBC,0x002D);
	LCD_SSD2828_RegWr(0xBF,0x1AD5);
	LCD_SSD2828_DatWr(0x1B1A);
	LCD_SSD2828_DatWr(0x001B);
	LCD_SSD2828_DatWr(0x0201);
	LCD_SSD2828_DatWr(0x0403);
	LCD_SSD2828_DatWr(0x0605);
	LCD_SSD2828_DatWr(0x0807);
	LCD_SSD2828_DatWr(0x0A09);
	LCD_SSD2828_DatWr(0x240B);
	LCD_SSD2828_DatWr(0x1825);
	LCD_SSD2828_DatWr(0x2618);
	LCD_SSD2828_DatWr(0x1827);
	LCD_SSD2828_DatWr(0x1818);
	LCD_SSD2828_DatWr(0x1818);
	LCD_SSD2828_DatWr(0x1818);
	LCD_SSD2828_DatWr(0x1818);
	LCD_SSD2828_DatWr(0x1818);
	LCD_SSD2828_DatWr(0x1818);
	LCD_SSD2828_DatWr(0x1818);
	LCD_SSD2828_DatWr(0x2018);
	LCD_SSD2828_DatWr(0x1821);
	LCD_SSD2828_DatWr(0x1818);
	LCD_SSD2828_DatWr(0x0018);	
	TIMER_Delay_ms(5);
	
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
	LCD_SSD2828_RegWr(0xBC,0x002D);
	LCD_SSD2828_RegWr(0xBF,0x1AD6);
	LCD_SSD2828_DatWr(0x1B1A);
	LCD_SSD2828_DatWr(0x0B1B);
	LCD_SSD2828_DatWr(0x090A);
	LCD_SSD2828_DatWr(0x0708);
	LCD_SSD2828_DatWr(0x0506);
	LCD_SSD2828_DatWr(0x0304);
	LCD_SSD2828_DatWr(0x0102);
	LCD_SSD2828_DatWr(0x2100);
	LCD_SSD2828_DatWr(0x5820);
	LCD_SSD2828_DatWr(0x2758);
	LCD_SSD2828_DatWr(0x1826);
	LCD_SSD2828_DatWr(0x1818);
	LCD_SSD2828_DatWr(0x1818);
	LCD_SSD2828_DatWr(0x1818);
	LCD_SSD2828_DatWr(0x1818);
	LCD_SSD2828_DatWr(0x1818);
	LCD_SSD2828_DatWr(0x1818);
	LCD_SSD2828_DatWr(0x1818);
	LCD_SSD2828_DatWr(0x2518);
	LCD_SSD2828_DatWr(0x1824);
	LCD_SSD2828_DatWr(0x1818);
	LCD_SSD2828_DatWr(0x0018);
	TIMER_Delay_ms(5);

	//! u8 lcd_init_C6[]={0xC6,0xBD};	
	LCD_SSD2828_RegWr(0xBC,0x0002);
	LCD_SSD2828_RegWr(0xBF,0xBDC6);
	TIMER_Delay_ms(5);
	
	//! u8 lcd_init_BD[]={0xBD,0x02};
	LCD_SSD2828_RegWr(0xBC,0x0002);
	LCD_SSD2828_RegWr(0xBF,0x02BD);
	TIMER_Delay_ms(5);
	
	//! u8 lcd_init_D8[]={0xD8,0xFF,0xFF,0xEE,
	//!    				  0xEB,0xFB,0xA0,0xFF,
	//!					  0xFF,0xEE,0xEB,0xFB,0xA0};
	LCD_SSD2828_RegWr(0xBC,0x000D);
	LCD_SSD2828_RegWr(0xBF,0xFFD8);
	LCD_SSD2828_DatWr(0xEEFF);
	LCD_SSD2828_DatWr(0xFBEB);
	LCD_SSD2828_DatWr(0xFFA0);
	LCD_SSD2828_DatWr(0xEEFF);
	LCD_SSD2828_DatWr(0xFBEB);
	LCD_SSD2828_DatWr(0x00A0);		
	TIMER_Delay_ms(5);
	
	//! u8 lcd_init_CC[]={0xCC,0x01};	
	LCD_SSD2828_RegWr(0xBC,0x0002);
	LCD_SSD2828_RegWr(0xBF,0x01CC);
	TIMER_Delay_ms(5);
	
	//! u8 lcd_init_C0[]={0xC0,0x30,0x14};	
	LCD_SSD2828_RegWr(0xBC,0x0003);
	LCD_SSD2828_RegWr(0xBF,0x30C0);
	LCD_SSD2828_DatWr(0x0014);
	TIMER_Delay_ms(5);
	
	//! u8 lcd_init_C7[]={0xC7,0x00,0xC0,0x40,0xC0};
	LCD_SSD2828_RegWr(0xBC,0x0005);
	LCD_SSD2828_RegWr(0xBF,0x00C7);
	LCD_SSD2828_DatWr(0x40C0);
	LCD_SSD2828_DatWr(0x00C0);	
	TIMER_Delay_ms(5);
	
	//! u8 lcd_init_B6[]={0xB6,0x64,0x64};
	LCD_SSD2828_RegWr(0xBC,0x0003);
	LCD_SSD2828_RegWr(0xBF,0x64B6);
	LCD_SSD2828_DatWr(0x0064);
	TIMER_Delay_ms(5);
	
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
	LCD_SSD2828_RegWr(0xBC,0x002B);
	LCD_SSD2828_RegWr(0xBF,0x00E0);
	LCD_SSD2828_DatWr(0x1912);
	LCD_SSD2828_DatWr(0x2A23);
	LCD_SSD2828_DatWr(0x253F);
	LCD_SSD2828_DatWr(0x0842);
	LCD_SSD2828_DatWr(0x0D0A);
	LCD_SSD2828_DatWr(0x0E18);
	LCD_SSD2828_DatWr(0x1210);
	LCD_SSD2828_DatWr(0x1210);
	LCD_SSD2828_DatWr(0x160C);
	LCD_SSD2828_DatWr(0x1C18);
	LCD_SSD2828_DatWr(0x1200);
	LCD_SSD2828_DatWr(0x2319);
	LCD_SSD2828_DatWr(0x3F2A);
	LCD_SSD2828_DatWr(0x4225);
	LCD_SSD2828_DatWr(0x0A08);
	LCD_SSD2828_DatWr(0x180D);
	LCD_SSD2828_DatWr(0x100E);
	LCD_SSD2828_DatWr(0x1012);
	LCD_SSD2828_DatWr(0x0C12);
	LCD_SSD2828_DatWr(0x1816);
	LCD_SSD2828_DatWr(0x001C);	
	TIMER_Delay_ms(5);

	//! mipi_0data(0x11);	
	LCD_SSD2828_RegWr(0xBC,0x0001);
	LCD_SSD2828_RegWr(0xBF,0x0011);
	TIMER_Delay_ms(120);

	//! mipi_0data(0xCC);	
	LCD_SSD2828_RegWr(0xBC,0x0002);
	LCD_SSD2828_RegWr(0xBF,0x05CC);		//1001 1101 0101 0001
	TIMER_Delay_ms(1);

	//! mipi_0data(0x29);	
	LCD_SSD2828_RegWr(0xBC,0x0001);
	LCD_SSD2828_RegWr(0xBF,0x0029);
	TIMER_Delay_ms(1);

	LCD_SSD2828_RegWr(0xBC,0x0001);
	LCD_SSD2828_RegWr(0xBF,0x0022);
	TIMER_Delay_ms(1);

	return true;
}
//------------------------------------------------------------------------------
void LCD_MIPI_SSD2828_Sleep (void)
{
   //! mipi_0data(0x10);
   LCD_SSD2828_RegWr(0xBC,0x0001);
   LCD_SSD2828_RegWr(0xBF,0x0010);
   LCD_SSD2828_RegWr(0xB7,0x0342); 	//!< TXD[11]: Transmit on
									//!< LPE[10]: Short Packet
									//!< EOT[9]: Send EOT Packet at the end of HS transmission
									//!< ECD[8]: Disable ECC CRC Check
									//!< REN[7]: Write operation
									//!< DCS[6]: Generic packet
									//!< CSS[5]: The clock source is tx_clk
									//!< HCLK[4]: HS clock is enabled
									//!< VEN[3]: Video mode is disabled
									//!< SLP[2]: Sleep mode is disabled
									//!< CKE[1]: Clock Lane Enable
									//!< HS[0]: LP mode
}
//------------------------------------------------------------------------------
void LCD_MIPI_SSD2828_Wakeup (void)
{
   //! mipi_0data(0x11);
	LCD_SSD2828_RegWr(0xBC,0x0001);
	LCD_SSD2828_RegWr(0xBF,0x0011);
	LCD_SSD2828_RegWr(0xBC,0x0001);
	LCD_SSD2828_RegWr(0xBF,0x0022);
	TIMER_Delay_ms(1);
	LCD_MIPI_SSD2828_Start();
}
//------------------------------------------------------------------------------
void LCD_MIPI_SSD2828_Start (void)
{
	LCD_SSD2828_RegWr(0xB7,0x0349);		//!< TXD[11]: Transmit on
										//!< LPE[10]: Short Packet
										//!< EOT[9]: Send EOT Packet at the end of HS transmission
										//!< ECD[8]: Disable ECC CRC Check
										//!< REN[7]: Write operation
										//!< DCS[6]: DCS packet
										//!< CSS[5]: The clock source is tx_clk
										//!< HCLK[4]: HS clock is enabled
										//!< VEN[3]: Video mode is enabled
										//!< SLP[2]: Sleep mode is disabled
										//!< CKE[1]: Clock Lane Enable
										//!< HS[0]: HS mode
}
//------------------------------------------------------------------------------
void LCD_MIPI_SSD2828 (void)
{
	//! MIPI SSD2828
	printd(DBG_Debug3Lvl, "LCD MIPI SSD2828\n");
	printd(DBG_Debug3Lvl, "DE Mode\n");
	printd(DBG_Debug3Lvl, "Screen Size 720 x 1280\n");
	//! LCD Setting
	LCD->LCD_MODE = LCD_DE;
	LCD->SEL_TV = 0;		

	LCD->LCD_HO_SIZE = 720;
	LCD->LCD_VO_SIZE = 1280;	
	//! Timing
	LCD->LCD_HT_DM_SIZE = 172;
	LCD->LCD_VT_DM_SIZE = 20;
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
	LCD->LCD_PCK_RIS = 1;	
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
	GLB->LCDPLL_FRA = (0.37 * (1 << 20));
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
