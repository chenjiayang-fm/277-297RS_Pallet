/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		SC230AI.c
	\brief		Sensor SC230AI relation function
	\author		Wenwei
	\version	2.0
	\date		2021-9-1
	\copyright	Copyright(C) 2021 SONiX Technology Co.,Ltd. All rights reserved.
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



#if (SEN_USE == SEN_SC230AI)
#define TRY_COUNTS 3
AE_EVENT_PROCESS tAE_SensorPara;
struct SENSOR_SETTING sensor_cfg;
tfSENObj xtSENInst;
I2C1_Type *pI2C_type;
SEN_STATE sensor_state;
uint16_t ExpLine;
uint32_t ExpGain;
uint8_t ExpGain462=50;

const uint8_t ubSEN_InitTable[] = {
	
	0x83,0x01,0x03,0x01,
	0x83,0x01,0x00,0x00,
	0x83,0x36,0xe9,0x80,
	0x83,0x37,0xf9,0x80,
	0x83,0x30,0x1f,0x07,
	0x83,0x32,0x0c,0x09,
	0x83,0x32,0x0d,0x60,
	
	//0x83,0x32,0x00,0x00,
	//0x83,0x32,0x01,0x08,
	//0x83,0x32,0x02,0x00,
	//0x83,0x32,0x03,0x10,
	//0x83,0x32,0x04,0x07,
	//0x83,0x32,0x05,0xa7,
	//0x83,0x32,0x06,0x04,
	//0x83,0x32,0x07,0x57,
	//0x83,0x32,0x08,0x07,
	//0x83,0x32,0x09,0x88,
	//0x83,0x32,0x0a,0x04,
	//0x83,0x32,0x0b,0x40,
	//0x83,0x32,0x10,0x00,
	//0x83,0x32,0x11,0x0c,
	//0x83,0x32,0x12,0x00,
	//0x83,0x32,0x13,0x04,
	0x83,0x32,0x81,0x00, //brian add HDR en

	0x83,0x33,0x01,0x07,
	0x83,0x33,0x04,0x50,
	0x83,0x33,0x06,0x70,
	0x83,0x33,0x08,0x18,
	0x83,0x33,0x09,0x68,
	0x83,0x33,0x0a,0x01,
	0x83,0x33,0x0b,0x20,
	0x83,0x33,0x1e,0x41,
	0x83,0x33,0x1f,0x59,
	0x83,0x33,0x33,0x10,
	0x83,0x33,0x34,0x40,
	0x83,0x33,0x5d,0x60,
	0x83,0x33,0x5e,0x06,
	0x83,0x33,0x5f,0x08,
	0x83,0x33,0x64,0x5e,
	0x83,0x33,0x7c,0x02,
	0x83,0x33,0x7d,0x0a,
	0x83,0x33,0x90,0x01,
	0x83,0x33,0x91,0x0b,
	0x83,0x33,0x92,0x0f,
	0x83,0x33,0x93,0x09,
	0x83,0x33,0x94,0x0d,
	0x83,0x33,0x95,0x60,
	0x83,0x33,0x96,0x48,
	0x83,0x33,0x97,0x49,
	0x83,0x33,0x98,0x4b,
	0x83,0x33,0x99,0x07,
	0x83,0x33,0x9a,0x0a,
	0x83,0x33,0x9b,0x0d,
	0x83,0x33,0x9c,0x60,
	0x83,0x33,0xa2,0x04,
	0x83,0x33,0xaf,0x40,
	0x83,0x33,0xb1,0x80,
	0x83,0x33,0xb3,0x40,
	0x83,0x33,0xb9,0x0a,
	0x83,0x33,0xf9,0xa0,
	0x83,0x33,0xfb,0xbf,
	0x83,0x33,0xfc,0x5f,
	0x83,0x33,0xfd,0x7f,
	0x83,0x34,0x9f,0x03,
	0x83,0x34,0xa6,0x4b,
	0x83,0x34,0xa7,0x5f,
	0x83,0x34,0xa8,0x30,
	0x83,0x34,0xa9,0x20,
	0x83,0x34,0xaa,0x01,
	0x83,0x34,0xab,0x28,
	0x83,0x34,0xac,0x01,
	0x83,0x34,0xad,0x58,
	0x83,0x34,0xf8,0x7f,
	0x83,0x34,0xf9,0x10,
	0x83,0x36,0x30,0xc0,
	0x83,0x36,0x33,0x44,
	0x83,0x36,0x3b,0x20,
	0x83,0x36,0x70,0x09,
	0x83,0x36,0x74,0xb0,
	0x83,0x36,0x75,0x80,
	0x83,0x36,0x76,0x88,
	0x83,0x36,0x7c,0x40,
	0x83,0x36,0x7d,0x49,
	0x83,0x36,0x90,0x44,
	0x83,0x36,0x91,0x33,
	0x83,0x36,0x92,0x43,
	0x83,0x36,0x9c,0x49,
	0x83,0x36,0x9d,0x4f,
	0x83,0x36,0xae,0x4b,
	0x83,0x36,0xaf,0x4f,
	0x83,0x36,0xb0,0x87,
	0x83,0x36,0xb1,0x94,
	0x83,0x36,0xb2,0xbc,
	0x83,0x36,0xd0,0x01,
	0x83,0x36,0xea,0x09,
	0x83,0x36,0xeb,0x0c,
	0x83,0x36,0xec,0x1c,
	0x83,0x36,0xed,0x24,
	0x83,0x37,0x22,0x97,
	0x83,0x37,0x28,0x90,
	0x83,0x37,0xfa,0x09,
	0x83,0x37,0xfb,0x32,
	0x83,0x37,0xfc,0x10,
	0x83,0x37,0xfd,0x34,
	0x83,0x39,0x01,0x02,
	0x83,0x39,0x02,0xc5,
	0x83,0x39,0x04,0x04,
	0x83,0x39,0x07,0x00,
	0x83,0x39,0x08,0x41,
	0x83,0x39,0x09,0x00,
	0x83,0x39,0x0a,0x00,
	0x83,0x39,0x33,0x84,
	0x83,0x39,0x34,0x10,
	0x83,0x39,0x40,0x78,
	0x83,0x39,0x42,0x04,
	0x83,0x39,0x43,0x11,
	0x83,0x3e,0x00,0x00,
	0x83,0x3e,0x01,0x8c,
	0x83,0x3e,0x02,0x20,
	0x83,0x44,0x0e,0x02,
	0x83,0x48,0x19,0x06,
	0x83,0x48,0x1b,0x03,
	0x83,0x48,0x1d,0x0b,
	0x83,0x48,0x1f,0x03,
	0x83,0x48,0x21,0x08,
	0x83,0x48,0x23,0x03,
	0x83,0x48,0x25,0x03,
	0x83,0x48,0x27,0x03,
	0x83,0x48,0x29,0x05,
	0x83,0x50,0x10,0x01,
	0x83,0x57,0x87,0x08,
	0x83,0x57,0x88,0x03,
	0x83,0x57,0x89,0x00,
	0x83,0x57,0x8a,0x10,
	0x83,0x57,0x8b,0x08,
	0x83,0x57,0x8c,0x00,
	0x83,0x57,0x90,0x08,
	0x83,0x57,0x91,0x04,
	0x83,0x57,0x92,0x00,
	0x83,0x57,0x93,0x10,
	0x83,0x57,0x94,0x08,
	0x83,0x57,0x95,0x00,
	0x83,0x57,0x99,0x06,
	0x83,0x57,0xad,0x00,
	0x83,0x5a,0xe0,0xfe,
	0x83,0x5a,0xe1,0x40,
	0x83,0x5a,0xe2,0x3f,
	0x83,0x5a,0xe3,0x38,
	0x83,0x5a,0xe4,0x28,
	0x83,0x5a,0xe5,0x3f,
	0x83,0x5a,0xe6,0x38,
	0x83,0x5a,0xe7,0x28,
	0x83,0x5a,0xe8,0x3f,
	0x83,0x5a,0xe9,0x3c,
	0x83,0x5a,0xea,0x2c,
	0x83,0x5a,0xeb,0x3f,
	0x83,0x5a,0xec,0x3c,
	0x83,0x5a,0xed,0x2c,
	0x83,0x5a,0xf4,0x3f,
	0x83,0x5a,0xf5,0x38,
	0x83,0x5a,0xf6,0x28,
	0x83,0x5a,0xf7,0x3f,
	0x83,0x5a,0xf8,0x38,
	0x83,0x5a,0xf9,0x28,
	0x83,0x5a,0xfa,0x3f,
	0x83,0x5a,0xfb,0x3c,
	0x83,0x5a,0xfc,0x2c,
	0x83,0x5a,0xfd,0x3f,
	0x83,0x5a,0xfe,0x3c,
	0x83,0x5a,0xff,0x2c,
	0x83,0x36,0xe9,0x53,
	0x83,0x37,0xf9,0x53,
	0x83,0x01,0x00,0x01,	
};

#define cbAE_SensorVTblColSZ		(sizeof(ctSC230_GainTbl[0]))
#define cbAE_SensorVTblRowSZ		(sizeof(ctSC230_GainTbl)/sizeof(ctSC230_GainTbl[0]))
    
struct Sen_GainTblObj{
	uint32_t    ulGainLevelTable;
	uint8_t     ubAGain;
	uint8_t     ubDCoarseGain;
	uint8_t     ubDFineGain;
};

struct Sen_GainTblObj ctSC230_GainTbl[] = {
		{    64 ,	  0x0,	0x7,	0x80},//1x
		{    66 ,	  0x0,	0x7,	0x84},//
		{    68 ,	  0x0,	0x7,	0x88},//
		{    70 ,	  0x0,	0x7,	0x8c},//
		{    72 ,	  0x0,	0x7,	0x90},//
		{    74 ,	  0x0,	0x7,	0x94},//
		{    76 ,	  0x0,	0x7,	0x98},//
		{    78 ,	  0x0,	0x7,	0x9c},//
		{    80 ,	  0x0,	0x7,	0xa0},//
		{    82 ,	  0x0,	0x7,	0xa4},//
		{    84 ,	  0x0,	0x7,	0xa8},//
		{    86 ,	  0x0,	0x7,	0xac},//
		{    88 ,	  0x0,	0x7,	0xb0},//
		{    90 ,	  0x0,	0x7,	0xb4},//
		{    92 ,	  0x0,	0x7,	0xb8},//
		{    94 ,	  0x0,	0x7,	0xbc},//
		{    96 ,	  0x0,	0x7,	0xc0},//
		{    98 ,	  0x0,	0x7,	0xc4},//
		{    100,	  0x0,	0x7,	0xc8},//
		{    102,	  0x0,	0x7,	0xcc},//
		{    104,	  0x0,	0x7,	0xd0},//
		{    106,	  0x0,	0x7,	0xd4},//
		{    108,	  0x0,	0x7,	0xd8},//
		{    110,	  0x0,	0x7,	0xdc},//
		{    112,	  0x0,	0x7,	0xe0},//
		{    114,	  0x0,	0x7,	0xe4},//
		{    116,	  0x0,	0x7,	0xe8},//
		{    118,	  0x0,	0x7,	0xec},//
		{    120,	  0x0,	0x7,	0xf0},//
		{    122,	  0x0,	0x7,	0xf4},//
		{    124,	  0x0,	0x7,	0xf8},//
		{    126,	  0x0,	0x7,	0xfc},//
		{    128,	  0x01,	0x7,	0x80},//2x
		{    132,	  0x01,	0x7,	0x84},//
		{    136,	  0x01,	0x7,	0x88},//
		{    140,	  0x01,	0x7,	0x8c},//
		{    144,	  0x01,	0x7,	0x90},//
		{    148,	  0x01,	0x7,	0x94},//
		{    152,	  0x01,	0x7,	0x98},//
		{    156,	  0x01,	0x7,	0x9c},//
		{    160,	  0x01,	0x7,	0xa0},//
		{    164,	  0x01,	0x7,	0xa4},//
		{    168,	  0x01,	0x7,	0xa8},//
		{    172,	  0x01,	0x7,	0xac},//
		{    176,	  0x01,	0x7,	0xb0},//
		{    180,	  0x01,	0x7,	0xb4},//
		{    184,	  0x01,	0x7,	0xb8},//
		{    188,	  0x01,	0x7,	0xbc},//
		{    192,	  0x01,	0x7,	0xc0},//
		{    196,	  0x01,	0x7,	0xc4},//
		{    200,	  0x01,	0x7,	0xc8},//
		{    204,	  0x01,	0x7,	0xcc},//
		{    208,	  0x01,	0x7,	0xd0},//
		{    212,	  0x01,	0x7,	0xd4},//
		{    216,	  0x01,	0x7,	0xd8},//
//		{    220,	  0x01,	0x7,	0xdc},//
//		{    224,	  0x01,	0x7,	0xe0},//
//		{    228,	  0x01,	0x7,	0xe4},//
//		{    232,	  0x01,	0x7,	0xe8},//
//		{    236,	  0x01,	0x7,	0xec},//
//		{    240,	  0x01,	0x7,	0xf0},//
//		{    244,	  0x01,	0x7,	0xf4},//
//		{    248,	  0x01,	0x7,	0xf8},//
//		{    252,	  0x01,	0x7,	0xfc},//
		{    217,	  0x40,	0x7,	0x80},//3.391x
		{    224,	  0x40,	0x7,	0x84},//
		{    230,	  0x40,	0x7,	0x88},//
		{    237,	  0x40,	0x7,	0x8c},//
		{    244,	  0x40,	0x7,	0x90},//
		{    250,	  0x40,	0x7,	0x94},//
		{    257,	  0x40,	0x7,	0x98},//
		{    264,	  0x40,	0x7,	0x9c},//
		{    271,	  0x40,	0x7,	0xa0},//
		{    278,	  0x40,	0x7,	0xa4},//
		{    284,	  0x40,	0x7,	0xa8},//
		{    291,	  0x40,	0x7,	0xac},//
		{    298,	  0x40,	0x7,	0xb0},//
		{    305,	  0x40,	0x7,	0xb4},//
		{    311,	  0x40,	0x7,	0xb8},//
		{    318,	  0x40,	0x7,	0xbc},//
		{    325,	  0x40,	0x7,	0xc0},//
		{    332,	  0x40,	0x7,	0xc4},//
		{    339,	  0x40,	0x7,	0xc8},//
		{    345,	  0x40,	0x7,	0xcc},//
		{    352,	  0x40,	0x7,	0xd0},//
		{    359,	  0x40,	0x7,	0xd4},//
		{    366,	  0x40,	0x7,	0xd8},//
		{    373,	  0x40,	0x7,	0xdc},//
		{    379,	  0x40,	0x7,	0xe0},//
		{    386,	  0x40,	0x7,	0xe4},//
		{    393,	  0x40,	0x7,	0xe8},//
		{    400,	  0x40,	0x7,	0xec},//
		{    406,	  0x40,	0x7,	0xf0},//
		{    413,	  0x40,	0x7,	0xf4},//
		{    420,	  0x40,	0x7,	0xf8},//
		{    427,	  0x40,	0x7,	0xfc},//
		{    434,	  0x48,	0x7,	0x80},//6.782x
		{    448,	  0x48,	0x7,	0x84},//
		{    461,	  0x48,	0x7,	0x88},//
		{    474,	  0x48,	0x7,	0x8c},//
		{    488,	  0x48,	0x7,	0x90},//
		{    501,	  0x48,	0x7,	0x94},//
		{    515,	  0x48,	0x7,	0x98},//
		{    528,	  0x48,	0x7,	0x9c},//
		{    542,	  0x48,	0x7,	0xa0},//
		{    556,	  0x48,	0x7,	0xa4},//
		{    569,	  0x48,	0x7,	0xa8},//
		{    583,	  0x48,	0x7,	0xac},//
		{    596,	  0x48,	0x7,	0xb0},//
		{    610,	  0x48,	0x7,	0xb4},//
		{    623,	  0x48,	0x7,	0xb8},//
		{    637,	  0x48,	0x7,	0xbc},//
		{    651,	  0x48,	0x7,	0xc0},//
		{    664,	  0x48,	0x7,	0xc4},//
		{    678,	  0x48,	0x7,	0xc8},//
		{    691,	  0x48,	0x7,	0xcc},//
		{    705,	  0x48,	0x7,	0xd0},//
		{    718,	  0x48,	0x7,	0xd4},//
		{    732,	  0x48,	0x7,	0xd8},//
		{    746,	  0x48,	0x7,	0xdc},//
		{    759,	  0x48,	0x7,	0xe0},//
		{    773,	  0x48,	0x7,	0xe4},//
		{    786,	  0x48,	0x7,	0xe8},//
		{    800,	  0x48,	0x7,	0xec},//
		{    813,	  0x48,	0x7,	0xf0},//
		{    827,	  0x48,	0x7,	0xf4},//
		{    840,	  0x48,	0x7,	0xf8},//
		{    854,	  0x48,	0x7,	0xfc},//
		{    868 ,	  0x49,	0x7,	0x80},//13.564x
		{    895 ,	  0x49,	0x7,	0x84},//    
		{    922 ,	  0x49,	0x7,	0x88},//    
		{    949 ,	  0x49,	0x7,	0x8c},//    
		{    976 ,	  0x49,	0x7,	0x90},//    
		{    1003,	  0x49,	0x7,	0x94},//    
		{    1030,	  0x49,	0x7,	0x98},//    
		{    1057,	  0x49,	0x7,	0x9c},//    
		{    1085,	  0x49,	0x7,	0xa0},//    
		{    1112,	  0x49,	0x7,	0xa4},//    
		{    1139,	  0x49,	0x7,	0xa8},//    
		{    1166,	  0x49,	0x7,	0xac},//    
		{    1193,	  0x49,	0x7,	0xb0},//    
		{    1220,	  0x49,	0x7,	0xb4},//    
		{    1247,	  0x49,	0x7,	0xb8},//    
		{    1275,	  0x49,	0x7,	0xbc},//    
		{    1302,	  0x49,	0x7,	0xc0},//    
		{    1329,	  0x49,	0x7,	0xc4},//    
		{    1356,	  0x49,	0x7,	0xc8},//    
		{    1383,	  0x49,	0x7,	0xcc},//    
		{    1410,	  0x49,	0x7,	0xd0},//    
		{    1437,	  0x49,	0x7,	0xd4},//    
		{    1464,	  0x49,	0x7,	0xd8},//    
		{    1492,	  0x49,	0x7,	0xdc},//    
		{    1519,	  0x49,	0x7,	0xe0},//    
		{    1546,	  0x49,	0x7,	0xe4},//    
		{    1573,	  0x49,	0x7,	0xe8},//    
		{    1600,	  0x49,	0x7,	0xec},//    
		{    1627,	  0x49,	0x7,	0xf0},//    
		{    1654,	  0x49,	0x7,	0xf4},//    
		{    1681,	  0x49,	0x7,	0xf8},//    
		{    1709,	  0x49,	0x7,	0xfc},//  
		{    1736,	  0x4b,	0x7,	0x80},//  27.128x
		{    1790,	  0x4b,	0x7,	0x84},//         
		{    1844,	  0x4b,	0x7,	0x88},//         
		{    1898,	  0x4b,	0x7,	0x8c},//         
		{    1953,	  0x4b,	0x7,	0x90},//         
		{    2007,	  0x4b,	0x7,	0x94},//         
		{    2061,	  0x4b,	0x7,	0x98},//         
		{    2115,	  0x4b,	0x7,	0x9c},//         
		{    2170,	  0x4b,	0x7,	0xa0},//         
		{    2224,	  0x4b,	0x7,	0xa4},//         
		{    2278,	  0x4b,	0x7,	0xa8},//         
		{    2333,	  0x4b,	0x7,	0xac},//         
		{    2387,	  0x4b,	0x7,	0xb0},//         
		{    2441,	  0x4b,	0x7,	0xb4},//         
		{    2495,	  0x4b,	0x7,	0xb8},//         
		{    2550,	  0x4b,	0x7,	0xbc},//         
		{    2604,	  0x4b,	0x7,	0xc0},//         
		{    2658,	  0x4b,	0x7,	0xc4},//         
		{    2712,	  0x4b,	0x7,	0xc8},//         
		{    2767,	  0x4b,	0x7,	0xcc},//         
		{    2821,	  0x4b,	0x7,	0xd0},//         
		{    2875,	  0x4b,	0x7,	0xd4},//         
		{    2929,	  0x4b,	0x7,	0xd8},//         
		{    2984,	  0x4b,	0x7,	0xdc},//         
		{    3038,	  0x4b,	0x7,	0xe0},//         
		{    3092,	  0x4b,	0x7,	0xe4},//         
		{    3146,	  0x4b,	0x7,	0xe8},//         
		{    3201,	  0x4b,	0x7,	0xec},//         
		{    3255,	  0x4b,	0x7,	0xf0},//         
		{    3309,	  0x4b,	0x7,	0xf4},//         
		{    3363,	  0x4b,	0x7,	0xf8},//         
		{    3418,	  0x4b,	0x7,	0xfc},//         
		{    3472,	  0x4f,	0x7,	0x80},//  54.256x
		{    3581,	  0x4f,	0x7,	0x84},//         
		{    3689,	  0x4f,	0x7,	0x88},//         
		{    3797,	  0x4f,	0x7,	0x8c},//         
		{    3906,	  0x4f,	0x7,	0x90},//         
		{    4014,	  0x4f,	0x7,	0x94},//         
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
	
	/*
	printf("-------------------------------\n");
	switch (ubPclkIdx)
	{
        //T_term-pd = 140ns (by oscilloscope)
        //DLAN_PDD_SEL = (T_term-pd / 16.67ns) = 8.398        
		case SEN_FPS25:
			MIPI->DLAN_PDD_LSCALE_SEL0 = 7;//7;	
			break;       
		case SEN_FPS30:
			MIPI->DLAN_PDD_LSCALE_SEL0 = 7;//7;
			break;
		default:
			MIPI->DLAN_PDD_LSCALE_SEL0 = 7;//7;
			break;
	}
	printf("[FPS=%d]PDD select:%d\n",ubPclkIdx, MIPI->DLAN_PDD_LSCALE_SEL0);
	printf("-------------------------------\n");
	*/
	MIPI->DLAN_PDD_LSCALE_SEL0 = 12;	
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
	bSEN_I2C_Read (SC230AI_CHIP_ID_HIGH_ADDR, &pBuf[1]);
	bSEN_I2C_Read (SC230AI_CHIP_ID_LOW_ADDR, &pBuf[0]);
	if (SC230AI_CHIP_ID != uwPID)
	{
		printd(DBG_ErrorLvl, "Read chip ID fail.<0x%x 0x%x>\n", SC230AI_CHIP_ID, uwPID);
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
    printd(DBG_CriticalLvl, "CS230AI Sensor\n");

	for (i=0; i<sizeof(ubSEN_InitTable); )
	{
		if (ubSEN_InitTable[i] == 0x83)	// write
		{
			bSEN_I2C_Write(ubSEN_InitTable[i+1], ubSEN_InitTable[i+2], ubSEN_InitTable[i+3]);
            i+=4;
		}else if (ubSEN_InitTable[i] == 0xbb){
            TIMER_Delay_ms(((ubSEN_InitTable[i+1]<<8) + ubSEN_InitTable[i+2]));
            i+=3;
        }
	}
    //mirror and flip
    //bSEN_I2C_Write(0x30, 0x07, 0x3);
    //SEN_PclkSetting(SEN_FPS30);
    
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
    //SEN_SetSensorRate(SENSOR_120MHz, 3);
	SEN_SetSensorRate(SENSOR_96MHz, 4);
    // Enable sensor PCLK
    SEN->SEN_CLK_EN = 1;
    //Set ISP pipeline
    sensor_cfg.tPathType = BAYER_SENSOR_NORMAL;
    SEN_SenosrOutputType(sensor_cfg.tPathType);
    // change ISP_LH_SEL to 1, for D2-D9 input
    SEN->ISP_LH_SEL = 0;
    // change SN_SEN->RAW_REORDER to 0, start from B
    SEN->RAW_REORDER = 0;//3;
    // Set dummy line & pixel
    SEN->DMY_DIV = 3;
    SEN->NUM_DMY_LN = 45;//16;
    SEN->NUM_DMY_DSTB = 480;//256;
    SEN->DMY_INSERT = 1;
    
    // Initial dummy sensor
    if (ubSEN_SetSensorInitTable() != 1)
    {
        printf("SC230AI startup failed! \n\r");
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
	ExpLine= xtSENInst.xtSENCtl.uwDmyLine ;
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
    //uint16_t uwTemp;
	//Set dummy lines
	bSEN_I2C_WriteTry(SC230AI_FRAME_LENGTH_H, (uint8_t)((uwDL>>8) &0x007f), TRY_COUNTS);
	bSEN_I2C_WriteTry(SC230AI_FRAME_LENGTH_L, (uint8_t)(uwDL &0x00ff), TRY_COUNTS);     
    
    // for smartsense suggestion
    //uwTemp = uwDL - 0x02;   
    //bSEN_I2C_WriteTry(SC2235_D_FRAME_LENGTH_H, (uint8_t)((uwTemp>>8) &0x00ff), TRY_COUNTS);  
    //bSEN_I2C_WriteTry(SC2235_D_FRAME_LENGTH_L, (uint8_t)(uwTemp &0x00ff), TRY_COUNTS); 
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
    bSEN_I2C_WriteTry(SC230AI_EXPH, (uint8_t)((uwExpLine>>12)&0x000f), TRY_COUNTS);
    bSEN_I2C_WriteTry(SC230AI_EXPM, (uint8_t)((uwExpLine>>4)&0x00ff), TRY_COUNTS);
    bSEN_I2C_WriteTry(SC230AI_EXPL, (uint8_t)((uwExpLine&0x0f) << 4), TRY_COUNTS);
    // save exposure line
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
	uint8_t ubIndex;  
	
    // update struct value.
   xtSENInst.xtSENCtl.uwGain = ulGainX1024;
	ulGainX1024=ulGainX1024/16;
	if(ulGainX1024 < 64)
	{
		ulGainX1024 = 64;
	}
   if(ulGainX1024 == ulOldGainValue)
       return;
	 

	for(ubIndex=(cbAE_SensorVTblRowSZ-1); ubIndex>0; ubIndex--)
	{
			if(ulGainX1024>=ctSC230_GainTbl[ubIndex].ulGainLevelTable)
			{
					break;
			}
	}

	bSEN_I2C_WriteTry(0x3e09, ctSC230_GainTbl[ubIndex].ubAGain, TRY_COUNTS);
	bSEN_I2C_WriteTry(0x3e06, ctSC230_GainTbl[ubIndex].ubDCoarseGain, TRY_COUNTS);
	bSEN_I2C_WriteTry(0x3e07, ctSC230_GainTbl[ubIndex].ubDFineGain , TRY_COUNTS);
	

    //save gain value
    ulOldGainValue = ulGainX1024;
}

//------------------------------------------------------------------------------
void SEN_SetMirrorFlip(uint8_t ubMirrorEn, uint8_t ubFlipEn)
{
	uint8_t result=0xFF;
    if(!ubIQ_GetLoadIqFinishFg())
    {
    	printf(" Mirror & Flip  SC230AI  Reject !!!!\n");
        return;
    }
    
    SEN_SetDetectVideoState(0);
    if(ubMirrorEn)
        xtSENInst.ubImgMode |=  SC230AI_MIRROR;
    else
        xtSENInst.ubImgMode &=  ~SC230AI_MIRROR;
    
    if(ubFlipEn)
        xtSENInst.ubImgMode |=  SC230AI_FLIP;
    else
        xtSENInst.ubImgMode &=  ~SC230AI_FLIP;
    printf("SC230AI Mirror & Flip = 0x%02x \n " ,xtSENInst.ubImgMode );
    result = bSEN_I2C_Write(0x32, 0x21, xtSENInst.ubImgMode);
    printf("SC230AI Mirror & Flip Result = %d \n " ,result );    
    SEN_SetRawReorder(ubMirrorEn, ubFlipEn);
    TIMER_Delay_ms(33);
    SEN_SetDetectVideoState(1);
}

//------------------------------------------------------------------------------
void SEN_GroupHoldOnVSync(void)
{
    //bSEN_I2C_Write(0x30, 0x01, 0x1);
}

//------------------------------------------------------------------------------
void SEN_GroupHoldOffVSync(void)
{
    //bSEN_I2C_Write(0x30, 0x01, 0x0);
}

//------------------------------------------------------------------------------
void SEN_SetSensorImageSize(void)
{
	// Image for ISP
	sensor_cfg.xtSENWin.uwHSize = ISP_WIDTH;
	sensor_cfg.xtSENWin.uwVSize = ISP_HEIGHT - 8;
    sensor_cfg.xtSENWin.uwHStart = 0;
    sensor_cfg.xtSENWin.uwVStart = 0;
}

//------------------------------------------------------------------------------
void SEN_SetSensorType(void)
{
    sensor_cfg.ubSensorType = SEN_SC230AI;	
    printf("sensor type is SC230AI\r\n");
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
	/*
    uint8_t ubIdx; 
    uint16_t uwExpLine;
    uint32_t ulGainX1024;
    
    uwExpLine = ulExp;
    ulGainX1024 = ulGain/2;

    //EXPLINE
    bSEN_I2C_WriteTry(IMX462_DUMMY_LINE_M, (uint8_t)(((uwExpLine)>>8)&0x00ff), TRY_COUNTS);	
    bSEN_I2C_WriteTry(IMX462_DUMMY_LINE_L, (uint8_t)((uwExpLine)&0x00ff), TRY_COUNTS);  

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
    
    bSEN_I2C_WriteTry(IMX462_GAIN, xtSENInst.ubBuf[2], TRY_COUNTS);
	*/
}

void SEN_SetEnvironment(void)
{  
	/*
    int32_t slResTemp[0x40];
    uint32_t ulValue;
    uint8_t RowTest ,ColTest;
    
    ulValue = ulAE_AvgOfYSum();
    RowTest = sizeof(slSensorEnvir) / sizeof(slSensorEnvir[0]);
    ColTest = sizeof(slSensorEnvir[0]) / sizeof(slSensorEnvir[0])[0];
    IQ_DynamicLinearInterpolation(ulValue, RowTest, ColTest, &slSensorEnvir[0][0], slResTemp);

    SEN_SetSensorEnvironment(slResTemp[0], slResTemp[1]);
    AE_SetEnvironment(slResTemp[2], slResTemp[3]);
	*/
}
#endif
