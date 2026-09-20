/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		LCD_AHD_NVP6021.c
	\brief		NVP6021 AHD encoder start-up Driver
	\author		futao_jiang
	\version	1.1
	\date		2022/03/21
	\copyright	Copyright(C) 2022 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#include <stdint.h>
#include <stdio.h>
#include <_510PF.h>
#include <TIMER.h>
#include "I2C.h"
#include "LCD_TYPE.h"
#include "LCD.h"
//#include "LCD_AHD_NVP6021.h"
#include "bsp_SNCC71_NVP6021_AHD_RX.h"


//------------------------------------------------------------------------------
#if (LCD_PANEL == LCD_AHD_NVP6021_RGB24)


//! Logic error assertion(debug only)
#define LCD_AHD_NVP6021_ERR_HOOK do{ printf("NVP6021: Error at Line(%d).\r\n", __LINE__); osDelay(1000); }while(1)


//! ------------------------------------------------------------------------------------------
//! 						NVP6021 expected video output mode
//! ------------------------------------------------------------------------------------------
#define NVP6021_AHD_720_NORMAL					0 //! 720p 25fps or 720p 30fps
#define NVP6021_AHD_720_HIGH_FPS				2 //! 720p 50fps(BT.1847) or 720p 60fps(BT.1543)
#define NVP6021_AHD_1080_NORMAL					4 //! 1080p 25fps or 1080p 30fps
#define NVP6021_AHD_EXPECTED_OUTPUT_MODE		NVP6021_AHD_1080_NORMAL //! N/P format is decided by hardware jumper(High=PAL, Low=NTSC)
//! ------------------------------------------------------------------------------------------
static NVP6021_VDO_MODE_e eNVP6021_OutputMode = (NVP6021_VDO_MODE_e)NVP6021_AHD_EXPECTED_OUTPUT_MODE;


//! ------------------------------------------------------------------------------------------
//! 										I2C
//! ------------------------------------------------------------------------------------------
//#define I2C_WRITE_CHECK_EN					1
//#define I2C_WRITE_RETRY_CNT					3

static I2C1_Type *pLCD_I2C_Type;

//! Pre-Converter
#if ( NVP6021_PRE_CONVERTER_TYPE == 0 )
	#define NVP6021_PRE_CONVERTER_I2C_ADDR		0x2D
	#define LT9211_PATTERN_EN					0
#elif ( NVP6021_PRE_CONVERTER_TYPE == 1 )
	#define NVP6021_PRE_CONVERTER_I2C_ADDR		0x00
#elif ( NVP6021_PRE_CONVERTER_TYPE == 2 )
	#define NVP6021_PRE_CONVERTER_I2C_ADDR		0x00
#endif

static bool bAHD_Preconverter_Read( uint8_t ubAddress, uint8_t *pValue) {
	return bI2C_MasterProcess(pLCD_I2C_Type, NVP6021_PRE_CONVERTER_I2C_ADDR, &ubAddress, 1, pValue, 1);
}

static bool bAHD_Preconverter_Write( uint8_t ubAddress, uint8_t ubValue ) {
	uint8_t pBuf[2];

    pBuf[0] = ubAddress;
    pBuf[1] = ubValue;	

    return bI2C_MasterProcess(pLCD_I2C_Type, NVP6021_PRE_CONVERTER_I2C_ADDR, &pBuf[0], 2, NULL, 0);
}

//! AHD Encoder
#define NVP6021_I2C_SA0						0
#define NVP6021_I2C_SA1						0
#define NVP6021_I2C_ADDR					( 0x30|((NVP6021_I2C_SA0&0x01u)<<1u)|((NVP6021_I2C_SA1&0x01u)<<2u) )

static bool bAHD_NVP6021_Read( uint8_t ubAddress, uint8_t *pValue) {
	return bI2C_MasterProcess(pLCD_I2C_Type, NVP6021_I2C_ADDR, &ubAddress, 1, pValue, 1);
}

static bool bAHD_NVP6021_Write( uint8_t ubAddress, uint8_t ubValue ) {
	uint8_t pBuf[2];

    pBuf[0] = ubAddress;
    pBuf[1] = ubValue;	

    return bI2C_MasterProcess(pLCD_I2C_Type, NVP6021_I2C_ADDR, &pBuf[0], 2, NULL, 0);
}


//! ------------------------------------------------------------------------------------------
//! 									Pre-Converter
//! ------------------------------------------------------------------------------------------
static const Video_Timing_t BT1120_TimingTable[NVP6021_AHD_FMT_MAX] = {
//! hs	hbp		hfp		hact	htotal	vs	vbp	vfp	vact	vtotal	pclk_kHz
{	40,	1760,	880,	1280,	3960,	5,	20,	5,	720,	750,	74250,	},	//! NVP6021_AHD_720P25
{	40,	980,	1000,	1280,	3300,	5,	20,	5,	720,	750,	74250,	},	//! NVP6021_AHD_720P30
{	40,	440,	220,	1280,	1980,	5,	20,	5,	720,	750,	74250,	},	//! NVP6021_AHD_720P50
{	40,	220,	110,	1280,	1650,	5,	20,	5,	720,	750,	74250,	},	//! NVP6021_AHD_720P60
{	40,	440,	240,	1920,	2640,	5,	36,	4,	1080,	1125,	74250,	},	//! NVP6021_AHD_1080P25
{	40,	160,	80,		1920,	2200,	5,	36,	4,	1080,	1125,	74250,	}	//! NVP6021_AHD_1080P30
};

#if ( NVP6021_PRE_CONVERTER_TYPE == 0 )
#if LT9211_PATTERN_EN
static void LT9211_Pattern( const Video_Timing_t *tTestTiming ) {
	uint32_t pclk_khz;
	uint8_t dessc_pll_post_div;
	uint32_t pcr_m, pcr_k;
	
	pclk_khz = tTestTiming->pclk_kHz;     

	bAHD_Preconverter_Write(0xff,0xf9);
	bAHD_Preconverter_Write(0x3e,0x80);  
 
	bAHD_Preconverter_Write(0xff,0x85);
	bAHD_Preconverter_Write(0x88,0xc0); //! 0x90:TTL RX-->Mipi TX; 0xd0:lvds RX->MipiTX; 0xc0:Chip Video pattern gen->Lvd TX

	bAHD_Preconverter_Write(0xa1,0x77); //! pattern type
	bAHD_Preconverter_Write(0xa2,0xff); 

	bAHD_Preconverter_Write(0xa3,(uint8_t)((tTestTiming->hs+tTestTiming->hbp)/256));
	bAHD_Preconverter_Write(0xa4,(uint8_t)((tTestTiming->hs+tTestTiming->hbp)%256)); //! h_start

	bAHD_Preconverter_Write(0xa5,(uint8_t)((tTestTiming->vs+tTestTiming->vbp)%256)); //! v_start

   	bAHD_Preconverter_Write(0xa6,(uint8_t)(tTestTiming->hact/256));
	bAHD_Preconverter_Write(0xa7,(uint8_t)(tTestTiming->hact%256)); //! hactive

	bAHD_Preconverter_Write(0xa8,(uint8_t)(tTestTiming->vact/256));
	bAHD_Preconverter_Write(0xa9,(uint8_t)(tTestTiming->vact%256)); //! vactive

   	bAHD_Preconverter_Write(0xaa,(uint8_t)(tTestTiming->htotal/256));
	bAHD_Preconverter_Write(0xab,(uint8_t)(tTestTiming->htotal%256)); //! htotal

   	bAHD_Preconverter_Write(0xac,(uint8_t)(tTestTiming->vtotal/256));
	bAHD_Preconverter_Write(0xad,(uint8_t)(tTestTiming->vtotal%256)); //! vtotal

   	bAHD_Preconverter_Write(0xae,(uint8_t)(tTestTiming->hs/256)); 
	bAHD_Preconverter_Write(0xaf,(uint8_t)(tTestTiming->hs%256)); //! hsa

	bAHD_Preconverter_Write(0xb0,(uint8_t)(tTestTiming->vs%256)); //! vsa

	//! dessc pll to generate pixel clk
	bAHD_Preconverter_Write(0xff,0x82); //! dessc pll
	bAHD_Preconverter_Write(0x2d,0x48); //! pll ref select xtal 
	
	if(pclk_khz < 44000) {
	  	bAHD_Preconverter_Write(0x35,0x83);
		dessc_pll_post_div = 16;
	} else if(pclk_khz < 88000) {
	  	bAHD_Preconverter_Write(0x35,0x82);
		dessc_pll_post_div = 8;
	} else if(pclk_khz < 176000) {
	  	bAHD_Preconverter_Write(0x35,0x81);
		dessc_pll_post_div = 4;
	} else if(pclk_khz < 352000) {
	  	bAHD_Preconverter_Write(0x35,0x80);
		dessc_pll_post_div = 0;
	}
	
	pcr_m = (pclk_khz * dessc_pll_post_div)/25;
	pcr_k = pcr_m%1000;
	pcr_m = pcr_m/1000;

	pcr_k <<= 14; 

	//! pixel clk
 	bAHD_Preconverter_Write(0xff,0xd0); //! pcr
	bAHD_Preconverter_Write(0x2d,0x7f);
	bAHD_Preconverter_Write(0x31,0x00);

	bAHD_Preconverter_Write(0x26,0x80|((uint8_t)pcr_m));
	bAHD_Preconverter_Write(0x27,(uint8_t)((pcr_k>>16)&0xff)); //! K
	bAHD_Preconverter_Write(0x28,(uint8_t)((pcr_k>>8)&0xff)); //! K
	bAHD_Preconverter_Write(0x29,(uint8_t)(pcr_k&0xff)); //! K
}
#endif //! #if LT9211_PATTERN_EN

static void LT9211_Init( const Video_Timing_t *tTxVdoTiming ) {
	uint8_t *pByte1, *pByte2;
	uint8_t loopx;
	uint16_t tmp_data;
#if !LT9211_PATTERN_EN
	uint8_t sync_polarity;
	uint32_t fm_value;
	Video_Timing_t tCheckTiming;
#endif //! #if !LT9211_PATTERN_EN
	
	//! Chip reset
	LCD_AHD_PRE_CONVERTER_RST = 0;
	osDelay(20);
	LCD_AHD_PRE_CONVERTER_RST = 1;
	osDelay(5);
	
	//! Read ID
	bAHD_Preconverter_Write(0xff,0x81); //! register bank
	bAHD_Preconverter_Read(0x00, pByte1);
	bAHD_Preconverter_Read(0x01, pByte2);
	printf("LT9211: Chip ID=0x%02x%02x\n", *pByte1, *pByte2);
	bAHD_Preconverter_Read(0x02, pByte1);
	printf("LT9211: Chip Ver=0x%02x\n", *pByte1);
	
	//! System clock init
	bAHD_Preconverter_Write(0xff,0x82);
    bAHD_Preconverter_Write(0x01,0x18);
	
    bAHD_Preconverter_Write(0xff,0x86);
    bAHD_Preconverter_Write(0x06,0x61); 	
    bAHD_Preconverter_Write(0x07,0xa8); //! fm for sys_clk
	  
    bAHD_Preconverter_Write(0xff,0x87); //! txpll
    bAHD_Preconverter_Write(0x14,0x08); //! default value
    bAHD_Preconverter_Write(0x15,0x00); //! default value
    bAHD_Preconverter_Write(0x18,0x0f);
    bAHD_Preconverter_Write(0x22,0x08); //! default value
    bAHD_Preconverter_Write(0x23,0x00); //! default value
    bAHD_Preconverter_Write(0x26,0x0f);
	
#if LT9211_PATTERN_EN
	LT9211_Pattern(&(BT1120_TimingTable[eNVP6021_OutputMode]));
#else
	//! TTL RX PHY
	bAHD_Preconverter_Write(0xff,0x82);
	bAHD_Preconverter_Write(0x28,0x40);
	bAHD_Preconverter_Write(0x61,0x09); //! LT9211_Write(0x61,0x01);
	bAHD_Preconverter_Write(0xff,0x81);
	bAHD_Preconverter_Read(0x02, pByte1);
	if( (*pByte1) == 0xe4 ) {
		bAHD_Preconverter_Write(0xff,0x82);
		bAHD_Preconverter_Write(0x63,0xFF); //! U4:00, U5:FF
	}
	else {
		bAHD_Preconverter_Write(0xff,0x82);
		bAHD_Preconverter_Write(0x63,0x00); //! U4:00, U5:FF		
	}
	
	//! Data mapping
	bAHD_Preconverter_Write(0xff,0x85);
	bAHD_Preconverter_Write(0x88,0x90);
	bAHD_Preconverter_Write(0x45,0x00); //! D0-D23:RGB
    bAHD_Preconverter_Write(0x47,0x07); //! (0x47,0x07)(0x47,0x37)
	
	//! ClkDetDebug
	bAHD_Preconverter_Write(0xff,0x86);
	bAHD_Preconverter_Write(0x00,0x14);
	osDelay(100);
    fm_value = 0;
	bAHD_Preconverter_Read(0x08, pByte1);
	fm_value = ((*pByte1)&(0x0f));
    fm_value = (fm_value<<8) ;
	bAHD_Preconverter_Read(0x09, pByte1);
	fm_value = fm_value + (*pByte1);
	fm_value = (fm_value<<8) ;
	bAHD_Preconverter_Read(0x0a, pByte1);
	fm_value = fm_value + (*pByte1);
    printf("LT9211--> input ttlclk: %d\n", fm_value);
	
	//! Video check
	bAHD_Preconverter_Write(0xff,0x86);
    bAHD_Preconverter_Write(0x20,0x00);
    osDelay(100);
	bAHD_Preconverter_Read(0x70, pByte1);
	sync_polarity = *pByte1;
	
	bAHD_Preconverter_Read(0x71, pByte1);
	tCheckTiming.vs = *pByte1;
	
	bAHD_Preconverter_Read(0x72, pByte1);
	tCheckTiming.hs = *pByte1;
	bAHD_Preconverter_Read(0x73, pByte1);
    tCheckTiming.hs = (tCheckTiming.hs<<8) + (*pByte1);
	
	bAHD_Preconverter_Read(0x74, pByte1);
	tCheckTiming.vbp = (uint8_t)*pByte1;
	bAHD_Preconverter_Read(0x75, pByte1);
    tCheckTiming.vfp = (uint8_t)*pByte1;
	bAHD_Preconverter_Read(0x76, pByte1);
	tCheckTiming.hbp = *pByte1;
	bAHD_Preconverter_Read(0x77, pByte1);
	tCheckTiming.hbp = (tCheckTiming.hbp<<8) + (*pByte1);
	bAHD_Preconverter_Read(0x78, pByte1);
	tCheckTiming.hfp = (*pByte1);
	bAHD_Preconverter_Read(0x79, pByte1);
	tCheckTiming.hfp = (tCheckTiming.hfp<<8) + (*pByte1);
	bAHD_Preconverter_Read(0x7A, pByte1);
	tCheckTiming.vtotal = (*pByte1);
	bAHD_Preconverter_Read(0x7B, pByte1);
	tCheckTiming.vtotal = (tCheckTiming.vtotal<<8) + (*pByte1);
	bAHD_Preconverter_Read(0x7C, pByte1);
	tCheckTiming.htotal = (*pByte1);
	bAHD_Preconverter_Read(0x7D, pByte1);
	tCheckTiming.htotal = (tCheckTiming.htotal<<8) + (*pByte1);
	bAHD_Preconverter_Read(0x7E, pByte1);
	tCheckTiming.vact = (*pByte1);
	bAHD_Preconverter_Read(0x7F, pByte1);
	tCheckTiming.vact = (tCheckTiming.vact<<8)+ (*pByte1);
	bAHD_Preconverter_Read(0x80, pByte1);
	tCheckTiming.hact = (*pByte1);
	bAHD_Preconverter_Read(0x81, pByte1);
	tCheckTiming.hact = (tCheckTiming.hact<<8) + (*pByte1);

	printf("LT9211: sync_polarity = %x\n", sync_polarity);
	
	if(!(sync_polarity & 0x01)) //! hsync
	{
        bAHD_Preconverter_Write(0xff,0x85);
		bAHD_Preconverter_Read(0x47, pByte1);
        bAHD_Preconverter_Write(0x47, ((*pByte1)| 0x10));
	}
	if(!(sync_polarity & 0x02)) //! vsync
	{
        bAHD_Preconverter_Write(0xff,0x85);
		bAHD_Preconverter_Read(0x47, pByte1);
        bAHD_Preconverter_Write(0x47, ((*pByte1)| 0x20));
	}
    printf("LT9211: hfp, hs, hbp, hact, htotal = %d_%d_%d_%d_%d\n",	tCheckTiming.hfp,
																		tCheckTiming.hs,
																		tCheckTiming.hbp,
																		tCheckTiming.hact,
																		tCheckTiming.htotal	);
	printf("LT9211: vfp, vs, vbp, vact, vtotal = %d_%d_%d_%d_%d\n",	tCheckTiming.vfp,
																		tCheckTiming.vs,
																		tCheckTiming.vbp,
																		tCheckTiming.vact,
																		tCheckTiming.vtotal	);
#endif //! #if LT9211_PATTERN_EN
	osDelay(100);
	
	//! Tx PHY
	bAHD_Preconverter_Write(0xff,0x82);
	bAHD_Preconverter_Write(0x62,0x01); //! ttl output enable
	bAHD_Preconverter_Write(0x6b,0xff);
	
	//! Tx Digital
	bAHD_Preconverter_Write(0xff,0x85);
	//! BT1120 internal sync polarity
	bAHD_Preconverter_Write(0x60,0x33); //! 1 1
//	bAHD_Preconverter_Write(0x60,0x23); //! 1 0
//	bAHD_Preconverter_Write(0x60,0x13); //! 0 1
//	bAHD_Preconverter_Write(0x60,0x03); //! 0 0
	bAHD_Preconverter_Write(0x6d,0x00); //! 0x08 YC SWAP
	bAHD_Preconverter_Write(0x6e,0x06); //! HIGH 16BIT
	
	bAHD_Preconverter_Write(0x6f,0x04);
	
	bAHD_Preconverter_Write(0xff,0x81);
	bAHD_Preconverter_Write(0x0d,0xfd);
	bAHD_Preconverter_Write(0x0d,0xff);
	
	osDelay(10);
	
	//! Tx PLL
	bAHD_Preconverter_Write(0xff,0x82);
	bAHD_Preconverter_Write(0x36,0x01); //! b7:txpll_pd
	bAHD_Preconverter_Write(0x37,0x2a);
	bAHD_Preconverter_Write(0x38,0x06);
	bAHD_Preconverter_Write(0x39,0x30);
	bAHD_Preconverter_Write(0x3a,0x8e);
	bAHD_Preconverter_Write(0xff,0x87);
	bAHD_Preconverter_Write(0x37,0x14);
	bAHD_Preconverter_Write(0x13,0x00);
	bAHD_Preconverter_Write(0x13,0x80);
	osDelay(100);
	for ( loopx = 0; loopx < 10; loopx++ ) { //! Check Tx PLL cal
		bAHD_Preconverter_Write(0xff,0x87);
		bAHD_Preconverter_Read(0x1f, pByte1);
		if( (*pByte1) & 0x80 ) {
			bAHD_Preconverter_Read(0x20, pByte1);
			if( (*pByte1) & 0x80)
				printf("LT9211: tx pll lock!\n");
			else
				printf("LT9211: tx pll unlocked!\n");			
			printf("LT9211: tx pll cal done!\n");
			break;
		} else {
			printf("LT9211: tx pll unlocked!\n");
		}
	}
	printf("LT9211: system success\n");
	
	//! Tx BT set
	tmp_data = tTxVdoTiming->hs+tTxVdoTiming->hbp;
    bAHD_Preconverter_Write(0xff,0x85);
    bAHD_Preconverter_Write(0x61,(uint8_t)(tmp_data>>8));
    bAHD_Preconverter_Write(0x62,(uint8_t)tmp_data);
    bAHD_Preconverter_Write(0x63,(uint8_t)(tTxVdoTiming->hact>>8));
    bAHD_Preconverter_Write(0x64,(uint8_t)tTxVdoTiming->hact);
    bAHD_Preconverter_Write(0x65,(uint8_t)(tTxVdoTiming->htotal>>8));
    bAHD_Preconverter_Write(0x66,(uint8_t)tTxVdoTiming->htotal);		
    tmp_data = tTxVdoTiming->vs+tTxVdoTiming->vbp;
    bAHD_Preconverter_Write(0x67,(uint8_t)tmp_data);
    bAHD_Preconverter_Write(0x68,0x00);
    bAHD_Preconverter_Write(0x69,(uint8_t)(tTxVdoTiming->vact>>8));
    bAHD_Preconverter_Write(0x6a,(uint8_t)tTxVdoTiming->vact);
    bAHD_Preconverter_Write(0x6b,(uint8_t)(tTxVdoTiming->vtotal>>8));
    bAHD_Preconverter_Write(0x6c,(uint8_t)tTxVdoTiming->vtotal);
	
	//! LT9211 CSC
	printf("LT9211: CSC rgb888 to ycbcr422\n");
    bAHD_Preconverter_Write(0xff,0xf9);
    bAHD_Preconverter_Write(0x86,0x0f);
    bAHD_Preconverter_Write(0x87,0x30);
}
#elif ( NVP6021_PRE_CONVERTER_TYPE == 1 )
	#error "NVP6021_PRE_CONVERTER_TYPE Not available! -1"
#elif ( NVP6021_PRE_CONVERTER_TYPE == 2 )
	#error "NVP6021_PRE_CONVERTER_TYPE Not available! -2"
#endif


//! ------------------------------------------------------------------------------------------
//! 										NVP6021
//! ------------------------------------------------------------------------------------------
static const uint8_t NVP6021_InitTable[][NVP6021_AHD_FMT_MAX][2] = {
// -----------------------------------------------------------------------------------------------------------------------------------------------------
// Seq //		720P25		/**/	    720P30		/**/		720P50		/**/		720P60		/**/		1080P25		/**/		1080P30		/**/
//	   //	 Reg     Dat	/**/	 Reg	 Dat	/**/	 Reg	 Dat	/**/	 Reg	 Dat	/**/	 Reg	 Dat	/**/	Reg		Dat		/**/
// -----------------------------------------------------------------------------------------------------------------------------------------------------
/*   1 */	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/
/*   2 */	0x00,	0x07,	/**/	0x00,	0x07,	/**/	0x00,	0x01,	/**/	0x00,	0x01,	/**/	0x00,	0x01,	/**/	0x00,	0x01,	/**/
/*   3 */	0x01,	0xE3,	/**/	0x01,	0xE2,	/**/	0x01,	0xE3,	/**/	0x01,	0xE2,	/**/	0x01,	0xF1,	/**/	0x01,	0xF0,	/**/
/*   4 */	0x05,	0x00,	/**/	0x05,	0x00,	/**/	0x04,	0x00,	/**/	0x04,	0x00,	/**/	0x04,	0x00,	/**/	0x04,	0x00,	/**/
/*   5 */	0x06,	0x20,	/**/	0x06,	0x20,	/**/	0xFF,	0x01,	/**/	0xFF,	0x01,	/**/	0xFF,	0x01,	/**/	0xFF,	0x01,	/**/
/*   6 */	0x07,	0x18,	/**/	0x07,	0x18,	/**/	0x0E,	0x00,	/**/	0x0E,	0x00,	/**/	0x0E,	0x00,	/**/	0x0E,	0x00,	/**/
/*   7 */	0x05,	0xFF,	/**/	0x05,	0xFF,	/**/	0xFF,	0x02,	/**/	0xFF,	0x02,	/**/	0xFF,	0x02,	/**/	0xFF,	0x02,	/**/
/*   8 */	0x05,	0x00,	/**/	0x05,	0x00,	/**/	0x00,	0xFE,	/**/	0x00,	0xFE,	/**/	0x00,	0xFE,	/**/	0x00,	0xFE,	/**/
/*   9 */	0x11,	0x00,	/**/	0x11,	0x00,	/**/	0x02,	0x00,	/**/	0x02,	0x00,	/**/	0x02,	0x00,	/**/	0x02,	0x00,	/**/
/*  10 */	0x12,	0x60,	/**/	0x12,	0x60,	/**/	0x04,	0x80,	/**/	0x04,	0x80,	/**/	0x04,	0x80,	/**/	0x04,	0x80,	/**/
/*  11 */	0xFF,	0x01,	/**/	0xFF,	0x01,	/**/	0x05,	0x00,	/**/	0x05,	0x00,	/**/	0x05,	0x00,	/**/	0x05,	0x00,	/**/
/*  12 */	0x0E,	0x00,	/**/	0x0E,	0x00,	/**/	0x06,	0x10,	/**/	0x06,	0x10,	/**/	0x06,	0x10,	/**/	0x06,	0x10,	/**/
/*  13 */	0xFF,	0x02,	/**/	0xFF,	0x02,	/**/	0x0C,	0x04,	/**/	0x0C,	0x04,	/**/	0x0C,	0x04,	/**/	0x0C,	0x04,	/**/
/*  14 */	0x00,	0xFE,	/**/	0x00,	0xFE,	/**/	0x0D,	0x3F,	/**/	0x0D,	0x3F,	/**/	0x0D,	0x3F,	/**/	0x0D,	0x3F,	/**/
/*  15 */	0x01,	0xEA,	/**/	0x01,	0xEA,	/**/	0x0E,	0x00,	/**/	0x0E,	0x00,	/**/	0x0E,	0x00,	/**/	0x0E,	0x00,	/**/
/*  16 */	0x02,	0x00,	/**/	0x02,	0x00,	/**/	0x10,	0xEB,	/**/	0x10,	0xEB,	/**/	0x10,	0xEB,	/**/	0x10,	0xEB,	/**/
/*  17 */	0x03,	0x00,	/**/	0x03,	0x00,	/**/	0x11,	0x10,	/**/	0x11,	0x10,	/**/	0x11,	0x10,	/**/	0x11,	0x10,	/**/
/*  18 */	0x04,	0x50,	/**/	0x04,	0x50,	/**/	0x12,	0xF0,	/**/	0x12,	0xF0,	/**/	0x12,	0xF0,	/**/	0x12,	0xF0,	/**/
/*  19 */	0x05,	0x00,	/**/	0x05,	0x00,	/**/	0x13,	0x10,	/**/	0x13,	0x10,	/**/	0x13,	0x10,	/**/	0x13,	0x10,	/**/
/*  20 */	0x06,	0x10,	/**/	0x06,	0x10,	/**/	0x14,	0x01,	/**/	0x14,	0x01,	/**/	0x14,	0x01,	/**/	0x14,	0x01,	/**/
// ----//-------------------/**/--------------------/**/--------------------/**/--------------------/**/--------------------/**/--------------------/**/
// Seq //		720P25		/**/	    720P30		/**/		720P50		/**/		720P60		/**/		1080P25		/**/		1080P30		/**/
//	   //	 Reg     Dat	/**/	 Reg	 Dat	/**/	 Reg	 Dat	/**/	 Reg	 Dat	/**/	 Reg	 Dat	/**/	Reg		Dat		/**/
// ----//-------------------/**/--------------------/**/--------------------/**/--------------------/**/--------------------/**/--------------------/**/
/*  21 */	0x07,	0x00,	/**/	0x07,	0x00,	/**/	0x15,	0x00,	/**/	0x15,	0x00,	/**/	0x15,	0x00,	/**/	0x15,	0x00,	/**/
/*  22 */	0x08,	0x00,	/**/	0x08,	0x00,	/**/	0x16,	0x00,	/**/	0x16,	0x00,	/**/	0x16,	0x00,	/**/	0x16,	0x00,	/**/
/*  23 */	0x09,	0x00,	/**/	0x09,	0x00,	/**/	0x17,	0x00,	/**/	0x17,	0x00,	/**/	0x17,	0x00,	/**/	0x17,	0x00,	/**/
/*  24 */	0x0A,	0x00,	/**/	0x0A,	0x00,	/**/	0x18,	0x00,	/**/	0x18,	0x00,	/**/	0x18,	0x00,	/**/	0x18,	0x00,	/**/
/*  25 */	0x0B,	0x00,	/**/	0x0B,	0x00,	/**/	0x19,	0x00,	/**/	0x19,	0x00,	/**/	0x19,	0x00,	/**/	0x19,	0x00,	/**/
/*  26 */	0x0C,	0x04,	/**/	0x0C,	0x04,	/**/	0x1C,	0x80,	/**/	0x1C,	0x80,	/**/	0x1C,	0x80,	/**/	0x1C,	0x80,	/**/
/*  27 */	0x0D,	0x3F,	/**/	0x0D,	0x3F,	/**/	0x1D,	0x80,	/**/	0x1D,	0x80,	/**/	0x1D,	0x80,	/**/	0x1D,	0x80,	/**/
/*  28 */	0x0E,	0x00,	/**/	0x0E,	0x00,	/**/	0x1E,	0x80,	/**/	0x1E,	0x80,	/**/	0x1E,	0x80,	/**/	0x1E,	0x80,	/**/
/*  29 */	0x0F,	0x00,	/**/	0x0F,	0x00,	/**/	0x20,	0x00,	/**/	0x20,	0x00,	/**/	0x20,	0x00,	/**/	0x20,	0x00,	/**/
/*  30 */	0x10,	0xEB,	/**/	0x10,	0xEB,	/**/	0x21,	0x00,	/**/	0x21,	0x00,	/**/	0x21,	0x00,	/**/	0x21,	0x00,	/**/
/*  31 */	0x11,	0x10,	/**/	0x11,	0x10,	/**/	0x22,	0x00,	/**/	0x22,	0x00,	/**/	0x22,	0x00,	/**/	0x22,	0x00,	/**/
/*  32 */	0x12,	0xF0,	/**/	0x12,	0xF0,	/**/	0x23,	0x00,	/**/	0x23,	0x00,	/**/	0x23,	0x00,	/**/	0x23,	0x00,	/**/
/*  33 */	0x13,	0x10,	/**/	0x13,	0x10,	/**/	0x24,	0x00,	/**/	0x24,	0x00,	/**/	0x24,	0x00,	/**/	0x24,	0x00,	/**/
/*  34 */	0x14,	0x01,	/**/	0x14,	0x01,	/**/	0x25,	0x00,	/**/	0x25,	0x00,	/**/	0x25,	0x00,	/**/	0x25,	0x00,	/**/
/*  35 */	0x15,	0x00,	/**/	0x15,	0x00,	/**/	0x26,	0x00,	/**/	0x26,	0x00,	/**/	0x26,	0x00,	/**/	0x26,	0x00,	/**/
/*  36 */	0x16,	0x00,	/**/	0x16,	0x00,	/**/	0x27,	0x00,	/**/	0x27,	0x00,	/**/	0x27,	0x00,	/**/	0x27,	0x00,	/**/
/*  37 */	0x17,	0x00,	/**/	0x17,	0x00,	/**/	0x28,	0x00,	/**/	0x28,	0x00,	/**/	0x28,	0x00,	/**/	0x28,	0x00,	/**/
/*  38 */	0x18,	0x00,	/**/	0x18,	0x00,	/**/	0x29,	0x00,	/**/	0x29,	0x00,	/**/	0x29,	0x00,	/**/	0x29,	0x00,	/**/
/*  39 */	0x19,	0x00,	/**/	0x19,	0x00,	/**/	0x2A,	0x00,	/**/	0x2A,	0x00,	/**/	0x2A,	0x00,	/**/	0x2A,	0x00,	/**/
/*  40 */	0x1A,	0x00,	/**/	0x1A,	0x00,	/**/	0x2B,	0x00,	/**/	0x2B,	0x00,	/**/	0x2B,	0x00,	/**/	0x2B,	0x00,	/**/
// ----//-------------------/**/--------------------/**/--------------------/**/--------------------/**/--------------------/**/--------------------/**/
// Seq //		720P25		/**/	    720P30		/**/		720P50		/**/		720P60		/**/		1080P25		/**/		1080P30		/**/
//	   //	 Reg     Dat	/**/	 Reg	 Dat	/**/	 Reg	 Dat	/**/	 Reg	 Dat	/**/	 Reg	 Dat	/**/	Reg		Dat		/**/
// ----//-------------------/**/--------------------/**/--------------------/**/--------------------/**/--------------------/**/--------------------/**/
/*  41 */	0x1B,	0x00,	/**/	0x1B,	0x00,	/**/	0x2C,	0x00,	/**/	0x2C,	0x00,	/**/	0x2C,	0x00,	/**/	0x2C,	0x00,	/**/
/*  42 */	0x1C,	0x80,	/**/	0x1C,	0x80,	/**/	0x2D,	0x00,	/**/	0x2D,	0x00,	/**/	0x2D,	0x00,	/**/	0x2D,	0x00,	/**/
/*  43 */	0x1D,	0x80,	/**/	0x1D,	0x80,	/**/	0x2E,	0x00,	/**/	0x2E,	0x00,	/**/	0x2E,	0x00,	/**/	0x2E,	0x00,	/**/
/*  44 */	0x1E,	0x80,	/**/	0x1E,	0x80,	/**/	0x2F,	0x00,	/**/	0x2F,	0x00,	/**/	0x2F,	0x00,	/**/	0x2F,	0x00,	/**/
/*  45 */	0x1F,	0x00,	/**/	0x1F,	0x00,	/**/	0x30,	0x00,	/**/	0x30,	0x00,	/**/	0x30,	0x00,	/**/	0x30,	0x00,	/**/
/*  46 */	0x20,	0x00,	/**/	0x20,	0x00,	/**/	0x31,	0x00,	/**/	0x31,	0x00,	/**/	0x31,	0x00,	/**/	0x31,	0x00,	/**/
/*  47 */	0x21,	0x00,	/**/	0x21,	0x00,	/**/	0x32,	0x00,	/**/	0x32,	0x00,	/**/	0x32,	0x00,	/**/	0x32,	0x00,	/**/
/*  48 */	0x22,	0x00,	/**/	0x22,	0x00,	/**/	0x33,	0x00,	/**/	0x33,	0x00,	/**/	0x33,	0x00,	/**/	0x33,	0x00,	/**/
/*  49 */	0x23,	0x00,	/**/	0x23,	0x00,	/**/	0x34,	0x00,	/**/	0x34,	0x00,	/**/	0x34,	0x00,	/**/	0x34,	0x00,	/**/
/*  50 */	0x24,	0x00,	/**/	0x24,	0x00,	/**/	0x36,	0x01,	/**/	0x36,	0x01,	/**/	0x36,	0x01,	/**/	0x36,	0x01,	/**/
/*  51 */	0x25,	0x00,	/**/	0x25,	0x00,	/**/	0x37,	0x80,	/**/	0x37,	0x80,	/**/	0x37,	0x80,	/**/	0x37,	0x80,	/**/
/*  52 */	0x26,	0x00,	/**/	0x26,	0x00,	/**/	0x39,	0x00,	/**/	0x39,	0x00,	/**/	0x39,	0x00,	/**/	0x39,	0x00,	/**/
/*  53 */	0x27,	0x00,	/**/	0x27,	0x00,	/**/	0x3C,	0x00,	/**/	0x3C,	0x00,	/**/	0x3C,	0x00,	/**/	0x3C,	0x00,	/**/
/*  54 */	0x28,	0x00,	/**/	0x28,	0x00,	/**/	0x3D,	0x00,	/**/	0x3D,	0x00,	/**/	0x3D,	0x00,	/**/	0x3D,	0x00,	/**/
/*  55 */	0x29,	0x00,	/**/	0x29,	0x00,	/**/	0x3E,	0x00,	/**/	0x3E,	0x00,	/**/	0x3E,	0x00,	/**/	0x3E,	0x00,	/**/
/*  56 */	0x2A,	0x00,	/**/	0x2A,	0x00,	/**/	0x40,	0x01,	/**/	0x40,	0x01,	/**/	0x40,	0x01,	/**/	0x40,	0x01,	/**/
/*  57 */	0x2B,	0x00,	/**/	0x2B,	0x00,	/**/	0x41,	0xFF,	/**/	0x41,	0xFF,	/**/	0x41,	0xFF,	/**/	0x41,	0xFF,	/**/
/*  58 */	0x2C,	0x00,	/**/	0x2C,	0x00,	/**/	0x42,	0x80,	/**/	0x42,	0x80,	/**/	0x42,	0x80,	/**/	0x42,	0x80,	/**/
/*  59 */	0x2D,	0x00,	/**/	0x2D,	0x00,	/**/	0x48,	0x00,	/**/	0x48,	0x00,	/**/	0x48,	0x00,	/**/	0x48,	0x00,	/**/
/*  60 */	0x2E,	0x00,	/**/	0x2E,	0x00,	/**/	0x49,	0x00,	/**/	0x49,	0x00,	/**/	0x49,	0x00,	/**/	0x49,	0x00,	/**/
// ----//-------------------/**/--------------------/**/--------------------/**/--------------------/**/--------------------/**/--------------------/**/
// Seq //		720P25		/**/	    720P30		/**/		720P50		/**/		720P60		/**/		1080P25		/**/		1080P30		/**/
//	   //	 Reg     Dat	/**/	 Reg	 Dat	/**/	 Reg	 Dat	/**/	 Reg	 Dat	/**/	 Reg	 Dat	/**/	Reg		Dat		/**/
// ----//-------------------/**/--------------------/**/--------------------/**/--------------------/**/--------------------/**/--------------------/**/
/*  61 */	0x2F,	0x00,	/**/	0x2F,	0x00,	/**/	0x4A,	0x00,	/**/	0x4A,	0x00,	/**/	0x4A,	0x00,	/**/	0x4A,	0x00,	/**/
/*  62 */	0x30,	0x00,	/**/	0x30,	0x00,	/**/	0x4B,	0x00,	/**/	0x4B,	0x00,	/**/	0x4B,	0x00,	/**/	0x4B,	0x00,	/**/
/*  63 */	0x31,	0x00,	/**/	0x31,	0x00,	/**/	0x4C,	0x00,	/**/	0x4C,	0x00,	/**/	0x4C,	0x00,	/**/	0x4C,	0x00,	/**/
/*  64 */	0x32,	0x00,	/**/	0x32,	0x00,	/**/	0x4D,	0x00,	/**/	0x4D,	0x00,	/**/	0x4D,	0x00,	/**/	0x4D,	0x00,	/**/
/*  65 */	0x33,	0x00,	/**/	0x33,	0x00,	/**/	0x4E,	0x00,	/**/	0x4E,	0x00,	/**/	0x4E,	0x00,	/**/	0x4E,	0x00,	/**/
/*  66 */	0x34,	0x00,	/**/	0x34,	0x00,	/**/	0x4F,	0x00,	/**/	0x4F,	0x00,	/**/	0x4F,	0x00,	/**/	0x4F,	0x00,	/**/
/*  67 */	0x35,	0x00,	/**/	0x35,	0x00,	/**/	0x50,	0x00,	/**/	0x50,	0x00,	/**/	0x50,	0x00,	/**/	0x50,	0x00,	/**/
/*  68 */	0x36,	0x01,	/**/	0x36,	0x01,	/**/	0x51,	0x00,	/**/	0x51,	0x00,	/**/	0x51,	0x00,	/**/	0x51,	0x00,	/**/
/*  69 */	0x37,	0x80,	/**/	0x37,	0x80,	/**/	0x52,	0x00,	/**/	0x52,	0x00,	/**/	0x52,	0x00,	/**/	0x52,	0x00,	/**/
/*  70 */	0x38,	0x00,	/**/	0x38,	0x00,	/**/	0x53,	0x00,	/**/	0x53,	0x00,	/**/	0x53,	0x00,	/**/	0x53,	0x00,	/**/
/*  71 */	0x39,	0x00,	/**/	0x39,	0x00,	/**/	0x54,	0x00,	/**/	0x54,	0x00,	/**/	0x54,	0x00,	/**/	0x54,	0x00,	/**/
/*  72 */	0x3A,	0x13,	/**/	0x3A,	0x13,	/**/	0x55,	0x00,	/**/	0x55,	0x00,	/**/	0x55,	0x00,	/**/	0x55,	0x00,	/**/
/*  73 */	0x3B,	0x00,	/**/	0x3B,	0x00,	/**/	0x56,	0x00,	/**/	0x56,	0x00,	/**/	0x56,	0x00,	/**/	0x56,	0x00,	/**/
/*  74 */	0x3C,	0x00,	/**/	0x3C,	0x00,	/**/	0x57,	0x00,	/**/	0x57,	0x00,	/**/	0x57,	0x00,	/**/	0x57,	0x00,	/**/
/*  75 */	0x3D,	0x00,	/**/	0x3D,	0x00,	/**/	0x59,	0x00,	/**/	0x59,	0x00,	/**/	0x59,	0x00,	/**/	0x59,	0x00,	/**/
/*  76 */	0x3E,	0x00,	/**/	0x3E,	0x00,	/**/	0x5A,	0x00,	/**/	0x5A,	0x00,	/**/	0x5A,	0x00,	/**/	0x5A,	0x00,	/**/
/*  77 */	0x3F,	0x00,	/**/	0x3F,	0x00,	/**/	0x5C,	0x00,	/**/	0x5C,	0x00,	/**/	0x5C,	0x00,	/**/	0x5C,	0x00,	/**/
/*  78 */	0x40,	0x01,	/**/	0x40,	0x01,	/**/	0x5D,	0x00,	/**/	0x5D,	0x00,	/**/	0x5D,	0x00,	/**/	0x5D,	0x00,	/**/
/*  79 */	0x41,	0xFF,	/**/	0x41,	0xFF,	/**/	0x5E,	0x00,	/**/	0x5E,	0x00,	/**/	0x5E,	0x00,	/**/	0x5E,	0x00,	/**/
/*  80 */	0x42,	0x80,	/**/	0x42,	0x80,	/**/	0x5F,	0x00,	/**/	0x5F,	0x00,	/**/	0x5F,	0x00,	/**/	0x5F,	0x00,	/**/
// ----//-------------------/**/--------------------/**/--------------------/**/--------------------/**/--------------------/**/--------------------/**/
// Seq //		720P25		/**/	    720P30		/**/		720P50		/**/		720P60		/**/		1080P25		/**/		1080P30		/**/
//	   //	 Reg     Dat	/**/	 Reg	 Dat	/**/	 Reg	 Dat	/**/	 Reg	 Dat	/**/	 Reg	 Dat	/**/	Reg		Dat		/**/
// ----//-------------------/**/--------------------/**/--------------------/**/--------------------/**/--------------------/**/--------------------/**/
/*  81 */	0x43,	0x00,	/**/	0x43,	0x00,	/**/	0x60,	0x80,	/**/	0x60,	0x80,	/**/	0x60,	0x80,	/**/	0x60,	0x80,	/**/
/*  82 */	0x44,	0x00,	/**/	0x44,	0x00,	/**/	0x61,	0x80,	/**/	0x61,	0x80,	/**/	0x61,	0x80,	/**/	0x61,	0x80,	/**/
/*  83 */	0x45,	0x00,	/**/	0x45,	0x00,	/**/	0x63,	0xE0,	/**/	0x63,	0xE0,	/**/	0x63,	0xE0,	/**/	0x63,	0xE0,	/**/
/*  84 */	0x46,	0x00,	/**/	0x46,	0x00,	/**/	0x64,	0x19,	/**/	0x64,	0x19,	/**/	0x64,	0x19,	/**/	0x64,	0x19,	/**/
/*  85 */	0x47,	0x00,	/**/	0x47,	0x00,	/**/	0x65,	0x04,	/**/	0x65,	0x04,	/**/	0x65,	0x04,	/**/	0x65,	0x04,	/**/
/*  86 */	0x48,	0x00,	/**/	0x48,	0x00,	/**/	0x66,	0xEB,	/**/	0x66,	0xEB,	/**/	0x66,	0xEB,	/**/	0x66,	0xEB,	/**/
/*  87 */	0x49,	0x00,	/**/	0x49,	0x00,	/**/	0x67,	0x60,	/**/	0x67,	0x60,	/**/	0x67,	0x60,	/**/	0x67,	0x60,	/**/
/*  88 */	0x4A,	0x00,	/**/	0x4A,	0x00,	/**/	0x68,	0x00,	/**/	0x68,	0x00,	/**/	0x68,	0x00,	/**/	0x68,	0x00,	/**/
/*  89 */	0x4B,	0x00,	/**/	0x4B,	0x00,	/**/	0x69,	0x00,	/**/	0x69,	0x00,	/**/	0x69,	0x00,	/**/	0x69,	0x00,	/**/
/*  90 */	0x4C,	0x00,	/**/	0x4C,	0x00,	/**/	0x6A,	0x00,	/**/	0x6A,	0x00,	/**/	0x6A,	0x00,	/**/	0x6A,	0x00,	/**/
/*  91 */	0x4D,	0x00,	/**/	0x4D,	0x00,	/**/	0x6B,	0x00,	/**/	0x6B,	0x00,	/**/	0x6B,	0x00,	/**/	0x6B,	0x00,	/**/
/*  92 */	0x4E,	0x00,	/**/	0x4E,	0x00,	/**/	0x3A,	0x11,	/**/	0x3A,	0x11,	/**/	0x3A,	0x11,	/**/	0x3A,	0x11,	/**/
/*  93 */	0x4F,	0x00,	/**/	0x4F,	0x00,	/**/	0x01,	0xE3,	/**/	0x01,	0xE2,	/**/	0x01,	0xF1,	/**/	0x01,	0xF0,	/**/
/*  94 */	0x50,	0x00,	/**/	0x50,	0x00,	/**/	0xFF,	0x02,	/**/	0xFF,	0x02,	/**/	0xFF,	0x02,	/**/	0xFF,	0x02,	/**/
/*  95 */	0x51,	0x00,	/**/	0x51,	0x00,	/**/	0x3D,	0x80,	/**/	0x3D,	0x80,	/**/	0x3D,	0x80,	/**/	0x3D,	0x80,	/**/
/*  96 */	0x52,	0x00,	/**/	0x52,	0x00,	/**/	0x5C,	0x52,	/**/	0x5C,	0x52,	/**/	0x5C,	0x52,	/**/	0x5C,	0x52,	/**/
/*  97 */	0x53,	0x00,	/**/	0x53,	0x00,	/**/	0x5D,	0xCF,	/**/	0x5D,	0xC5,	/**/	0x5D,	0xC3,	/**/	0x5D,	0xCA,	/**/
/*  98 */	0x54,	0x00,	/**/	0x54,	0x00,	/**/	0x5E,	0xE7,	/**/	0x5E,	0xF9,	/**/	0x5E,	0x7D,	/**/	0x5E,	0xF0,	/**/
/*  99 */	0x55,	0x00,	/**/	0x55,	0x00,	/**/	0x5F,	0x2C,	/**/	0x5F,	0x2C,	/**/	0x5F,	0xC8,	/**/	0x5F,	0x2C,	/**/
/* 100 */	0x56,	0x00,	/**/	0x56,	0x00,	/**/	0x1D,	0xB0,	/**/	0x1D,	0xB0,	/**/	0x1D,	0xB0,	/**/	0x1D,	0xB0,	/**/
// ----//-------------------/**/--------------------/**/--------------------/**/--------------------/**/--------------------/**/--------------------/**/
// Seq //		720P25		/**/	    720P30		/**/		720P50		/**/		720P60		/**/		1080P25		/**/		1080P30		/**/
//	   //	 Reg     Dat	/**/	 Reg	 Dat	/**/	 Reg	 Dat	/**/	 Reg	 Dat	/**/	 Reg	 Dat	/**/	Reg		Dat		/**/
// ----//-------------------/**/--------------------/**/--------------------/**/--------------------/**/--------------------/**/--------------------/**/
/* 101 */	0x57,	0x00,	/**/	0x57,	0x00,	/**/	0x1E,	0xB0,	/**/	0x1E,	0xB0,	/**/	0x1E,	0xB0,	/**/	0x1E,	0xB0,	/**/
/* 102 */	0x58,	0x00,	/**/	0x58,	0x00,	/**/	0x37,	0xB0,	/**/	0x37,	0xB0,	/**/	0x37,	0xB0,	/**/	0x37,	0xB0,	/**/
/* 103 */	0x59,	0x00,	/**/	0x59,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/
/* 104 */	0x5A,	0x00,	/**/	0x5A,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/
/* 105 */	0x5B,	0x00,	/**/	0x5B,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/
/* 106 */	0x5C,	0x00,	/**/	0x5C,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/
/* 107 */	0x5D,	0x00,	/**/	0x5D,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/
/* 108 */	0x5E,	0x00,	/**/	0x5E,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/
/* 109 */	0x5F,	0x00,	/**/	0x5F,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/
/* 110 */	0x60,	0x80,	/**/	0x60,	0x80,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/
/* 111 */	0x61,	0x80,	/**/	0x61,	0x80,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/
/* 112 */	0x62,	0x00,	/**/	0x62,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/
/* 113 */	0x63,	0xE0,	/**/	0x63,	0xE0,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/
/* 114 */	0x64,	0x19,	/**/	0x64,	0x19,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/
/* 115 */	0x65,	0x04,	/**/	0x65,	0x04,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/
/* 116 */	0x66,	0xEB,	/**/	0x66,	0xEB,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/
/* 117 */	0x67,	0x60,	/**/	0x67,	0x60,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/
/* 118 */	0x68,	0x00,	/**/	0x68,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/
/* 119 */	0x69,	0x00,	/**/	0x69,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/
/* 120 */	0x6A,	0x00,	/**/	0x6A,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/
// ----//-------------------/**/--------------------/**/--------------------/**/--------------------/**/--------------------/**/--------------------/**/
// Seq //		720P25		/**/	    720P30		/**/		720P50		/**/		720P60		/**/		1080P25		/**/		1080P30		/**/
//	   //	 Reg     Dat	/**/	 Reg	 Dat	/**/	 Reg	 Dat	/**/	 Reg	 Dat	/**/	 Reg	 Dat	/**/	Reg		Dat		/**/
// ----//-------------------/**/--------------------/**/--------------------/**/--------------------/**/--------------------/**/--------------------/**/
/* 121 */	0x6B,	0x00,	/**/	0x6B,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/
/* 122 */	0x6C,	0x00,	/**/	0x6C,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/
/* 123 */	0x6D,	0x00,	/**/	0x6D,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/
/* 124 */	0x6E,	0x00,	/**/	0x6E,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/
/* 125 */	0x6F,	0x00,	/**/	0x6F,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/
/* 126 */	0x70,	0x00,	/**/	0x70,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/
/* 127 */	0x71,	0x00,	/**/	0x71,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/
/* 128 */	0x72,	0x00,	/**/	0x72,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/
/* 129 */	0x73,	0x00,	/**/	0x73,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/
/* 130 */	0x74,	0x00,	/**/	0x74,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/
/* 131 */	0x75,	0x00,	/**/	0x75,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/
/* 132 */	0x76,	0x00,	/**/	0x76,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/
/* 133 */	0x77,	0x00,	/**/	0x77,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/
/* 134 */	0x78,	0x00,	/**/	0x78,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/
/* 135 */	0x79,	0x00,	/**/	0x79,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/
/* 136 */	0x7A,	0x00,	/**/	0x7A,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/
/* 137 */	0x7B,	0x00,	/**/	0x7B,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/
/* 138 */	0x7C,	0x00,	/**/	0x7C,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/
/* 139 */	0x7D,	0x00,	/**/	0x7D,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/
/* 140 */	0x7E,	0x00,	/**/	0x7E,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/
// ----//-------------------/**/--------------------/**/--------------------/**/--------------------/**/--------------------/**/--------------------/**/
// Seq //		720P25		/**/	    720P30		/**/		720P50		/**/		720P60		/**/		1080P25		/**/		1080P30		/**/
//	   //	 Reg     Dat	/**/	 Reg	 Dat	/**/	 Reg	 Dat	/**/	 Reg	 Dat	/**/	 Reg	 Dat	/**/	Reg		Dat		/**/
// ----//-------------------/**/--------------------/**/--------------------/**/--------------------/**/--------------------/**/--------------------/**/
/* 141 */	0x7F,	0x00,	/**/	0x7F,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/
/* 142 */	0x3D,	0x80,	/**/	0x3D,	0x80,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/
/* 143 */	0x5C,	0x4F,	/**/	0x5C,	0x4E,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/
/* 144 */	0x5D,	0x10,	/**/	0x5D,	0xE5,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/
/* 145 */	0x5E,	0x08,	/**/	0x5E,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/
/* 146 */	0x5F,	0x2C,	/**/	0x5F,	0xEE,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/
/* 147 */	0x1D,	0xB0,	/**/	0x1D,	0xB0,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/
/* 148 */	0x1E,	0xB0,	/**/	0x1E,	0xB0,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/
/* 149 */	0x37,	0xB0,	/**/	0x37,	0xB0,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/
/* 150 */	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/	0xFF,	0x00,	/**/
// -----------------------------------------------------------------------------------------------------------------------------------------------------
};

static void NVP6021_Init( NVP6021_VDO_MODE_e eVdoMode ) {
	uint8_t *pByte1;
	uint8_t i;
	
	//! Chip reset
	LCD_AHD_NVP6021_RST = 0;
	osDelay(20);
	LCD_AHD_NVP6021_RST = 1;
	osDelay(5);
	
	//! Read ID
	bAHD_NVP6021_Write(0xff, 0x00); // bank 0
	bAHD_NVP6021_Read(0x0E, pByte1);
	printf("NVP6021: ID=0x%02X\n", *pByte1);
	bAHD_NVP6021_Read(0x0F, pByte1);
	printf("NVP6021: Rev=0x%02X\n", *pByte1);
	
	//! Show vdo mode
	printf("NVP6021: Video mode = %s\n",	(eVdoMode==NVP6021_AHD_720P25 )?"NVP6021_AHD_720P25" :
											(eVdoMode==NVP6021_AHD_720P30 )?"NVP6021_AHD_720P30" :
											(eVdoMode==NVP6021_AHD_720P50 )?"NVP6021_AHD_720P50" :
											(eVdoMode==NVP6021_AHD_720P60 )?"NVP6021_AHD_720P60" :
											(eVdoMode==NVP6021_AHD_1080P25)?"NVP6021_AHD_1080P25":
											(eVdoMode==NVP6021_AHD_1080P30)?"NVP6021_AHD_1080P30":"Format Error!");
	if ( eVdoMode >= NVP6021_AHD_FMT_MAX ) LCD_AHD_NVP6021_ERR_HOOK;
	
	//! Init seq
	for ( i = 0; i < 0xff; i++ ) {
		bAHD_NVP6021_Write(NVP6021_InitTable[i][eVdoMode][0], NVP6021_InitTable[i][eVdoMode][1]);
		
		if ( (i == 0x06) && (eVdoMode==NVP6021_AHD_720P25||eVdoMode==NVP6021_AHD_720P30) )
			TIMER_Delay_us(100); //! Wait PLL reset complete
		
		if ( (i > 0x63) && (NVP6021_InitTable[i][eVdoMode][0] == 0xFF) )
			break;
	}
	
	//! Test pattern
#if NVP6021_TEST_PATTERN_EN
	printf("NVP6021: Try to open the test pattern.\r\n");
	switch ( eVdoMode ) {
		case NVP6021_AHD_720P25:
		case NVP6021_AHD_720P30: { //! 16bit data bus width
			bAHD_NVP6021_Write(0xFF, 0x01);
			bAHD_NVP6021_Write(0x03, 0xE0);
			bAHD_NVP6021_Write(0xFF, 0x02);
			bAHD_NVP6021_Write(0x04, 0x81);
		} break;
		case NVP6021_AHD_720P50:
		case NVP6021_AHD_720P60:
		case NVP6021_AHD_1080P25:
		case NVP6021_AHD_1080P30: { //! 16bit data bus width
			bAHD_NVP6021_Write(0xFF, 0x02);
			bAHD_NVP6021_Write(0x04, 0x81);
		}
		default: break;
	}
#endif //! #if NVP6021_TEST_PATTERN_EN
}


//! ------------------------------------------------------------------------------------------
//! 									SDK Interface
//! ------------------------------------------------------------------------------------------
void LCD_NVP6021_Init( void ) {
	//! Preset output format
#if ( NVP6021_DEBUG_EN ) //! Debug mode
	#if ( NVP6021_FORCE_OUTPUT_FORMAT == 0 ) //! AHD_720P25
		#if ( NVP6021_PRE_CONVERTER_TYPE == 0 )
			#error "[LCD]Error: AHD720P25 format is Not supported in LT9211+NVP6021 hardware environment!"
		#endif
	#elif ( NVP6021_FORCE_OUTPUT_FORMAT == 1 ) //! AHD_720P30
		eNVP6021_OutputMode = NVP6021_AHD_720P30;
	#elif ( NVP6021_FORCE_OUTPUT_FORMAT == 2 ) //! AHD_720P50
		eNVP6021_OutputMode = NVP6021_AHD_720P50;
	#elif ( NVP6021_FORCE_OUTPUT_FORMAT == 3 ) //! AHD_720P60
		eNVP6021_OutputMode = NVP6021_AHD_720P60;
	#elif ( NVP6021_FORCE_OUTPUT_FORMAT == 4 ) //! AHD_1080P25
		eNVP6021_OutputMode = NVP6021_AHD_1080P25;
	#elif ( NVP6021_FORCE_OUTPUT_FORMAT == 5 ) //! AHD_1080P30
		eNVP6021_OutputMode = NVP6021_AHD_1080P30;
	#else
		#error "NVP6021: Unknow Output Format!!!\r\n");
	#endif
#else //! UI Auto
	if ( !LCD_AHD_IS_SET_PAL_OUT() ) { //! NTSC
		eNVP6021_OutputMode = (NVP6021_VDO_MODE_e)((eNVP6021_OutputMode >= (NVP6021_AHD_FMT_MAX-1))? (NVP6021_AHD_FMT_MAX-1) : (eNVP6021_OutputMode+1));
	} else { //! PAL
	#if ( NVP6021_PRE_CONVERTER_TYPE == 0 )
		if ( eNVP6021_OutputMode == NVP6021_AHD_720P25 ) {
			printf("[LCD]Error: AHD720P25 format is Not supported in LT9211+NVP6021 hardware environment! Switch to AHD720P30...\r\n");
			eNVP6021_OutputMode = NVP6021_AHD_720P30;
		}
	#endif
	}
#endif //! #if ( NVP6021_DEBUG_EN ) //! Debug mode
	
	pLCD_I2C_Type = pI2C_MasterInit(I2C_2, I2C_SCL_400K);
	
    LCD->LCD_MODE = LCD_DE;
    LCD->SEL_TV = 0;

    LCD->LCD_HO_SIZE = BT1120_TimingTable[eNVP6021_OutputMode].hact;
    LCD->LCD_VO_SIZE = BT1120_TimingTable[eNVP6021_OutputMode].vact;
	
    // Timing
    LCD->LCD_HT_DM_SIZE = BT1120_TimingTable[eNVP6021_OutputMode].hfp;
    LCD->LCD_VT_DM_SIZE = BT1120_TimingTable[eNVP6021_OutputMode].vfp;
    LCD->LCD_HT_START = BT1120_TimingTable[eNVP6021_OutputMode].hbp+BT1120_TimingTable[eNVP6021_OutputMode].hs;
    LCD->LCD_VT_START = BT1120_TimingTable[eNVP6021_OutputMode].vbp+BT1120_TimingTable[eNVP6021_OutputMode].vs;

    LCD->LCD_HS_WIDTH = BT1120_TimingTable[eNVP6021_OutputMode].hs;
    LCD->LCD_VS_WIDTH = BT1120_TimingTable[eNVP6021_OutputMode].vs;
            
    LCD->LCD_PCK_SPEED = 24;
    LCD->LCD_RGB_REVERSE = 0;
    LCD->LCD_EVEN_RGB = 1;
    LCD->LCD_ODD_RGB = 1;
    LCD->LCD_HSYNC_HIGH = 0;
    LCD->LCD_VSYNC_HIGH = 0;
    LCD->LCD_DE_HIGH = 1;
    LCD->LCD_PCK_RIS = 1;
    LCD->LCD_FS0_FIELD_MODE = 0;
    LCD->LCD_FS1_FIELD_MODE = 0;
}

void AHD_NVP6021_Start( void ) {
	LT9211_Init(&(BT1120_TimingTable[eNVP6021_OutputMode]));
	NVP6021_Init(eNVP6021_OutputMode);
}
void AHD_NVP6021_DacSuspend( void ) {
	printf("NVP6021: DAC Suspend!\r\n");
	bAHD_NVP6021_Write(0xFF, 0x00);
	bAHD_NVP6021_Write(0x04, 0x01);
}
void AHD_NVP6021_DacResume( void ) {
	printf("NVP6021: DAC Resume!\r\n");
	bAHD_NVP6021_Write(0xFF, 0x00);
	bAHD_NVP6021_Write(0x04, 0x00);
}

void LCD_PixelPllSetting( void ) {
	float fPixelClock;
	float fFVCO;
	uint8_t ubFps;
	
	switch ( eNVP6021_OutputMode ) {
		case NVP6021_AHD_720P25: {
			LCD->LCD_PCK_SPEED = 2;
            GLB->LCDPLL_FRA = 0x60000;
            ubFps = 25;
		} break;
		case NVP6021_AHD_720P30: {
			LCD->LCD_PCK_SPEED = 2;
            GLB->LCDPLL_FRA = 0x60000;
            ubFps = 30;
		} break;
		case NVP6021_AHD_720P50: {
			LCD->LCD_PCK_SPEED = 2;
            GLB->LCDPLL_FRA = 0x60000;
            ubFps = 50;
		} break;
		case NVP6021_AHD_720P60: {
			LCD->LCD_PCK_SPEED = 2;
            GLB->LCDPLL_FRA = 0x60000;
            ubFps = 60;
		} break;
		case NVP6021_AHD_1080P25: {
            LCD->LCD_PCK_SPEED = 2;
            GLB->LCDPLL_FRA = 0x60000;
			ubFps = 25;
		} break;
		case NVP6021_AHD_1080P30: {
            LCD->LCD_PCK_SPEED = 2;
            GLB->LCDPLL_FRA = 0x60000;
			ubFps = 30;
		} break;
		default: break;
	}
	
	//! Calculation Pixel Clock
	fPixelClock = ((float)ubFps) * (LCD->LCD_HO_SIZE + LCD->LCD_HT_DM_SIZE + LCD->LCD_HT_START) *
								   (LCD->LCD_VO_SIZE + LCD->LCD_VT_DM_SIZE + LCD->LCD_VT_START) / 1000000;
    GLB->LCDPLL_INT = 12;
	
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
	
	printf("LCD: fFVCO=%f MHz\r\n", fFVCO); //range:148.3M ~ 165M
}


#endif //! #if (LCD_PANEL == LCD_AHD_NVP6021_RGB24)
