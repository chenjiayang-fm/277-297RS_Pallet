/*!
	\file		AHD_TP2803.c
	\brief		AHD TP2803 Funcation
	\author		
	\version	
	\date		
	\copyright	
*/
//------------------------------------------------------------------------------
#include <stdio.h>
#include <stdint.h>
#include "LCD.h"
#include "I2C.h"
#include "TIMER.h"
#include "LCD_Type.h"
//------------------------------------------------------------------------------
I2C1_Type *pLCD_I2C_type;
I2C_TYP I2C_Sel = I2C_2;

uint8_t ubAHD_InitTable[] = {
	// 720P@30 BT656
	// Slave address is 0x8A
	// Register, data
	//0x82, 0x02, 0xcb /*0x8b*/, // test pattern ?

	/* 576i */
	0x82, 0x02, 0x1A,
	//0x82, 0x02, 0x5A,	// test pattern
	0x82, 0x03, 0x43,
	0x82, 0x05, 0x20,
	0x82, 0x07, 0x81,
	0x82, 0x08, 0x20,
	0x82, 0x09, 0x42,
	0x82, 0x0a, 0x28,
	0x82, 0x0b, 0x05,
	0x82, 0x0c, 0x04,
	0x82, 0x0d, 0x04,
	0x82, 0x0e, 0x3C,
	0x82, 0x0f, 0x60,
	0x82, 0x10, 0x03,
	0x82, 0x11, 0x20,
	0x82, 0x12, 0x04,

	0x82, 0x1B, 0x97,
	0x82, 0x1C, 0x81,
	0x82, 0x1D, 0xB6,

	// Ricky add
	0x82, 0x24, 0x23,
	0x82, 0x25, 0x60,
	0x82, 0x26, 0xD0,
	0x82, 0x27, 0x52,
	0x82, 0x28, 0x40,
	//

	0x82, 0x41, 0x30,
	0x82, 0x42, 0x12,
	0x82, 0x43, 0x47,
	0x82, 0x44, 0x49,
	0x82, 0x45, 0xCB,
};

//------------------------------------------------------------------------------
#if (LCD_PANEL == LCD_AHD_TP2803_BT656)
bool bAHD_TP2803_Read(uint8_t ubAddress, uint8_t *pValue)
{
	return bI2C_MasterProcess (pLCD_I2C_type, AHD_SLAVE_ADDR, &ubAddress, 1, pValue, 1);
}
//------------------------------------------------------------------------------
bool bAHD_TP2803_Write(uint8_t ubAddress, uint8_t ubValue)
{
	uint8_t pBuf[2];
	
	pBuf[0] = ubAddress;
	pBuf[1] = ubValue;	
	
	//printd(DBG_ErrorLvl, "%s 0x%x 0x%x\n", __FUNCTION__, ubAddress, ubValue);	

	return bI2C_MasterProcess (pLCD_I2C_type, AHD_SLAVE_ADDR, &pBuf[0], 2, NULL, 0);
}
//------------------------------------------------------------------------------
void AHD_TP2803_Start(void)
{
	uint8_t     *pBuf, temp;
	uint16_t 	uwPID = 0;
	int i;
	
	TP2803_RESET_OUT = 1;
	TIMER_Delay_ms(100);
	TP2803_RESET_OUT = 0;
	TIMER_Delay_ms(300);
	TP2803_RESET_OUT = 1;
	TIMER_Delay_ms(100);

	pBuf = (uint8_t*)&uwPID;
	// I2C by Read Decoder ID
	bAHD_TP2803_Read (TP2803_CHIP_ID_HIGH_ADDR, &pBuf[1]);
	bAHD_TP2803_Read (TP2803_CHIP_ID_LOW_ADDR, &pBuf[0]);
	if (TP2803_CHIP_ID != uwPID)
	{
		printd(DBG_ErrorLvl, "This is not TP2803 Decoder!! 0x%x 0x%x\n", TP2803_CHIP_ID, uwPID);
        TIMER_Delay_us(10000);
//        goto _RETRY;
	}
    printd(DBG_CriticalLvl, "TP2803 Decoder <0x%X>\n", uwPID);

	for (i=0; i<sizeof(ubAHD_InitTable); i+=3)
	{
		if (ubAHD_InitTable[i] == 0x82)	// write
		{
			bAHD_TP2803_Write(ubAHD_InitTable[i+1], ubAHD_InitTable[i+2]);
			//bAHD_TP2803_Read(ubAHD_InitTable[i+1], &temp);
			//printd(DBG_ErrorLvl, "tmp: 0x%x\n", temp);
		}else if (ubAHD_InitTable[i] == 0xbb){
            TIMER_Delay_ms(((ubAHD_InitTable[i+1]<<8) + ubAHD_InitTable[i+2]));
        }
	}
}
//------------------------------------------------------------------------------
bool bAHD_TP2803_IturBt656 (void)
{
	//! TM035KDH03
	printf("AHD TP2803\n");
	printf("BT656 Mode\n");
	printf("Screen Size 720 x 576\n");

	TP2803_RESET_PADIO = 0;
	TP2803_RESET_OUT_EN = 1;
	TP2803_I2C_CLK_PADIO = 4;
	TP2803_I2C_SDA_PADIO = 4;

	pLCD_I2C_type = pI2C_MasterInit(I2C_2, I2C_SCL_400K);

	// 576i
	LCD->LCD_MODE = LCD_BT656_BT601;
	LCD->SEL_TV = 0;
	
	//! BT Mode
	LCD->BT_MODE = 2;
	LCD->BT_PROG = 0;
	LCD->BT_VS1_ST = 0;
	LCD->BT_VS1_END = 24;
	LCD->BT_VS2_ST = 312;
	LCD->BT_VS2_END = 337;
	LCD->BT_FLD1 = 2;
	LCD->BT_FLD2 = 314;

	LCD->LCD_HO_SIZE = 720;
	LCD->LCD_VO_SIZE = 576;
	//! Timing
	LCD->LCD_HT_DM_SIZE = 0;
	LCD->LCD_VT_DM_SIZE = 41;
	LCD->LCD_HT_START = 0;
	LCD->LCD_VT_START = 4;

	LCD->LCD_HS_WIDTH = 140;
	LCD->LCD_VS_WIDTH = 4;

	LCD->LCD_PCK_SPEED = 2 /*4*/;
	LCD->LCD_PCK_RIS = 1;
	LCD->LCD_FS0_FIELD_MODE = 0;
	LCD->LCD_FS1_FIELD_MODE = 0;

	return true;
}
//------------------------------------------------------------------------------
void LCD_PixelPllSetting(void)
{
	float fPixelClock;
	uint8_t ubFps = 25;
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
			fPixelClock = ((float)ubFps) * (LCD->LCD_VO_SIZE + LCD->LCD_VT_START + LCD->LCD_VS_WIDTH) *
											((LCD->LCD_HO_SIZE << 1) + (LCD->LCD_HT_START ) + (LCD->LCD_HS_WIDTH << 1) + 8) / 1000000;
			printf("[AHD] %f = (%u * (%u + %u + %u) * (%u + %u + %u + 8)) / 1000000\n",
				fPixelClock,
				ubFps, LCD->LCD_VO_SIZE, LCD->LCD_VT_START, LCD->LCD_VS_WIDTH,
				(LCD->LCD_HO_SIZE << 1), (LCD->LCD_HT_START ), (LCD->LCD_HS_WIDTH << 1));
			break;
		default:
			return;
	}

	printd(DBG_InfoLvl, "LCD Pixel Clock = %f MHz\n", fPixelClock);


	LCD->LCD_PCK_SPEED = 6;//2;
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
	GLB->LCDPLL_INT = 13;
	//GLB->LCDPLL_FRA = 0x60000;
	GLB->LCDPLL_FRA = 0x80000;//0x70A3D;//0x7bb99;
	TIMER_Delay_us(1);
	GLB->LCDPLL_INT_FRA_VLD = 1;
	TIMER_Delay_us(1);
	//! LCD Controller rate
	GLB->LCD_RATE = 1;

	fPixelClock = 27.0000;	// PCLK for TP2983 must be 27MHz

	if (GLB->LCDPLL_CK_SEL == 0)
		fFVCO = fPixelClock * LCD->LCD_PCK_SPEED * 6;
	else if (GLB->LCDPLL_CK_SEL == 1)
		fFVCO = fPixelClock * LCD->LCD_PCK_SPEED * 2;
	else
		fFVCO = fPixelClock * LCD->LCD_PCK_SPEED * 1;

	if ((fFVCO < 148.3) || (fFVCO > 165)) { //range:148.3M ~ 165M
		printd(DBG_ErrorLvl, "fFVCO=%f MHz,Change PLL pls!\n", fFVCO);
	//	while(1);
	}
}
#endif
