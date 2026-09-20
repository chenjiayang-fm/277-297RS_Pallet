/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		OV9750.c
	\brief		Sensor OV9750 relation function
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
#include "IQ_PARSER_API.h"
#include "BSP.h"

#if (SEN_USE == SEN_OV9750)
#define TRY_COUNTS      3
AE_EVENT_PROCESS tAE_SensorPara;
struct SENSOR_SETTING sensor_cfg;
tfSENObj xtSENInst;
I2C1_Type *pI2C_type;

const uint8_t ubSEN_InitTable[] = {
    //OV9750 960P 60FPS DVP Test ADC10bits
	0x83, 0x01, 0x00, 0x00,
	0x83, 0x03, 0x00, 0x02,
	0x83, 0x03, 0x02, 0x50,
	0x83, 0x03, 0x03, 0x01,
	0x83, 0x03, 0x04, 0x01,
	0x83, 0x03, 0x05, 0x01,
	0x83, 0x03, 0x06, 0x01,
	0x83, 0x03, 0x0a, 0x00,
	0x83, 0x03, 0x0b, 0x00,     //prediv = 0x030B[3:0] = 1.
	0x83, 0x03, 0x0d, 0x1e,     //multiplier = 0x030D[7:0] = 30.
	0x83, 0x03, 0x0e, 0x01,     //sys_divider = 0x030E[2:0] = 1.5
	0x83, 0x03, 0x0f, 0x04,     //sys_pre_div = 1 + 0x030F[3:0] = 5.
	0x83, 0x03, 0x12, 0x01,     //prediv0 = 0x0312[4] = 1.
	0x83, 0x03, 0x1e, 0x04,
	0x83, 0x30, 0x00, 0x0f,
	0x83, 0x30, 0x01, 0xff,
	0x83, 0x30, 0x02, 0xe1,
	0x83, 0x30, 0x05, 0xf0,
	0x83, 0x30, 0x11, 0x20,
	0x83, 0x30, 0x16, 0x00,
	0x83, 0x30, 0x18, 0x32,
	0x83, 0x30, 0x1a, 0xf0,
	0x83, 0x30, 0x1b, 0xf0,
	0x83, 0x30, 0x1c, 0xf0,
	0x83, 0x30, 0x1d, 0xf0,
	0x83, 0x30, 0x1e, 0xf0,
	0x83, 0x30, 0x22, 0x21,
	0x83, 0x30, 0x31, 0x0a,
    //0x3030[7] = 0, PLL1. =1, PLL2.
	0x83, 0x30, 0x32, 0x80,
	0x83, 0x30, 0x3c, 0xff,
	0x83, 0x30, 0x3e, 0xff,
	0x83, 0x30, 0x40, 0xf0,
	0x83, 0x30, 0x41, 0x00,
	0x83, 0x30, 0x42, 0xf0,
	0x83, 0x31, 0x04, 0x01,
	0x83, 0x31, 0x06, 0x15,     //sys_pre_div = 0x3106[3:2] = 2, Sclk_pdiv = 0x3106[7:4] = 1.
	0x83, 0x31, 0x07, 0x01,
	0x83, 0x35, 0x00, 0x00,
	0x83, 0x35, 0x01, 0x3d,
	0x83, 0x35, 0x02, 0x00,
	0x83, 0x35, 0x03, 0x08,
	0x83, 0x35, 0x04, 0x03,
	0x83, 0x35, 0x05, 0x83,
	0x83, 0x35, 0x08, 0x02,
	0x83, 0x35, 0x09, 0x80,
	0x83, 0x36, 0x00, 0x65,
	0x83, 0x36, 0x01, 0x60,
	0x83, 0x36, 0x02, 0x22,
	0x83, 0x36, 0x10, 0xb8,
	0x83, 0x36, 0x12, 0x18,
	0x83, 0x36, 0x13, 0x3a,
	0x83, 0x36, 0x15, 0x79,
	0x83, 0x36, 0x17, 0x07,
	0x83, 0x36, 0x21, 0x90,
	0x83, 0x36, 0x22, 0x00,
	0x83, 0x36, 0x23, 0x00,
	0x83, 0x36, 0x33, 0x10,
	0x83, 0x36, 0x34, 0x10,
	0x83, 0x36, 0x35, 0x10,
	0x83, 0x36, 0x36, 0x10,
	0x83, 0x36, 0x50, 0x00,
	0x83, 0x36, 0x52, 0xff,
	0x83, 0x36, 0x54, 0x20,
	0x83, 0x36, 0x53, 0x34,
	0x83, 0x36, 0x55, 0x20,
	0x83, 0x36, 0x56, 0xff,
	0x83, 0x36, 0x57, 0xc4,
	0x83, 0x36, 0x5a, 0xff,
	0x83, 0x36, 0x5b, 0xff,
	0x83, 0x36, 0x5e, 0xff,
	0x83, 0x36, 0x5f, 0x00,
	0x83, 0x36, 0x68, 0x00,
	0x83, 0x36, 0x6a, 0x07,
	0x83, 0x36, 0x6d, 0x00,
	0x83, 0x36, 0x6e, 0x10,
	0x83, 0x37, 0x02, 0x1d,
	0x83, 0x37, 0x03, 0x10,
	0x83, 0x37, 0x04, 0x14,
	0x83, 0x37, 0x05, 0x00,
	0x83, 0x37, 0x06, 0x27,
	0x83, 0x37, 0x09, 0x24,
	0x83, 0x37, 0x0a, 0x00,
	0x83, 0x37, 0x0b, 0x7d,
	0x83, 0x37, 0x14, 0x24,
	0x83, 0x37, 0x1a, 0x5e,
	0x83, 0x37, 0x30, 0x82,
	0x83, 0x37, 0x33, 0x00,
	0x83, 0x37, 0x3e, 0x18,
	0x83, 0x37, 0x55, 0x00,
	0x83, 0x37, 0x58, 0x00,
	0x83, 0x37, 0x5b, 0x13,
	0x83, 0x37, 0x72, 0x23,
	0x83, 0x37, 0x73, 0x05,
	0x83, 0x37, 0x74, 0x16,
	0x83, 0x37, 0x75, 0x12,
	0x83, 0x37, 0x76, 0x08,
	0x83, 0x37, 0xa8, 0x38,
	0x83, 0x37, 0xb5, 0x36,
	0x83, 0x37, 0xc2, 0x04,
	0x83, 0x37, 0xc5, 0x00,
	0x83, 0x37, 0xc7, 0x01,
	0x83, 0x37, 0xc8, 0x00,
	0x83, 0x37, 0xd1, 0x13,
    // H_CROP_START = (0x3800[2:0]<<8) + 0x3801[7:0] = 0
	0x83, 0x38, 0x00, 0x00,
	0x83, 0x38, 0x01, 0x00,
    // V_CROP_START = (0x3802[2:0]<<8) + 0x3803[7:0] = 4
	0x83, 0x38, 0x02, 0x00,
	0x83, 0x38, 0x03, 0x04,
    // H_CROP_END = (0x3804[2:0]<<8) + 0x3805[7:0] = 1295
	0x83, 0x38, 0x04, 0x05,
	0x83, 0x38, 0x05, 0x0f,
    // V_CROP_END = (0x3806[2:0]<<8) + 0x3807[7:0] = 971
	0x83, 0x38, 0x06, 0x03,
	0x83, 0x38, 0x07, 0xcb,
    // H_OUTPUT_SIZE = (0x3808[2:0]<<8) + 0x3809[7:0] = 1280
	0x83, 0x38, 0x08, 0x05,
	0x83, 0x38, 0x09, 0x00,
    // V_OUTPUT_SIZE = (0x380A[2:0]<<8) + 0x380B[7:0] = 960
	0x83, 0x38, 0x0a, 0x03,
	0x83, 0x38, 0x0b, 0xc0,
    // HTS = (0x380C[2:0]<<8) + 0x380D[7:0] = 810
	0x83, 0x38, 0x0c, 0x03,
	0x83, 0x38, 0x0d, 0x2a,
    // VTS = (0x380E[2:0]<<8) + 0x380F[7:0] = 988
	0x83, 0x38, 0x0e, 0x03,
	0x83, 0x38, 0x0f, 0xdc,
    // H_WIN_OFF = (0x3810[2:0]<<8) + 0x3811[7:0] = 8
	0x83, 0x38, 0x10, 0x00,
	0x83, 0x38, 0x11, 0x08,
    // V_WIN_OFF = (0x3812[2:0]<<8) + 0x3813[7:0] = 4
	0x83, 0x38, 0x12, 0x00,
	0x83, 0x38, 0x13, 0x04,
    // H_INC_ODD = (0x3814) = 1
    // H_INC_EVEN = (0x3815) = 1
	0x83, 0x38, 0x14, 0x01,
	0x83, 0x38, 0x15, 0x01,
    //VSYNC_START
	0x83, 0x38, 0x16, 0x00,
	0x83, 0x38, 0x17, 0x00,
    //VSYNC_END
	0x83, 0x38, 0x18, 0x00,
	0x83, 0x38, 0x19, 0x00,
    //FORMAT1
	0x83, 0x38, 0x20, 0x80,
    //FORMAT2
	0x83, 0x38, 0x21, 0x40,
	0x83, 0x38, 0x26, 0x00,
	0x83, 0x38, 0x27, 0x08,
    // V_INC_ODD = (0x3814) = 1
    // V_INC_EVEN = (0x3815) = 1
	0x83, 0x38, 0x2a, 0x01,
	0x83, 0x38, 0x2b, 0x01,
	0x83, 0x38, 0x36, 0x02,
	0x83, 0x38, 0x38, 0x10,
	0x83, 0x38, 0x61, 0x00,
	0x83, 0x38, 0x62, 0x00,
	0x83, 0x38, 0x63, 0x02,
	0x83, 0x3b, 0x00, 0x00,
	0x83, 0x3c, 0x00, 0x89,
	0x83, 0x3c, 0x01, 0xab,
	0x83, 0x3c, 0x02, 0x01,
	0x83, 0x3c, 0x03, 0x00,
	0x83, 0x3c, 0x04, 0x00,
	0x83, 0x3c, 0x05, 0x03,
	0x83, 0x3c, 0x06, 0x00,
	0x83, 0x3c, 0x07, 0x05,
	0x83, 0x3c, 0x0c, 0x00,
	0x83, 0x3c, 0x0d, 0x00,
	0x83, 0x3c, 0x0e, 0x00,
	0x83, 0x3c, 0x0f, 0x00,
	0x83, 0x3c, 0x40, 0x00,
	0x83, 0x3c, 0x41, 0xa3,
	0x83, 0x3c, 0x43, 0x7d,
	0x83, 0x3c, 0x56, 0x80,
	0x83, 0x3c, 0x80, 0x08,
	0x83, 0x3c, 0x82, 0x01,
	0x83, 0x3c, 0x83, 0x61,
	0x83, 0x3d, 0x85, 0x17,
	0x83, 0x3f, 0x08, 0x08,
	0x83, 0x3f, 0x0a, 0x00,
	0x83, 0x3f, 0x0b, 0x30,
	0x83, 0x40, 0x00, 0xcd,
	0x83, 0x40, 0x03, 0x40,
	0x83, 0x40, 0x09, 0x0d,
	0x83, 0x40, 0x10, 0xf0,
	0x83, 0x40, 0x11, 0x70,
	0x83, 0x40, 0x17, 0x10,
	0x83, 0x40, 0x40, 0x00,
	0x83, 0x40, 0x41, 0x00,
	0x83, 0x43, 0x03, 0x00,
	0x83, 0x43, 0x07, 0x30,
	0x83, 0x45, 0x00, 0x30,
	0x83, 0x45, 0x02, 0x40,
	0x83, 0x45, 0x03, 0x06,
	0x83, 0x45, 0x08, 0xaa,
	0x83, 0x45, 0x0b, 0x00,
	0x83, 0x45, 0x0c, 0x00,
	0x83, 0x46, 0x00, 0x00,
	0x83, 0x46, 0x01, 0x80,
	0x83, 0x47, 0x00, 0x04,
	0x83, 0x47, 0x04, 0x00,
	0x83, 0x47, 0x05, 0x04,
	0x83, 0x48, 0x37, 0x14,
	0x83, 0x48, 0x4a, 0x3f,
	0x83, 0x50, 0x00, 0x10,
	0x83, 0x50, 0x01, 0x01,
	0x83, 0x50, 0x02, 0x28,
	0x83, 0x50, 0x04, 0x0c,
	0x83, 0x50, 0x06, 0x0c,
	0x83, 0x50, 0x07, 0xe0,
	0x83, 0x50, 0x08, 0x01,
	0x83, 0x50, 0x09, 0xb0,
	0x83, 0x50, 0x2a, 0x18,
	0x83, 0x59, 0x01, 0x00,
	0x83, 0x5a, 0x01, 0x00,
	0x83, 0x5a, 0x03, 0x00,
	0x83, 0x5a, 0x04, 0x0c,
	0x83, 0x5a, 0x05, 0xe0,
	0x83, 0x5a, 0x06, 0x09,
	0x83, 0x5a, 0x07, 0xb0,
	0x83, 0x5a, 0x08, 0x06,
	0x83, 0x5e, 0x00, 0x00,
	0x83, 0x5e, 0x10, 0xfc,
	0x83, 0x01, 0x00, 0x01,
};

//------------------------------------------------------------------------------
bool bSEN_I2C_Read(uint16_t uwAddress, uint8_t *pValue)
{
	uint8_t *pAddr,pBuf[2];
	
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
        printf("wr 0x%x fail \r\n",uwAddress); 
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
    tAE_SensorPara.ulSensorPclk = 48000000;  
    tAE_SensorPara.ulSensorPixelPerLine = 1620;
    tAE_SensorPara.ulSensorLinePerFrame = 988;
	tAE_SensorPara.ulSensorFrameRate = ubPclkIdx;     
    tAE_SensorPara.ulMaximumSensorFrameRate = 30;    
    //auto calculat Max Exposure
	ulPCK = tAE_SensorPara.ulSensorPclk;
	uwPPL = (unsigned short)(tAE_SensorPara.ulSensorPixelPerLine);
    
    if ((ubPclkIdx == 0) || (ubPclkIdx > 30))
    {
        xtSENInst.uwMaxExpLine = (ulPCK / 30 / (unsigned int)uwPPL);
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
    SENSOR_RESET_OUT = 1;
    TIMER_Delay_ms(30);
}

//------------------------------------------------------------------------------
uint8_t ubSEN_CheckSensorState(void)
{
	uint8_t     *pBuf;	
	uint16_t 	uwPID = 0;
      
	pBuf = (uint8_t*)&uwPID;  
	// I2C by Read Sensor ID
	bSEN_I2C_Read (OV9750_CHIP_ID_HIGH_ADDR, &pBuf[1]);
	bSEN_I2C_Read (OV9750_CHIP_ID_LOW_ADDR, &pBuf[0]);
	if (OV9750_CHIP_ID != uwPID)
	{
		printd(DBG_ErrorLvl, "Read chip ID fail.<0x%x 0x%x>\n", OV9750_CHIP_ID, uwPID);
        return 0;
	}
    return 1;
}

//------------------------------------------------------------------------------
uint8_t ubSEN_SetSensorInitTable(void)
{
    I2C_SCL_SPEED_TYP tI2C_Clock = I2C_SCL_300K;
	uint32_t 	i;	
	
	pI2C_type = pI2C_MasterInit (I2C_2, tI2C_Clock);
	IQ_SetI2cType(pI2C_type, tI2C_Clock);
    //HW reset.
    SEN_SensorHwReset();
    
    if(ubSEN_CheckSensorState() != 1)
        return 0;
    printd(DBG_CriticalLvl, "Ov9750 Sensor\n");

    //software reset
    bSEN_I2C_Write(0x01, 0x03, 0x01);        
	for (i=0; i<sizeof(ubSEN_InitTable); i+=4)
	{
		if (ubSEN_InitTable[i] == 0x83)	// write
		{
			bSEN_I2C_Write(ubSEN_InitTable[i+1], ubSEN_InitTable[i+2], ubSEN_InitTable[i+3]);
		}
	}
	return 1;
}

//------------------------------------------------------------------------------
uint8_t ubSEN_Open(struct SENSOR_SETTING *setting)
{		
    SEN_AeCbFunc();
    // Set ISP clock
    SEN_SetISPRate(6);
    // Set parallel mode
    SEN->MIPI_MODE = 0;
    // Set sensor clock
    SEN_SetSensorRate(SENSOR_96MHz, 8);	
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
    // change SN_SEN->RAW_REORDER to 0, start from B
    SEN->RAW_REORDER = 0;
    // set dummy line & pixel
    SEN->DMY_DIV = 3; // dummy clock=sysclk/2
    SEN->NUM_DMY_LN = 24;
    SEN->NUM_DMY_DSTB = 256;
    SEN->DMY_INSERT = 1;

    //delay 1ms
    TIMER_Delay_us(1000);		
    // Initial dummy sensor
    if (ubSEN_SetSensorInitTable() != 1)
    {
        printf("Ov9750 startup failed! \n\r");
        return 0;
    }
    SEN_PclkSetting(SEN_FPS30);
    // clear all debug flag
    SEN->REG_0x1300 = 0x1ff;
    SEN->REG_0x1304 = 0x3;
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

	if(xtSENInst.xtSENCtl.uwExpLine < 2)
	{
		xtSENInst.xtSENCtl.uwExpLine = 2;
	}	

	//Calculate Dummy line
	if(xtSENInst.xtSENCtl.uwExpLine > (xtSENInst.uwMaxExpLine - 4))
	{
		xtSENInst.xtSENCtl.uwDmyLine = (xtSENInst.xtSENCtl.uwExpLine + 4);
	}
	else
	{
		xtSENInst.xtSENCtl.uwDmyLine = xtSENInst.uwMaxExpLine;
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
	//Set dummy lines
    bSEN_I2C_WriteTry(OV9750_VTS_H, (uint8_t)((uwDL>>8) &0x00ff), TRY_COUNTS);
    bSEN_I2C_WriteTry(OV9750_VTS_L, (uint8_t)(uwDL &0x00ff), TRY_COUNTS);
}

//------------------------------------------------------------------------------
void SEN_SetExpLine(void)
{
    SEN_WrExpLine(xtSENInst.xtSENCtl.uwExpLine);
}

//------------------------------------------------------------------------------
void SEN_WrExpLine(uint16_t uwExpLine)
{
    //Set long exposure
    bSEN_I2C_WriteTry(OV9750_EXP_H, (uint8_t)((uwExpLine>>12)&0x000f), TRY_COUNTS);
    bSEN_I2C_WriteTry(OV9750_EXP_M, (uint8_t)((uwExpLine>>4)&0x00ff), TRY_COUNTS);
    bSEN_I2C_WriteTry(OV9750_EXP_L, (uint8_t)((uwExpLine<<4)&0x00ff), TRY_COUNTS);
}

//------------------------------------------------------------------------------
void SEN_WrMaxExpLine(uint32_t ulValue)
{
    xtSENInst.uwMaxExpLine = ulValue;
}

//------------------------------------------------------------------------------
void SEN_WrGain(uint32_t ulGainX1024)
{
	uint16_t uwAGaintmp;
	uint16_t uwDGaintmp;
		
	//avoid warning
	uwDGaintmp = uwDGaintmp;
	uwAGaintmp = uwAGaintmp;
    
    // update struct value.
    xtSENInst.xtSENCtl.uwGain = ulGainX1024;
	//	Min globe gain is 1x gain
	if (ulGainX1024 < 1024)			//Limit min value
	{
		ulGainX1024 = 1024;
	}		
		
    if(ulGainX1024 < 2048)							            //analog 1x~2x
    {
        uwAGaintmp = 0x80 +((ulGainX1024-1024)>>3);
        uwDGaintmp = 0x400;
    }
    else if(ulGainX1024 < 4096)					                //analog 2x~4x
    {		
        uwAGaintmp = 0x100 + ((ulGainX1024-2048)>>3);
        uwDGaintmp = 0x400;
    }
    else if(ulGainX1024 < 8192)					                //analog 4x~8x
    {
        uwAGaintmp = 0x200 + ((ulGainX1024-4096)>>3);
        uwDGaintmp = 0x400;
    }
    else if(ulGainX1024 < 15873)					            //analog 8x~15.5x
    {
        uwAGaintmp = 0x400+((ulGainX1024-8192)>>6);
        uwDGaintmp = 0x400;
    }
    else if(ulGainX1024 < 16384)					            //analog 15.5x~16x
    {
        uwAGaintmp = 0x7C0;
        uwDGaintmp = 0x400+((ulGainX1024-15873)/15);
    }
    else if(ulGainX1024 < 32768)					            //digital*analog 16x~32x
    {			 
        uwAGaintmp = 0x7C0;
        uwDGaintmp = 0x423+(((ulGainX1024-16386)*1024)/15953);
        
    }		
    else if(ulGainX1024 < 65536)
    {														    //digital*analog 32x~64x
        uwAGaintmp = 0x7c0;
        uwDGaintmp = 0x842+(((ulGainX1024-32768)*1024)/16936);
        							
    }
	// long gain
    bSEN_I2C_WriteTry(OV9750_GAIN_H, ((uwAGaintmp>>8) & 0xff), TRY_COUNTS);
    bSEN_I2C_WriteTry(OV9750_GAIN_L, (uwAGaintmp & 0xff), TRY_COUNTS);
	// R gain
    bSEN_I2C_WriteTry(OV9750_RGAIN_H, ((uwDGaintmp>>8) &0x000f), TRY_COUNTS);
    bSEN_I2C_WriteTry(OV9750_RGAIN_L, (uwDGaintmp &0x00ff), TRY_COUNTS);
    // G gain
    bSEN_I2C_WriteTry(OV9750_GGAIN_H, ((uwDGaintmp>>8) &0x000f), TRY_COUNTS);
    bSEN_I2C_WriteTry(OV9750_GGAIN_L, (uwDGaintmp &0x00ff), TRY_COUNTS);
    // B gain
    bSEN_I2C_WriteTry(OV9750_BGAIN_H, ((uwDGaintmp>>8) &0x000f), TRY_COUNTS);
    bSEN_I2C_WriteTry(OV9750_BGAIN_L, (uwDGaintmp &0x00ff), TRY_COUNTS);
}

//------------------------------------------------------------------------------
void SEN_SetMirrorFlip(uint8_t ubMirrorEn, uint8_t ubFlipEn)
{
	uint8_t ubBuf1, ubBuf2, n;

    if(!ubIQ_GetLoadIqFinishFg())
        return;
    
	bSEN_I2C_Read (OV9750_FMT_1, &ubBuf1);
    ubBuf1 = ubBuf1 & ~(OV9750_FLIP);
    bSEN_I2C_Read (OV9750_FMT_2, &ubBuf2);
    ubBuf2 = ubBuf2 & ~(OV9750_MIRROR);
    
    if(ubMirrorEn && ubFlipEn)
    {
        ubBuf1 = ubBuf1 | OV9750_FLIP;
        ubBuf2 = ubBuf2 | OV9750_MIRROR;
        n = 0x20;
    }else if(ubMirrorEn){
        ubBuf2 = ubBuf2 | OV9750_MIRROR;
        n = 0x0;
    }else if(ubFlipEn){
        ubBuf1 = ubBuf1 | OV9750_FLIP;
        n = 0x20;
    }else{
        n = 0x0;
    }

    bSEN_I2C_WriteTry(OV9750_FMT_1, ubBuf1, TRY_COUNTS);
    bSEN_I2C_WriteTry(OV9750_FMT_2, ubBuf2, TRY_COUNTS);
    n=n;
    //bSEN_I2C_WriteTry(0x450b, n, TRY_COUNTS);

    SEN_SetRawReorder(ubMirrorEn, ubFlipEn);
}

//------------------------------------------------------------------------------
void SEN_GroupHoldOnVSync(void)
{

}

//------------------------------------------------------------------------------
void SEN_GroupHoldOffVSync(void)
{

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
    sensor_cfg.ubSensorType = SEN_OV9750;	
    printf("sensor type is OV9750\r\n");
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
#endif
