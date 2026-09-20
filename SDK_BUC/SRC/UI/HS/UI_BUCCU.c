/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		UI_BUCCAM.c
	\brief		User Interface of BUC Camera Unit (for High Speed Mode)
	\author		Hanyi Chiu
	\version	0.5
	\date		2019/12/24
	\copyright	Copyright (C) 2019 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#include <string.h>
#include <stdlib.h>
#include "BSP.h"
#include "UI_BUCCU.h"
#include "SEN.h"
#include "RTC_API.h"
#include "ISP_API.h"
#include "MD_API.h"
#include "SF_API.h"
#include "Buzzer.h"
#include "FWU_API.h"
#include "SADC.h"
#include "ADO.h"

#define osUI_SIGNALS	0x66
#if (defined(BSP_D_SNCC70_TX_V2)||defined(BSP_D_SN93714_TX_V1))
/**
 * Key event mapping table
 *
 * @param ubKeyID  			Key ID
 * @param ubKeyCnt 			Key count	(100ms per 1 count, ex.long press 5s, the count set to 50)
 * @param KeyEventFuncPtr 	Key event mapping to function
 */
UI_KeyEventMap_t UiKeyEventMap[] =
{
	{NULL,				0,					NULL},
#if defined(BSP_SN93712_VBM_TX_V1)
	{GKEY_ID6, 			0,					UI_PairingKey},
#elif (defined(BSP_SNCC72_AHD_RN6752M_TX_V1) || defined(BSP_SNCC72_AHD_TP9950_TX_V2)) 
	{GKEY_ID0, 			0,					UI_PairingKey},
#elif ( defined(BSP_D_SNCC72_TX_V1))
	{GKEY_ID8, 			0,					UI_PairingKey},
    {GKEY_ID8, 			50,					UI_FwUpdateKey},
#else
	{PKEY_ID0, 			0,					UI_PairingKey},
	{PKEY_ID0, 			50,					UI_FwUpdateKey},
#endif
	{PKEY_ID0, 			20,					UI_PowerKey},
};

static APP_State_t tUI_SyncAppState;
static uint8_t ubUI_ClearThdCntFlag;
static UI_ThreadNotify_t tosUI_Notify;
osSemaphoreId osUI_CuSemId;
//------------------------------------------------------------------------------
void UI_KeyEventExec(void *pvKeyEvent)
{
	static uint8_t ubUI_KeyEventIdx = 0;
	uint16_t uwUiKeyEvent_Cnt = 0, uwIdx;

	KEY_Event_t *ptKeyEvent = (KEY_Event_t *)pvKeyEvent;
	uwUiKeyEvent_Cnt = sizeof UiKeyEventMap / sizeof(UI_KeyEventMap_t);
	if(ptKeyEvent->ubKeyAction == KEY_UP_ACT)
	{
		if((ubUI_KeyEventIdx) && (ubUI_KeyEventIdx < uwUiKeyEvent_Cnt))
		{
			if(UiKeyEventMap[ubUI_KeyEventIdx].KeyEventFuncPtr)
				UiKeyEventMap[ubUI_KeyEventIdx].KeyEventFuncPtr();
		}
		ubUI_KeyEventIdx = 0;
		return;
	}
	for(uwIdx = 1; uwIdx < uwUiKeyEvent_Cnt; uwIdx++)
	{
		if((ptKeyEvent->ubKeyID  == UiKeyEventMap[uwIdx].ubKeyID) &&
		   (ptKeyEvent->uwKeyCnt == UiKeyEventMap[uwIdx].uwKeyCnt))
		{
			ubUI_KeyEventIdx = uwIdx;
//			if((ptKeyEvent->uwKeyCnt != 0) && (UiKeyEventMap[ubUI_KeyEventIdx].KeyEventFuncPtr))
//			{
//				UiKeyEventMap[ubUI_KeyEventIdx].KeyEventFuncPtr();
//				ubUI_KeyEventIdx = 0;
//			}
		}
	}
}
//------------------------------------------------------------------------------
void UI_StateReset(void)
{
	osSemaphoreDef(UiCuSem);
	osUI_CuSemId = osSemaphoreCreate(osSemaphore(UiCuSem), 1);
    
      if(tTWC_RegTransCbFunc(TWC_UI_SETTING, UI_RecvCamResponse, NULL) != TWC_SUCCESS)
		printd(DBG_ErrorLvl, "UI Setting 2-way command fail!\n");
}
//------------------------------------------------------------------------------
void UI_UpdateFwUpgStatus(void *ptUpgStsReport)
{
}
//------------------------------------------------------------------------------
void UI_UpdateAppStatus(void *ptAppStsReport)
{
	APP_StatusReport_t *pAppStsRpt = (APP_StatusReport_t *)ptAppStsReport;
	osSemaphoreWait(osUI_CuSemId, osWaitForever);
	switch(pAppStsRpt->tAPP_ReportType)
	{
		case APP_PAIRSTS_RPT:
		{
			PAIRING_LED_IO = 0;
			break;
		}
		case APP_LINKSTS_RPT:
			break;
		default:
			break;
	}

	tUI_SyncAppState = pAppStsRpt->tAPP_State;

	osSemaphoreRelease(osUI_CuSemId);
}
//------------------------------------------------------------------------------
void UI_UpdateStatus(uint16_t *pThreadCnt)
{
	APP_EventMsg_t tUI_GetLinkStsMsg = {0};

	osSemaphoreWait(osUI_CuSemId, osWaitForever);	
	UI_CLEAR_THREADCNT(ubUI_ClearThdCntFlag, *pThreadCnt);
	switch(tUI_SyncAppState)
	{
		case APP_LINK_STATE:
			(*pThreadCnt)++;
			break;
		case APP_LOSTLINK_STATE:
			break;
		case APP_PAIRING_STATE:
			if((*pThreadCnt % UI_PAIRINGLED_PERIOD) == 0)
				PAIRING_LED_IO = ~PAIRING_LED_IO;
			(*pThreadCnt)++;
			osSemaphoreRelease(osUI_CuSemId);
			return;
		default:
			break;
	}
	PAIRING_LED_IO = 0;
	tUI_GetLinkStsMsg.ubAPP_Event = APP_LINKSTATUS_REPORT_EVENT;
	UI_SendMessageToAPP(&tUI_GetLinkStsMsg);
	osSemaphoreRelease(osUI_CuSemId);
}
//------------------------------------------------------------------------------
void UI_EventHandles(UI_Event_t *ptEventPtr)
{
	switch(ptEventPtr->tEventType)
	{
		case AKEY_EVENT:
		case PKEY_EVENT:
		case GKEY_EVENT:
			UI_KeyEventExec(ptEventPtr->pvEvent);
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_PowerKey(void)
{
	BUZ_PlayPowerOffSound();
	osDelay(600);			//wait buzzer play finish
	POWER_LED_IO  = 0;
	SIGNAL_LED_IO(0);
	RTC_WriteUserRam(RTC_RECORD_PWRSTS_ADDR, RTC_PWRSTS_KEEP_TAG);
	RTC_SetGPO_1(0, RTC_PullDownEnable);
	printd(DBG_Debug1Lvl, "Power OFF!\n");
	RTC_PowerDisable();
	while(1);
}
//------------------------------------------------------------------------------
void UI_PairingKey(void)
{
	APP_EventMsg_t tUI_PairMessage = {0};

	tUI_PairMessage.ubAPP_Event = (APP_PAIRING_STATE == tUI_SyncAppState)?APP_PAIRING_STOP_EVENT:APP_PAIRING_START_EVENT;
	UI_SendMessageToAPP(&tUI_PairMessage);
	BUZ_PlaySingleSound();
}
//------------------------------------------------------------------------------
void UI_FwUpdateKey(void)
{
    printf("UI_FwUpdateKeyF !\n");
    FWU_Enable();
}
//------------------------------------------------------------------------------
void UI_SystemSetup(void)
{

}
//------------------------------------------------------------------------------
void UI_ResetDevSetting(void)
{

}
//------------------------------------------------------------------------------
APP_EventMsg_t *tUI_ViewTypeSetup(UI_CamViewType_t tViewType)
{
    static APP_EventMsg_t tUI_ViewTypeParam = {0};
	return &tUI_ViewTypeParam;    
}

//------------------------------------------------------------------------------
UI_Result_t UI_SendRequestToCAM(osThreadId thread_id, UI_CUReqCmd_t *ptReqCmd)
{
	UI_Result_t tReq_Result = rUI_SUCCESS;
	osEvent tReq_Event;
	APP_StaNumMap_t *pUI_CamNumMap = APP_GetSTANumMappingTable(ptReqCmd->tDS_CamNum);
	uint8_t ubUI_TwcRetry = 5;
	
	tosUI_Notify.thread_id = thread_id;
	tosUI_Notify.iSignals  = osUI_SIGNALS;
	while(--ubUI_TwcRetry)
	{
		if(tTWC_Send(pUI_CamNumMap->tTWC_StaNum, TWC_UI_SETTING, ptReqCmd->ubCmd, ptReqCmd->ubCmd_Len, 10) == TWC_SUCCESS)
			break;
		osDelay(10);
	}
	if(!ubUI_TwcRetry)
	{
		tTWC_StopTwcSend(pUI_CamNumMap->tTWC_StaNum, TWC_UI_SETTING);
		return rUI_FAIL;
	}
	if(tosUI_Notify.thread_id != NULL)
	{
		tReq_Event = osSignalWait(tosUI_Notify.iSignals, UI_TWC_TIMEOUT);
		tReq_Result = (tReq_Event.status == osEventSignal)?(tReq_Event.value.signals == tosUI_Notify.iSignals)?tosUI_Notify.tReportSts:rUI_FAIL:rUI_FAIL;
		tTWC_StopTwcSend(pUI_CamNumMap->tTWC_StaNum, TWC_UI_SETTING);
		tosUI_Notify.thread_id  = NULL;
		tosUI_Notify.iSignals   = NULL;
		tosUI_Notify.tReportSts = rUI_SUCCESS;
	}
	return tReq_Result;
}
//------------------------------------------------------------------------------
void UI_RecvCamResponse(TWC_TAG tRecv_StaNum, TWC_STATUS tStatus)
{
	if(NULL == tosUI_Notify.thread_id)
		return;
	tosUI_Notify.tReportSts = (tStatus == TWC_SUCCESS)?rUI_SUCCESS:rUI_FAIL;
	if(osSignalSet(tosUI_Notify.thread_id, osUI_SIGNALS) != osOK)
		printd(DBG_ErrorLvl, "UI thread notify fail !\n");
}
#endif

