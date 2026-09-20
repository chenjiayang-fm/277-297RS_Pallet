/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		TWC_IF.h
	\brief		Two Way Command Interface API header
	\author		Justin
	\version	0.91
	\date		2019/05/05
	\copyright	Copyright(C) 2019 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------
#ifndef __TWCR_H
#define __TWCR_H

#include "_510PF.h"
#include "TWC_API.h"

#define TWCR_QUEUE_SIZE					64
#define TWCR_MAX_FUNCTION_OBJ 			64
#define TWCR_DATA_MAX_SIZE				32

typedef struct
{
	uint8_t	ubSN;		//Station Number
	uint8_t ubFuncIdx;	//Index for function(Fixed)
	uint8_t ubDataIdx;	//Index for Data	(Dynamic)	
}TWCR_Que_t;
	
#endif	/*__ST53510_TWC_IF_API_H*/


