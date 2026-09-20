/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		QR_API.h
	\brief		QR Decode API 
	\author		Bruce Hsu	
	\version	0.1
	\date		2018/09/11
	\copyright	Copyright(C) 2018 SONiX Technology Co.,Ltd. All rights reserved.
*/

#ifndef __QR_API__
#define __QR_API__

/** \defgroup QR_MODULE QR Code Modules
 * \ingroup OTHER_MODULE
 * 
 * @{
 */ 
 
#include "_510PF.h"

#define MAX_RESULT_LENGTH (2061+1)    //!< QR Decode Result Max Length 

typedef enum {
	QR_OK = 0,	//!< QR Decode OK 
	QR_Fail		//!< QR Decode Fail
}QR_Result;

/*!
\brief QR Decode Process
\param ulInputAddr		QR Code Input Address 
\param ulWidth			QR Code Width
\param ulHeight			QR Code Height
\param pDecodeText		Decoded Text
\return QR_Result
*/
QR_Result QRDecode(uint32_t ulInputAddr,uint32_t ulWidth,uint32_t ulHeight,int8_t * pDecodeText);
/*!
\brief QR Setup
\param ulWidth			QR Code Width
\param ulHeight			QR Code Height
\param pFinish_CB		Finish Call Back Function
\return(no)
*/
void QR_Setup(uint32_t ulWidth, uint32_t ulHeight, void (*pFinish_CB)(uint8_t *));
/*!
\brief QR Stop
\return(no)
*/
void QR_Stop(void);
/*!
\brief Take Mutex and Start QR Process
\param arg			QR Code Input Address
\return(no)
*/
void QR_Process(void * arg);
/*!
\brief QR Relase Mutex
\return(no)
*/
void QR_Release(void);
/*!
\brief Set QR Resume Status
\return(no)
*/
void QR_Resume(void);
/*!
\brief Set QR Suspend Status
\return(no)
*/
void QR_Suspend(void);

#endif
 /** @} */