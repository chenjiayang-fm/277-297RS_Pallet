/*!
	\file		RN6752.c
	\brief		Sensor RN6752 relation function
	\author		BoCun
	\version	3.2
	\date		2020/09/09
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
#include "BUF.h"
#include "BSP.h"

/*
Note
1.  RN6752 support AHD(1080P/720P), CVI/TVI(720P) and CVBS(N/P). 
2.  Fotmat: YUV422 8bit
3.  SEN_IQ_BIN_STATE = SEN_BYPASS_IQ;  <== ignored IQ(bypass dynamic IQ, AE, AWB and ISP)
4.  CVBS input resolution is NTSC:720x(480/2) or PAL:720x(544/2) (one field)
*/

#if (SEN_USE == SEN_RN6752)
#define TRY_COUNTS 3
static void SEN_CheckStateThread(void const *argument);
AE_EVENT_PROCESS tAE_SensorPara;
struct SENSOR_SETTING sensor_cfg;
tfSENObj xtSENInst;
I2C1_Type *pI2C_type;
SEN_STATE sensor_state;

uint8_t ubInitTable_FHD_2lane[] = {
    // 1080P@30 with mipi 2 data lanes + 1 clock lane out
    // pin24/23 data lane0, pin18/17 data lane1
    // pin16/15 data lane2, pin12/11 data lane3
    // pin14/13 clock lane
    // Slave address is 0x58
    // Register, data

    // if clock source(Xin) of RN6752 is 26MHz, please add these procedures marked first
    //0xD2, 0x85, // disable auto clock detect
    //0xD6, 0x37, // 27MHz default
    //0xD8, 0x18, // switch to 26MHz clock
    //delay(100), // delay 100ms

    0x82, 0x81, 0x01, // turn on video decoder0
    0x82, 0xDF, 0xFE, // enable ch0 as HD format
    0x82, 0xF0, 0xc0, // 144MHz output//0xc0

    // ch0
    0x82, 0xFF, 0x00, // switch to ch0 (default; optional)
    0x82, 0x00, 0x30, // internal use* 0x20=LPF off, 0x30=LPF on
    0x82, 0x06, 0x08, // internal use*
    0x82, 0x07, 0x63, // HD format
    0x82, 0x2A, 0x01, // filter control
    0x82, 0x3A, 0x20, // Insert Channel ID in SAV/EAV code
    0x82, 0x3F, 0x10, // channel ID
    0x82, 0x4C, 0x37, // equalizer
    0x82, 0x4F, 0x03, // sync control
    0x82, 0x50, 0x03, // 1080p resolution///0x03
    0x82, 0x56, 0x02, // BT 144M mode
    0x82, 0x5F, 0x44, // blank level
    0x82, 0x63, 0xF8, // filter control
    0x55,
//    0x82, 0x59, 0x00, // extended register access
//    0x82, 0x5A, 0x49, // data for extended register     //49==>FHD 30fps
//    0x82, 0x58, 0x01, // enable extended register write
    0x82, 0x59, 0x33, // extended register access
    0x82, 0x5A, 0x23, // data for extended register
    0x82, 0x58, 0x01, // enable extended register write
    0x82, 0x51, 0xF4, // scale factor1
    0x82, 0x52, 0x29, // scale factor2
    0x82, 0x53, 0x15, // scale factor3
    0x82, 0x5B, 0x01, // H-scaling control
    0x82, 0x5E, 0x0F, // enable H-scaling control
    0x82, 0x6A, 0x87, // H-scaling control
    0x82, 0x28, 0x92, // cropping
	
    0x66, //! Image Color tuning
	//! AHD 1080p 25
	0x01, 0x04, // Brightness
	0x02, 0x86, // Contrast
	0x03, 0xC0, // saturation
	0x04, 0x60, // hue
	0x05, 0x04, // sharpness(0x00~0x7F)
	0x57, 0x23, // black/white stretch(0x00~0x3F)
	//! AHD 1080p 30
	0x01, 0x04, // Brightness
	0x02, 0x86, // Contrast
	0x03, 0xC0, // saturation
	0x04, 0x80, // hue
	0x05, 0x04, // sharpness(0x00~0x7F)
	0x57, 0x23, // black/white stretch(0x00~0x3F)
	
    0x82, 0x68, 0x00, // coring
    0x82, 0x37, 0x33,
    0x82, 0x61, 0x6C,

    0x82, 0x81, 0x01, // turn on video decoder

    // mipi link1
    0x82, 0xFF, 0x09, // switch to mipi tx1
    0x82, 0x00, 0x03, // enable bias
    0x82, 0xFF, 0x08, // switch to mipi csi1
    0x82, 0x04, 0x03, // csi1 and tx1 reset
    0x82, 0x6C, 0x11, // disable ch output; turn on ch0
    0x82, 0x06, 0x4C, // 2 lanes
    0x82, 0x21, 0x01, // enable hs clock
    0x82, 0x78, 0xC0, // Y/C counts for ch0
    0x82, 0x79, 0x03, // Y/C counts for ch0
    0x82, 0x6C, 0x01, // enable ch output
    0x82, 0x04, 0x00, // csi1 and tx1 reset finish
    // 0x20, 0xAA, // invert clock phase
    // 0x07, 0x05, // enable non-clock

    // mipi link3
    0x82, 0xFF, 0x0A, // switch to mipi csi3
    0x82, 0x6C, 0x10, // disable ch output; turn off ch0~3
};

uint8_t ubInitTable_D1_PAL_2lane[] = {
    // D1@50 with mipi 2 data lanes + 1 clock lane out
    // pin24/23 data lane0, pin18/17 data lane1
    // pin16/15 data lane2, pin12/11 data lane3
    // pin14/13 clock lane
    // Slave address is 0x58
    // Register, data

    // if clock source(Xin) of RN6752 is 26MHz, please add these procedures marked first
    //0xD2, 0x85, // disable auto clock detect
    //0xD6, 0x37, // 27MHz default
    //0xD8, 0x18, // switch to 26MHz clock
    //delay(100), // delay 100ms

    0x82, 0x81, 0x01, // turn on video decoder
    0x82, 0xA3, 0x04,
    0x82, 0xDF, 0x0F, // enable CVBS format

    0x82, 0x88, 0x00,
    0x82, 0xF6, 0x00,

    // ch0
    0x82, 0xFF, 0x00, // switch to ch0 (default; optional)
    0x82, 0x00, 0x00, // internal use*
    0x82, 0x06, 0x08, // internal use*
    0x82, 0x07, 0x62, // HD format
    0x82, 0x2A, 0x81, // filter control
    0x82, 0x3A, 0x20, // Insert Channel ID in SAV/EAV code
    0x82, 0x3F, 0x10, // channel ID
    0x82, 0x4C, 0x37, // equalizer
    0x82, 0x4F, 0x00, // sync control
    0x82, 0x50, 0x00, // 720p resolution
    0x82, 0x56, 0x01, // 72M mode
    0x82, 0x5F, 0x00, // blank level
    0x82, 0x63, 0x75, // filter control
    0x82, 0x59, 0x00, // extended register access
    0x82, 0x5A, 0x00, // data for extended register
    0x82, 0x58, 0x01, // enable extended register write
    0x82, 0x59, 0x33, // extended register access
    0x82, 0x5A, 0x02, // data for extended register
    0x82, 0x58, 0x01, // enable extended register write
    0x82, 0x5B, 0x00, // H-scaling control
    0x82, 0x5E, 0x01, // enable H-scaling control
    0x82, 0x6A, 0x00, // H-scaling control
    0x82, 0x28, 0x92, // cropping 0xB2
    0x82, 0x20, 0x24,
    0x82, 0x23, 0x17,   //V0_Start = 23
    0x82, 0x24, 0x37,   //V0_End = 311      V = 311 - 23 = 288
    0x82, 0x25, 0x17,   //V1_Start = 23
    0x82, 0x26, 0x00,   //V1_End = 0
    0x82, 0x42, 0x00,   
    0x82, 0x01, 0x04, // Brightness
	0x82, 0x02, 0x86, // Contrast
	0x82, 0x03, 0xC0, // saturation
	0x82, 0x04, 0x80, // hue
    0x82, 0x05, 0x03, // sharpness
    0x82, 0x57, 0x20, // black/white stretch
    0x82, 0x68, 0x32, // coring
    0x82, 0x37, 0x33,
    0x82, 0x61, 0x6C,

    0x82, 0x81, 0x01,

    // mipi link1
    0x82, 0xFF, 0x09, // switch to mipi tx1
    0x82, 0x00, 0x03, // enable bias
    0x82, 0xFF, 0x08, // switch to mipi csi1
    0x82, 0x04, 0x03, // csi1 and tx1 reset
    0x82, 0x6C, 0x11, // disable ch output; turn on ch0
    0x82, 0x06, 0x4C, // 2 lanes
    0x82, 0x21, 0x01, // enable hs clock
    0x82, 0x78, 0x68, // Y/C counts for ch0
    0x82, 0x79, 0x01, // Y/C counts for ch0
    0x82, 0x6C, 0x01, // enable ch output
    0x82, 0x04, 0x00, // csi1 and tx1 reset finish
    //0x07, 0x05, //enable non-continuous clock
    //0x20, 0xAA, // invert hs clock

    // mipi link3
    0x82, 0xFF, 0x0A, // switch to mipi csi3
    0x82, 0x6C, 0x10, // disable ch output; turn off ch0~3
};

uint8_t ubInitTable_D1_NTSC_2lane[] = {
    // D1@60 with mipi 2 data lanes + 1 clock lane out
    // pin24/23 data lane0, pin18/17 data lane1
    // pin16/15 data lane2, pin12/11 data lane3
    // pin14/13 clock lane
    // Slave address is 0x58
    // Register, data

    // if clock source(Xin) of RN6752 is 26MHz, please add these procedures marked first
    //0xD2, 0x85, // disable auto clock detect
    //0xD6, 0x37, // 27MHz default
    //0xD8, 0x18, // switch to 26MHz clock
    //delay(100), // delay 100ms

    0x82, 0x81, 0x01, // turn on video decoder
    0x82, 0xA3, 0x04,
    0x82, 0xDF, 0x0F, // enable CVBS format

    0x82, 0x88, 0x00,
    0x82, 0xF6, 0x00,

    // ch0
    0x82, 0xFF, 0x00, // switch to ch0 (default; optional)
    0x82, 0x00, 0x00, // internal use*
    0x82, 0x06, 0x08, // internal use*
    0x82, 0x07, 0x63, // HD format
    0x82, 0x2A, 0x81, // filter control
    0x82, 0x3A, 0x20, // Insert Channel ID in SAV/EAV code
    0x82, 0x3F, 0x10, // channel ID
    0x82, 0x4C, 0x37, // equalizer
    0x82, 0x4F, 0x00, // sync control
    0x82, 0x50, 0x00, // 720p resolution
    0x82, 0x56, 0x01, // 72M mode
    0x82, 0x5F, 0x00, // blank level
    0x82, 0x63, 0x75, // filter control
    0x82, 0x59, 0x00, // extended register access
    0x82, 0x5A, 0x00, // data for extended register
    0x82, 0x58, 0x01, // enable extended register write
    0x82, 0x59, 0x33, // extended register access
    0x82, 0x5A, 0x02, // data for extended register
    0x82, 0x58, 0x01, // enable extended register write
    0x82, 0x5B, 0x00, // H-scaling control
    0x82, 0x5E, 0x01, // enable H-scaling control
    0x82, 0x6A, 0x00, // H-scaling control
    0x82, 0x28, 0x92, // cropping 0x92 0xB2
    0x82, 0x20, 0x24,
    0x82, 0x23, 0x11,   //V0_Start = 17
    0x82, 0x24, 0x05,   //V0_End = 261  V = 261 - 17 = 244
    0x82, 0x25, 0x11,
    0x82, 0x26, 0x00,
    0x82, 0x42, 0x00,
    0x82, 0x01, 0x04, // Brightness
	0x82, 0x02, 0x86, // Contrast
	0x82, 0x03, 0xC0, // saturation
	0x82, 0x04, 0x80, // hue
    0x82, 0x05, 0x03, // sharpness
    0x82, 0x57, 0x20, // black/white stretch
    0x82, 0x68, 0x32, // coring
    0x82, 0x37, 0x33,
    0x82, 0x61, 0x6C,

    0x82, 0x81, 0x01,

    // mipi link1
    0x82, 0xFF, 0x09, // switch to mipi tx1
    0x82, 0x00, 0x03, // enable bias
    0x82, 0xFF, 0x08, // switch to mipi csi1
    0x82, 0x04, 0x03, // csi1 and tx1 reset
    0x82, 0x6C, 0x11, // disable ch output; turn on ch0
    0x82, 0x06, 0x4C, // 2 lanes
    0x82, 0x21, 0x01, // enable hs clock
    0x82, 0x78, 0x68, // Y/C counts for ch0
    0x82, 0x79, 0x01, // Y/C counts for ch0
    0x82, 0x6C, 0x01, // enable ch output
    0x82, 0x04, 0x00, // csi1 and tx1 reset finish
    //0x07, 0x05, //enable non-continuous clock
    //0x20, 0xAA, // invert hs clock

    // mipi link3
    0x82, 0xFF, 0x0A, // switch to mipi csi3
    0x82, 0x6C, 0x10, // disable ch output; turn off ch0~3
};

uint8_t ubInitTable_HD_1lane_AHD[] = {
    // 720P@30 with mipi 1 data lanes + 1 clock lane out
    // pin24/23 data lane0, pin18/17 data lane1
    // pin16/15 data lane2, pin12/11 data lane3
    // pin14/13 clock lane
    // Slave address is 0x58
    // Register, data

    // if clock source(Xin) of RN6752 is 26MHz, please add these procedures marked first
    //0xD2, 0x85, // disable auto clock detect
    //0xD6, 0x37, // 27MHz default
    //0xD8, 0x18, // switch to 26MHz clock
    //delay(100), // delay 100ms

    0x82, 0x81, 0x01, // turn on video decoder
    0x82, 0xDF, 0xFE, // enable HD format

    // ch0
    0x82, 0xFF, 0x00, // switch to ch0 (default; optional)
    0x82, 0x00, 0x30, // internal use* 0x20=LPF off, 0x30=LPF on
    0x82, 0x06, 0x08, // internal use*
    0x82, 0x07, 0x63, // HD format
    0x82, 0x2A, 0x01, // filter control
    0x82, 0x3A, 0x20, // Insert Channel ID in SAV/EAV code
    0x82, 0x3F, 0x10, // channel ID
    0x82, 0x4C, 0x37, // equalizer
    0x82, 0x4F, 0x03, // sync control
    0x82, 0x50, 0x02, // 720p resolution
    0x82, 0x56, 0x01, // BT 72M mode
    0x82, 0x5F, 0x40, // blank level
    0x82, 0x63, 0xF5, // filter control
    0x55,
//    0x82, 0x59, 0x00, // extended register access
//    0x82, 0x5A, 0x42, // data for extended register     //44==>HD 30fps
//    0x82, 0x58, 0x01, // enable extended register write
    0x82, 0x59, 0x33, // extended register access
    0x82, 0x5A, 0x23, // data for extended register
    0x82, 0x58, 0x01, // enable extended register write
    0x82, 0x51, 0xE1, // scale factor1
    0x82, 0x52, 0x88, // scale factor2
    0x82, 0x53, 0x12, // scale factor3
    0x82, 0x5B, 0x07, // H-scaling control
    0x82, 0x5E, 0x08, // enable H-scaling control
    0x82, 0x6A, 0x82, // H-scaling control
    0x82, 0x28, 0x92, // cropping
	
    0x66, //! Image Color tuning
	//! AHD 720p 25
	0x01, 0x04, // Brightness
	0x02, 0x86, // Contrast
	0x03, 0xC0, // saturation
	0x04, 0x75, // hue
	0x05, 0x04, // sharpness(0x00~0x7F)
	0x57, 0x23, // black/white stretch(0x00~0x3F)
	//! AHD 720p 30
	0x01, 0x04, // Brightness
	0x02, 0x86, // Contrast
	0x03, 0xC0, // saturation
	0x04, 0x60, // hue
	0x05, 0x04, // sharpness(0x00~0x7F)
	0x57, 0x23, // black/white stretch(0x00~0x3F)
	
    0x82, 0x68, 0x32, // coring
    0x82, 0x37, 0x33,
    0x82, 0x61, 0x6C,

    0x82, 0x81, 0x01, // turn on video decoder

    // mipi link1
    0x82, 0xFF, 0x09, // switch to mipi tx1
    0x82, 0x00, 0x03, // enable bias
    0x82, 0xFF, 0x08, // switch to mipi csi1
    0x82, 0x04, 0x03, // csi1 and tx1 reset
    0x82, 0x6C, 0x11, // disable ch output; turn on ch0
    0x82, 0x06, 0x44, // 1 lanes
    0x82, 0x21, 0x01, // enable hs clock
    0x82, 0x78, 0x80, // Y/C counts for ch0
    0x82, 0x79, 0x02, // Y/C counts for ch0
    0x82, 0x6C, 0x01, // enable ch output
    0x82, 0x04, 0x00, // csi1 and tx1 reset finish
    //0x07, 0x05, //enable non-continuous clock
    //0x20, 0xAA, // invert hs clock
     
    // mipi link3
    0x82, 0xFF, 0x0A, // switch to mipi csi3
    0x82, 0x6C, 0x10, // disable ch output; turn off ch0~3
};

uint8_t ubInitTable_HD_1lane_TVI[] = {
    // 720P@25 with mipi 1 data lanes + 1 clock lane out
    // pin24/23 data lane0, pin18/17 data lane1
    // pin16/15 data lane2, pin12/11 data lane3
    // pin14/13 clock lane
    // Slave address is 0x58
    // Register, data

    // if clock source(Xin) of RN6752 is 26MHz, please add these procedures marked first
    //0xD2, 0x85, // disable auto clock detect
    //0xD6, 0x37, // 27MHz default
    //0xD8, 0x18, // switch to 26MHz clock
    //delay(100), // delay 100ms

    0x82, 0x81, 0x01, // turn on video decoder
    0x82, 0xDF, 0xFE, // enable HD format

    0x82, 0x88, 0x40, // disable SCLK0B out
    0x82, 0xF6, 0x40, // disable SCLK3A out

    // ch0
    0x82, 0xFF, 0x00, // switch to ch0 (default; optional)
    0x82, 0x00, 0x30, // internal use* 0x20=LPF off, 0x30=LPF on
    0x82, 0x06, 0x18, // internal use*
    0x82, 0x07, 0x63, // HD format
    0x82, 0x2A, 0x01, // filter control
    0x82, 0x3A, 0x20, // Insert Channel ID in SAV/EAV code
    0x82, 0x3F, 0x10, // channel ID
    0x82, 0x4C, 0x37, // equalizer
    0x82, 0x4F, 0x03, // sync control
    0x82, 0x50, 0x02, // 720p resolution
    0x82, 0x56, 0x01, // 72M mode and BT656 mode
    0x82, 0x5F, 0x40, // blank level
    0x82, 0x63, 0xF6, // filter control
    0x55,
//    0x82, 0x59, 0x00, // extended register access
//    0x82, 0x5A, 0x4A, // data for extended register
//    0x82, 0x58, 0x01, // enable extended register write
    0x82, 0x59, 0x33, // extended register access
    0x82, 0x5A, 0x23, // data for extended register
    0x82, 0x58, 0x01, // enable extended register write
    0x82, 0x51, 0xC1, // scale factor1
    0x82, 0x52, 0x88, // scale factor2
    0x82, 0x53, 0x14, // scale factor3
    0x82, 0x5B, 0x07, // H-scaling control
    0x82, 0x5E, 0x08, // enable H-scaling control
    0x82, 0x6A, 0x88, // H-scaling control
    0x82, 0x28, 0x92, // cropping
	
    0x66, //! Image Color tuning
	//! TVI 720p 25
	0x01, 0x04, // Brightness
	0x02, 0x86, // Contrast
	0x03, 0xC0, // saturation
	0x04, 0x80, // hue
	0x05, 0x03, // sharpness(0x00~0x7F)
	0x57, 0x23, // black/white stretch(0x00~0x3F)
	//! TVI 720p 30
	0x01, 0x04, // Brightness
	0x02, 0x86, // Contrast
	0x03, 0xC0, // saturation
	0x04, 0x80, // hue
	0x05, 0x03, // sharpness(0x00~0x7F)
	0x57, 0x23, // black/white stretch(0x00~0x3F)
	
    0x82, 0x68, 0x32, // coring
    0x82, 0x37, 0x33,
    0x82, 0x61, 0x6C,

    0x82, 0x81, 0x01, // turn on video decoder

    // mipi link1
    0x82, 0xFF, 0x09, // switch to mipi tx1
    0x82, 0x00, 0x03, // enable bias
    0x82, 0xFF, 0x08, // switch to mipi csi1
    0x82, 0x04, 0x03, // csi1 and tx1 reset
    0x82, 0x6C, 0x11, // disable ch output; turn on ch0
    0x82, 0x06, 0x44, // 1 lanes
    0x82, 0x21, 0x01, // enable hs clock
    0x82, 0x78, 0x80, // Y/C counts for ch0
    0x82, 0x79, 0x02, // Y/C counts for ch0
    0x82, 0x6C, 0x01, // enable ch output
    0x82, 0x04, 0x00, // csi1 and tx1 reset finish
    //0x07, 0x05, //enable non-continuous clock
    //0x20, 0xAA, // invert hs clock

    // mipi link3
    0x82, 0xFF, 0x0A, // switch to mipi csi3
    0x82, 0x6C, 0x10, // disable ch output; turn off ch0~3
};

uint8_t ubInitTable_HD_1lane_CVI[] = {
    // 720P@25 with mipi 1 data lanes + 1 clock lane out
    // pin24/23 data lane0, pin18/17 data lane1
    // pin16/15 data lane2, pin12/11 data lane3
    // pin14/13 clock lane
    // Slave address is 0x58
    // Register, data

    // if clock source(Xin) of RN6752 is 26MHz, please add these procedures marked first
    //0xD2, 0x85, // disable auto clock detect
    //0xD6, 0x37, // 27MHz default
    //0xD8, 0x18, // switch to 26MHz clock
    //delay(100), // delay 100ms

    0x82, 0x81, 0x01, // turn on video decoder
    0x82, 0xDF, 0xFE, // enable HD format

    0x82, 0x88, 0x40, // disable SCLK0B out
    0x82, 0xF6, 0x40, // disable SCLK3A out

    // ch0
    0x82, 0xFF, 0x00, // switch to ch0 (default; optional)
    0x82, 0x00, 0x30, // internal use* 0x20=LPF off, 0x30=LPF on
    0x82, 0x06, 0x18, // internal use*
    0x82, 0x07, 0x63, // HD format
    0x82, 0x2A, 0x01, // filter control
    0x82, 0x3A, 0x20, // Insert Channel ID in SAV/EAV code
    0x82, 0x3F, 0x10, // channel ID
    0x82, 0x4C, 0x37, // equalizer
    0x82, 0x4F, 0x03, // sync control
    0x82, 0x50, 0x02, // 720p resolution
    0x82, 0x56, 0x01, // 72M mode and BT656 mode
    0x82, 0x5F, 0x44, // blank level
    0x82, 0x63, 0xF6, // filter control
    0x55,
//    0x82, 0x59, 0x00, // extended register access
//    0x82, 0x5A, 0x43, // data for extended register
//    0x82, 0x58, 0x01, // enable extended register write
    0x82, 0x59, 0x33, // extended register access
    0x82, 0x5A, 0x23, // data for extended register
    0x82, 0x58, 0x01, // enable extended register write
    0x82, 0x51, 0x11, // scale factor1
    0x82, 0x52, 0x94, // scale factor2
    0x82, 0x53, 0x15, // scale factor3
    0x82, 0x5B, 0x00, // H-scaling control
    0x82, 0x5E, 0x00, // enable H-scaling control
    0x82, 0x6A, 0x84, // H-scaling control
    0x82, 0x28, 0x92, // cropping
	
	0x66, //! Image Color tuning
	//! CVI 720p 25
	0x01, 0x02, // Brightness
	0x02, 0x86, // Contrast
	0x03, 0xC0, // saturation
	0x04, 0xE2, // hue
	0x05, 0x03, // sharpness(0x00~0x7F)
	0x57, 0x23, // black/white stretch(0x00~0x3F)
	//! CVI 720p 30
	0x01, 0x02, // Brightness
	0x02, 0x86, // Contrast
	0x03, 0xC0, // saturation
	0x04, 0xE0, // hue
	0x05, 0x03, // sharpness(0x00~0x7F)
	0x57, 0x23, // black/white stretch(0x00~0x3F)
	
    0x82, 0x68, 0x32, // coring
    0x82, 0x37, 0x33,
    0x82, 0x61, 0x6C,

    0x82, 0x81, 0x01, // turn on video decoder

    // mipi link1
    0x82, 0xFF, 0x09, // switch to mipi tx1
    0x82, 0x00, 0x03, // enable bias
    0x82, 0xFF, 0x08, // switch to mipi csi1
    0x82, 0x04, 0x03, // csi1 and tx1 reset
    0x82, 0x6C, 0x11, // disable ch output; turn on ch0
    0x82, 0x06, 0x44, // 1 lanes
    0x82, 0x21, 0x01, // enable hs clock
    0x82, 0x78, 0x80, // Y/C counts for ch0
    0x82, 0x79, 0x02, // Y/C counts for ch0
    0x82, 0x6C, 0x01, // enable ch output
    0x82, 0x04, 0x00, // csi1 and tx1 reset finish
    //0x07, 0x05, //enable non-continuous clock
    //0x20, 0xAA, // invert hs clock

    // mipi link3
    0x82, 0xFF, 0x0A, // switch to mipi csi3
    0x82, 0x6C, 0x10, // disable ch output; turn off ch0~3
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
    tAE_SensorPara.ulSensorPixelPerLine = 1440;
    tAE_SensorPara.ulSensorLinePerFrame = 1000;
	tAE_SensorPara.ulSensorFrameRate = ubPclkIdx;     
    tAE_SensorPara.ulMaximumSensorFrameRate = (sensor_state.ubVideoFPS == 1)? SEN_FPS30: SEN_FPS25;
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
			MIPI->DLAN_PDD_LSCALE_SEL0 = 8;	
			break;       
		case SEN_FPS30:
			MIPI->DLAN_PDD_LSCALE_SEL0 = 8;
			break;
		default:
			MIPI->DLAN_PDD_LSCALE_SEL0 = 8;
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


//------------------------------------------------------------------------------//
//                         RN6752 control register                              //
//------------------------------------------------------------------------------//
//------------------------------------------------------------------------------
void SEN_SetBrightness(uint8_t ubBrit)
{
	bSEN_I2C_Write(0xFF, 0x00);
    bSEN_I2C_Write(0x01, ubBrit);
}

//------------------------------------------------------------------------------
void SEN_SetContrast(uint8_t ubCont)
{
	bSEN_I2C_Write(0xFF, 0x00);
    bSEN_I2C_Write(0x02, ubCont);
}

//------------------------------------------------------------------------------
void SEN_SetSaturation(uint8_t ubSatu)
{
	bSEN_I2C_Write(0xFF, 0x00);
    bSEN_I2C_Write(0x03, ubSatu);
}

//------------------------------------------------------------------------------
void SEN_SetHue(uint8_t ubHue)
{
	bSEN_I2C_Write(0xFF, 0x00);
    bSEN_I2C_Write(0x04, ubHue);
}
//------------------------------------------------------------------------------
static void SEN_PreventColorErr(void)
{
	uint8_t *pDataBuf;
	uint8_t ubRegValue;
	
	pDataBuf = (uint8_t *)&ubRegValue;
	bSEN_I2C_Write(0xFF, 0x00);
	bSEN_I2C_Read(0x7F, pDataBuf);
	
	ubRegValue >>= 4;
	if ( (ubRegValue > 0) && (ubRegValue < 10) )
	{
		bSEN_I2C_Write(0x03, 0xC0);
	}
	else
	{
		bSEN_I2C_Write(0x03, 0x00);
	}
	
	if ( ubRegValue > 10 )
	{
		bSEN_I2C_Write(0x0D, 0x15);
	}
}

//------------------------------------------------------------------------------
uint8_t ubSEN_CheckSensorState(void)
{
	uint8_t     *pBuf;	
	uint16_t 	uwPID = 0;

	pBuf = (uint8_t*)&uwPID;
	// I2C by Read Sensor ID
	bSEN_I2C_Read (RN6752_CHIP_ID_HIGH_ADDR, &pBuf[1]);
	bSEN_I2C_Read (RN6752_CHIP_ID_LOW_ADDR, &pBuf[0]);

	if (RN6752_CHIP_ID != uwPID)
	{
		printd(DBG_ErrorLvl, "Read chip ID fail.<0x%x 0x%x>\n", RN6752_CHIP_ID, uwPID);
        return 0;
	}
    return 1;
}

//------------------------------------------------------------------------------
uint8_t ubSEN_GetFieldState(void)
{
    uint8_t ubReg_0x00;
    
    bSEN_I2C_Read(RN6752_REG_0x00, &ubReg_0x00);
    sensor_state.ubField = (((ubReg_0x00 & RN6752_CVBS_FIELD)>>7) == 0x0)? RN6752_EVEN_FIELD:RN6752_ODD_FIELD;
    return sensor_state.ubField;
}

//------------------------------------------------------------------------------
void SEN_DetectState(void)
{
    uint8_t ubTemp;
    uint8_t ubReg_0x00 = 0;
    uint8_t ubReg_0x4B = 0;
    
    bSEN_I2C_Write(RN6752_PAGE_ADDR, RN6752_PAGE_0);
    bSEN_I2C_Read(RN6752_REG_0x00, &ubReg_0x00);
    bSEN_I2C_Read(RN6752_REG_0x4B, &ubReg_0x4B);
    // Detect flow
    //  1. check input signal.
    //  2. check inpur resolution and frame rate.
    //  3. if resolution is 720P, check 720P_format.
    if(((ubReg_0x00 & RN6752_VIDEO_DETECT)>>4) == RN6752_ValidVideo)
    {
        ubTemp = ((ubReg_0x00 & RN6752_VIDEO_FORMAT)>>5);
        sensor_state.ubVideoFormat   = (ubTemp == 0x1)? RN6752_720P : (ubTemp == 0x2)? RN6752_1080P : RN6752_NTSC_PAL;
        if(sensor_state.ubVideoFormat == RN6752_NTSC_PAL)
        {
            ubTemp = ((ubReg_0x00 & RN6752_CVBS_FIELD)>>7);
            sensor_state.ubField    = (ubTemp == 0x0)? RN6752_EVEN_FIELD:RN6752_ODD_FIELD;        
        }
        ubTemp = ((ubReg_0x00 & RN6752_FRAME_DETECT)>>0);
        sensor_state.ubVideoFPS      = (ubTemp == 0x1)? RN6752_30fps : RN6752_25fps;
        if(sensor_state.ubVideoFormat == RN6752_720P)
        {
            ubTemp = ((ubReg_0x4B & RN6752_720P_FORMAT)>>0);
            if(ubTemp == 0x0)
            {
                sensor_state.ubVideoType = RN6752_AHD;
            }else if(ubTemp == 0x1){
                sensor_state.ubVideoType = RN6752_TVI;
            }else if(ubTemp == 0x3){
                bSEN_I2C_Write(0x4B, 0x18);
                TIMER_Delay_ms(10);
                bSEN_I2C_Read(RN6752_REG_0x4B, &ubReg_0x4B);
                ubTemp = ((ubReg_0x4B & RN6752_720P_FORMAT)>>0);
                sensor_state.ubVideoType = (ubTemp == 0x3)? RN6752_CVI:RN6752_AHD;
            }
            bSEN_I2C_Write(0x4B, 0x08);
            //sensor_state.ubVideoType = (ubTemp == 0x1)? RN6752_TVI : (ubTemp == 0x3)? RN6752_CVI : RN6752_AHD;        
        }else{
            sensor_state.ubVideoType = RN6752_AHD;
        }

        sensor_state.ubVliadVideo = SEN_ValidVideo;
    }else{
        sensor_state.ubVliadVideo = SEN_NoVideo;
    }
}

//------------------------------------------------------------------------------
static void SEN_CheckStateThread(void const *argument)
{
    uint32_t ulTemp;
    uint8_t ubDoReSensorFlg	     = 0;
    uint8_t ubVideoCnt           = 0;
    uint8_t ubNoVideoCnt         = 0;
    uint8_t ubCloseVideoFlg      = 0;
    uint8_t ubStableFlag         = 0;    
    uint8_t ubThreadCnt          = 0;
    uint8_t ubThreadFg           = 0;
	uint8_t ubPreventColorErrCnt = 0;

    while(1)
    {
        if(ubThreadFg)
        {			
            SEN_DetectState();
            //printf("<%d %d %d %d 0x%x> ",sensor_state.ubVliadVideo, sensor_state.ubVideoFormat, sensor_state.ubVideoFPS, sensor_state.ubVideoType, sensor_state.ubInputType);
            if(sensor_state.ubVliadVideo == RN6752_NoVideo)
            {
               //Close Video
               ubVideoCnt = 0;
               if((ubNoVideoCnt++) >= 2)
                    SEN_SetDetectVideoState(0);
               ubStableFlag = 0;
			   ubPreventColorErrCnt = 0;
            }else{
                if((ubVideoCnt++) >= 2)
                {
                    ubStableFlag = 1;
                }
                if(ubStableFlag)
                {
                    if(sensor_state.ubOldVideoFormat != sensor_state.ubVideoFormat)
                        ubDoReSensorFlg = 1;
                    if(sensor_state.ubOldVideoFPS != sensor_state.ubVideoFPS)
                        ubDoReSensorFlg = 1;
                    if(sensor_state.ubVideoFormat == RN6752_720P)
                    {
                        if(sensor_state.ubOldVideoType != sensor_state.ubVideoType)
                            ubDoReSensorFlg = 1;                    
                    }
                    
                    if(ubDoReSensorFlg)
                    {
                        if(ubSEN_GetDetectVideoState() != 1)
                        {
                            SEN_ReSensorInit();
                            SEN_SetISPWindowSize();
                            ubSEN_InitProcess();
                            {
                                BUF_Reset(BUF_SEN_1_YUV);
                                ulTemp = ulBUF_GetSen1YuvFreeBuf();
                                SEN_SetPathAddr(SENSOR_PATH1,ulTemp);
                                SEN_UpdatePathAddr();
                            }
                            ubDoReSensorFlg = 0;
                        }                      
                        ubCloseVideoFlg = 1;
                    }
                    //Open video
                    ubNoVideoCnt = 0;
                    if(ubCloseVideoFlg)
                    {
                        SEN_SetDetectVideoState(0);
                        ubCloseVideoFlg = 0;
                    }else{
                        SEN_SetDetectVideoState(1);
                    }
					//! Correct Saturation
					if (ubPreventColorErrCnt < 140)
					{
//						printf("Waiting for RN6752M stable(%d)...\r\n", ubPreventColorErrCnt);
						ubPreventColorErrCnt++;
						SEN_PreventColorErr();
					}
					else if (ubPreventColorErrCnt == 140)
					{
//						printf("Prevent Saturation = 0x00;\r\n");
						ubPreventColorErrCnt++;
						SEN_SetSaturation(0xC0);
					}
                }
            }        
        }
        
        if(((ubThreadCnt++) >= 2) && (ubThreadFg == 0))
        {
            ubThreadFg = 1;
        }
		
        osDelay(300);
    }
}

//------------------------------------------------------------------------------
void SEN_SetModeControl(uint8_t ubFormat, uint8_t ubType, uint8_t ubFPS)
{
    bSEN_I2C_Write(RN6752_PAGE_ADDR, RN6752_PAGE_0);
    bSEN_I2C_Write(0x59, 0x00);
    if(ubFormat == RN6752_720P)
    {
        if(ubFPS == RN6752_25fps)
        {
            if(ubType == RN6752_AHD)
            {
                bSEN_I2C_Write(0x5A, 0x42);
                sensor_state.ubInputType = SENSOR_AHD_720P_PAL;
            }
            else if(ubType == RN6752_CVI)
            {
                bSEN_I2C_Write(0x5A, 0x43);
                sensor_state.ubInputType = SENSOR_CVI_720P_PAL;
            }
            else if(ubType == RN6752_TVI)
            {
                bSEN_I2C_Write(0x5A, 0x4A);
                sensor_state.ubInputType = SENSOR_TVI_720P_PAL;
            }
        }else if(ubFPS == RN6752_30fps){
            if(ubType == RN6752_AHD)
            {
                bSEN_I2C_Write(0x5A, 0x44);
                sensor_state.ubInputType = SENSOR_AHD_720P_NTSC;
            }
            else if(ubType == RN6752_CVI)
            {
                bSEN_I2C_Write(0x5A, 0x47);
                sensor_state.ubInputType = SENSOR_CVI_720P_NTSC;
            }
            else if(ubType == RN6752_TVI)
            {
                bSEN_I2C_Write(0x5A, 0x4B);
                sensor_state.ubInputType = SENSOR_TVI_720P_NTSC;
            }   
        }
    }else if(ubFormat == RN6752_1080P){
        if(ubFPS == RN6752_25fps)
        {
            bSEN_I2C_Write(0x5A, 0x48); 
            sensor_state.ubInputType = SENSOR_AHD_1080P_PAL;
        }
        else
        {
            bSEN_I2C_Write(0x5A, 0x49);
            sensor_state.ubInputType = SENSOR_AHD_1080P_NTSC;
        }
    }
    bSEN_I2C_Write(0x58, 0x01);
}

//------------------------------------------------------------------------------//
//      Configure follow :                                                      //
//              RN675x_reset--->RN675xM_Pre_initial--->RN675x_init_cfg          //
//------------------------------------------------------------------------------//
//------------------------------------------------------------------------------
void SEN_SetRN6752HwReset(void)
{
	//HW reset RN6752
    RN6752_RESET_OUT_EN = 1;
	RN6752_RESET_OUT = 1;
	RN6752_RESET_OUT = 0;
	TIMER_Delay_ms(30);
	RN6752_RESET_OUT = 1;
	TIMER_Delay_ms(60);
}

//------------------------------------------------------------------------------
static void SEN_PreInit(void)
{
	uint8_t rom_byte1, rom_byte2, rom_byte3, rom_byte4, rom_byte5, rom_byte6;

	bSEN_I2C_Write(0xE1, 0x80);
	bSEN_I2C_Write(0xFA, 0x81);
	bSEN_I2C_Read (0xFB, &rom_byte1);
	bSEN_I2C_Read (0xFB, &rom_byte2);
	bSEN_I2C_Read (0xFB, &rom_byte3);
	bSEN_I2C_Read (0xFB, &rom_byte4);
	bSEN_I2C_Read (0xFB, &rom_byte5);
	bSEN_I2C_Read (0xFB, &rom_byte6);

	// config. decoder accroding to rom_byte5 and rom_byte6
	if ((rom_byte6 == 0x00) && (rom_byte5 == 0x00))
	{
		bSEN_I2C_Write(0xEF, 0xAA);  
		bSEN_I2C_Write(0xE7, 0xFF);
		bSEN_I2C_Write(0xFF, 0x09);
		bSEN_I2C_Write(0x03, 0x0C);
		bSEN_I2C_Write(0xFF, 0x0B);
		bSEN_I2C_Write(0x03, 0x0C);
	}
	else if (((rom_byte6 == 0x34) && (rom_byte5 == 0xA9)) ||
	 ((rom_byte6 == 0x2C) && (rom_byte5 == 0xA8)))
	{
		bSEN_I2C_Write(0xEF, 0xAA);  
		bSEN_I2C_Write(0xE7, 0xFF);
		bSEN_I2C_Write(0xFC, 0x60);
		bSEN_I2C_Write(0xFF, 0x09);
		bSEN_I2C_Write(0x03, 0x18);
		bSEN_I2C_Write(0xFF, 0x0B);
		bSEN_I2C_Write(0x03, 0x18);
	}
	else
	{
		bSEN_I2C_Write(0xEF, 0xAA);  
		bSEN_I2C_Write(0xFC, 0x60);
		bSEN_I2C_Write(0xFF, 0x09);
		bSEN_I2C_Write(0x03, 0x18);
		bSEN_I2C_Write(0xFF, 0x0B);
		bSEN_I2C_Write(0x03, 0x18);	
	}
}
//------------------------------------------------------------------------------
uint8_t ubSEN_SetSensorInitTable(void)
{
    I2C_SCL_SPEED_TYP tI2C_Clock = I2C_SCL_400K;
    static bool bInitFlg = FALSE;
    uint8_t     ubReg_0x00 = 0;
    uint8_t     ubCount_bit4 = 0;
	uint32_t 	i, j, k;
	
	uint16_t uwMinResH;
    uint16_t uwMinResV;

    // HW reset
    SEN_SetRN6752HwReset();
    // Init I2C
	pI2C_type = pI2C_MasterInit(I2C_2, tI2C_Clock);
	IQ_SetI2cType(pI2C_type, tI2C_Clock);
    
    if(ubSEN_CheckSensorState() != 1)
        return 0;
    printd(DBG_CriticalLvl, "RN6752 Sensor\n");
    
    // RICHNEX fae suggest.
    bSEN_I2C_Write(0x4B, 0x08);
    bSEN_I2C_Write(0x49, 0x81);
    // ==>(15-7 = 8)*33.3ms =266.64ms
    bSEN_I2C_Write(0x19, 0x07);    
    // First detect video format, video type and frame rate.
    if(bInitFlg == FALSE)
    {
        bSEN_I2C_Write(RN6752_PAGE_ADDR, RN6752_PAGE_0);
        for (i=0; i<100; i++ ) 
        {
            TIMER_Delay_ms(1);
            bSEN_I2C_Read(RN6752_REG_0x00, &ubReg_0x00);
            if(((ubReg_0x00 & RN6752_VIDEO_DETECT)>>4) == RN6752_NoVideo)
            {
                ubCount_bit4 = 0;
                continue;
            }else{
                if(ubCount_bit4++ > 10)
                {
                    break;
                }
            }
        }
        // Wait 1.0s for RN6752 stable.
        TIMER_Delay_ms(1000);
        SEN_DetectState();
        //printf("<0x%x 0x%x 0x%x>", sensor_state.ubVideoFormat, sensor_state.ubVideoFPS, sensor_state.ubVideoType);
        if(sensor_state.ubVliadVideo == RN6752_NoVideo)
        {
            sensor_state.ubVideoFormat      = RN6752_1080P;
            sensor_state.ubVideoFPS         = RN6752_30fps;
            sensor_state.ubVideoType        = RN6752_AHD;
        }
        bInitFlg = TRUE;
    }
 
    SEN_PreInit();
    if(sensor_state.ubVideoFormat == RN6752_1080P){
        for (i=0; i<sizeof(ubInitTable_FHD_2lane); )
        {
            if (ubInitTable_FHD_2lane[i] == 0x82)
            {
                bSEN_I2C_Write(ubInitTable_FHD_2lane[i+1], ubInitTable_FHD_2lane[i+2]);
                i+=3;
            }else if (ubInitTable_FHD_2lane[i] == 0x55){
                SEN_SetModeControl(sensor_state.ubVideoFormat, sensor_state.ubVideoType, sensor_state.ubVideoFPS);
                i+=1;
            }else if (ubInitTable_FHD_2lane[i] == 0x66) {
				k = (sensor_state.ubVideoFPS == RN6752_25fps)?0:12;
				for ( j = 0; j < 6; j++ ) {
					bSEN_I2C_Write(ubInitTable_FHD_2lane[i+1+2*j+k], ubInitTable_FHD_2lane[i+1+(2*j+1)+k]);
				}
				i+=25;
			}
        }
        uwMinResH = 1920;
        uwMinResV = 1088;
		SEN_SetAhdCamRes(uwMinResH,uwMinResV);
    }else if(sensor_state.ubVideoFormat == RN6752_720P){
        if(sensor_state.ubVideoType == RN6752_AHD)
        {
            for (i=0; i<sizeof(ubInitTable_HD_1lane_AHD); )
            {
                if (ubInitTable_HD_1lane_AHD[i] == 0x82)
                {
                    bSEN_I2C_Write(ubInitTable_HD_1lane_AHD[i+1], ubInitTable_HD_1lane_AHD[i+2]);
                    i+=3;
                }else if (ubInitTable_HD_1lane_AHD[i] == 0x55){
                    SEN_SetModeControl(sensor_state.ubVideoFormat, sensor_state.ubVideoType, sensor_state.ubVideoFPS);
                    i+=1;
                }else if (ubInitTable_HD_1lane_AHD[i] == 0x66) {
					k = (sensor_state.ubVideoFPS == RN6752_25fps)?0:12;
					for ( j = 0; j < 6; j++ ) {
						bSEN_I2C_Write(ubInitTable_HD_1lane_AHD[i+1+2*j+k], ubInitTable_HD_1lane_AHD[i+1+(2*j+1)+k]);
					}
					i+=25;
				}
            }
        }
        if(sensor_state.ubVideoType == RN6752_TVI)
        {
            for (i=0; i<sizeof(ubInitTable_HD_1lane_TVI); )
            {
                if (ubInitTable_HD_1lane_TVI[i] == 0x82)
                {
                    bSEN_I2C_Write(ubInitTable_HD_1lane_TVI[i+1], ubInitTable_HD_1lane_TVI[i+2]);
                    i+=3;
                }else if (ubInitTable_HD_1lane_TVI[i] == 0x55){
                    SEN_SetModeControl(sensor_state.ubVideoFormat, sensor_state.ubVideoType, sensor_state.ubVideoFPS);
                    i+=1;
                }else if (ubInitTable_HD_1lane_TVI[i] == 0x66) {
					k = (sensor_state.ubVideoFPS == RN6752_25fps)?0:12;
					for ( j = 0; j < 6; j++ ) {
						bSEN_I2C_Write(ubInitTable_HD_1lane_TVI[i+1+2*j+k], ubInitTable_HD_1lane_TVI[i+1+(2*j+1)+k]);
					}
					i+=25;
				}
            }
        }
        if(sensor_state.ubVideoType == RN6752_CVI)
        {
            for (i=0; i<sizeof(ubInitTable_HD_1lane_CVI); )
            {
                if (ubInitTable_HD_1lane_CVI[i] == 0x82)
                {
                    bSEN_I2C_Write(ubInitTable_HD_1lane_CVI[i+1], ubInitTable_HD_1lane_CVI[i+2]);
                    i+=3;
                }else if (ubInitTable_HD_1lane_CVI[i] == 0x55){
                    SEN_SetModeControl(sensor_state.ubVideoFormat, sensor_state.ubVideoType, sensor_state.ubVideoFPS);
                    i+=1;
                }else if (ubInitTable_HD_1lane_CVI[i] == 0x66) {
					k = (sensor_state.ubVideoFPS == RN6752_25fps)?0:12;
					for ( j = 0; j < 6; j++ ) {
						bSEN_I2C_Write(ubInitTable_HD_1lane_CVI[i+1+2*j+k], ubInitTable_HD_1lane_CVI[i+1+(2*j+1)+k]);
					}
					i+=25;
				}
            }
        }	
        uwMinResH = 1280;
        uwMinResV = 720;
		SEN_SetAhdCamRes(uwMinResH,uwMinResV);
    }else if(sensor_state.ubVideoFormat == RN6752_NTSC_PAL){
        if(sensor_state.ubVideoFPS == RN6752_25fps)
        {
            for (i=0; i<sizeof(ubInitTable_D1_PAL_2lane); i+=3)
            {
                if (ubInitTable_D1_PAL_2lane[i] == 0x82)
                {
                    bSEN_I2C_Write(ubInitTable_D1_PAL_2lane[i+1], ubInitTable_D1_PAL_2lane[i+2]);
                }
            }
            sensor_state.ubInputType = SENSOR_CVBS_PAL;
        }
        if(sensor_state.ubVideoFPS == RN6752_30fps)
        {
            for (i=0; i<sizeof(ubInitTable_D1_NTSC_2lane); i+=3)
            {
                if (ubInitTable_D1_NTSC_2lane[i] == 0x82)
                {
                    bSEN_I2C_Write(ubInitTable_D1_NTSC_2lane[i+1], ubInitTable_D1_NTSC_2lane[i+2]);
                }
            }
            sensor_state.ubInputType = SENSOR_CVBS_NTSC;
        }
		uwMinResH = 640;
		uwMinResV = 480;
		SEN_SetAhdCamRes(uwMinResH,uwMinResV);
    }
    printf("Input type: %s %s %s\r\n",	(sensor_state.ubVideoType == RN6752_AHD)?"AHD":
										(sensor_state.ubVideoType == RN6752_TVI)?"TVI":
										(sensor_state.ubVideoType == RN6752_CVI)?"CVI":"",
										(sensor_state.ubVideoFormat == RN6752_1080P)?"1080P":
										(sensor_state.ubVideoFormat == RN6752_720P)?"720P":"D1",
										(sensor_state.ubVideoFPS == RN6752_25fps)?"PAL":"NTSC");
	
    // save current state.
    sensor_state.ubOldVideoFormat   = sensor_state.ubVideoFormat;
    sensor_state.ubOldVideoFPS      = sensor_state.ubVideoFPS ;
    sensor_state.ubOldVideoType     = sensor_state.ubVideoType;
	return 1;
}

//------------------------------------------------------------------------------
uint8_t ubSEN_Open(struct SENSOR_SETTING *setting)
{
    osThreadDef(SEN_CheckState, SEN_CheckStateThread, osPriorityNormal, 1, 1024);
    osThreadCreate(osThread(SEN_CheckState), NULL);
    SEN_SyncCvbsFieldCbFunc(ubSEN_GetFieldState);
    
    SEN_AeCbFunc();
    // Set ISP clock
    SEN_SetISPRate(4);
    // Set mipi mode
    SEN->MIPI_MODE = 1;
    // Power down MIPI analog phy.
    MIPI->MIPI_PD = 0;
    // Set sensor clock
    SEN_SetSensorRate(SENSOR_96MHz, 4);
    // Enable sensor clock
    SEN->SEN_CLK_EN = 1;
    //Set ISP pipeline
    sensor_cfg.tPathType = YUV_SENSOR_YUV_IN;
    SEN_SenosrOutputType(sensor_cfg.tPathType);
    // change ISP_LH_SEL to 1, for D2-D9 input
    SEN->ISP_LH_SEL = 1;
    // Set initial table
    if (ubSEN_SetSensorInitTable() != 1)
    {
        printf("RN6752 startup failed! \n\r");
        return 0;
    }
    // Enable MIPI and set MIPI channel select.
    if(sensor_state.ubVideoFormat == RN6752_720P)
        SEN_MIPIControl(MIPI_1LANE, MIPI_CHANNEL0);
    else if((sensor_state.ubVideoFormat == RN6752_1080P) || (sensor_state.ubVideoFormat == RN6752_NTSC_PAL))
        SEN_MIPIControl(MIPI_2LANE, MIPI_CHANNEL0);
	MIPI_PowerDownTimingSelect((sensor_state.ubVideoFPS == 1)? SEN_FPS30: SEN_FPS25);
	MIPI_AutoPhaseDetect();
	SEN_PclkSetting((sensor_state.ubVideoFPS == 1)? SEN_FPS30: SEN_FPS25);
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
void SEN_ReSensorInit(void)
{
    // Set initial table
    if (ubSEN_SetSensorInitTable() != 1)
    {
        printf("RN6752 startup failed! \n\r");
    }
    // Enable MIPI and set MIPI channel select.
    if(sensor_state.ubVideoFormat == RN6752_720P)
        SEN_MIPIControl(MIPI_1LANE, MIPI_CHANNEL0);
    else if((sensor_state.ubVideoFormat == RN6752_1080P) || (sensor_state.ubVideoFormat == RN6752_NTSC_PAL))
        SEN_MIPIControl(MIPI_2LANE, MIPI_CHANNEL0);
	MIPI_PowerDownTimingSelect((sensor_state.ubVideoFPS == 1)? SEN_FPS30: SEN_FPS25);
	MIPI_AutoPhaseDetect();
	SEN_PclkSetting((sensor_state.ubVideoFPS == 1)? SEN_FPS30: SEN_FPS25);
}

//------------------------------------------------------------------------------
uint16_t uwSEN_CalExpLine(uint32_t ulAlgExpTime)
{
    return (ulSEN_GetPixClk() / 1000000L * (uint32_t)ulAlgExpTime / (uint32_t)ulSEN_GetPckPerLine() / 10);
}

//------------------------------------------------------------------------------
void SEN_CalExpLDmyL(uint32_t ulAlgExpTime)
{

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

}

//------------------------------------------------------------------------------
void SEN_SetExpLine(void)
{
    SEN_WrExpLine(xtSENInst.xtSENCtl.uwExpLine);
}

//------------------------------------------------------------------------------
void SEN_WrExpLine(uint16_t uwExpLine)
{

}

//------------------------------------------------------------------------------
void SEN_WrMaxExpLine(uint32_t ulValue)
{
    xtSENInst.uwMaxExpLine = ulValue;
}

//------------------------------------------------------------------------------
void SEN_WrGain(uint32_t ulGainX1024)
{
    // update struct value.
    xtSENInst.xtSENCtl.uwGain = ulGainX1024;
}

//------------------------------------------------------------------------------
void SEN_SetMirrorFlip(uint8_t ubMirrorEn, uint8_t ubFlipEn)
{

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
    if(sensor_state.ubVideoFormat == RN6752_720P)
    {
        sensor_cfg.xtSENWin.uwHSize = 1280;
        sensor_cfg.xtSENWin.uwVSize = 720;
        sensor_cfg.xtSENWin.uwHStart = 0;
        sensor_cfg.xtSENWin.uwVStart = 0;
    }else if(sensor_state.ubVideoFormat == RN6752_1080P){
        sensor_cfg.xtSENWin.uwHSize = 1920;
        sensor_cfg.xtSENWin.uwVSize = 1080;
        sensor_cfg.xtSENWin.uwHStart = 0;
        sensor_cfg.xtSENWin.uwVStart = 0;
    }else if(sensor_state.ubVideoFormat == RN6752_NTSC_PAL){
        if(sensor_state.ubVideoFPS == RN6752_25fps)
        {
            sensor_cfg.xtSENWin.uwHSize = 720;
            sensor_cfg.xtSENWin.uwVSize = (544/2);
            sensor_cfg.xtSENWin.uwHStart = 0;
            sensor_cfg.xtSENWin.uwVStart = 0;
        }else{
            sensor_cfg.xtSENWin.uwHSize = 720;
            sensor_cfg.xtSENWin.uwVSize = (480/2);
            sensor_cfg.xtSENWin.uwHStart = 0;
            sensor_cfg.xtSENWin.uwVStart = 0;
        }
    }
}

//------------------------------------------------------------------------------
void SEN_SetSensorType(void)
{
    sensor_cfg.ubSensorType = SEN_RN6752;
    printd(DBG_Debug1Lvl, "sensor type is RN6752\n");
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

void SEN_SetEnvironment(void)
{

}
#endif
