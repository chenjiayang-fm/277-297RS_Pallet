/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		AE_API.h
	\brief		Auto exposure API header
	\author			
	\version	2.1
	\date		2020-05-28
	\copyright	Copyright(C) 2020 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _AE_API_H_
#define _AE_API_H_

#include "_510PF.h"

//==============================================================================
// DEFINITION
//==============================================================================
/*!
\brief AE event status
*/
enum
{
	AE_EVENT_SUSPEND,               //!< AE suspend
	AE_EVENT_RESUME,                //!< AE resume
	AE_EVENT_VSYNC,                 //!< AE vsync
	AE_EVENT_START,                 //!< AE start
	AE_EVENT_STOP                   //!< AE stop
};

/*!
\brief AE return status
*/
typedef enum
{
	AE_SUCCESS	= 0,                //!< return success
	AE_FAIL,                        //!< retrun fail
}AE_STATUS;

/*!
\brief AE Opcode type1
*/
typedef enum
{
	AE_OPC_EXP_LINE         = 0,    //!< opcode "exposure line"
	AE_OPC_DUMMY_LINE       = 1,    //!< opcode "dummy line"
	AE_OPC_GROUP_HOLD_ON    = 2,    //!< opcode "hold on"
	AE_OPC_GROUP_HOLD_OFF   = 3,    //!< opcode "hold off"
	AE_OPC_MAX                      //!< opcode maximum
}AE_XuOpc_t;

/*!
\brief AE Opcode type2
*/
typedef enum
{
	AE_OPC2_GAIN            = 0,    //!< opcode2 "gain"
	AE_OPC2_CAL_EXP_DUM     = 1,    //!< opcode2 "calculate exposure and dummy"
    AE_OPC2_MAX_EXP_LINE    = 2,    //!< opcode2 "maximum exposure line"
	AE_OPC2_MAX                     //!< opcode2 maximum
}AE_Xu2Opc_t;

/*!
\brief AE relation setting structure
*/
typedef struct {
    uint32_t ulSensorPclk;				    //!< Sensor pixel clock
    uint32_t ulSensorPixelPerLine;	        //!< Sensor Pixel per line
    uint32_t ulSensorLinePerFrame;	        //!< Sensor lines per frame
    uint32_t ulSensorFrameRate;             //!< Sensor frame rate
    uint32_t ulMaximumSensorFrameRate;      //!< Sensor maximun frame rate
}AE_EVENT_PROCESS;

typedef void(*pvAeXuCbFunc)(void);
typedef void(*pvAeXu2CbFunc)(uint32_t);
typedef uint16_t (*pvAeCalExpLineCbFunc)(uint32_t);
//==============================================================================
// MACRO FUNCTION 
//==============================================================================
#define AE_Suspend()				{ AE_Statemachine(AE_EVENT_SUSPEND); }
#define AE_Resume()					{ AE_Statemachine(AE_EVENT_RESUME); }
#define AE_VSync()					{ AE_Statemachine(AE_EVENT_VSYNC); }
#define AE_Start()					{ AE_Statemachine(AE_EVENT_START); }
#define AE_Stop()					{ AE_Statemachine(AE_EVENT_STOP); }
//------------------------------------------------------------------------
/*!
\brief Set AE default state.
\return(no)
*/
void AE_Init(void);
//------------------------------------------------------------------------
/*!
\brief AE operating process.
\param ubEvent 	AE current state.
\return(no)
*/
void AE_Statemachine(uint8_t ubEvent);
//------------------------------------------------------------------------
/*!
\brief AE Frame End ISR Handler.
\return(no)
*/
void AE_FrmEndIsr_Handler(void);
//------------------------------------------------------------------------
/*!
\brief 	Set I value of PID.
\param  ubI     I value.
\return	(no)
*/
void AE_SetIvalue(uint8_t ubI);
//------------------------------------------------------------------------
/*!
\brief 	Get AE library version	
\return	AE lib version.
\par [Example]
\code		 
	 uwAE_GetVersion();
\endcode
*/
uint16_t uwAE_GetVersion (void);
//------------------------------------------------------------------------
/*!
\brief 	Set AE PID	
\param ubLowFrame 	low frame rate set to 1.
\return	(no)
\par [Example]
\code		 
	 AE_SetPID(1);
\endcode
*/
void AE_SetPID(uint8_t ubLowFrame);
//------------------------------------------------------------------------
/*!
\brief Set AE ctrl table value.
\return(no)
*/
void AE_SetCtrlTable(void);
//------------------------------------------------------------------------
/*!
\brief Set AE table value.
\return(no)
*/
void AE_SetIqValue(void);
uint8_t* pbAE_GetAlgVerID(void);
//------------------------------------------------------------------------
/*!
\brief 	Get AE state from AE struct.
\return	I value.
*/
uint8_t ubAE_GetPID_Ivalue(void);
//------------------------------------------------------------------------
/*!
\brief Get current exposure index.
\return exposure index.
*/
uint8_t ubAE_GetCurrExpIdx(void);
//------------------------------------------------------------------------
/*!
\brief Get power frequence.
\return 1:50Hz, 2:60Hz.
*/
uint8_t ubAE_GetPwrFreq(void);
//------------------------------------------------------------------------
/*!
\brief Get current gain.
\return gain value.
*/
uint16_t uwAE_GetCurrGain(void);
//------------------------------------------------------------------------
/*!
\brief Get current Y target.
\return Y value.
*/
uint32_t ulAE_GetCurrAbsY(void);
//------------------------------------------------------------------------
/*!
\brief Init AE callback function.
\return(no)
*/
void AE_CbFuncInit(void);
//------------------------------------------------------------------------
/*!
\brief Callback function for AE
\param tOpc 	Operation code
\param pvCb		Callback function
\return status
*/
AE_STATUS tAE_RegXuCbFunc(AE_XuOpc_t tOpc, pvAeXuCbFunc pvCb);
AE_STATUS tAE_RegXu2CbFunc(AE_Xu2Opc_t tOpc, pvAeXu2CbFunc pvCB);
void AE_CalExpLineCbFunc(pvAeCalExpLineCbFunc pvCB);
//------------------------------------------------------------------------
/*!
\brief process of callback function
\param tOpc 	Operation code
\return(no)
*/
void AE_CbFuncProcess(AE_XuOpc_t tOpc);
void AE_CbFunc2Process(AE_Xu2Opc_t tOpc, uint32_t ulValue);
uint16_t AE_CbFuncCalExpLineProcess(uint32_t ulValue);
//------------------------------------------------------------------------------
/*!
\brief 	copy AE relation information parameter.
\return	(no)
*/
void AE_EventProcess(AE_EVENT_PROCESS tProcess);
//------------------------------------------------------------------------
/*!
\brief Get image average Y sum.
\return Y value.
*/
uint32_t ulAE_AvgOfYSum(void);
//------------------------------------------------------------------------
/*!
\brief Set environment parameter.
\param ulAeExpIdx 	exposure index
\param ulAeGain 	gain
\return(no)
*/
void AE_SetEnvironment(uint32_t ulAeExpIdx, uint32_t ulAeGain);
#endif
