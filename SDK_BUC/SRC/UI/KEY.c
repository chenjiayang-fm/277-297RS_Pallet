/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		KEY.c
	\brief		KEY function
	\author		Hanyi Chiu
	\version	0.3
	\date		2019/12/12
	\copyright	Copyright(C) 2019 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include "KEY.h"
#include "UI.h"

#define KEY_MAJORVER	0
#define KEY_MINORVER	3

osMessageQId *pKEY_EventQH;
static void KEY_Thread(void const *argument);
//------------------------------------------------------------------------------
void KEY_Init(osMessageQId *pEventQueueHandle)
{	
	PKEY_Init();
#ifndef AKEY_DISABLE	
	AKEY_Init();
#endif	
#if GKEY_EN
	GKEY_Init();
#endif
	pKEY_EventQH = pEventQueueHandle;
    osThreadDef(KEY_Thread, KEY_Thread, THREAD_PRIO_KEY_HANDLER, 1, THREAD_STACK_KEY_HANDLER);
    osThreadCreate(osThread(KEY_Thread), NULL);
}
//------------------------------------------------------------------------------
static void KEY_Thread(void const *argument)
{
	while(1)
	{
		PKEY_Thread();
	#ifndef AKEY_DISABLE	
		AKEY_Thread();
	#endif
    #if GKEY_EN
		GKEY_Thread();
	#endif
        osDelay(KEY_THREAD_PERIOD);
	}
}
//------------------------------------------------------------------------------
void KEY_QueueSend(KEY_Type_t tKeyType, void *pvKeyEvent)
{
	UI_Event_t tKeyEvent;

	//tKeyEvent.tEventType = (tKeyType == AKEY)?AKEY_EVENT:(tKeyType == PKEY)?PKEY_EVENT:EVENT_NONE;
	if(tKeyType == PKEY)
		tKeyEvent.tEventType = PKEY_EVENT;
#ifndef AKEY_DISABLE		
	else if(tKeyType == AKEY)
		tKeyEvent.tEventType = AKEY_EVENT;
#endif	
#if GKEY_EN
	else if(tKeyType == GKEY)
		tKeyEvent.tEventType = GKEY_EVENT;
#endif
#if 1
	else if(tKeyType == IRKEY)
		tKeyEvent.tEventType = IRKEY_EVENT;
	
#endif
		
	tKeyEvent.pvEvent 	 = pvKeyEvent;
    if(osMessagePut(*pKEY_EventQH, &tKeyEvent, 0) != osOK)
		printd(DBG_ErrorLvl, "KEY Q Full\n");
}
//------------------------------------------------------------------------------
uint16_t uwKEY_GetVersion(void)
{
    return ((KEY_MAJORVER << 8) + KEY_MINORVER);
}
