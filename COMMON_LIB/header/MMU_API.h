/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		MMU_API.h
	\brief		MMU Function API Header
	\author		Pierce
	\version	0.7
	\date		2020/05/20
	\copyright	Copyright(C) 2020 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _MMU_API_H_
#define _MMU_API_H_
#include "_510PF.h"
#include "DDR_API.h"
//------------------------------------------------------------------------------
/*!
	\brief 	MMU Disable I-Cache Function	
	\par [Example]
	\code
		 MMU_DisableICache();
	\endcode
*/
void MMU_DisableICache(void);
//------------------------------------------------------------------------------
/*!
	\brief 	MMU Disable D-Cache Function	
	\par [Example]
	\code
		 MMU_DisableDCache();
	\endcode
*/
void MMU_DisableDCache(void);
//------------------------------------------------------------------------------
/*!
	\brief 	MMU Disable MMU Function	
	\par [Example]
	\code
		 MMU_DisableMMU();
	\endcode
*/
void MMU_DisableMMU(void);
//------------------------------------------------------------------------------
/*!
	\brief 	MMU Enable I-Cache Function	
	\par [Example]
	\code
		 MMU_EnableICache();
	\endcode
*/
void MMU_EnableICache(void);
//------------------------------------------------------------------------------
/*!
	\brief 	MMU Enable D-Cache Function	
	\par [Example]
	\code
		 MMU_EnableDCache();
	\endcode
*/
void MMU_EnableDCache(void);
//------------------------------------------------------------------------------
/*!
	\brief 	MMU Enable MMU Function	
	\par [Example]
	\code
		 MMU_EnableMMU();
	\endcode
*/
void MMU_EnableMMU(void);
//------------------------------------------------------------------------------
/*!
	\brief 	MMU Get CPU State Function	
	\return	CPU state vaule
	\note	The function prints the I-Cache state\n
			The function prints the D-Cache state\n
			The function prints the MMU state\n
	\par [Example]
	\code
		 ulMMU_CpuState();
	\endcode
*/
uint32_t ulMMU_CpuState(void);
//------------------------------------------------------------------------------
/*!
	\brief 		MMU Initial Function
	\par [Example]
	\code
		 MMU_Init(void);
	\endcode
*/
void MMU_Init(void);
//------------------------------------------------------------------------------
/*!
	\brief 		Get I-Cache Start Address Function	
	\return	    I-Cache start address
	\par [Example]
	\code
		 uint32_t ulICacheAddr;		 
		 ulICacheAddr = ulMMU_GetICacheStartAddr();
	\endcode
*/
uint32_t ulMMU_GetICacheStartAddr(void);
//------------------------------------------------------------------------------
/*!
	\brief 		Get I-Cache Size Function	
	\return	    I-Cache Size
	\par [Example]
	\code
		 uint32_t ulRoSize;		 
		 ulRoSize = ulMMU_GetICacheSize();
	\endcode
*/
uint32_t ulMMU_GetICacheSize(void);
//------------------------------------------------------------------------------
/*!
	\brief 		Get D-Cache Start Address Function	
	\return	    D-Cache start address
	\par [Example]
	\code
		 uint32_t ulDCacheAddr;		 
		 ulDCacheAddr = ulMMU_GetDCacheStartAddr();
	\endcode
*/
uint32_t ulMMU_GetDCacheStartAddr(void);
//------------------------------------------------------------------------------
/*!
	\brief 		Get D-Cache Size Function	
	\return	    D-Cache Size
	\par [Example]
	\code
		 uint32_t ulRwSize;		 
		 ulRwSize = ulMMU_GetDCacheSize();
	\endcode
*/
uint32_t ulMMU_GetDCacheSize(void);
//------------------------------------------------------------------------------
/*!
	\brief 		Get Cache Heap Start Address Function	
	\return	    Cache Heap start address
	\par [Example]
	\code
		 uint8_t *pCacheHeap;		 
		 pCacheHeap = (uint8_t*)ulMMU_GetCacheHeapStartAddr();
	\endcode
*/
uint32_t ulMMU_GetCacheHeapStartAddr(void);
//------------------------------------------------------------------------------
/*!
	\brief 		Get Un-Cache Heap Start Address Function	
	\return	    Un-Cache Heap start address
	\par [Example]
	\code
		 uint8_t *pUnCacheHeap;		 
		 pUnCacheHeap = (uint8_t*)ulMMU_GetUnCacheHeapStartAddr();
	\endcode
*/
uint32_t ulMMU_GetUnCacheHeapStartAddr(void);
//------------------------------------------------------------------------------
/*!
	\brief 		Get Un-Cache Heap Size Function	
	\return	    Un-Cache Heap size
	\par [Example]
	\code
		 uint32_t ulUnCacheHeapSzie;		 
		 ulUnCacheHeapSzie = ulMMU_GetUnCacheHeapSize();
	\endcode
*/
uint32_t ulMMU_GetUnCacheHeapSize(void);
//------------------------------------------------------------------------------
/*!
	\brief 		Get Buffer Start Address Function	
	\return	    Buffer start address
	\par [Example]
	\code
		 uint32_t ulBUF_InitFreeBufAddr;
		 
		 MMU_Init();
		 ulBUF_InitFreeBufAddr = ulMMU_GetBufStartAddr();
	\endcode
*/
uint32_t ulMMU_GetBufStartAddr(void);
//------------------------------------------------------------------------------
/*!
	\brief 	Get MMU Function Version	
	\return	Unsigned short value, high byte is the major version and low byte is the minor version
	\par [Example]
	\code		 
		 uint16_t uwVer;
		 
		 uwVer = uwMMU_GetVersion();
		 printf("MMU Version = %d.%d\n", uwVer >> 8, uwVer & 0xFF);
	\endcode
*/
uint16_t uwMMU_GetVersion (void);
#endif
