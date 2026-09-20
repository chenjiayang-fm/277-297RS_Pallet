/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		LCD_SSD2828_YN550FH002N.c
	\brief		LCD SSD2828 YN550FH002N Funcation
	\author		Pierce
	\version	0.1
	\date		2019/01/10
	\copyright	Copyright(C) 2019 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#include <stdio.h>
#include <stdint.h>
#include "LCD.h"
#include "TIMER.h"
#include "SPI.h"
//------------------------------------------------------------------------------
#if (LCD_PANEL == LCD_SSD2828_YN550FH002N)

#if 1
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
bool bLCD_MIPI_SSD2828_Init (void)
{
	uint16_t uwId;
    SPI_Setup_t spi_setup;

	printd(DBG_Debug3Lvl,"MIPI SSD2828 Init\n");

    spi_setup.ubSPI_CPOL    = 0;
    spi_setup.ubSPI_CPHA    = 0;
    spi_setup.tSPI_Mode     = SPI_MASTER;
    spi_setup.uwClkDiv      = ((float)160 / GLB->APBC_RATE) + 0.99 - 1;         // SPI Clock <= 1.5MHz
    
    plLCD_SpiData = osUncachedMalloc(4);
    if(plLCD_SpiData == NULL)
    {
        printd(DBG_CriticalLvl, "LCD: osUncachedMalloc fail!!\n");
        while(1);
    }
	SPI_Init(&spi_setup);
	TIMER_Delay_ms(20);
	if (0x2828 != (uwId = LCD_SSD2828_RegRd(0xB0)))
	{
		printd(DBG_ErrorLvl, "LCD: SSD2828 Fail %X\n", uwId);
		return false;
	}
	//==========================================================================
	LCD_SSD2828_RegWr(0xB1, 0x0419);   // VSA = 4,HSA = 120           
	LCD_SSD2828_RegWr(0xB2, 0x0D37);   // VBP = 3,HBP = 120         
	LCD_SSD2828_RegWr(0xB3, 0x0496);   // VFP = 9,HFP = 16         
	LCD_SSD2828_RegWr(0xB4, 0x0438);   // HACT = 720                
	LCD_SSD2828_RegWr(0xB5, 0x0780);   // VACT = 1280               
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
	LCD_SSD2828_RegWr(0xBA,0xC053);   // 10-251<Fout<500,MS=1,NS=22                                                                    
	LCD_SSD2828_RegWr(0xBB,0x000D);   // LP mode clock, Divide by 6                                                                    
	LCD_SSD2828_RegWr(0xD5,0x1860);                                                                                                      
	LCD_SSD2828_RegWr(0xC9,0x2A0C);   // HS Zero Delay = HZD * nibble_clk ; HS Prepare Delay =  4 nibble_clk + HPD * nibble_clk        
	LCD_SSD2828_RegWr(0xCA,0x4309);   // CLK Zero Delay = CZD * nibble_clk ; CLK Prepare Delay =  3 nibble_clk + CPD * nibble_clk      
	LCD_SSD2828_RegWr(0xCB,0x0C49);   // CLK Pre Delay = CPED * nibble_clk + 0-1 * lp_clk(min,max) ; CLK Post Delay = CPTD * nibble_clk
	LCD_SSD2828_RegWr(0xCC,0x1314);   // CLK Trail Delay = CTD * nibble_clk ; HS Trail Delay = HTD * nibble_clk                        
	LCD_SSD2828_RegWr(0xDE,0x0003);   // 4 lane mode                                                                                   
	LCD_SSD2828_RegWr(0xB9,0x0001);   // Divide by 1,Enable Sys_clk output,PLL enable                                                  	
	LCD_SSD2828_RegWr(0xD6,0x0005);	  // RGB,R is in the higher portion of the pixel.	
	LCD_SSD2828_RegWr(0xB8,0x0000);

	//! Driver IC Initial Code
	TIMER_Delay_ms(5);
	
	//==========================================================================
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

#if 0
	//==========================================================================
	LCD_SSD2828_RegWr(0xBC,4);		//!< Transmit Data Count
	//!SPI_3W_SET_CMD(0xBF);
	//!SPI_3W_SET_PAs(0xB9);
	//!SPI_3W_SET_PAs(0xFF);
	LCD_SSD2828_RegWr(0xBF,0xFFB9);	//!< Data
	//!SPI_3W_SET_PAs(0x83);
	//!SPI_3W_SET_PAs(0x99);
	LCD_SSD2828_DatWr(0x9983);		//!< Data
	TIMER_Delay_ms(5);

	//==========================================================================
	LCD_SSD2828_RegWr(0xBC,2);
	//!SPI_3W_SET_CMD(0xBF);
	//!SPI_3W_SET_PAs(0xD2);
	//!SPI_3W_SET_PAs(0x77);
	LCD_SSD2828_RegWr(0xBF,0x77D2);
	TIMER_Delay_ms(5);
	
	//==========================================================================
	LCD_SSD2828_RegWr(0xBC,0x10);
	//!SPI_3W_SET_CMD(0xBF);
	//!SPI_3W_SET_PAs(0xB1);
	//!SPI_3W_SET_PAs(0x02);
	LCD_SSD2828_RegWr(0xBF,0x2B1);
	//!SPI_3W_SET_PAs(0x04);
	//!SPI_3W_SET_PAs(0x74);
	LCD_SSD2828_DatWr(0x7404);
	//!SPI_3W_SET_PAs(0x94);
	//!SPI_3W_SET_PAs(0x01);
	LCD_SSD2828_DatWr(0x194);
	//!SPI_3W_SET_PAs(0x32);
	//!SPI_3W_SET_PAs(0x33);
	LCD_SSD2828_DatWr(0x3332);
	//!SPI_3W_SET_PAs(0x11);
	//!SPI_3W_SET_PAs(0x11);
	LCD_SSD2828_DatWr(0x1111);
	//!SPI_3W_SET_PAs(0xAB);
	//!SPI_3W_SET_PAs(0x4D);
	LCD_SSD2828_DatWr(0x4DAB);
	//!SPI_3W_SET_PAs(0x56);
	//!SPI_3W_SET_PAs(0x73);
	LCD_SSD2828_DatWr(0x7356);
	//!SPI_3W_SET_PAs(0x02);
	//!SPI_3W_SET_PAs(0x02);
	LCD_SSD2828_DatWr(0x202);
	TIMER_Delay_ms(5);
	
	//==========================================================================
	LCD_SSD2828_RegWr(0xBC,0x10);
	//!SPI_3W_SET_CMD(0xBF);
	//!SPI_3W_SET_PAs(0xB2);
	//!SPI_3W_SET_PAs(0x00);
	LCD_SSD2828_RegWr(0xBF,0xB2);
	//!SPI_3W_SET_PAs(0x80);
	//!SPI_3W_SET_PAs(0x80);
	LCD_SSD2828_DatWr(0x8080);
	//!SPI_3W_SET_PAs(0xAE);
	//!SPI_3W_SET_PAs(0x05);
	LCD_SSD2828_DatWr(0x5AE);
	//!SPI_3W_SET_PAs(0x07);
	//!SPI_3W_SET_PAs(0x5A);
	LCD_SSD2828_DatWr(0x5A07);	//0x91
	//!SPI_3W_SET_PAs(0x11);
	//!SPI_3W_SET_PAs(0x00);
	LCD_SSD2828_DatWr(0x11);
	//!SPI_3W_SET_PAs(0x00);
	//!SPI_3W_SET_PAs(0x10);
	LCD_SSD2828_DatWr(0x1000);
	//!SPI_3W_SET_PAs(0x1E);
	//!SPI_3W_SET_PAs(0x70);
	LCD_SSD2828_DatWr(0x701E);
	//!SPI_3W_SET_PAs(0x03);
	//!SPI_3W_SET_PAs(0xD4);
	LCD_SSD2828_DatWr(0xD403);
	TIMER_Delay_ms(5);
	
	//==========================================================================
	LCD_SSD2828_RegWr(0xBC,0x2D);
	//!SPI_3W_SET_CMD(0xBF);
	//!SPI_3W_SET_PAs(0xB4);
	//!SPI_3W_SET_PAs(0x00);
	LCD_SSD2828_RegWr(0xBF,0xB4);
	//!SPI_3W_SET_PAs(0xFF);
	//!SPI_3W_SET_PAs(0x02);
	LCD_SSD2828_DatWr(0x2FF);
	//!SPI_3W_SET_PAs(0xC0);
	//!SPI_3W_SET_PAs(0x02);
	LCD_SSD2828_DatWr(0x2C0);
	//!SPI_3W_SET_PAs(0xC0);
	//!SPI_3W_SET_PAs(0x00);
	LCD_SSD2828_DatWr(0xC0);
	//!SPI_3W_SET_PAs(0x00);
	//!SPI_3W_SET_PAs(0x08);
	LCD_SSD2828_DatWr(0x800);
	//!SPI_3W_SET_PAs(0x00);
	//!SPI_3W_SET_PAs(0x04);
	LCD_SSD2828_DatWr(0x400);
	//!SPI_3W_SET_PAs(0x06);
	//!SPI_3W_SET_PAs(0x00);
	LCD_SSD2828_DatWr(6);
	//!SPI_3W_SET_PAs(0x32);
	//!SPI_3W_SET_PAs(0x04);
	LCD_SSD2828_DatWr(0x432);
	//!SPI_3W_SET_PAs(0x0A);
	//!SPI_3W_SET_PAs(0x08);
	LCD_SSD2828_DatWr(0x80A);
	//!SPI_3W_SET_PAs(0x21);
	//!SPI_3W_SET_PAs(0x03);
	LCD_SSD2828_DatWr(0x321);
	//!SPI_3W_SET_PAs(0x01);
	//!SPI_3W_SET_PAs(0x00);
	LCD_SSD2828_DatWr(1);
	//!SPI_3W_SET_PAs(0x0F);
	//!SPI_3W_SET_PAs(0xB8);
	LCD_SSD2828_DatWr(0xB80F);
	//!SPI_3W_SET_PAs(0x8B);
	//!SPI_3W_SET_PAs(0x02);
	LCD_SSD2828_DatWr(0x28B);
	//!SPI_3W_SET_PAs(0xC0);
	//!SPI_3W_SET_PAs(0x02);
	LCD_SSD2828_DatWr(0x2C0);
	//!SPI_3W_SET_PAs(0xC0);
	//!SPI_3W_SET_PAs(0x00);
	LCD_SSD2828_DatWr(0xC0);
	//!SPI_3W_SET_PAs(0x00);
	//!SPI_3W_SET_PAs(0x08);
	LCD_SSD2828_DatWr(0x800);
	//!SPI_3W_SET_PAs(0x00);
	//!SPI_3W_SET_PAs(0x04);
	LCD_SSD2828_DatWr(0x400);
	//!SPI_3W_SET_PAs(0x06);
	//!SPI_3W_SET_PAs(0x00);
	LCD_SSD2828_DatWr(6);
	//!SPI_3W_SET_PAs(0x32);
	//!SPI_3W_SET_PAs(0x04);
	LCD_SSD2828_DatWr(0x432);
	//!SPI_3W_SET_PAs(0x0A);
	//!SPI_3W_SET_PAs(0x08);
	LCD_SSD2828_DatWr(0x80A);
	//!SPI_3W_SET_PAs(0x01);
	//!SPI_3W_SET_PAs(0x00);
	LCD_SSD2828_DatWr(1);
	//!SPI_3W_SET_PAs(0x0F);
	//!SPI_3W_SET_PAs(0xB8);
	LCD_SSD2828_DatWr(0xB80F);
	//!SPI_3W_SET_PAs(0x01);
	LCD_SSD2828_DatWr(1);
	TIMER_Delay_ms(5);

	//==========================================================================	
	LCD_SSD2828_RegWr(0xBC,0x22);
	//!SPI_3W_SET_CMD(0xBF);
	//!SPI_3W_SET_PAs(0xD3);
	//!SPI_3W_SET_PAs(0x00);
	LCD_SSD2828_RegWr(0xBF,0xD3);
	//!SPI_3W_SET_PAs(0x00);
	//!SPI_3W_SET_PAs(0x00);
	LCD_SSD2828_DatWr(0);
	//!SPI_3W_SET_PAs(0x00);
	//!SPI_3W_SET_PAs(0x00);
	LCD_SSD2828_DatWr(0);
	//!SPI_3W_SET_PAs(0x00);
	//!SPI_3W_SET_PAs(0x06);
	LCD_SSD2828_DatWr(0x600);
	//!SPI_3W_SET_PAs(0x00);
	//!SPI_3W_SET_PAs(0x00);
	LCD_SSD2828_DatWr(0);
	//!SPI_3W_SET_PAs(0x10);
	//!SPI_3W_SET_PAs(0x04);
	LCD_SSD2828_DatWr(0x410);
	//!SPI_3W_SET_PAs(0x00);
	//!SPI_3W_SET_PAs(0x04);
	LCD_SSD2828_DatWr(0x400);
	//!SPI_3W_SET_PAs(0x00);
	//!SPI_3W_SET_PAs(0x00);
	LCD_SSD2828_DatWr(0);
	//!SPI_3W_SET_PAs(0x00);
	//!SPI_3W_SET_PAs(0x00);
	LCD_SSD2828_DatWr(0);
	//!SPI_3W_SET_PAs(0x00);
	//!SPI_3W_SET_PAs(0x00);
	LCD_SSD2828_DatWr(0);
	//!SPI_3W_SET_PAs(0x00);
	//!SPI_3W_SET_PAs(0x00);
	LCD_SSD2828_DatWr(0);
	//!SPI_3W_SET_PAs(0x00);
	//!SPI_3W_SET_PAs(0x00);
	LCD_SSD2828_DatWr(0);
	//!SPI_3W_SET_PAs(0x01);
	//!SPI_3W_SET_PAs(0x00);
	LCD_SSD2828_DatWr(1);
	//!SPI_3W_SET_PAs(0x05);
	//!SPI_3W_SET_PAs(0x05);
	LCD_SSD2828_DatWr(0x505);
	//!SPI_3W_SET_PAs(0x07);
	//!SPI_3W_SET_PAs(0x00);
	LCD_SSD2828_DatWr(7);
	//!SPI_3W_SET_PAs(0x00);
	//!SPI_3W_SET_PAs(0x00);
	LCD_SSD2828_DatWr(0);
	//!SPI_3W_SET_PAs(0x05);
	//!SPI_3W_SET_PAs(0x40);
	LCD_SSD2828_DatWr(0x4005);
	TIMER_Delay_ms(5);
	
	//==========================================================================
	LCD_SSD2828_RegWr(0xBC,0x21);
	//!	SPI_3W_SET_CMD(0xBF);
	//! SPI_3W_SET_PAs(0xD5);
	//! SPI_3W_SET_PAs(0x18);
	LCD_SSD2828_RegWr(0xBF,0x18D5);
	//! SPI_3W_SET_PAs(0x18);
	//! SPI_3W_SET_PAs(0x19);
	LCD_SSD2828_DatWr(0x1918);
	//! SPI_3W_SET_PAs(0x19);
	//! SPI_3W_SET_PAs(0x18);
	LCD_SSD2828_DatWr(0x1819);
	//! SPI_3W_SET_PAs(0x18);
	//! SPI_3W_SET_PAs(0x21);
	LCD_SSD2828_DatWr(0x2118);
	//! SPI_3W_SET_PAs(0x20);
	//! SPI_3W_SET_PAs(0x01);
	LCD_SSD2828_DatWr(0x120);
	//! SPI_3W_SET_PAs(0x00);
	//! SPI_3W_SET_PAs(0x07);
	LCD_SSD2828_DatWr(0x700);
	//! SPI_3W_SET_PAs(0x06);
	//! SPI_3W_SET_PAs(0x05);
	LCD_SSD2828_DatWr(0x506);
	//! SPI_3W_SET_PAs(0x04);
	//! SPI_3W_SET_PAs(0x03);
	LCD_SSD2828_DatWr(0x304);
	//! SPI_3W_SET_PAs(0x02);
	//! SPI_3W_SET_PAs(0x18);
	LCD_SSD2828_DatWr(0x1802);
	//! SPI_3W_SET_PAs(0x18);
	//! SPI_3W_SET_PAs(0x18);
	LCD_SSD2828_DatWr(0x1818);
	//! SPI_3W_SET_PAs(0x18);
	//! SPI_3W_SET_PAs(0x18);
	LCD_SSD2828_DatWr(0x1818);
	//! SPI_3W_SET_PAs(0x18);
	//! SPI_3W_SET_PAs(0x2F);
	LCD_SSD2828_DatWr(0x2F18);
	//! SPI_3W_SET_PAs(0x2F);
	//! SPI_3W_SET_PAs(0x30);
	LCD_SSD2828_DatWr(0x302F);
	//! SPI_3W_SET_PAs(0x30);
	//! SPI_3W_SET_PAs(0x31);
	LCD_SSD2828_DatWr(0x3130);
	//! SPI_3W_SET_PAs(0x31);
	//! SPI_3W_SET_PAs(0x18);
	LCD_SSD2828_DatWr(0x1831);
	//! SPI_3W_SET_PAs(0x18);
	//! SPI_3W_SET_PAs(0x18);
	LCD_SSD2828_DatWr(0x1818);
	//! SPI_3W_SET_PAs(0x18);
	LCD_SSD2828_DatWr(0x18);
	TIMER_Delay_ms(5);
	
	//==========================================================================
	LCD_SSD2828_RegWr(0xBC,0x21);
	//! SPI_3W_SET_CMD(0xBF);
	//! SPI_3W_SET_PAs(0xD6);
	//! SPI_3W_SET_PAs(0x18);
	LCD_SSD2828_RegWr(0xBF,0x18D6);
	//! SPI_3W_SET_PAs(0x18);
	//! SPI_3W_SET_PAs(0x19);
	LCD_SSD2828_DatWr(0x1918);
	//! SPI_3W_SET_PAs(0x19);
	//! SPI_3W_SET_PAs(0x40);
	LCD_SSD2828_DatWr(0x4019);
	//! SPI_3W_SET_PAs(0x40);
	//! SPI_3W_SET_PAs(0x20);
	LCD_SSD2828_DatWr(0x2040);
	//! SPI_3W_SET_PAs(0x21);
	//! SPI_3W_SET_PAs(0x06);
	LCD_SSD2828_DatWr(0x621);
	//! SPI_3W_SET_PAs(0x07);
	//! SPI_3W_SET_PAs(0x00);
	LCD_SSD2828_DatWr(7);
	//! SPI_3W_SET_PAs(0x01);
	//! SPI_3W_SET_PAs(0x02);
	LCD_SSD2828_DatWr(0x201);
	//! SPI_3W_SET_PAs(0x03);
	//! SPI_3W_SET_PAs(0x04);
	LCD_SSD2828_DatWr(0x403);
	//! SPI_3W_SET_PAs(0x05);
	//! SPI_3W_SET_PAs(0x40);
	LCD_SSD2828_DatWr(0x4005);
	//! SPI_3W_SET_PAs(0x40);
	//! SPI_3W_SET_PAs(0x40);
	LCD_SSD2828_DatWr(0x4040);
	//! SPI_3W_SET_PAs(0x40);
	//! SPI_3W_SET_PAs(0x40);
	LCD_SSD2828_DatWr(0x4040);
	//! SPI_3W_SET_PAs(0x40);
	//! SPI_3W_SET_PAs(0x2F);
	LCD_SSD2828_DatWr(0x2F40);
	//! SPI_3W_SET_PAs(0x2F);
	//! SPI_3W_SET_PAs(0x30);
	LCD_SSD2828_DatWr(0x302F);
	//! SPI_3W_SET_PAs(0x30);
	//! SPI_3W_SET_PAs(0x31);
	LCD_SSD2828_DatWr(0x3130);
	//! SPI_3W_SET_PAs(0x31);
	//! SPI_3W_SET_PAs(0x40);
	LCD_SSD2828_DatWr(0x4031);
	//! SPI_3W_SET_PAs(0x40);
	//! SPI_3W_SET_PAs(0x40);
	LCD_SSD2828_DatWr(0x4040);
	//! SPI_3W_SET_PAs(0x40);
	LCD_SSD2828_DatWr(0x40);
	TIMER_Delay_ms(5);
	
	//==========================================================================
	LCD_SSD2828_RegWr(0xBC,0x11);
	//! SPI_3W_SET_CMD(0xBF);
	//! SPI_3W_SET_PAs(0xD8);
	//! SPI_3W_SET_PAs(0xA2);
	LCD_SSD2828_RegWr(0xBF,0xA2D8);
	//! SPI_3W_SET_PAs(0xAA);
	//! SPI_3W_SET_PAs(0x02);
	LCD_SSD2828_DatWr(0x2AA);
	//! SPI_3W_SET_PAs(0xA0);
	//! SPI_3W_SET_PAs(0xA2);
	LCD_SSD2828_DatWr(0xA2A0);
	//! SPI_3W_SET_PAs(0xA8);
	//! SPI_3W_SET_PAs(0x02);
	LCD_SSD2828_DatWr(0x2A8);
	//! SPI_3W_SET_PAs(0xA0);
	//! SPI_3W_SET_PAs(0xB0);
	LCD_SSD2828_DatWr(0xB0A0);
	//! SPI_3W_SET_PAs(0x00);
	//! SPI_3W_SET_PAs(0x00);
	LCD_SSD2828_DatWr(0);
	//! SPI_3W_SET_PAs(0x00);
	//! SPI_3W_SET_PAs(0xB0);
	LCD_SSD2828_DatWr(0xB000);
	//! SPI_3W_SET_PAs(0x00);
	//! SPI_3W_SET_PAs(0x00);
	LCD_SSD2828_DatWr(0);
	//! SPI_3W_SET_PAs(0x00);
	LCD_SSD2828_DatWr(0);
	TIMER_Delay_ms(5);
	
	//==========================================================================
	LCD_SSD2828_RegWr(0xBC,2);
	//! SPI_3W_SET_CMD(0xBF);
	//! SPI_3W_SET_PAs(0xBD);
	//! SPI_3W_SET_PAs(0x01);
	LCD_SSD2828_RegWr(0xBF,0x1BD);	
	TIMER_Delay_ms(5);

	//==========================================================================
	LCD_SSD2828_RegWr(0xBC,0x11);
	//! SPI_3W_SET_CMD(0xBF);
	//! SPI_3W_SET_PAs(0xD8);
	//! SPI_3W_SET_PAs(0xB0);
	LCD_SSD2828_RegWr(0xBF,0xB0D8);
	//! SPI_3W_SET_PAs(0x00);
	//! SPI_3W_SET_PAs(0x00);
	LCD_SSD2828_DatWr(0);
	//! SPI_3W_SET_PAs(0x00);
	//! SPI_3W_SET_PAs(0xB0);
	LCD_SSD2828_DatWr(0xB000);
	//! SPI_3W_SET_PAs(0x00);
	//! SPI_3W_SET_PAs(0x00);
	LCD_SSD2828_DatWr(0);
	//! SPI_3W_SET_PAs(0x00);
	//! SPI_3W_SET_PAs(0xE2);
	LCD_SSD2828_DatWr(0xE200);
	//! SPI_3W_SET_PAs(0xAA);
	//! SPI_3W_SET_PAs(0x03);
	LCD_SSD2828_DatWr(0x3AA);
	//! SPI_3W_SET_PAs(0xF0);
	//! SPI_3W_SET_PAs(0xE2);
	LCD_SSD2828_DatWr(0xE2F0);	
	//! SPI_3W_SET_PAs(0xAA);
	//! SPI_3W_SET_PAs(0x03);
	LCD_SSD2828_DatWr(0x3AA);	
	//! SPI_3W_SET_PAs(0xF0);
	LCD_SSD2828_DatWr(0xF0);	
	TIMER_Delay_ms(5);
	
	//==========================================================================
	LCD_SSD2828_RegWr(0xBC,2);
	//! SPI_3W_SET_CMD(0xBF);
	//! SPI_3W_SET_PAs(0xBD);
	//! SPI_3W_SET_PAs(0x02);
	LCD_SSD2828_RegWr(0xBF,0x2BD);
	TIMER_Delay_ms(5);
	
	//==========================================================================
	LCD_SSD2828_RegWr(0xBC,9);
	//! SPI_3W_SET_CMD(0xBF);
	//! SPI_3W_SET_PAs(0xD8);
	//! SPI_3W_SET_PAs(0xE2);
	LCD_SSD2828_RegWr(0xBF,0xE2D8);
	//! SPI_3W_SET_PAs(0xAA);
	//! SPI_3W_SET_PAs(0x03);
	LCD_SSD2828_DatWr(0x3AA);
	//! SPI_3W_SET_PAs(0xF0);
	//! SPI_3W_SET_PAs(0xE2);
	LCD_SSD2828_DatWr(0xE2F0);
	//! SPI_3W_SET_PAs(0xAA);
	//! SPI_3W_SET_PAs(0x03);
	LCD_SSD2828_DatWr(0x3AA);
	//! SPI_3W_SET_PAs(0xF0);
	LCD_SSD2828_DatWr(0xF0);
	TIMER_Delay_ms(5);
	
	//==========================================================================
	LCD_SSD2828_RegWr(0xBC,2);
	//! SPI_3W_SET_CMD(0xBF);
	//! SPI_3W_SET_PAs(0xBD);
	//! SPI_3W_SET_PAs(0x00);
	LCD_SSD2828_RegWr(0xBF,0xBD);
	TIMER_Delay_ms(5);
	
	//==========================================================================	
	LCD_SSD2828_RegWr(0xBC,3);
	//! SPI_3W_SET_CMD(0xBF);
	//! SPI_3W_SET_PAs(0xB6);
	//! SPI_3W_SET_PAs(0x8D);
	LCD_SSD2828_RegWr(0xBF,0x8DB6);
	//! SPI_3W_SET_PAs(0x8D);
	LCD_SSD2828_DatWr(0x8D);
	TIMER_Delay_ms(5);
	
	//==========================================================================	
	LCD_SSD2828_RegWr(0xBC,0x37);
	//! SPI_3W_SET_CMD(0xBF);
	//! SPI_3W_SET_PAs(0xE0);
	//! SPI_3W_SET_PAs(0x00);
	LCD_SSD2828_RegWr(0xBF,0xE0);
	//! SPI_3W_SET_PAs(0x0E);
	//! SPI_3W_SET_PAs(0x19);
	LCD_SSD2828_DatWr(0x190E);
	//! SPI_3W_SET_PAs(0x13);
	//! SPI_3W_SET_PAs(0x2E);
	LCD_SSD2828_DatWr(0x2E13);
	//! SPI_3W_SET_PAs(0x39);
	//! SPI_3W_SET_PAs(0x48);
	LCD_SSD2828_DatWr(0x4839);
	//! SPI_3W_SET_PAs(0x44);
	//! SPI_3W_SET_PAs(0x4D);
	LCD_SSD2828_DatWr(0x4D44);
	//! SPI_3W_SET_PAs(0x57);
	//! SPI_3W_SET_PAs(0x5F);
	LCD_SSD2828_DatWr(0x5F57);
	//! SPI_3W_SET_PAs(0x66);
	//! SPI_3W_SET_PAs(0x6C);
	LCD_SSD2828_DatWr(0x6C66);
	//! SPI_3W_SET_PAs(0x76);
	//! SPI_3W_SET_PAs(0x7F);
	LCD_SSD2828_DatWr(0x7F76);
	//! SPI_3W_SET_PAs(0x85);
	//! SPI_3W_SET_PAs(0x8A);
	LCD_SSD2828_DatWr(0x8A85);
	//! SPI_3W_SET_PAs(0x95);
	//! SPI_3W_SET_PAs(0x9A);
	LCD_SSD2828_DatWr(0x9A95);
	//! SPI_3W_SET_PAs(0xA4);
	//! SPI_3W_SET_PAs(0x9B);
	LCD_SSD2828_DatWr(0x9BA4);
	//! SPI_3W_SET_PAs(0xAB);
	//! SPI_3W_SET_PAs(0xB0);
	LCD_SSD2828_DatWr(0xB0AB);
	//! SPI_3W_SET_PAs(0x5C);
	//! SPI_3W_SET_PAs(0x58);
	LCD_SSD2828_DatWr(0x585C);
	//! SPI_3W_SET_PAs(0x64);
	//! SPI_3W_SET_PAs(0x77);
	LCD_SSD2828_DatWr(0x7764);
	//! SPI_3W_SET_PAs(0x00);
	//! SPI_3W_SET_PAs(0x0E);
	LCD_SSD2828_DatWr(0xE00);
	//! SPI_3W_SET_PAs(0x19);
	//! SPI_3W_SET_PAs(0x13);
	LCD_SSD2828_DatWr(0x1319);
	//! SPI_3W_SET_PAs(0x2E);
	//! SPI_3W_SET_PAs(0x39);
	LCD_SSD2828_DatWr(0x392E);
	//! SPI_3W_SET_PAs(0x48);
	//! SPI_3W_SET_PAs(0x44);
	LCD_SSD2828_DatWr(0x4448);
	//! SPI_3W_SET_PAs(0x4D);
	//! SPI_3W_SET_PAs(0x57);
	LCD_SSD2828_DatWr(0x574D);
	//! SPI_3W_SET_PAs(0x5F);
	//! SPI_3W_SET_PAs(0x66);
	LCD_SSD2828_DatWr(0x665F);
	//! SPI_3W_SET_PAs(0x6C);
	//! SPI_3W_SET_PAs(0x76);
	LCD_SSD2828_DatWr(0x766C);
	//! SPI_3W_SET_PAs(0x7F);
	//! SPI_3W_SET_PAs(0x85);
	LCD_SSD2828_DatWr(0x857F);
	//! SPI_3W_SET_PAs(0x8A);
	//! SPI_3W_SET_PAs(0x95);
	LCD_SSD2828_DatWr(0x958A);
	//! SPI_3W_SET_PAs(0x9A);
	//! SPI_3W_SET_PAs(0xA4);
	LCD_SSD2828_DatWr(0xA49A);
	//! SPI_3W_SET_PAs(0x9B);
	//! SPI_3W_SET_PAs(0xAB);
	LCD_SSD2828_DatWr(0xAB9B);
	//! SPI_3W_SET_PAs(0xB0);
	//! SPI_3W_SET_PAs(0x5C);
	LCD_SSD2828_DatWr(0x5CB0);
	//! SPI_3W_SET_PAs(0x58);
	//! SPI_3W_SET_PAs(0x64);
	LCD_SSD2828_DatWr(0x6458);
	//! SPI_3W_SET_PAs(0x77);
	LCD_SSD2828_DatWr(0x77);
	TIMER_Delay_ms(5);

#endif
	//==========================================================================
	//! mipi_0data(0x11);	
	LCD_SSD2828_RegWr(0xBC,1);
	LCD_SSD2828_RegWr(0xBF,0x11);
	TIMER_Delay_ms(150);
	
	LCD_SSD2828_RegWr(0xBC,1);
	LCD_SSD2828_RegWr(0xBF,0x21);
	TIMER_Delay_ms(1);
	
//	LCD_SSD2828_RegWr(0xBC,2);
//	LCD_SSD2828_RegWr(0xBF,0x336);
//	TIMER_Delay_ms(1);

	//! mipi_0data(0xCC);	
//	LCD_SSD2828_RegWr(0xBC,2);
//	LCD_SSD2828_RegWr(0xBF,0x8CC);		//1001 1101 0101 0001
//	TIMER_Delay_ms(1);

	//! mipi_0data(0x29);	
	LCD_SSD2828_RegWr(0xBC,1);
	LCD_SSD2828_RegWr(0xBF,0x29);
	TIMER_Delay_ms(1);
	
	return true;
}
//------------------------------------------------------------------------------
void LCD_MIPI_SSD2828_Sleep (void)
{
   //! mipi_0data(0x10);
   LCD_SSD2828_RegWr(0xBC,1);
   LCD_SSD2828_RegWr(0xBF,0x10);
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
	LCD_SSD2828_RegWr(0xBC,1);
	LCD_SSD2828_RegWr(0xBF,0x11);
	LCD_SSD2828_RegWr(0xBC,1);
	LCD_SSD2828_RegWr(0xBF,0x22);
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
	printd(DBG_Debug3Lvl, "Screen Size 1080 x 1920\n");
	//! LCD Setting
	LCD->LCD_MODE = LCD_DE;
	LCD->SEL_TV = 0;		

	LCD->LCD_HO_SIZE = 1080;
	LCD->LCD_VO_SIZE = 1920;	
	//! Timing
	LCD->LCD_HT_DM_SIZE = 150;
	LCD->LCD_VT_DM_SIZE = 4;
	LCD->LCD_HT_START = 25 + 30;
	LCD->LCD_VT_START = 4 + 9;
	
	LCD->LCD_HS_WIDTH = 25;
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
	uint8_t ubFps = 59;
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
	GLB->LCDPLL_CK_SEL = 2;
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
	GLB->LCDPLL_FRA = 0.2375 * (1 << 20);
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
		//while(1);
	}
}
#endif
