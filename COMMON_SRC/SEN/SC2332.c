/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		SC2332.c
	\brief		Sensor SC2332 relation function
	\author		BoCun
	\version	1.0
	\date		2020-11-25
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

#if (SEN_USE == SEN_SC2332)
#define TRY_COUNTS 3
AE_EVENT_PROCESS tAE_SensorPara;
struct SENSOR_SETTING sensor_cfg;
tfSENObj xtSENInst;
I2C1_Type *pI2C_type;
SEN_STATE sensor_state;

const uint8_t ubSEN_InitTable[] = { 
#if 0    
    0x83, 0x01, 0x03, 0x01,
    0x83, 0x01, 0x00, 0x00,
    0x83, 0x36, 0xe9, 0x80,
    0x83, 0x36, 0xf9, 0x80,
    0x83, 0x30, 0x01, 0xff,
    0x83, 0x30, 0x02, 0xf0,
    0x83, 0x30, 0x0a, 0x24,
    0x83, 0x30, 0x18, 0x6f,
    0x83, 0x30, 0x1a, 0xf8,
    0x83, 0x30, 0x1c, 0x94,
    0x83, 0x30, 0x3f, 0x81,
    0x83, 0x32, 0x08, 0x07, 
    0x83, 0x32, 0x09, 0x80,             // window horizontal = 0x780 = 1920
    0x83, 0x32, 0x0a, 0x04,
    0x83, 0x32, 0x0b, 0x38,             // window vertical = 0x438 = 1080
    0x83, 0x32, 0x0e, 0x04,
    0x83, 0x32, 0x0f, 0x65,             // frame length = 0x465 = 1125
    0x83, 0x32, 0x14, 0x11,
    0x83, 0x32, 0x15, 0x11,
    0x83, 0x32, 0x53, 0x0c,
    0x83, 0x32, 0x74, 0x09,
    0x83, 0x33, 0x01, 0x05,
    0x83, 0x33, 0x04, 0x68,
    0x83, 0x33, 0x06, 0x40,
    0x83, 0x33, 0x0b, 0xc6,
    0x83, 0x33, 0x0e, 0x38,
    0x83, 0x33, 0x1c, 0x01,
    0x83, 0x33, 0x1e, 0x61,
    0x83, 0x33, 0x33, 0x10,
    0x83, 0x33, 0x64, 0x17,
    0x83, 0x33, 0x91, 0x18,
    0x83, 0x33, 0x92, 0x38,
    0x83, 0x33, 0x93, 0x0a,
    0x83, 0x33, 0x94, 0x10,
    0x83, 0x33, 0x95, 0x50,
    0x83, 0x36, 0x20, 0x88,
    0x83, 0x36, 0x22, 0x06,
    0x83, 0x36, 0x30, 0xd8,
    0x83, 0x36, 0x34, 0x44,
    0x83, 0x36, 0x37, 0x16,
    0x83, 0x36, 0x3a, 0x1f,
    0x83, 0x36, 0x41, 0x01,
    0x83, 0x36, 0x70, 0x1c,
    0x83, 0x36, 0x77, 0x84,
    0x83, 0x36, 0x78, 0x86,
    0x83, 0x36, 0x79, 0x8b,
    0x83, 0x36, 0x7e, 0x18,
    0x83, 0x36, 0x7f, 0x38,
    0x83, 0x36, 0x90, 0x53,
    0x83, 0x36, 0x91, 0x63,
    0x83, 0x36, 0x92, 0x63,
    0x83, 0x36, 0x9c, 0x08,
    0x83, 0x36, 0x9d, 0x38,
    0x83, 0x36, 0xa4, 0x08,
    0x83, 0x36, 0xa5, 0x18,
    0x83, 0x30, 0x00, 0x0f,
    0x83, 0x36, 0xa8, 0x08,
    0x83, 0x36, 0xa9, 0x28,
    0x83, 0x36, 0xaa, 0x2a,
    0x83, 0x36, 0xea, 0x1f,
    0x83, 0x36, 0xfc, 0x11,
    0x83, 0x36, 0xfd, 0x31,
    0x83, 0x39, 0x1d, 0x0c,
    0x83, 0x3e, 0x01, 0x8c,
    0x83, 0x3e, 0x03, 0x0b,
    0x83, 0x3e, 0x08, 0x03,
    0x83, 0x3e, 0x09, 0x20,
    0x83, 0x3e, 0x1b, 0x15,
    0x83, 0x3f, 0x03, 0x0d,
    0x83, 0x45, 0x09, 0x20,
    0x83, 0x46, 0x03, 0x09,
    0x83, 0x57, 0x8a, 0x30,
    0x83, 0x57, 0x8b, 0x30,
    0x83, 0x57, 0x93, 0x18,
    0x83, 0x57, 0x94, 0x10,
    0x83, 0x36, 0xe9, 0x59,
    0x83, 0x36, 0xf9, 0x21,
//    0x83, 0x01, 0x00, 0x01,

//    // output window height((1080 to 1088
//    0x83, 0x32, 0x0a, 0x04,		
//    0x83, 0x32, 0x0b, 0x40,
//    // row start position (4 to 6
//    0x83, 0x32, 0x12, 0x00,
//    0x83, 0x32, 0x13, 0x04,
#else
    0x83, 0x01, 0x03, 0x01,
    0x83, 0x01, 0x00, 0x00,
    0x83, 0x36, 0xe9, 0x80,
    0x83, 0x36, 0xf9, 0x80,
    0x83, 0x30, 0x01, 0xff,
    0x83, 0x30, 0x02, 0xf0,
    0x83, 0x30, 0x0a, 0x24,
    0x83, 0x30, 0x18, 0x6f,
    0x83, 0x30, 0x1a, 0xf8,
    0x83, 0x30, 0x1c, 0x94,
    0x83, 0x30, 0x3f, 0x81,
    0x83, 0x32, 0x00, 0x00,
    0x83, 0x32, 0x01, 0x00,
    0x83, 0x32, 0x02, 0x00,
    0x83, 0x32, 0x03, 0x00,
    0x83, 0x32, 0x04, 0x07,
    0x83, 0x32, 0x05, 0x8b,
    0x83, 0x32, 0x06, 0x04,
    0x83, 0x32, 0x07, 0x43,
    0x83, 0x32, 0x08, 0x07,
    0x83, 0x32, 0x09, 0x88,
    0x83, 0x32, 0x0a, 0x04,
    0x83, 0x32, 0x0b, 0x40,
    0x83, 0x32, 0x0e, 0x04,
    0x83, 0x32, 0x0f, 0x65,
    0x83, 0x32, 0x10, 0x00,
    0x83, 0x32, 0x11, 0x02,
    0x83, 0x32, 0x12, 0x00,
    0x83, 0x32, 0x13, 0x03,
    0x83, 0x32, 0x14, 0x11,
    0x83, 0x32, 0x15, 0x11,
    0x83, 0x32, 0x53, 0x0c,
    0x83, 0x32, 0x74, 0x09,
    0x83, 0x33, 0x01, 0x05,
    0x83, 0x33, 0x04, 0x68,
    0x83, 0x33, 0x06, 0x40,
    0x83, 0x33, 0x0b, 0xc6,
    0x83, 0x33, 0x0e, 0x38,
    0x83, 0x33, 0x1c, 0x01,
    0x83, 0x33, 0x1e, 0x61,
    0x83, 0x33, 0x33, 0x10,
    0x83, 0x33, 0x64, 0x17,
    0x83, 0x33, 0x91, 0x18,
    0x83, 0x33, 0x92, 0x38,
    0x83, 0x33, 0x93, 0x0a,
    0x83, 0x33, 0x94, 0x10,
    0x83, 0x33, 0x95, 0x50,
    0x83, 0x36, 0x20, 0x88,
    0x83, 0x36, 0x22, 0x06,
    0x83, 0x30, 0x00, 0x0f,
    0x83, 0x36, 0x30, 0xd8,
    0x83, 0x36, 0x34, 0x44,
    0x83, 0x36, 0x37, 0x16,
    0x83, 0x36, 0x3a, 0x1f,
    0x83, 0x36, 0x41, 0x01,
    0x83, 0x36, 0x70, 0x1c,
    0x83, 0x36, 0x77, 0x84,
    0x83, 0x36, 0x78, 0x86,
    0x83, 0x36, 0x79, 0x8b,
    0x83, 0x36, 0x7e, 0x18,
    0x83, 0x36, 0x7f, 0x38,
    0x83, 0x36, 0x90, 0x53,
    0x83, 0x36, 0x91, 0x63,
    0x83, 0x36, 0x92, 0x63,
    0x83, 0x36, 0x9c, 0x08,
    0x83, 0x36, 0x9d, 0x38,
    0x83, 0x36, 0xa4, 0x08,
    0x83, 0x36, 0xa5, 0x18,
    0x83, 0x36, 0xa8, 0x08,
    0x83, 0x36, 0xa9, 0x28,
    0x83, 0x36, 0xaa, 0x2a,
    0x83, 0x36, 0xea, 0x1f,
    0x83, 0x36, 0xfc, 0x11,
    0x83, 0x36, 0xfd, 0x31,
    0x83, 0x39, 0x1d, 0x0c,
    0x83, 0x3e, 0x01, 0x8c,
    0x83, 0x3e, 0x03, 0x0b,
    0x83, 0x3e, 0x08, 0x03,
    0x83, 0x3e, 0x09, 0x20,
    0x83, 0x3e, 0x1b, 0x15,
    0x83, 0x3f, 0x03, 0x22,
    0x83, 0x45, 0x09, 0x20,
    0x83, 0x46, 0x03, 0x09,
    0x83, 0x57, 0x8a, 0x30,
    0x83, 0x57, 0x8b, 0x30,
    0x83, 0x57, 0x93, 0x18,
    0x83, 0x57, 0x94, 0x10,
    0x83, 0x36, 0xe9, 0x59,
    0x83, 0x36, 0xf9, 0x21,
    //column start point
    0x83, 0x32, 0x10, 0x00,
    0x83, 0x32, 0x11, 0x04,     //02->04
    
//    0x83, 0x01, 0x00, 0x01,
#endif
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
    // support 1-30 fps for 1920x1080
    if(ubPclkIdx > 30)
    {
        ubPclkIdx = 30;
    }    
    // set sensor struct value
    tAE_SensorPara.ulSensorPclk = 74250000;  
    tAE_SensorPara.ulSensorPixelPerLine = 2200;
    tAE_SensorPara.ulSensorLinePerFrame = 1125;
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
	bSEN_I2C_Read (SC2332_CHIP_ID_HIGH_ADDR, &pBuf[1]);
	bSEN_I2C_Read (SC2332_CHIP_ID_LOW_ADDR, &pBuf[0]);
	if (SC2332_CHIP_ID != uwPID)
	{
		printd(DBG_ErrorLvl, "I2C read fail.<0x%x 0x%x>\n", SC2332_CHIP_ID, uwPID);
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
 	printf("SC2332 \r\n");
    // initial table
	for (i=0; i<sizeof(ubSEN_InitTable); i+=4)
	{
		if (ubSEN_InitTable[i] == 0x83)	// write
		{
			bSEN_I2C_Write(ubSEN_InitTable[i+1], ubSEN_InitTable[i+2], ubSEN_InitTable[i+3]);
		}
	}
    TIMER_Delay_ms(50);
	// start streaming
	bSEN_I2C_Write(0x01, 0x00, 0x01);
	return 1;
}

//------------------------------------------------------------------------------
uint8_t ubSEN_Open(struct SENSOR_SETTING *setting)
{
    SEN_AeCbFunc();
    // Set ISP clock
    SEN_SetISPRate(5);		
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
    SEN->VSYNC_RIS = 0;
    SEN->HSYNC_RIS = 1;
    SEN->VSYNC_HIGH = 0;
    SEN->PCK_DLH_RIS = 1;
    SEN->SYNC_MODE = 1;
    SEN->SENSOR_MODE = 0;    
    // change SN_SEN->RAW_REORDER to 0, start from B
    SEN->RAW_REORDER = 1;
    // set dummy line & pixel
    SEN->DMY_DIV = 3;
    SEN->NUM_DMY_LN = 24;
    SEN->NUM_DMY_DSTB = 256;
    SEN->DMY_INSERT = 1;
	
    // Initial dummy sensor
    if (ubSEN_SetSensorInitTable() != 1)
    {
        printf("SC2332 startup failed! \n\r");
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
    
	if(xtSENInst.xtSENCtl.uwExpLine < 1)
	{
		xtSENInst.xtSENCtl.uwExpLine = 1;
	}	

	//Calculate Dummy line   
	if(xtSENInst.xtSENCtl.uwExpLine > (xtSENInst.uwMaxExpLine - 4))
	{
		xtSENInst.xtSENCtl.uwDmyLine = (xtSENInst.xtSENCtl.uwExpLine + 4);
	}else{
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
	bSEN_I2C_WriteTry(SC2332_FRAME_LENGTH_H, (uint8_t)((uwDL>>8) &0x00ff), TRY_COUNTS);
	bSEN_I2C_WriteTry(SC2332_FRAME_LENGTH_L, (uint8_t)(uwDL &0x00ff), TRY_COUNTS);     
}

//------------------------------------------------------------------------------
void SEN_SetExpLine(void)
{
    SEN_WrExpLine(xtSENInst.xtSENCtl.uwExpLine);
}

//------------------------------------------------------------------------------
void SEN_WrExpLine(uint16_t uwExpLine)
{    
    static uint32_t ulOldExpLine = 0;
    if(uwExpLine == ulOldExpLine)
        return;
    
    // set exposure
    bSEN_I2C_WriteTry(SC2332_EXPH, (uint8_t)((uwExpLine>>12)&0x001f), TRY_COUNTS);
    bSEN_I2C_WriteTry(SC2332_EXPM, (uint8_t)((uwExpLine>>4)&0x00ff), TRY_COUNTS);
    bSEN_I2C_WriteTry(SC2332_EXPL, (uint8_t)((uwExpLine&0x0f) << 4), TRY_COUNTS);
    
    ulOldExpLine = uwExpLine;
}

//------------------------------------------------------------------------------
void SEN_WrMaxExpLine(uint32_t ulValue)
{
    xtSENInst.uwMaxExpLine = ulValue;
}

//------------------------------------------------------------------------------
void SEN_WrGain(uint32_t ulGainX1024)
{
    static uint32_t ulOldGainValue = 0;
    uint16_t uwGain_H = 0;
    uint8_t ubAGain_3e08 = 0x03;        // Again
    uint8_t ubAGainF_3e09 = 0x10;       // Again(FINE)
    uint8_t ubDGain_3e06 = 0x00;        // Dgain
    uint8_t ubDGainF_3e07 = 0x80;       // Dgain(FINE)
    uint8_t ubReg_0x3974 = 0;
    uint8_t ubReg_0x3e06 = 0;
    // update struct value.
    xtSENInst.xtSENCtl.uwGain = ulGainX1024;
    ubReg_0x3e06 = ubReg_0x3e06;
    //	Min globe gain is 1x gain
	if (ulGainX1024 < 1024)			//Limit min value
	{
		ulGainX1024 = 1024;
	}else if(ulGainX1024 > 63488){
        ulGainX1024 = 63488;
    }
    if(ulGainX1024 == ulOldGainValue)
        return;

    if(ulGainX1024 < 2048)              // x1~x2
    {
        uwGain_H = 1;
        ubAGain_3e08 = 0x03;
        ubAGainF_3e09 = (((ulGainX1024 - (uwGain_H<<10))/(uwGain_H<<5)) + 0x20);
        ubDGain_3e06 = 0x00;
        ubDGainF_3e07 = 0x80;
    }else if(ulGainX1024 < 4096){       // x2~x4
        uwGain_H = 2;
        ubAGain_3e08 = 0x07;
        ubAGainF_3e09 = (((ulGainX1024 - (uwGain_H<<10))/(uwGain_H<<5)) + 0x20);
        ubDGain_3e06 = 0x00;
        ubDGainF_3e07 = 0x80;
    }else if(ulGainX1024 < 8192){       // x4~x8
        uwGain_H = 4;
        ubAGain_3e08 = 0x0F;
        ubAGainF_3e09 = (((ulGainX1024 - (uwGain_H<<10))/(uwGain_H<<5)) + 0x20);
        ubDGain_3e06 = 0x00;
        ubDGainF_3e07 = 0x80;
    }else if(ulGainX1024 < 16384){      // x8~x16
        uwGain_H = 8;
        ubAGain_3e08 = 0x1F;
        ubAGainF_3e09 = (((ulGainX1024 - (uwGain_H<<10))/(uwGain_H<<5)) + 0x20);
        ubDGain_3e06 = 0x00;
        ubDGainF_3e07 = 0x80;
    }else if(ulGainX1024 < 32768){      // x16~x32
        uwGain_H = 16;
        ubAGain_3e08 = 0x1F;
        ubAGainF_3e09 = 0x3F;
        ubDGain_3e06 = 0x00;
        ubDGainF_3e07 = (((ulGainX1024 - (uwGain_H<<10))/(uwGain_H<<3)) + 0x80);
    }else if(ulGainX1024 < 65536){      // x32~x64
        uwGain_H = 32;
        ubAGain_3e08 = 0x1F;
        ubAGainF_3e09 = 0x7F;
        ubDGain_3e06 = 0x01;
        ubDGainF_3e07 = (((ulGainX1024 - (uwGain_H<<10))/(uwGain_H<<3)) + 0x80);
    }

    bSEN_I2C_Read(0x3974 ,&ubReg_0x3974);
    //group latch
    bSEN_I2C_WriteTry(0x3812, 0x00, TRY_COUNTS);
    if((ubReg_0x3974>=0x10) || (ulGainX1024 > 40*1024))
    {
        bSEN_I2C_Write(0x57, 0x99, 0x07); 
    }else if((ubReg_0x3974<=0x0e) && (ulGainX1024 <= 30*1024)){
        bSEN_I2C_Write(0x57, 0x99, 0x00);
    }
    if(ubReg_0x3974 >= 0x15)
    {
        bSEN_I2C_Write(0x36, 0x37, 0x2c);
        ubReg_0x3e06 = ubDGain_3e06 | 0x04;        //i2c_write(0x3e06[3], 1)
    }else if(ubReg_0x3974 <= 0x0c){
        bSEN_I2C_Write(0x36, 0x37, 0x16);
        ubReg_0x3e06 = ubDGain_3e06 | 0x00;        //i2c_write(0x3e06[3], 1)
    }
    bSEN_I2C_WriteTry(SC2332_AGAIN, ubAGain_3e08, TRY_COUNTS);
    bSEN_I2C_WriteTry(SC2332_AGAIN_F, ubAGainF_3e09, TRY_COUNTS);
    bSEN_I2C_WriteTry(SC2332_DGAIN, ubDGain_3e06, TRY_COUNTS);
    bSEN_I2C_WriteTry(SC2332_DGAIN_F, ubDGainF_3e07, TRY_COUNTS);
    bSEN_I2C_WriteTry(0x3812, 0x30, TRY_COUNTS);
    // save gain value    
    ulOldGainValue = ulGainX1024;
}

//------------------------------------------------------------------------------
void SEN_SetMirrorFlip(uint8_t ubMirrorEn, uint8_t ubFlipEn)
{
    if(!ubIQ_GetLoadIqFinishFg())
        return;
    
    SEN_SetDetectVideoState(0);
    if(ubMirrorEn)
        xtSENInst.ubImgMode |=  SC2332_MIRROR;
    else
        xtSENInst.ubImgMode &=  ~SC2332_MIRROR;
    
    if(ubFlipEn)
        xtSENInst.ubImgMode |=  SC2332_FLIP;
    else
        xtSENInst.ubImgMode &=  ~SC2332_FLIP;
    
    bSEN_I2C_Write(0x32, 0x21, xtSENInst.ubImgMode);
    SEN_SetRawReorder(ubMirrorEn, ubFlipEn);
    TIMER_Delay_ms(33);
    SEN_SetDetectVideoState(1);
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
    sensor_cfg.ubSensorType = SEN_SC2332;	
    printf("sensor type is SC2332\r\n");
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
    uint8_t ubDGain_3e06;   
    uint8_t ubDGainF_3e07;  
    uint8_t ubAGain_3e08;   
    uint8_t ubAGainF_3e09;
    uint8_t ubReg_0x3974 = 0;
    uint8_t ubReg_0x3e06 = 0;
    uint16_t uwExpLine;
    uint16_t uwGain_H;
    uint32_t ulGainX1024;
    
    uwExpLine = ulExp;
    ulGainX1024 = ulGain;
    ubReg_0x3e06 = ubReg_0x3e06;
    
    //EXPLINE
    bSEN_I2C_WriteTry(SC2332_EXPH, (uint8_t)((uwExpLine>>12)&0x001f), TRY_COUNTS);
    bSEN_I2C_WriteTry(SC2332_EXPM, (uint8_t)((uwExpLine>>4)&0x00ff), TRY_COUNTS);
    bSEN_I2C_WriteTry(SC2332_EXPL, (uint8_t)((uwExpLine&0x0f) << 4), TRY_COUNTS);
   
    //GAIN
    if(ulGainX1024 < 2048)              // x1~x2
    {
        uwGain_H = 1;
        ubAGain_3e08 = 0x03;
        ubAGainF_3e09 = (((ulGainX1024 - (uwGain_H<<10))/(uwGain_H<<5)) + 0x20);
        ubDGain_3e06 = 0x00;
        ubDGainF_3e07 = 0x80;
    }else if(ulGainX1024 < 4096){       // x2~x4
        uwGain_H = 2;
        ubAGain_3e08 = 0x07;
        ubAGainF_3e09 = (((ulGainX1024 - (uwGain_H<<10))/(uwGain_H<<5)) + 0x20);
        ubDGain_3e06 = 0x00;
        ubDGainF_3e07 = 0x80;
    }else if(ulGainX1024 < 8192){       // x4~x8
        uwGain_H = 4;
        ubAGain_3e08 = 0x0F;
        ubAGainF_3e09 = (((ulGainX1024 - (uwGain_H<<10))/(uwGain_H<<5)) + 0x20);
        ubDGain_3e06 = 0x00;
        ubDGainF_3e07 = 0x80;
    }else if(ulGainX1024 < 16384){      // x8~x16
        uwGain_H = 8;
        ubAGain_3e08 = 0x1F;
        ubAGainF_3e09 = (((ulGainX1024 - (uwGain_H<<10))/(uwGain_H<<5)) + 0x20);
        ubDGain_3e06 = 0x00;
        ubDGainF_3e07 = 0x80;
    }else if(ulGainX1024 < 32768){      // x16~x32
        uwGain_H = 16;
        ubAGain_3e08 = 0x1F;
        ubAGainF_3e09 = 0x3F;
        ubDGain_3e06 = 0x00;
        ubDGainF_3e07 = (((ulGainX1024 - (uwGain_H<<10))/(uwGain_H<<3)) + 0x80);
    }else if(ulGainX1024 < 65536){      // x32~x64
        uwGain_H = 32;
        ubAGain_3e08 = 0x1F;
        ubAGainF_3e09 = 0x7F;
        ubDGain_3e06 = 0x01;
        ubDGainF_3e07 = (((ulGainX1024 - (uwGain_H<<10))/(uwGain_H<<3)) + 0x80);
    }
    bSEN_I2C_Read(0x3974 ,&ubReg_0x3974);
    //group latch
    bSEN_I2C_WriteTry(0x3812, 0x00, TRY_COUNTS);
    if((ubReg_0x3974>=0x10) || (ulGainX1024 > 40*1024))
    {
        bSEN_I2C_Write(0x57, 0x99, 0x07); 
    }else if((ubReg_0x3974<=0x0e) && (ulGainX1024 <= 30*1024)){
        bSEN_I2C_Write(0x57, 0x99, 0x00);
    }
    if(ubReg_0x3974 >= 0x15)
    {
        bSEN_I2C_Write(0x36, 0x37, 0x2c);
        ubReg_0x3e06 = ubDGain_3e06 | 0x04;        //i2c_write(0x3e06[3], 1)
    }else if(ubReg_0x3974 <= 0x0c){
        bSEN_I2C_Write(0x36, 0x37, 0x16);
        ubReg_0x3e06 = ubDGain_3e06 | 0x00;        //i2c_write(0x3e06[3], 1)
    }
    
    bSEN_I2C_WriteTry(SC2332_AGAIN, ubAGain_3e08, TRY_COUNTS);
    bSEN_I2C_WriteTry(SC2332_AGAIN_F, ubAGainF_3e09, TRY_COUNTS);
    bSEN_I2C_WriteTry(SC2332_DGAIN, ubDGain_3e06, TRY_COUNTS);
    bSEN_I2C_WriteTry(SC2332_DGAIN_F, ubDGainF_3e07, TRY_COUNTS);
    bSEN_I2C_WriteTry(0x3812, 0x30, TRY_COUNTS);
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
