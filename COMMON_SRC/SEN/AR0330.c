/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		AR0330.c
	\brief		Sensor AR0330 relation function
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
#include "CQ_API.h"
#include "CQI2C.h"
#include "BSP.h"

#if (SEN_USE == SEN_AR0330)
struct SENSOR_SETTING sensor_cfg;
AE_EVENT_PROCESS tAE_SensorPara;
tfSENObj xtSENInst;
I2C1_Type *pI2C_type;
SEN_STATE sensor_state;

#if 0
const SENSOR_REG ubSEN_InitTable[] = {
	{0x3052,0xa114},	// reserved
	{0x304A,0x0070},	// reserved
	{0x31ae,0x0301},
	{0x301a,0x0058},
	{0x301a,0x0058},
	{0x301a,0x00d8},
	{0x301a,0x10d8},	// SET Parallel / stop streaming
    
	{0x3064,0x1802},	// SMIA_TEST
	{0x3078,0x0001},	// TEST_DATA_GREENB
	{0x3040,0x0000},
	{0x3046,0x4038},
	{0x3048,0x8480},
	{0x302a,0x0006},	// VT_PIX_CLK_DIV = 6
	{0x302c,0x0001},	// VT_SYS_CLK_DIV = 1
	{0x302e,0x0005},	// PRE_PLL_CLK_DIV = 5
	{0x3030,0x0064},	// PLL_MULTIPLIER = 100
	{0x3036,0x000c},	// OP_PIX_CLK_DIV = 12
	{0x3038,0x0001},	// OP_SYS_CLK_DIV = 1

#if 0     
	{0x300a,0x04a0},	// FRAME_LENGTH_LINES  = 1184
#else
    {0x300a,0x04ee},	// FRAME_LENGTH_LINES  = 1262
#endif    
	{0x300c,0x0420},	// LINE_LENGTH_PCK = 2112
	{0x3012,0x04a0},	// COARSE_INTEGRATION_TIME = 592
	{0x3014,0x0000},	// FINE_INTEGRATION_TIME = 0
 
#if 0    
	{0x3002,0x0000},	// Y_ADDR_START = 0
	{0x3004,0x00c6},	// X_ADDR_START = 198
	{0x3006,0x043f},	// Y_ADDR_END = 1087
	{0x3008,0x0845},	// X_ADDR_END = 2117
#else
 	{0x3002, 0x00e0},	// Y_ADDR_START = 224
	{0x3004, 0x00c0},	// X_ADDR_START = 192
	{0x3006, 0x051f},	// Y_ADDR_END = 1311
	{0x3008, 0x083F},	// X_ADDR_END = 2111   
#endif    
	{0x30a2,0x0001},	// X_ODD_INC
	{0x30a6,0x0001},	// Y_ODD_INC
	{0x301e,0x0020},	// a8,offset
	{0x306e,0xfc10},	// DATAPATH_SELECT
	{0x3040,0x0000},	// READ_MODE = 0
	{0x3042,0x039c},	// EXTRA_DELAY = 924
	{0x30ba,0x002c},	// DIGITAL_CTRL = 44
	{0x3088,0x80ba},	// SEQ_CTRL_PORT = 32954
	{0x3086,0x0253},	// SEQ_DATA_PORT = 595
	{0x3060,0x0010},	// set gain = 2x
};
#else 
const SENSOR_REG ubSEN_InitTable[] = {
//    {0x301A, 0x10DD},           // RESET_REGISTER
//    {0xfffe, 0x0064},           // delay 100ms
    {0x3052, 0xA114},           // reserved
    {0x304A, 0x0070},           // reserved
    {0x31AE, 0x0301},           // SERIAL_FORMAT
    {0xfffe, 0x0064},           // delay 100ms
    {0x31AC, 0x0a0a},           // 10bit
    {0x301A, 0x0058},           // RESET_REGISTER
    {0x301A, 0x0058},           // RESET_REGISTER
    {0xfffe, 0x000a},           // delay 10ms
    {0x301A, 0x00D8},           // RESET_REGISTER
    {0x301A, 0x10D8},           // RESET_REGISTER
    {0x3064, 0x1802},           // SMIA_TEST
    {0x3078, 0x0001},           // TEST_DATA_GREENB
    {0x3046, 0x4038},           // FLASH
    {0x3048, 0x8480},           // FLASH2

    {0x3180, 0x8029},           //


    {0x302A, 0x0006},           //VT_PIX_CLK_DIV = 6
    {0x302C, 0x0001},           //VT_SYS_CLK_DIV = 1
    {0x302E, 0x0005},           //PRE_PLL_CLK_DIV = 5
    {0x3030, 0x007B},           //PLL_MULTIPLIER = 124
    {0x3036, 0x000C},           //OP_PIX_CLK_DIV = 12
    {0x3038, 0x0001},           //OP_SYS_CLK_DIV = 1

    {0x300A, 0x0522},           //FRAME_LENGTH_LINES = 1324
    {0x300C, 0x04E0},           //LINE_LENGTH_PCK = 2496/2=1248
    {0x3012, 0x0496},           //COARSE_INTEGRATION_TIME = 293
    {0x3014, 0x0000},           //FINE_INTEGRATION_TIME = 0

    {0x30AA, 0x0522},           //FRAME_LENGTH_LINES_CB = 1314
    {0x303E, 0x04E0},           //LINE_LENGTH_PCK_CB = 1248
    {0x3016, 0x0496},           //COARSE_INTEGRATION_TIME_CB = 1313
    {0x3018, 0x0000},           //FINE_INTEGRATION_TIME_CB = 0
    
    {0x3042, 0x0080},           //EXTRA_DELAY = 981
    {0x3086, 0xE653},           //SEQ_DATA_PORT = 58963

    //window size
    {0x3002, 0x007E},           //Y_ADDR_START = 110
    {0x3004, 0x0006},           //X_ADDR_START = 6
    {0x3006, (0x058D+1)},       //Y_ADDR_END = 1421
    {0x3008, 0x0905},           //X_ADDR_END = 2309
    {0x30A2, 0x0001},           //X_ODD_INC = 1
    {0x30A6, 0x0001},           //Y_ODD_INC = 1
    
    {0x308C, 0x007E},           //Y_ADDR_START_CB = 110
    {0x308A, 0x0006},           //X_ADDR_START_CB = 6
    {0x3090, (0x058D+1)},       //Y_ADDR_END_CB = 1421
    {0x308E, 0x0905},           //X_ADDR_END_CB = 2309
    {0x30AE, 0x0001},           //X_ODD_INC_CB = 1
    {0x30A8, 0x0001},           //Y_ODD_INC_CB = 1
    // RGB Gain
    {0x305a, 0x0080},           //R Gain
    {0x3058, 0x0080},           //B Gain
    
    //---zjh 20131204
    {0x301e, 0x0020},//a8,offset
        
    //Max Slew on Pixel Clock, Max Slew on Data Interface
    {0x306E, 0xFC10},           // DATAPATH_SELECT
    {0x3040, 0x0000},           //READ_MODE = 0
    {0x30BA, 0x002C},           //DIGITAL_CTRL = 44
    {0x3088, 0x80BA},           //SEQ_CTRL_PORT = 32954
    {0x301A, 0x10DC},           // RESET_REGISTER

    {0x300A, 0x0522},           //FRAME_LENGTH_LINES = 1324
    {0x300C, 0x04E0},           //LINE_LENGTH_PCK = 2496/2=1248
    {0x3014, 0x0000},
    {0x3042, 0x0000},
    //{0x3040, 0xC000},
    {0x30BA, 0x002C},
    {0x301A, 0x10D8},
    {0x3088, 0x80BA},
    {0x3086, 0x0253},
    {0x301A, 0x00DC},
    {0x301A, 0x00DC},
    //{0x3012, 0x028C},
    {0x3012, 0x051C},

};
#endif

//------------------------------------------------------------------------------
bool bSEN_I2C_Read(uint16_t uwAddr, uint8_t *pValue)
{
	uint8_t *pAddr, pBuf[2];
	
	pAddr = (uint8_t*)&uwAddr;
	pBuf[0] = pAddr[1];
	pBuf[1] = pAddr[0];	
	
    #if (I2C_TYPE_SELECT == I2C_TYPE_POLLING)
        return bI2C_MasterProcess (pI2C_type, SEN_SLAVE_ADDR, &pBuf[0], 2, pValue, 2);
    #else
        return bI2C_MasterInt (pI2C_type, I2C_INT, SEN_SLAVE_ADDR, &pBuf[0], 2, pValue, 2);
    #endif
	
}

//------------------------------------------------------------------------------
bool bSEN_I2C_Write(uint8_t ubAddr1, uint8_t ubAddr2, uint8_t ubValue1, uint8_t ubValue2)
{	
	uint8_t pBuf[4];
	
	pBuf[0] = ubAddr1;
	pBuf[1] = ubAddr2;
	pBuf[2] = ubValue1;	
	pBuf[3] = ubValue2;	
			
    #if (I2C_TYPE_SELECT == I2C_TYPE_POLLING)
        return bI2C_MasterProcess (pI2C_type, SEN_SLAVE_ADDR, &pBuf[0], 4, NULL, 0);
    #else
        return bI2C_MasterInt (pI2C_type, I2C_INT, SEN_SLAVE_ADDR, &pBuf[0], 4, NULL, 0);
    #endif
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
    tAE_SensorPara.ulSensorPclk = 98000000;  
    tAE_SensorPara.ulSensorPixelPerLine = 2496;
    tAE_SensorPara.ulSensorLinePerFrame = 2112;
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
      
	pBuf = (uint8_t*)&ubPID; 
	// I2C by Read Sensor ID
    bSEN_I2C_Read (AR0330_CHIP_ID_ADDR, pBuf);
	if (AR0330_CHIP_ID != ubPID)
	{
		printd(DBG_ErrorLvl, "I2C read fail.<0x%x 0x%x>\n", AR0330_CHIP_ID, ubPID);
        return 0;
	}
    return 1;
}

//------------------------------------------------------------------------------
uint8_t ubSEN_SetSensorInitTable(void)
{
    I2C_SCL_SPEED_TYP tI2C_Clock = I2C_SCL_400K;
	uint16_t 	uwPID = 0;
	uint16_t	i;
	uint8_t     *pBuf;		
	
	// init I2C1
	pI2C_type = pI2C_MasterInit (I2C_2, tI2C_Clock);
	IQ_SetI2cType(pI2C_type, tI2C_Clock);
    //HW reset.
    SEN_SensorHwReset();
    
    if(ubSEN_CheckSensorState() != 1)
        return 0;
    printd(DBG_CriticalLvl, "AR0330 Sensor\n");
    
	// sensor soft reset	
	bSEN_I2C_Write(0x30, 0x1A, 0x10, 0xDD);
	// wait 150000 * (1/ExtClk) = 6.25ms		
	TIMER_Delay_us(10000);    
    for (i=0; i<(sizeof(ubSEN_InitTable)/sizeof(ubSEN_InitTable[0])); i++)
    {
        if(ubSEN_InitTable[i].uwAddress == 0xfffe)
        {
            TIMER_Delay_us((ubSEN_InitTable[i].uwData & 0xff) * 1000);
        }else{
            bSEN_I2C_Write((uint8_t)(ubSEN_InitTable[i].uwAddress >> 8) & 0xff, (uint8_t)(ubSEN_InitTable[i].uwAddress & 0xff),
                            (uint8_t)(ubSEN_InitTable[i].uwData >> 8) & 0xff, (uint8_t)(ubSEN_InitTable[i].uwData & 0xff));          
        }
    }

	bSEN_I2C_Write(0x30, 0x1A , 0x10 ,0xDC);
 	return 1;
}

//------------------------------------------------------------------------------
uint8_t ubSEN_Open(struct SENSOR_SETTING *setting)
{
    SEN_AeCbFunc();
    // Set ISP clock
    SEN_SetISPRate(3);
    SEN_SetAXIRate(3);
    // set Parallel mode
    SEN->MIPI_MODE = 0;
    // Set sensor clock
	// ExtClk = SenClk = 96M / 4 = 24MHz    
    SEN_SetSensorRate(SENSOR_96MHz, 4);	
    // enable sensor PCLK
    SEN->SEN_CLK_EN = 1;
    //Set ISP pipeline
    sensor_cfg.tPathType = BAYER_SENSOR_NORMAL;
    SEN_SenosrOutputType(sensor_cfg.tPathType);
    // Change ISP_LH_SEL to 0, for 10bit raw input
    SEN->ISP_LH_SEL = 1;
    // Timing
    SEN->VSYNC_RIS = 1;
    SEN->HSYNC_RIS = 1;
    SEN->VSYNC_HIGH = 1;
    SEN->PCK_DLH_RIS = 1;
    SEN->SYNC_MODE = 1;		
    SEN->SENSOR_MODE = 1;
    // change SN_SEN->RAW_REORDER to 0, start from B
    SEN->RAW_REORDER = 2;
    // set dummy line & pixel
    SEN->DMY_DIV = 4; // dummy clock=sysclk/2
    SEN->NUM_DMY_LN = 26;
    SEN->NUM_DMY_DSTB = 256;
    SEN->DMY_INSERT = 1;

    //delay 1ms
    TIMER_Delay_us(1000);		
    // Initial dummy sensor
    if (ubSEN_SetSensorInitTable() != 1)
    {
        printf("AR0330 startup failed! \n\r");
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

    //msut last beacause SEN_DSTB_TSEL is write only.
    SEN->SEN_DSTB_TSEL = 1;	

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
    // calc exposure lines    
	xtSENInst.xtSENCtl.uwExpLine = uwSEN_CalExpLine(ulAlgExpTime);

	if(xtSENInst.xtSENCtl.uwExpLine < 1)
	{
		xtSENInst.xtSENCtl.uwExpLine = 1;
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
	//remove
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
	uwDL = uwDL;	
}

//------------------------------------------------------------------------------
void SEN_SetExpLine(void)
{
    SEN_WrExpLine(xtSENInst.xtSENCtl.uwExpLine);
}

//------------------------------------------------------------------------------
void SEN_WrExpLine(uint16_t uwExpLine)
{
	//Set exposure line
	bSEN_I2C_Write(0x30, 0x12, (uint8_t)((uwExpLine>>8) & 0xff), (uint8_t)(uwExpLine & 0xff));
}

//------------------------------------------------------------------------------
void SEN_WrMaxExpLine(uint32_t ulValue)
{
    xtSENInst.uwMaxExpLine = ulValue;
}

//------------------------------------------------------------------------------
void SEN_WrGain(uint16_t uwGainX1024)
{
	uint16_t  uwAnalog_1st_gain, uwAnalog_2nd_gain;
    uint16_t  uwAnalogGain;
	uint16_t  uwDigitalGain;
        
    // update struct value.
    xtSENInst.xtSENCtl.uwGain = ulGainX1024;
	if (uwGainX1024 < 1024)			     //Limit min value
	{
		uwGainX1024 = 1024;
	} 

    if(uwGainX1024 < (2*1024))           // 1x~1.99x
    {
        uwAnalog_1st_gain = 0x0000;     
        uwAnalog_2nd_gain = 0;
        uwDigitalGain = (uwGainX1024 >> 3);
    }else if(uwGainX1024 < (4*1024)){    // 2x~3.99x
        uwAnalog_1st_gain = 0x0010;
        uwAnalog_2nd_gain = 0;
        uwDigitalGain = (uwGainX1024 >> 4);
    }else if(uwGainX1024 < (8*1024)){    // 4x~7.99x
        uwAnalog_1st_gain = 0x0020;     
        uwAnalog_2nd_gain = 0;
        uwDigitalGain = (uwGainX1024 >> 5);
    }else if(uwGainX1024 < (16*1024)){   // 8x~15.99x
        uwAnalog_1st_gain = 0x0030;    
        uwAnalog_2nd_gain = 0;
        uwDigitalGain = (uwGainX1024 >> 6);
    }else if(uwGainX1024 < (32*1024)){   // 16x~31.99x    
        uwAnalog_1st_gain = 0x0030;    
        uwAnalog_2nd_gain = 0;
        uwDigitalGain = (uwGainX1024 >> 6);
    }else{                               // 4x~7.99x
        uwAnalog_1st_gain = 0x0030;
        uwAnalog_2nd_gain = 0;
        uwDigitalGain = (uwGainX1024 >> 6);
    }
    // Set analog gain
    uwAnalogGain = (uwAnalog_1st_gain|uwAnalog_2nd_gain);
	bSEN_I2C_Write(0x30, 0x60, 0x00, (uwAnalogGain & 0x3F));
    
    //Set digital gain
    bSEN_I2C_Write(0x30, 0x5e, (uint8_t)((uwDigitalGain>>8) & 0xff), (uint8_t)(uwDigitalGain & 0xff));
}

//------------------------------------------------------------------------------
void SEN_SetMirrorFlip(uint8_t ubMirrorEn, uint8_t ubFlipEn)
{
    if(!ubIQ_GetLoadIqFinishFg())
        return;
    
    if(ubMirrorEn)
        xtSENInst.ubImgMode |=  AR0330_MIRROR;
    else
        xtSENInst.ubImgMode &=  ~AR0330_MIRROR;
    
    if(ubFlipEn)
        xtSENInst.ubImgMode |=  AR0330_FLIP;
    else
        xtSENInst.ubImgMode &=  ~AR0330_FLIP;
    
//    bSEN_I2C_Write(0x30, 0x40, xtSENInst.ubImgMode);
    //
//    SEN_SetRawReorder(ubMirrorEn, ubFlipEn);
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
	sensor_cfg.xtSENWin.uwVSize = (ISP_HEIGHT - 2);// for 3DNR bug
    sensor_cfg.xtSENWin.uwHStart = 0;
    sensor_cfg.xtSENWin.uwVStart = 2;
}

//------------------------------------------------------------------------------
void SEN_SetSensorType(void)
{
    sensor_cfg.ubSensorType = SEN_AR0330;
    printf("sensor type is AR0330\r\n");    
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
