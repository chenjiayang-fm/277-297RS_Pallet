/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		ISP_API.h
	\brief		ISP API header
	\author			
	\version	1.4
	\date		2020-05-19
	\copyright	Copyright(C) 2020 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _ISP_API_H_
#define _ISP_API_H_
#include "_510PF.h"

//==============================================================================
// DEFINITION
//==============================================================================
#define DS_TEXT_NUM 32

#define ISP_FuncEnable											\
						{										\
							GLB->ISP_FUNC_DIS 	 = 0;			\
							GLB->ISP_MD_FUNC_DIS = 0;			\
							GLB->ISP_DIS_FUNC_DIS = 0;			\
							GLB->ISP_YUV_PATH1_FUNC_DIS = 0;	\
							GLB->ISP_YUV_PATH2_FUNC_DIS = 0;	\
							GLB->ISP_YUV_PATH3_FUNC_DIS = 0;	\
						}
#define ISP_FuncDisable											\
						{										\
							GLB->ISP_FUNC_DIS 	 = 1;			\
							GLB->ISP_MD_FUNC_DIS = 1;			\
							GLB->ISP_DIS_FUNC_DIS = 1;			\
							GLB->ISP_YUV_PATH1_FUNC_DIS = 1;	\
							GLB->ISP_YUV_PATH2_FUNC_DIS = 1;	\
							GLB->ISP_YUV_PATH3_FUNC_DIS = 1;	\
						}
#define	uwISP_GetHSz()      ( SEN->H_SIZE << 1)
#define	uwISP_GetVSz()      ( SEN->V_SIZE << 1)

/*!
\brief ISP date stamp status 
*/
typedef enum
{
	DS_OFF = 0,         //!< DS disable index
	DS_ON,              //!< DS enable index
}DS_SWITCH_TYPE;

/*!
\brief ISP date stamp path selection 
*/
typedef enum
{
	DS_PATH1 = 1,       //!< DS path1 index
	DS_PATH2,           //!< DS path2 index
}DS_PATH_TYPE;

/*!
\brief ISP date stamp size selection 
*/
enum
{
	DS_8x16 = 0,        //!< DS 8x16 index
	DS_12x16,           //!< DS 12x16 index
	DS_16x16,	        //!< DS 16x16 index
};

/*!
\brief ISP date stamp gain index
*/
typedef enum
{
	DS_GAIN8 = 0,       //!< DS gain8 index
	DS_GAIN1,           //!< DS gain1 index
	DS_GAIN2,           //!< DS gain2 index
	DS_GAIN3,           //!< DS gain3 index
	DS_GAIN4,           //!< DS gain4 index
	DS_GAIN5,           //!< DS gain5 index
	DS_GAIN6,           //!< DS gain6 index
	DS_GAIN7,           //!< DS gain7 index
}DS_GAIN_TYPE;

/*!
\brief ISP private mask status 
*/
typedef enum
{
	PM_OFF = 0,         //!< PM disable index
	PM_ON,              //!< PM enable index
}PM_SWITCH_TYPE;

/*!
\brief ISP private mask block size selection 
*/
typedef enum
{
	PM_8x6 = 1,         //!< PM 8x6 index
	PM_16x12,           //!< PM 16x12 index
}PM_BLOCK_TYPE;

/*!
\brief ISP path3 image format index 
*/
enum _IMG_FORMAT
{
	SONIX420 = 0,       //!< sonix420 index
	Y_ONLY,             //!< Y only index
	YCBCR_YUY2,         //!< YCbCr_YUY2 index
	YCBCR_SIGN,         //!< YCbCr_sign index
	RGB888,             //!< RGB888 index
	BGR888,             //!< BGR888 index
	RGB565,             //!< RGB565 index
	BGR565,             //!< BGR565 index
	RGB888_PLANAR,      //!< RGB888_plan index
	BGR888_PLANAR,      //!< BGR888_plan index
	I420_YCBCR,         //!< I420_YCbCr index
    I420_YUV,           //!< I420_YUV index
};

/*!
\brief ISP func status 
*/
enum
{
	ISP_OFF = 0,        //!< ISP disable index
	ISP_ON,             //!< ISP enable index
};

/*!
\brief Day / Night mode status 
*/
enum
{
	DAY_MODE = 0,       //!< Day mode index
	NIGHT_MODE,         //!< Night mode index
};

/*!
\brief ISP path index 
*/
enum {
	ISP_PATH1 = 1,      //!< ISP path1 index
	ISP_PATH2,          //!< ISP path2 index
	ISP_PATH3,          //!< ISP path3 index
};

/*!
\brief ISP scaler mode 
*/
typedef enum
{
    ISP_SCALE_SAME_RATIO = 0,           //!< same ration scale
    ISP_SCALE_NORMAL_MODE = 1,          //!< fine scale
}ISP_SCALE_TYPE;

typedef uint8_t (*pvISP_IqBypassCbFunc)(void);
//==============================================================================
// FUNCTION
//==============================================================================
//------------------------------------------------------------------------
/*!
\brief Set ISP report initial state
\return(no)
\par [Example]
\code 
		ISP_Init();
\endcode
*/
void ISP_Init(void);
//------------------------------------------------------------------------------//
//                               Date stamp                                     //
//------------------------------------------------------------------------------//
//------------------------------------------------------------------------
/*!
\brief Initial ISP date stamp
\param ubMode   1=LocalWithSensor,0=LocalNoSensor
\return(no)
\par [Example]
\code 
	ISP_DateStampInit(ubMode);
\endcode
*/
void ISP_DateStampInit(uint8_t ubMode);
//------------------------------------------------------------------------
/*!
\brief Set DS switch.
\param tPath 				    DS_PATH1/DS_PATH2.
\param tType                    DS_ON/DS_OFF.
\return(no)
\par [Example]
\code 
    [note]
    
                            |--------|
                        |---|   DS1  |---->   Path1 output image + (DS1)
    --------------      |   |--------|
  --|     ISP    |----->|
    --------------      |   |--------|
                        |---|   DS2  |----->  Path2 output image + (DS2)
                            |--------|
    
    ISP_SetOsdSwitch(DS_PATH1, DS_OFF);
\endcode
*/
void ISP_SetOsdSwitch(DS_PATH_TYPE tPath, DS_SWITCH_TYPE tType);
//------------------------------------------------------------------------
/*!
\brief Set DS font color.
\param tPath 				    DS_PATH1/DS_PATH2.
\param ubIndex 				    Select color(1~3).
\param ubRed		            Red value(0~255).
\param ubGreen		            Green value(0~255).
\param ubBlue		            Blue value(0~255).
\return(no)
\par [Example]
\code
	RGB -> YUV
	Y = 0.3 * R + 0.59 * G + 0.11 * B
	U = 0.493 * (B - Y)
	V = 0.877 * (R - Y)	
    
	ISP_SetOsdColor(DS_PATH1, DS_COLOR1, 255, 0, 0);        //red
\endcode
*/
void ISP_SetOsdColor(DS_PATH_TYPE tPath, uint8_t ubIndex, uint8_t ubRed, uint8_t ubGreen, uint8_t ubBlue);
//------------------------------------------------------------------------
/*!
\brief Date stamp font table.
\param tPath 			    DS_PATH1/DS_PATH2.
\param pllFontTable 		Point of font table.
\return(no)
\par [Example]
\code
		ullDS_FontTable[512] =
		{
			///////////////0////////////////////
			0x0022222222222200,
			0x0223333333333220,
			0x2233332222333322,
			0x2333322002233332,
			0x2333322002233332,
			0x2333322002233332,
			0x2333322002233332,
			0x2333322002233332,
			0x2333322002233332,
			0x2333322002233332,
			0x2333322002233332,
			0x2333322002233332,
			0x2333322002233332,
			0x2233332222333322,
			0x0223333333333220,
			0x0022222222222200,	
			...
		};
		and SEN->OSD1_SIZE = DS_16x16;		
		// 16x16 (2 bits per pixel, 16 pixels per line, 16 line per char):
		
		ISP_LoadOsdFontTable(DS_DS1, &ullDS_FontTable[0], 0, 0);		
\endcode
*/
void ISP_LoadOsdFontTable(DS_PATH_TYPE tPath, uint64_t *pllFontTable, uint8_t ubFontFlipEn, uint8_t ubFontMirrorEn);
//------------------------------------------------------------------------------
/*!
\brief Set DS line1/2 gain
\param tPath 				    DS_PATH1/DS_PATH2.
\param tGain                    DS_GAIN1~DS_GAIN8
\return(no)
\par [Example]
\code
      |--------|    |--> line1 text
   -->|   DS1  |----| 
      |--------|    |--> line2 text

      |--------|    |--> line1 text
   -->|   DS2  |----|
      |--------|    |--> line2 text
    
	ISP_SetLine1Gain(DS_PATH1, DS_GAIN2);
\endcode
*/
void ISP_SetLine1Gain(DS_PATH_TYPE tPath, DS_GAIN_TYPE tGain);
void ISP_SetLine2Gain(DS_PATH_TYPE tPath, DS_GAIN_TYPE tGain);
//------------------------------------------------------------------------
/*!
\brief Set DS line1/2 position, length and display.
\param tPath 				    DS_PATH1/DS_PATH2.
\param ubDateStampLine1 		Point of Line1/2 data.
\param ubLength 				Show line1/2 length.(max is 32)
\param uwX 				        Position of X-axis.
\param uwY 				        Position of Y-axis.
\return(no)
\par [Example]
\code 
		uint8_t ubISP_DSLine1[32] = {	
			0,	 1,  2,	 3,	 4,	 5,  6,	 7,	 8,	 9,	10,	11,	12,	13, 14,	15,
			16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
		};

		index value mapping font table value and font value show on image.
		ISP_OsdLine1Display(DS_PATH1 ,&ubISP_DSLine1[0] ,length, PosX, PosY);
\endcode
*/
void ISP_OsdLine1Display(DS_PATH_TYPE tPath, uint8_t *ubDateStampLine1, uint8_t ubLength, uint16_t uwX, uint16_t uwY);
void ISP_OsdLine2Display(DS_PATH_TYPE tPath, uint8_t *ubDateStampLine2, uint8_t ubLength, uint16_t uwX, uint16_t uwY);
//------------------------------------------------------------------------------//
//                               Private Mask                                   //
//------------------------------------------------------------------------------//
//------------------------------------------------------------------------
/*!
\brief Set PM block size.
\param uwHsize 					PM horizontal size.
\param uwVsize 					PM vertical size.
\param tCase 					PM Case. (PM_8x6/PM_16x12)
\return(no)
\par [Example]
\code
		ISP_SetPrivateMaskSize(1280, 720, PM_8x6);

		sensor resolution is HD and select case1
		PRI_MASK_H_SIZE = H_SIZE/8 = 1280/8 = 160 = 0xA0
		PRI_MASK_V_SIZE = V_SIZE/6 = 720 /6 = 120 = 0x78
		each mask block is 160x120
		sensor resolution is HD and select case2
		PRI_MASK_H_SIZE = H_SIZE/16 = 1280/16 = 80 = 0x50
		PRI_MASK_V_SIZE = V_SIZE/12 = 720 /12 = 60 = 0x3C
		each mask block is 80x60
\endcode
*/
void ISP_SetPrivateMaskSize(uint16_t uwHsize, uint16_t uwVsize, PM_BLOCK_TYPE tCase);
//------------------------------------------------------------------------
/*!
\brief Set PM block color.
\param ubYColor 					Y[0 to 15]
\param ubUColor 					U[0 to 15]
\param ubVColor 					V[0 to 15]
\return(no)
\par [Example]
\code
	ISP_SetPrivateMaskColor(0 ,0x08 ,0x08);

	 0: select 8'h00,  1: select 8'h10,  2: select 8'h20,   3: select 8'h30,
	 4: select 8'h40,  5: select 8'h50,  6: select 8'h60,   7: select 8'h70,
	 8: select 9'h80,  9: select 8'h90, 10: select 8'ha0,  11: select 8'hb0,
	12: select 8'hc0, 13: select 8'hd0, 14: select 8'he0,  15: select 8'hf0.

	R = Y + 1.139 * (V - 128)
	G = Y - 0.394 * (U - 128) - 0.58 *(V - 128)
	B = Y + 2.032 * (U - 128)
\endcode
*/
void ISP_SetPrivateMaskColor(uint8_t ubYColor, uint8_t ubUColor, uint8_t ubVColor);
//------------------------------------------------------------------------
/*!
\brief Set PM switch.
\param tType 			    		PM_ON/PM_OFF.
\return(no)
\par [Example]
\code 
		ISP_SetPrivateMaskSwitch(PM_ON);
\endcode
*/
void ISP_SetPrivateMaskSwitch(PM_SWITCH_TYPE tType);
//------------------------------------------------------------------------
/*!
\brief PM trigger.
\return(no)
\par [Example]
\code 
	***If user want to change PM register, must trigger ***	
		ISP_PrivateMaskTrigger();		
\endcode
*/
void ISP_PrivateMaskTrigger(void);
//------------------------------------------------------------------------
/*!
\brief Conversion PM data.
\param ubType 					PM Case. (PM_8x6/PM_16x12)
\param ubPrivateArea 			Point of array data.
\return(no)
\par [Example]
\code 
	ubPrivateMaskData1[24] = {	
		0xaa,
		0x55,
		0x33,
		0xcc,
		0x69,
		0x69,
	};
	and type is PM_8x6.

	ISP_ConversionPrivateMask(PM_8x6, &ubPrivateMaskData1[0]);	
	---------------------------------
	| 1 | 0 | 1 | 0 | 1 | 0 | 1 | 0 |		
	| 0 | 1 | 0 | 1 | 0 | 1 | 0 | 1 |	
	| 0 | 0 | 1 | 1 | 0 | 0 | 1 | 1 |	
	| 1 | 1 | 0 | 0 | 1 | 1 | 0 | 0 |	
	| 0 | 1 | 1 | 0 | 1 | 0 | 0 | 1 |	
	| 0 | 1 | 1 | 0 | 1 | 0 | 0 | 1 |
	|--------------------------------  1/0 => mask/normal area
\endcode
*/
void ISP_ConversionPrivateMask(uint8_t ubType, uint8_t * ubPrivateArea);
//------------------------------------------------------------------------------//
//                              ISP function switch                             //
//------------------------------------------------------------------------------//
//------------------------------------------------------------------------
/*!
\brief IQ bypass state call back function
\param pvCB     Callback function (IQ bypass state func)
\return (no)
*/
void ISP_IqBypassCbFunc(pvISP_IqBypassCbFunc pvCB);
//------------------------------------------------------------------------
/*!
\brief Dynamic range control switch.
\param ubFlag 	DRC switch.
\return(no)
\par [Example]
\code 
        uint8_t ubSwitch = ISP_ON / ISP_OFF;
		ISP_DRCSwitch(ubSwitch);
\endcode
*/
void ISP_DRCSwitch(uint8_t ubFlag);
//------------------------------------------------------------------------
/*!
\brief Vertical lens distortion compensation switch.
\param ubFlag 	VLDC switch.
\return(no)
\par [Example]
\code 
        uint8_t ubSwitch = ISP_ON / ISP_OFF;
		ISP_VldcSwitch(ubSwitch);
\endcode
*/
void ISP_VLDCSwitch(uint8_t ubFlag);
//------------------------------------------------------------------------
/*!
\brief Lens shading correction switch.
\param ubFlag 	LSC switch.
\return(no)
\par [Example]
\code 
        uint8_t ubSwitch = ISP_ON / ISP_OFF;
		ISP_LscSwitch(ubSwitch);
\endcode
*/
void ISP_LscSwitch(uint8_t ubFlag);
//------------------------------------------------------------------------
/*!
\brief Noise reduce 2D switch.
\param ubFlag 	NR2D switch.
\return(no)
\par [Example]
\code 
        uint8_t ubSwitch = ISP_ON / ISP_OFF;
		ISP_NR2DSwitch(ubSwitch);
\endcode
*/
void ISP_NR2DSwitch(uint8_t ubFlag);
//------------------------------------------------------------------------
/*!
\brief Noise reduce 3D switch.
\param ubFlag 	NR3D switch.
\return(no)
\par [Example]
\code 
        uint8_t ubSwitch = ISP_ON / ISP_OFF;
		ISP_NR3DSwitch(ubSwitch);
\endcode
*/
void ISP_NR3DSwitch(uint8_t ubFlag);
//------------------------------------------------------------------------------//
//                         Scaler function application                          //
//------------------------------------------------------------------------------//
//------------------------------------------------------------------------
/*!
\brief Scale-up setting, when scale-up happened need to set this function.
\param ubScaleUp 	1:scale-up /0:scale-normal.
\return(no)
\par [Example]
\code 
		ISP_ScalerSetting(0);
\endcode
*/
void ISP_ScalerSetting(uint8_t ubScaleUp);
//------------------------------------------------------------------------
/*!
\brief Zoom to area.
\param uwZoomHsize 	Zoom horizontal size.
\param uwZoomVsize 	Zoom vertical size .
\param uwHStart 	Horizontal start position.
\param uwVStart 	Vertical start position.
\param uwHO 	    output horizontal size.
\param uwVO  	    output vertical size.
\return(no)
\par [Example]
\code
    Note:
       1.((Ho/2 > uwZoomHsize) && (Vo/2 > uwZoomVsize))?    non-fair(scale-up max is 2) : fair
       2.((uwZoomHsize + uwHStart > sensor output) &&         
            (uwZoomVsize + uwVStart > sensor output))?      non-fair(over image) : fair

    sensor output                  ISP scaler = HO x VO
    -----------                       -----------	
    |  -----  |                       |         |
    |  | A |  | ------------------->  |    A    |
    |  -----  |                       |         |
    -----------                       -----------
        zoom to area-A, where, 
            ISP_Zoom2Area(A_H, A_V, A_Hs, A_Vs, HO, VO);
\endcode
*/
void ISP_Zoom2Area(uint16_t uwZoomHsize, uint16_t uwZoomVsize, uint16_t uwHStart, uint16_t uwVStart, uint16_t uwHO, uint16_t uwVO);
//------------------------------------------------------------------------
/*!
\brief Pan-tilt-zoom achieve.
\param ubPath 	Select path.
\param uwFrameHsize 	Horizontal size of frame.
\param uwFrameVsize 	Vertical size of frame.
\param uwHStart 	Horizontal start point.
\param uwVStart 	Vertical start point.
\return(no)
\par [Example]
\code
		sensor output = 1280x720					ISP scale = 320x240
		-----------                       -----------
		|  A |	  |                       |		    |
		|-----	  |	------------------->  |	   A	|
		|		  |                       |		    |
		-----------                       -----------
	ex:
		Block A are composed of any H/Vsize and H/VStart 
		If block A is 320x240,
			H/V_start limit are (0~(1280-320)) and (0~(720-240)).
		(RATIO_H/V = 128)
			
		If block A is 640x480,
			H/V_start limit are (0~640) and (0~240).
		(RATIO_H/V = 256)		
\endcode
*/
void ISP_PTZapplication(uint8_t ubPath, uint16_t uwFrameHsize, uint16_t uwFrameVsize, uint16_t uwHStart, uint16_t uwVStart);
//------------------------------------------------------------------------
/*!
\brief Cropping from ISP window.
\param ubPath 	Select path1/2/3.
\param uwHsize 		Horizontal size of cropping.
\param uwVsize 		Vertical size of cropping.
\param uwHStart 	Horizontal start point.
\param uwVStart 	Vertical start point.
\return(no)
\par [Example]
\code
		sensor output = 1280x720					ISP scaler size is 640x480
		-----------                       -----------
		|  A |	  |                       |		    |
		|-----	  |	------------------->  |	   A    |
		|		  |                       |		    |
		-----------                       -----------
		ISP_WindowCropping(SENSOR_PATH1, 640, 480, 0, 0);		
		
	ex:
		Block A are composed of any H/Vsize and H/VStart 
		If block A is 320x240,
			H/V_start limit are (0~960) and (0~480).
		(RATIO_H/V = 128)
			
		If block A is 640x480,
			H/V_start limit are (0~640) and (0~240).
		(RATIO_H/V = 128)	
\endcode
*/
void ISP_WindowCropping(uint8_t ubPath, uint16_t uwHsize, uint16_t uwVsize, uint16_t uwHStart, uint16_t uwVStart);
//------------------------------------------------------------------------
/*!
\brief Set ISP output scaler size.
\param ubPath 	Select path1/2/3.
\param uwScalerHsize 	Horizontal size of ISP output.
\param uwScalerVsize 	Vertical size of ISP output.
\param uwHsize          Horizontal size of sensor output.
\param uwVsize          Vertical size of sensor output.
\return(no)
\par [Example]
\code 
		sensor output=HsizexVsize  		ISP scale = ScalerHxScalerV
      uwHsize                         uwScalerHsize
    -----------                       -----------	
    |         |                       |         | 
    |   SEN   |	uwVsize               |   ISP   | uwScalerVsize
    |         |                       |         |
    -----------                       -----------
		***only path1 support scale-up and maximum is 2.
		
		ISP_SetScaler(SENSOR_PATH1, ulUpImgW, ulUpImgH, ulImgW, ulImgH);
\endcode
*/
void ISP_SetScaler(uint8_t ubPath, uint16_t uwScalerHsize, uint16_t uwScalerVsize, uint16_t uwHsize, uint16_t uwVsize);
//------------------------------------------------------------------------
/*!
\brief Set scale mode.
\param tMode 		ISP_SCALE_NORMAL_MODE/ISP_SCALE_SAME_RATIO.
\return(no)
*/
void ubISP_SetScaleMode(ISP_SCALE_TYPE tMode);
//------------------------------------------------------------------------------//
//                         Image Quality control                                //
//------------------------------------------------------------------------------//
//------------------------------------------------------------------------
/*!
\brief Set Saturation.
\param ubSatu 		Saturation value.
\return(no)
\par [Example]
\code 
        ubSatu = 0~255;
		ISP_SetIQSaturation(ubCont);
\endcode
*/
void ISP_SetIQSaturation(uint8_t ubSatu);
//------------------------------------------------------------------------
/*!
\brief UVC Set Saturation.
\param uwSatu 		Saturation value.
\return(no)
\par [Example]
\code 
        uwSatu = 0   ~ 128;
		ISP_UvcSetIQSaturation(uwSatu);
\endcode
*/
void ISP_UvcSetIQSaturation(uint16_t uwSatu);
//------------------------------------------------------------------------
/*!
\brief Set Chroma.
\param ubChroma 		Chroma value.
\return(no)
\par [Example]
\code 
        ubChroma = 0~255;
		ISP_SetIQChroma(ubChroma);
\endcode
*/
void ISP_SetIQChroma(uint8_t ubChroma);
//------------------------------------------------------------------------
/*!
\brief UVC Set Chroma.
\param swChroma 		Chroma value.
\return(no)
\par [Example]
\code 
        swChroma = -40 ~ 40;
		ISP_UvcSetIQChroma(swChroma);
\endcode
*/
void ISP_UvcSetIQChroma(int16_t swChroma);
//------------------------------------------------------------------------
/*!
\brief Set Contrast.
\param ubCont 		Contrast value.
\return(no)
\par [Example]
\code 
        ubCont = 0~255;
		ISP_SetIQContrast(ubCont);
\endcode
*/
void ISP_SetIQContrast(uint8_t ubCont);
//------------------------------------------------------------------------
/*!
\brief UVCSet Contrast.
\param uwCont 		Contrast value.
\return(no)
\par [Example]
\code 
        uwCont = 0 ~ 64;
		ISP_UvcSetIQContrast(uwCont);
\endcode
*/
void ISP_UvcSetIQContrast(uint16_t uwCont) ;
//------------------------------------------------------------------------
/*!
\brief Set Brightness.
\param ubBri 		Brightness value.
\return(no)
\par [Example]
\code 
        ubBri = 0~255;
		ISP_SetIQBrightness(ubBri);
\endcode
*/
void ISP_SetIQBrightness(uint8_t ubBri);
//------------------------------------------------------------------------
/*!
\brief UVC Set Brightness.
\param swBrit 		Brightness value.
\return(no)
\par [Example]
\code 
        swBrit = -64 ~ 64;
		ISP_UvcSetIQBrightness(swBrit);
\endcode
*/
void ISP_UvcSetIQBrightness(int16_t swBrit);
//------------------------------------------------------------------------
/*!
\brief Set Gamma.
\param uwGamma 		Gamma value.
\return(no)
\par [Example]
\code 
        uwGamma = 72  ~ 500;
		ISP_UvcSetIQGamma(uwGamma);
\endcode
*/
void ISP_UvcSetIQGamma(uint16_t uwGamma);
//------------------------------------------------------------------------
/*!
\brief Set day/night mode.(change color, dynamic IQ)
\param ubMode 		DAY_MODE/NIGHT_MODE.
\return(no)
*/
void ISP_DayNightModeSwitch(uint8_t ubMode);
//------------------------------------------------------------------------
/*!
\brief protect LSC switch bug.
\return(no)
*/
void ISP_ProtLsc(void);
//------------------------------------------------------------------------------
/*!
\brief 	Get IR cut mode state.	
*/
uint8_t ubSEN_GetDayNightMode(void);
//------------------------------------------------------------------------------//
//                         Other ISP control                                    //
//------------------------------------------------------------------------------//
//------------------------------------------------------------------------
/*!
\brief Set AE power frequency.
\param ubPwrFreq 		set 50/60Hz.
\return(no)
\par [Example]
\code 
        uint8_t ubPwrFreq = SENSOR_PWR_FREQ_60HZ;

		ISP_SetAePwrFreq(ubType);
\endcode
*/
void ISP_SetAePwrFreq(uint8_t ubPwrFreq);
//------------------------------------------------------------------------
/*!
\brief Set path3 image output format type.
\param ubType 	Formar type.
\return(no)
\par [Example]
\code 
        ISP stream3 image output format
		0000: SONIX420 
		0001: Y_ONLY 
		0010: YCBCR_YUY2 
        0011: YCBCR_SIGN
		0100: RGB888
		0101: BGR888
		0110: RGB565 
		0111: BGR565 
		1000: RGB888_PLANAR 
		1001: BGR888_PLANAR 
		1010: I420_YCBCR
        1011: I420_YUV
        Others: Reserved

        uint8_t ubType = SONIX420;

		ISP_SetPath3ImgFormat(ubType);
\endcode
*/
void ISP_SetPath3ImgFormat(uint8_t ubType);
//------------------------------------------------------------------------
/*!
\brief Set 3DNR fame buffer compression.
\return(no)
\par [Example]
\code 
		ISP_Set3DNR_FBC();
\endcode
*/
void ISP_Set3DNR_FBC(void);
//------------------------------------------------------------------------
/*!
\brief Get LSC block gain table.
\return(no)
\par [Example]
\code 
		ISP_GetBlockGainTable();
\endcode
*/
void ISP_GetBlockGainTable(void);
//------------------------------------------------------------------------
/*!
\brief Set LSC block gain table.
\return(no)
\par [Example]
\code 
        uint8_t ubType;
        
		ISP_SetBlockGainTable(ubType);
        where ubType =  0, Normal
                        1, Flip 
                        2, Mirror
                        3, Mirror+Flip.
		need to matching SENSOR mirror/flip and RAW_REORDER type.
\endcode
*/
void ISP_SetBlockGainTable(uint8_t ubType);
//------------------------------------------------------------------------
/*!
\brief Set ISP mirror and flip.
\param ubMirrorEn 	mirror switch.
\param ubFlipEn 	flip switch.
\return(no)
\par [Example]
\code 
        Normal ==>  -------------   Flip ==>    -------------
                    | 0 | 1 | 2 |               | 6 | 7 | 8 |
                    -------------               -------------
                    | 3 | 4 | 5 |               | 3 | 4 | 5 |
                    -------------               -------------
                    | 6 | 7 | 8 |               | 0 | 1 | 2 |
                    -------------               -------------
        Mirror ==>  -------------   M+F ==>     -------------
                    | 2 | 1 | 0 |               | 8 | 7 | 6 |
                    -------------               -------------
                    | 5 | 4 | 3 |               | 5 | 4 | 3 |
                    -------------               -------------
                    | 8 | 7 | 6 |               | 2 | 1 | 0 |
                    -------------               -------------
                    
    uint8_t ubMirrorEn, ubFlipEn;
    
    ubMirrorEn = 1;     // mirror enable
    ubFlipEn = 0;       // flip disable
    
    ISP_SetMirrorFlip(ubMirrorEn, ubFlipEn);
\endcode
*/
void ISP_SetMirrorFlip(uint8_t ubMirrorEn, uint8_t ubFlipEn);
//------------------------------------------------------------------------
/*!
\brief 	Get ISP function version	
\return	image signal process version.
\par [Example]
\code		 
	 uwISP_GetVersion();
\endcode
*/
uint16_t uwISP_GetVersion(void);
#endif
