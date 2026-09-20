/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		LCD_HDMI_IT66121_API.h
	\brief		LCD HDMI API Header
	\author		Pierce
	\version	0.6
	\date		2020/06/01
	\copyright	Copyright(C) 2020 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _LCD_HDMI_IT66121_API_H_
#define _LCD_HDMI_IT66121_API_H_
#include <stdint.h>
//------------------------------------------------------------------------------
//! HDMI Library Support Function
//#define HDMI_TESTA
#define	HDMI_EDID
//#define HDMI_HDCP
//#define HDMI_HDCP_SHA
//#define HDMI_AUDIO
//------------------------------------------------------------------------------
/*!
	\brief 	HDMI Tx Resolution
*/
typedef enum 
{
    HDMI_1920x1080P60,
//  HDMI_1920x1080P50,
//	HDMI_1920x1080P30,
//	HDMI_1920x1080P25,
//	HDMI_1920x1080P24,
	HDMI_1280x720P60,
//	HDMI_1280x720P50,
//	HDMI_720x576P50_VIC17,
//	HDMI_720x576P50_VIC18,
//	HDMI_720x480P60_VIC2,
//	HDMI_720x480P60_VIC3,
	HDMI_640x480P60,
//	HDMI_RES_MAX,
}HDMI_RES_TYP;
//------------------------------------------------------------------------------
/*!
	\brief 	HDMI Tx State
*/
typedef enum 
{
    HDMI_PLUG_IN,
    HDMI_OUTPUT,
	HDMI_PLUG_OUT
}HDMI_STATE_TYP;
//------------------------------------------------------------------------------
/*!
	\brief 	HDMI Tx Initial
	\par [Example]
	\code    
		 HDMI_Init();
	\endcode
*/
void HDMI_Init (void);
//------------------------------------------------------------------------------
/*!
	\brief 	LCD Output Timing Initial for HDMI Tx
	\par [Example]
	\code    
		 HDMI_LcdTimingInit();
	\endcode
*/
void HDMI_LcdTimingInit (void);
//------------------------------------------------------------------------------
/*!
	\brief 	Detect HDMI Tx State
	\return	HDMI_PLUG_IN  Detect HDMI Rx plug in
	\return	HDMI_OUTPUT   HDMI Tx output now
	\return	HDMI_PLUG_OUT Detect HDMI Rx plug output
	\par [Example]
	\code
		 HDMI_STATE_TYP tState;
		 tState = tHDMI_HdmiDetect();
	\endcode
*/
HDMI_STATE_TYP tHDMI_HdmiDetect (void);
//------------------------------------------------------------------------------
/*!
	\brief 	Get HDMI IT66121 Function Version	
	\return	Unsigned short value, high byte is the major version and low byte is the minor version
	\par [Example]
	\code		 
		 uint16_t uwVer;
		 
		 uwVer = uwHDMI_IT66121_GetVersion();
		 printf("HDMI IT66121 Version = %d.%d\n", uwVer >> 8, uwVer & 0xFF);
	\endcode
*/
uint16_t uwHDMI_IT66121_GetVersion (void);
//------------------------------------------------------------------------------
/*!
	\brief 		HDMI Force Resolution Function
	\param[in]	tRes 	  User Force Resolution 1080P / 720P / VGA
	\par [Example]
	\code
		HDMI_ForceSetRes(HDMI_1920x1080P60);
	\endcode
*/
void HDMI_ForceSetRes (HDMI_RES_TYP tRes);
#endif
