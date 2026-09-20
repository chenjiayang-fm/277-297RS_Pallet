/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		PS5268.c
	\brief		Sensor PS5268 relation function
	\author		BoCun
	\version	1.0
	\date		2022-04-15
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

#if (SEN_USE == SEN_PS5268)
#define TRY_COUNTS 3
AE_EVENT_PROCESS tAE_SensorPara;
struct SENSOR_SETTING sensor_cfg;
tfSENObj xtSENInst;
I2C1_Type *pI2C_type;
SEN_STATE sensor_state;

const uint8_t ubSEN_InitTable[] = { 
	//pck 81000000Hz 1920x1088+16dummy
	0x83, 0x01, 0x08, 0x02,
	0x83, 0x01, 0x0B, 0x05,
	0x83, 0x01, 0x14, 0x12,
	0x83, 0x01, 0x15, 0xC0,
	0x83, 0x01, 0x47, 0x04,//CMD_vsize[10:8]//443
	0x83, 0x01, 0x48, 0x53,//CMD_vszie[7:0]
	0x83, 0x01, 0x4b, 0x04,//CMD_ISP_vsize[10:8]//440
	0x83, 0x01, 0x4c, 0x50,//CMD_ISP_vsize[7:0]
	0x83, 0x01, 0x78, 0xC0,
	0x83, 0x01, 0x79, 0x2A,
	                  
	0x83, 0x02, 0x2E, 0x0E,
	0x83, 0x02, 0x2F, 0x19,
	0x83, 0x0E, 0x10, 0x02,
	0x83, 0x0E, 0x00, 0x10,
	0x83, 0x02, 0x2D, 0x01,
	0x83, 0x02, 0x26, 0xB1,
	0x83, 0x02, 0x27, 0x39,
	0x83, 0x02, 0x1C, 0x00,
	0x83, 0x02, 0x33, 0x70,
	0x83, 0x02, 0x4D, 0x21,
	0x83, 0x06, 0x64, 0x02,
	0x83, 0x06, 0x65, 0xAD,
	0x83, 0x0B, 0x02, 0x02,
	0x83, 0x0B, 0x0A, 0xFF,
	0x83, 0x0e, 0x0d, 0x04,//WOI_vsize[10:8]
	0x83, 0x0e, 0x0e, 0x50,//WOI_vsize[7:0]
	                  
	0x83, 0x12, 0x00, 0x00,
	0x83, 0x12, 0x01, 0x01,
	0x83, 0x13, 0x00, 0x00,
	0x83, 0x13, 0x01, 0x01,
	0x83, 0x13, 0x03, 0x02,
	//LTM HDR
	0x83, 0x13, 0x04,0x06, //????    
	0x83, 0x13, 0x06,0x89,//    //Brightness_1-2xGain [3:0]&[7:4]
	0x83, 0x13, 0x07,0x88,//   //Brightness_4-8xGain  [3:0]&[7:4]
	0x83, 0x13, 0x08,0x36,//         //Brightness_16-32xGain [3:0]&[7:4]
	0x83, 0x13, 0x09,0x48,//    //Contrast_1-2xGain     [3:0]&[7:4]
	0x83, 0x13, 0x0A,0x23,//    //Contrast_4-8xGain     [3:0]&[7:4]
	0x83, 0x13, 0x0B,0x11,          //Contrast_16-32xGain     [3:0]&[7:4]
	0x83, 0x13, 0x0C,0x40,//83,   //Inverse_1xGain
	0x83, 0x13, 0x0D,0x50,//83,   //Inverse_2xGain
	0x83, 0x13, 0x0E,0x60,//83,   //Inverse_4xGain
	0x83, 0x13, 0x0F,0x80,//83,   //Inverse_8xGain
	0x83, 0x13, 0x10,0x80,//70,   //Inverse_16xGain
	0x83, 0x13, 0x11,0x80,//47,   //Inverse_32xGain                        
	0x83, 0x13, 0x14,0x05,
	0x83, 0x13, 0x18,0xF0,
	0x83, 0x13, 0x19,0x04,
	0x83, 0x13, 0x1a,0x07,	
	//??????   
	0x83, 0x01, 0x11,0x01,
	0x83, 0x01, 0x0F,0x01,

	0xff, 0xff, 0xff, 0xff
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
    // support 1-30 fps for 1920x1080
    if(ubPclkIdx > 30)
    {
        ubPclkIdx = 30;
    }
    // set sensor struct value

    tAE_SensorPara.ulSensorPclk = 81000000; 
    tAE_SensorPara.ulSensorPixelPerLine = 2400;
    tAE_SensorPara.ulSensorLinePerFrame = 1124;//1124+1
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
	uint16_t 	ubPID = 0;
      
	pBuf = (uint8_t*)&ubPID; 
	// I2C by Read Sensor ID
    bSEN_I2C_Read (PS5268_CHIP_ID_HIGH_ADDR, &pBuf[1]);
	bSEN_I2C_Read (PS5268_CHIP_ID_LOW_ADDR, &pBuf[0]);
	if (PS5268_CHIP_ID != ubPID)
	{
		printd(DBG_ErrorLvl, "I2C read fail.<0x%x 0x%x>\n", PS5268_CHIP_ID, ubPID);
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
    printd(DBG_CriticalLvl, "PS5268 Sensor\n");
	for (i=0; i<sizeof(ubSEN_InitTable); i+=4)
	{
		if (ubSEN_InitTable[i] == 0x83)	// write
		{
			bSEN_I2C_Write(ubSEN_InitTable[i+1], ubSEN_InitTable[i+2], ubSEN_InitTable[i+3]);
		}else if (ubSEN_InitTable[i] == 0xbb){
            TIMER_Delay_ms(((ubSEN_InitTable[i+1]<<8) + ubSEN_InitTable[i+2]));
        }
	}
    //mirror and flip
	SEN_SetMirrorFlip(0,0);
	
    SEN_PclkSetting(SEN_FPS30);
    
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
    SEN->RAW_REORDER = 3;
    // Set dummy line & pixel
    SEN->DMY_DIV = 3;
    SEN->NUM_DMY_LN = 24;
    SEN->NUM_DMY_DSTB = 256;
    SEN->DMY_INSERT = 1;
    
	//delay 1ms
    TIMER_Delay_us(1000);
    // Initial dummy sensor
    if (ubSEN_SetSensorInitTable() != 1)
    {
        printf("PS5268 startup failed! \n\r");
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

	//Calculate Dummy line
	if(xtSENInst.xtSENCtl.uwExpLine < 2)
	{
		xtSENInst.xtSENCtl.uwExpLine = 2;
	}	
	
	//Calculate Dummy line		
	if(xtSENInst.xtSENCtl.uwExpLine > xtSENInst.uwMaxExpLine-8)
	{
		xtSENInst.xtSENCtl.uwDmyLine = (xtSENInst.xtSENCtl.uwExpLine+8);
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
	static uint16_t uwOldDummy = 0;
    if(uwDL == uwOldDummy)  return;
	
    //Set Dummy Line
    bSEN_I2C_WriteTry(PS5268_FRAME_LINE_H, (uint8_t)((uwDL>>8) & 0x00ff), TRY_COUNTS);		
    bSEN_I2C_WriteTry(PS5268_FRAME_LINE_L, (uint8_t)((uwDL) & 0x00ff), TRY_COUNTS);	
	//Update
	bSEN_I2C_WriteTry(0x0111, 0x01, TRY_COUNTS);	   
	uwOldDummy = uwDL;
}

//------------------------------------------------------------------------------
void SEN_SetExpLine(void)
{
    SEN_WrExpLine(xtSENInst.xtSENCtl.uwExpLine);
}

//------------------------------------------------------------------------------
void SEN_WrExpLine(uint16_t uwExpLine)
{
	uint32_t tmp;
	
	if(uwExpLine <= xtSENInst.uwMaxExpLine)
	{
		tmp = xtSENInst.uwMaxExpLine-uwExpLine;
	}
	else
	{
		tmp = 2;
	}
	//MSB
	bSEN_I2C_WriteTry(PS5268_EXL_H, (uint16_t)((tmp>>8) & 0x00ff), TRY_COUNTS); 
	TIMER_Delay_us(20);
	//LSB
	bSEN_I2C_WriteTry(PS5268_EXL_L, (uint16_t)(tmp & 0x00ff), TRY_COUNTS); 
	TIMER_Delay_us(20);
	//Update
	bSEN_I2C_WriteTry(0x0111, 0x01, TRY_COUNTS);
}

//------------------------------------------------------------------------------
void SEN_WrMaxExpLine(uint32_t ulValue)
{
    xtSENInst.uwMaxExpLine = ulValue;
}

//------------------------------------------------------------------------------
void SEN_WrGain(uint32_t ulGainX1024)
{
	//Max globe gain is 64x gain
	static  uint8_t sghd;
	
    uint32_t tal_gain;  
	uint32_t udata,tmp;
	
	// update struct value.
    xtSENInst.xtSENCtl.uwGain = ulGainX1024;
	
	if (ulGainX1024 <= 1024)		//Limit min value, 1x
	{
		ulGainX1024 = 1024;
		
	}			 
	if (ulGainX1024 >= 65535)		//Limit min value, 1x
	{
		ulGainX1024 = 65535;
		
	}
		
   	tal_gain = ulGainX1024 >> 6;
	
	if(tal_gain > 0x40)		// > 4.0x
	{
		sghd = 0;
	}
	else if(tal_gain < 0x30)	// < 3.0x
	{
		sghd = 1;
	}
	
	if(sghd == 0)	
	tal_gain >>= 1;	// Based on 2.0x
	if (tal_gain <= 16)		//Limit min value, 1x
	{
		tal_gain = 16;
		
	}	
	if (tal_gain >= 512)		//Limit min value, 1x
	{
		tal_gain = 512;
		
	}	
    tmp = 0;
	udata = (tal_gain>>4);
    //leading one
    while(udata>>=1)
	{
        tmp++;
    }
	if(tmp > 4)	tmp = 4;
	tmp = (tmp<<4)+(tal_gain>>tmp)-0x10;
	
	//SGHD
	bSEN_I2C_WriteTry(0x0128, (uint8_t)sghd, TRY_COUNTS);	
	//gain_idx
	bSEN_I2C_WriteTry(0x012b, (uint8_t)((tmp)& 0x007f), TRY_COUNTS);
	//UpdateFlag
	bSEN_I2C_WriteTry(0x0111, 0x01, TRY_COUNTS);
	//SGHD
	bSEN_I2C_WriteTry(0x0128, (uint8_t)sghd, TRY_COUNTS); 

}

//------------------------------------------------------------------------------
void SEN_SetMirrorFlip(uint8_t ubMirrorEn, uint8_t ubFlipEn)
{
    if(!ubIQ_GetLoadIqFinishFg())
        return;
    
    if(ubMirrorEn)
        xtSENInst.ubImgMode |=  PS5268_MIRROR;
    else
        xtSENInst.ubImgMode &=  ~PS5268_MIRROR;
    bSEN_I2C_Write(0x01, 0x40, xtSENInst.ubImgMode);
	
    if(ubFlipEn)
        xtSENInst.ubImgMode |=  PS5268_FLIP;
    else
        xtSENInst.ubImgMode &=  ~PS5268_FLIP;
    
    bSEN_I2C_Write(0x01, 0x41, xtSENInst.ubImgMode);
    SEN_SetRawReorder(ubMirrorEn, ubFlipEn); 
}

//------------------------------------------------------------------------------
void SEN_GroupHoldOnVSync(void)
{
    bSEN_I2C_Write(0x30, 0x01, 0x1);
}

//------------------------------------------------------------------------------
void SEN_GroupHoldOffVSync(void)
{
    bSEN_I2C_Write(0x30, 0x01, 0x0);
}

//------------------------------------------------------------------------------
void SEN_SetSensorImageSize(void)
{
	// Image for ISP
	sensor_cfg.xtSENWin.uwHSize = ISP_WIDTH;
	sensor_cfg.xtSENWin.uwVSize = ISP_HEIGHT;
    sensor_cfg.xtSENWin.uwHStart = 0;
    sensor_cfg.xtSENWin.uwVStart = 2;
}

//------------------------------------------------------------------------------
void SEN_SetSensorType(void)
{
    sensor_cfg.ubSensorType = SEN_PS5268;	
    printf("sensor type is PS5268\r\n");
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
    uint16_t uwExpLine;
	uint8_t  ubGaintmp;
    uint32_t ulGainX1024;
    
    uwExpLine = ulExp;
    ulGainX1024 = ulGain/2;

    //EXPLINE
    bSEN_I2C_WriteTry(PS5268_FRAME_LINE_H, (uint8_t)(((uwExpLine)>>8)&0x00ff), TRY_COUNTS);	
    bSEN_I2C_WriteTry(PS5268_FRAME_LINE_L, (uint8_t)((uwExpLine)&0x00ff), TRY_COUNTS);  

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
	
	//Analog Gain
	bSEN_I2C_WriteTry(PS5268_GAIN, (ubGaintmp & 0xff), TRY_COUNTS);
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
