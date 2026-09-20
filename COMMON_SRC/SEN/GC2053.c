/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		GC2053.c
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

#if (SEN_USE == SEN_GC2053)
#define TRY_COUNTS 3
#define GainTableColSZ		(sizeof(ctGC2053_GainTable)/sizeof(ctGC2053_GainTable[0]))
AE_EVENT_PROCESS tAE_SensorPara;
struct SENSOR_SETTING sensor_cfg;
tfSENObj xtSENInst;
I2C1_Type *pI2C_type;
SEN_STATE sensor_state;

struct GC2053_GainTable {
    uint32_t ulGainLevelTable;
	uint8_t ubReg0xb4;
	uint8_t ubReg0xb3;
    uint8_t ubReg0xb8;
    uint8_t ubReg0xb9;
};

static struct GC2053_GainTable ctGC2053_GainTable[] = {
    {64,    0x00,    0x00,  0x01,  0x00,},     //1
    {74,    0x00,    0x10,  0x01,  0x0C,},     //1.185
    {89,    0x00,    0x20,  0x01,  0x1B,},     //1.4
    {102,   0x00,    0x30,  0x01,  0x2C,},     //1.659
    {127,   0x00,    0x40,  0x01,  0x3f,},     //2
    {147,   0x00,    0x50,  0x02,  0x16,},     //2.37
    {177,   0x00,    0x60,  0x02,  0x35,},     //2.8
    {203,   0x00,    0x70,  0x03,  0x16,},     //3.318
    {260,   0x00,    0x80,  0x04,  0x02,},     //4
    {300,   0x00,    0x90,  0x04,  0x31,},     //4.74
    {361,   0x00,    0xA0,  0x05,  0x32,},     //5.6
    {415,   0x00,    0xB0,  0x06,  0x35,},     //6.636
    {504,   0x00,    0xC0,  0x08,  0x04,},     //8
    {581,   0x00,    0x5A,  0x09,  0x19,},     //9.48
    {722,   0x00,    0x83,  0x0B,  0x0F,},     //11.2
    {832,   0x00,    0x93,  0x0D,  0x12,},     //13.272
    {1027,  0x00,    0x84,  0x10,  0x00,},     //16
    {1182,  0x00,    0x94,  0x12,  0x3A,},     //18.96
    {1408,  0x00,    0x2C,  0x1A,  0x02,},     //22.4
    {1621,  0x01,    0x3C,  0x1B,  0x20,},     //26.544
    {1990,  0x00,    0x8c,  0x20,  0x0f,},     //32
    {2291,  0x00,    0x9C,  0x26,  0x07,},     //37.92
    {2850,  0x02,    0x94,  0x36,  0x21,},     //44.8
    {3282,  0x02,    0x74,  0x37,  0x3A,},     //53.088
    {4048,  0x00,    0xC6,  0x3d,  0x02,},     //64
    {5180,  0x00,    0xDC,  0x3F,  0x3F,},     //75.84
    {5500,  0x02,    0x85,  0x3F,  0x3F,},     //89.6
    {6744,  0x03,    0x36,  0x3F,  0x3F,},     //106.17
    {7073,  0x00,    0xCE,  0x3F,  0x3F,},     //128
};

#if (SEN_INTERFACE == SEN_DVP)
    #if ((ISP_RES == ISP_FHD) && (ISP_FPS == SEN_30FPS))
const uint8_t ubSEN_InitTable[] = {
	//------------------------------
	// Initial Table
	//------------------------------
    // GC2053_24Minput_30fps
    0x82,  0xfe,  0x80,
    0x82,  0xfe,  0x80,
    0x82,  0xfe,  0x80,
    0x82,  0xfe,  0x00,
    0x82,  0xf2,  0x00, 
    0x82,  0xf3,  0x0f, 
    0x82,  0xf4,  0x36, 
    0x82,  0xf5,  0xc0, 
    0x82,  0xf6,  0x44,
    0x82,  0xf7,  0x01, 
    0x82,  0xf8,  0x63,
    0x82,  0xf9,  0x40, 
    0x82,  0xfc,  0x8e,

    0x82,  0xfe,  0x00,
    0x82,  0x87,  0x18,
    0x82,  0xee,  0x30,
    0x82,  0xd0,  0xb7,

    //Shutter time = 0x460 = 1120
    0x82,  0x03,  0x04,
    0x82,  0x04,  0x60,
    //Line length = (0x44c << 1) = (1100 << 1) = 2200
    0x82,  0x05,  0x04,
    0x82,  0x06,  0x4c,
    0x82,  0x07,  0x00,
    0x82,  0x08,  0x11,
    //Row start = 0x02 = 2
    0x82,  0x09,  0x00, 
    0x82,  0x0a,  0x02,
    //Col start = 0x02 = 2
    0x82,  0x0b,  0x00,
    0x82,  0x0c,  0x02,
    //win_height = 0x440 = 1088=>1096
    0x82,  0x0d,  0x04,
    0x82,  0x0e,  0x48,//0x40
    0x82,  0x12,  0xe2,
    0x82,  0x13,  0x16,
    0x82,  0x19,  0x0a,


    0x82,  0x21,  0x1c,
    0x82,  0x28,  0x0a,
    0x82,  0x29,  0x24,
    0x82,  0x2b,  0x04,
    0x82,  0x32,  0xf8,
    0x82,  0x37,  0x03, 
    0x82,  0x39,  0x15,
    0x82,  0x43,  0x07,
    0x82,  0x44,  0x40, 
    0x82,  0x46,  0x0b,
    0x82,  0x4b,  0x20,
    0x82,  0x4e,  0x08, 
    0x82,  0x55,  0x20, 
    0x82,  0x66,  0x05, 
    0x82,  0x67,  0x05, 
    0x82,  0x77,  0x01,
    0x82,  0x78,  0x00,
    0x82,  0x7c,  0x93,
    0x82,  0x8c,  0x12, 
    0x82,  0x8d,  0x92,
    0x82,  0x90,  0x00,     //0x1,frame length according exposure time
    0x82,  0x9d,  0x10,
    0x82,  0xce,  0x7c,
    0x82,  0xd2,  0x41,
    0x82,  0xd3,  0xdc,
    0x82,  0xe6,  0x50,

    0x82,  0xb6,  0xc0,
    0x82,  0xb0,  0x70,
    0x82,  0xb1,  0x01,
    0x82,  0xb2,  0x00,
    0x82,  0xb3,  0x00,
    0x82,  0xb4,  0x00,
    0x82,  0xb8,  0x01,
    0x82,  0xb9,  0x00,

    0x82,  0x26,  0x30,
    0x82,  0xfe,  0x01,             //page select==>1
    0x82,  0x40,  0x23,
    0x82,  0x55,  0x07,
    0x82,  0x60,  0x40, 
    0x82,  0xfe,  0x04,
    0x82,  0x14,  0x78,
    0x82,  0x15,  0x78, 
    0x82,  0x16,  0x78, 
    0x82,  0x17,  0x78,
    0x82,  0xfe,  0x01,
    //Out window y1 = 0x00 = 0
    0x82,  0x92,  0x00,
    //Out window x1 = 0x03 = 3
    0x82,  0x94,  0x03,
    //Out window height = 0x438 = 1088
    0x82,  0x95,  0x04,
    0x82,  0x96,  0x40,//0x38
    //Out window width = 0x780 = 1920
    0x82,  0x97,  0x07,
    0x82,  0x98,  0x80,

    0x82,  0xfe,  0x01,
    0x82,  0x01,  0x05,
    0x82,  0x02,  0x89,
    0x82,  0x04,  0x01,
    0x82,  0x07,  0xa6,
    0x82,  0x08,  0xa9,
    0x82,  0x09,  0xa8,
    0x82,  0x0a,  0xa7,
    0x82,  0x0b,  0xff,
    0x82,  0x0c,  0xff,
    0x82,  0x0f,  0x00,
    0x82,  0x50,  0x1c,
    0x82,  0x89,  0x03,

    0x82,  0x28,  0x86,
    0x82,  0x29,  0x86,
    0x82,  0x2a,  0x86,
    0x82,  0x2b,  0x68,
    0x82,  0x2c,  0x68,
    0x82,  0x2d,  0x68,
    0x82,  0x2e,  0x68,
    0x82,  0x2f,  0x68,

    0x82,  0x30,  0x4f,
    0x82,  0x31,  0x68,
    0x82,  0x32,  0x67,
    0x82,  0x33,  0x66,
    0x82,  0x34,  0x66,
    0x82,  0x35,  0x66,
    0x82,  0x36,  0x66,
    0x82,  0x37,  0x66,

    0x82,  0x38,  0x62,
    0x82,  0x39,  0x62,
    0x82,  0x3a,  0x62,
    0x82,  0x3b,  0x62,
    0x82,  0x3c,  0x62,
    0x82,  0x3d,  0x62,
    0x82,  0x3e,  0x62,
    0x82,  0x3f,  0x62,

    0x82,  0xfe,  0x01,
    0x82,  0x9a,  0x06,
    0x82,  0xfe,  0x00,
    0x82,  0x7b,  0x2a,
    0x82,  0x23,  0x2d,
    0x82,  0xfe,  0x03,             //page select==>3
    0x82,  0x01,  0x20, 
    0x82,  0x02,  0x56,
    0x82,  0x03,  0xb2, 
    0x82,  0x12,  0x80,
    0x82,  0x13,  0x07,
    0x82,  0xfe,  0x00,             //page select==>0
    0x82,  0x3e,  0x40,
};
#elif ((ISP_RES == ISP_HD) && (ISP_FPS == SEN_60FPS))
const uint8_t ubSEN_InitTable[] = {
	//------------------------------
	// Initial Table
	//------------------------------
    // GC2053_24Minput_60fps
    0x82,  0xff,  0x07,
    0x82,  0x04,  0x00,
    0x82,  0x16,  0x12,

    //ff  28  03
    //ff  29  02
    //ff  2a  1b
    //ff  2b  1b
    //ff  2c  0a
    //ff  2d  0a
    //ff  2e  02
    //ff  2f  01
    //ff  30  13
    //ff  31  01

    0x82,  0x14,  0x2b,
    0x82,  0x20,  0x3d, //reset low, mclk enable, powndown high

    0xbb,  0x00,  0x64, //sleep 100ms
    0x82,  0x20,  0x3f, //reset high, mclk enable, powndown high
    0x82,  0xff,  0x6e, //8 addr 8 data

    /*system*/
    0x82,  0xfe,  0x80,
    0x82,  0xfe,  0x80,
    0x82,  0xfe,  0x80,
    0x82,  0xfe,  0x00,
    0x82,  0xf2,  0x00, 
    0x82,  0xf3,  0x0f, 
    0x82,  0xf4,  0x36, 
    0x82,  0xf5,  0xc0, 
    0x82,  0xf6,  0x23, 
    0x82,  0xf7,  0x01, 
    0x82,  0xf8,  0x24, 
    0x82,  0xf9,  0x42, 
    0x82,  0xfc,  0x8e,

    /*cisctl&analog*/
    0x82,  0xfe,  0x00,
    0x82,  0x87,  0x18, 
    0x82,  0xee,  0x30, 
    0x82,  0xd0,  0xb7, 

    //Shutter time = 0x2d0 = 720
    0x82,  0x03,  0x02,
    0x82,  0x04,  0xd0,
    //Line length = (0x401 << 1) = (1024 << 1) = 2050
    0x82,  0x05,  0x04,
    0x82,  0x06,  0x01,
    0x82,  0x07,  0x00,
    0x82,  0x08,  0x11,
    //Row start = 0x9a = 154
    0x82,  0x09,  0x00,
    0x82,  0x0a,  0x9a,
    //Col start = 0x140 = 320
    0x82,  0x0b,  0x01,
    0x82,  0x0c,  0x40,
    //height = 0x2e7 = 743
    0x82,  0x0d,  0x02,
    0x82,  0x0e,  0xe7,
    //width = 0x514 = 1300
    0x82,  0x0f,  0x05,
    0x82,  0x10,  0x14,
    0x82,  0x12,  0xe2,
    0x82,  0x13,  0x16,
    0x82,  0x19,  0x0a,
    0x82,  0x21,  0x1c,
    0x82,  0x28,  0x0a,
    0x82,  0x29,  0x24,
    0x82,  0x2b,  0x04,
    0x82,  0x32,  0xf8,
    0x82,  0x37,  0x03,
    0x82,  0x39,  0x15,
    0x82,  0x43,  0x07,
    0x82,  0x44,  0x40, 
    0x82,  0x46,  0x0b, 
    0x82,  0x4b,  0x20, 
    0x82,  0x4e,  0x08, 
    0x82,  0x55,  0x20, 
    0x82,  0x66,  0x05, 
    0x82,  0x67,  0x05, 
    0x82,  0x77,  0x00, 
    0x82,  0x78,  0x20, 
    0x82,  0x7c,  0x93, 
    0x82,  0x8c,  0x12, 
    0x82,  0x8d,  0x90,
    0x82,  0x9d,  0x10,
    0x82,  0xce,  0x7c, 
    0x82,  0xd2,  0x41, 
    0x82,  0xd3,  0xdc, 
    0x82,  0xe6,  0x50, 
    0x82,  0x44,  0x20,

    /*gain*/
    0x82,  0xb6,  0xc0,
    0x82,  0xb0,  0x70,
    0x82,  0xb1,  0x01,
    0x82,  0xb2,  0x00,
    0x82,  0xb3,  0x00,
    0x82,  0xb4,  0x00,
    0x82,  0xb8,  0x01,
    0x82,  0xb9,  0x00,

    /*blk*/
    0x82,  0x26,  0x30,
    0x82,  0xfe,  0x01,
    0x82,  0x40,  0x23,
    0x82,  0x55,  0x07,
    0x82,  0x60,  0x40,
    0x82,  0xfe,  0x04,
    0x82,  0x14,  0x78,
    0x82,  0x15,  0x78,
    0x82,  0x16,  0x78,
    0x82,  0x17,  0x78,


    /*window*/
    0x82,  0xfe,  0x01,
    //Out window y1 = 0x00 = 0
    0x82,  0x92,  0x00,
    //Out window x1 = 0x03 = 3
    0x82,  0x94,  0x03,
    //height = 0x2d0 = 720
    0x82,  0x95,  0x02,
    0x82,  0x96,  0xd0,
    //height = 0x500 = 1280
    0x82,  0x97,  0x05,
    0x82,  0x98,  0x00, 

    /*ISP*/
    0x82,  0xfe,  0x01,
    0x82,  0x01,  0x05,
    0x82,  0x02,  0x89, 
    0x82,  0x04,  0x01, 
    0x82,  0x07,  0xa6,
    0x82,  0x08,  0xa9,
    0x82,  0x09,  0xa8,
    0x82,  0x0a,  0xa7,
    0x82,  0x0b,  0xff,
    0x82,  0x0c,  0xff,
    0x82,  0x0f,  0x00,
    0x82,  0x50,  0x1c,
    0x82,  0x89,  0x03,

    0x82,  0xfe,  0x04,
    0x82,  0x28,  0x86,//84
    0x82,  0x29,  0x86,//84
    0x82,  0x2a,  0x86,//84
    0x82,  0x2b,  0x68,//84
    0x82,  0x2c,  0x68,//84
    0x82,  0x2d,  0x68,//84
    0x82,  0x2e,  0x68,//83
    0x82,  0x2f,  0x68,//82

    0x82,  0x30,  0x4f,//82
    0x82,  0x31,  0x68,//82
    0x82,  0x32,  0x67,//82
    0x82,  0x33,  0x66,//82
    0x82,  0x34,  0x66,//82
    0x82,  0x35,  0x66,//82
    0x82,  0x36,  0x66,//64
    0x82,  0x37,  0x66,//68

    0x82,  0x38,  0x62,
    0x82,  0x39,  0x62,
    0x82,  0x3a,  0x62,
    0x82,  0x3b,  0x62,
    0x82,  0x3c,  0x62,
    0x82,  0x3d,  0x62,
    0x82,  0x3e,  0x62,
    0x82,  0x3f,  0x62,

    /****DVP & MIPI****/

    0x82,  0xfe,  0x01,
    0x82,  0x9a,  0x06, 
    0x82,  0xfe,  0x00,
    0x82,  0x7b,  0x2a, 
    0x82,  0x23,  0x2d, 
    0x82,  0xfe,  0x03,
    0x82,  0x01,  0x20, 
    0x82,  0x02,  0x56, 
    0x82,  0x03,  0xb2, 
    0x82,  0x12,  0x00,
    0x82,  0x13,  0x05, 
    0x82,  0xfe,  0x00,
    0x82,  0x3e,  0x40, 
};
    #endif
#elif (SEN_INTERFACE == SEN_MIPI)
const uint8_t ubSEN_InitTable[] = {
    //------------------------------
    // Initial Table
    //------------------------------
    // release_V1_GC2053_MIPI_2lane_base24M_30fps_20181206
    /****system****/
    0x82, 0xfe, 0x80,
    0x82, 0xfe, 0x80,
    0x82, 0xfe, 0x80,
    0x82, 0xfe, 0x00,
    0x82, 0xf2, 0x00,
    0x82, 0xf3, 0x00,
    0x82, 0xf4, 0x36,
    0x82, 0xf5, 0xc0,
    0x82, 0xf6, 0x44,
    0x82, 0xf7, 0x01,
    0x82, 0xf8, 0x63,
    0x82, 0xf9, 0x40,
    0x82, 0xfc, 0x8e,
    /****CISCTL & ANALOG****/
    0x82, 0xfe, 0x00,
    0x82, 0x87, 0x18,
    0x82, 0xee, 0x30,
    0x82, 0xd0, 0xb7,
    0x82, 0x03, 0x04,
    0x82, 0x04, 0x60,
    0x82, 0x05, 0x04,
    0x82, 0x06, 0x4c,
    0x82, 0x07, 0x00,
    0x82, 0x08, 0x11,
    0x82, 0x09, 0x00,
    0x82, 0x0a, 0x02,
    0x82, 0x0b, 0x00,
    0x82, 0x0c, 0x02,
    0x82, 0x0d, 0x04,
    0x82, 0x0e, 0x48,//0x40
    0x82, 0x12, 0xe2,
    0x82, 0x13, 0x16,
    0x82, 0x19, 0x0a,
    0x82, 0x21, 0x1c,
    0x82, 0x28, 0x0a,
    0x82, 0x29, 0x24,
    0x82, 0x2b, 0x04,
    0x82, 0x32, 0xf8,
    0x82, 0x37, 0x03,
    0x82, 0x39, 0x15,
    0x82, 0x43, 0x07,
    0x82, 0x44, 0x40,
    0x82, 0x46, 0x0b,
    0x82, 0x4b, 0x20,
    0x82, 0x4e, 0x08,
    0x82, 0x55, 0x20,
    0x82, 0x66, 0x05,
    0x82, 0x67, 0x05,
    0x82, 0x77, 0x01,
    0x82, 0x78, 0x00,
    0x82, 0x7c, 0x93,
    0x82, 0x8c, 0x12,
    0x82, 0x8d, 0x92,
    0x82, 0x90, 0x00,       //0x1,frame length according exposure time
    0x82, 0x9d, 0x10,
    0x82, 0xce, 0x7c,
    0x82, 0xd2, 0x41,
    0x82, 0xd3, 0xdc,
    0x82, 0xe6, 0x50,
    /*gain*/
    0x82, 0xb6, 0xc0,
    0x82, 0xb0, 0x70,
    0x82, 0xb1, 0x01,
    0x82, 0xb2, 0x00,
    0x82, 0xb3, 0x00,
    0x82, 0xb4, 0x00,
    0x82, 0xb8, 0x01,
    0x82, 0xb9, 0x00,
    /*blk*/
    0x82, 0x26, 0x30,
    0x82, 0xfe, 0x01,
    0x82, 0x40, 0x23,
    0x82, 0x55, 0x07,
    0x82, 0x60, 0x40,
    0x82, 0xfe, 0x04,
    0x82, 0x14, 0x78,
    0x82, 0x15, 0x78,
    0x82, 0x16, 0x78,
    0x82, 0x17, 0x78,
    /*window*/
    0x82, 0xfe, 0x01,
    0x82, 0x92, 0x00,
    0x82, 0x94, 0x03,
    0x82, 0x95, 0x04,
    0x82, 0x96, 0x40,//0x38
    0x82, 0x97, 0x07,
    0x82, 0x98, 0x80,
    /*ISP*/
    0x82, 0xfe, 0x01,
    0x82, 0x01, 0x05,
    0x82, 0x02, 0x89,
    0x82, 0x04, 0x01,
    0x82, 0x07, 0xa6,
    0x82, 0x08, 0xa9,
    0x82, 0x09, 0xa8,
    0x82, 0x0a, 0xa7,
    0x82, 0x0b, 0xff,
    0x82, 0x0c, 0xff,
    0x82, 0x0f, 0x00,
    0x82, 0x50, 0x1c,
    0x82, 0x89, 0x03,
    0x82, 0xfe, 0x04,
    0x82, 0x28, 0x86,
    0x82, 0x29, 0x86,
    0x82, 0x2a, 0x86,
    0x82, 0x2b, 0x68,
    0x82, 0x2c, 0x68,
    0x82, 0x2d, 0x68,
    0x82, 0x2e, 0x68,
    0x82, 0x2f, 0x68,
    0x82, 0x30, 0x4f,
    0x82, 0x31, 0x68,
    0x82, 0x32, 0x67,
    0x82, 0x33, 0x66,
    0x82, 0x34, 0x66,
    0x82, 0x35, 0x66,
    0x82, 0x36, 0x66,
    0x82, 0x37, 0x66,
    0x82, 0x38, 0x62,
    0x82, 0x39, 0x62,
    0x82, 0x3a, 0x62,
    0x82, 0x3b, 0x62,
    0x82, 0x3c, 0x62,
    0x82, 0x3d, 0x62,
    0x82, 0x3e, 0x62,
    0x82, 0x3f, 0x62,
    /****DVP & MIPI****/
    0x82, 0xfe, 0x01,
    0x82, 0x9a, 0x06,
    0x82, 0xfe, 0x00,
    0x82, 0x7b, 0x2a,
    0x82, 0x23, 0x2d,
    0x82, 0xfe, 0x03,
    0x82, 0x01, 0x27,
    0x82, 0x02, 0x5f,
    0x82, 0x03, 0xb6,
    0x82, 0x12, 0x80,
    0x82, 0x13, 0x07,
    0x82, 0x15, 0x12,
    0x82, 0xfe, 0x00,
    0x82, 0x3e, 0x91,
};
#endif



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
#if (SEN_INTERFACE == SEN_DVP)
    #if ((ISP_RES == ISP_FHD) && (ISP_FPS == SEN_30FPS))
        tAE_SensorPara.ulSensorPclk = 74976000;  
        tAE_SensorPara.ulSensorPixelPerLine = 2200;
        tAE_SensorPara.ulSensorLinePerFrame = 1136;
        tAE_SensorPara.ulSensorFrameRate = ubPclkIdx;     
        tAE_SensorPara.ulMaximumSensorFrameRate = 30;
    #elif ((ISP_RES == ISP_HD) && (ISP_FPS == SEN_60FPS))
        tAE_SensorPara.ulSensorPclk = 72000000;  
        tAE_SensorPara.ulSensorPixelPerLine = 1538;
        tAE_SensorPara.ulSensorLinePerFrame = 780;
        tAE_SensorPara.ulSensorFrameRate = ubPclkIdx;     
        tAE_SensorPara.ulMaximumSensorFrameRate = 60;
    #endif    
#elif (SEN_INTERFACE == SEN_MIPI)
    tAE_SensorPara.ulSensorPclk = 74250000;  
    tAE_SensorPara.ulSensorPixelPerLine = 2200;
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

//------------------------------------------------------------------------------//
//                         Power down timing selection                          //
//------------------------------------------------------------------------------//
//------------------------------------------------------------------------------
void MIPI_PowerDownTimingSelect(uint8_t ubPclkIdx)
{	
	printf("-------------------------------\n");
	switch (ubPclkIdx)
	{
        //long-packet = 164ns
        //short-packet = 156ns
        //T_term-pd = 156ns (by oscilloscope)
        //DLAN_PDD_SEL = (T_term-pd / 16.67ns) = 9.358       
		case SEN_FPS25:
			MIPI->DLAN_PDD_LSCALE_SEL0 = 9;	
			break;       
		case SEN_FPS30:
			MIPI->DLAN_PDD_LSCALE_SEL0 = 10;
			break;
		default:
			MIPI->DLAN_PDD_LSCALE_SEL0 = 9;
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
        if(count > 5)
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
	bSEN_I2C_Read (GC2053_CHIP_ID_HIGH_ADDR, &pBuf[1]);
	bSEN_I2C_Read (GC2053_CHIP_ID_LOW_ADDR, &pBuf[0]);
	if (GC2053_CHIP_ID != uwPID)
	{
		printd(DBG_ErrorLvl, "Read chip ID fail.<0x%x 0x%x>\n", GC2053_CHIP_ID, uwPID);
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
    printd(DBG_CriticalLvl, "GC2053 Sensor\n");
	for (i=0; i<sizeof(ubSEN_InitTable); i+=3)
	{
		if (ubSEN_InitTable[i] == 0x82)	// write
		{
			bSEN_I2C_Write(ubSEN_InitTable[i+1], ubSEN_InitTable[i+2]);
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
#if (SEN_INTERFACE == SEN_DVP)
    // Set ISP clock
    SEN_SetISPRate(4);
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
    SEN->RAW_REORDER = 3;
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
        printf("GC2053 startup failed! \n\r");
        return 0;
    }
    SEN_PclkSetting(SEN_FPS30);
#elif (SEN_INTERFACE == SEN_MIPI)
    // Set ISP clock
    SEN_SetISPRate(4);		
    // Set MIPI mode
    SEN->MIPI_MODE = 1;
    // Power down MIPI analog phy.
    MIPI->MIPI_PD = 0;
    // Set sensor clock
    SEN_SetSensorRate(SENSOR_96MHz, 4);	
    // enable sensor PCLK
    SEN->SEN_CLK_EN = 1;
    //Set ISP pipeline
    sensor_cfg.tPathType = BAYER_SENSOR_NORMAL;
    SEN_SenosrOutputType(sensor_cfg.tPathType);
    // change ISP_LH_SEL to 1, for D2-D9 input
    SEN->ISP_LH_SEL = 0;
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
        printf("GC2053_MIPI startup failed! \n\r");
        return 0;
    }
    // Enable MIPI and set MIPI channel select.
    SEN_MIPIControl(MIPI_2LANE, MIPI_CHANNEL0);    
	MIPI_PowerDownTimingSelect(SEN_FPS30);
	MIPI_AutoPhaseDetect();
    SEN_PclkSetting(SEN_FPS30);
#endif
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
    bSEN_I2C_WriteTry(GC2053_FRAME_H, (uint8_t)((uwDL>>8) & 0xff), TRY_COUNTS);
    bSEN_I2C_WriteTry(GC2053_FRAME_L, (uint8_t)(uwDL & 0xff), TRY_COUNTS);	
}

//------------------------------------------------------------------------------
void SEN_SetExpLine(void)
{
    SEN_WrExpLine(xtSENInst.xtSENCtl.uwExpLine);
}

//------------------------------------------------------------------------------
void SEN_WrExpLine(uint16_t uwExpLine)
{
	bSEN_I2C_WriteTry(GC2053_EXP_H, (uint8_t)((uwExpLine>>8) & 0xff), TRY_COUNTS);
	bSEN_I2C_WriteTry(GC2053_EXP_L, (uint8_t)(uwExpLine & 0xff), TRY_COUNTS);	
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
    uint32_t ulDigital_gain = 0;
    uint8_t ubIndex, i;
    
    // update struct value.
    xtSENInst.xtSENCtl.uwGain = ulGainX1024;
    ulGainX1024 = ulGainX1024 / 16;
	//	Set min gain is 1x gain
	if (ulGainX1024 < 64)
	{
		ulGainX1024 = 64;
	}
    
    if(ulGainX1024 == ulOldGainValue)
        return;
    //
    bSEN_I2C_Write(GC2053_PAGE_SEL, GC_PAGE0);
    for(i=0; i<GainTableColSZ; i++)
    {
        if((ulGainX1024 >= ctGC2053_GainTable[i].ulGainLevelTable) && (ulGainX1024 < ctGC2053_GainTable[i+1].ulGainLevelTable))
        {
            ubIndex = i;
            break;
        }
    }

    ulDigital_gain = ulGainX1024 * 64 / ctGC2053_GainTable[ubIndex].ulGainLevelTable;
    bSEN_I2C_Write(GC2053_PAGE_SEL, GC_PAGE0);
    bSEN_I2C_Write(0xb1, (ulDigital_gain>>6));
    bSEN_I2C_Write(0xb2, ((ulDigital_gain&0x3f)<<2));    
    bSEN_I2C_Write(0xb3, ctGC2053_GainTable[ubIndex].ubReg0xb3);    
    bSEN_I2C_Write(0xb4, ctGC2053_GainTable[ubIndex].ubReg0xb4);
    bSEN_I2C_Write(0xb8, ctGC2053_GainTable[ubIndex].ubReg0xb8);
    bSEN_I2C_Write(0xb9, ctGC2053_GainTable[ubIndex].ubReg0xb9);

    // save gain value    
    ulOldGainValue = ulGainX1024;
}

//------------------------------------------------------------------------------
void SEN_SetMirrorFlip(uint8_t ubMirrorEn, uint8_t ubFlipEn)
{
    if(!ubIQ_GetLoadIqFinishFg())
        return;
    
    if(ubMirrorEn)
        xtSENInst.ubImgMode |=  GC2053_MIRROR;
    else
        xtSENInst.ubImgMode &=  ~GC2053_MIRROR;
    
    if(ubFlipEn)
        xtSENInst.ubImgMode |=  GC2053_FLIP;
    else
        xtSENInst.ubImgMode &=  ~GC2053_FLIP;
    
    bSEN_I2C_Write(0x17, xtSENInst.ubImgMode);
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
    sensor_cfg.ubSensorType = SEN_GC2053;    
#if (SEN_INTERFACE == SEN_DVP)
    printd(DBG_Debug1Lvl, "sensor type is GC2053\n");
#elif (SEN_INTERFACE == SEN_MIPI)
    printd(DBG_Debug1Lvl, "sensor type is GC2053 MIPI\n");
#endif
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
    uint8_t ubIndex, i;
    uint16_t uwExpLine;
    uint32_t ulGainX1024;
    uint32_t ulDigital_gain = 0;    
    
    uwExpLine = ulExp;
    ulGainX1024 = ulGain/16;

    //EXPLINE
	bSEN_I2C_WriteTry(GC2053_EXP_H, (uint8_t)((uwExpLine>>8) & 0xff), TRY_COUNTS);
	bSEN_I2C_WriteTry(GC2053_EXP_L, (uint8_t)(uwExpLine & 0xff), TRY_COUNTS);  
 
    //GAIN
    bSEN_I2C_Write(GC2053_PAGE_SEL, GC_PAGE0);
    for(i=0; i<GainTableColSZ; i++)
    {
        if((ulGainX1024 >= ctGC2053_GainTable[i].ulGainLevelTable) && (ulGainX1024 < ctGC2053_GainTable[i+1].ulGainLevelTable))
        {
            ubIndex = i;
            break;
        }
    }

    ulDigital_gain = ulGainX1024 * 64 / ctGC2053_GainTable[ubIndex].ulGainLevelTable;
    bSEN_I2C_Write(GC2053_PAGE_SEL, GC_PAGE0);
    bSEN_I2C_Write(0xb1, (ulDigital_gain>>6));
    bSEN_I2C_Write(0xb2, ((ulDigital_gain&0x3f)<<2));    
    bSEN_I2C_Write(0xb3, ctGC2053_GainTable[ubIndex].ubReg0xb3);    
    bSEN_I2C_Write(0xb4, ctGC2053_GainTable[ubIndex].ubReg0xb4);
    bSEN_I2C_Write(0xb8, ctGC2053_GainTable[ubIndex].ubReg0xb8);
    bSEN_I2C_Write(0xb9, ctGC2053_GainTable[ubIndex].ubReg0xb9);
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
