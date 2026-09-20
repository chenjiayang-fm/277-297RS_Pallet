/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		IQ_PARSER_API.h
	\brief		Sensor parser funcations API header
	\author		BoCun
	\version	3.2
	\date		2020/12/22
	\copyright	Copyright(C) 2020 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _IQ_PARSER_API_H
#define _IQ_PARSER_API_H

#include "_510PF.h"
#include "I2C.h"

#define IQ_XU_EVENTQUEUE_SZ			5
//==============================================================================
// FUNCTION
//==============================================================================
typedef struct {
    uint32_t ulIQver[2];                //!< IQ bin file version
    uint32_t ulIQSize;                  //!< IQ bin file size
    uint32_t ulIqFrameRate;             //!< IQ bin frame rate
    uint32_t ulReorderPattern;          //!< reorder state
    uint32_t ulIspFuncFlag;             //!< Control AE,AWB and Dyn IQ flag
    uint32_t ulSuppressGain;            //!< LSC suppress
} IQ_PARSER_INF;

typedef uint8_t (*pvSenSlaveAddrCbFunc)(void);
typedef uint8_t (*pvIspBypassStateCbFunc)(void);
typedef uint32_t (*pvIqBinAddrCbFunc)(void);
//------------------------------------------------------------------------
void IQ_SensorSlaveAddrCbFunc(pvSenSlaveAddrCbFunc pvCB);
void IQ_IspBypassStateCbFunc(pvIspBypassStateCbFunc pvCB);
void IQ_IqBinAddrCbFunc(pvIqBinAddrCbFunc pvCB);
//------------------------------------------------------------------------
/*!
\brief  Parser IQ.bin file.
\return (no)	
*/
void IQ_ReadIQTable(void);
//------------------------------------------------------------------------
/*!
\brief  Load IQ.bin to dram and start parser IQ.bin.
\return (no)	
*/
void IQ_LoadDataFromStorage(void);
//------------------------------------------------------------------------------
/*!
\brief Setup I2C for USB Device XU.
\param pI2C 		 	I2C type
\return(no)
*/
void IQ_SetI2cType(I2C1_Type *pI2C, I2C_SCL_SPEED_TYP uwI2cClock);
//------------------------------------------------------------------------
/*!
\brief Get dynamic frame rate.
\return Get frame rate value.
\par [Example]
\code 
		ubIQ_GetDynFrameRate();
\endcode
*/
uint8_t ubIQ_GetDynFrameRate(void);
//------------------------------------------------------------------------
/*!
\brief Set dynamic frame rate.
\param ubFrameRate 			frame rate.
\return(no)
\par [Example]
\code 
		IQ_SetDynFrameRate(ulAsicData);
\endcode
*/
void IQ_SetDynFrameRate(uint8_t ubFrameRate);
//------------------------------------------------------------------------
/*!
\brief Get suppress gain.
\return Suppress gain value.
*/
uint32_t ulIQ_GetSuppressGain(void);
//------------------------------------------------------------------------
/*!
\brief Set suppress gain.
\param ulGainX128 			suppress gain(this value 128 equal 1x gain).
\return(no)
\par [Example]
\code 
		IQ_SetSuppressGain(ulGainX128);
\endcode
*/
void IQ_SetSuppressGain(uint32_t ulGainX128);
//------------------------------------------------------------------------
/*!
\brief Set auto exposure PID value.
\param ubLowFrame 			low frame rate set to 1.
\return(no)
\par [Example]
\code 
		IQ_SetAEPID(0);
\endcode
*/
void IQ_SetAEPID(uint8_t ubLowFrame);
//------------------------------------------------------------------------
/*!
\brief Set Dynamic IQ LSC.
\param data 	Point of LSC array.
\param ubRow 	Row of array.
\param ubCol 	Column of array.
\return(no)
*/
void IQ_SetLscLinearInf(void *data, uint8_t ubCol, uint8_t ubRow);
//------------------------------------------------------------------------
/*!
\brief Set absy mode Dynamic IQ .
\param data 	Point of absy mode array.
\param ubRow 	Row of array.
\param ubCol 	Column of array.
\return(no)
*/
void IQ_SetAbsyLinearInf(void *data, uint8_t ubCol, uint8_t ubRow);
//------------------------------------------------------------------------
/*!
\brief Set NR FHD table at day mode.
\param data 	Point of Day NR array.
\param ubRow 	Row of array.
\param ubCol 	Column of array.
\return(no)
\par [Example]
\code 
		IQ_SetDayNrRes1Inf((void *)slIQ_DayNrRes1Linear, col, row);
\endcode
*/
void IQ_SetDayNrRes1Inf(void *data, uint8_t ubCol, uint8_t ubRow);
//------------------------------------------------------------------------
/*!
\brief Set NR HD table at day mode.
\param data 	Point of Day NR array.
\param ubRow 	Row of array.
\param ubCol 	Column of array.
\return(no)
\par [Example]
\code 
		IQ_SetDayNrRes2Inf((void *)slIQ_DayNrRes2Linear, col, row);
\endcode
*/
void IQ_SetDayNrRes2Inf(void *data, uint8_t ubCol, uint8_t ubRow);
//------------------------------------------------------------------------
/*!
\brief Set NR VGA table at day mode.
\param data 	Point of Day NR array.
\param ubRow 	Row of array.
\param ubCol 	Column of array.
\return(no)
\par [Example]
\code 
		IQ_SetDayNrRes3Inf((void *)slIQ_DayNrRes3Linear, col, row);
\endcode
*/
void IQ_SetDayNrRes3Inf(void *data, uint8_t ubCol, uint8_t ubRow);
//------------------------------------------------------------------------
/*!
\brief Set NR FHD table at night mode.
\param data 	Point of LSC array.
\param ubRow 	Row of array.
\param ubCol 	Column of array.
\return(no)
\par [Example]
\code 
		IQ_SetNightNrRes1Inf((void *)slIQ_NightNrRes1Linear, col, row);
\endcode
*/
void IQ_SetNightNrRes1Inf(void *data, uint8_t ubCol, uint8_t ubRow);
//------------------------------------------------------------------------
/*!
\brief Set NR HD table at night mode.
\param data 	Point of LSC array.
\param ubRow 	Row of array.
\param ubCol 	Column of array.
\return(no)
\par [Example]
\code 
		IQ_SetNightNrRes2Inf((void *)slIQ_NightNrRes2Linear, col, row);
\endcode
*/
void IQ_SetNightNrRes2Inf(void *data, uint8_t ubCol, uint8_t ubRow);
//------------------------------------------------------------------------
/*!
\brief Set NR VGA table at night mode.
\param data 	Point of LSC array.
\param ubRow 	Row of array.
\param ubCol 	Column of array.
\return(no)
\par [Example]
\code 
		IQ_SetNightNrRes3Inf((void *)slIQ_NightNrVgaLinear, col, row);
\endcode
*/
void IQ_SetNightNrRes3Inf(void *data, uint8_t ubCol, uint8_t ubRow);
//------------------------------------------------------------------------
/*!
\brief Set Dynamic IQ CCM.
\param data 	Point of LSC array.
\param ubRow 	Row of array.
\param ubCol 	Column of array.
\return(no)
\par [Example]
\code 
		IQ_SetCcmLinearInf((void *)slIQ_CcmLinear, col, row);
\endcode
*/
void IQ_SetCcmLinearInf(void *data, uint8_t ubCol, uint8_t ubRow);
//------------------------------------------------------------------------
/*!
\brief Set Dynamic IQ CCM Low Light Color.
\param data 	Point of LSC array.
\param ubRow 	Row of array.
\param ubCol 	Column of array.
\return(no)
\par [Example]
\code 
		IQ_SetCcmLowLightLinearInf((void *)slIQ_CcmLowLightLinear, col, row);
\endcode
*/
void IQ_SetCcmLowLightLinearInf(void *data, uint8_t ubCol, uint8_t ubRow);
//------------------------------------------------------------------------
/*!
\brief Set path1 resolution1 FIR.
\param data 	Point of path1 array.
\param ubRow 	Row of array.
\param ubCol 	Column of array.
\return(no)
\par [Example]
\code 
		IQ_SetPath1Res1Inf((void *)slIQ_Path1ResFhd, col, row);
\endcode
*/
void IQ_SetPath1Res1Inf(void *data, uint8_t ubCol, uint8_t ubRow);
//------------------------------------------------------------------------
/*!
\brief Set path1 resolution2 FIR.
\param data 	Point of path1 array.
\param ubRow 	Row of array.
\param ubCol 	Column of array.
\return(no)
\par [Example]
\code 
		IQ_SetPath1Res2Inf((void *)slIQ_Path1ResHd, col, row);
\endcode
*/
void IQ_SetPath1Res2Inf(void *data, uint8_t ubCol, uint8_t ubRow);
//------------------------------------------------------------------------
/*!
\brief Set path1 resolution3 FIR.
\param data 	Point of path1 array.
\param ubRow 	Row of array.
\param ubCol 	Column of array.
\return(no)
\par [Example]
\code 
		IQ_SetPath1Res3Inf((void *)slIQ_Path1ResVga, col, row);
\endcode
*/
void IQ_SetPath1Res3Inf(void *data, uint8_t ubCol, uint8_t ubRow);
//------------------------------------------------------------------------
/*!
\brief Set path2 resolution1 FIR.
\param data 	Point of path2 array.
\param ubRow 	Row of array.
\param ubCol 	Column of array.
\return(no)
\par [Example]
\code 
		IQ_SetPath2Res1Inf((void *)slIQ_Path2ResFhd, col, row);
\endcode
*/
void IQ_SetPath2Res1Inf(void *data, uint8_t ubCol, uint8_t ubRow);
//------------------------------------------------------------------------
/*!
\brief Set path2 resolution2 FIR.
\param data 	Point of path2 array.
\param ubRow 	Row of array.
\param ubCol 	Column of array.
\return(no)
\par [Example]
\code 
		IQ_SetPath2Res2Inf((void *)slIQ_Path2ResHd, col, row);
\endcode
*/
void IQ_SetPath2Res2Inf(void *data, uint8_t ubCol, uint8_t ubRow);
//------------------------------------------------------------------------
/*!
\brief Set path2 resolution3 FIR.
\param data 	Point of path2 array.
\param ubRow 	Row of array.
\param ubCol 	Column of array.
\return(no)
\par [Example]
\code 
		IQ_SetPath2Res3Inf((void *)slIQ_Path2ResVga, col, row);
\endcode
*/
void IQ_SetPath2Res3Inf(void *data, uint8_t ubCol, uint8_t ubRow);
//------------------------------------------------------------------------
/*!
\brief Set path3 resolution1 FIR.
\param data 	Point of path3 array.
\param ubRow 	Row of array.
\param ubCol 	Column of array.
\return(no)
\par [Example]
\code 
		IQ_SetPath3Res1Inf((void *)slIQ_Path2ResFhd, col, row);
\endcode
*/
void IQ_SetPath3Res1Inf(void *data, uint8_t ubCol, uint8_t ubRow);
//------------------------------------------------------------------------
/*!
\brief Set path3 resolution2 FIR.
\param data 	Point of path3 array.
\param ubRow 	Row of array.
\param ubCol 	Column of array.
\return(no)
\par [Example]
\code 
		IQ_SetPath3Res2Inf((void *)slIQ_Path2ResHd, col, row);
\endcode
*/
void IQ_SetPath3Res2Inf(void *data, uint8_t ubCol, uint8_t ubRow);
//------------------------------------------------------------------------
/*!
\brief Set path3 resolution3 FIR.
\param data 	Point of path3 array.
\param ubRow 	Row of array.
\param ubCol 	Column of array.
\return(no)
\par [Example]
\code 
		IQ_SetPath3Res3Inf((void *)slIQ_Path2ResVga, col, row);
\endcode
*/
void IQ_SetPath3Res3Inf(void *data, uint8_t ubCol, uint8_t ubRow);
//------------------------------------------------------------------------
/*!
\brief Get sensor rate.
\return Get sensor rate value.
\par [Example]
\code 
		ulIQ_GetMclk();
\endcode
*/
uint32_t ulIQ_GetMclk(void);
//------------------------------------------------------------------------
/*!
\brief Set sensor rate.
\param ulVal 		sensor rate division.
\return(no)
\par [Example]
\code 
		IQ_SetMclk(ulAsicData);
\endcode
*/
void IQ_SetMclk(uint32_t ulVal);
//------------------------------------------------------------------------
/*!
\brief Get sensor horizontal start point.
\return Get sensor horizontal start point value.
\par [Example]
\code 
		ulIQ_GetHStart();
\endcode
*/
uint32_t ulIQ_GetHStart(void);
//------------------------------------------------------------------------
/*!
\brief Set sensor horizontal start point.
\param ulVal 		Set sensor horizontal start point value.
\return(no)
\par [Example]
\code 
		IQ_SetHStart(ulAsicData);
\endcode
*/
void IQ_SetHStart(uint32_t ulVal);
//------------------------------------------------------------------------
/*!
\brief Get sensor vertical start point.
\return Get sensor vertical start point value.
\par [Example]
\code 
		ulIQ_GetVStart();
\endcode
*/
uint32_t ulIQ_GetVStart(void);
//------------------------------------------------------------------------
/*!
\brief Set sensor vertical start point.
\param ulVal 		Set sensor vertical start point value.
\return(no)
\par [Example]
\code 
		IQ_SetVStart(ulAsicData);
\endcode
*/
void IQ_SetVStart(uint32_t ulVal);
//------------------------------------------------------------------------
/*!
\brief Get sensor horizontal size.
\return Get sensor horizontal size value.
\par [Example]
\code 
		ulIQ_GetHSize();
\endcode
*/
uint32_t ulIQ_GetHSize(void);
//------------------------------------------------------------------------
/*!
\brief Set sensor horizontal size.
\param ulVal 		Set sensor horizontal size value.
\return(no)
\par [Example]
\code 
		IQ_SetHSize(ulAsicData);
\endcode
*/
void IQ_SetHSize(uint32_t ulVal);
//------------------------------------------------------------------------
/*!
\brief Get sensor vertical size.
\return Get sensor vertical size value.
\par [Example]
\code 
		ulIQ_GetVSize();
\endcode
*/
uint32_t ulIQ_GetVSize(void);
//------------------------------------------------------------------------
/*!
\brief Set sensor vertical size.
\param ulVal 		Set sensor vertical size value.
\return(no)
\par [Example]
\code 
		IQ_SetVSize(ulAsicData);
\endcode
*/
void IQ_SetVSize(uint32_t ulVal);
//------------------------------------------------------------------------
/*!
\brief Get ReorderPattern.
\return Get ReorderPattern value.
\par [Example]
\code 
		ulIQ_GetIspReorderPattern();
\endcode
*/
uint32_t ulIQ_GetIspReorderPattern(void);
//------------------------------------------------------------------------
/*!
\brief Set ReorderPattern.
\param ulPattern 	Set ReorderPattern value.
\return(no)
\par [Example]
\code 
		IQ_SetIspReorderPattern(ulAsicData);
\endcode
*/
void IQ_SetIspReorderPattern(uint32_t ulPattern);
//------------------------------------------------------------------------
/*!
\brief Get ISP function flag.
\return Get ISP function state.
\par [Example]
\code 
		ulIQ_GetIspFuncFlag();
\endcode
*/
uint32_t ulIQ_GetIspFuncFlag(void);
//------------------------------------------------------------------------
/*!
\brief Set ISP function flag.
\param ulFlag 			Set ISP function state.
\return(no)
\par [Example]
\code 
		IQ_SetIspFuncFlag(ulAsicData);
\endcode
*/
void IQ_SetIspFuncFlag(uint32_t ulFlag);
//------------------------------------------------------------------------
/*!
\brief Tuning tool switch.
\param Mode 			flag.
\return(no)
\par [Example]
\code 
		IQ_SetupTuningToolMode(KNL_TUNINGMODE_OFF);
\endcode
*/
void IQ_SetupTuningToolMode(uint8_t Mode);
//------------------------------------------------------------------------------
/*!
\brief 	Get load IQ.bin finish flag.
\return	falg.
\par [Example]
\code		 
	 ubIQ_GetLoadIqFinishFg();
\endcode
*/
uint8_t ubIQ_GetLoadIqFinishFg(void);
//------------------------------------------------------------------------------
/*!
\brief 	Set load IQ.bin finish flag.	
\param ubFg falg.
\return	(no)
\par [Example]
\code		 
	 IQ_SetLoadIqFinishFg(1);
\endcode
*/
void IQ_SetLoadIqFinishFg(uint8_t ubFg);
//------------------------------------------------------------------------------
/*!
\brief 	copy parser relation information parameter.
\return	(no)
*/
void IQ_ParserParaInfProcess(IQ_PARSER_INF tProcess);
//------------------------------------------------------------------------------
/*!
\brief 	Set IQ bin file frame rate.	
\param ubValue frame rate.
\return	(no)
*/
void IQ_SetIQBinFrameRate(uint8_t ubValue);
//------------------------------------------------------------------------------
/*!
\brief 	Get IQ bin file frame rate.	
\return	IQ bin file frame rate
*/
uint8_t ubIQ_GetIQBinFrameRate(void);
//------------------------------------------------------------------------------
/*!
\brief 	print IQ bin version.	
\return	(no)
*/
void IQ_PrintIqBinVer(void);
#endif
