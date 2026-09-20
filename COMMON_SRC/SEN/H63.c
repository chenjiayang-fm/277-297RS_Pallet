/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		H62.c
	\brief		Sensor H62 relation function
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

#if (SEN_USE == SEN_H63)
#define TRY_COUNTS 3
AE_EVENT_PROCESS tAE_SensorPara;
struct SENSOR_SETTING sensor_cfg;
tfSENObj xtSENInst;
I2C1_Type *pI2C_type;
SEN_STATE sensor_state;

const uint8_t ubSEN_InitTable[] = {
	//------------------------------
	// Initial Table
	//------------------------------
    // H63_24Minput_30fps_43.2MPclk_720p
    0x82, 0x12, 0x44,  
    0x82, 0x48, 0x85,  
    0x82, 0x48, 0x05,  
    0x82, 0x0E, 0x11,  
    0x82, 0x0F, 0x14,  
    0x82, 0x10, 0x24,  
    0x82, 0x11, 0x80,  
    0x82, 0x0D, 0xD0,  
    0x82, 0x5F, 0x41,  
    0x82, 0x60, 0x20,  
    0x82, 0x58, 0x18,  
    0x82, 0x57, 0x60,  
    0x82, 0x20, 0xC0,  
    0x82, 0x21, 0x03,  
    0x82, 0x22, 0xEE,  
    0x82, 0x23, 0x02,  
    0x82, 0x24, 0x80,  
    0x82, 0x25, 0xD0,  
    0x82, 0x26, 0x22,  
    0x82, 0x27, 0xF0,  
    0x82, 0x28, 0x15,  
    0x82, 0x29, 0x02,  
    0x82, 0x2A, 0xE6,  
    0x82, 0x2B, 0x12,  
    0x82, 0x2C, 0x00,  
    0x82, 0x2D, 0x00,  
    0x82, 0x2E, 0xBA,  
    0x82, 0x2F, 0x40,  
    0x82, 0x41, 0x84,  
    0x82, 0x42, 0x32,  
    0x82, 0x46, 0x00,  
    0x82, 0x47, 0x42,  
    0x82, 0x76, 0x40,  
    0x82, 0x77, 0x06,  
    0x82, 0x80, 0x01,  
    0x82, 0xAF, 0x22,  
    0x82, 0x1D, 0xFF,  
    0x82, 0x1E, 0x1F,  
    0x82, 0x6C, 0xC0,  
    0x82, 0x30, 0x86,  
    0x82, 0x31, 0x04,  
    0x82, 0x32, 0x19,  
    0x82, 0x33, 0x10,  
    0x82, 0x34, 0x2A,  
    0x82, 0x35, 0x2A,  
    0x82, 0x3A, 0xA0,  
    0x82, 0x3B, 0x00,  
    0x82, 0x3C, 0x38,  
    0x82, 0x3D, 0x41,  
    0x82, 0x3E, 0xE0,  
    0x82, 0x56, 0x12,  
    0x82, 0x59, 0x46,  
    0x82, 0x5A, 0x02,  
    0x82, 0x85, 0x1E,  
    0x82, 0x8A, 0x04,  
    0x82, 0x9C, 0x61,  
    0x82, 0x5B, 0xAC,  
    0x82, 0x5C, 0x61,  
    0x82, 0x5D, 0xA6,  
    0x82, 0x5E, 0x14,  
    0x82, 0x64, 0xE0,  
    0x82, 0x66, 0x04,  
    0x82, 0x67, 0x53,  
    0x82, 0x68, 0x00,  
    0x82, 0x69, 0x74,  
    0x82, 0x7A, 0x60,  
    0x82, 0x8F, 0x91,  
    0x82, 0xAE, 0x30,  
    0x82, 0x13, 0x81,  
    0x82, 0x96, 0x84,  
    0x82, 0x4A, 0x05,  
    0x82, 0x7E, 0xCD,  
    0x82, 0x50, 0x02,  
    0x82, 0x49, 0x10,  
    0x82, 0x7B, 0x4A,  
    0x82, 0x7C, 0x0C,  
    0x82, 0x7F, 0x56,  
    0x82, 0x62, 0x21,  
    0x82, 0x90, 0x00,  
    0x82, 0x8C, 0xFF,  
    0x82, 0x8D, 0xC7,  
    0x82, 0x8E, 0x00,  
    0x82, 0x8B, 0x01,  
    0x82, 0x0C, 0x00,  
    0x82, 0xBB, 0x11,  
    0x82, 0xA0, 0x10,  
    0x82, 0x6A, 0x17,  
    0x82, 0x65, 0x34,  
    0x82, 0x82, 0x00,  
    0x82, 0x19, 0x20,  
    0x82, 0x12, 0x04,  
    0x82, 0x48, 0x85,  
    0x82, 0x48, 0x05,  
};

//------------------------------------------------------------------------------
bool bSEN_I2C_Read(uint8_t ubAddress, uint8_t *pValue)
{	
    #if (I2C_TYPE_SELECT == I2C_TYPE_POLLING)
        return bI2C_MasterProcess (pI2C_type, SEN_SLAVE_ADDR, &ubAddress, 1, pValue, 1);
    #else
        return bI2C_MasterInt (pI2C_type, I2C_INT, SEN_SLAVE_ADDR, &ubAddress, 1, pValue, 1);
    #endif
}

//------------------------------------------------------------------------------
bool bSEN_I2C_Write(uint8_t ubAddress, uint8_t ubValue)
{		
	uint8_t pBuf[2];
	
	pBuf[0] = ubAddress;
	pBuf[1] = ubValue;	

    #if (I2C_TYPE_SELECT == I2C_TYPE_POLLING)
        return bI2C_MasterProcess (pI2C_type, SEN_SLAVE_ADDR, &pBuf[0], 2, NULL, 0);
    #else
        return bI2C_MasterInt (pI2C_type, I2C_INT, SEN_SLAVE_ADDR, &pBuf[0], 2, NULL, 0);
    #endif
}

//------------------------------------------------------------------------------
bool bSEN_I2C_WriteTry(uint8_t ubAddress, uint8_t ubValue, uint8_t ubTryCnt)
{	
    uint8_t pBuf[2], ret, i = 0;
    
    pBuf[0] = ubAddress;        
    pBuf[1] = ubValue;
    
    do{
    #if (I2C_TYPE_SELECT == I2C_TYPE_POLLING)
        ret = bI2C_MasterProcess (pI2C_type, SEN_SLAVE_ADDR, &pBuf[0], 2, NULL, 0);
    #else
        ret = bI2C_MasterInt (pI2C_type, I2C_INT, SEN_SLAVE_ADDR, &pBuf[0], 2, NULL, 0);
    #endif
    }while((ret == 0) && ((i++) < ubTryCnt));
    if(ret == 0)
        printf("wr 0x%x fail \r\n",ubAddress);  
    
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
    tAE_SensorPara.ulSensorPclk = 43200000;  
    tAE_SensorPara.ulSensorPixelPerLine = 1920;
    tAE_SensorPara.ulSensorLinePerFrame = 750;
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
void MIPI_PowerDownTimingSelect(uint8_t ubPclkIdx)
{	
	printf("-------------------------------\n");
    
	switch (ubPclkIdx)
	{
		case SEN_FPS15:
			MIPI->DLAN_PDD_LSCALE_SEL0 = 18;	
			break;       
		case SEN_FPS30:
            //T_term-pd = 160ns (by oscilloscope)
            //DLAN_PDD_SEL = (T_term-pd / 16.67ns) = 9.59
			MIPI->DLAN_PDD_LSCALE_SEL0 = 10;
			break;
		default:
			MIPI->DLAN_PDD_LSCALE_SEL0 = 10;
			break;
	}
	printf("[FPS=%d]PDD select:%d\n",ubPclkIdx, MIPI->DLAN_PDD_LSCALE_SEL0);
	printf("-------------------------------\n");
}

//------------------------------------------------------------------------------//
//                         Auto phase detection                                 //
//------------------------------------------------------------------------------//
//------------------------------------------------------------------------------
uint8_t MIPI_MiddlePoint(uint32_t ulValue)
{
    uint8_t ubFirstZeroFlg = 0;
    uint8_t ubBit;
    uint8_t i;
    uint8_t ubZeroStar[16], ubZeroEnd[16];
    uint8_t ubZeroLength[16];
    uint8_t ubCnt1 = 0, ubCnt2 = 0;
    uint8_t ubMaxLength = 0;
    uint8_t ubIdx = 0;
    uint8_t ubMiddleValue;
    //
    memset(&ubZeroStar, 0, 16);
    memset(&ubZeroEnd, 0, 16);
    memset(&ubZeroLength, 0, 16);
    // calculate zero area.
    for(i=0; i<32; i++)
    {
        ubBit = (ulValue&(1<<i))>>i;
        if((ubBit == 0) && (ubFirstZeroFlg == 0))
        {
            ubZeroStar[ubCnt1] = i;
            ubCnt1++;
            ubFirstZeroFlg = 1;
        }else if((ubBit == 1) && (ubFirstZeroFlg == 1)){
            ubZeroEnd[ubCnt2] = i-1;
            ubCnt2++;
            ubFirstZeroFlg = 0;
        }
    }
    // select zero and longest area.
    for(i=0; i<ubCnt1; i++)
    {
        ubZeroLength[i] = (ubZeroEnd[i] > ubZeroStar[i])? (ubZeroEnd[i] - ubZeroStar[i] + 1) : (31 - ubZeroStar[i] + 1);
        
        if(ubMaxLength > ubZeroLength[i])
        {
            ubMaxLength = ubMaxLength;
        }else{
            ubMaxLength = ubZeroLength[i];
            ubIdx = i;
        }
    }
    //
    ubMiddleValue = (ubZeroEnd[ubIdx] > ubZeroStar[ubIdx])?
        ((ubZeroEnd[ubIdx] + ubZeroStar[ubIdx] + 1) / 2):((31 + ubZeroStar[ubIdx] + 1) / 2);
    
    return ubMiddleValue;
}

//------------------------------------------------------------------------------
void MIPI_AutoPhaseDetect(void)
{
    uint8_t ubClkSel;
    uint32_t count = 0;
    uint32_t PhaseReport0;
    uint32_t PhaseReport1;

    //Step 1, without setting CLK_SEL/DATA_LANE0_SEL/DATA_LANE1_SEL
    MIPI->DATA_LANE0_SEL0 = 0;		MIPI->DATA_LANE0_SEL1 = 0;		MIPI->DATA_LANE0_SEL2 = 0;		MIPI->DATA_LANE0_SEL3 = 0;		MIPI->DATA_LANE0_SEL4 = 0;		
    MIPI->DATA_LANE1_SEL0 = 0;		MIPI->DATA_LANE1_SEL1 = 0;		MIPI->DATA_LANE1_SEL2 = 0;		MIPI->DATA_LANE1_SEL3 = 0;		MIPI->DATA_LANE1_SEL4 = 0;	
    MIPI->CLK_SEL0 = 0;		MIPI->CLK_SEL1 = 0;		MIPI->CLK_SEL2 = 0;		MIPI->CLK_SEL3 = 0;		MIPI->CLK_SEL4 = 0;	
    
    //Step 2, clear Phase ready flag.
    MIPI->CLR_PHASE_DET_RDY = 1;
    
    //Step 3, Set up Phase Fail Condition.
    MIPI->PHASE_FAIL_CONDITION = 0;
    
    //Step 4, Set SHRINK_1BYTE_DATA_SIZE is 0.
    MIPI->SHRINK_1BYTE_DATA_SIZE = 0;
    
    //Setp 5, Set Auto Detect Condition.
    MIPI->AUTO_FW_DET_TRI = 1;
    MIPI->TRI_MODE = 1;
    MIPI->PACKET_CNT_SIZE = 3;//31
    MIPI->PHA_DET_EN = 1;
    
    //Step 6, Wait Phase detection ready.
    while(!MIPI->PHASE_DET_RDY)
    {
        if(count > 3)
        {
            printf("Chk MIPI Err flag!\r\n");
            break;
        }
        TIMER_Delay_ms(33);
        count++;
    }
    printf("-------------------------------\n");
    //Get phase detect report
    PhaseReport0 = MIPI->PHASE_DET_REPORT0;
    printf("PhaseReport0=0x%X\n",PhaseReport0);
    //Select the middle point of the longest PHASE_PASS region
    if( PhaseReport0==0 )
	{
		ubClkSel = (31+1)/2;
	}
	else
	{
        ubClkSel = MIPI_MiddlePoint(PhaseReport0);
	}
	//Step7, Set DATA_LANE0_SEL
	MIPI->DATA_LANE0_SEL0 = (ubClkSel & 0x01)>>0;
	MIPI->DATA_LANE0_SEL1 = (ubClkSel & 0x02)>>1;
	MIPI->DATA_LANE0_SEL2 = (ubClkSel & 0x04)>>2;
	MIPI->DATA_LANE0_SEL3 = (ubClkSel & 0x08)>>3;
	MIPI->DATA_LANE0_SEL4 = (ubClkSel & 0x10)>>4;
    
    //If run in 2 Lanes mode, Read PHASE_DETECT_REPORT1 as Step6 method and set DATA_LANE1_SEL.
    PhaseReport1 = MIPI->PHASE_DET_REPORT1;
    printf("PhaseReport1=0x%X\n",PhaseReport1);
    if( PhaseReport1==0 )
	{
		ubClkSel = (31+1)/2;
	}
	else
	{
        ubClkSel = MIPI_MiddlePoint(PhaseReport1);
	}
	//Step7, Set DATA_LANE1_SEL
	MIPI->DATA_LANE1_SEL0 = (ubClkSel & 0x01)>>0;
	MIPI->DATA_LANE1_SEL1 = (ubClkSel & 0x02)>>1;
	MIPI->DATA_LANE1_SEL2 = (ubClkSel & 0x04)>>2;
	MIPI->DATA_LANE1_SEL3 = (ubClkSel & 0x08)>>3;
	MIPI->DATA_LANE1_SEL4 = (ubClkSel & 0x10)>>4;
	printf("-------------------------------\n");
    
    //Step8, Disable phase detection function.
    MIPI->AUTO_FW_DET_TRI = 0;
    MIPI->TRI_MODE = 0;
    MIPI->PHA_DET_EN = 0;
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
	bSEN_I2C_Read (H63_CHIP_ID_HIGH_ADDR, &pBuf[1]);
	bSEN_I2C_Read (H63_CHIP_ID_LOW_ADDR, &pBuf[0]);
	if (H63_CHIP_ID != uwPID)
	{
		printd(DBG_ErrorLvl, "Read chip ID fail.<0x%x 0x%x>\n", H63_CHIP_ID, uwPID);
        return 0;
	}
    return 1;
}

//------------------------------------------------------------------------------
uint8_t ubSEN_SetSensorInitTable(void)
{
    I2C_SCL_SPEED_TYP tI2C_Clock = I2C_SCL_300K;
	uint8_t     temp;	
	uint32_t 	i;	
    
	pI2C_type = pI2C_MasterInit(I2C_2, tI2C_Clock);
	IQ_SetI2cType(pI2C_type, tI2C_Clock);
    //HW reset.
    SEN_SensorHwReset();
    
    if(ubSEN_CheckSensorState() != 1)
        return 0;
    printd(DBG_CriticalLvl, "H63 Sensor\n");
    //stream off
    bSEN_I2C_Read (0x74, &temp);
    temp |= (0x01<<7);
    bSEN_I2C_Write(0x74, temp);
    //Please set this trigger to frame end
    bSEN_I2C_Write(0x1F, 0x80);     
	//wait 33ms
    TIMER_Delay_ms(33);
    
	for (i=0; i<sizeof(ubSEN_InitTable); i+=3)
	{
		if (ubSEN_InitTable[i] == 0x82)	// write
		{
			bSEN_I2C_Write(ubSEN_InitTable[i+1], ubSEN_InitTable[i+2]);
		}else if (ubSEN_InitTable[i] == 0xbb){
            TIMER_Delay_ms(((ubSEN_InitTable[i+1]<<8) + ubSEN_InitTable[i+2]));
        }
	}   
//    // Set mirror and flip
//    bSEN_I2C_Write(0x12, 0x30);
    //stream on
//    bSEN_I2C_Read (0x74, &temp);
//    temp &= ~(0x01<<7);
//    bSEN_I2C_Write(0x74, temp);
    //Please set this trigger to frame end
    bSEN_I2C_Write(0x1F, 0x80);
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
    SEN->VSYNC_HIGH = 1;
    SEN->PCK_DLH_RIS = 1;
    SEN->SYNC_MODE = 1;
    SEN->SENSOR_MODE = 1;
    // change SN_SEN->RAW_REORDER to 2, start from Gr
    SEN->RAW_REORDER = 0;
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
        printf("H63 startup failed! \n\r");
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
	bSEN_I2C_WriteTry(0xc6, H63_FRAME_H, TRY_COUNTS);
	bSEN_I2C_WriteTry(0xc7, (uint8_t)((uwDL>>8) & 0xff), TRY_COUNTS);
	bSEN_I2C_WriteTry(0xc8, H63_FRAME_L, TRY_COUNTS);
	bSEN_I2C_WriteTry(0xc9, (uint8_t)(uwDL & 0xff), TRY_COUNTS);		
}

//------------------------------------------------------------------------------
void SEN_SetExpLine(void)
{
    SEN_WrExpLine(xtSENInst.xtSENCtl.uwExpLine);
}

//------------------------------------------------------------------------------
void SEN_WrExpLine(uint16_t uwExpLine)
{
	static uint16_t uwOldExpline = 0;
	
	if(uwExpLine == uwOldExpline)
		return;
	bSEN_I2C_WriteTry(0xc0, H63_EXP_H, TRY_COUNTS);
	bSEN_I2C_WriteTry(0xc1, (uint8_t)((uwExpLine>>8) & 0xff), TRY_COUNTS);
	bSEN_I2C_WriteTry(0xc2, H63_EXP_L, TRY_COUNTS);
	bSEN_I2C_WriteTry(0xc3, (uint8_t)(uwExpLine & 0xff), TRY_COUNTS);	
	
	uwOldExpline = uwExpLine;
}

//------------------------------------------------------------------------------
void SEN_WrMaxExpLine(uint32_t ulValue)
{
    xtSENInst.uwMaxExpLine = ulValue;
}

//------------------------------------------------------------------------------
void SEN_WrGain(uint32_t ulGainX1024)
{
	uint8_t  ubGaintmp;
	//avoid warning
	ubGaintmp = ubGaintmp;
	
    // update struct value.
    xtSENInst.xtSENCtl.uwGain = ulGainX1024;
	//	Min globe gain is 1x gain
	if (ulGainX1024 < 1024)			//Limit min value
	{
		ulGainX1024 = 1024;
	}		
		
    if(ulGainX1024 < 2048)							            //analog 1x~2x
    {
        ubGaintmp = 0x00+((ulGainX1024-1024)>>6);
    }
    else if(ulGainX1024 < 4096)					                //analog 2x~4x
    {		
        ubGaintmp = 0x10+((ulGainX1024-2048)>>7);
    }
    else if(ulGainX1024 < 8192)					                //analog 4x~8x
    {
        ubGaintmp = 0x20+((ulGainX1024-4096)>>8);
    }
    else if(ulGainX1024 < 16384)					            //analog 8x~16x
    {
        ubGaintmp = 0x30+((ulGainX1024-8192)>>9);
    }
    else if(ulGainX1024 < 32768)					            //digital*analog 16x~32x
    {			 
        ubGaintmp = 0x40+((ulGainX1024-16384)>>10);
    }		
    else
    {														    //digital*analog 32x~64x
        ubGaintmp = 0x4f;							
    }
	// gain	
	bSEN_I2C_WriteTry(0xc4, H63_GAIN, TRY_COUNTS);
	bSEN_I2C_WriteTry(0xc5, (ubGaintmp & 0xff), TRY_COUNTS);
}

//------------------------------------------------------------------------------
void SEN_SetMirrorFlip(uint8_t ubMirrorEn, uint8_t ubFlipEn)
{
    if(!ubIQ_GetLoadIqFinishFg())
        return;

    SEN_SetDetectVideoState(0);
    
    bSEN_I2C_Read (0x12, &xtSENInst.ubImgMode);
    if(ubMirrorEn)
        xtSENInst.ubImgMode |=  H63_MIRROR;
    else
        xtSENInst.ubImgMode &=  ~H63_MIRROR;
    
    if(ubFlipEn)
        xtSENInst.ubImgMode |=  H63_FLIP;
    else
        xtSENInst.ubImgMode &=  ~H63_FLIP;
    
    bSEN_I2C_Write(0x12, xtSENInst.ubImgMode);
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
	bSEN_I2C_Write(0x1f, 0x80);
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
    sensor_cfg.ubSensorType = SEN_H63;    
    printd(DBG_Debug1Lvl, "sensor type is H63\n");
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
    uint8_t  ubGaintmp;
    uint32_t ulGainX1024;
    //EXPLINE
	bSEN_I2C_WriteTry(0xc0, H63_EXP_H, TRY_COUNTS);
	bSEN_I2C_WriteTry(0xc1, (uint8_t)((ulExp>>8) & 0xff), TRY_COUNTS);
	
	bSEN_I2C_WriteTry(0xc2, H63_EXP_L, TRY_COUNTS);
	bSEN_I2C_WriteTry(0xc3, (uint8_t)(ulExp & 0xff), TRY_COUNTS);
    
    //GAIN
    ulGainX1024 = ulGain;
    if(ulGainX1024 < 2048)							            //analog 1x~2x
    {
        ubGaintmp = 0x00+((ulGainX1024-1024)>>6);
    }
    else if(ulGainX1024 < 4096)					                //analog 2x~4x
    {		
        ubGaintmp = 0x10+((ulGainX1024-2048)>>7);
    }
    else if(ulGainX1024 < 8192)					                //analog 4x~8x
    {
        ubGaintmp = 0x20+((ulGainX1024-4096)>>8);
    }
    else if(ulGainX1024 < 16384)					            //analog 8x~16x
    {
        ubGaintmp = 0x30+((ulGainX1024-8192)>>9);
    }
    else if(ulGainX1024 < 32768)					            //digital*analog 16x~32x
    {			 
        ubGaintmp = 0x40+((ulGainX1024-16384)>>10);
    }		
    else
    {														    //digital*analog 32x~64x
        ubGaintmp = 0x4f;							
    }
	bSEN_I2C_WriteTry(0xc4, H63_GAIN, TRY_COUNTS);
	bSEN_I2C_WriteTry(0xc5, (ubGaintmp & 0xff), TRY_COUNTS);
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
