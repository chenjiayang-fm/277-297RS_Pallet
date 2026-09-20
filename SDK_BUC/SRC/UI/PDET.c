/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		PDET.c
	\brief		Power Detection Function
	\author		Justin Chen
	\version	1
	\date		2019/10/29
	\copyright	Copyright(C) 2019 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include "SADC.h"
#include "bsp_config.h"
#include "PDET.h"

void PDET_Init(void)
{		
	SET_PWR_REPORT_PIN;
}
