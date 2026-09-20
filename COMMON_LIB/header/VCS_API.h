/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		VCS_API.h
	\brief		Version Control API header file
	\author		Hanyi Chiu
	\version	0.1
	\date		2019/10/18
	\copyright	Copyright(C) 2019 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _VCS_API_H_
#define _VCS_API_H_

#include "_510PF.h"

//------------------------------------------------------------------------------
/*!
\brief 	Obtain VCS Version	
\return	Version
*/
uint16_t uwVCS_GetVersion(void);

#endif
