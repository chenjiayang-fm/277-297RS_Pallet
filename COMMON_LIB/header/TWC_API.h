/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		TWC_API.h
	\brief		Two Way Command API header
	\author		Bing
	\version	0.1
	\date		2017/02/16
	\copyright	Copyright(C) 2017 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------
#ifndef _TWC_API_H_
#define _TWC_API_H_

#include "_510PF.h"

typedef enum
{
	TWC_STA1		= 0,
	TWC_STA2		= 1,
	TWC_STA3		= 2,
	TWC_STA4		= 3,
	TWC_AP_SLAVE	= 4,
	TWC_AP_MASTER	= 0xF,
}TWC_TAG;

typedef enum
{
	TWC_RESEV	= 0,
	TWC_Test1	= 1,
	TWC_Test2	= 2,
	TWC_APP,
	TWC_EN,
	TWC_PRP,
    TWC_PAP,
    TWC_PAAP,
	TWC_UI_SETTING,
	TWC_RESEND_I_EXT,	//For External
	TWC_RESEND_I_INT,	//For Internal
	TWC_VDORES_SETTING,
    TWC_SYNC_TIMESTAMP,
    TWC_SYNC_GOP,
	TWC_STA_ON_LINE,
    TWC_SYNC_FRAME,
	TWC_TRXBW_RPT,
	TWC_TX_AHD_RPT,
	TWC_TX_REC_RPT,
	TWC_RF433_BTY_INFO,//RF433	
	TWC_Laser_CTRL,
	TWC_Led_CTRL,
	TWC_TX_SIGNAL_RPT = 30,
	TWC_TX_TYPE,
	TWC_TX_VERSION,
	TWC_ZOOM,
	TWC_TX_PCCNT_RPT,
	TWC_PELCO = 34,
	/*TWC_RESEV	= 0,
	TWC_Test1	= 1,
	TWC_Test2	= 2,
	TWC_APP,
	TWC_EN,
	TWC_PRP,
    TWC_PAP,
    TWC_PAAP,
	TWC_UI_SETTING,
	TWC_RESEND_I_EXT,	//For External
	TWC_RESEND_I_INT,	//For Internal
	TWC_VDORES_SETTING,
    TWC_SYNC_TIMESTAMP,
    TWC_SYNC_GOP,
	TWC_STA_ON_LINE,
    TWC_SYNC_FRAME,
	TWC_TRXBW_RPT,
	TWC_TX_AHD_RPT,
	TWC_TX_REC_RPT,

	TWC_TX_PCCNT_RPT,*/
//	............
	TWC_USE63	= 63,	 
}TWC_OPC;

typedef enum
{
	TWC_SUCCESS	= 0,
	TWC_FAIL,
	TWC_BUSY,
}TWC_STATUS;

typedef void(*pvRptTxStsFunc)(TWC_TAG, TWC_STATUS);
typedef void(*pvRecvDataFunc)(TWC_TAG, uint8_t *);

#ifdef A7130
#define TWC_Init					TWCA_Init
#define TWC_Start					TWCA_Start
#define tTWC_RegTransCbFunc(...)	tTWCA_RegTransCbFunc(__VA_ARGS__)
#define tTWC_Send(...)				tTWCA_Send(__VA_ARGS__)
#define tTWC_StopTwcSend(...)		tTWCA_StopTwcSend(__VA_ARGS__)
#define uwTWC_GetVersion			uwTWCA_GetVersion

//------------------------------------------------------------------------
/*!
\brief TWC Initial
\param (No)
\return(No)
*/
void TWCA_Init(void);
//------------------------------------------------------------------------
/*!
\brief TWC Start
\return (No)
*/
void TWCA_Start(void);
//------------------------------------------------------------------------
/*!
\brief Register transaction callback function of TWC
\param Opc				(input) TWC Operating Code
\param ReportSts_cb 	(input) Callback function: Report transmit status (ACK mode)
\param RecvData_cb 		(input) Callback function: Receive data
\return TWC_STATUS \n
		(TWC_SUCCESS)	Get TWC,Reqistered Success \n
		(TWC_FAIL)		Get TWC,Reqistered Fail \n
*/
TWC_STATUS tTWCA_RegTransCbFunc(TWC_OPC Opc, pvRptTxStsFunc ReportSts_cb, pvRecvDataFunc RecvData_cb);
//------------------------------------------------------------------------
/*!
\brief Send TWC Command
\param Tag 		(input) Send TWC Target
\param Opc		(input) Send TWC Operating Code
\param *Data 	(input) Send TWC Data Address Point
\param ubLen 	(input) Send TWC Data Length
\param uRetry 	(input) Send TWC Retry Time
\par Note
	1. Operating Code = 1 ~ 63 \n
	2. Data Length Max = 32Bytes \n
\return TWC_STATUS \n
		(TWC_SUCCESS)	Set TWC In Buffer Success \n
		(TWC_FAIL)		Set TWC In Buffer Fail \n
		(TWC_BUSY)		Set TWC In Buffer Busy \n	
*/
TWC_STATUS tTWCA_Send(TWC_TAG Tag,TWC_OPC Opc,uint8_t *Data,uint8_t ubLen,uint8_t ubRetry);
//------------------------------------------------------------------------
/*!
\brief Stop Send TWC Command
\param Tag 		(input) Stop Send TWC Target
\param Opc		(input) Stop Send TWC Operating Code
\return TWC_STATUS \n
		(TWC_SUCCESS)	TWC Operating Code Exist \n
		(TWC_FAIL)		TWC Operating Code Not Exist \n
*/
TWC_STATUS tTWCA_StopTwcSend(TWC_TAG Tag,TWC_OPC Opc);
//------------------------------------------------------------------------
/*!
\brief 	Get TWC Version	
\return	Version
*/
uint16_t uwTWCA_GetVersion(void);

#endif //! End of A7130

#ifdef RTC676x

#define TWC_Init					TWCR_Init
#define TWC_Start					TWCR_Start
#define tTWC_RegTransCbFunc(...)	tTWCR_RegTransCbFunc(__VA_ARGS__)
#define tTWC_Send(...)				tTWCR_Send(__VA_ARGS__)
#define tTWC_StopTwcSend(...)		tTWCR_StopTwcSend(__VA_ARGS__)
#define uwTWC_GetVersion			uwTWCR_GetVersion

//------------------------------------------------------------------------
/*!
\brief TWC Interface Initial
\param (No)
\return(No)
*/
void TWCR_Init(void);
//------------------------------------------------------------------------
/*!
\brief TWC Interface Start
\return (No)
*/
void TWCR_Start(void);
//------------------------------------------------------------------------
/*!
\brief Register transaction callback function of TWC
\param Opc				(input) TWC Operating Code
\param ReportSts_cb 	(input) Callback function: Report transmit status (ACK mode)
\param RecvData_cb 		(input) Callback function: Receive data
\return TWC_STATUS \n
		(TWC_IF_SUCCESS)	Get TWC,Reqistered Success \n
		(TWC_IF_FAIL)		Get TWC,Reqistered Fail \n
*/
TWC_STATUS tTWCR_RegTransCbFunc(TWC_OPC Opc, pvRptTxStsFunc ReportSts_cb, pvRecvDataFunc RecvData_cb);
//------------------------------------------------------------------------
/*!
\brief Send TWC Interface Command
\param Tag 		(input) Send TWC Target
\param Opc		(input) Send TWC Operating Code
\param *Data 	(input) Send TWC Data Address Point
\param ubLen 	(input) Send TWC Data Length
\param uRetry 	(input) Send TWC Retry Time
\par Note
	1. Operating Code = 1 ~ 63 \n
	2. Data Length Max = 32Bytes \n
\return TWC_STATUS \n
		(TWC_SUCCESS)	Set TWC In Buffer Success \n
		(TWC_FAIL)		Set TWC In Buffer Fail \n
		(TWC_BUSY)		Set TWC In Buffer Busy \n	
*/
TWC_STATUS tTWCR_Send(TWC_TAG Tag, TWC_OPC Opc, uint8_t *Data, uint8_t ubLen, uint8_t ubRetry);
//------------------------------------------------------------------------
/*!
\brief Stop Send TWC Interface Command
\param Tag 		(input) Stop Send TWC Target
\param Opc		(input) Stop Send TWC Operating Code
\return TWC_IF_STATUS \n
		(TWC_IF_SUCCESS)	TWC Operating Code Exist \n
		(TWC_IF_FAIL)		TWC Operating Code Not Exist \n
*/
TWC_STATUS tTWCR_StopTwcSend(TWC_TAG Tag, TWC_OPC Opc);
//------------------------------------------------------------------------
/*!
\brief 	Get TWC Interface Version	
\return	Version
*/
uint16_t uwTWCR_GetVersion(void);
//------------------------------------------------------------------------
void TWCR_RcvData(uint8_t ubSN, TWC_OPC Opc, uint8_t *pBuf);

#endif	//! End of RTC676x

#ifdef S2019A

#define TWC_Init					TWCW_Init
#define TWC_Start					TWCW_Start
#define tTWC_RegTransCbFunc(...)	tTWCW_RegTransCbFunc(__VA_ARGS__)
#define tTWC_Send(...)				tTWCW_Send(__VA_ARGS__)
#define tTWC_StopTwcSend(...)		tTWCW_StopTwcSend(__VA_ARGS__)
#define uwTWC_GetVersion			uwTWCW_GetVersion

//------------------------------------------------------------------------
/*!
\brief TWC Interface Initial
\param (No)
\return(No)
*/
void TWCW_Init(void);
//------------------------------------------------------------------------
/*!
\brief TWC Interface Start
\return (No)
*/
void TWCW_Start(void);
//------------------------------------------------------------------------
/*!
\brief Register transaction callback function of TWC
\param Opc				(input) TWC Operating Code
\param ReportSts_cb 	(input) Callback function: Report transmit status (ACK mode)
\param RecvData_cb 		(input) Callback function: Receive data
\return TWC_STATUS \n
		(TWC_IF_SUCCESS)	Get TWC,Reqistered Success \n
		(TWC_IF_FAIL)		Get TWC,Reqistered Fail \n
*/
TWC_STATUS tTWCW_RegTransCbFunc(TWC_OPC Opc, pvRptTxStsFunc ReportSts_cb, pvRecvDataFunc RecvData_cb);
//------------------------------------------------------------------------
/*!
\brief Send TWC Interface Command
\param Tag 		(input) Send TWC Target
\param Opc		(input) Send TWC Operating Code
\param *Data 	(input) Send TWC Data Address Point
\param ubLen 	(input) Send TWC Data Length
\param uRetry 	(input) Send TWC Retry Time
\par Note
	1. Operating Code = 1 ~ 63 \n
	2. Data Length Max = 32Bytes \n
\return TWC_STATUS \n
		(TWC_SUCCESS)	Set TWC In Buffer Success \n
		(TWC_FAIL)		Set TWC In Buffer Fail \n
		(TWC_BUSY)		Set TWC In Buffer Busy \n	
*/
TWC_STATUS tTWCW_Send(TWC_TAG Tag, TWC_OPC Opc, uint8_t *Data, uint8_t ubLen, uint8_t ubRetry);
//------------------------------------------------------------------------
/*!
\brief Stop Send TWC Interface Command
\param Tag 		(input) Stop Send TWC Target
\param Opc		(input) Stop Send TWC Operating Code
\return TWC_IF_STATUS \n
		(TWC_IF_SUCCESS)	TWC Operating Code Exist \n
		(TWC_IF_FAIL)		TWC Operating Code Not Exist \n
*/
TWC_STATUS tTWCW_StopTwcSend(TWC_TAG Tag, TWC_OPC Opc);
//------------------------------------------------------------------------
/*!
\brief 	Get TWC Interface Version	
\return	Version
*/
uint16_t uwTWCW_GetVersion(void);

#endif	//! End of #ifdef S2019A

#endif
