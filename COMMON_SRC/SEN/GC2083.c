/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		GC2083.c
	\brief		Sensor GC2053 relation function
	\author		BoCun
	\version	3.2
	\date		2020-09-09
	\copyright	Copyright(C) 2020 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "SEN.h"
#include "AE_API.h"
#include "I2C.h"
#include "TIMER.h"
#include "IQ_API.h"
#include "IQ_PARSER_API.h"
#include "BSP.h"

#if (SEN_USE == SEN_GC2083)
#define TRY_COUNTS 3
#define total		sizeof(gainLevelTable) / sizeof(uint32_t)
AE_EVENT_PROCESS tAE_SensorPara;
struct SENSOR_SETTING sensor_cfg;
tfSENObj xtSENInst;
I2C1_Type *pI2C_type;
SEN_STATE sensor_state;

static uint8_t regValTable[29][13] ={ 		
 //0x00d0  0x0dc1  0x00b8  0x00b9  0x0155  0x0410  0x0411  0x0412  0x0413  0x0414  0x0415  0x0416  0x0417
	{0x00, 0x00, 0x01, 0x00, 0x03, 0x11, 0x11, 0x11, 0x11, 0x6f, 0x6f, 0x6f, 0x6f},
	{0x10, 0x00, 0x01, 0x0c, 0x03, 0x11, 0x11, 0x11, 0x11, 0x6f, 0x6f, 0x6f, 0x6f},
	{0x01, 0x00, 0x01, 0x1a, 0x03, 0x11, 0x11, 0x11, 0x11, 0x6f, 0x6f, 0x6f, 0x6f},
	{0x11, 0x00, 0x01, 0x2b, 0x03, 0x11, 0x11, 0x11, 0x11, 0x6f, 0x6f, 0x6f, 0x6f},
	{0x02, 0x00, 0x02, 0x00, 0x03, 0x11, 0x11, 0x11, 0x11, 0x6f, 0x6f, 0x6f, 0x6f},
	{0x12, 0x00, 0x02, 0x18, 0x03, 0x11, 0x11, 0x11, 0x11, 0x6f, 0x6f, 0x6f, 0x6f},
	{0x03, 0x00, 0x02, 0x33, 0x03, 0x11, 0x11, 0x11, 0x11, 0x6f, 0x6f, 0x6f, 0x6f},
	{0x13, 0x00, 0x03, 0x15, 0x03, 0x11, 0x11, 0x11, 0x11, 0x6f, 0x6f, 0x6f, 0x6f},
	{0x04, 0x00, 0x04, 0x00, 0x03, 0x11, 0x11, 0x11, 0x11, 0x6f, 0x6f, 0x6f, 0x6f},
	{0x14, 0x00, 0x04, 0xe0, 0x03, 0x11, 0x11, 0x11, 0x11, 0x6f, 0x6f, 0x6f, 0x6f},
	{0x05, 0x00, 0x05, 0x26, 0x03, 0x11, 0x11, 0x11, 0x11, 0x6f, 0x6f, 0x6f, 0x6f},
	{0x15, 0x00, 0x06, 0x2b, 0x03, 0x11, 0x11, 0x11, 0x11, 0x6f, 0x6f, 0x6f, 0x6f},
	{0x44, 0x00, 0x08, 0x00, 0x03, 0x11, 0x11, 0x11, 0x11, 0x6f, 0x6f, 0x6f, 0x6f},
	{0x54, 0x00, 0x09, 0x22, 0x03, 0x11, 0x11, 0x11, 0x11, 0x6f, 0x6f, 0x6f, 0x6f},
	{0x45, 0x00, 0x0b, 0x0d, 0x03, 0x11, 0x11, 0x11, 0x11, 0x6f, 0x6f, 0x6f, 0x6f},	
	{0x55, 0x00, 0x0d, 0x16, 0x03, 0x11, 0x11, 0x11, 0x11, 0x6f, 0x6f, 0x6f, 0x6f},
	{0x04, 0x01, 0x10, 0x00, 0x19, 0x16, 0x16, 0x16, 0x16, 0x6f, 0x6f, 0x6f, 0x6f},
	{0x14, 0x01, 0x13, 0x04, 0x19, 0x16, 0x16, 0x16, 0x16, 0x6f, 0x6f, 0x6f, 0x6f},
	{0x24, 0x01, 0x16, 0x1a, 0x19, 0x16, 0x16, 0x16, 0x16, 0x6f, 0x6f, 0x6f, 0x6f},
	{0x34, 0x01, 0x1a, 0x2b, 0x19, 0x16, 0x16, 0x16, 0x16, 0x6f, 0x6f, 0x6f, 0x6f},
	{0x44, 0x01, 0x20, 0x00, 0x36, 0x18, 0x18, 0x18, 0x18, 0x6f, 0x6f, 0x6f, 0x6f},
	{0x54, 0x01, 0x26, 0x07, 0x36, 0x18, 0x18, 0x18, 0x18, 0x6f, 0x6f, 0x6f, 0x6f},
	{0x64, 0x01, 0x2c, 0x33, 0x36, 0x18, 0x18, 0x18, 0x18, 0x6f, 0x6f, 0x6f, 0x6f},
	{0x74, 0x01, 0x35, 0x17, 0x36, 0x18, 0x18, 0x18, 0x18, 0x6f, 0x6f, 0x6f, 0x6f},
	{0x84, 0x01, 0x35, 0x17, 0x64, 0x16, 0x16, 0x16, 0x16, 0x72, 0x72, 0x72, 0x72},
	{0x94, 0x01, 0x35, 0x17, 0x64, 0x16, 0x16, 0x16, 0x16, 0x72, 0x72, 0x72, 0x72},
	{0x85, 0x01, 0x35, 0x17, 0x64, 0x16, 0x16, 0x16, 0x16, 0x72, 0x72, 0x72, 0x72},
	{0x95, 0x01, 0x35, 0x17, 0x64, 0x16, 0x16, 0x16, 0x16, 0x72, 0x72, 0x72, 0x72},
	{0xa5, 0x01, 0x35, 0x17, 0x64, 0x16, 0x16, 0x16, 0x16, 0x72, 0x72, 0x72, 0x72},
};

 static uint32_t gainLevelTable[30] = {
	64, 
	77, 
	92, 
	110,
	128,
	154,
	186,
	223,
	269,
	323,
	381,
	457,
	544,
	653,
	762,
	914,
	1078,
	1293,
	1541,
	1849,
	2177,
	2612,
	3136,
	3764,
	4710,				
	5652,
	6656,
	7988,
	9474,							
	0xffffffff
};

const uint8_t ubSEN_InitTable[] = {
	//window_size=1928x1088 
	//mclk=24mhz,pclk=75mhz 
	//vts=1125,fps=30       
	// rowtime=29.6us       
	/*system*/              
	0x82, 0x03, 0xfe, 0xf0, 
	0x82, 0x03, 0xfe, 0xf0, 
	0x82, 0x03, 0xfe, 0xf0, 
	0x82, 0x03, 0xfe, 0x00, 
	0x82, 0x03, 0xf2, 0x00, 
	0x82, 0x03, 0xf3, 0x0f, 
	0x82, 0x03, 0xf4, 0x36, 
	0x82, 0x03, 0xf5, 0xc0, 
	0x82, 0x03, 0xf6, 0x24, 
	0x82, 0x03, 0xf7, 0x01, 
	0x82, 0x03, 0xf8, 0x32, 
	0x82, 0x03, 0xf9, 0x43, 
	0x82, 0x03, 0xfc, 0x8e, 
	0x82, 0x03, 0x81, 0x07, 
	0x82, 0x00, 0xd7, 0x29, 
	/*cisctl&analog*/       
	0x82, 0x0d, 0x6d, 0x18, 
	0x82, 0x00, 0xd5, 0x03, 
	0x82, 0x00, 0x82, 0x01, 
	0x82, 0x0d, 0xb3, 0xd4, 
	0x82, 0x0d, 0xb0, 0x0d, 
	0x82, 0x0d, 0xb5, 0x96, 
	0x82, 0x0d, 0x03, 0x01, 
	0x82, 0x0d, 0x04, 0x30, 
	0x82, 0x0d, 0x05, 0x05,   //2220
	0x82, 0x0d, 0x06, 0xc8, 
	0x82, 0x0d, 0x07, 0x00, 
	0x82, 0x0d, 0x08, 0x11, 
	0x82, 0x0d, 0x09, 0x00, 
	0x82, 0x0d, 0x0a, 0x02, 
	0x82, 0x00, 0x0b, 0x00, 
	0x82, 0x00, 0x0c, 0x02, 
	0x82, 0x0d, 0x0d, 0x04,   //1088
	0x82, 0x0d, 0x0e, 0x40, 
	0x82, 0x00, 0x0f, 0x07,   //1936
	0x82, 0x00, 0x10, 0x90, 
	0x82, 0x0d, 0x13, 0x14, 
	0x82, 0x00, 0x17, 0x0c, 
	0x82, 0x0d, 0x73, 0x92, 
	0x82, 0x0d, 0x76, 0x00, 
	0x82, 0x00, 0x76, 0x00, 
	0x82, 0x0d, 0x41, 0x04,   //1125
	0x82, 0x0d, 0x42, 0x65, 
	0x82, 0x0d, 0x7a, 0x10, 
	0x82, 0x0d, 0x19, 0x31, 
	0x82, 0x0d, 0x25, 0x0b, 
	0x82, 0x0d, 0x20, 0x60, 
	0x82, 0x0d, 0x27, 0x03, 
	0x82, 0x0d, 0x29, 0x60, 
	0x82, 0x0d, 0x43, 0x10, 
	0x82, 0x0d, 0x49, 0x10, 
	0x82, 0x0d, 0x55, 0x18, 
	0x82, 0x0d, 0xc2, 0x44, 
	0x82, 0x00, 0x58, 0x3c, 
	0x82, 0x00, 0xd8, 0x68, 
	0x82, 0x00, 0xd9, 0x14, 
	0x82, 0x00, 0xda, 0xc1, 
	0x82, 0x00, 0x50, 0x18, 
	0x82, 0x00, 0x85, 0x00, 
	0x82, 0x00, 0x86, 0x10, 
	0x82, 0x0d, 0xb6, 0x3d, 
	0x82, 0x00, 0xd2, 0xbc, 
	0x82, 0x0d, 0x66, 0x42, 
	0x82, 0x00, 0x8c, 0x07, 
	0x82, 0x00, 0x8d, 0xff, 
										
	/*gain*/               
	0x82, 0x00, 0x7a, 0x50, 
	0x82, 0x00, 0xd0, 0x00, 
	0x82, 0x0d, 0xc1, 0x00, 
										
	/*dark sun*/           
	0x82, 0x00, 0x77, 0x01, 
	0x82, 0x00, 0x78, 0x65, 
	0x82, 0x00, 0x79, 0x04, 
	0x82, 0x00, 0x67, 0xa0, 
	0x82, 0x00, 0x54, 0xff, 
	0x82, 0x00, 0x55, 0x02, 
	0x82, 0x00, 0x56, 0x00, 
	0x82, 0x00, 0x57, 0x04, 
	0x82, 0x00, 0x5a, 0xff, 
	0x82, 0x00, 0x5b, 0x07, 
										
	/*blk*/                 
	0x82, 0x00, 0x26, 0x01, 
	0x82, 0x01, 0x52, 0x02, 
	0x82, 0x01, 0x53, 0x50, 
	0x82, 0x01, 0x55, 0x93, 
	0x82, 0x04, 0x10, 0x16, 
	0x82, 0x04, 0x11, 0x16, 
	0x82, 0x04, 0x12, 0x16, 
	0x82, 0x04, 0x13, 0x16, 
	0x82, 0x04, 0x14, 0x6f, 
	0x82, 0x04, 0x15, 0x6f, 
	0x82, 0x04, 0x16, 0x6f, 
	0x82, 0x04, 0x17, 0x6f, 
	0x82, 0x04, 0xe0, 0x18, 
										
	/*window*/             
	0x82, 0x00, 0x15, 0x00, 
	0x82, 0x0d, 0x15, 0x00, 
	0x82, 0x01, 0x92, 0x00, 
	0x82, 0x01, 0x94, 0x08, 
	0x82, 0x01, 0x95, 0x04, 
										
	0x82, 0x01, 0x96, 0x40, 
	0x82, 0x01, 0x97, 0x07, 
	0x82, 0x01, 0x98, 0x80, 
	0x82, 0x01, 0x99, 0x00, 
										
	/*isp*/                
	0x82, 0x01, 0x02, 0xa9, 
	0x82, 0x01, 0x58, 0x00, 
	0x82, 0x01, 0x07, 0xa6, 
	0x82, 0x01, 0x08, 0xa9, 
	0x82, 0x01, 0x09, 0xa8, 
	0x82, 0x01, 0x0a, 0xa7, 
	0x82, 0x01, 0x0b, 0xff, 
	0x82, 0x01, 0x0c, 0xff, 
										
	0x82, 0x04, 0x28, 0x86, 
	0x82, 0x04, 0x29, 0x86, 
	0x82, 0x04, 0x2a, 0x86, 
	0x82, 0x04, 0x2b, 0x68, 
	0x82, 0x04, 0x2c, 0x68, 
	0x82, 0x04, 0x2d, 0x68, 
	0x82, 0x04, 0x2e, 0x68, 
	0x82, 0x04, 0x2f, 0x68, 
										
	0x82, 0x04, 0x30, 0x4f, 
	0x82, 0x04, 0x31, 0x68, 
	0x82, 0x04, 0x32, 0x67, 
	0x82, 0x04, 0x33, 0x66, 
	0x82, 0x04, 0x34, 0x66, 
	0x82, 0x04, 0x35, 0x66, 
	0x82, 0x04, 0x36, 0x66, 
	0x82, 0x04, 0x37, 0x66, 
										
	0x82, 0x04, 0x38, 0x62, 
	0x82, 0x04, 0x39, 0x62, 
	0x82, 0x04, 0x3a, 0x62, 
	0x82, 0x04, 0x3b, 0x62, 
	0x82, 0x04, 0x3c, 0x62, 
	0x82, 0x04, 0x3d, 0x62, 
	0x82, 0x04, 0x3e, 0x62, 
	0x82, 0x04, 0x3f, 0x62, 
										
	/*DVP & MIPI*/         
	0x82, 0x02, 0x90, 0x1f, 
	0x82, 0x00, 0xdc, 0x03, 
	0x82, 0x00, 0xdd, 0xea, 
	0x82, 0x02, 0x01, 0x20, 
	0x82, 0x02, 0x12, 0x07, 
	0x82, 0x02, 0x13, 0x80, // 88
	0x82, 0x02, 0x3e, 0x40,         
};


//------------------------------------------------------------------------------
bool bSEN_I2C_Read(uint16_t uwAddress, uint8_t *pValue)
{
		uint8_t *pAddr, pBuf[2];
		
		pAddr = (uint8_t*)&uwAddress;
		pBuf[0] = pAddr[1];
		pBuf[1] = pAddr[0];
	
    #if (I2C_TYPE_SELECT == I2C_TYPE_POLLING)
        return bI2C_MasterProcess (pI2C_type, SEN_SLAVE_ADDR, &pBuf[0], 2, pValue, 1);
    #else
        return bI2C_MasterInt (pI2C_type, I2C_INT, SEN_SLAVE_ADDR, &pBuf[0], 2, pValue, 1);
    #endif
}

//------------------------------------------------------------------------------
bool bSEN_I2C_Write(uint8_t ubAddress1, uint8_t ubAddress2, uint8_t ubValue)
{	
		uint8_t pBuf[3];
		
		pBuf[0] = ubAddress1;
		pBuf[1] = ubAddress2;
		pBuf[2] = ubValue;	

    #if (I2C_TYPE_SELECT == I2C_TYPE_POLLING)
        return bI2C_MasterProcess (pI2C_type, SEN_SLAVE_ADDR, &pBuf[0], 3, NULL, 0);
    #else
        return bI2C_MasterInt (pI2C_type, I2C_INT, SEN_SLAVE_ADDR, &pBuf[0], 3, NULL, 0);
    #endif
}

//------------------------------------------------------------------------------
bool bSEN_I2C_WriteTry(uint16_t uwAddress, uint8_t ubValue, uint8_t ubTryCnt)
{	
    uint8_t pBuf[3], ret, i=0;
    
    pBuf[0] = (uint8_t)((uwAddress>>8) & 0x00ff);        
    pBuf[1] = (uwAddress & 0x00ff);
    pBuf[2] = ubValue;
       
    do{
    #if (I2C_TYPE_SELECT == I2C_TYPE_POLLING)
        ret = bI2C_MasterProcess (pI2C_type, SEN_SLAVE_ADDR, &pBuf[0], 3, NULL, 0);
    #else
        ret = bI2C_MasterInt (pI2C_type, I2C_INT, SEN_SLAVE_ADDR, &pBuf[0], 3, NULL, 0);
    #endif
    }while((ret == 0) && ((i++) < ubTryCnt));
    if(ret == 0)
        printf("wr fail 0x%x.\r\n",uwAddress);    
    
		return ret;
}

//------------------------------------------------------------------------------
void SEN_PclkSetting(uint8_t ubPclkIdx)
{
    uint16_t uwPPL;
    uint32_t ulPCK; 
    // support 1-30 fps for 1280x800
    if(ubPclkIdx > 30)
    {
				ubPclkIdx = 30;
    }
    // set sensor struct value
    tAE_SensorPara.ulSensorPclk = 75000000;  
    tAE_SensorPara.ulSensorPixelPerLine = 2220;   //1600
    tAE_SensorPara.ulSensorLinePerFrame = 1125;  //1562
    tAE_SensorPara.ulSensorFrameRate = ubPclkIdx;     
    tAE_SensorPara.ulMaximumSensorFrameRate = 30;
 
    //auto calculat Max Exposure
	  ulPCK = tAE_SensorPara.ulSensorPclk;
	  uwPPL = (unsigned short)(tAE_SensorPara.ulSensorPixelPerLine);
       
    if ((ubPclkIdx == 0) || (ubPclkIdx > tAE_SensorPara.ulMaximumSensorFrameRate))
    {
        xtSENInst.uwMaxExpLine = (ulPCK / tAE_SensorPara.ulMaximumSensorFrameRate / (unsigned int)uwPPL);
    }else{
        xtSENInst.uwMaxExpLine = (ulPCK / tAE_SensorPara.ulSensorFrameRate / (unsigned int)uwPPL);
    }
    AE_EventProcess(tAE_SensorPara);
}

//------------------------------------------------------------------------------
void SEN_SensorHwReset(void)
{
    SENSOR_RESET_OUT_EN = 1;
    SENSOR_RESET_OUT = 0;
    TIMER_Delay_ms(30);
    SENSOR_RESET_OUT = 0;
    TIMER_Delay_ms(30);
}

//------------------------------------------------------------------------------
uint8_t ubSEN_CheckSensorState(void)
{
		uint8_t     *pBuf;	
		uint16_t 	uwPID = 0;
				
		pBuf = (uint8_t*)&uwPID;  
		// I2C by Read Sensor ID
		bSEN_I2C_Read (GC2083_CHIP_ID_HIGH_ADDR, &pBuf[1]);
		bSEN_I2C_Read (GC2083_CHIP_ID_LOW_ADDR, &pBuf[0]);
		if (GC2083_CHIP_ID != uwPID)
		{
			printd(DBG_ErrorLvl, "Read chip ID fail.<0x%x 0x%x>\n", GC2083_CHIP_ID, uwPID);
					return 0;
		}
			return 1;
}

//------------------------------------------------------------------------------
uint8_t ubSEN_SetSensorInitTable(void)
{
		I2C_SCL_SPEED_TYP tI2C_Clock = I2C_SCL_300K;
		uint32_t 	i;	
			
		pI2C_type = pI2C_MasterInit(I2C_2, tI2C_Clock);
		IQ_SetI2cType(pI2C_type, tI2C_Clock);
			//HW reset.
		SEN_SensorHwReset();
			
		if(ubSEN_CheckSensorState() != 1)
				return 0;
		printd(DBG_CriticalLvl, "GC2083 Sensor\n");
		for (i=0; i<sizeof(ubSEN_InitTable); i+=4)
		{
			if (ubSEN_InitTable[i] == 0x82)	// write
			{
				bSEN_I2C_Write(ubSEN_InitTable[i+1], ubSEN_InitTable[i+2], ubSEN_InitTable[i+3]);
			}else if (ubSEN_InitTable[i] == 0xbb){
							TIMER_Delay_ms(((ubSEN_InitTable[i+1]<<8) + ubSEN_InitTable[i+2]));
					}
		}
		return 1;
}

//------------------------------------------------------------------------------
uint8_t ubSEN_Open(struct SENSOR_SETTING *setting)
{
    SEN_AeCbFunc();

    // Set ISP clock
    SEN_SetISPRate(3);
    // Set parallel mode
    SEN->MIPI_MODE = 0;
    // Set sensor clock
    SEN_SetSensorRate(SENSOR_96MHz, 4);	
    // enable sensor PCLK
    SEN->SEN_CLK_EN = 1;
    //Set ISP pipeline
    sensor_cfg.tPathType = BAYER_SENSOR_NORMAL;
    SEN_SenosrOutputType(sensor_cfg.tPathType);
    // Change ISP_LH_SEL to 0, for 10bit raw input
    SEN->ISP_LH_SEL = 0;
    // Timing
    SEN->VSYNC_RIS = 1;
    SEN->HSYNC_RIS = 1;
    SEN->VSYNC_HIGH = 1;
    SEN->PCK_DLH_RIS = 1;
    SEN->SYNC_MODE = 1;
    SEN->SENSOR_MODE = 1;
    // change SN_SEN->RAW_REORDER to 2, start from Gr
    SEN->RAW_REORDER = 2;
    // set dummy line & pixel
    SEN->DMY_DIV = 3;  
    SEN->NUM_DMY_LN = 16;
    SEN->NUM_DMY_DSTB = 256;
    SEN->DMY_INSERT = 1;
    
    //delay 1ms
    TIMER_Delay_us(1000);		
    // Initial dummy sensor
    if (ubSEN_SetSensorInitTable() != 1)
    {
        printf("GC2083 startup failed! \n\r");
        return 0;
    }
    SEN_PclkSetting(SEN_FPS30);
    // clear all debug flag
    SEN->REG_0x1300 = 0x1ff;
    SEN->REG_0x1304 = 0x3;
    MIPI->REG_0x1804 = 0xff;
    MIPI->REG_0x1814 = 0x1f0000;
    // Unable write to dram.
    SEN->IMG_TX_EN = 0;        
    // enable interrupt
    SEN->HW_END_INT_EN = 1;
    SEN->SEN_VSYNC_INT_EN = 1;
    SEN->SEN_HSYNC_INT_EN = 1;
    
    return 1;
}

//------------------------------------------------------------------------------
uint16_t uwSEN_CalExpLine(uint32_t ulAlgExpTime)
{
    return (ulSEN_GetPixClk() / 1000000L * (uint32_t)ulAlgExpTime / (uint32_t)ulSEN_GetPckPerLine() / 10);
}

//------------------------------------------------------------------------------
void SEN_CalExpLDmyL(uint32_t ulAlgExpTime)
{
		//Transfor the ExpLine of Algorithm	to ExpLine and DmyLine of Sensor here
		xtSENInst.xtSENCtl.ulExpTime = ulAlgExpTime;
		xtSENInst.xtSENCtl.uwExpLine = uwSEN_CalExpLine(ulAlgExpTime);

		if(xtSENInst.xtSENCtl.uwExpLine < 1)
		{
			xtSENInst.xtSENCtl.uwExpLine = 1;
		}	
					
		//Calculate Dummy line
		if(xtSENInst.xtSENCtl.uwExpLine <= xtSENInst.uwMaxExpLine)
		{
			xtSENInst.xtSENCtl.uwDmyLine = xtSENInst.uwMaxExpLine;
		}
		else
		{
			xtSENInst.xtSENCtl.uwDmyLine =xtSENInst.xtSENCtl.uwExpLine;
		}
}

//------------------------------------------------------------------------------
uint32_t ulSEN_GetPixClk(void)
{
		return (tAE_SensorPara.ulSensorPclk);
}

//------------------------------------------------------------------------------
uint32_t ulSEN_GetPckPerLine(void)
{
		return (tAE_SensorPara.ulSensorPixelPerLine);
}

//------------------------------------------------------------------------------
void SEN_WriteTotalLine(void)
{

}

//------------------------------------------------------------------------------
void SEN_SetDummyLine(void)
{
    SEN_WrDummyLine(xtSENInst.xtSENCtl.uwDmyLine);
}

//------------------------------------------------------------------------------
void SEN_WrDummyLine(uint16_t uwDL)
{   
    bSEN_I2C_WriteTry(GC2083_FRAME_H, (uint8_t)((uwDL>>8) & 0x0f), TRY_COUNTS);
    bSEN_I2C_WriteTry(GC2083_FRAME_L, (uint8_t)(uwDL & 0xff), TRY_COUNTS);	
}

//------------------------------------------------------------------------------
void SEN_SetExpLine(void)
{
    SEN_WrExpLine(xtSENInst.xtSENCtl.uwExpLine);
}

//------------------------------------------------------------------------------
void SEN_WrExpLine(uint16_t uwExpLine)
{
		bSEN_I2C_WriteTry(GC2083_EXP_H, (uint8_t)((uwExpLine>>8) & 0x3f), TRY_COUNTS);
		bSEN_I2C_WriteTry(GC2083_EXP_L, (uint8_t)(uwExpLine & 0xff), TRY_COUNTS);	
}

//------------------------------------------------------------------------------
void SEN_WrMaxExpLine(uint32_t ulValue)
{
    xtSENInst.uwMaxExpLine = ulValue;
}

//------------------------------------------------------------------------------
void SEN_WrGain(uint32_t ulGainX1024)
{

		uint8_t i;
		uint32_t tol_dig_gain = 0;
		
		ulGainX1024 = ulGainX1024/16;
		for(i = 0; i < total; i++)
			{
				if((gainLevelTable[i] <= ulGainX1024)&&(ulGainX1024 < gainLevelTable[i+1]))
					break;
			}
			
		tol_dig_gain = ulGainX1024*64/gainLevelTable[i];	

		bSEN_I2C_WriteTry(0x00d0, regValTable[i][0], TRY_COUNTS);
		bSEN_I2C_WriteTry(0x031d, 0x2e, TRY_COUNTS);
		bSEN_I2C_WriteTry(0x0dc1, regValTable[i][1], TRY_COUNTS);
		bSEN_I2C_WriteTry(0x031d, 0x28, TRY_COUNTS);
		bSEN_I2C_WriteTry(0x00b8, regValTable[i][2], TRY_COUNTS);
		bSEN_I2C_WriteTry(0x00b9, regValTable[i][3], TRY_COUNTS);
		bSEN_I2C_WriteTry(0x0155, regValTable[i][4], TRY_COUNTS);

		bSEN_I2C_WriteTry(0x0410, regValTable[i][5], TRY_COUNTS);
		bSEN_I2C_WriteTry(0x0411, regValTable[i][6], TRY_COUNTS);
		bSEN_I2C_WriteTry(0x0412, regValTable[i][7], TRY_COUNTS);
		bSEN_I2C_WriteTry(0x0413, regValTable[i][8], TRY_COUNTS);
		bSEN_I2C_WriteTry(0x0414, regValTable[i][9], TRY_COUNTS);
		bSEN_I2C_WriteTry(0x0415, regValTable[i][10], TRY_COUNTS);
		bSEN_I2C_WriteTry(0x0416, regValTable[i][11], TRY_COUNTS);
		bSEN_I2C_WriteTry(0x0417, regValTable[i][12], TRY_COUNTS);

		bSEN_I2C_WriteTry(0x00b1, (tol_dig_gain>>6), TRY_COUNTS);
		bSEN_I2C_WriteTry(0x00b2, ((tol_dig_gain&0x3f)<<2), TRY_COUNTS);
	
}

//------------------------------------------------------------------------------
void SEN_SetMirrorFlip(uint8_t ubMirrorEn, uint8_t ubFlipEn)
{

    if(!ubIQ_GetLoadIqFinishFg())
        return;
    
    if(ubMirrorEn)
        xtSENInst.ubImgMode |=  GC2083_MIRROR;
    else
				xtSENInst.ubImgMode &=  ~GC2083_MIRROR;
		
	

    if(ubFlipEn)
        xtSENInst.ubImgMode |=  GC2083_FLIP;
    else
        xtSENInst.ubImgMode &=  ~GC2083_FLIP;
		
		bSEN_I2C_Write(0x0d,0x15, xtSENInst.ubImgMode);
	  bSEN_I2C_Write(0x00,0x15, xtSENInst.ubImgMode);
		
    //
    SEN_SetRawReorder(ubMirrorEn, ubFlipEn);  
} 

//------------------------------------------------------------------------------
void SEN_GroupHoldOnVSync(void)
{

}

//------------------------------------------------------------------------------
void SEN_GroupHoldOffVSync(void)
{
//	bSEN_I2C_Write(0x1f, 0x80);
}

//------------------------------------------------------------------------------
void SEN_SetSensorImageSize(void)
{
    // Image for ISP
    sensor_cfg.xtSENWin.uwHSize = ISP_WIDTH;
    sensor_cfg.xtSENWin.uwVSize = ISP_HEIGHT;
    sensor_cfg.xtSENWin.uwHStart = 0;
    sensor_cfg.xtSENWin.uwVStart = 0;	
}

//------------------------------------------------------------------------------
void SEN_SetSensorType(void)
{
		sensor_cfg.ubSensorType = SEN_GC2083;    

		printd(DBG_Debug1Lvl, "sensor type is GC2083\n");

}

//------------------------------------------------------------------------------
void SEN_AeCbFunc(void)
{
    tAE_RegXuCbFunc(AE_OPC_GROUP_HOLD_ON, SEN_GroupHoldOnVSync);
    tAE_RegXuCbFunc(AE_OPC_GROUP_HOLD_OFF, SEN_GroupHoldOffVSync);
    tAE_RegXuCbFunc(AE_OPC_DUMMY_LINE, SEN_SetDummyLine);
    tAE_RegXuCbFunc(AE_OPC_EXP_LINE, SEN_SetExpLine);
    tAE_RegXu2CbFunc(AE_OPC2_GAIN, SEN_WrGain);
    tAE_RegXu2CbFunc(AE_OPC2_CAL_EXP_DUM, SEN_CalExpLDmyL);
    tAE_RegXu2CbFunc(AE_OPC2_MAX_EXP_LINE, SEN_WrMaxExpLine);
    AE_CalExpLineCbFunc(uwSEN_CalExpLine);
}

//------------------------------------------------------------------------------
int32_t slSensorEnvir[11][6] = {
    { 1500, 20000,     5,   1024,   1,     8},
    { 1395,  1415,    10,   1024,   1,    16},
    { 1350,  1360,    20,   1024,   1,    64},
    { 1200,  1250,   100,   1024,   1,   280},
    { 1150,  1190,   200,   1024,   1,   360},
    { 1000,  1100,   360,   1536,   1,   512},
    {  800,   850,   720,   2048,   1,   512},
    {  400,   450,   720,   2048,   1,   512},
    {  200,   250,   720,   2560,   2,   512},
    {  100,   150,  1080,   3072,   3,   512},
    {    5,    40,  2880,   6144,   6,  1024},
};

void SEN_SetSensorEnvironment(uint32_t ulExp, uint32_t ulGain)
{


		//EXPLINE
		bSEN_I2C_WriteTry(GC2083_EXP_H, (uint8_t)((ulExp>>8) & 0xff), TRY_COUNTS);
		bSEN_I2C_WriteTry(GC2083_EXP_L, (uint8_t)(ulExp & 0xff), TRY_COUNTS);  

		//GAIN
		uint8_t i;
		uint32_t tol_dig_gain = 0;
		uint32_t ulGainX1024;
		
		ulGainX1024 = ulGainX1024/16;
		for(i = 0; i < total; i++)
			{
				if((gainLevelTable[i] <= ulGainX1024)&&(ulGainX1024 < gainLevelTable[i+1]))
					break;
			}
			
		tol_dig_gain = ulGainX1024*64/gainLevelTable[i];	

		bSEN_I2C_WriteTry(0x00d0, regValTable[i][0], TRY_COUNTS);
		bSEN_I2C_WriteTry(0x031d, 0x2e, TRY_COUNTS);
		bSEN_I2C_WriteTry(0x0dc1, regValTable[i][1], TRY_COUNTS);
		bSEN_I2C_WriteTry(0x031d, 0x28, TRY_COUNTS);
		bSEN_I2C_WriteTry(0x00b8, regValTable[i][2], TRY_COUNTS);
		bSEN_I2C_WriteTry(0x00b9, regValTable[i][3], TRY_COUNTS);
		bSEN_I2C_WriteTry(0x0155, regValTable[i][4], TRY_COUNTS);

		bSEN_I2C_WriteTry(0x0410, regValTable[i][5], TRY_COUNTS);
		bSEN_I2C_WriteTry(0x0411, regValTable[i][6], TRY_COUNTS);
		bSEN_I2C_WriteTry(0x0412, regValTable[i][7], TRY_COUNTS);
		bSEN_I2C_WriteTry(0x0413, regValTable[i][8], TRY_COUNTS);
		bSEN_I2C_WriteTry(0x0414, regValTable[i][9], TRY_COUNTS);
		bSEN_I2C_WriteTry(0x0415, regValTable[i][10], TRY_COUNTS);
		bSEN_I2C_WriteTry(0x0416, regValTable[i][11], TRY_COUNTS);
		bSEN_I2C_WriteTry(0x0417, regValTable[i][12], TRY_COUNTS);

		bSEN_I2C_WriteTry(0x00b1, (tol_dig_gain>>6), TRY_COUNTS);
		bSEN_I2C_WriteTry(0x00b2, ((tol_dig_gain&0x3f)<<2), TRY_COUNTS);
}

void SEN_SetEnvironment(void)
{  
    int32_t slResTemp[0x40];
    uint32_t ulValue;
    uint8_t RowTest ,ColTest;
    
    ulValue = ulAE_AvgOfYSum();
    RowTest = sizeof(slSensorEnvir) / sizeof(slSensorEnvir[0]);
    ColTest = sizeof(slSensorEnvir[0]) / sizeof(slSensorEnvir[0])[0];
    IQ_DynamicLinearInterpolation(ulValue, RowTest, ColTest, &slSensorEnvir[0][0], slResTemp);

    SEN_SetSensorEnvironment(slResTemp[0], slResTemp[1]);
    AE_SetEnvironment(slResTemp[2], slResTemp[3]);
}
#endif
