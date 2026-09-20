/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		EFUSE_API.h
	\brief		e-Fuse Access function header file
	\author		Hanyi
	\version	0.3
	\date		2021/4/29
	\copyright	Copyright (C) 2019 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _EFUSE_API_H_
#define _EFUSE_API_H_
//------------------------------------------------------------------------------
#include "_510PF.h"

typedef enum
{
	EFUSE_RW_FAIL,
	EFUSE_RW_PASS,
}EFUSE_Result_t;

typedef enum
{
	EFUSE_DATA0 = 0,
	EFUSE_DATA1,
	EFUSE_DATA2,
	EFUSE_DATA3,
}EFUSE_UserDataAddr_t;

//------------------------------------------------------------------------------
/*!
\brief eFuse initialize
\return(no)
*/
void EFUSE_Init(void);
//------------------------------------------------------------------------------
/*!
\brief Read eFuse data
\param tUserAddr 		eFuse data address
\param pEfuseData		Data pointer for eFuse data
\param uwPassword		eFuse Password
\return read result
*/
EFUSE_Result_t tEFUSE_ReadUserData(EFUSE_UserDataAddr_t tUserAddr, uint32_t *pEfuseData, uint16_t uwPassword);
//------------------------------------------------------------------------------
/*!
\brief 	Get eFuse Version
\return	Version
*/
uint16_t uwEFUSE_GetVersion(void);

#endif
