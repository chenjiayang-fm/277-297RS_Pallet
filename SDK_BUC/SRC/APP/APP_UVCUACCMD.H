/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		APP_UVCCMD.h
	\brief		UVC Command Application header file
	\author		Scott
	\version	1.0
	\date		2020/12/07
	\copyright	Copyright(C) 2018 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _APP_UVCUACCMD_H_
#define _APP_UVCUACCMD_H_

#include "_510PF.h"

void APP_UvcCtCmd(uint8_t ubCs, uint8_t ubLen, uint8_t * buf);

void APP_UvcPuCmd(uint8_t ubCs, uint8_t ubLen, uint8_t * buf);

void APP_UacCmd(uint8_t ubCs, uint8_t ubLen, uint8_t * buf);

void APP_UvcImageInf(uint8_t * list);
#endif
