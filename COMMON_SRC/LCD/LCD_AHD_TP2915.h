/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		LCD_AHD_TP2915.h
	\brief		Techpoint TP2915 HD-TVI Video Encoder Driver (AHD)
	\author		Nick Huang/futao_jiang
	\version	1.6
	\date		2022/03/04
	\copyright	Copyright(C) 2022 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _LCD_AHD_TP2195_H_
#define _LCD_AHD_TP2195_H_
#include "_510PF.h"
//------------------------------------------------------------------------------
//! The macro define for AHD TP2915
enum
{
    AHD_TP2915_AHD1080P30 = 0,
    AHD_TP2915_AHD1080P25,
    AHD_TP2915_AHD720P30,
    AHD_TP2915_AHD720P25,
    AHD_TP2915_TVI1080P30,
    AHD_TP2915_TVI1080P25,
	AHD_TP2915_TVI720P60,
    AHD_TP2915_TVI720P30,
    AHD_TP2915_TVI720P25,
    AHD_TP2915_CVBS_NTSC,
    AHD_TP2915_CVBS_PAL,
	
	AHD_TP2915_FORMAT_MAX
};
//------------------------------------------------------------------------------
/*!
	\brief 		The AHD bridge of AHD2915 Initial Function
	\par [Example]
	\code    
		 AHD_TP2915_Init();
	\ndcode
*/
void AHD_TP2915_Init(void);
//------------------------------------------------------------------------------
/*!
	\brief 		The AHD bridge of AHD2915 Start Display Function		
	\par [Example]
	\code    
		 LCD_MIPI_TC358778_Start();
	\ndcode
*/
void AHD_TP2915_Start(void);
//------------------------------------------------------------------------------
/*!
	\brief 		The AHD bridge of AHD2915 Changes Output Type Function		
	\param[in]	ubOutputType is the setting output type (AHD_TP2915_AHD1080P30 AHD_TP2915_AHD1080P25 ...)
	\par [Example]
	\code    
		 AHD_TP2915_SetOutputType(AHD_TP2915_AHD1080P30);
	\ndcode
*/
void AHD_TP2915_SetOutputType(uint8_t ubOutputType);
//------------------------------------------------------------------------------
/*!
	\brief 		The AHD Bridge TP2915 Enter to Suspend from Displaying Function		
	\par [Example]
	\code    
		 AHD_TP2915_DacSuspend();
	\ndcode
*/
void AHD_TP2915_DacSuspend(void);
//------------------------------------------------------------------------------
/*!
	\brief 		The AHD Bridge TP2915 Enter to Wake Up from Suspend Function		
	\par [Example]
	\code    
		 AHD_TP2915_DacResume();
	\ndcode
*/
void AHD_TP2915_DacResume(void);


#endif
