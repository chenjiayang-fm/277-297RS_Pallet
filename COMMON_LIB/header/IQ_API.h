/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		IQ_API.h
	\brief		Image quality API function header
	\author		BoCun
	\version	1.2
	\date		2020/05/19
	\copyright	Copyright(C) 2020 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _IQ_API_H_
#define _IQ_API_H_

//==============================================================================
// FILE INCLUSION
//==============================================================================
#include "_510PF.h"

//==============================================================================
// DEFINITION
//==============================================================================
#define IQ_EVENT_SUSPEND			(0)
#define IQ_EVENT_RESUME				(1)
#define IQ_EVENT_VSYNC				(2)
#define IQ_EVENT_START				(3)
#define IQ_EVENT_STOP				(4)

#define IQ_STATE_SUSPEND			(0)
#define IQ_STATE_STANDBY			(1)
#define IQ_STATE_DO					(2)
#define IQ_STATE_WAIT				(3)
// ------------ TUNING --------------
// Library Parameter
#define IQ_WAIT_FRAMES				(0)

//==============================================================================
// MACRO FUNCTION 
//==============================================================================
#define IQ_SetSkip(ubSp)								{ xtIQDnInst.ubSkip = ubSp; }
#define ubIQ_GetSkip()									( xtIQDnInst.ubSkip)

#define IQ_DynamicSuspend()								{ IQ_DynamicStatemachine3G5(IQ_EVENT_SUSPEND); }
#define IQ_DynamicResume()								{ IQ_DynamicStatemachine3G5(IQ_EVENT_RESUME); }
#define IQ_DynamicVSync()								{ IQ_DynamicStatemachine3G5(IQ_EVENT_VSYNC); }
#define IQ_DynamicStart()								{ IQ_DynamicStatemachine3G5(IQ_EVENT_START); }
#define IQ_DynamicStop()								{ IQ_DynamicStatemachine3G5(IQ_EVENT_STOP); }

//==============================================================================
// STRUCT
//==============================================================================
typedef struct tagIQJudgeObj {
	// Dynamic IQ Judge Value
	uint16_t uwAWB_BGainX128;
	uint16_t uwAWB_GGainX128;
	uint16_t uwAWB_RGainX128;
	uint8_t ubAE_Expidx;
	uint16_t uwAE_CurrGain;
	uint32_t ulAE_AbsY;
	uint32_t ulAF_W0Sum;
	uint32_t ulAF_W1Sum;
}IQJudgeObj;

typedef struct tagIQDNObj {
	// Dynamic IQ System and State Control
	uint8_t ubState;
	uint8_t ubWaitCnt;
	uint8_t ubSkip;
	uint32_t uwCurrPrevHSz;
	// Dynamic IQ Judge Value
	IQJudgeObj xtIQJudgeInst;	
}IQDNObj;

//==============================================================================
// IQ extern item
//==============================================================================
extern IQDNObj xtIQDnInst;

//------------------------------------------------------------------------
/*!
\brief Set IQ default state.
\return(no)
\par [Example]
\code 
		IQ_Init();	
\endcode	
*/
void IQ_Init(void);
//------------------------------------------------------------------------
/*!
\brief Linear Interpolation	Function.
\param uwJudgeVal 	-
\param ubRowMaxNum 	-
\param ubLinearNum 	-
\param plLinear 		-
\param plReturnVal 	-
\return(no)
*/
void IQ_DynamicLinearInterpolation(uint32_t ulJudgeVal, uint8_t ubRowMaxNum, uint8_t ubLinearNum, int32_t *plLinear, int32_t *plReturnVal);
//------------------------------------------------------------------------
/*!
\brief Set brightness/contrast.
\param uwGamma 	gamma value.
\param swBrit 	bright value.
\param uwCntrs 	constrast value.
\param uwGain 	gain.
\return(no)
*/
void IQ_SetGammaBrightnessContrast(uint16_t uwGamma, int16_t swBrit, uint16_t uwCntrs, uint16_t uwGain);
//------------------------------------------------------------------------
/*!
\brief Set hue/saturation.
\param swHue 		Hue value.
\param uwSatu 	Saturation value.
\return(no)
*/
void IQ_SetClrMtx(int16_t swHue, uint16_t uwSatu);
//------------------------------------------------------------------------
/*!
\brief Set edge enhance.
\param ubHGain 	horizontal Gain.
\param ubVGain 	vertical Gain.
\param ubThd 		threshold value.
\return(no)
*/
void IQ_SetEdgeEnhnce(uint8_t ubHGain, uint8_t ubVGain, uint8_t ubThd);
//------------------------------------------------------------------------
/*!
\brief Set resolution of IQ struct.
\return(no)
*/
void IQ_SetISPRes(void);
//------------------------------------------------------------------------
/*!
\brief Dynamic IQ SDK.
\return(no)
*/
void IQ_DYNAMIC_SDK(void);
//------------------------------------------------------------------------
/*!
\brief Dynamic IQ inital SDK.
\return(no)
*/
void IQ_DynamicInit_SDK(void);
//------------------------------------------------------------------------
/*!
\brief Set resolution.
\param uwWidth 			Resolution width.
\param uwHeight 		Resolution height.
\param ubFps 				Frame rate.
\param ubPath 			Path1/2.
\return(no)
*/
void IQ_SetResolution_SDK(uint16_t uwWidth, uint16_t uwHeight, uint8_t ubFps, uint8_t ubPath) ;
//------------------------------------------------------------------------
/*!
\brief Set IQ command value.
\param ubId 	command item.
\param swVal 	value.
\return(no)
*/
void IQ_SetCmdVal(uint8_t ubId, int16_t swVal);
//------------------------------------------------------------------------
/*!
\brief Setting bin file value.
\param ulCmd 		command item.
\param ulAddr 	address.
\param ulMask 	data mask .
\param ulVal 		value.
\return(no)
*/
void IQ_SetBinVal(uint32_t ulCmd, uint32_t ulAddr, uint32_t ulMask, uint32_t ulVal);
//------------------------------------------------------------------------
/*!
\brief Switch tunning tool.
\param ubFuncEn 	on/off.
\return(no)
*/
void IQ_SetTunningSwitch(uint8_t ubFuncEn);
//------------------------------------------------------------------------
/*!
\brief Set Dynamic IQ default state.
\return(no)
*/
void IQ_DynamicInit(void);
//------------------------------------------------------------------------
/*!
\brief Set Dynamic IQ value(AWB,AE).
\return(no)
*/
void IQ_DynamicJudgeVal(void);
//------------------------------------------------------------------------
/*!
\brief Dynamic 	IQ operating process.
\param ubEvent 	Dynamic IQ current state.
\return(no)
*/
void IQ_DynamicStatemachine3G5(uint8_t ubEvent);
//------------------------------------------------------------------------
/*!
\brief Set noise reduce(NR) value.
\param ulId 	NR index.
\param ulVal 	NR value.
\retval true	0->set nr value success.
\retval false	1->set nr value fail.
*/
uint8_t ubIQ_SetNrTurningVal(uint32_t ulId, uint32_t ulVal);
//------------------------------------------------------------------------
/*!
\brief Get noise reduce(NR) value.
\param ulId 	NR index.
\param ulMask 	NR value.
\retval value	Get value.
*/
uint32_t ulIQ_GetNrTurningVal(uint32_t ulId, uint32_t ulMask);
//------------------------------------------------------------------------
/*!
\brief 	Get IQ Version	
\return	Version
*/
uint16_t uwIQ_GetVersion(void);
//------------------------------------------------------------------------
/*!
\brief Parser Dynamic IQ (black level).
\param ubTemp 	Dynamic IQ value.
\return(no)
*/
void IQ_SetAeCtrlPct(uint8_t *ubTemp);
#endif
