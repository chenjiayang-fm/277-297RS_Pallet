/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		WDT.c
	\brief		Watch Dog Timer Function
	\author		Hanyi Chiu
	\version	0.5
	\date		2020/09/18
	\copyright	Copyright(C) 2020 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#include <stdio.h>
#include "WDT.h"
#include "INTC.h"

#define WDT_MAJORVER	0
#define WDT_MINORVER	5

uint32_t ulWDT_DeviceAddr[] = {WDRT_BASE, WDIT_BASE};
typedef void(*pvWDT_doReset)(void);
#ifdef RTOS
uint8_t ubWDT_OsThdListBuf[4096];
#endif
//------------------------------------------------------------------------------
void WDT_RST_Enable(WDT_CLK_SRC_t Clock_Src, uint8_t ubTimeOut)
{
	uint32_t ulClock = 0, ulTimer_Cnt;

	WDT_TimerClr(WDT_RST);
	WDT_TimerClr(WDT_INT);
	WDRT->WDOG_CTR     = 0;
	WDIT->WDOG_CTR     = 0;
	WDRT->WD_LENGTH    = 0xFF;
	WDIT->WD_LENGTH    = 0xFF;
	WDRT->WDOG_CLOCK   = Clock_Src;
	WDIT->WDOG_CLOCK   = Clock_Src;
	ulClock			   = (WDT_CLK_APBCLK == Clock_Src)?(IP_CLK/GLB->APBC_RATE):EXT_CLK;
	if(ubTimeOut)
	{
		ulTimer_Cnt		   = ulClock * (ubTimeOut - 0.2);
		WDIT->WDOG_LOAD    = ulTimer_Cnt;
		WDIT->WDOG_RESTART = 0x5AB9;
		#if (defined(RTOS) && (1 == OS_MAJOR_VER) && (OS_MINOR_VER < 3))
		INTC_IrqSetup(INTC_WDOG_IRQ, INTC_EDGE_TRIG, WDIT_INT_Handler);
		#else
		INTC_IrqSetup(INTC_WDOG_IRQ, WDIT_INT_Handler);
		#endif
		INTC_IrqEnable(INTC_WDOG_IRQ);
		WDIT->WDOG_INTR_EN = 1;
		WDIT->WDOG_EN      = 1;
	}
#if 1
	else
	{
		pvWDT_doReset pRstFuncPtr;

		WDRT->WDOG_LOAD    = ulClock / 10;
		WDRT->WDOG_RESTART = 0x5AB9;
		WDRT->WDOG_RST_EN  = 1;
		WDRT->WDOG_EN      = 1;
		pRstFuncPtr = (pvWDT_doReset)0x10109800;
		pRstFuncPtr();
	}
	ulTimer_Cnt		   = ulClock * ubTimeOut;
	WDRT->WDOG_LOAD    = ulTimer_Cnt;
	WDRT->WDOG_RESTART = 0x5AB9;
	WDRT->WDOG_RST_EN  = 1;
	WDRT->WDOG_EN      = 1;
#endif
}
//------------------------------------------------------------------------------
void WDT_Disable(WDT_DEIVCE_t tDevice)
{
	if(WDT_RST == tDevice)
		WDIT->WDOG_CTR = 0;
	WDT_DEV->WDOG_CTR = 0;
}
//------------------------------------------------------------------------------
void WDT_TimerClr(WDT_DEIVCE_t tDevice)
{
	if(WDT_RST == tDevice)
	{
		WDIT->CLR_WDOG_FLAG = 1;
		WDIT->WDOG_RESTART  = 0x5AB9;
	}
	WDT_DEV->CLR_WDOG_FLAG = 1;
	WDT_DEV->WDOG_RESTART  = 0x5AB9;
}
//------------------------------------------------------------------------------
void WDIT_INT_Handler(void)
{
#ifdef RTOS
	osStatus osGetSts;
	uint16_t uwIdx;
#endif
	WDIT->CLR_WDOG_FLAG = 1;
	WDIT->WDOG_RESTART  = 0x5AB9;
	INTC_IrqClear(INTC_WDOG_IRQ);
	printf("<<<WDT INT>>>\n");
	if((WDRT->WDOG_EN) && (WDRT->WDOG_RST_EN))
	{
		pvWDT_doReset pRstFuncPtr = (pvWDT_doReset)0x10109800;
		pRstFuncPtr();
	}
#ifdef RTOS
	else
	{
		INTC->IRQ0_EN = 0;
		INTC->IRQ1_EN = 0;
		osGetSts = osThreadList(&ubWDT_OsThdListBuf[0], sizeof(ubWDT_OsThdListBuf));
		if(osOK == osGetSts)
		{
			for(uwIdx = 0; uwIdx < sizeof(ubWDT_OsThdListBuf); uwIdx++)
				printf("%c", ubWDT_OsThdListBuf[uwIdx]);
		}
	}
#endif
}
//------------------------------------------------------------------------------
uint16_t uwWDT_GetVersion(void)
{
    return ((WDT_MAJORVER << 8) + WDT_MINORVER);
}
