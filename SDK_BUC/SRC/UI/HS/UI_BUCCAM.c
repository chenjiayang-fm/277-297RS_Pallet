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
	\version	0.7
	\date		2021/03/31
	\copyright	Copyright (C) 2021 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#include <string.h>
#include <stdlib.h>
#include "BSP.h"
#include "UI_BUCCAM.h"
#include "SEN.h"
#include "RTC_API.h"
#include "ISP_API.h"
#include "MD_API.h"
#include "SF_API.h"
#include "Buzzer.h"
#include "FWU_API.h"
#include "SADC.h"
#include "ADO.h"

#define osUI_SIGNALS	0x6A

/**
 * Key event mapping table
 *
 * @param ubKeyID  			Key ID
 * @param ubKeyCnt 			Key count	(100ms per 1 count, ex.long press 5s, the count set to 50)
 * @param KeyEventFuncPtr 	Key event mapping to function
 */
const UI_KeyEventMap_t UiKeyEventMap[] =
{
	{NULL,				0,					NULL},
#if defined(BSP_D_SN93712_VBM_TX_V2)
    #if (APP_DOORPHONE_ENABLE==0)
	{GKEY_ID6, 			0,					UI_PairingKey},
	#else
    {GKEY_ID6, 			0,					UI_DPPairingKey1},
    {GKEY_ID8, 			0,					UI_DPPairingKey2},
    {GKEY_ID17, 		0,					UI_DPKey1},
    {GKEY_ID20, 		0,					UI_DPKey2},
    #endif
#elif defined(BSP_D_SNCC72_TX_V1)
	{GKEY_ID8, 			0,					UI_PairingKey},
#elif (defined(BSP_SNCC72_AHD_RN6752M_TX_V1) || defined(BSP_SNCC72_AHD_TP9950_TX_V2)) 
	{GKEY_ID19, 		0,					UI_PairingKey},
#elif defined(BSP_SN93710_FHD_REC_TX_V4)
    #if (APP_DOORPHONE_ENABLE==0)
	{PKEY_ID0, 			0,					UI_PairingKey},
    #else
    {PKEY_ID0,          0,                  UI_DPPairingKey1},
    {GKEY_ID8,          0,                  UI_DPPairingKey2},
    {GKEY_ID9,          0,                  UI_DPKey1},
    {GKEY_ID9,          30,                 UI_DPPairingKey1},
    {GKEY_ID10,         0,                  UI_DPKey2},
    {GKEY_ID10,         30,                 UI_DPPairingKey2},
    #endif
#else
	{PKEY_ID0, 			0,					UI_PairingKey},
    {AKEY_MENU,         0,                  UI_ChgDisplayModeKey},
#endif
	{PKEY_ID0, 			20,					UI_PowerKey},
};
const UI_SettingFuncPtr_t tUiSettingMap2Func[] =
{
	[UI_PTZ_SETTING] 			= NULL,	
	[UI_RECMODE_SETTING] 		= NULL,
	[UI_RECRES_SETTING] 		= NULL,
	[UI_SDCARD_SETTING] 		= NULL,
	[UI_PHOTOMODE_SETTING] 		= NULL,
	[UI_PHOTORES_SETTING] 		= NULL,
	[UI_SYSINFO_SETTING] 		= NULL,
#ifdef A7130
	[UI_VOXMODE_SETTING] 		= UI_PowerSaveSetting,
	[UI_ECOMODE_SETTING] 		= UI_PowerSaveSetting,
	[UI_WORMODE_SETTING] 		= UI_PowerSaveSetting,
#else
	[UI_VOXMODE_SETTING] 		= NULL,
	[UI_ECOMODE_SETTING] 		= NULL,
	[UI_WORMODE_SETTING] 		= NULL,
#endif
	[UI_ADOANR_SETTING]			= UI_ANRSetting,
	[UI_ADOAEC_SETTING]			= UI_AECSetting,
	[UI_ADO_SETTING]			= UI_ADOSetting,
	[UI_IMGPROC_SETTING]		= UI_ImageProcSetting,
	[UI_MD_SETTING]				= NULL,
	[UI_VOICETRIG_SETTING]		= UI_VoiceTrigSetting,
	[UI_PERDBGMODE_SETTING]		= UI_PerDebugModeSetting,
};

const ADO_R2R_VOL tUI_VOLTable[] = {R2R_VOL_n45DB, R2R_VOL_n32p4DB, R2R_VOL_n26p2DB, R2R_VOL_n21p4DB, R2R_VOL_n14p6DB, R2R_VOL_n8p2DB};

static UI_CamStatus_t tUI_CamStsInfo;
static APP_State_t tUI_SyncAppState;
static UI_ThreadNotify_t tosUI_Notify;
static uint8_t ubUI_ClearThdCntFlag;
static uint8_t ubUI_WorModeEnFlag;
static uint8_t ubUI_WorWakeUpCnt;
static uint8_t ubUI_SyncDisVoxFlag;
static uint8_t ubUI_PerDbgMode;
osSemaphoreId osUI_CamSemId;
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
			if((ptKeyEvent->uwKeyCnt != 0) && (UiKeyEventMap[ubUI_KeyEventIdx].KeyEventFuncPtr))
			{
				UiKeyEventMap[ubUI_KeyEventIdx].KeyEventFuncPtr();
				ubUI_KeyEventIdx = 0;
			}
		}
	}
}
//------------------------------------------------------------------------------
void UI_StateReset(void)
{
	osSemaphoreDef(UiCamSem);
	osUI_CamSemId = osSemaphoreCreate(osSemaphore(UiCamSem), 1);
	if(tTWC_RegTransCbFunc(TWC_UI_SETTING, UI_RecvCUResponse, UI_RecvCURequest) != TWC_SUCCESS)
		printd(DBG_ErrorLvl, "UI setting 2way command fail !\n");
	ubUI_ClearThdCntFlag = FALSE;
	ubUI_SyncDisVoxFlag  = FALSE;
	UI_LoadDevStatusInfo();
#ifdef A7130
	ubUI_WorModeEnFlag = (PS_WOR_MODE == tUI_CamStsInfo.tCamPsMode)?TRUE:FALSE;
#else
	ubUI_WorModeEnFlag = FALSE;
#endif
	ubUI_WorWakeUpCnt = 0;
	tUI_CamStsInfo.tCamScanMode = CAMSET_OFF;
	ubUI_PerDbgMode = FALSE;
	KNL_SetDsUpdCbFunc(UI_ImageDateStampFunc);
#ifdef AKEY_DISABLE	
	SADC_Init();
	SADC_SetPinMode(SADC_CH1, SADC_PIN_ANALOG_INPUT_MODE);
	SADC_Enable();
#endif	
}
//------------------------------------------------------------------------------
void UI_UpdateFwUpgStatus(void *ptUpgStsReport)
{
}
//------------------------------------------------------------------------------
void UI_UpdateAppStatus(void *ptAppStsReport)
{
	APP_StatusReport_t *pAppStsRpt = (APP_StatusReport_t *)ptAppStsReport;
	static uint8_t ubUI_BuSysSetFlag = FALSE;

	osSemaphoreWait(osUI_CamSemId, osWaitForever);
	switch(pAppStsRpt->tAPP_ReportType)
	{
		case APP_PAIRSTS_RPT:
		{
			UI_Result_t tPair_Result = (UI_Result_t)pAppStsRpt->ubAPP_Report[0];

			if(rUI_SUCCESS == tPair_Result)
				UI_ResetDevSetting();
			PAIRING_LED_IO = 0;
			break;
		}
		case APP_LINKSTS_RPT:
			break;
		default:
			break;
	}
	ubUI_ClearThdCntFlag = (tUI_SyncAppState == pAppStsRpt->tAPP_State)?FALSE:TRUE;
	tUI_SyncAppState = pAppStsRpt->tAPP_State;
	if((FALSE == ubUI_BuSysSetFlag) && (APP_IDLE_STATE == tUI_SyncAppState))
	{
		UI_SystemSetup();
		ubUI_BuSysSetFlag = TRUE;
	}
	osSemaphoreRelease(osUI_CamSemId);
}
//------------------------------------------------------------------------------
#ifdef AKEY_DISABLE
const uint16_t uwUI_MirrorFlipTable[] = { 0x281, 0x1AD, 0xF6};
#endif
void UI_DetectMirrorFlip (void)
{
#ifdef AKEY_DISABLE	
#if (defined(BSP_SNCC70_AHD_TP9950_TX_V1) || defined(BSP_SNCC70_AHD_RN6752M_TX_V1))
	static UI_CamMirrorFlip_t tNowImg = UI_IMG_UNKNOW;
	UI_CamMirrorFlip_t 		  tImg;
	uint16_t 				 uwValue;
	
	//! Detect IO8
	uwValue = uwSADC_GetReport(SADC_CH1);
	tImg = (uwUI_MirrorFlipTable[0] < uwValue)?UI_IMG_MIRROR:
		   (uwUI_MirrorFlipTable[1] < uwValue)?UI_IMG_NORMAL:
		   (uwUI_MirrorFlipTable[2] < uwValue)?UI_IMG_MIRROR_FLIP:UI_IMG_FLIP;
	if (tNowImg != tImg)
	{
		tNowImg = tImg;
		switch (tNowImg)
		{
			case UI_IMG_MIRROR:
				ISP_SetMirrorFlip(1, 0);
				break;
			case UI_IMG_NORMAL:
				ISP_SetMirrorFlip(0, 0);
				break;
			case UI_IMG_MIRROR_FLIP:
				ISP_SetMirrorFlip(1, 1);
				break;
			case UI_IMG_FLIP:
				ISP_SetMirrorFlip(0, 1);
			default:
				break;
		}
	}
#endif
#endif	
}
//------------------------------------------------------------------------------
void UI_UpdateStatus(uint16_t *pThreadCnt)
{
	APP_EventMsg_t tUI_GetLinkStsMsg = {0};

	osSemaphoreWait(osUI_CamSemId, osWaitForever);	
	UI_DetectMirrorFlip();
	UI_CLEAR_THREADCNT(ubUI_ClearThdCntFlag, *pThreadCnt);
	switch(tUI_SyncAppState)
	{
		case APP_LINK_STATE:
#ifdef A7130
			if((TRUE == ubUI_WorModeEnFlag) && ((*pThreadCnt % UI_CHKWORSTS_PERIOD) != 0))
				UI_ChangePsModeToNormalMode();
			if(PS_VOX_MODE == tUI_CamStsInfo.tCamPsMode)
				UI_VoxTrigger();
#endif
			if(CAMSET_ON == tUI_CamStsInfo.tCamScanMode)
				UI_VoiceTrigger();
			if(MD_ON == tUI_CamStsInfo.MdParam.ubMD_Mode)
				UI_SetMotionEvent();
			if((*pThreadCnt % UI_UPDATESTS_PERIOD) != 0)
				UI_UpdateCamStatusToCU();
			(*pThreadCnt)++;
			break;
		case APP_LOSTLINK_STATE:
#ifdef A7130
			if(PS_WOR_MODE == tUI_CamStsInfo.tCamPsMode)
			{
				if(FALSE == ubUI_WorModeEnFlag)
					UI_ChangePsModeToWorMode();
				if(!ubUI_WorWakeUpCnt)
					UI_VoiceTrigger();
				else if(++ubUI_WorWakeUpCnt > (6000 / UI_TASK_PERIOD))
					UI_PowerSaveSetting(&tUI_CamStsInfo.tCamPsMode);
			}
#endif
			break;
		case APP_PAIRING_STATE:
			if((*pThreadCnt % UI_PAIRINGLED_PERIOD) == 0)
				PAIRING_LED_IO = ~PAIRING_LED_IO;
			(*pThreadCnt)++;
			osSemaphoreRelease(osUI_CamSemId);
			return;
		default:
			break;
	}
	PAIRING_LED_IO = 0;
	tUI_GetLinkStsMsg.ubAPP_Event = APP_LINKSTATUS_REPORT_EVENT;
	UI_SendMessageToAPP(&tUI_GetLinkStsMsg);
	osSemaphoreRelease(osUI_CamSemId);
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
	POWER_LED_IO   = 0;
	PAIRING_LED_IO = 0;
#if (!defined(BSP_D_SN93714_TX_V1) || !APP_SD_FUNC_ENABLE)	
	SIGNAL_LED_IO(0);
#endif	
	RTC_WriteUserRam(RTC_RECORD_PWRSTS_ADDR, RTC_PWRSTS_KEEP_TAG);
	RTC_SetGPO_1(0, RTC_PullDownEnable);
	printd(DBG_Debug1Lvl, "Power OFF !\n");
	RTC_PowerDisable();
	while(1);
}

//------------------------------------------------------------------------------
void UI_ChgDisplayModeKey(void)
{
#if (SEN_USE == SEN_GL3004)
    static uint8_t ubIdx = 0;

    ubIdx = ((ubIdx++) > 6)? 0:ubIdx; 
    SEN_ChgDisplayMode(ubIdx);
#endif
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
void UI_UpdateCamStatusToCU(void)
{
//	UI_CamReqCmd_t tUI_CamSts;

//	tUI_BuSts.ubCmd[UI_TWC_TYPE]		= UI_REPORT;
//	tUI_BuSts.ubCmd[UI_REPORT_ITEM] 	= UI_UPDATE_CAMSTS;
//	tUI_BuSts.ubCmd[UI_REPORT_DATA] 	= 100;
//	tUI_BuSts.ubCmd_Len  				= 3;
//	UI_SendRequestToCU(NULL, &tUI_CamSts);
}
//------------------------------------------------------------------------------
UI_Result_t UI_SendRequestToCU(osThreadId thread_id, UI_CamReqCmd_t *ptReqCmd)
{
	UI_Result_t tReq_Result = rUI_SUCCESS;
	osEvent tReq_Event;
	uint8_t ubUI_TwcRetry = 5;

	tosUI_Notify.thread_id = thread_id;
	tosUI_Notify.iSignals  = osUI_SIGNALS;
	while(--ubUI_TwcRetry)
	{
		if(tTWC_Send(TWC_AP_MASTER, TWC_UI_SETTING, ptReqCmd->ubCmd, ptReqCmd->ubCmd_Len, 10) == TWC_SUCCESS)
			break;
		osDelay(10);
	}
	if(!ubUI_TwcRetry)
	{
		tTWC_StopTwcSend(TWC_AP_MASTER, TWC_UI_SETTING);
		tosUI_Notify.thread_id = NULL;
		return rUI_FAIL;
	}
	if(tosUI_Notify.thread_id != NULL)
	{
		tReq_Event = osSignalWait(tosUI_Notify.iSignals, UI_TWC_TIMEOUT);
		tReq_Result = (tReq_Event.status == osEventSignal)?(tReq_Event.value.signals == tosUI_Notify.iSignals)?tosUI_Notify.tReportSts:rUI_FAIL:rUI_FAIL;
		tTWC_StopTwcSend(TWC_AP_MASTER, TWC_UI_SETTING);
		tosUI_Notify.thread_id  = NULL;
		tosUI_Notify.iSignals   = NULL;
		tosUI_Notify.tReportSts = rUI_SUCCESS;
	}
	return tReq_Result;
}
//------------------------------------------------------------------------------
void UI_RecvCUResponse(TWC_TAG tRecv_StaNum, TWC_STATUS tStatus)
{
//	UI_CamNum_t tCamNum;
//	TWC_TAG tTWC_StaNum;

//	APP_KNLRoleMap2CamNum(ubKNL_GetRole(), tCamNum);
//	tTWC_StaNum = APP_GetSTANumMappingTable(tCamNum)->tTWC_StaNum;
//	if((tRecv_StaNum != tTWC_StaNum) || (NULL == tosUI_Notify.thread_id))
//		return;
	if((tRecv_StaNum != TWC_AP_MASTER) || (NULL == tosUI_Notify.thread_id))
		return;
	tosUI_Notify.tReportSts = (tStatus == TWC_SUCCESS)?rUI_SUCCESS:rUI_FAIL;
	if(osSignalSet(tosUI_Notify.thread_id, osUI_SIGNALS) != osOK)
		printd(DBG_ErrorLvl, "UI thread notify fail !\n");
}
//------------------------------------------------------------------------------
void UI_RecvCURequest(TWC_TAG tRecv_StaNum, uint8_t *pTwc_Data)
{
//	UI_CamNum_t tCamNum;
//	TWC_TAG tTWC_StaNum;

//	APP_KNLRoleMap2CamNum(ubKNL_GetRole(), tCamNum);
//	tTWC_StaNum = APP_GetSTANumMappingTable(tCamNum)->tTWC_StaNum;
//	if(tRecv_StaNum != tTWC_StaNum)
//		return;
	switch(pTwc_Data[UI_TWC_TYPE])
	{
		case UI_SETTING:
			if(tUiSettingMap2Func[pTwc_Data[UI_SETTING_ITEM]].pvAction)
				tUiSettingMap2Func[pTwc_Data[UI_SETTING_ITEM]].pvAction((uint8_t *)(&pTwc_Data[UI_SETTING_DATA]));
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_SetMotionEvent(void)
{       
	if(ubMD_EvenStateFunc())
	{
		UI_CamReqCmd_t tUI_MdMsg;

		tUI_MdMsg.ubCmd[UI_TWC_TYPE]	= UI_REPORT;
		tUI_MdMsg.ubCmd[UI_REPORT_ITEM] = UI_MD_TRIG;
		tUI_MdMsg.ubCmd[UI_REPORT_DATA] = TRUE;
		tUI_MdMsg.ubCmd_Len  			= 3;
		UI_SendRequestToCU(NULL, &tUI_MdMsg);
	}
}
//------------------------------------------------------------------------------
void UI_SystemSetup(void)
{
    if (tUSBD_GetClassMode() != USBD_UVC_MODE)
        UI_IspSetup();
#if APP_ADO_AEC_NR_TYPE == AEC_NR_HW
	ADO_HwAecNr_I2C1_Init();
#endif
	UI_ANRSetting(&tUI_CamStsInfo.tCamAnrMode);
	UI_AECSetting(&tUI_CamStsInfo.tCamAecMode);
	UI_VoiceTrigSetting(&tUI_CamStsInfo.tCamScanMode);
#ifdef A7130
	if(PS_VOX_MODE == tUI_CamStsInfo.tCamPsMode)
		ubUI_SyncDisVoxFlag = TRUE;
	else
		UI_PowerSaveSetting(&tUI_CamStsInfo.tCamPsMode);
#endif
}
#define ADC_SUMRPT_VOX_THL			5000
#define ADC_SUMRPT_VOX_THH			5000
#define ADC_SUMRPT_VOICETRIG_THL	7000
#define ADC_SUMRPT_VOICETRIG_THH	7000
//------------------------------------------------------------------------------
void UI_PowerSaveSetting(void *pvPS_Mode)
{
	APP_EventMsg_t tUI_PsMessage = {0};
	UI_PowerSaveMode_t *pPS_Mode = (UI_PowerSaveMode_t *)pvPS_Mode;

	switch(pPS_Mode[0])
	{
		case PS_VOX_MODE:
			tUI_PsMessage.ubAPP_Event 	   = APP_POWERSAVE_EVENT;
			tUI_PsMessage.ubAPP_Message[0] = 2;		//! Message Length
			tUI_PsMessage.ubAPP_Message[1] = pPS_Mode[0];
			tUI_PsMessage.ubAPP_Message[2] = TRUE;
			UI_SendMessageToAPP(&tUI_PsMessage);
			if(CAMSET_ON == tUI_CamStsInfo.tCamScanMode)
			{
				ADO_SetAdcRpt(ADC_SUMRPT_VOICETRIG_THL, ADC_SUMRPT_VOICETRIG_THH, ADO_OFF);
				tUI_CamStsInfo.tCamScanMode = CAMSET_OFF;
			}
			ADO_SetAdcRpt(ADC_SUMRPT_VOX_THL, ADC_SUMRPT_VOX_THH, ADO_ON);
			tUI_CamStsInfo.tCamPsMode = PS_VOX_MODE;
			UI_UpdateDevStatusInfo();
			printd(DBG_InfoLvl, "		=> VOX Mode Enable\n");
			break;
		case PS_ECO_MODE:
			tUI_PsMessage.ubAPP_Event 	   = APP_POWERSAVE_EVENT;
			tUI_PsMessage.ubAPP_Message[0] = 2;		//! Message Length
			tUI_PsMessage.ubAPP_Message[1] = pPS_Mode[0];
			tUI_PsMessage.ubAPP_Message[2] = TRUE;
			UI_SendMessageToAPP(&tUI_PsMessage);
			tUI_CamStsInfo.tCamPsMode = PS_ECO_MODE;
			break;
		case PS_WOR_MODE:
			ADO_SetAdcRpt(ADC_SUMRPT_VOICETRIG_THL, ADC_SUMRPT_VOICETRIG_THH, ADO_ON);
			ubUI_WorWakeUpCnt = 0;
			ubUI_WorModeEnFlag = FALSE;
			if(PS_WOR_MODE == tUI_CamStsInfo.tCamPsMode)
				break;
			tUI_CamStsInfo.tCamPsMode   = PS_WOR_MODE;
			tUI_CamStsInfo.tCamScanMode = CAMSET_OFF;
			UI_UpdateDevStatusInfo();
			printd(DBG_InfoLvl, "		=> WOR Mode\n");
			break;
		case POWER_NORMAL_MODE:
			if(PS_VOX_MODE == tUI_CamStsInfo.tCamPsMode)
				UI_DisableVox();
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_ChangePsModeToWorMode(void)
{
	APP_EventMsg_t tUI_PsMessage = {0};

	tUI_PsMessage.ubAPP_Event 	   = APP_POWERSAVE_EVENT;
	tUI_PsMessage.ubAPP_Message[0] = 4;		//! Message Length
	tUI_PsMessage.ubAPP_Message[1] = PS_WOR_MODE;
	tUI_PsMessage.ubAPP_Message[2] = FALSE;
	tUI_PsMessage.ubAPP_Message[3] = FALSE;
	tUI_PsMessage.ubAPP_Message[4] = TRUE;
	UI_SendMessageToAPP(&tUI_PsMessage);
	if(FALSE == ubUI_WorModeEnFlag)
		ubUI_WorModeEnFlag = TRUE;
}
//------------------------------------------------------------------------------
void UI_ChangePsModeToNormalMode(void)
{
	APP_EventMsg_t tUI_PsMessage = {0};

	tUI_PsMessage.ubAPP_Event 	   = APP_POWERSAVE_EVENT;
	tUI_PsMessage.ubAPP_Message[0] = 4;		//! Message Length
	tUI_PsMessage.ubAPP_Message[1] = PS_WOR_MODE;
	tUI_PsMessage.ubAPP_Message[2] = TRUE;
	tUI_PsMessage.ubAPP_Message[3] = FALSE;
	tUI_PsMessage.ubAPP_Message[4] = (!ubUI_WorWakeUpCnt)?TRUE:FALSE;
	UI_SendMessageToAPP(&tUI_PsMessage);
	ADO_SetAdcRpt(ADC_SUMRPT_VOICETRIG_THL, ADC_SUMRPT_VOICETRIG_THH, ADO_OFF);
	tUI_CamStsInfo.tCamPsMode = POWER_NORMAL_MODE;
	UI_UpdateDevStatusInfo();
	ubUI_WorModeEnFlag = FALSE;
	ubUI_WorWakeUpCnt  = 0;
	printd(DBG_InfoLvl, "		=> WOR Disable\n");
}
//------------------------------------------------------------------------------
void UI_DisableVox(void)
{
	APP_EventMsg_t tUI_VoxMsg = {0};

	ADO_SetAdcRpt(ADC_SUMRPT_VOX_THL, ADC_SUMRPT_VOX_THH, ADO_OFF);
	tUI_VoxMsg.ubAPP_Event 	    = APP_POWERSAVE_EVENT;
	tUI_VoxMsg.ubAPP_Message[0] = 2;		//! Message Length
	tUI_VoxMsg.ubAPP_Message[1] = PS_VOX_MODE;
	tUI_VoxMsg.ubAPP_Message[2] = FALSE;
	UI_SendMessageToAPP(&tUI_VoxMsg);
	tUI_CamStsInfo.tCamPsMode = POWER_NORMAL_MODE;
	UI_UpdateDevStatusInfo();
	printd(DBG_InfoLvl, "		=> VOX Mode Disable\n");
}
//------------------------------------------------------------------------------
void UI_VoxTrigger(void)
{
	UI_CamReqCmd_t tUI_VoxReqCmd;
	uint32_t ulUI_AdcRpt = 0;

	tUI_VoxReqCmd.ubCmd[UI_TWC_TYPE]	= UI_REPORT;
	tUI_VoxReqCmd.ubCmd[UI_REPORT_ITEM] = UI_VOX_TRIG;
	tUI_VoxReqCmd.ubCmd[UI_REPORT_DATA] = FALSE;
	tUI_VoxReqCmd.ubCmd_Len  			= 3;
	if(TRUE == ubUI_SyncDisVoxFlag)
	{
		UI_SendRequestToCU(NULL, &tUI_VoxReqCmd);
		tUI_CamStsInfo.tCamPsMode = POWER_NORMAL_MODE;
		UI_UpdateDevStatusInfo();
		ubUI_SyncDisVoxFlag = FALSE;
		return;
	}
	ulUI_AdcRpt = ulADO_GetAdcSumHigh();
	if(ulUI_AdcRpt > ADC_SUMRPT_VOX_THH)
	{
		UI_SendRequestToCU(NULL, &tUI_VoxReqCmd);
		UI_DisableVox();
	}
}
//------------------------------------------------------------------------------
void UI_VoiceTrigSetting(void *pvTrigMode)
{
	UI_CamsSetMode_t *pVoiceTrigMode = (UI_CamsSetMode_t *)pvTrigMode;

	tUI_CamStsInfo.tCamScanMode = *pVoiceTrigMode;
	ADO_SetAdcRpt(ADC_SUMRPT_VOICETRIG_THL, ADC_SUMRPT_VOICETRIG_THH, (CAMSET_ON == tUI_CamStsInfo.tCamScanMode)?ADO_ON:ADO_OFF);
}
//------------------------------------------------------------------------------
void UI_VoiceTrigger(void)
{
	UI_CamReqCmd_t tUI_VoiceReqCmd;
	uint32_t ulUI_AdcRpt = 0;

	ulUI_AdcRpt = ulADO_GetAdcSumHigh();
	if(PS_WOR_MODE == tUI_CamStsInfo.tCamPsMode)
	{
		if(ulUI_AdcRpt > ADC_SUMRPT_VOICETRIG_THH)
		{
			APP_EventMsg_t tUI_PsMessage = {0};

			tUI_PsMessage.ubAPP_Event 	   = APP_POWERSAVE_EVENT;
			tUI_PsMessage.ubAPP_Message[0] = 4;		//! Message Length
			tUI_PsMessage.ubAPP_Message[1] = PS_WOR_MODE;
			tUI_PsMessage.ubAPP_Message[2] = TRUE;
			tUI_PsMessage.ubAPP_Message[3] = TRUE;
			tUI_PsMessage.ubAPP_Message[4] = TRUE;
			UI_SendMessageToAPP(&tUI_PsMessage);
			ADO_SetAdcRpt(ADC_SUMRPT_VOICETRIG_THL, ADC_SUMRPT_VOICETRIG_THH, ADO_OFF);
			ubUI_WorWakeUpCnt++;
			printd(DBG_InfoLvl, "		=> Voice Trigger\n");
		}
	}
	else if((PS_VOX_MODE == tUI_CamStsInfo.tCamPsMode) ||
		    (CAMSET_ON == tUI_CamStsInfo.tCamScanMode))
	{
		if(ulUI_AdcRpt > ADC_SUMRPT_VOICETRIG_THH)
		{
			tUI_VoiceReqCmd.ubCmd[UI_TWC_TYPE]	  = UI_REPORT;
			tUI_VoiceReqCmd.ubCmd[UI_REPORT_ITEM] = UI_VOICE_TRIG;
			tUI_VoiceReqCmd.ubCmd[UI_REPORT_DATA] = TRUE;
			tUI_VoiceReqCmd.ubCmd_Len  			  = 3;
			UI_SendRequestToCU(NULL, &tUI_VoiceReqCmd);
			printd(DBG_InfoLvl, "		=> Voice Trigger\n");
		}
	}
}
//------------------------------------------------------------------------------
uint8_t ubGetAecNrCommand(UI_CamsSetMode_t AecSwitch, UI_CamsSetMode_t NrSwitch)
{
	//printf("		=> aec:%d,  nr:%d\n",AecSwitch,NrSwitch);	
	if( AecSwitch==CAMSET_ON && NrSwitch==CAMSET_OFF )
	{
		return 0x80;
	}
	else if( AecSwitch==CAMSET_OFF && NrSwitch==CAMSET_ON )
	{
		return 0x08;
	}
	else if( AecSwitch==CAMSET_ON && NrSwitch==CAMSET_ON )
	{
		return 0x88;
	}
	else if( AecSwitch==CAMSET_OFF && NrSwitch==CAMSET_OFF )
	{
		return 0x00;
	}
	return 0;
}
//------------------------------------------------------------------------------
void UI_ANRSetting(void *pvAnrMode)
{
	uint8_t *pUI_AnrMode = (uint8_t *)pvAnrMode;
	
	//! AEC and NR Setting
#if(ADO_ENC_TYPE==AUDIO32_ENC)
#if APP_ADO_AEC_NR_TYPE == AEC_NR_SW
	ADO_Noise_Process_Type((CAMSET_ON == (UI_CamsSetMode_t)pUI_AnrMode[0])?NOISE_NR:NOISE_DISABLE, AEC_NR_16kHZ);
#elif APP_ADO_AEC_NR_TYPE == AEC_NR_HW
	ADO_Noise_Process_Type(NOISE_DISABLE,AEC_NR_16kHZ);
	ADO_SetDacMute(DAC_MR_0p5DB_1SAMPLE, ADO_ON);
#endif
#elif(ADO_ENC_TYPE==HW_ALAW_ENC)
	ADO_Noise_Process_Type(NOISE_DISABLE,AEC_NR_16kHZ);
#elif(ADO_ENC_TYPE==SW_ALAW_ENC)
#if APP_ADO_AEC_NR_TYPE == AEC_NR_SW
	ADO_Noise_Process_Type((CAMSET_ON == (UI_CamsSetMode_t)pUI_AnrMode[0])?NOISE_NR:NOISE_DISABLE, AEC_NR_16kHZ);
#elif APP_ADO_AEC_NR_TYPE == AEC_NR_HW
	ADO_Noise_Process_Type(NOISE_DISABLE,AEC_NR_16kHZ);
	ADO_SetDacMute(DAC_MR_0p5DB_1SAMPLE, ADO_ON);
#endif
#elif(ADO_ENC_TYPE==SW_AAC_ENC)
#if APP_ADO_AEC_NR_TYPE == AEC_NR_SW
	ADO_Noise_Process_Type((CAMSET_ON == (UI_CamsSetMode_t)pUI_AnrMode[0])?NOISE_NR:NOISE_DISABLE, AEC_NR_16kHZ);
#elif APP_ADO_AEC_NR_TYPE == AEC_NR_HW
	ADO_Noise_Process_Type(NOISE_DISABLE,AEC_NR_16kHZ);
	ADO_SetDacMute(DAC_MR_0p5DB_1SAMPLE, ADO_ON);
#endif
#endif

	if(tUI_CamStsInfo.tCamAnrMode != (UI_CamsSetMode_t)pUI_AnrMode[0])
	{
		tUI_CamStsInfo.tCamAnrMode = (UI_CamsSetMode_t)pUI_AnrMode[0];
		UI_UpdateDevStatusInfo();
	}
	printd(DBG_InfoLvl, "		=> ANR %s\n", (CAMSET_ON == tUI_CamStsInfo.tCamAnrMode)?"ON":"OFF");

#if APP_ADO_AEC_NR_TYPE == AEC_NR_HW
	ADO_HwAecNr_Command(ubGetAecNrCommand(tUI_CamStsInfo.tCamAecMode,tUI_CamStsInfo.tCamAnrMode));
	ADO_SetDacMute(DAC_MR_0p5DB_1SAMPLE, ADO_OFF);
#endif
}
//------------------------------------------------------------------------------
void UI_AECSetting(void *pvAecMode)
{
	uint8_t *pUI_AecMode = (uint8_t *)pvAecMode;
	
#if APP_ADO_AEC_NR_TYPE == AEC_NR_HW
		ADO_Noise_Process_Type(NOISE_DISABLE,AEC_NR_16kHZ);
		ADO_SetDacMute(DAC_MR_0p5DB_1SAMPLE, ADO_ON);
#endif

	//! Setting AEC
	if(tUI_CamStsInfo.tCamAecMode != (UI_CamsSetMode_t)pUI_AecMode[0])
	{
		tUI_CamStsInfo.tCamAecMode = (UI_CamsSetMode_t)pUI_AecMode[0];
		if(tUI_CamStsInfo.tCamAecMode ==  CAMSET_ON)
		{
			ADO_Noise_Process_Type(NOISE_AEC,AEC_NR_16kHZ);
		}
		else
		{
			ADO_Noise_Process_Type(NOISE_DISABLE,AEC_NR_16kHZ);
		}
//		UI_UpdateDevStatusInfo();
	}
	printd(DBG_InfoLvl, "		=> AEC %s\n", (CAMSET_ON == tUI_CamStsInfo.tCamAecMode)?"ON":"OFF");
	
#if APP_ADO_AEC_NR_TYPE == AEC_NR_HW
	ADO_HwAecNr_Command(ubGetAecNrCommand(tUI_CamStsInfo.tCamAecMode,tUI_CamStsInfo.tCamAnrMode));
	ADO_SetDacMute(DAC_MR_0p5DB_1SAMPLE, ADO_OFF);
#endif
}
//------------------------------------------------------------------------------
void UI_ADOSetting(void *pvAdoSet)
{
	uint8_t *pUI_AdoMSet = (uint8_t *)pvAdoSet;
	uint16_t uwVal;
	printd(DBG_InfoLvl, "=> UI_ADOSetting %x %x %x %x %x %x \n", pUI_AdoMSet[0], pUI_AdoMSet[1], pUI_AdoMSet[2], pUI_AdoMSet[3], pUI_AdoMSet[4], pUI_AdoMSet[5]);
	switch (pUI_AdoMSet[0]) 
	{
		case UI_ADO_ADCSAMPLERATE:

		break;
		
		case UI_ADO_ADCVOLL:
			uwVal = ((uint16_t)(pUI_AdoMSet[2]+pUI_AdoMSet[3]*0x100)-0x1400)/128;	
			printd(DBG_InfoLvl,"=> UI_ADO_ADCVOLL %x  \n",uwVal);	
			ADO_SetAdcGain((ADO_ADCGAIN)(uwVal), 0x1, ADO_ON);
			break;
		
		case UI_ADO_ADCVOLR:
			break;
		
		case UI_ADO_ADCMUTE:
			if(pUI_AdoMSet[2])
				ADO_SetAdcMute(ADC_MS_3DB_4SAMPLE, ADO_ON );
			else
				ADO_SetAdcMute(ADC_MS_3DB_4SAMPLE, ADO_OFF);
			break;
		default:
		  break;
	}

}
//------------------------------------------------------------------------------
void UI_IspSetup(void)
{
	UI_IspSettingFuncPtr_t tUI_IspFunc[UI_IMGSETTING_MAX] = 
	{
		[UI_IMG3DNR_SETTING] 		= {ISP_NR3DSwitch, (uint8_t *)&tUI_CamStsInfo.tCam3DNRMode},
		[UI_IMGvLDC_SETTING] 		= {ISP_VLDCSwitch, (uint8_t *)&tUI_CamStsInfo.tCamvLDCMode},
		[UI_IMGWDR_SETTING] 		= {NULL, NULL},
		[UI_IMGDIS_SETTING] 		= {NULL, NULL},
		[UI_IMGCBR_SETTING] 		= {NULL, NULL},
		[UI_IMGCONDENSE_SETTING] 	= {NULL, NULL},
		[UI_FLICKER_SETTING] 		= {ISP_SetAePwrFreq, 	(uint8_t *)&tUI_CamStsInfo.tCamFlicker},
		[UI_IMGBL_SETTING] 			= {ISP_SetIQBrightness, (uint8_t *)&tUI_CamStsInfo.tCamColorParam.ubColorBL},
		[UI_IMGCONTRAST_SETTING] 	= {ISP_SetIQContrast, 	(uint8_t *)&tUI_CamStsInfo.tCamColorParam.ubColorContrast},
		[UI_IMGSATURATION_SETTING] 	= {ISP_SetIQSaturation, (uint8_t *)&tUI_CamStsInfo.tCamColorParam.ubColorSaturation},
		[UI_IMGHUE_SETTING]			= {ISP_SetIQChroma, 	(uint8_t *)&tUI_CamStsInfo.tCamColorParam.ubColorHue},
		[UI_IMGFLIP_SETTING] 		= {NULL, NULL},
		[UI_IMGMIRROR_SETTING] 		= {NULL, NULL},
	};
	uint8_t ubUI_IspItem, ubUI_IspParam;

	for(ubUI_IspItem = UI_IMG3DNR_SETTING; ubUI_IspItem < UI_IMGSETTING_MAX; ubUI_IspItem++)
	{
		if(tUI_IspFunc[ubUI_IspItem].pvImgFunc)
		{
			ubUI_IspParam = (*tUI_IspFunc[ubUI_IspItem].pImgParam) * (((UI_IMGBL_SETTING        == ubUI_IspItem) ||
																	  (UI_IMGCONTRAST_SETTING   == ubUI_IspItem) ||
														              (UI_IMGSATURATION_SETTING == ubUI_IspItem) ||
															          (UI_IMGHUE_SETTING 		== ubUI_IspItem))?2:1);
			if(UI_FLICKER_SETTING == ubUI_IspItem)
				ubUI_IspParam = ((CAMFLICKER_50HZ == ubUI_IspParam)?SENSOR_PWR_FREQ_50HZ:SENSOR_PWR_FREQ_60HZ);
			tUI_IspFunc[ubUI_IspItem].pvImgFunc(ubUI_IspParam);
		}
	}
	ISP_SetMirrorFlip(tUI_CamStsInfo.tCamImgMirror, tUI_CamStsInfo.tCamImgFlip);
}
//------------------------------------------------------------------------------
void UI_IspCtSeting(uint16_t uwZoom,int32_t slPan ,int32_t slTilt,uint8_t ubMode)
{
	uint16_t uwUiZoom = 0;
	int8_t sbUiPan = 0,sbUiTilt = 0;
	static uint16_t uwH,uwV;
	static uint16_t uwZoomHsize; 
	static uint16_t uwZoomVsize;
	static uint8_t ubUpdate = 0;

	if(ubMode == 0)
	{
		uwUiZoom = uwZoom;
		sbUiPan = slPan/3600;
		sbUiTilt = slTilt/3600;	
		printd(DBG_InfoLvl,"ISP %d %d %d ",uwUiZoom,sbUiPan,sbUiTilt);
		uwZoomHsize = ISP_WIDTH*10/(10+uwUiZoom);		//1920
		uwZoomVsize = ISP_HEIGHT *10/(10+uwUiZoom);		//1088	
		
		uwH = (ISP_WIDTH  - uwZoomHsize)/2 + (ISP_WIDTH  - uwZoomHsize)/20*sbUiPan;
		uwV = (ISP_HEIGHT - uwZoomVsize)/2 + (ISP_HEIGHT - uwZoomVsize)/20*sbUiTilt;
		ubUpdate = 1;
		printd(DBG_InfoLvl," %d %d %d %d \n",uwZoomHsize,uwZoomVsize,uwH,uwV);	
	}
	if((ubMode) && (ubUpdate))
	{
		ubUpdate = 0;
		ISP_Zoom2Area(uwZoomHsize,uwZoomVsize,uwH,uwV,uwKNL_GetVdoH(0),uwKNL_GetVdoV(0));
	}
}	
//------------------------------------------------------------------------------
void UI_ImageProcSetting(void *pvImgProc)
{
	uint8_t *pUI_ImgProc = (uint8_t *)pvImgProc;
	int16_t swVal;
	uint16_t uwVal;
	static uint16_t uwImageProcZoom = 0;
	static int32_t slImageProcPan = 0;
	static int32_t slImageProcTilt = 0;

	switch(pUI_ImgProc[0])
	{
		case UI_CT_SETTING:
			swVal = (int16_t)(pUI_ImgProc[3]+pUI_ImgProc[4]*0x100);	
			uwVal = (uint16_t)(pUI_ImgProc[3]+pUI_ImgProc[4]*0x100);
			switch(pUI_ImgProc[1])
			{	
				case 2:	//UVC_CT_AE_MODE_CONTROL		
					break;
				case 3:	//UVC_CT_AE_PRIORITY_CONTROL				
					break;
				case 4:	//UVC_CT_EXPOSURE_TIME_ABSOLUTE_CONTROL				
					break;		
				case 6:	//UVC_CT_FOCUS_ABSOLUTE_CONTROL			
					break;
				case 8:	//UVC_CT_FOCUS_AUTO_CONTROL					
					break;
				case 0x0B:	//UVC_CT_ZOOM_ABSOLUTE_CONTROL	
					uwImageProcZoom = uwVal;
					UI_IspCtSeting(uwImageProcZoom,slImageProcPan,slImageProcTilt,0);			
					break;		
				case 0x0D:	//UVC_CT_PANTILT_ABSOLUTE_CONTROL	
					slImageProcPan = 	(int32_t)(((uint32_t)pUI_ImgProc[3])<<0) + (((uint32_t)pUI_ImgProc[4])<<8) +(((uint32_t)pUI_ImgProc[5])<<16) +(((uint32_t)pUI_ImgProc[6])<<24);
					slImageProcTilt = 	(int32_t)(((uint32_t)pUI_ImgProc[7])<<0) + (((uint32_t)pUI_ImgProc[8])<<8) +(((uint32_t)pUI_ImgProc[9])<<16) +(((uint32_t)pUI_ImgProc[10])<<24);			
					UI_IspCtSeting(uwImageProcZoom,slImageProcPan,slImageProcTilt,0);	
					break;
				default:
					return;
			}
			printd(DBG_InfoLvl,"UI_CT_SETTING %x %x %x %x %x_%d \n",pUI_ImgProc[1],pUI_ImgProc[2],pUI_ImgProc[3],pUI_ImgProc[4],swVal,swVal );
			break;
		case UI_PU_SETTING:
			swVal = (int16_t)(pUI_ImgProc[3]+pUI_ImgProc[4]*0x100);	
			uwVal = (uint16_t)(pUI_ImgProc[3]+pUI_ImgProc[4]*0x100);		
			switch(pUI_ImgProc[1])
			{
				case 1:	//UVC_PU_BACKLIGHT_COMPENSATION_CONTROL					
					break;			
				case 2:	//UVC_PU_BRIGHTNESS_CONTROL
					ISP_UvcSetIQBrightness(swVal);						
					break;
				case 3:	//UVC_PU_CONTRAST_CONTROL
					ISP_UvcSetIQContrast(uwVal);					
					break;
				case 4:	//UVC_PU_GAIN_CONTROL				
					break;		
				case 5:	//UVC_PU_POWER_LINE_FREQUENCY_CONTROL
					ISP_SetAePwrFreq((pUI_ImgProc[3] == 1)?SENSOR_PWR_FREQ_50HZ:SENSOR_PWR_FREQ_60HZ);				
					break;
				case 6:	//UVC_PU_HUE_CONTROL	
					ISP_UvcSetIQChroma(swVal);				
					break;
				case 7:	//UVC_PU_SATURATION_CONTROL
					ISP_UvcSetIQSaturation(uwVal);						
					break;
				case 8:	//UVC_PU_SHARPNESS_CONTROL					
					break;
				case 9:	//UVC_PU_GAMMA_CONTROL	
					ISP_UvcSetIQGamma(uwVal);				
					break;
				case 0x0A:	//UVC_PU_WHITE_BALANCE_TEMPERATURE_CONTROL					
					break;
				case 0x0B:	//UVC_PU_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL		
					//pUI_ImgProc[3];				
					break;
				default:
					return;
			}
	
			printd(DBG_InfoLvl,"UI_PU_SETTING %x %x %x %x %x %x %x_%d \n",pUI_ImgProc[1],pUI_ImgProc[2],pUI_ImgProc[3],pUI_ImgProc[4],pUI_ImgProc[5],pUI_ImgProc[6],swVal,swVal  );
			break;
		case UI_IMG3DNR_SETTING:
			tUI_CamStsInfo.tCam3DNRMode = (UI_CamsSetMode_t)pUI_ImgProc[1];
			ISP_NR3DSwitch(tUI_CamStsInfo.tCam3DNRMode);
			break;
		case UI_IMGvLDC_SETTING:
			tUI_CamStsInfo.tCamvLDCMode = (UI_CamsSetMode_t)pUI_ImgProc[1];
			ISP_VLDCSwitch(tUI_CamStsInfo.tCamvLDCMode);
			break;
		case UI_IMGWDR_SETTING:
			break;
		case UI_IMGDIS_SETTING:
			tUI_CamStsInfo.tCamDisMode = (UI_CamsSetMode_t)pUI_ImgProc[1];
			break;
		case UI_IMGCBR_SETTING:
			tUI_CamStsInfo.tCamCbrMode = (UI_CamsSetMode_t)pUI_ImgProc[1];
			break;
		case UI_IMGCONDENSE_SETTING:
			tUI_CamStsInfo.tCamCondenseMode = (UI_CamsSetMode_t)pUI_ImgProc[1];
			break;
		case UI_FLICKER_SETTING:
			tUI_CamStsInfo.tCamFlicker = (UI_CamFlicker_t)pUI_ImgProc[1];
			ISP_SetAePwrFreq((CAMFLICKER_50HZ == tUI_CamStsInfo.tCamFlicker)?SENSOR_PWR_FREQ_50HZ:SENSOR_PWR_FREQ_60HZ);
			printd(DBG_InfoLvl, "		=> Flicker: %s\n", (CAMFLICKER_50HZ == tUI_CamStsInfo.tCamFlicker)?"50Hz":"60Hz");
			break;
		case UI_IMGBL_SETTING:
			tUI_CamStsInfo.tCamColorParam.ubColorBL = pUI_ImgProc[1];
			ISP_SetIQBrightness((tUI_CamStsInfo.tCamColorParam.ubColorBL*2));
			break;
		case UI_IMGCONTRAST_SETTING:
			tUI_CamStsInfo.tCamColorParam.ubColorContrast = pUI_ImgProc[1];
			ISP_SetIQContrast((tUI_CamStsInfo.tCamColorParam.ubColorContrast*2));
			break;
		case UI_IMGSATURATION_SETTING:
			tUI_CamStsInfo.tCamColorParam.ubColorSaturation = pUI_ImgProc[1];
			ISP_SetIQSaturation((tUI_CamStsInfo.tCamColorParam.ubColorSaturation*2));
			break;
		case UI_IMGHUE_SETTING:
			tUI_CamStsInfo.tCamColorParam.ubColorHue = pUI_ImgProc[1];
			ISP_SetIQChroma(tUI_CamStsInfo.tCamColorParam.ubColorHue*2);
			break;
		case UI_IMGFLIP_SETTING:
			tUI_CamStsInfo.tCamImgFlip = (UI_CamImgFlip_t)pUI_ImgProc[1];
			ISP_SetMirrorFlip(tUI_CamStsInfo.tCamImgMirror, tUI_CamStsInfo.tCamImgFlip);
			break;
		case UI_IMGMIRROR_SETTING:
			tUI_CamStsInfo.tCamImgMirror = (UI_CamImgMirror_t)pUI_ImgProc[1];
			ISP_SetMirrorFlip(tUI_CamStsInfo.tCamImgMirror, tUI_CamStsInfo.tCamImgFlip);
			break;
		default:
			return;
	}
	UI_UpdateDevStatusInfo();
}
//------------------------------------------------------------------------------
#define MD_TRIG_LVL	16
void UI_MDTrigger(void)
{
	uint32_t ulUI_MdTrig = 0;
    uint32_t ulUI_MdTrig_LV = 0;
    
	ulUI_MdTrig = uwMD_GetCnt(MD_REG1_CNT_01);    
	printd(DBG_InfoLvl, "=>MD Trig1: %d\n", ulUI_MdTrig);
   	//ulUI_MdTrig_LV = (((tUI_CamStsInfo.MdParam.ubMD_Param[2]+1) * (tUI_CamStsInfo.MdParam.ubMD_Param[3]+1)) < 30)? 
   	//     ((tUI_CamStsInfo.MdParam.ubMD_Param[2]+1) * (tUI_CamStsInfo.MdParam.ubMD_Param[3]+1) * MD_TRIG_LVL) : (30 * MD_TRIG_LVL);
    
	if(ulUI_MdTrig > ulUI_MdTrig_LV)
	{
		UI_CamReqCmd_t tUI_MdMsg;

		tUI_MdMsg.ubCmd[UI_TWC_TYPE]	= UI_REPORT;
		tUI_MdMsg.ubCmd[UI_REPORT_ITEM] = UI_MD_TRIG;
		tUI_MdMsg.ubCmd[UI_REPORT_DATA] = TRUE;
		tUI_MdMsg.ubCmd_Len  			= 3;
		UI_SendRequestToCU(NULL, &tUI_MdMsg);
		printd(DBG_InfoLvl, "=>MD Trig2: %d\n", ulUI_MdTrig);
	}
}
//------------------------------------------------------------------------------
void UI_MDSetting(void *pvMdParam)
{
#define MD_H_WINDOWSIZE		64
#define MD_V_WINDOWSIZE		48
	uint16_t uwMD_X          = 0;
	uint16_t uwMD_Y          = 0;
	uint16_t uwMD_BlockSIdx  = 0;
	uint16_t uwMD_BlockEIdx  = 0;
	uint16_t uwMD_BlockH     = sensor_cfg.xtSENWin.uwHSize / 30;
	uint16_t uwMD_BlockV     = sensor_cfg.xtSENWin.uwVSize / 23;
	uint16_t uwMD_H_WinNum   = sensor_cfg.xtSENWin.uwHSize / MD_H_WINDOWSIZE;
	uint16_t uwMD_StartIdx   = 0, i, j;
	uint16_t uwMD_TotalBlock = (30 * 23) / 2;	//1 block is 4 bits data, 1 byte is 2 block
	uint16_t uwMD_BlockNum1  = 0, uwMD_BlockNum2 = 0;
	uint8_t ubMD_BlockCnt    = 0;
	uint8_t *pMD_Param       = (uint8_t *)pvMdParam;
	uint8_t *pMD_BlockValue, *pMD_BlockGroup;
	static uint8_t ubUI_MdUpdateFlag = FALSE;

	if((pMD_Param[2] == 0) && (pMD_Param[3] == 0))
	{
		tUI_CamStsInfo.MdParam.ubMD_Mode = MD_OFF;
		if(TRUE == ubUI_MdUpdateFlag)
		{
			MD_Switch(tUI_CamStsInfo.MdParam.ubMD_Mode);
			UI_UpdateDevStatusInfo();
			printd(DBG_InfoLvl, "=>MD OFF\n");
		}
		else
			ubUI_MdUpdateFlag = TRUE;
		return;
	}
	printd(DBG_InfoLvl, "=>MD %d_%d_%d\n", ((pMD_Param[1] << 8) | pMD_Param[0]),  pMD_Param[2],  pMD_Param[3]);
	ubMD_BlockCnt  = pMD_Param[3] + 1;
	pMD_BlockValue = malloc(uwMD_TotalBlock);
	pMD_BlockGroup = malloc(uwMD_TotalBlock);
	for(i = 0; i < uwMD_TotalBlock; i++)
	{
		pMD_BlockValue[i] = MD_REG1_CNT_00;
		pMD_BlockGroup[i] = 0x88;
	}
	uwMD_StartIdx  = ((pMD_Param[1] << 8) | pMD_Param[0]);
	uwMD_X = (uwMD_StartIdx % uwMD_H_WinNum) * MD_H_WINDOWSIZE;
	uwMD_Y = (uwMD_StartIdx / uwMD_H_WinNum) * MD_V_WINDOWSIZE;
	uwMD_BlockSIdx = ((uwMD_X / uwMD_BlockH) + ((uwMD_Y / uwMD_BlockV) * 30));
	printd(DBG_InfoLvl, "=>MD x=%d,y=%d,S=%d\n",uwMD_X,uwMD_Y,uwMD_BlockSIdx);
	if(!((uwMD_StartIdx + pMD_Param[2] + 1) % uwMD_H_WinNum))
	{
		uwMD_Y = ((uwMD_StartIdx + pMD_Param[2]) / uwMD_H_WinNum) * MD_V_WINDOWSIZE;
		uwMD_BlockEIdx = (((uwMD_Y / uwMD_BlockV) + 1) * 30) - 1;
		printd(DBG_InfoLvl, "1=>MD y=%d,e=%d\n",uwMD_Y,uwMD_BlockEIdx);
	}
	else
	{
		uwMD_X = ((uwMD_StartIdx + pMD_Param[2]) % uwMD_H_WinNum) * MD_H_WINDOWSIZE;
		uwMD_Y = ((uwMD_StartIdx + pMD_Param[2]) / uwMD_H_WinNum) * MD_V_WINDOWSIZE;
		uwMD_BlockEIdx = ((uwMD_X / uwMD_BlockH) + ((uwMD_Y / uwMD_BlockV) * 30));
		printd(DBG_InfoLvl, "2=>MD x=%d,y=%d,e=%d\n",uwMD_X,uwMD_Y,uwMD_BlockEIdx);
	}
	ubMD_BlockCnt  = ((((ubMD_BlockCnt * MD_V_WINDOWSIZE) / uwMD_BlockV) + 1) > 23) ? 23 : (((ubMD_BlockCnt * MD_V_WINDOWSIZE) / uwMD_BlockV) + 1);
	printd(DBG_InfoLvl, "3=>MD c=%d\n",ubMD_BlockCnt);

	MD_Init();
    MD_SetUserThreshold(MD_TRIG_LVL);
	MD_Switch(MD_OFF);
	for(i = uwMD_BlockSIdx; i <= uwMD_BlockEIdx; i++)
	{
		for(j = 0; j < ubMD_BlockCnt; j++)
		{
			uwMD_BlockNum1 = (i + (j * 30)) / 2;
			uwMD_BlockNum2 = (i + (j * 30)) % 2;
			if(!uwMD_BlockNum2)
			{
				pMD_BlockValue[uwMD_BlockNum1] = (pMD_BlockValue[uwMD_BlockNum1] & 0xF0) | MD_REG1_CNT_01;
				pMD_BlockGroup[uwMD_BlockNum1] = (pMD_BlockGroup[uwMD_BlockNum1] & 0xF0) | 2;
			}
			else
			{
				pMD_BlockValue[uwMD_BlockNum1] = (pMD_BlockValue[uwMD_BlockNum1] & 0x0F) | (MD_REG1_CNT_01 << 4);
				pMD_BlockGroup[uwMD_BlockNum1] = (pMD_BlockGroup[uwMD_BlockNum1] & 0x0F) | (2 << 4);
			}
		}
	}
	MD_SetROIindex((uint32_t *)pMD_BlockValue);
	MD_SetROIweight((uint32_t *)pMD_BlockGroup);
	MD_SetSensitivity(80);
	tUI_CamStsInfo.MdParam.ubMD_Mode = MD_ON;
	MD_Switch(tUI_CamStsInfo.MdParam.ubMD_Mode);
	free(pMD_BlockValue);
	free(pMD_BlockGroup);
	if(TRUE == ubUI_MdUpdateFlag)
	{
		for(i = 0; i < 4; i++)
			tUI_CamStsInfo.MdParam.ubMD_Param[i] = pMD_Param[i];
		UI_UpdateDevStatusInfo();
	}
	else
		ubUI_MdUpdateFlag = TRUE;
}
//------------------------------------------------------------------------------
void UI_ResetDevSetting(void)
{
	uint8_t i;

	UI_CLEAR_CAMSETTINGTODEFU(tUI_CamStsInfo.tCamAnrMode,  		CAMSET_OFF);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_CamStsInfo.tCam3DNRMode, 		CAMSET_ON);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_CamStsInfo.tCamvLDCMode, 		CAMSET_ON);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_CamStsInfo.tCamAecMode,  		CAMSET_OFF);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_CamStsInfo.tCamDisMode,  		CAMSET_OFF);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_CamStsInfo.tCamFlicker,		CAMFLICKER_60HZ);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_CamStsInfo.tCamCbrMode,  		CAMSET_ON);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_CamStsInfo.tCamCondenseMode, 	CAMSET_OFF);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_CamStsInfo.tCamColorParam.ubColorBL, 		  64);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_CamStsInfo.tCamColorParam.ubColorContrast,	  64);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_CamStsInfo.tCamColorParam.ubColorSaturation, 64);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_CamStsInfo.tCamColorParam.ubColorHue, 		  64);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_CamStsInfo.MdParam.ubMD_Mode,	MD_OFF);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_CamStsInfo.tCamPsMode,		POWER_NORMAL_MODE);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_CamStsInfo.tCamImgFlip,		CAMIMGFLIP_DISABLE);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_CamStsInfo.tCamImgMirror,		CAMIMGMIRROR_DISABLE);
	for(i = 0; i < 4; i++)
		tUI_CamStsInfo.MdParam.ubMD_Param[i] = 0;
	UI_UpdateDevStatusInfo();
	UI_SystemSetup();
}
//------------------------------------------------------------------------------
void UI_LoadDevStatusInfo(void)
{
	uint32_t ulUI_SFAddr = pSF_Info->ulSize - (UI_SF_START_SECTOR * pSF_Info->ulSecSize);
	uint8_t i;

	memset(&tUI_CamStsInfo, 0xFF, sizeof(UI_CamStatus_t));
	SF_Read(ulUI_SFAddr, sizeof(UI_CamStatus_t), (uint8_t *)&tUI_CamStsInfo);
	printd(DBG_InfoLvl, "UI TAG:%s\n",tUI_CamStsInfo.cbUI_DevStsTag);
	printd(DBG_InfoLvl, "UI VER:%s\n",tUI_CamStsInfo.cbUI_FwVersion);
	if ((strncmp(tUI_CamStsInfo.cbUI_DevStsTag, SF_STA_UI_SECTOR_TAG, sizeof(tUI_CamStsInfo.cbUI_DevStsTag) - 1) == 0)
	&& (strncmp(tUI_CamStsInfo.cbUI_FwVersion, SN937XX_FW_VERSION, sizeof(tUI_CamStsInfo.cbUI_FwVersion) - 1) == 0)) {

	} else {
		printd(DBG_ErrorLvl, "TAG no match, Reset UI\n");
	}
	UI_CHK_CAMSFUNCTS(tUI_CamStsInfo.tCamAnrMode,  		CAMSET_OFF);
	UI_CHK_CAMSFUNCTS(tUI_CamStsInfo.tCam3DNRMode, 		CAMSET_ON);
	UI_CHK_CAMSFUNCTS(tUI_CamStsInfo.tCamvLDCMode, 		CAMSET_ON);
	UI_CHK_CAMSFUNCTS(tUI_CamStsInfo.tCamAecMode,  		CAMSET_OFF);
	UI_CHK_CAMSFUNCTS(tUI_CamStsInfo.tCamDisMode,  		CAMSET_OFF);
	UI_CHK_CAMFLICER(tUI_CamStsInfo.tCamFlicker);
	UI_CHK_CAMSFUNCTS(tUI_CamStsInfo.tCamCbrMode,  		CAMSET_ON);
	UI_CHK_CAMSFUNCTS(tUI_CamStsInfo.tCamCondenseMode, 	CAMSET_OFF);
	UI_CHK_CAMPARAM(tUI_CamStsInfo.tCamColorParam.ubColorBL, 		64);
	UI_CHK_CAMPARAM(tUI_CamStsInfo.tCamColorParam.ubColorContrast, 	64);
	UI_CHK_CAMPARAM(tUI_CamStsInfo.tCamColorParam.ubColorSaturation, 64);
	UI_CHK_CAMPARAM(tUI_CamStsInfo.tCamColorParam.ubColorHue, 		64);
	UI_CHK_MDMODE(tUI_CamStsInfo.MdParam.ubMD_Mode,		MD_OFF);
	UI_CHK_PSMODE(tUI_CamStsInfo.tCamPsMode,			POWER_NORMAL_MODE);
	UI_CHK_CAMIMGSET(tUI_CamStsInfo.tCamImgFlip, CAMIMGFLIP_ENABLE, CAMIMGFLIP_DISABLE);
	UI_CHK_CAMIMGSET(tUI_CamStsInfo.tCamImgMirror, CAMIMGMIRROR_ENABLE, CAMIMGMIRROR_DISABLE);
	for(i = 0; i < 4; i++)
	{
		if(MD_OFF == tUI_CamStsInfo.MdParam.ubMD_Mode)
			tUI_CamStsInfo.MdParam.ubMD_Param[i] = 0;
		else
			UI_CHK_CAMPARAM(tUI_CamStsInfo.MdParam.ubMD_Param[i], 0);
	}
	ADO_SetDacR2RVol(tUI_VOLTable[VOL_LVL4]);
}
//------------------------------------------------------------------------------
void UI_UpdateDevStatusInfo(void)
{
	uint32_t ulUI_SFAddr = pSF_Info->ulSize - (UI_SF_START_SECTOR * pSF_Info->ulSecSize);

	memcpy(tUI_CamStsInfo.cbUI_DevStsTag, SF_STA_UI_SECTOR_TAG, sizeof(tUI_CamStsInfo.cbUI_DevStsTag) - 1);
	memcpy(tUI_CamStsInfo.cbUI_FwVersion, SN937XX_FW_VERSION, sizeof(tUI_CamStsInfo.cbUI_FwVersion) - 1);
	SF_DisableWrProtect();
	SF_Erase(SF_SE, ulUI_SFAddr, pSF_Info->ulSecSize, 1);
	SF_Write(ulUI_SFAddr, sizeof(UI_CamStatus_t), (uint8_t *)&tUI_CamStsInfo);
	SF_EnableWrProtect();
}
//------------------------------------------------------------------------------
//! Image Date stamp example
typedef enum
{
	UI_DS_R1_UA 	= 1,
	UI_DS_R1_LA 	= 27,
	UI_DS_R1_NUM 	= 53,
	UI_DS_R1_SYMBOL	= 63,
	UI_DS_R2_UA 	= 66,
	UI_DS_R2_LA 	= 92,
	UI_DS_R2_NUM 	= 118,
	UI_DS_R2_SYMBOL	= 128,
	UI_DS_SPACE		= 0xFFFF,
}UI_DsImgIdx_t;
void UI_ImageDateStampFunc(KNL_ImgCtrlDsInfo_t tDsInfo)
{
	static uint16_t uwUI_ImgDsH = 0, uwUI_ImgDsV = 0;
	static uint16_t uwUI_DsPosX = 0, uwUI_DsPosY = 0, uwUI_DbgDsPosY = 0;
	static uint8_t ubUI_EnDbgDsLine = FALSE;
	IMG_DS_IMG_INFO	tDsImgInfo[64];
	uint16_t uwDsH = IMG_DS1_HD_Width, uwDsV = IMG_DS3_Height;
#if APP_TIMESTAMP_FUNC_ENABLE
#if (BSP_RTC_TIMER_SEL == RTC_TIMER_INTERNAL)
	RTC_Calendar_t tSysCalendar;
#elif (BSP_RTC_TIMER_SEL == RTC_TIMER_EXTERNAL)
	RTC_PCF85063A_Calendar_t tSysCalendar;
#endif
#endif
	uint16_t uwDsImgPool[2][4] =
								{
									{UI_DS_R1_UA, UI_DS_R1_LA, UI_DS_R1_NUM, UI_DS_R1_SYMBOL},
									{UI_DS_R2_UA, UI_DS_R2_LA, UI_DS_R2_NUM, UI_DS_R2_SYMBOL},
								};
	uint16_t uwImgCtrl_DsIdx[64], uwXStart = 0;
	uint32_t ulRfBwRpt = 0;
	uint8_t ubDsCnt = 0, ubIdx = 0;
	uint8_t ubLine2Idx = 0xFF;

	if(FALSE == tDsInfo.ubDsEn)
	{
		IMG_DSDisable(DS_1);
		uwUI_DsPosX = 0;
		uwUI_DsPosY = 0;
		return;
	}
	if(tDsInfo.uwImg_HSize == FHD_WIDTH)
	{
		uwDsH = IMG_DS1_FHD_Width;
		uwDsV = IMG_DS3_Height;
	}
	else if(tDsInfo.uwImg_HSize == HD_WIDTH)
	{
		uwDsH = IMG_DS1_HD_Width;
		uwDsV = IMG_DS3_Height;
	}
	else if(tDsInfo.uwImg_HSize == VGA_WIDTH)
	{
		uwDsH = IMG_DS2_Width;
		uwDsV = IMG_DS3_Height;
	}
	if((uwUI_ImgDsH != uwDsH) || (uwUI_ImgDsV != uwDsV))
	{
		uwUI_ImgDsH = uwDsH;
		uwUI_ImgDsV = uwDsV;
		IMG_DSInit(DS_1, DS_WT_8DIV8, SINGLE, tDsInfo.uwImg_HSize, tDsInfo.uwImg_VSize, 0, 0, uwUI_ImgDsH, uwUI_ImgDsV, tDsInfo.ulDs_BufAddr);
	}
	if((uwUI_DsPosX != tDsInfo.uwDs_PosX) || (uwUI_DsPosY != tDsInfo.uwDs_PosY))
	{
		IMG_DSEraserBuf(DS_1, uwUI_DsPosX, uwUI_DsPosY, uwUI_ImgDsH, uwUI_ImgDsV);
		uwUI_DsPosX = tDsInfo.uwDs_PosX;
		uwUI_DsPosY = tDsInfo.uwDs_PosY;
	}
	IMG_DS_Setup(DS_1, 0, tDsInfo.uwDs_PosY, uwUI_ImgDsH, uwUI_ImgDsV);

#if APP_TIMESTAMP_FUNC_ENABLE
#if (BSP_RTC_TIMER_SEL == RTC_TIMER_INTERNAL)
	RTC_GetCalendar(&tSysCalendar);
#elif (BSP_RTC_TIMER_SEL == RTC_TIMER_EXTERNAL)
	RTC_PCF85063A_GetCalendar(&tSysCalendar);
#endif
#if (BSP_RTC_TIMER_SEL != RTC_TIMER_NULL)	
	uwImgCtrl_DsIdx[ubIdx++]	= tSysCalendar.uwYear  / 1000 + uwDsImgPool[0][2];
	uwImgCtrl_DsIdx[ubIdx++] 	= (tSysCalendar.uwYear / 100) % 10 + uwDsImgPool[0][2];
	uwImgCtrl_DsIdx[ubIdx++] 	= (tSysCalendar.uwYear / 10) % 10 + uwDsImgPool[0][2];
	uwImgCtrl_DsIdx[ubIdx++] 	= (tSysCalendar.uwYear) % 10 + uwDsImgPool[0][2];
	uwImgCtrl_DsIdx[ubIdx++] 	= uwDsImgPool[0][3] + 2;
	uwImgCtrl_DsIdx[ubIdx++] 	= (tSysCalendar.ubMonth / 10) % 10 + uwDsImgPool[0][2];
	uwImgCtrl_DsIdx[ubIdx++] 	= (tSysCalendar.ubMonth) % 10 + uwDsImgPool[0][2];
	uwImgCtrl_DsIdx[ubIdx++] 	= uwDsImgPool[0][3] + 2;
	uwImgCtrl_DsIdx[ubIdx++] 	= (tSysCalendar.ubDate / 10) % 10 + uwDsImgPool[0][2];
	uwImgCtrl_DsIdx[ubIdx++] 	= (tSysCalendar.ubDate) % 10 + uwDsImgPool[0][2];
	uwImgCtrl_DsIdx[ubIdx++] 	= UI_DS_SPACE;
	uwImgCtrl_DsIdx[ubIdx++] 	= (tSysCalendar.ubHour / 10) % 10 + uwDsImgPool[0][2];
	uwImgCtrl_DsIdx[ubIdx++] 	= (tSysCalendar.ubHour) % 10 + uwDsImgPool[0][2];
	uwImgCtrl_DsIdx[ubIdx++] 	= uwDsImgPool[0][3];
	uwImgCtrl_DsIdx[ubIdx++] 	= (tSysCalendar.ubMin / 10) % 10 + uwDsImgPool[0][2];
	uwImgCtrl_DsIdx[ubIdx++] 	= (tSysCalendar.ubMin) % 10 + uwDsImgPool[0][2];
	uwImgCtrl_DsIdx[ubIdx++] 	= uwDsImgPool[0][3];
	uwImgCtrl_DsIdx[ubIdx++] 	= (tSysCalendar.ubSec / 10) % 10 + uwDsImgPool[0][2];
	uwImgCtrl_DsIdx[ubIdx++] 	= (tSysCalendar.ubSec) % 10 + uwDsImgPool[0][2];
#endif	
#endif
	if(TRUE == ubUI_PerDbgMode)
	{
		uint8_t ubLinkSts;

	#ifdef A7130
		ubLinkSts = ubKNL_GetRtCommLinkStatus(KNL_MASTER_AP);
		ulRfBwRpt = ulBB_GetBBFlow(BB_GET_TXMAP_VOD_FLOW) / 1024;
	#endif
	#ifdef RTC676x
		ubLinkSts = ubKNL_GetRtCommLinkStatus(0);
		ulRfBwRpt = ulRTC676x_GetRtBw(0) / 1024;
	#endif
		ubUI_EnDbgDsLine = TRUE;
		ubLine2Idx 		 = ubIdx;
		uwImgCtrl_DsIdx[ubIdx++] = uwDsImgPool[0][0] + 11;	//! L
		uwImgCtrl_DsIdx[ubIdx++] = uwDsImgPool[0][1] + 8;	//! i
		uwImgCtrl_DsIdx[ubIdx++] = uwDsImgPool[0][1] + 13;	//! n
		uwImgCtrl_DsIdx[ubIdx++] = uwDsImgPool[0][1] + 10;	//! k
		uwImgCtrl_DsIdx[ubIdx++] = uwDsImgPool[0][3];
		uwImgCtrl_DsIdx[ubIdx++] = (ubLinkSts / 1) % 10 + uwDsImgPool[0][2];
		uwImgCtrl_DsIdx[ubIdx++] = UI_DS_SPACE;
		uwImgCtrl_DsIdx[ubIdx++] = uwDsImgPool[0][0] + 1;	//! B
		uwImgCtrl_DsIdx[ubIdx++] = uwDsImgPool[0][0] + 22;	//! W
		uwImgCtrl_DsIdx[ubIdx++] = uwDsImgPool[0][3];
		uwImgCtrl_DsIdx[ubIdx++] = ((ulRfBwRpt / 100) % 10 == 0)?UI_DS_SPACE:((ulRfBwRpt / 100) % 10 + uwDsImgPool[0][2]);
		uwImgCtrl_DsIdx[ubIdx++] = (ulRfBwRpt / 10) % 10 + uwDsImgPool[0][2];
		uwImgCtrl_DsIdx[ubIdx++] = (ulRfBwRpt / 1) % 10 + uwDsImgPool[0][2];
		uwImgCtrl_DsIdx[ubIdx++] = uwDsImgPool[0][0] + 10;	//! K
		uwImgCtrl_DsIdx[ubIdx++] = uwDsImgPool[0][0] + 1;	//! B
		uwImgCtrl_DsIdx[ubIdx++] = UI_DS_SPACE;
        uwImgCtrl_DsIdx[ubIdx++] = (tDsInfo.ubDs[0] / 10) % 10 + uwDsImgPool[0][2];
        uwImgCtrl_DsIdx[ubIdx++] = (tDsInfo.ubDs[0] / 1) % 10 + uwDsImgPool[0][2];
	}
	else
	{
		if(TRUE == ubUI_EnDbgDsLine)
		{
			IMG_DSEraserBuf(DS_1, uwUI_DsPosX, uwUI_DbgDsPosY, uwUI_ImgDsH, uwUI_ImgDsV);
			ubUI_EnDbgDsLine = FALSE;
		}
	}
	ubDsCnt 	   = ubIdx;
	uwXStart 	   = tDsInfo.uwDs_PosX;
	uwUI_DbgDsPosY = 0;
	for(ubIdx = 0; ubIdx < ubDsCnt; ubIdx++)
	{
		if(UI_DS_SPACE == uwImgCtrl_DsIdx[ubIdx])
		{
			uwXStart += 20;
			continue;
		}
		tIMG_DS_GetOsdImgInfor(DS_1, 1, DS_IMG1, uwImgCtrl_DsIdx[ubIdx], 1, &tDsImgInfo[ubIdx]);
		if(!uwUI_DbgDsPosY)
			uwUI_DbgDsPosY = tDsImgInfo[ubIdx].uwVSize;
		tDsImgInfo[ubIdx].uwXStart = uwXStart;
		tDsImgInfo[ubIdx].uwYStart = (TRUE == ubUI_EnDbgDsLine)?(ubLine2Idx)?(ubIdx >= ubLine2Idx)?uwUI_DbgDsPosY:0:uwUI_DbgDsPosY:0;
		tIMG_DS_Img1(DS_1, &tDsImgInfo[ubIdx], IMG_DS_QUEUE);
		uwXStart = ((TRUE == ubUI_EnDbgDsLine) && ((ubIdx + 1) == ubLine2Idx))?tDsInfo.uwDs_PosX:(tDsImgInfo[ubIdx].uwXStart + tDsImgInfo[ubIdx].uwHSize);
	}
	IMG_DSEnable(DS_1);
}
//------------------------------------------------------------------------------
void UI_PerDebugModeSetting(void *pvDbgMode)
{
	ubUI_PerDbgMode = *(uint8_t *)pvDbgMode;
	KNL_EnPerDebugMode(ubUI_PerDbgMode, NULL);
}
#if (APP_DOORPHONE_ENABLE==1)
void UI_DPPairingKey1(void)
{
	APP_EventMsg_t tUI_PairMessage = {0};
	tUI_PairMessage.ubAPP_Event = (APP_PAIRING_STATE == tUI_SyncAppState)?APP_PAIRING_STOP_EVENT:APP_PAIRING_START_EVENT;
    tUI_PairMessage.ubAPP_Message[0] = 0;       // group id Rx1
	UI_SendMessageToAPP(&tUI_PairMessage);
	BUZ_PlaySingleSound();
}

void UI_DPPairingKey2(void)
{
	APP_EventMsg_t tUI_PairMessage = {0};
	tUI_PairMessage.ubAPP_Event = (APP_PAIRING_STATE == tUI_SyncAppState)?APP_PAIRING_STOP_EVENT:APP_PAIRING_START_EVENT;
    tUI_PairMessage.ubAPP_Message[0] = 1;       // group id Rx2
	UI_SendMessageToAPP(&tUI_PairMessage);
	BUZ_PlaySingleSound();
}
void UI_DPKey1(void)
{
	APP_EventMsg_t tUI_CallingMessage = {0};
    tUI_CallingMessage.ubAPP_Event = (APP_PAIRING_STATE == tUI_SyncAppState)?APP_PAIRING_STOP_EVENT:APP_DP_KEY_EVENT;
    tUI_CallingMessage.ubAPP_Message[0] = 0;    // group id Rx1
	UI_SendMessageToAPP(&tUI_CallingMessage);
}
void UI_DPKey2(void)
{
	APP_EventMsg_t tUI_CallingMessage = {0};
    tUI_CallingMessage.ubAPP_Event = (APP_PAIRING_STATE == tUI_SyncAppState)?APP_PAIRING_STOP_EVENT:APP_DP_KEY_EVENT;
    tUI_CallingMessage.ubAPP_Message[0] = 1;    // group id Rx2
	UI_SendMessageToAPP(&tUI_CallingMessage);
}
#endif

