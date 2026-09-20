/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		DDR_API.h
	\brief		DDR Self-Refresh Funcation Header
	\author		Pierce
	\version	0.7
	\date		2019/10/18
	\copyright	Copyright(C) 2019 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _DDR_API_H_
#define _DDR_API_H_
#include "_510PF.h"
//------------------------------------------------------------------------------
//! DDR Infor
//#if defined(VBM_PU) || defined(BUC_CU)
//#if defined(VBM_PU)
//#define DDR_MB_SZ					(32)					//!< DDR memory size (N MB)
//#endif
//#if defined(BUC_CU)
//#define DDR_MB_SZ					(64)					//!< DDR memory size (N MB)
//#endif

//#else
//#define DDR_MB_SZ					(32)					//!< DDR memory size (N MB)
//#endif
#define DDR_ADDR_START				(0)						//!< DDR memory start address
//#define DDR_BSZ_MAX					(DDR_MB_SZ*1024*1024)
//#define DDR_ADDR_END				(0 + DDR_BSZ_MAX - 1)	//!< DDR memory end address

typedef enum
{
	SYS_PERMODE1,
	SYS_PERMODE2,
	SYS_PERMODE3,
	SYS_PERMODE4,
	SYS_PERMODE5,
	SYS_PERMODE6,
	SYS_PERMODE7,
}DDR_SysMode_t;

//------------------------------------------------------------------------------
/*!
	\brief 		DDR Self-Refresh Function	
	\par [Example]
	\code    
		 DDR_SelfRefresh();
	\endcode
*/
void DDR_SelfRefresh (void);

//------------------------------------------------------------------------------
/*!
	\brief 		Improve DDR performance
*/
void DDR_ImprovePriority(DDR_SysMode_t tSysMode);
//------------------------------------------------------------------------------
/*!
	\brief 	Get DDR Function Version	
	\return	Unsigned short value, high byte is the major version and low byte is the minor version
	\par [Example]
	\code		 
		 uint16_t uwVer;
		 
		 uwVer = uwDDR_GetVersion();
		 printf("DDR Version = %d.%d\n", uwVer >> 8, uwVer & 0xFF);
	\endcode
*/
uint16_t uwDDR_GetVersion (void);
//------------------------------------------------------------------------------
/*!
	\brief 	Get DDR Capacity.
	\return	DDR capacity, unit:Bytes
	\par [Example]
*/
uint32_t ulDDR_GetCapacity(void);
#endif
