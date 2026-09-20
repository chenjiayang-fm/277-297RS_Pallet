/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		IMX307.c
	\brief		Sensor IMX307 relation function
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

#if (SEN_USE == SEN_IMX307)
#define TRY_COUNTS 3
AE_EVENT_PROCESS tAE_SensorPara;
struct SENSOR_SETTING sensor_cfg;
tfSENObj xtSENInst;
I2C1_Type *pI2C_type;
SEN_STATE sensor_state;

const uint8_t ubSEN_InitTable[] = {
    /*
        If MCLK = 37.125MHz
        HMAX = 0x1130 = 4400, VMAX = 0x465 = 1125, fps = 30Hz
        PCLK = HMAX * VMAX * fps = 148500000 = 148.5MHz
        But SEN_CLK = 120MHz/3 = 40MHz
        HMAX = 0x1130 = 4400, VMAX = 0x465 = 1125, fps = 32.323Hz
        PCLK = HMAX * VMAX * fps = 148500000 = 160MHz
        ******
        Want fps = 30Hz,
        160MHz = HMAX * 1125 * 30 ==> HMAX = 4740 = 0x1284
        ******
        Sensor output window size
        X_OUT_SIZE = 1948
        Y_OUT_SIZE = 1097
    */
    //sensor stanby	
    0x83, 0x30, 0x00, 0x01,
    0x83, 0x30, 0x01, 0x01,
    0x83, 0x30, 0x02, 0x01,
    //setting
    0x83, 0x30, 0x05, 0x00,
    0x83, 0x30, 0x07, 0x00,
    0x83, 0x30, 0x09, 0x12,
    0x83, 0x30, 0x0A, 0x3C,
    0x83, 0x30, 0x0B, 0x00,
    0x83, 0x30, 0x11, 0x0A,
    0x83, 0x30, 0x12, 0x64,
    0x83, 0x30, 0x14, 0x00,
    //VMAX = 0x465 = 1125
    0x83, 0x30, 0x18, 0x65,
    0x83, 0x30, 0x19, 0x04,
    0x83, 0x30, 0x1A, 0x00,
    //HMAX = 0x1130 = 4400
//    0x83, 0x30, 0x1C, 0x30,
//    0x83, 0x30, 0x1D, 0x11,
    0x83, 0x30, 0x1C, 0x80,//0x84
    0x83, 0x30, 0x1D, 0x12,
    0x83, 0x30, 0x20, 0xFE,
    0x83, 0x30, 0x21, 0x03,
    0x83, 0x30, 0x22, 0x00,
    0x83, 0x30, 0x46, 0x00,
    0x83, 0x30, 0x48, 0x00,
    0x83, 0x30, 0x49, 0x08,
    0x83, 0x30, 0x4B, 0x0A,
    0x83, 0x30, 0x5C, 0x18,
    0x83, 0x30, 0x5D, 0x03,
    0x83, 0x30, 0x5E, 0x20,
    0x83, 0x30, 0x5F, 0x01,
    0x83, 0x30, 0x9E, 0x4A,
    0x83, 0x30, 0x9F, 0x4A,

    0x83, 0x31, 0x1C, 0x0E,
    0x83, 0x31, 0x28, 0x04,
    0x83, 0x31, 0x29, 0x1D,
    0x83, 0x31, 0x3B, 0x41,
    0x83, 0x31, 0x5E, 0x1A,
    0x83, 0x31, 0x64, 0x1A,
    0x83, 0x31, 0x7C, 0x12,
    0x83, 0x31, 0xEC, 0x37,

    //These re, gisters are set in CSI-2 interface only.	
    0x83, 0x34, 0x05, 0x10,
    0x83, 0x34, 0x07, 0x01,
    //OPB_SIZE_V = 0x0A = 10
    0x83, 0x34, 0x14, 0x0A,
    //Y_OUT_SIZE = 0x0449 = 1097
    0x83, 0x34, 0x18, 0x49,
    0x83, 0x34, 0x19, 0x04,
    0x83, 0x34, 0x41, 0x0A,
    0x83, 0x34, 0x42, 0x0A,
    0x83, 0x34, 0x43, 0x01,
    0x83, 0x34, 0x44, 0x20,
    0x83, 0x34, 0x45, 0x25,
    0x83, 0x34, 0x46, 0x57,
    0x83, 0x34, 0x47, 0x00,
    0x83, 0x34, 0x48, 0x37,
    0x83, 0x34, 0x49, 0x00,
    0x83, 0x34, 0x4A, 0x1F,
    0x83, 0x34, 0x4B, 0x00,
    0x83, 0x34, 0x4C, 0x1F,
    0x83, 0x34, 0x4D, 0x00,
    0x83, 0x34, 0x4E, 0x1F,
    0x83, 0x34, 0x4F, 0x00,
    0x83, 0x34, 0x50, 0x77,
    0x83, 0x34, 0x51, 0x00,
    0x83, 0x34, 0x52, 0x1F,
    0x83, 0x34, 0x53, 0x00,
    0x83, 0x34, 0x54, 0x17,
    0x83, 0x34, 0x55, 0x00,
    //X_OUT_SIZE = 0x079C = 1948
    0x83, 0x34, 0x72, 0x9C,
    0x83, 0x34, 0x73, 0x07,
    0x83, 0x34, 0x80, 0x49,
    //stanby cancel	
    0x83, 0x30, 0x00, 0x00,
    0x83, 0x30, 0x01, 0x00,
    0x83, 0x30, 0x02, 0x00,
};

#define cbAE_SensorVTblColSZ		(sizeof(ctSensor_SensorVTbl[0]))
#define cbAE_SensorVTblRowSZ		(sizeof(ctSensor_SensorVTbl)/sizeof(ctSensor_SensorVTbl[0]))
    
struct Sen_GainTblObj{
	uint32_t    ulAlgGain;
	uint8_t     ubSenGain;
};

struct Sen_GainTblObj ctSensor_SensorVTbl[] = {
    {    512,	  0},
    {    530,	  1},
    {    549,	  2},
    {    568,	  3},
    {    588,	  4},
    {    609,	  5},
    {    630,	  6},
    {    652,	  7},
    {    675,	  8},
    {    699,	  9},
    {    723,	 10},
    {    749,	 11},
    {    775,	 12},
    {    802,	 13},
    {    830,	 14},
    {    860,	 15},
    {    890,	 16},
    {    921,	 17},
    {    953,	 18},
    {    987,	 19},
    {   1022,	 20},
    {   1057,	 21},
    {   1095,	 22},
    {   1133,	 23},
    {   1173,	 24},
    {   1214,	 25},
    {   1257,	 26},
    {   1301,	 27},
    {   1347,	 28},
    {   1394,	 29},
    {   1443,	 30},
    {   1494,	 31},
    {   1546,	 32},
    {   1601,	 33},
    {   1657,	 34},
    {   1715,	 35},
    {   1775,	 36},
    {   1838,	 37},
    {   1902,	 38},
    {   1969,	 39},
    {   2038,	 40},
    {   2110,	 41},
    {   2184,	 42},
    {   2261,	 43},
    {   2340,	 44},
    {   2423,	 45},
    {   2508,	 46},
    {   2596,	 47},
    {   2687,	 48},
    {   2781,	 49},
    {   2879,	 50},
    {   2980,	 51},
    {   3085,	 52},
    {   3194,	 53},
    {   3306,	 54},
    {   3422,	 55},
    {   3542,	 56},
    {   3667,	 57},
    {   3796,	 58},
    {   3929,	 59},
    {   4067,	 60},
    {   4210,	 61},
    {   4358,	 62},
    {   4511,	 63},
    {   4669,	 64},
    {   4834,	 65},
    {   5003,	 66},
    {   5179,	 67},
    {   5361,	 68},
    {   5550,	 69},
    {   5745,	 70},
    {   5947,	 71},
    {   6156,	 72},
    {   6372,	 73},
    {   6596,	 74},
    {   6828,	 75},
    {   7068,	 76},
    {   7316,	 77},
    {   7573,	 78},
    {   7839,	 79},
    {   8115,	 80},
    {   8400,	 81},
    {   8695,	 82},
    {   9001,	 83},
    {   9317,	 84},
    {   9644,	 85},
    {   9983,	 86},
    {  10334,	 87},
    {  10697,	 88},
    {  11073,	 89},
    {  11462,	 90},
    {  11865,	 91},
    {  12282,	 92},
    {  12714,	 93},
    {  13160,	 94},
    {  13623,	 95},
    {  14102,	 96},
    {  14597,	 97},
    {  15110,	 98},
    {  15641,	 99},
    {  16191,	100},
    {  16760,	101},
    {  17349,	102},
    {  17958,	103},
    {  18590,	104},
    {  19243,	105},
    {  19919,	106},
    {  20619,	107},
    {  21344,	108},
    {  22094,	109},
    {  22870,	110},
    {  23674,	111},
    {  24506,	112},
    {  25367,	113},
    {  26259,	114},
    {  27181,	115},
    {  28136,	116},
    {  29125,	117},
    {  30149,	118},
    {  31208,	119},
    {  32305,	120},
    {  33440,	121},
    {  34615,	122},
    {  35832,	123},
    {  37091,	124},
    {  38395,	125},
    {  39744,	126},
    {  41141,	127},
    {  42586,	128},
    {  44083,	129},
    {  45632,	130},
    {  47236,	131},
    {  48896,	132},
    {  50614,	133},
    {  52393,	134},
    {  54234,	135},
    {  56140,	136},
    {  58113,	137},
    {  60155,	138},
    {  62269,	139},
    {  64457,	140},
    {  66722,	141},
    {  69067,	142},
    {  71494,	143},
    {  74007,	144},
    {  76607,	145},
    {  79299,	146},
    {  82086,	147},
    {  84971,	148},
    {  87957,	149},
    {  91048,	150},
    {  94248,	151},
    {  97560,	152},
    { 100988,	153},
    { 104537,	154},
    { 108211,	155},
    { 112013,	156},
    { 115950,	157},
    { 120025,	158},
    { 124242,	159},
    { 128609,	160},
    { 133128,	161},
    { 137807,	162},
    { 142649,	163},
    { 147662,	164},
    { 152852,	165},
    { 158223,	166},
    { 163783,	167},
    { 169539,	168},
    { 175497,	169},
    { 181664,	170},
    { 188049,	171},
    { 194657,	172},
    { 201498,	173},
    { 208579,	174},
    { 215909,	175},
    { 223496,	176},
    { 231350,	177},
    { 239480,	178},
    { 247896,	179},
    { 256608,	180},
    { 265626,	181},
    { 274960,	182},
    { 284623,	183},
    { 294625,	184},
    { 304979,	185},
    { 315697,	186},
    { 326791,	187},
    { 338275,	188},
    { 350163,	189},
    { 362468,	190},
    { 375206,	191},
    { 388392,	192},
    { 402041,	193},
    { 416169,	194},
    { 430794,	195},
    { 445933,	196},
    { 461604,	197},
    { 477826,	198},
    { 494618,	199},
    { 512000,	200},
    { 529993,	201},
    { 548618,	202},
    { 567898,	203},
    { 587855,	204},
    { 608513,	205},
    { 629898,	206},
    { 652034,	207},
    { 674947,	208},
    { 698667,	209},
    { 723219,	210},
    { 748635,	211},
    { 774943,	212},
    { 802177,	213},
    { 830367,	214},
    { 859548,	215},
    { 889754,	216},
    { 921022,	217},
    { 953389,	218},
    { 986893,	219},
    {1021574,	220},
    {1057475,	221},
    {1094637,	222},
    {1133104,	223},
    {1172924,	224},
    {1214143,	225},
    {1256811,	226},
    {1300978,	227},
    {1346697,	228},
    {1394023,	229},
    {1443012,	230},
    {1493723,	231},
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
    tAE_SensorPara.ulSensorPclk = 79650000;  
    tAE_SensorPara.ulSensorPixelPerLine = 2360;
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

//------------------------------------------------------------------------------//
//                         Power down timing selection                          //
//------------------------------------------------------------------------------//
//------------------------------------------------------------------------------
void MIPI_PowerDownTimingSelect(uint8_t ubPclkIdx)
{	
	printf("-------------------------------\n");
	switch (ubPclkIdx)
	{
        //T_term-pd = 140ns (by oscilloscope)
        //DLAN_PDD_SEL = (T_term-pd / 16.67ns) = 8.398        
		case SEN_FPS25:
			MIPI->DLAN_PDD_LSCALE_SEL0 = 7;	
			break;       
		case SEN_FPS30:
			MIPI->DLAN_PDD_LSCALE_SEL0 = 7;
			break;
		default:
			MIPI->DLAN_PDD_LSCALE_SEL0 = 7;
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
	uint8_t 	ubPID = 0;
      
	pBuf = (uint8_t*)&ubPID; 
	// I2C by Read Sensor ID
    bSEN_I2C_Read (IMX307_CHIP_MODELID_ADDR, &pBuf[0]);
	if (IMX307_CHIP_MODELID != ubPID)
	{
		printd(DBG_ErrorLvl, "I2C read fail.<0x%x 0x%x>\n", IMX307_CHIP_MODELID, ubPID);
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
    printd(DBG_CriticalLvl, "IMX307 Sensor\n");

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
    bSEN_I2C_Write(0x30, 0x07, 0x3);
    bSEN_I2C_Write(0x30, 0x01, 0x0);  
    SEN_PclkSetting(SEN_FPS30);
    
	return 1;
}

//------------------------------------------------------------------------------
uint8_t ubSEN_Open(struct SENSOR_SETTING *setting)
{
    SEN_AeCbFunc();
    // Set ISP clock
    SEN_SetISPRate(5);		
    // Set parallel mode
    SEN->MIPI_MODE = 1;
    // Power down MIPI analog phy.
    MIPI->MIPI_PD = 0;
    // Set sensor clock
    SEN_SetSensorRate(SENSOR_120MHz, 3);	
    // Enable sensor PCLK
    SEN->SEN_CLK_EN = 1;
    //Set ISP pipeline
    sensor_cfg.tPathType = BAYER_SENSOR_NORMAL;
    SEN_SenosrOutputType(sensor_cfg.tPathType);
    // change ISP_LH_SEL to 1, for D2-D9 input
    SEN->ISP_LH_SEL = 0;
    // change SN_SEN->RAW_REORDER to 0, start from B
    SEN->RAW_REORDER = 3;
    // Set dummy line & pixel
    SEN->DMY_DIV = 3;
    SEN->NUM_DMY_LN = 16;
    SEN->NUM_DMY_DSTB = 256;
    SEN->DMY_INSERT = 1;
    
    // Initial dummy sensor
    if (ubSEN_SetSensorInitTable() != 1)
    {
        printf("IMX307 startup failed! \n\r");
        return 0;
    }
    // Enable MIPI and set MIPI channel select.
    SEN_MIPIControl(MIPI_2LANE, MIPI_CHANNEL0);    
    
	MIPI_PowerDownTimingSelect(SEN_FPS30);
	MIPI_AutoPhaseDetect();
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
	static uint16_t uwOldDummy = 0;
    if(uwDL == uwOldDummy)  return;
    //Set Dummy Line
    bSEN_I2C_WriteTry(IMX307_FRAME_LENGTH_M, (uint8_t)((uwDL>>8) & 0x00ff), TRY_COUNTS);		
    bSEN_I2C_WriteTry(IMX307_FRAME_LENGTH_L, (uint8_t)((uwDL) & 0x00ff), TRY_COUNTS);	   
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
    bSEN_I2C_WriteTry(IMX307_DUMMY_LINE_M, (uint8_t)(((uwTargetExpLine)>>8)&0x00ff), TRY_COUNTS);	
    bSEN_I2C_WriteTry(IMX307_DUMMY_LINE_L, (uint8_t)((uwTargetExpLine)&0x00ff), TRY_COUNTS);     
    
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
		if(ulGainX1024 <= ctSensor_SensorVTbl[ubIdx].ulAlgGain)
		{
			xtSENInst.ubBuf[2] = ctSensor_SensorVTbl[ubIdx].ubSenGain;
			break;
		}
	}

	if(ubIdx == cbAE_SensorVTblRowSZ)
	{
		xtSENInst.ubBuf[2] = ctSensor_SensorVTbl[ubIdx-1].ubSenGain;		
	}
    
    bSEN_I2C_WriteTry(IMX307_GAIN, xtSENInst.ubBuf[2], TRY_COUNTS);
    //save gain value
    ulOldGainValue = ulGainX1024;
}

//------------------------------------------------------------------------------
void SEN_SetMirrorFlip(uint8_t ubMirrorEn, uint8_t ubFlipEn)
{
    if(!ubIQ_GetLoadIqFinishFg())
        return;
    
    if(ubMirrorEn)
        xtSENInst.ubImgMode |=  IMX307_MIRROR;
    else
        xtSENInst.ubImgMode &=  ~IMX307_MIRROR;
    
    if(ubFlipEn)
        xtSENInst.ubImgMode |=  IMX307_FLIP;
    else
        xtSENInst.ubImgMode &=  ~IMX307_FLIP;
    
    bSEN_I2C_Write(0x30, 0x07, xtSENInst.ubImgMode);
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
    sensor_cfg.xtSENWin.uwVStart = 0;
}

//------------------------------------------------------------------------------
void SEN_SetSensorType(void)
{
    sensor_cfg.ubSensorType = SEN_IMX307;	
    printf("sensor type is IMX307\r\n");
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
    bSEN_I2C_WriteTry(IMX307_DUMMY_LINE_M, (uint8_t)(((uwExpLine)>>8)&0x00ff), TRY_COUNTS);	
    bSEN_I2C_WriteTry(IMX307_DUMMY_LINE_L, (uint8_t)((uwExpLine)&0x00ff), TRY_COUNTS);  

    //GAIN
	for(ubIdx = 0; ubIdx<cbAE_SensorVTblRowSZ; ubIdx++)
	{
		if(ulGainX1024 <= ctSensor_SensorVTbl[ubIdx].ulAlgGain)
		{
			xtSENInst.ubBuf[2] = ctSensor_SensorVTbl[ubIdx].ubSenGain;
			break;
		}
	}

	if(ubIdx == cbAE_SensorVTblRowSZ)
	{
		xtSENInst.ubBuf[2] = ctSensor_SensorVTbl[ubIdx-1].ubSenGain;		
	}
    
    bSEN_I2C_WriteTry(IMX307_GAIN, xtSENInst.ubBuf[2], TRY_COUNTS);
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
