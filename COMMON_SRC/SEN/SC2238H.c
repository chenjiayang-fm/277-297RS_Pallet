/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		SC2238H.c
	\brief		Sensor SC2238H relation function
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

#if (SEN_USE == SEN_SC2238H)
#define TRY_COUNTS 3
AE_EVENT_PROCESS tAE_SensorPara;
struct SENSOR_SETTING sensor_cfg;
tfSENObj xtSENInst;
I2C1_Type *pI2C_type;
SEN_STATE sensor_state;

const uint8_t ubSEN_InitTable[] = {
//SC2235_24Minput_30fps_81MPclk_1080p       
    0x83, 0x01, 0x03, 0x01,
    0x83, 0x01, 0x00, 0x00,
    0x83, 0x30, 0x39, 0x80,
    0x83, 0x36, 0x21, 0x28,
                      
    0x83, 0x33, 0x09, 0x60,
    0x83, 0x33, 0x1f, 0x4d,
    0x83, 0x33, 0x21, 0x4f,
    0x83, 0x33, 0xb5, 0x10,
                      
    0x83, 0x33, 0x03, 0x20,
    0x83, 0x33, 0x1e, 0xd,
    0x83, 0x33, 0x20, 0xf,
                      
    0x83, 0x36, 0x22, 0x02,
    0x83, 0x36, 0x33, 0x42,
    0x83, 0x36, 0x34, 0x42,
                      
    0x83, 0x33, 0x06, 0x66,
    0x83, 0x33, 0x0b, 0xd1,
                      
    0x83, 0x33, 0x01, 0x0e,
                      
    0x83, 0x32, 0x0c, 0x08,
    0x83, 0x32, 0x0d, 0x98,
                      
    0x83, 0x33, 0x64, 0x05,// [2] 1: write at sampling ending
                      
    0x83, 0x36, 0x3c, 0x28, //bypass nvdd
    0x83, 0x36, 0x3b, 0x0a, //HVDD
    0x83, 0x36, 0x35, 0xa0, //TXVDD
                      
    0x83, 0x45, 0x00, 0x59,
    0x83, 0x3d, 0x08, 0x00,
    0x83, 0x39, 0x08, 0x11,
                      
    0x83, 0x36, 0x3c, 0x08,
                      
    0x83, 0x3e, 0x03, 0x03,
    0x83, 0x3e, 0x01, 0x46,
                      
                      
    // 0x3e08,0x7f,   
    // 0x3e09,0x1f,   
    // 0x5000,0x00,   
    // 0x3908,0x31,   
    // 0703        
    0x83, 0x33, 0x81, 0x0a,
    0x83, 0x33, 0x48, 0x09,
    0x83, 0x33, 0x49, 0x50,
    0x83, 0x33, 0x4a, 0x02,
    0x83, 0x33, 0x4b, 0x60,

    0x83, 0x33, 0x80, 0x04,
    0x83, 0x33, 0x40, 0x06,
    0x83, 0x33, 0x41, 0x50,
    0x83, 0x33, 0x42, 0x02,
    0x83, 0x33, 0x43, 0x60,

    //07, 0x07

    0x83, 0x36, 0x32, 0x88, //anti sm
    0x83, 0x33, 0x09, 0xa0,
    0x83, 0x33, 0x1f, 0x8d,
    0x83, 0x33, 0x21, 0x8f,
                      
                      
    0x83, 0x33, 0x5e, 0x01,  //ana dithering
    0x83, 0x33, 0x5f, 0x03,
    0x83, 0x33, 0x7c, 0x04,
    0x83, 0x33, 0x7d, 0x06,
    0x83, 0x33, 0xa0, 0x05,
    0x83, 0x33, 0x01, 0x05,
                      
    //atuo logic  
                      
    0x83, 0x36, 0x70, 0x08 , //[3]:3633 logic ctrl  real value in 3682
    0x83, 0x36, 0x7e, 0x07,  //gain0
    0x83, 0x36, 0x7f, 0x0f,  //gain1
    0x83, 0x36, 0x77, 0x2f,  //<gain0
    0x83, 0x36, 0x78, 0x22,  //gain0 - gain1
    0x83, 0x36, 0x79, 0x43,  //>gain1
                      
    0x83, 0x33, 0x7f, 0x03, //new auto precharge  330e in 3372   [7:6] 11: close div_rst 00:open div_rst
    0x83, 0x33, 0x68, 0x02,
    0x83, 0x33, 0x69, 0x00,
    0x83, 0x33, 0x6a, 0x00,
    0x83, 0x33, 0x6b, 0x00,
    0x83, 0x33, 0x67, 0x08,
    0x83, 0x33, 0x0e, 0x30,
                      
    0x83, 0x33, 0x66, 0x7c, // div_rst gap
                      
    0x83, 0x36, 0x35, 0xc1,
    0x83, 0x36, 0x3b, 0x09,
    0x83, 0x36, 0x3c, 0x07,
                      
    0x83, 0x39, 0x1e, 0x00,
                      
    0x83, 0x36, 0x37, 0x14, //fullwell 7K
                      
    0x83, 0x33, 0x06, 0x54,
    0x83, 0x33, 0x0b, 0xd8,
    0x83, 0x36, 0x6e, 0x08,  // ofs auto en [3]
    0x83, 0x36, 0x6f, 0x2f,  // ofs+finegain  real ofs in 0x3687[4:0]
                      
    0x83, 0x36, 0x31, 0x84,
    0x83, 0x36, 0x30, 0x48,
    0x83, 0x36, 0x22, 0x06,
                      
                      
    //ramp by sc    
    0x83, 0x36, 0x38, 0x1f,
    0x83, 0x36, 0x25, 0x02,
    0x83, 0x36, 0x36, 0x24,
                      
    //0714        
    0x83, 0x33, 0x48, 0x08,
    0x83, 0x3e, 0x03, 0x0b,
                      
    //7.17 fpn    
    0x83, 0x33, 0x42, 0x03,
    0x83, 0x33, 0x43, 0xa0,
    0x83, 0x33, 0x4a, 0x03,
    0x83, 0x33, 0x4b, 0xa0,
                      
    //0718        
    0x83, 0x33, 0x43, 0xb0,
    0x83, 0x33, 0x4b, 0xb0,
                      
    //0720        
                      
    //digital ctr l
    0x83, 0x38, 0x02, 0x01,
    0x83, 0x32, 0x35, 0x04,
    0x83, 0x32, 0x36, 0x63, // vts-2
                      
    //fpn           
    0x83, 0x33, 0x43, 0xd0,
    0x83, 0x33, 0x4b, 0xd0,
    0x83, 0x33, 0x48, 0x07,
    0x83, 0x33, 0x49, 0x80,
                      
    //0724        
    0x83, 0x39, 0x1b, 0x4d,
                      
    0x83, 0x33, 0x42, 0x04,
    0x83, 0x33, 0x43, 0x20,
    0x83, 0x33, 0x4a, 0x04,
    0x83, 0x33, 0x4b, 0x20,
                      
    //0804        
    0x83, 0x32, 0x22, 0x29,
    0x83, 0x39, 0x01, 0x02,
                      
    //0808        
                      
    //di, 0xgital ctr l
    0x83, 0x3f, 0x00, 0x07,  // bit[2] = 1
    0x83, 0x3f, 0x04, 0x08,
    0x83, 0x3f, 0x05, 0x74,  // hts - 0x24
                      
    //0809        
    0x83, 0x33, 0x0b, 0xc8,
                      
    //0817        
    0x83, 0x33, 0x06, 0x4a,
    0x83, 0x33, 0x0b, 0xca,
    0x83, 0x36, 0x39, 0x09,
                      
    //manual DPC  
    0x83, 0x57, 0x80, 0xff,
    0x83, 0x57, 0x81, 0x04,
    0x83, 0x57, 0x85, 0x18,
                      
    //0822
    // pixel clock = sensor clock * (64-PLLDIV) / (PreDiv * (SYSEL+1))
    //             = 24MHz * (64-37) / (2*(3+1)) = 81MHz
    // 0x3039[3:1] = PreDiv = 0x10 = 2, 0x3039[6:4] = SYSEL = 0x11 = 3
    // PLLDIV = {0x3039[0], 0x303a[7:3]} = 37 
    0x83, 0x30, 0x39, 0x35, 
    0x83, 0x30, 0x3a, 0x2e,
    0x83, 0x30, 0x34, 0x05,
    0x83, 0x30, 0x35, 0x2a,
    // pixel per line = {0x320c, 0x320d} = 2250
    // line per frame = {0x320e, 0x320f} = 1200
    0x83, 0x32, 0x0c, 0x08,
    0x83, 0x32, 0x0d, 0xca,
    0x83, 0x32, 0x0e, 0x04,
    0x83, 0x32, 0x0f, 0xb0,
                      
    0x83, 0x3f, 0x04, 0x08,
    0x83, 0x3f, 0x05, 0xa6, // hts - 0x24
                      
    0x83, 0x32, 0x35, 0x04,
    0x83, 0x32, 0x36, 0xae, // vts-2
                      
                      
    //0825        
    0x83, 0x33, 0x13, 0x05,
    0x83, 0x36, 0x78, 0x42,
                      
    //for AE contro l per frame
    0x83, 0x36, 0x70, 0x00,
    0x83, 0x36, 0x33, 0x42,
                      
    0x83, 0x38, 0x02, 0x00,
                      
    //20180126	  
    0x83, 0x36, 0x77, 0x3f,
    0x83, 0x33, 0x06, 0x44,//20180126[3c,4a]
    0x83, 0x33, 0x0b, 0xca,//20180126[c2,d3]
                      
    //20180202    
    0x83, 0x32, 0x37, 0x08,
    0x83, 0x32, 0x38, 0x9a, //hts-0x30
                      
    //20180417    
    0x83, 0x36, 0x41, 0x01,
                      
                      
    0x83, 0x33, 0x01, 0x12,//[8,15]20180126
    0x83, 0x36, 0x31, 0x84,
    0x83, 0x36, 0x6f, 0x2f,
    0x83, 0x36, 0x22, 0xc6,//20180117
		
    
    // output window height((1080 to 1088
    0x83, 0x32, 0x0a, 0x04,		
    0x83, 0x32, 0x0b, 0x40,
    // row start position (8 to 6
    0x83, 0x32, 0x12, 0x00,
    0x83, 0x32, 0x13, 0x06,
	//0x83, 0x01, 0x00, 0x01,
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
    tAE_SensorPara.ulSensorPclk = 43200000;  
    tAE_SensorPara.ulSensorPixelPerLine = 1440;
    tAE_SensorPara.ulSensorLinePerFrame = 1000;
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
	bSEN_I2C_Read (SC2238H_CHIP_ID_HIGH_ADDR, &pBuf[1]);
	bSEN_I2C_Read (SC2238H_CHIP_ID_LOW_ADDR, &pBuf[0]);
	if (SC2238H_CHIP_ID != uwPID)
	{
		printd(DBG_ErrorLvl, "I2C read fail.<0x%x 0x%x>\n", SC2238H_CHIP_ID, uwPID);
        return 0;
	}
    return 1;
}

//------------------------------------------------------------------------------
uint8_t ubSEN_SetSensorInitTable(void)
{
    I2C_SCL_SPEED_TYP tI2C_Clock = I2C_SCL_300K;
	uint8_t     ubTemp;
	uint32_t 	i;	
	
	pI2C_type = pI2C_MasterInit (I2C_2, tI2C_Clock);
	IQ_SetI2cType(pI2C_type, tI2C_Clock);
    //HW reset.
    SEN_SensorHwReset();
    
    // software reset
    bSEN_I2C_Read (0x0103, &ubTemp);
    ubTemp |= (0x1<<0);
    bSEN_I2C_Write(0x01, 0x03, ubTemp);
    TIMER_Delay_ms(30);
    
    if(ubSEN_CheckSensorState() != 1)
        return 0;
    printd(DBG_CriticalLvl, "SC2238H Sensor\n");

	// stop streaming(sleep mode
	bSEN_I2C_Write(0x01, 0x00, 0x00); 
	
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
    SEN->PCK_DLH_RIS = 0;
    SEN->SYNC_MODE = 1;		
    SEN->SENSOR_MODE = 0;
    // change SN_SEN->RAW_REORDER to 0, start from B
    SEN->RAW_REORDER = 1;
    // set dummy line & pixel
    SEN->DMY_DIV = 3;       // dummy clock=sysclk/2
    SEN->NUM_DMY_LN = 24;
    SEN->NUM_DMY_DSTB = 256;
    SEN->DMY_INSERT = 1;

    // Initial dummy sensor
    if (ubSEN_SetSensorInitTable() != 1)
    {
        printf("SC2235 startup failed! \n\r");
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
    uint16_t uwTemp;
	//Set dummy lines
	bSEN_I2C_WriteTry(SC2238H_FRAME_LENGTH_H, (uint8_t)((uwDL>>8) &0x00ff), TRY_COUNTS);
	bSEN_I2C_WriteTry(SC2238H_FRAME_LENGTH_L, (uint8_t)(uwDL &0x00ff), TRY_COUNTS);     
    
    // for smartsense suggestion
    uwTemp = uwDL - 0x02;   
    bSEN_I2C_WriteTry(SC2238H_D_FRAME_LENGTH_H, (uint8_t)((uwTemp>>8) &0x00ff), TRY_COUNTS);  
    bSEN_I2C_WriteTry(SC2238H_D_FRAME_LENGTH_L, (uint8_t)(uwTemp &0x00ff), TRY_COUNTS);   
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
    uint8_t ubTemp;
    
    if(uwExpLine == ulOldExpLine)
        return;
    
    // set exposure
    bSEN_I2C_WriteTry(SC2238H_EXPH, (uint8_t)((uwExpLine>>12)&0x001f), TRY_COUNTS);
    bSEN_I2C_WriteTry(SC2238H_EXPM, (uint8_t)((uwExpLine>>4)&0x00ff), TRY_COUNTS);
    bSEN_I2C_WriteTry(SC2238H_EXPL, (uint8_t)((uwExpLine&0x0f) << 4), TRY_COUNTS);
    
    // set trigger
    //step1~3
	bSEN_I2C_Write(0x39, 0x03, 0x84);
	bSEN_I2C_Write(0x39, 0x03, 0x04);    
    //step5
    ubTemp = ((uwExpLine >> 4) & 0XFF); 
    if(ubTemp < 0x05)
    {
        bSEN_I2C_Write(0x33, 0x14, 0x12);
    }else if(ubTemp > 0x0a){
        bSEN_I2C_Write(0x33, 0x14, 0x02);    
    }
    
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
    uint8_t ubGain_3e08 = 0x03;
    uint8_t ubGain_3e09 = 0x10;
    
    // update struct value.
    xtSENInst.xtSENCtl.uwGain = ulGainX1024;
    //	Min globe gain is 1x gain
	if (ulGainX1024 < 1024)			//Limit min value
	{
		ulGainX1024 = 1024;
	}else if(ulGainX1024 > 63488){
        ulGainX1024 = 63488;
    }
    //
    if(ulGainX1024 == ulOldGainValue)
        return;

    //
    if(ulGainX1024 < 2048)              // x1~x2
    {
        uwGain_H = 1;
        ubGain_3e08 = 0x03;
    }else if(ulGainX1024 < 4096){       // x2~x4
        uwGain_H = 2;
        ubGain_3e08 = 0x07;    
    }else if(ulGainX1024 < 8192){       // x4~x8
        uwGain_H = 4;
        ubGain_3e08 = 0x0F;    
    }else if(ulGainX1024 < 16384){      // x8~x16
        uwGain_H = 8;
        ubGain_3e08 = 0x1F;    
    }else if(ulGainX1024 < 32768){      // x16~x32
        uwGain_H = 16;
        ubGain_3e08 = 0x3F;    
    }else if(ulGainX1024 < 65536){      // x32~x64
        uwGain_H = 32;
        ubGain_3e08 = 0x7F;    
    }
    ubGain_3e09 = (((ulGainX1024 - (uwGain_H<<10))/(uwGain_H<<6)) | 0x10);

    // manual DPC
    if(ulGainX1024 < 10240)             // < x10
    {
        bSEN_I2C_Write(0x57, 0x81, 0x04);
        bSEN_I2C_Write(0x57, 0x85, 0x18);
    }else{
        bSEN_I2C_Write(0x57, 0x81, 0x02);
        bSEN_I2C_Write(0x57, 0x85, 0x08);
    }
    
    //latch
    bSEN_I2C_WriteTry(0x3812, 0x00, TRY_COUNTS);
    
    // set gain
    bSEN_I2C_WriteTry(SC2238H_GAIN_H, ubGain_3e08, TRY_COUNTS);
    bSEN_I2C_WriteTry(SC2238H_GAIN_L, ubGain_3e09, TRY_COUNTS);
    
    // set trigger
    // step4
    if(((ubGain_3e08>>2)&0x07)==0x00)             // gain < 2
    {
        bSEN_I2C_Write(0x33, 0x01, 0x12);
        bSEN_I2C_Write(0x36, 0x31, 0x84);
        bSEN_I2C_Write(0x36, 0x6f, 0x2f);
        bSEN_I2C_Write(0x36, 0x22, 0xc6);
    }else if((((ubGain_3e08>>2)&0x07))<=0x03){	
        bSEN_I2C_Write(0x33, 0x01, 0x15);
        bSEN_I2C_Write(0x36, 0x31, 0x88);
        bSEN_I2C_Write(0x36, 0x6f, 0x2f);
        bSEN_I2C_Write(0x36, 0x22, 0xc6);
    }else if((((ubGain_3e08>>2)&0x07)==0x07) && ubGain_3e09<=0x1e){
        bSEN_I2C_Write(0x33, 0x01, 0x1e);
        bSEN_I2C_Write(0x36, 0x31, 0x88);
        bSEN_I2C_Write(0x36, 0x6f, 0x2f);
        bSEN_I2C_Write(0x36, 0x22, 0xc6);
    }else if((((ubGain_3e08>>2)&0x07)==0x07) && ubGain_3e09==0x1f){ 
        bSEN_I2C_Write(0x33, 0x01, 0xff);
        bSEN_I2C_Write(0x36, 0x31, 0x88);
        bSEN_I2C_Write(0x36, 0x6f, 0x3a);
        bSEN_I2C_Write(0x36, 0x22, 0x06);
    }
    //
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
        xtSENInst.ubImgMode |=  SC2238H_MIRROR;
    else
        xtSENInst.ubImgMode &=  ~SC2238H_MIRROR;
    
    if(ubFlipEn)
        xtSENInst.ubImgMode |=  SC2238H_FLIP;
    else
        xtSENInst.ubImgMode &=  ~SC2238H_FLIP;
    
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
    sensor_cfg.ubSensorType = SEN_SC2238H;	
    printf("sensor type is SC2238\r\n");
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
    uint8_t ubTemp;
    uint8_t ubGain_3e08;   
    uint8_t ubGain_3e09;    
    uint16_t uwExpLine;
    uint16_t uwGain_H;
    uint32_t ulGainX1024;
    
    uwExpLine = ulExp;
    ulGainX1024 = ulGain;

    //EXPLINE
    bSEN_I2C_WriteTry(SC2238H_EXPH, (uint8_t)((uwExpLine>>12)&0x001f), TRY_COUNTS);
    bSEN_I2C_WriteTry(SC2238H_EXPM, (uint8_t)((uwExpLine>>4)&0x00ff), TRY_COUNTS);
    bSEN_I2C_WriteTry(SC2238H_EXPL, (uint8_t)((uwExpLine&0x0f) << 4), TRY_COUNTS);
   
    // set trigger
    //step1~3
	bSEN_I2C_Write(0x39, 0x03, 0x84);
	bSEN_I2C_Write(0x39, 0x03, 0x04);    
    //step5
    ubTemp = ((uwExpLine >> 4) & 0XFF); 
    if(ubTemp < 0x05)
    {
        bSEN_I2C_Write(0x33, 0x14, 0x12);
    }else if(ubTemp > 0x0a){
        bSEN_I2C_Write(0x33, 0x14, 0x02);    
    }
   
    //GAIN
    if(ulGainX1024 < 2048)              // x1~x2
    {
        uwGain_H = 1;
        ubGain_3e08 = 0x03;
    }else if(ulGainX1024 < 4096){       // x2~x4
        uwGain_H = 2;
        ubGain_3e08 = 0x07;    
    }else if(ulGainX1024 < 8192){       // x4~x8
        uwGain_H = 4;
        ubGain_3e08 = 0x0F;    
    }else if(ulGainX1024 < 16384){      // x8~x16
        uwGain_H = 8;
        ubGain_3e08 = 0x1F;    
    }else if(ulGainX1024 < 32768){      // x16~x32
        uwGain_H = 16;
        ubGain_3e08 = 0x3F;    
    }else if(ulGainX1024 < 65536){      // x32~x64
        uwGain_H = 32;
        ubGain_3e08 = 0x7F;    
    }  
    ubGain_3e09 = (((ulGainX1024 - (uwGain_H<<10))/(uwGain_H<<6)) | 0x10);

    // manual DPC
    if(ulGainX1024 < 10240)             // < x10
    {
        bSEN_I2C_Write(0x57, 0x81, 0x04);
        bSEN_I2C_Write(0x57, 0x85, 0x18);
    }else{
        bSEN_I2C_Write(0x57, 0x81, 0x02);
        bSEN_I2C_Write(0x57, 0x85, 0x08);
    }
    
    // set gain
    bSEN_I2C_WriteTry(SC2238H_GAIN_H, ubGain_3e08, TRY_COUNTS);
    bSEN_I2C_WriteTry(SC2238H_GAIN_L, ubGain_3e09, TRY_COUNTS);
    
    // set trigger
    // step4
    if(((ubGain_3e08>>2)&0x07)==0x00)             // gain < 2
    {
        bSEN_I2C_Write(0x33, 0x01, 0x12);
        bSEN_I2C_Write(0x36, 0x31, 0x84);
        bSEN_I2C_Write(0x36, 0x6f, 0x2f);
        bSEN_I2C_Write(0x36, 0x22, 0xc6);
    }else if((((ubGain_3e08>>2)&0x07))<=0x03){	
        bSEN_I2C_Write(0x33, 0x01, 0x15);
        bSEN_I2C_Write(0x36, 0x31, 0x88);
        bSEN_I2C_Write(0x36, 0x6f, 0x2f);
        bSEN_I2C_Write(0x36, 0x22, 0xc6);
    }else if((((ubGain_3e08>>2)&0x07)==0x07) && ubGain_3e09<=0x1e){
        bSEN_I2C_Write(0x33, 0x01, 0x1e);
        bSEN_I2C_Write(0x36, 0x31, 0x88);
        bSEN_I2C_Write(0x36, 0x6f, 0x2f);
        bSEN_I2C_Write(0x36, 0x22, 0xc6);
    }else if((((ubGain_3e08>>2)&0x07)==0x07) && ubGain_3e09==0x1f){ 
        bSEN_I2C_Write(0x33, 0x01, 0xff);
        bSEN_I2C_Write(0x36, 0x31, 0x88);
        bSEN_I2C_Write(0x36, 0x6f, 0x3a);
        bSEN_I2C_Write(0x36, 0x22, 0x06);
    }
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
