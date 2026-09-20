/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		IMX323.c
	\brief		Sensor IMX323 relation function
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

#if (SEN_USE == SEN_IMX323)
#define TRY_COUNTS 3
AE_EVENT_PROCESS tAE_SensorPara;
struct SENSOR_SETTING sensor_cfg;
tfSENObj xtSENInst;
I2C1_Type *pI2C_type;
SEN_STATE sensor_state;

#if (ISP_RES == ISP_HD)
//------------------------------
// Initial Table
//------------------------------
// IMX323_720p@60fps
// MCLK = 37.125MHz
// FRM_LENGTH = 750, LINE_LENGTH = 1650
// PCLK = LINE_LENGTH * FRM_LENGTH * fps = 74250000
// If input clock = 40MHz, then fps = 64.646Hz
// PCLK = LINE_LENGTH * FRM_LENGTH * fps = 80000000
//
// Set fps = 60Hz
// 80MHz = LINE_LENGTH * 750 * 60 ==> LINE_LENGTH = 1776
#define PIX_H (1776)
#define PIX_H2 (PIX_H/2)
#define PIX_V (750)

const uint8_t ubSEN_InitTable[] = {
    0x83, 0x30, 0x00, 0x31, // Standby
    0x83, 0x01, 0x00, 0x00, // Mode_Sel Standby
    0x83, 0x30, 0x2C, 0x01, // Master mode standby

    0x83, 0x00, 0x08, 0x00,
    0x83, 0x00, 0x09, 0x00,
    0x83, 0x01, 0x01, 0x00,
    0x83, 0x01, 0x04, 0x00,
    0x83, 0x01, 0x12, 0x0A,                 //I2C ADRES1
    0x83, 0x01, 0x13, 0x0A,                 //I2C ADRES2
    0x83, 0x02, 0x02, 0x00,
    0x83, 0x02, 0x03, 0x00,


    0x83, 0x03, 0x40, (PIX_V>>8),
    0x83, 0x03, 0x41, (PIX_V&0xff),         //FRM_LENGTH
    0x83, 0x03, 0x42, (PIX_H2>>8),
    0x83, 0x03, 0x43, (PIX_H2&0xff),        //LINE_LENGTH
    0x83, 0x30, 0x01, 0x00,
    0x83, 0x30, 0x02, 0x01,                 //MODE	
    0x83, 0x30, 0x03, (PIX_H2&0xff),
    0x83, 0x30, 0x04, (PIX_H2>>8),          //HMAX
    0x83, 0x30, 0x05, (PIX_V&0xff),
    0x83, 0x30, 0x06, (PIX_V>>8),           //VMAX

    0x83, 0x30, 0x07, 0x00,
    0x83, 0x30, 0x11, 0x00,
    0x83, 0x30, 0x12, 0x80,
    0x83, 0x30, 0x16, 0xF0,                 //WINPV
    0x83, 0x30, 0x1F, 0x73,
    0x83, 0x30, 0x20, 0x3C,
    0x83, 0x30, 0x21, 0x00,                 //10BITA = 0x3021[7] = 0
    0x83, 0x30, 0x22, 0x81,                 //720PMODE = 0x3022[7] = 1
    
    0x83, 0x30, 0x4F, 0x47,
    0x83, 0x30, 0x54, 0x14, //0x10
    0x83, 0x31, 0x1E, 0x00,
    
    0x83, 0x30, 0x27, 0x20,
    0x83, 0x30, 0x7A, 0x00,                 //10BITB
    0x83, 0x30, 0x7B, 0x00,                 //10BITC
    0x83, 0x30, 0x98, 0x26,
    0x83, 0x30, 0x99, 0x02,                 //10B1080P
    0x83, 0x30, 0x9A, 0x4c,
    0x83, 0x30, 0x9B, 0x04,                 //12B1080P
    0x83, 0x30, 0xCE, 0x00,                 //PRES
    0x83, 0x30, 0xCF, 0x00,
    0x83, 0x30, 0xD0, 0x00,                 //DRES
    0x83, 0x30, 0x3f, 0x0a, // imx323 only 

    // address	data
    0x83, 0x31, 0x17, 0x0D,
    0xbb, 0x00, 0x64, 0x00, // delay 100ms
    0x83, 0x30, 0x2C, 0x00,	// Master mode operation start
    0xbb, 0x00, 0x64, 0x00, // delay 100ms
    0x83, 0x30, 0x00, 0x30, // Register write:Valid
    0x83, 0x01, 0x00, 0x01,
};

#elif (ISP_RES == ISP_FHD)
//------------------------------
// Initial Table
//------------------------------
// IMX323_1080p@30fps
// MCLK = 37.125MHz
// FRM_LENGTH = 1125, LINE_LENGTH = 2200
// PCLK = LINE_LENGTH * FRM_LENGTH * fps = 74250000
// If input clock = 40MHz, then fps = 32.323Hz
// PCLK = LINE_LENGTH * FRM_LENGTH * fps = 80000000
//
// Set fps = 30Hz
// 80MHz = LINE_LENGTH * 1125 * 30 ==> LINE_LENGTH = 2370
#define PIX_H 2300
#define PIX_H2 (PIX_H/2)
#define PIX_H4 (PIX_H/4)
#define PIX_V 1125

const uint8_t ubSEN_InitTable[] = {
    0x83, 0x30, 0x00, 0x31, // Standby
    0x83, 0x01, 0x00, 0x00, // Mode_Sel Standby
    0x83, 0x30, 0x2C, 0x01, // Master mode standby

    0x83, 0x00, 0x08, 0x00,
    0x83, 0x00, 0x09, 0x00,
    0x83, 0x01, 0x01, 0x00,
    0x83, 0x01, 0x04, 0x00,
    0x83, 0x01, 0x12, 0x0A,
    0x83, 0x01, 0x13, 0x0A,
    0x83, 0x02, 0x02, 0x00,
    0x83, 0x02, 0x03, 0x00,


    0x83, 0x03, 0x40, (PIX_V>>8),
    0x83, 0x03, 0x41, (PIX_V&0xff),
    0x83, 0x03, 0x42, (PIX_H2>>8),
    0x83, 0x03, 0x43, (PIX_H2&0xff),

    0x83, 0x30, 0x01, 0x00,
    0x83, 0x30, 0x02, 0x0F,	
    0x83, 0x30, 0x03, (PIX_H2&0xff),
    0x83, 0x30, 0x04, (PIX_H2>>8),
    0x83, 0x30, 0x05, (PIX_V&0xff),
    0x83, 0x30, 0x06, (PIX_V>>8),

    0x83, 0x30, 0x07, 0x00,
    0x83, 0x30, 0x11, 0x00,
    0x83, 0x30, 0x12, 0x80,
    0x83, 0x30, 0x16, 0x3C,
    0x83, 0x30, 0x1F, 0x73,
    0x83, 0x30, 0x20, 0x3C,
    0x83, 0x30, 0x21, 0x80,
    0x83, 0x30, 0x22, 0x01, //0x00
    
    0x83, 0x30, 0x4F, 0x47,
    0x83, 0x30, 0x54, 0x14, //0x10
    0x83, 0x31, 0x1E, 0x00,
    
    0x83, 0x30, 0x27, 0x20,
    0x83, 0x30, 0x7A, 0x40,
    0x83, 0x30, 0x7B, 0x02,
    0x83, 0x30, 0x98, (PIX_H4&0xff),
    0x83, 0x30, 0x99, (PIX_H4>>8),
    0x83, 0x30, 0x9A, 0x4c,
    0x83, 0x30, 0x9B, 0x04,
    0x83, 0x30, 0xCE, 0x16,
    0x83, 0x30, 0xCF, 0x82,
    0x83, 0x30, 0xD0, 0x00,
    // 17-09-08
    0x83, 0x30, 0x3f, 0x0a, // imx323 only 

    // address	data
    0x83, 0x31, 0x17, 0x0D,
    
    0xbb, 0x00, 0x64, 0x00, // delay 100ms
    0x83, 0x30, 0x2C, 0x00,	// Master mode operation start
    0xbb, 0x00, 0x64, 0x00, // delay 100ms
    0x83, 0x30, 0x00, 0x30, // Register write:Valid
    0x83, 0x01, 0x00, 0x01,
};
#endif

#define cbAE_SensorVTblColSZ		(sizeof(ctSensor_SensorVTbl[0]))
#define cbAE_SensorVTblRowSZ		(sizeof(ctSensor_SensorVTbl)/sizeof(ctSensor_SensorVTbl[0]))
    
struct Sen_GainTblObj{
	uint16_t    uwAlgGain;
	uint8_t     ubSenGain;
};

struct Sen_GainTblObj ctSensor_SensorVTbl[] = {
	{512 ,   0 },
	{529 ,   1 },
	{548 ,   2 },
	{567 ,   3 },
	{587 ,   4 },
	{608 ,   5 },
	{629 ,   6 },
	{652 ,   7 },
	{674 ,   8 },
	{698 ,   9 },
	{723 ,   10},
	{748 ,   11},
	{774 ,   12},
	{802 ,   13},
	{830 ,   14},
	{859 ,   15},
	{889 ,   16},
	{921 ,   17},
	{953 ,   18},
	{986 ,   19},
	{1021,   20},
	{1057,   21},
	{1094,   22},
	{1133,   23},
	{1172,   24},
	{1214,   25},
	{1256,   26},
	{1300,   27},
	{1346,   28},
	{1394,   29},
	{1443,   30},
	{1493,   31},
	{1546,   32},
	{1600,   33},
	{1656,   34},
	{1715,   35},
	{1775,   36},
	{1837,   37},
	{1902,   38},
	{1969,   39},
	{2038,   40},
	{2109,   41},
	{2184,   42},
	{2260,   43},
	{2340,   44},
	{2422,   45},
	{2507,   46},
	{2595,   47},
	{2687,   48},
	{2781,   49},
	{2879,   50},
	{2980,   51},
	{3085,   52},
	{3193,   53},
	{3305,   54},
	{3421,   55},
	{3542,   56},
	{3666,   57},
	{3795,   58},
	{3928,   59},
	{4066,   60},
	{4209,   61},
	{4357,   62},
	{4510,   63},
	{4669,   64},
	{4833,   65},
	{5003,   66},
	{5179,   67},
	{5361,   68},
	{5549,   69},
	{5744,   70},
	{5946,   71},
	{6155,   72},
	{6371,   73},
	{6595,   74},
	{6827,   75},
	{7067,   76},
	{7315,   77},
	{7573,   78},
	{7839,   79},
	{8114,   80},
	{8399,   81},
	{8695,   82},
	{9000,   83},
	{9316,   84},
	{9644,   85},
	{9983,   86},
	{10334,  87},
	{10697,  88},
	{11073,  89},
	{11462,  90},
	{11865,  91},
	{12282,  92},
	{12713,  93},
	{13160,  94},
	{13622,  95},
	{14101,  96},
	{14597,  97},
	{15110,  98},
	{15641,  99},
	{16190, 100},
	{16759, 101},
	{17348, 102},
	{17958, 103},
	{18589, 104},
	{19242, 105},
	{19919, 106},
	{20619, 107},
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
    }while((ret==0) && ((i++) < ubTryCnt));
    if(ret == 0)     
        printf("wr fail 0x%x.\r\n",uwAddress);    
    
	return ret;
}
//------------------------------------------------------------------------------
void SEN_PclkSetting(uint8_t ubPclkIdx)
{
    uint16_t uwPPL;
    uint32_t ulPCK;
    #if (ISP_RES == ISP_HD)
        // support 1-60 fps for 1280x720
        if(ubPclkIdx > 60)
        {
            ubPclkIdx = 60;
        }
        // set sensor struct value
        tAE_SensorPara.ulSensorPclk = 80000000;  
        tAE_SensorPara.ulSensorPixelPerLine = 1776;
        tAE_SensorPara.ulSensorLinePerFrame = 750;
        tAE_SensorPara.ulSensorFrameRate = ubPclkIdx;     
        tAE_SensorPara.ulMaximumSensorFrameRate = 60;
    #elif (ISP_RES == ISP_FHD)
        // support 1-30 fps for 1920x1080
        if(ubPclkIdx > 30)
        {
            ubPclkIdx = 30;
        }
        // set sensor struct value
        tAE_SensorPara.ulSensorPclk = 80000000;  
        tAE_SensorPara.ulSensorPixelPerLine = 2300;
        tAE_SensorPara.ulSensorLinePerFrame = 1125;
        tAE_SensorPara.ulSensorFrameRate = ubPclkIdx;     
        tAE_SensorPara.ulMaximumSensorFrameRate = 30;
    #endif
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
    SENSOR_RESET_OUT = 1;
    TIMER_Delay_ms(30);
}

//------------------------------------------------------------------------------
uint8_t ubSEN_CheckSensorState(void)
{
	uint8_t     *pBuf;	
	uint8_t 	ubPID = 0;
      
	pBuf = (uint8_t*)&ubPID; 
	// I2C by Read Sensor ID
    bSEN_I2C_Read (IMX323_CHIP_MODELID_ADDR, &pBuf[0]);
	if (IMX323_CHIP_MODELID != ubPID)
	{
		printd(DBG_ErrorLvl, "I2C read fail.<0x%x 0x%x>\n", IMX323_CHIP_MODELID, ubPID);
        return 0;
	}
    return 1;
}

//------------------------------------------------------------------------------
uint8_t ubSEN_SetSensorInitTable(void)
{
    I2C_SCL_SPEED_TYP tI2C_Clock = I2C_SCL_300K;
	uint32_t 	i;	
    
    // Init I2C2
	pI2C_type = pI2C_MasterInit (I2C_2, tI2C_Clock);
    IQ_SetI2cType(pI2C_type, tI2C_Clock);
    //HW reset.
    SEN_SensorHwReset();
    
    if(ubSEN_CheckSensorState() != 1)
        return 0;
    printd(DBG_CriticalLvl, "IMX323 Sensor\n");

	bSEN_I2C_Write(0x30, 0x00, 0x31);	// Standby
	bSEN_I2C_Write(0x01, 0x00, 0x00);	// Mode_Sel Standby
	bSEN_I2C_Write(0x30, 0x2C, 0x01);	// Master mode standby

	for (i=0; i<sizeof(ubSEN_InitTable); i+=4)
	{
		if (ubSEN_InitTable[i] == 0x83)	// write
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
    SEN_SetISPRate(5);		
    // Set parallel mode
    SEN->MIPI_MODE = 0;
    // Set sensor clock
    SEN_SetSensorRate(SENSOR_120MHz, 3);	
    // Enable sensor PCLK
    SEN->SEN_CLK_EN = 1;	
    //Set ISP pipeline
    sensor_cfg.tPathType = BAYER_SENSOR_NORMAL;
    SEN_SenosrOutputType(sensor_cfg.tPathType);
    // Change ISP_LH_SEL to 0, for 10bit raw input
    SEN->ISP_LH_SEL = 0;
    // Timing
    SEN->VSYNC_RIS = 0;
    SEN->HSYNC_RIS = 0;
    SEN->VSYNC_HIGH = 1;
    SEN->PCK_DLH_RIS = 1;
    SEN->SYNC_MODE = 1;
    SEN->SENSOR_MODE = 0;
    // change SN_SEN->RAW_REORDER to 0, start from B
    SEN->RAW_REORDER = 0;
    // Set dummy line & pixel
    #if (ISP_RES == ISP_HD)  
        SEN->DMY_DIV = 4;
        SEN->NUM_DMY_LN = 16;
        SEN->NUM_DMY_DSTB = 256;
        SEN->DMY_INSERT = 1;
    #elif (ISP_RES == ISP_FHD)
        SEN->DMY_DIV = 3;
        SEN->NUM_DMY_LN = 16;
        SEN->NUM_DMY_DSTB = 256;
        SEN->DMY_INSERT = 0;
    #endif
    // Initial dummy sensor
    if (ubSEN_SetSensorInitTable() != 1)
    {
        printf("IMX323 startup failed! \n\r");
        return 0;
    }
    SEN_PclkSetting(SEN_FPS30);
    // Clear all debug flag
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

	//Calculate Dummy line
    if(xtSENInst.xtSENCtl.uwExpLine > xtSENInst.uwMaxExpLine)
	{
        xtSENInst.xtSENCtl.uwDmyLine = xtSENInst.xtSENCtl.uwExpLine;
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
    //Set Dummy Line         
    bSEN_I2C_WriteTry(IMX322_FRAME_LENGTH_H, (uint8_t)((uwDL>>8) & 0x00ff), TRY_COUNTS);		
    bSEN_I2C_WriteTry(IMX322_FRAME_LENGTH_L, (uint8_t)((uwDL) & 0x00ff), TRY_COUNTS);	   
}

//------------------------------------------------------------------------------
void SEN_SetExpLine(void)
{
    SEN_WrExpLine(xtSENInst.xtSENCtl.uwExpLine);
}


//------------------------------------------------------------------------------
void SEN_WrExpLine(uint16_t uwExpLine)
{
	uint16_t uwTargetExpLine;
    static uint16_t uwOldExpLine = 0;
    
	if(uwExpLine <= xtSENInst.uwMaxExpLine)
	{
	   uwTargetExpLine = xtSENInst.uwMaxExpLine - uwExpLine;
	}else{
	   uwTargetExpLine = 0;
	}		
    
    if(uwTargetExpLine == uwOldExpLine)
        return;
    // set exposure
    bSEN_I2C_WriteTry(IMX322_DUMMY_LINE_H, (uint8_t)(((uwTargetExpLine)>>8)&0x00ff), TRY_COUNTS);	
    bSEN_I2C_WriteTry(IMX322_DUMMY_LINE_L, (uint8_t)((uwTargetExpLine)&0x00ff), TRY_COUNTS);     
    
    uwOldExpLine = uwTargetExpLine;
}

//------------------------------------------------------------------------------
void SEN_WrMaxExpLine(uint32_t ulValue)
{
    xtSENInst.uwMaxExpLine = ulValue;
}

//------------------------------------------------------------------------------
void SEN_WrGain(uint32_t ulGainX1024)
{
	uint8_t ubIdx;
    static uint32_t ulOldGainValue = 0;
    
    // update struct value.
    xtSENInst.xtSENCtl.uwGain = ulGainX1024;
	ulGainX1024 = ulGainX1024/2;
	if(ulGainX1024 < 512)
	{
		ulGainX1024 = 512;
	}

    if(ulGainX1024 == ulOldGainValue)
        return;

	for(ubIdx = 0; ubIdx<cbAE_SensorVTblRowSZ; ubIdx++)
	{
		if(ulGainX1024 <= ctSensor_SensorVTbl[ubIdx].uwAlgGain)
		{
			xtSENInst.ubBuf[2] = ctSensor_SensorVTbl[ubIdx].ubSenGain;
			break;
		}
	}

	if(ubIdx == cbAE_SensorVTblRowSZ)
	{
		xtSENInst.ubBuf[2] = ctSensor_SensorVTbl[ubIdx-1].ubSenGain;		
	}
    
    bSEN_I2C_WriteTry(IMX322_GAIN, xtSENInst.ubBuf[2], TRY_COUNTS);
    //save gain value
    ulOldGainValue = ulGainX1024;
}

//------------------------------------------------------------------------------
void SEN_SetMirrorFlip(uint8_t ubMirrorEn, uint8_t ubFlipEn)
{
    if(!ubIQ_GetLoadIqFinishFg())
        return;
    
    if(ubMirrorEn)
        xtSENInst.ubImgMode |=  IMX322_MIRROR;
    else
        xtSENInst.ubImgMode &=  ~IMX322_MIRROR;
    
    if(ubFlipEn)
        xtSENInst.ubImgMode |=  IMX322_FLIP;
    else
        xtSENInst.ubImgMode &=  ~IMX322_FLIP;
    
    bSEN_I2C_Write(0x01, 0x01, xtSENInst.ubImgMode);
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

}

//------------------------------------------------------------------------------
void SEN_SetSensorImageSize(void)
{
	// Image for ISP
	sensor_cfg.xtSENWin.uwHSize = ISP_WIDTH;
	sensor_cfg.xtSENWin.uwVSize = ISP_HEIGHT;
    sensor_cfg.xtSENWin.uwHStart = 0x10;
    sensor_cfg.xtSENWin.uwVStart = 0x10;
}

//------------------------------------------------------------------------------
void SEN_SetSensorType(void)
{
    sensor_cfg.ubSensorType = SEN_IMX323;	
    printf("sensor type is IMX323\r\n");
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
    uint8_t ubIdx; 
    uint16_t uwExpLine;
    uint32_t ulGainX1024;
    
    uwExpLine = ulExp;
    ulGainX1024 = ulGain/2;

    //EXPLINE
    bSEN_I2C_WriteTry(IMX322_DUMMY_LINE_H, (uint8_t)(((uwExpLine)>>8)&0x00ff), TRY_COUNTS);	
    bSEN_I2C_WriteTry(IMX322_DUMMY_LINE_L, (uint8_t)((uwExpLine)&0x00ff), TRY_COUNTS);     
 

    //GAIN
	for(ubIdx = 0; ubIdx<cbAE_SensorVTblRowSZ; ubIdx++)
	{
		if(ulGainX1024 <= ctSensor_SensorVTbl[ubIdx].uwAlgGain)
		{
			xtSENInst.ubBuf[2] = ctSensor_SensorVTbl[ubIdx].ubSenGain;
			break;
		}
	}

	if(ubIdx == cbAE_SensorVTblRowSZ)
	{
		xtSENInst.ubBuf[2] = ctSensor_SensorVTbl[ubIdx-1].ubSenGain;		
	}
    
    bSEN_I2C_WriteTry(IMX322_GAIN, xtSENInst.ubBuf[2], TRY_COUNTS);
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
