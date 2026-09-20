/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		UI_BUCCU.c
	\brief		User Interface of AHD Rx (for High Speed Mode)
	\author		Hanyi Chiu
	\version	2.0
	\date		2022/03/08
	\copyright	Copyright (C) 2022 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#include "BSP.h"

#if defined(BSP_D_SNCC71_TP2915_AHD_RX_V3) ||\
	defined(BSP_D_SNCC71_NVP6021_AHD_RX_V4)

#include <string.h>
#include <math.h>
#include "UI_BUCCU[AHD].h"
#include "SF_API.h"
#include "EN_API.h"
#include "FWU_API.h"
#include "TIMER.h"
#include "VDO.h"
#include "Buzzer.h"
#include "PLY_API.h"
#include "REC_API.h"
#include "ADO.h"

#define osUI_SIGNALS	0x66

/**
 * Key event mapping table
 *
 * @param ubKeyID  			Key ID
 * @param ubKeyCnt 			Key count	(100ms per 1 count, ex.long press 5s, the count set to 50)
 * @param KeyEventFuncPtr 	Key event mapping to function
 */
const UI_KeyEventMap_t UiKeyEventMap[] =
{
#if defined(BSP_D_SNCC71_TP2915_AHD_RX_V3)
	{NULL,				0,			NULL,						NULL},
	{AKEY_MENU, 		0,			UI_Key1, 					BUZ_PlaySingleSound},
	{AKEY_MENU, 		20,			UI_PairKey, 				BUZ_PlaySingleSound},
	{AKEY_LEFT, 		0,			UI_Key2,					BUZ_PlaySingleSound},
	{AKEY_LEFT, 		20,			UI_DispPacket,				BUZ_PlaySingleSound},
	{AKEY_UP, 			0,			UI_Key3,					BUZ_PlaySingleSound},
	{AKEY_UP, 			20,			UI_DispRes,					BUZ_PlaySingleSound},
	{AKEY_DOWN, 		0,			UI_Key4,					BUZ_PlaySingleSound},
	{AKEY_DOWN, 		20,			UI_Date,					BUZ_PlaySingleSound},
#elif defined(BSP_D_SNCC71_NVP6021_AHD_RX_V4)
	{NULL,				0,			NULL,						NULL},
	{AKEY_MENU, 		0,			UI_PairKey, 				BUZ_PlaySingleSound},
	{AKEY_MENU, 		20,			UI_Key1, 					BUZ_PlaySingleSound},
	{AKEY_LEFT, 		0,			UI_Key2,					BUZ_PlaySingleSound},
	{AKEY_LEFT, 		20,			UI_DispPacket,				BUZ_PlaySingleSound},
	{AKEY_UP, 			0,			UI_Key3,					BUZ_PlaySingleSound},
	{AKEY_UP, 			20,			UI_DispRes,					BUZ_PlaySingleSound},
	{AKEY_DOWN, 		0,			UI_Key4,					BUZ_PlaySingleSound},
	{AKEY_DOWN, 		20,			UI_Date,					BUZ_PlaySingleSound},
#endif
};
const UI_ReportFuncPtr_t tUiReportMap2Func[] =
{
	[UI_UPDATE_CAMSTS] 			= UI_UpdateCamStatus,
	[UI_VOX_TRIG]				= NULL,
	[UI_MD_TRIG]				= NULL,
	[UI_VOICE_TRIG]				= UI_VoiceTrigger,
};
static UI_State_t tUI_State;
static UI_ChangeDispState_t tUI_ChangeDispState;	
static UI_CamAHDtype_t 		tUI_CamAhdType[CAM_4T];
static uint8_t			   ubUI_Signal[CAM_4T];
static APP_State_t tUI_SyncAppState;
static UI_CamStatus_t tUI_CamStatus[CAM_4T];
static UI_CUSetting_t tUI_CuSetting;
			  
osSemaphoreId osUI_CuSemId;						  
static UI_CamViewSelect_t tCamViewSel;
static UI_PairingInfo_t tPairInfo;
static UI_ThreadNotify_t tosUI_Notify;
static uint32_t ulUI_LogoIndex;
static uint8_t ubUI_CuStartUpFlag;
static uint8_t ubUI_ResetPeriodFlag;
static uint8_t ubUI_ScanStartFlag;

//! Functions Execute
static void UI_SignalThread(void const *argument);
osThreadId osUI_SignalThdId;
osMutexId osUI_OsdMutex;
#define UI_HVIEW_ICON_XOFFSET		756
//------------------------------------------------------------------------------
static UI_AHDosdStatus_t tUI_AHDosdStatus;
//------------------------------------------------------------------------------
void UI_KeyEventExec(void *pvKeyEvent)
{
	static uint8_t ubUI_KeyEventIdx = 0;
	KEY_Event_t *ptKeyEvent;
	uint16_t uwUiKeyEvent_Cnt = 0, uwIdx;

	ptKeyEvent = (KEY_Event_t *)pvKeyEvent;
	uwUiKeyEvent_Cnt = sizeof UiKeyEventMap / sizeof(UI_KeyEventMap_t);
	if(ptKeyEvent->ubKeyAction == KEY_UP_ACT)
	{
		if((UI_DPTZ_CONTROL_STATE == tUI_State) &&
		   ((ptKeyEvent->ubKeyID == AKEY_UP)   || (ptKeyEvent->ubKeyID == AKEY_DOWN) ||
		    (ptKeyEvent->ubKeyID == AKEY_LEFT) || (ptKeyEvent->ubKeyID == AKEY_RIGHT)))
		{
			UI_DPTZ_KeyRelease(ptKeyEvent->ubKeyID);
			return;
		}
		if(((ubUI_KeyEventIdx) && (ubUI_KeyEventIdx < uwUiKeyEvent_Cnt)) ||
		   (ptKeyEvent->ubKeyID == AKEY_PTT))
		{
			if(ptKeyEvent->ubKeyID == AKEY_PTT)
				;
			if(UiKeyEventMap[ubUI_KeyEventIdx].KeyEventFuncPtr)
			{
				UiKeyEventMap[ubUI_KeyEventIdx].KeyEventFuncPtr();
				if(UiKeyEventMap[ubUI_KeyEventIdx].pvKeyTone)
					UiKeyEventMap[ubUI_KeyEventIdx].pvKeyTone();
			}
		}
		ubUI_KeyEventIdx = 0;
		return;
	}
	for(uwIdx = 1; uwIdx < uwUiKeyEvent_Cnt; uwIdx++)
	{
		if((UI_DPTZ_CONTROL_STATE == tUI_State) &&
		   ((ptKeyEvent->ubKeyID == AKEY_UP)   || (ptKeyEvent->ubKeyID == AKEY_DOWN) ||
		    (ptKeyEvent->ubKeyID == AKEY_LEFT) || (ptKeyEvent->ubKeyID == AKEY_RIGHT)))
		{
			if(UI_DPTZ_KeyPress(ptKeyEvent->ubKeyID, uwIdx) == rUI_FAIL)
				continue;
			if(UiKeyEventMap[uwIdx].KeyEventFuncPtr)
			{
				UiKeyEventMap[uwIdx].KeyEventFuncPtr();
				if(UiKeyEventMap[ubUI_KeyEventIdx].pvKeyTone)
					UiKeyEventMap[ubUI_KeyEventIdx].pvKeyTone();
			}
			ubUI_KeyEventIdx = 0;
			break;
		}
		if((ptKeyEvent->ubKeyID  == UiKeyEventMap[uwIdx].ubKeyID) &&
		   (ptKeyEvent->uwKeyCnt == UiKeyEventMap[uwIdx].uwKeyCnt))
		{
			ubUI_KeyEventIdx = uwIdx;
			if(((ptKeyEvent->uwKeyCnt) && (UiKeyEventMap[ubUI_KeyEventIdx].KeyEventFuncPtr)) ||
			   (ptKeyEvent->ubKeyID == AKEY_PTT))
			{
				if(ptKeyEvent->ubKeyID == AKEY_PTT)
					;
				UiKeyEventMap[ubUI_KeyEventIdx].KeyEventFuncPtr();
				if(UiKeyEventMap[ubUI_KeyEventIdx].pvKeyTone)
					UiKeyEventMap[ubUI_KeyEventIdx].pvKeyTone();
				ubUI_KeyEventIdx = (ptKeyEvent->ubKeyID == AKEY_PTT)?ubUI_KeyEventIdx:0;
			}
		}
	}
}
//------------------------------------------------------------------------------
void UI_UpdateOSD (UI_UpdateOSDtype_t tMode, void *pvBuf)
{
	osMutexWait(osUI_OsdMutex, osWaitForever);
	switch (tMode)
	{
		case UI_ERASE_STATUS:
		{
			OSD_IMG_INFO tInfor, tInforANT;
			uint16_t 	uwIdxSft = OSD1IMG_ICON_MAX * tUI_AHDosdStatus.tRes;
			if (UI_REC_START == tUI_AHDosdStatus.tRec)
			{
				tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_REC + uwIdxSft, 1, &tInfor);
				if (ANT_NULL != tUI_AHDosdStatus.tAnt)
					tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_ANT5 +uwIdxSft, 1, &tInforANT);
				tInfor.uwHSize = tInforANT.uwXStart + tInforANT.uwHSize - 
								 tInfor.uwXStart;
				tUI_AHDosdStatus.tRec = UI_REC_NULL;
			}
			else if (ANT_NULL != tUI_AHDosdStatus.tAnt)
				tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_ANT5 + uwIdxSft, 1, &tInfor);
			else
				break;
			tUI_AHDosdStatus.tAnt = ANT_NULL;
			OSD_EraserImg1(&tInfor);
			break;
		}
		case UI_DRAW_ANT:
			if (FALSE == tUI_CuSetting.IconSts.ubShowLostLogoFlag)
			{
				OSD_IMG_INFO tInfor;
				UI_AntLvl_t	 tAnt;
				uint8_t   *pubBuf = (uint8_t*)pvBuf;
				
				tAnt = (pubBuf[0] > 10)?(pubBuf[0] > 20)?(pubBuf[0] > 40)?
					   (pubBuf[0] > 60)?(pubBuf[0] > 80)?ANT_SIGNALLVL5:
					   (ANT_SIGNALLVL4):ANT_SIGNALLVL3:ANT_SIGNALLVL2:ANT_SIGNALLVL1:
						ANT_NOSIGNAL;
				if (tUI_AHDosdStatus.tAnt != tAnt && 
					tUI_AHDosdStatus.tRes != UI_RES_UNKNOW)
				{
					tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_ANT5 + tAnt + 
										   OSD1IMG_ICON_MAX * tUI_AHDosdStatus.tRes, 
										1, &tInfor);
					tOSD_Img1(&tInfor, OSD_UPDATE);
					tUI_AHDosdStatus.tAnt = tAnt;
				}
			}
			break;
		case UI_ERASE_DATE:
		{
			OSD_IMG_INFO tInfor;
			uint16_t 	uwIdxSft = OSD1IMG_ICON_MAX * tUI_AHDosdStatus.tRes;
			uint16_t 	uwVsize;			
			tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_INDEX + uwIdxSft, 1, &tInfor);
			uwVsize = tInfor.uwVSize;
			tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_NUM0 + uwIdxSft, 1, &tInfor);			
			tInfor.uwVSize += uwVsize;
			tInfor.uwHSize = uwOSD_GetOsdImgHsize() - tInfor.uwXStart;
			OSD_EraserImg1(&tInfor);
			break;
		}
		case UI_DRAW_DATE:
		{
			OSD_IMG_INFO tInfor;
			uint16_t 	uwIdxSft = OSD1IMG_ICON_MAX * tUI_AHDosdStatus.tRes;
			uint16_t 	uwXstart[4];
			uint16_t 	uwHsize;
			uint8_t     ubi, ubMax = sizeof(tUI_AHDosdStatus.ubNextDate);
			tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_NUM0 + uwIdxSft, 1, &tInfor);
			uwXstart[0] = tInfor.uwXStart;
			uwHsize = tInfor.uwHSize;
			tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_NUM5 + uwIdxSft, 1, &tInfor);
			uwXstart[1] = tInfor.uwXStart;			
			for (ubi=1; ubi<ubMax; ++ubi)
			{	
				if (tUI_AHDosdStatus.ubNextDate[ubi] != tUI_AHDosdStatus.ubOsdDate[ubi])
					tOSD_GetOsdImgInfor(1, OSD_IMG1, tUI_AHDosdStatus.ubNextDate[ubi] + 
										   OSD1IMG_NUM0 + uwIdxSft, 1, &tInfor);
				if (1 == ubi)
					tInfor.uwXStart = uwXstart[2] = uwXstart[0];
				else if (9 == ubi)
					tInfor.uwXStart = uwXstart[2]  = uwXstart[1];
				else
					tInfor.uwXStart = uwXstart[2];
				if (tUI_AHDosdStatus.ubNextDate[ubi] != tUI_AHDosdStatus.ubOsdDate[ubi])
					tOSD_Img1(&tInfor, OSD_QUEUE);
				if (ubi == tUI_AHDosdStatus.ubNextDate[0])
					uwXstart[3] = uwXstart[2] + 2;
				uwXstart[2] += uwHsize;
				if (4 == ubi || 6 == ubi || 10 == ubi || 12 == ubi)
				{
					tOSD_GetOsdImgInfor(1, OSD_IMG1, 
					((4 == ubi || 6 == ubi)?OSD1IMG_SLASH:OSD1IMG_COLON) + uwIdxSft, 1, &tInfor);
					tInfor.uwXStart = uwXstart[2];
					uwXstart[2] = tInfor.uwXStart + tInfor.uwHSize;
					if (0xFF == tUI_AHDosdStatus.ubOsdDate[0])
						tOSD_Img1(&tInfor, OSD_QUEUE);
				}
			}
			if (tUI_AHDosdStatus.ubOsdDate[0] != tUI_AHDosdStatus.ubNextDate[0])
			{
				tOSD_GetOsdImgInfor(1, OSD_IMG1,OSD1IMG_INDEX + uwIdxSft, 1, &tInfor);
				uwHsize = tInfor.uwHSize;
				tInfor.uwHSize = uwOSD_GetHSize();
				OSD_EraserImg1(&tInfor);
				tInfor.uwXStart = uwXstart[3];
				tInfor.uwHSize = uwHsize;
				tOSD_Img1(&tInfor, OSD_QUEUE);
			}
			memcpy(tUI_AHDosdStatus.ubOsdDate, tUI_AHDosdStatus.ubNextDate, 
			sizeof(tUI_AHDosdStatus.ubOsdDate));
			OSD_UpdateQueueBuf();
			break;
		}
		case UI_ERASE_DISP:
		{
			OSD_IMG_INFO tInfor;
			tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_AHD + 
								   OSD1IMG_ICON_MAX * tUI_AHDosdStatus.tRes, 
								1, &tInfor);
			tInfor.uwXStart = 0;
			tInfor.uwHSize = uwOSD_GetOsdImgHsize();
			OSD_EraserImg1(&tInfor);
			tUI_AHDosdStatus.ubDispNum = 0;
			break;
		}
		case UI_DRAW_DISP:
			if (tUI_AHDosdStatus.ubDispNum)
			{
				OSD_IMG_INFO tInfor[tUI_AHDosdStatus.ubDispNum];
				uint8_t ubi;
				for	(ubi=tUI_AHDosdStatus.ubDispNum; ubi; --ubi)
				{
					tOSD_GetOsdImgInfor(1, OSD_IMG1, tUI_AHDosdStatus.uwDispIdx[ubi - 1], 
										1, &tInfor[ubi - 1]);
					
					if (tUI_AHDosdStatus.ubDispNum > ubi)
						tInfor[ubi - 1].uwXStart = tInfor[ubi].uwXStart - 
												   tInfor[ubi - 1].uwHSize;
					else
						tInfor[ubi - 1].uwXStart = uwOSD_GetOsdImgHsize() - 
												   tInfor[ubi - 1].uwHSize;
					tOSD_Img1(&tInfor[ubi - 1], OSD_QUEUE);
				
				}
				tInfor[0].uwHSize = tInfor[0].uwXStart;
				tInfor[0].uwXStart = 0;
				OSD_EraserImg1(&tInfor[0]);
			}
			break;
		case UI_CLEAR_ALL:
		{
			OSD_IMG_INFO tInfor;
			tInfor.uwXStart = 0;
			tInfor.uwYStart = 0;
			tInfor.uwHSize = uwOSD_GetOsdImgHsize();
			tInfor.uwVSize = uwOSD_GetOsdImgVsize();
			OSD_EraserImg1(&tInfor);
			tUI_AHDosdStatus.tAnt = ANT_NULL;
			tUI_AHDosdStatus.tRec = UI_REC_NULL;
			tUI_AHDosdStatus.ubDispCnt = 0;
			tUI_AHDosdStatus.ubDispNum = 0;
			break;
		}
		default:
			break;
	}
	osMutexRelease(osUI_OsdMutex);
}
//------------------------------------------------------------------------------
void UI_OnInitDialog(void)
{

	if (NULL == osUI_SignalThdId)
	{
		osThreadDef(UiSignal, UI_SignalThread, THREAD_PRIO_UI_SIGNAL_HANDLER, 1, 
											   THREAD_STACK_UI_SIGNAL_HANDLER);
		osUI_SignalThdId = osThreadCreate(osThread(UiSignal), NULL);
	}

	tCamViewSel.tCamViewType    = (VDO_DISP_TYPE == KNL_DISP_H)?H_VIEW:
								  (VDO_DISP_TYPE == KNL_DISP_QUAD)?QUAD_VIEW:
								  ((VDO_DISP_TYPE == KNL_DISP_DUAL_C) || (VDO_DISP_TYPE == KNL_DISP_DUAL_U))?DUAL_VIEW:SINGLE_VIEW;
	tCamViewSel.tCamViewPool[0] = ((VDO_DISP_TYPE == KNL_DISP_QUAD) || (VDO_DISP_TYPE == KNL_DISP_H))?CAM_4T:CAM1;
	tCamViewSel.tCamViewPool[1] = ((VDO_DISP_TYPE == KNL_DISP_DUAL_C) || (VDO_DISP_TYPE == KNL_DISP_DUAL_U))?CAM2:NO_CAM;

	OSD_LogoJpeg(ulUI_LogoIndex);
	UI_UpdateOSD(UI_DRAW_DISP,NULL);
	GPIO->GPIO_O0 	= 0;
	GPIO->GPIO_O13 	= 0;
	BUZ_PlayPowerOnSound();
}
//------------------------------------------------------------------------------
#if (APP_ADO_FUNC_ENABLE == 1)
KNL_ROLE PcConn_AdoRole;
#endif
#if APP_PC_CONNECT_EN == 1
void UI_ShowPcCnnPic(void)
{
#ifdef OP_AP
	OSD_IMG_INFO tOsdImgInfo;
	
	ubUI_StopUpdateStsBarFlag = TRUE;
	tUI_CuSetting.IconSts.ubDrawStsIconFlag = TRUE;
	UI_ClearStatusBarOsdIcon();
	UI_ClearCamConnectStatusFlag();
	OSD_Weight(OSD_WEIGHT_8DIV8);
	tOsdImgInfo.uwHSize  = uwOSD_GetHSize();
	tOsdImgInfo.uwVSize  = uwOSD_GetVSize();
	tOsdImgInfo.uwXStart = 0;
	tOsdImgInfo.uwYStart = 0;
	OSD_EraserImg1(&tOsdImgInfo);
	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_FWUSTARTBG, 1, &tOsdImgInfo);
	tOSD_Img1(&tOsdImgInfo, OSD_UPDATE);
	
	KEY_Suspend();
	UI_StopUpdateThread();
#if (APP_ADO_FUNC_ENABLE == 1)
	PcConn_AdoRole = ADO_Stop();
#endif
	VDO_Stop();
#endif
}
//------------------------------------------------------------------------------
void UI_PcConn_SdCardPlugout(void)
{
#ifdef OP_AP
	UI_ClearOsdImage();
	tUI_CuSetting.IconSts.ubDrawStsIconFlag = FALSE;
	ubUI_StopUpdateStsBarFlag = FALSE;
	if(FALSE == tUI_CuSetting.IconSts.ubShowLostLogoFlag)
		UI_DrawCUStatusIcon();
	tUI_State = UI_DISPLAY_STATE;

	VDO_Start();
#if (APP_ADO_FUNC_ENABLE == 1)
	ADO_Start(PcConn_AdoRole);
#endif
	UI_StartUpdateThread();
	KEY_Resume();
#endif
}
#endif //! #if APP_PC_CONNECT_EN == 1
//------------------------------------------------------------------------------
void UI_StateReset(void)
{
	osSemaphoreDef(UiCuSem);
	osUI_CuSemId = osSemaphoreCreate(osSemaphore(UiCuSem), 1);

	tosUI_Notify.thread_id	  	 = NULL;
	tosUI_Notify.iSignals	  	 = 0;
	ubUI_CuStartUpFlag		  	 = FALSE;
	ubUI_ResetPeriodFlag		 = FALSE;
	
	ubUI_ScanStartFlag    	  	 = FALSE;

	osUI_SignalThdId			 = NULL;

	osMutexDef(UiOsdMutex);
	osUI_OsdMutex 			 	 = osMutexCreate(osMutex(UiOsdMutex));
	tUI_State 		 		  	 = UI_DISPLAY_STATE;
	tUI_ChangeDispState			 = UI_DISP_STATE_KEEP;
	tUI_SyncAppState		  	 = APP_STATE_NULL;
	ulUI_LogoIndex			  	 = OSDLOGO_MUTE;

	if(tTWC_RegTransCbFunc(TWC_UI_SETTING, UI_RecvCamResponse, UI_RecvCamRequest) != TWC_SUCCESS)
		printd(DBG_ErrorLvl, "UI Setting 2-way command fail!\n");
	if(tTWC_RegTransCbFunc(TWC_TX_AHD_RPT, NULL, UI_RecvCamAhdType) != TWC_SUCCESS)
		printd(DBG_ErrorLvl, "Register TX AHD Report TWC Fail !\n");
	memset(tUI_CamAhdType, 0xFF, sizeof(tUI_CamAhdType));
	UI_LoadDevStatusInfo();
	tUI_AHDosdStatus.tAnt = ANT_NULL;
	tUI_AHDosdStatus.tRec = UI_REC_NULL;
#if APP_PC_CONNECT_EN
	KNL_PCCONN_INIT_t UI_PcConnInitInfo = {0};
	
	UI_PcConnInitInfo.EnterPcConnCb   = UI_ShowPcCnnPic;
	UI_PcConnInitInfo.LeavePcConnCb   = NULL;
	UI_PcConnInitInfo.OtherAction     = NULL;
	UI_PcConnInitInfo.SdCardPlugoutCb = UI_PcConn_SdCardPlugout;
	
	UI_PcConnInitInfo.SdFwuFileName.ubLen = 8;
	memcpy(&UI_PcConnInitInfo.SdFwuFileName.chName, "SNCC7XFW", UI_PcConnInitInfo.SdFwuFileName.ubLen);
	memcpy(&UI_PcConnInitInfo.SdFwuFileName.chExt, "BIN", 3);
	
	UI_PcConnInitInfo.EngModeFileName.ubLen = 2;
	memcpy(&UI_PcConnInitInfo.EngModeFileName.chName, "EM", UI_PcConnInitInfo.EngModeFileName.ubLen);
	memcpy(&UI_PcConnInitInfo.EngModeFileName.chExt, "BIN", 3);
	
	KNL_PcConnectInit(&UI_PcConnInitInfo);
#endif	
}
//------------------------------------------------------------------------------
void UI_UpdateFwUpgStatus(void *ptUpgStsReport)
{
	APP_StatusReport_t *pFWU_StsRpt = (APP_StatusReport_t *)ptUpgStsReport;

	static UI_State_t tUI_PreState;

	switch(pFWU_StsRpt->ubAPP_Report[0])
	{
		case FWU_UPG_INPROGRESS:
			tUI_PreState = tUI_State;
			tUI_State = UI_FWUPG_STATE;
			break;
		case FWU_UPG_SUCCESS:
		case FWU_UPG_FAIL:
		case FWU_UPG_DEVTAG_FAIL:		
			tUI_State = tUI_PreState;
			break;
		default:		
			if(100 == pFWU_StsRpt->ubAPP_Report[0])
				osDelay(1000);
			break;
	}
}
//------------------------------------------------------------------------------
void UI_UpdateAppStatus(void *ptAppStsReport)
{
	APP_StatusReport_t *pAppStsRpt = (APP_StatusReport_t *)ptAppStsReport;

	switch(pAppStsRpt->tAPP_ReportType)
	{
		case APP_PAIRSTS_RPT:
		{
			UI_Result_t tPair_Result  = (UI_Result_t)pAppStsRpt->ubAPP_Report[0];
			UI_CamNum_t tAppAdoSrcNum = (UI_CamNum_t)pAppStsRpt->ubAPP_Report[1];
			uint8_t ubAppPairFlag	  = pAppStsRpt->ubAPP_Report[2];

			if(TRUE == ubAppPairFlag)
			{
				tPairInfo.tPairSelCam   = (UI_CamNum_t)pAppStsRpt->ubAPP_Report[3];
				tPairInfo.tDispLocation = (UI_DisplayLocation_t)pAppStsRpt->ubAPP_Report[4];
			}
			if((rUI_SUCCESS == tPair_Result) &&
			   (tUI_CuSetting.tAdoSrcCamNum != tAppAdoSrcNum))
				pAppStsRpt->ubAPP_Report[1] = tUI_CuSetting.tAdoSrcCamNum;
			tUI_State = UI_DISPLAY_STATE;
			break;
		}
		case APP_LINKSTS_RPT:
			ubUI_Signal[0] = (pAppStsRpt->ubAPP_Report[4])?
						log10(pAppStsRpt->ubAPP_Report[4]) * 35:0;
			ubUI_Signal[1] = (pAppStsRpt->ubAPP_Report[5])?
						log10(pAppStsRpt->ubAPP_Report[5]) * 35:0;
			ubUI_Signal[2] = (pAppStsRpt->ubAPP_Report[6])?
						log10(pAppStsRpt->ubAPP_Report[6]) * 35:0;
			ubUI_Signal[3] = (pAppStsRpt->ubAPP_Report[7])?
						log10(pAppStsRpt->ubAPP_Report[7]) * 35:0;
			if (APP_LINK_STATE == tUI_SyncAppState)
				UI_UpdateOSD(UI_DRAW_ANT, &pAppStsRpt->ubAPP_Report[4]);
			break;	
		case APP_PAIRUDCAM_PRT:
		{
			UI_CamNum_t tDelCam 	= (UI_CamNum_t)pAppStsRpt->ubAPP_Report[0];
			UI_UnBindCam(tDelCam);
			break;
		}
		case APP_TRXBWSWRET_RPT:
		{
			UI_Result_t tUI_TrxBwSwRet;

			tUI_TrxBwSwRet = (rSUCCESS == pAppStsRpt->ubAPP_Report[0])?rUI_SUCCESS:rUI_FAIL;
			if(rUI_SUCCESS == tUI_TrxBwSwRet)
			{
				UI_CamsBwMode_t tBwMode;

				tBwMode = (UI_CamsBwMode_t)pAppStsRpt->ubAPP_Report[1];
				UI_SwitchViewTypeByTrxBwMode(tBwMode);
			}
			else
				printd(DBG_ErrorLvl, "  Switch BW Mode Fail !\n");		
			break;
		}
		default:
			break;
	}
	if(pAppStsRpt->tAPP_State == APP_LINK_STATE)
	{
		if(tUI_SyncAppState != pAppStsRpt->tAPP_State)
			UI_RemoveLostLinkLogo();
		ubUI_ResetPeriodFlag = TRUE;
		if((SCAN_VIEW == tCamViewSel.tCamViewType) && (FALSE == ubUI_ScanStartFlag) &&
		   (UI_RECPLAYLIST_STATE != tUI_State) && (UI_PHOTOPLAYLIST_STATE != tUI_State) && (UI_PHOTOPLAYNRDY_STATE != tUI_State))
			UI_EnableScanMode();
	}
	if(FALSE == ubUI_CuStartUpFlag)
	{
//		if(PS_VOX_MODE == tUI_CuSetting.tPsMode)
//			UI_EnableVox();
		ubUI_CuStartUpFlag = TRUE;
	}
	osSemaphoreWait(osUI_CuSemId, osWaitForever);
	tUI_CuSetting.IconSts.ubClearThdCntFlag = (tUI_SyncAppState == pAppStsRpt->tAPP_State)?FALSE:TRUE;
	tUI_SyncAppState = pAppStsRpt->tAPP_State;
	osSemaphoreRelease(osUI_CuSemId);
}
//------------------------------------------------------------------------------
void UI_UpdateStatus(uint16_t *pThreadCnt)
{
	APP_EventMsg_t tUI_GetLinkStsMsg = {0};
	uint8_t ubUI_SendMsg2AppFlag = FALSE;

	osSemaphoreWait(osUI_CuSemId, osWaitForever);
	if (tUI_AHDosdStatus.ubDispCnt)
	{
		--tUI_AHDosdStatus.ubDispCnt;
		if (!tUI_AHDosdStatus.ubDispCnt)
			UI_UpdateOSD(UI_ERASE_DISP, NULL);
	}
	UI_CLEAR_THREADCNT(tUI_CuSetting.IconSts.ubClearThdCntFlag, *pThreadCnt);
	switch(tUI_SyncAppState)
	{
		case APP_IDLE_STATE:
			ubUI_SendMsg2AppFlag = TRUE;
			POWER_LED_IO = 1;
			CAM1_LED_IO(0);
			CAM2_LED_IO(0);
			CAM3_LED_IO(0);
			CAM4_LED_IO(0);
			break;
		case APP_LOSTLINK_STATE:
			ubUI_SendMsg2AppFlag = TRUE;
			if(FALSE == tUI_CuSetting.IconSts.ubShowLostLogoFlag)
			{
				UI_ShowLostLinkLogo(pThreadCnt);
				(*pThreadCnt)++;
				goto END_UPDATESTS;
			}
			break;
		case APP_LINK_STATE:
			ubUI_SendMsg2AppFlag = TRUE;
			if(tUI_State == UI_DISPLAY_STATE)
			{			
				(*pThreadCnt)++;
				goto END_UPDATESTS;
			}
			break;
		case APP_PAIRING_STATE:
			POWER_LED_IO = ~POWER_LED_IO;
			switch (tPairInfo.tPairSelCam)
			{
				case CAM1:
					CAM1_LED_IO(!GET_CAM1_LED_IO);
					break;
				case CAM2:
					CAM2_LED_IO(!GET_CAM2_LED_IO);
					break;
				case CAM3:
					CAM3_LED_IO(!GET_CAM3_LED_IO);
					break;
				case CAM4:
					CAM4_LED_IO(!GET_CAM4_LED_IO);
					break;
				default:
					break;
			}
			osSemaphoreRelease(osUI_CuSemId);
			return;
		default:
			break;
	}
	*pThreadCnt 	    					= 0;
	tUI_CuSetting.IconSts.ubDrawStsIconFlag = (UI_SHOWSTSICON_STATE == tUI_State)?tUI_CuSetting.IconSts.ubDrawStsIconFlag:FALSE;
END_UPDATESTS:
	tUI_CuSetting.IconSts.ubRdPairIconFlag 	= FALSE;
	osSemaphoreRelease(osUI_CuSemId);
	if(ubUI_SendMsg2AppFlag == TRUE)
	{
		tUI_GetLinkStsMsg.ubAPP_Event 		= APP_LINKSTATUS_REPORT_EVENT;
		UI_SendMessageToAPP(&tUI_GetLinkStsMsg);
	}	
}
//------------------------------------------------------------------------------
void UI_EventHandles(UI_Event_t *ptEventPtr)
{
	if(FALSE == ubUI_CuStartUpFlag)
		return;
	switch(ptEventPtr->tEventType)
	{
		case AKEY_EVENT:
		case PKEY_EVENT:
			osSemaphoreWait(osUI_CuSemId, osWaitForever);
			UI_KeyEventExec(ptEventPtr->pvEvent);
			osSemaphoreRelease(osUI_CuSemId);
			break;
		case SCANMODE_EVENT:
			UI_ScanModeExec();
			break;
		case FWUPG_EVENT:
			KNL_SDUpgradeFwFunc();
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
APP_EventMsg_t *tUI_ViewTypeSetup(UI_CamViewType_t tViewType)
{
	static APP_EventMsg_t tUI_ViewTypeParam = {0};
	UI_CamNum_t tCamNum;

	tCamViewSel.tCamViewType = tViewType;
	if((DISPLAY_1T1R == tUI_CuSetting.ubTotalCamNum) ||
	   ((DISPLAY_2T1R == tUI_CuSetting.ubTotalCamNum) && (DUAL_VIEW == tViewType)))
		return NULL;
	switch(tViewType)
	{
		case SINGLE_VIEW:
		case DUAL_VIEW:
			tCamViewSel.tCamViewPool[0] = NO_CAM;
			tCamViewSel.tCamViewPool[1] = NO_CAM;
			for(tCamNum = CAM1; tCamNum < tUI_CuSetting.ubTotalCamNum; tCamNum++)
			{
				if(tUI_CamStatus[tCamNum].ulCAM_ID != INVALID_ID)
				{
					if(NO_CAM == tCamViewSel.tCamViewPool[0])
					{
						tCamViewSel.tCamViewPool[0] = tCamNum;
					}
					else if(NO_CAM == tCamViewSel.tCamViewPool[1])
					{
						tCamViewSel.tCamViewPool[1] = tCamNum;
						break;
					}
				}
			}
			if(NO_CAM == tCamViewSel.tCamViewPool[0])
				return NULL;
			if(NO_CAM == tCamViewSel.tCamViewPool[1])
				tCamViewSel.tCamViewPool[1] = ((tCamViewSel.tCamViewPool[0] + 1) >= ((DISPLAY_2T1R == tUI_CuSetting.ubTotalCamNum)?CAM_2T:CAM_4T))?CAM1:
											   ((UI_CamNum_t)(tCamViewSel.tCamViewPool[0] + 1));
			tUI_ViewTypeParam.ubAPP_Message[0] = tCamViewSel.tCamViewPool[0];
			tUI_ViewTypeParam.ubAPP_Message[1] = tCamViewSel.tCamViewPool[1];
			break;
		case QUAD_VIEW:
			for(tCamNum = CAM1; tCamNum <= CAM4; tCamNum++)
			{
				tCamViewSel.tCamViewPool[tCamNum] = tCamNum;
				tUI_ViewTypeParam.ubAPP_Message[tCamNum] = tCamViewSel.tCamViewPool[tCamNum];
			}
			break;
		default:
			return NULL;
	}
	tUI_CuSetting.tAdoSrcCamNum 	   = tCamViewSel.tCamViewPool[0];

	return &tUI_ViewTypeParam;
}
//------------------------------------------------------------------------------
void UI_ClearOsdImage(void)
{
	OSD_IMG_INFO tOsdImgInfo;

	tOsdImgInfo.uwHSize  = uwOSD_GetHSize();
	tOsdImgInfo.uwVSize  = uwOSD_GetVSize();
	tOsdImgInfo.uwXStart = 0;
	tOsdImgInfo.uwYStart = 0;
	OSD_EraserImg1(&tOsdImgInfo);
}
//------------------------------------------------------------------------------
void UI_PowerKey(void)
{
	BUZ_PlayPowerOffSound();
	osDelay(600);			//wait buzzer play finish
	LCDBL_ENABLE(UI_DISABLE);
	POWER_LED_IO  = 0;
	CAM1_LED_IO(0);
	CAM2_LED_IO(0);
	CAM3_LED_IO(0);
	CAM4_LED_IO(0);
	RTC_WriteUserRam(RTC_RECORD_PWRSTS_ADDR, RTC_PWRSTS_KEEP_TAG);
	RTC_SetGPO_1(0, RTC_PullDownEnable);
	printd(DBG_Debug1Lvl, "Power OFF!\n");
	RTC_PowerDisable();
	while(1);
}
//------------------------------------------------------------------------------
UI_Result_t UI_DPTZ_KeyPress(uint8_t ubKeyID, uint8_t ubKeyMapIdx)
{
	return rUI_FAIL;
}
//------------------------------------------------------------------------------
void UI_DPTZ_KeyRelease(uint8_t ubKeyID)
{
}
//------------------------------------------------------------------------------
void UI_DPTZ_Control(UI_ArrowKey_t tArrowKey)
{
}
//------------------------------------------------------------------------------
void UI_MD_Window(UI_ArrowKey_t tArrowKey)
{
}
//------------------------------------------------------------------------------
void UI_RemoveLostLinkLogo(void)
{
	tUI_CuSetting.IconSts.ubShowLostLogoFlag = FALSE;
}
//------------------------------------------------------------------------------
void UI_ShowLostLinkLogo(uint16_t *pThreadCnt)
{
	uint16_t uwUI_LostPeriod = (FALSE == ubUI_ResetPeriodFlag)?(UI_SHOWLOSTLOGO_PERIOD * 2):UI_SHOWLOSTLOGO_PERIOD;

	if((FALSE == tUI_CuSetting.IconSts.ubShowLostLogoFlag) && (*pThreadCnt == uwUI_LostPeriod))
	{
		tUI_CuSetting.IconSts.ubShowLostLogoFlag = TRUE;
		ubUI_ResetPeriodFlag = TRUE;		
		switch(tUI_State)
		{
		    case UI_RECPLAYLIST_STATE:
			case UI_PHOTOPLAYLIST_STATE:
			case UI_PAIRING_STATE:
				return;
			case UI_MAINMENU_STATE:
			case UI_SUBSUBSUBMENU_STATE:
			case UI_RECFOLDER_SEL_STATE:
			case UI_RECFILES_SEL_STATE:
			case UI_SDCARDFMT_STATE:
				break;
			case UI_SUBMENU_STATE:
			case UI_SUBSUBMENU_STATE:
				break;
			default:				
				tUI_CuSetting.IconSts.ubDrawStsIconFlag = FALSE;
				tUI_State = UI_DISPLAY_STATE;
				break;
		}
		UI_UpdateOSD(UI_ERASE_STATUS, NULL);
		tLCD_JpegDecodeDisable();
		OSD_LogoJpeg((UI_RES_480 == tUI_AHDosdStatus.tRes)?OSDLOGO_LOSTLINK_VGA:
														   OSDLOGO_LOSTLINK);
	}
}
//------------------------------------------------------------------------------					
void UI_UpdateCamStatus(UI_CamNum_t tCamNum, void *pvStatus)
{
}
//------------------------------------------------------------------------------
void UI_UnBindCam(UI_CamNum_t tUI_DelCam)
{
	APP_EventMsg_t tUI_UnindBuMsg = {0};

	if(INVALID_ID == tUI_CamStatus[tUI_DelCam].ulCAM_ID)
		return;
	tUI_CamStatus[tUI_DelCam].ulCAM_ID 	  = INVALID_ID;
	tUI_CamStatus[tUI_DelCam].tCamConnSts = CAM_OFFLINE;
	tUI_CuSetting.ubPairedCamNum -= (tUI_CuSetting.ubPairedCamNum == 0)?0:1;
	UI_ResetDevSetting(tUI_DelCam);
	UI_UpdateDevStatusInfo();
	tUI_UnindBuMsg.ubAPP_Event 		= APP_UNBIND_CAM_EVENT;
	tUI_UnindBuMsg.ubAPP_Message[0] = 1;		//! Message Length
	tUI_UnindBuMsg.ubAPP_Message[1] = tUI_DelCam;
	UI_SendMessageToAPP(&tUI_UnindBuMsg);
}
//------------------------------------------------------------------------------
void UI_VoxTrigger(UI_CamNum_t tCamNum, void *pvTrig)
{
	if(tCamNum > CAM4)
		return;

	if(PS_VOX_MODE == tUI_CuSetting.tPsMode)
		UI_DisableVox();
}
//------------------------------------------------------------------------------
void UI_EnableVox(void)
{
	APP_EventMsg_t tUI_PsMessage = {0};

	LCDBL_ENABLE(UI_DISABLE);
	tUI_PsMessage.ubAPP_Event 	   = APP_POWERSAVE_EVENT;
	tUI_PsMessage.ubAPP_Message[0] = 2;		//! Message Length
	tUI_PsMessage.ubAPP_Message[1] = PS_VOX_MODE;
	tUI_PsMessage.ubAPP_Message[2] = TRUE;
	UI_SendMessageToAPP(&tUI_PsMessage);
	tUI_CuSetting.tPsMode = PS_VOX_MODE;
	UI_UpdateDevStatusInfo();
}
//------------------------------------------------------------------------------
void UI_DisableVox(void)
{
	APP_EventMsg_t tUI_PsMessage = {0};
	UI_CUReqCmd_t tPsCmd;
	UI_CamNum_t tCamNum;

	for(tCamNum = CAM1; tCamNum < tUI_CuSetting.ubTotalCamNum; tCamNum++)
	{
		if(PS_ECO_MODE == tUI_CamStatus[tCamNum].tCamPsMode)
			continue;
		tUI_CamStatus[tCamNum].tCamPsMode = POWER_NORMAL_MODE;
		if(CAM_OFFLINE == tUI_CamStatus[tCamNum].tCamConnSts)
			continue;
		tPsCmd.tDS_CamNum 				= tCamNum;
		tPsCmd.ubCmd[UI_TWC_TYPE]		= UI_SETTING;
		tPsCmd.ubCmd[UI_SETTING_ITEM]   = UI_VOXMODE_SETTING;
		tPsCmd.ubCmd[UI_SETTING_DATA]   = POWER_NORMAL_MODE;
		tPsCmd.ubCmd_Len  				= 3;
		if(UI_SendRequestToCAM(osThreadGetId(), &tPsCmd) != rUI_SUCCESS)
			printd(DBG_ErrorLvl, "CAM%d:Disable VOX Notify Fail !\n", tCamNum);
	}
	tUI_PsMessage.ubAPP_Event 	   = APP_POWERSAVE_EVENT;
	tUI_PsMessage.ubAPP_Message[0] = 2;		//! Message Length
	tUI_PsMessage.ubAPP_Message[1] = PS_VOX_MODE;
	tUI_PsMessage.ubAPP_Message[2] = FALSE;
	UI_SendMessageToAPP(&tUI_PsMessage);
	tUI_CuSetting.tPsMode = POWER_NORMAL_MODE;
	UI_UpdateDevStatusInfo();
	tUI_State = UI_DISPLAY_STATE;
}
//------------------------------------------------------------------------------
void UI_DisableCuAdoOnlyMode(void)
{
	APP_EventMsg_t tUI_PsMessage = {0};

	UI_DisableScanMode();

	tUI_PsMessage.ubAPP_Event 	   = APP_POWERSAVE_EVENT;
	tUI_PsMessage.ubAPP_Message[0] = 2;		//! Message Length
	tUI_PsMessage.ubAPP_Message[1] = PS_ADOONLY_MODE;
	tUI_PsMessage.ubAPP_Message[2] = FALSE;
	UI_SendMessageToAPP(&tUI_PsMessage);
	tUI_CuSetting.tPsMode = POWER_NORMAL_MODE;
	tUI_State = UI_DISPLAY_STATE;
	switch(tCamViewSel.tCamViewType)
	{
		case SINGLE_VIEW:
			if(tCamViewSel.tCamViewPool[0] != tUI_CuSetting.tAdoSrcCamNum)
				UI_SwitchAudioSource(tCamViewSel.tCamViewPool[0]);
			if(SINGLE_VIEW == tCamViewSel.tCamViewType)
				break;
			UI_EnableScanMode();
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_MDTrigger(UI_CamNum_t tCamNum, void *pvTrig)
{
}
//------------------------------------------------------------------------------
void UI_VoiceTrigger(UI_CamNum_t tCamNum, void *pvTrig)
{
	if(DISPLAY_1T1R == tUI_CuSetting.ubTotalCamNum)
		return;

	if(SCAN_VIEW == tCamViewSel.tCamViewType)
		UI_DisableScanMode();
//	if(PS_VOX_MODE == tUI_CuSetting.tPsMode)
//	{
//		UI_DisableVox();
//		osDelay(50);
//	}
	tUI_CuSetting.IconSts.ubDrawStsIconFlag = TRUE;

	tUI_State = UI_DISPLAY_STATE;
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
		tosUI_Notify.thread_id = NULL;
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
//------------------------------------------------------------------------------
void UI_RecvCamRequest(TWC_TAG tRecv_StaNum, uint8_t *pTwc_Data)
{
	switch(pTwc_Data[UI_TWC_TYPE])
	{
		case UI_REPORT:
		{
			UI_CamNum_t tCamNum = NO_CAM;

			APP_TwcTagMap2CamNum(tRecv_StaNum, tCamNum);
			if(tUiReportMap2Func[pTwc_Data[UI_REPORT_ITEM]].pvAction)
				tUiReportMap2Func[pTwc_Data[UI_REPORT_ITEM]].pvAction(tCamNum, (uint8_t *)(&pTwc_Data[UI_REPORT_DATA]));
			break;
		}
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_ResetDevSetting(UI_CamNum_t tCamNum)
{
	tUI_CamStatus[tCamNum].tCamConnSts = CAM_OFFLINE;
	tUI_CamStatus[tCamNum].tCamPsMode = POWER_NORMAL_MODE;
	UI_CLEAR_CAMSETTINGTODEFU(tUI_CamStatus[tCamNum].tCamAnrMode,  		CAMSET_OFF);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_CamStatus[tCamNum].tCam3DNRMode, 		CAMSET_OFF);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_CamStatus[tCamNum].tCamvLDCMode, 		CAMSET_OFF);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_CamStatus[tCamNum].tCamAecMode,  		CAMSET_OFF);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_CamStatus[tCamNum].tCamDisMode,  		CAMSET_OFF);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_CamStatus[tCamNum].tCamFlicker, 		CAMFLICKER_50HZ);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_CamStatus[tCamNum].tCamCbrMode,  		CAMSET_ON);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_CamStatus[tCamNum].tCamCondenseMode,  CAMSET_OFF);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_CamStatus[tCamNum].tCamColorParam.ubColorBL, 		 	64);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_CamStatus[tCamNum].tCamColorParam.ubColorContrast,    64);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_CamStatus[tCamNum].tCamColorParam.ubColorSaturation, 	64);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_CamStatus[tCamNum].tCamColorParam.ubColorHue, 		64);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_CamStatus[tCamNum].tREC_Mode, 		REC_MANUAL);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_CamStatus[tCamNum].tREC_Resolution, 	RECRES_HD);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_CamStatus[tCamNum].tPHOTO_Func, 		PHOTOFUNC_OFF);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_CamStatus[tCamNum].tPHOTO_Resolution, PHOTORES_3M);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_CamStatus[tCamNum].tCamImgFlip, CAMIMGFLIP_DISABLE);
	UI_CLEAR_CAMSETTINGTODEFU(tUI_CamStatus[tCamNum].tCamImgMirror, CAMIMGMIRROR_DISABLE);
}
//------------------------------------------------------------------------------
uint8_t ubUI_AhdOutputType(UI_CamAHDtype_t *pt)
{
	uint8_t ubDispType;
	
	if (UI_DISP_CAM == pt->tPacket && UI_DISP_UNKNOW != tUI_CamAhdType[CAM1].tPacket)
		pt = &tUI_CamAhdType[CAM1];
	tUI_AHDosdStatus.ubDispCnt = UI_AHD_DISP_CNT_MAX;
	tUI_AHDosdStatus.ubDispNum = 3;
	tUI_AHDosdStatus.uwDispIdx[2] = pt->tBTS + OSD1IMG_25;
	switch (pt->tPacket)
	{
		case UI_DISP_AHD:
			tUI_AHDosdStatus.uwDispIdx[0] = OSD1IMG_AHD;			
			if (UI_RES_1080 == pt->tRes)
			{
				tUI_AHDosdStatus.tRes = UI_RES_1080;				
				tUI_AHDosdStatus.uwDispIdx[1] = OSD1IMG_1080P;
			#if defined(BSP_D_SNCC71_TP2915_AHD_RX_V3)
				ubDispType = (UI_BTS_NTSC == pt->tBTS)?AHD_TP2915_AHD1080P30:AHD_TP2915_AHD1080P25;
			#endif
			}
			else
			{
				tUI_AHDosdStatus.tRes = UI_RES_720;				
				tUI_AHDosdStatus.uwDispIdx[1] = OSD1IMG_720P;
			#if defined(BSP_D_SNCC71_TP2915_AHD_RX_V3)
				ubDispType = (UI_BTS_NTSC == pt->tBTS)?AHD_TP2915_AHD720P30:AHD_TP2915_AHD720P25;
			#endif
			}
			break;
		case UI_DISP_TVI:
			tUI_AHDosdStatus.uwDispIdx[0] = OSD1IMG_TVI;
			if (UI_RES_1080 == pt->tRes)
			{
				tUI_AHDosdStatus.tRes = UI_RES_1080;
				tUI_AHDosdStatus.uwDispIdx[1] = OSD1IMG_1080P;
			#if defined(BSP_D_SNCC71_TP2915_AHD_RX_V3)
				ubDispType = (UI_BTS_NTSC == pt->tBTS)?AHD_TP2915_TVI1080P30:AHD_TP2915_TVI1080P25;
			#endif
			}
			else
			{
				tUI_AHDosdStatus.tRes = UI_RES_720;
				tUI_AHDosdStatus.uwDispIdx[1] = OSD1IMG_720P;
			#if defined(BSP_D_SNCC71_TP2915_AHD_RX_V3)
				ubDispType = (UI_BTS_NTSC == pt->tBTS)?AHD_TP2915_TVI720P30:AHD_TP2915_TVI720P25;
			#endif
				
				if ( UI_FPS_60 == pt->tFPS )
				{
//					printf(" ----- AHD_TP2915_TVI720P60 ----- \r\n");
				#if defined(BSP_D_SNCC71_TP2915_AHD_RX_V3)
					ubDispType = AHD_TP2915_TVI720P60;
				#endif
				}
			}
			break;
		case UI_DISP_CVBS:
			tUI_AHDosdStatus.ubDispNum = 1;
			tUI_AHDosdStatus.tRes = UI_RES_480;
			tUI_AHDosdStatus.uwDispIdx[0] = pt->tBTS + OSD1IMG_PAL;
		#if defined(BSP_D_SNCC71_TP2915_AHD_RX_V3)
			ubDispType = (UI_BTS_NTSC == pt->tBTS)?AHD_TP2915_CVBS_NTSC:AHD_TP2915_CVBS_PAL;
		#endif
			break;
		case UI_DISP_CAM:
			tUI_CamAhdType[CAM1].tPacket = UI_DISP_AHD;
			tUI_CamAhdType[CAM1].tRes 	 = UI_RES_720;
			tUI_CamAhdType[CAM1].tBTS 	 = pt->tBTS;
		case UI_DISP_CVI:
		default:
			tUI_AHDosdStatus.tRes = UI_RES_720;
			tUI_AHDosdStatus.uwDispIdx[0] = OSD1IMG_AHD;
			tUI_AHDosdStatus.uwDispIdx[1] = OSD1IMG_720P;
		#if defined(BSP_D_SNCC71_TP2915_AHD_RX_V3)
			ubDispType = (UI_BTS_NTSC == pt->tBTS)?AHD_TP2915_AHD720P30:
												   AHD_TP2915_AHD720P25;
		#endif
			break;		
	}
	tUI_AHDosdStatus.uwDispIdx[0] += OSD1IMG_ICON_MAX * tUI_AHDosdStatus.tRes;
	tUI_AHDosdStatus.uwDispIdx[1] += OSD1IMG_ICON_MAX * tUI_AHDosdStatus.tRes;
	tUI_AHDosdStatus.uwDispIdx[2] += OSD1IMG_ICON_MAX * tUI_AHDosdStatus.tRes;
	return ubDispType;
}
//------------------------------------------------------------------------------
void UI_LoadDevStatusInfo(void)
{
	uint32_t ulUI_SFAddr = pSF_Info->ulSize - (UI_SF_START_SECTOR * pSF_Info->ulSecSize);
	UI_DeviceStatusInfo_t tUI_DevStsInfo = {{0}, {0}, {0}, {0}};
	UI_CamNum_t tCamNum;

	SF_Read(ulUI_SFAddr, sizeof(UI_DeviceStatusInfo_t), (uint8_t *)&tUI_DevStsInfo);
	memcpy(tUI_CamStatus, tUI_DevStsInfo.tCAM_StatusInfo, (CAM_4T * sizeof(UI_CamStatus_t)));
	memcpy(&tUI_CuSetting, &tUI_DevStsInfo.tCU_SettingInfo, sizeof(UI_CUSetting_t));
	printd(DBG_InfoLvl, "UI TAG:%s\n",tUI_DevStsInfo.cbUI_DevStsTag);
	printd(DBG_InfoLvl, "UI VER:%s\n",tUI_DevStsInfo.cbUI_FwVersion);
	if((strncmp(tUI_DevStsInfo.cbUI_DevStsTag, SF_AP_UI_SECTOR_TAG, sizeof(tUI_DevStsInfo.cbUI_DevStsTag) - 1)) ||
	   (strncmp(tUI_DevStsInfo.cbUI_FwVersion, SN937XX_FW_VERSION,  sizeof(tUI_DevStsInfo.cbUI_FwVersion) - 1)))
	{
		printd(DBG_ErrorLvl, "UI Default Setting !\n");
		for(tCamNum = CAM1; tCamNum < CAM_4T; tCamNum++)
		{
			memset(&tUI_CamStatus[tCamNum], 0xFF, sizeof(UI_CamStatus_t));
			tUI_CuSetting.AutoScan.tAutoScanCamPool[tCamNum] = NO_CAM;
		}
		memset(&tUI_CuSetting, 0xFF, sizeof(UI_CUSetting_t));
		UI_CLEAR_CALENDAR_TODEFU(tUI_CuSetting.tSysCalendar.uwYear, 2018);
		UI_CLEAR_CALENDAR_TODEFU(tUI_CuSetting.tSysCalendar.ubMonth,   1);
		UI_CLEAR_CALENDAR_TODEFU(tUI_CuSetting.tSysCalendar.ubDate,    1);
		UI_CLEAR_CALENDAR_TODEFU(tUI_CuSetting.tSysCalendar.ubHour,    0);
		UI_CLEAR_CALENDAR_TODEFU(tUI_CuSetting.tSysCalendar.ubMin,     0);
		UI_CLEAR_CALENDAR_TODEFU(tUI_CuSetting.tSysCalendar.ubSec,     0);
		UI_CLEAR_CALENDAR_TODEFU(tUI_CuSetting.RecInfo.tREC_Mode, REC_OFF);
		UI_CLEAR_CALENDAR_TODEFU(tUI_CuSetting.RecInfo.tREC_Time, RECTIME_10MIN);
        REC_TimeSet(0,600);
		tUI_CuSetting.tVdoMode = UI_PHOTOCAP_MODE;
		tUI_CuSetting.tOutputAHD.tPacket = UI_DISP_AHD;
		tUI_CuSetting.tOutputAHD.tRes = UI_RES_720;
	}
	tUI_CuSetting.ubTotalCamNum 			 = DISPLAY_MODE;
	tUI_CuSetting.tAdoSrcCamNum				 = (tUI_CuSetting.tAdoSrcCamNum > CAM4)?CAM1:tUI_CuSetting.tAdoSrcCamNum;
	tUI_CuSetting.BriLvL.tBL_UpdateLvL		 = BL_LVL5;
	tUI_CuSetting.VolLvL.tVOL_UpdateLvL		 = VOL_LVL3;
	tUI_CuSetting.IconSts.ubDrawStsIconFlag  = FALSE;
	tUI_CuSetting.IconSts.ubRdPairIconFlag   = FALSE;
	tUI_CuSetting.IconSts.ubClearThdCntFlag	 = FALSE;
	tUI_CuSetting.IconSts.ubShowLostLogoFlag = FALSE;
	tUI_CuSetting.IconSts.ubDrawMdTrigFlag 	 = FALSE;
	tUI_CuSetting.ubPairedCamNum			 = 0;
	tUI_CuSetting.tCamsBwMode				 = (DISPLAY_MODE == DISPLAY_1T1R)?CAMS_1T30_BWMODE:(DISPLAY_MODE == DISPLAY_2T1R)?CAMS_2T_BWMODE:CAMS_4T_BWMODE;
	UI_CHK_CUSYS(tUI_CuSetting.ubAEC_Mode, AECFUNC_MAX, AECFUNC_OFF);
	UI_CHK_CUSYS(tUI_CuSetting.ubCCA_Mode, CCAMODE_MAX, CCAFUNC_OFF);
	UI_CHK_CUSYS(tUI_CuSetting.tPsMode, POWER_NORMAL_MODE, POWER_NORMAL_MODE);
	UI_CHK_CUSYS(tUI_CuSetting.RecInfo.tREC_Mode, REC_RECMODE_MAX, REC_OFF);
	UI_CHK_CUSYS(tUI_CuSetting.RecInfo.tREC_Time, RECTIME_MAX, RECTIME_10MIN);
	UI_CHK_CUSYS(tUI_CuSetting.tVdoMode, UI_VDOMODE_MAX, UI_PHOTOCAP_MODE);
	UI_CHK_CUSYS(tUI_CuSetting.tCamsBwMode, CAMS_4T_BWMODE, CAMS_4T_BWMODE);
	UI_CHK_CUSYS(tUI_CuSetting.AutoScan.ubScanDuty, 45, 5);
	if(UI_PHOTOCAP_MODE == tUI_CuSetting.tVdoMode)
		tUI_CuSetting.RecInfo.tREC_Mode = REC_OFF;

	tUI_CuSetting.tOutputAHD.tBTS = (GPIO->GPIO_I10)?UI_BTS_NTSC:UI_BTS_PAL;
	tUI_CuSetting.tOutputAHD.tFPS = (UI_BTS_NTSC == tUI_CuSetting.tOutputAHD.tBTS)?UI_FPS_30:UI_FPS_25;
#if defined(BSP_D_SNCC71_TP2915_AHD_RX_V3)
	AHD_TP2915_SetOutputType(ubUI_AhdOutputType(&tUI_CuSetting.tOutputAHD));
#endif
	

    if(tUI_CuSetting.RecInfo.tREC_Time == RECTIME_1MIN)
        REC_TimeSet(0,60);
    else if(tUI_CuSetting.RecInfo.tREC_Time == RECTIME_3MIN)
        REC_TimeSet(0,180);
    else if(tUI_CuSetting.RecInfo.tREC_Time == RECTIME_5MIN)
        REC_TimeSet(0,300);
    else if(tUI_CuSetting.RecInfo.tREC_Time == RECTIME_10MIN)
        REC_TimeSet(0,600);

	for(tCamNum = CAM1; tCamNum < CAM_4T; tCamNum++)
	{
		if((strncmp(tUI_DevStsInfo.cbUI_DevStsTag, SF_AP_UI_SECTOR_TAG, sizeof(tUI_DevStsInfo.cbUI_DevStsTag) - 1) == 0) &&
		   (strncmp(tUI_DevStsInfo.cbUI_FwVersion, SN937XX_FW_VERSION, sizeof(tUI_DevStsInfo.cbUI_FwVersion) - 1) == 0))
		{
			tUI_CamStatus[tCamNum].tCamConnSts = CAM_ONLINE;
			if(tCamNum >= tUI_CuSetting.ubTotalCamNum)
			{
				tUI_CamStatus[tCamNum].ulCAM_ID = INVALID_ID;
				tUI_CuSetting.AutoScan.tAutoScanCamPool[tCamNum] = NO_CAM;
			}
			if(INVALID_ID != tUI_CamStatus[tCamNum].ulCAM_ID)
				tUI_CuSetting.ubPairedCamNum += 1;
			UI_CHK_CAMSFUNCS(tUI_CamStatus[tCamNum].tCamAnrMode,  		CAMSET_OFF);
			UI_CHK_CAMSFUNCS(tUI_CamStatus[tCamNum].tCam3DNRMode, 		CAMSET_OFF);
			UI_CHK_CAMSFUNCS(tUI_CamStatus[tCamNum].tCamvLDCMode, 		CAMSET_OFF);
			UI_CHK_CAMSFUNCS(tUI_CamStatus[tCamNum].tCamAecMode,  		CAMSET_OFF);
			UI_CHK_CAMSFUNCS(tUI_CamStatus[tCamNum].tCamDisMode,  		CAMSET_OFF);
			UI_CHK_CAMFLICKER(tUI_CamStatus[tCamNum].tCamFlicker);
			UI_CHK_CAMSFUNCS(tUI_CamStatus[tCamNum].tCamCbrMode,  		CAMSET_ON);
			UI_CHK_CAMSFUNCS(tUI_CamStatus[tCamNum].tCamCondenseMode,   CAMSET_OFF);
			UI_CHK_CAMPARAM(tUI_CamStatus[tCamNum].tCamColorParam.ubColorBL, 		 64);
			UI_CHK_CAMPARAM(tUI_CamStatus[tCamNum].tCamColorParam.ubColorContrast,   64);
			UI_CHK_CAMPARAM(tUI_CamStatus[tCamNum].tCamColorParam.ubColorSaturation, 64);
			UI_CHK_CAMPARAM(tUI_CamStatus[tCamNum].tCamColorParam.ubColorHue, 		 64);
			UI_CHK_CAMSYS(tUI_CamStatus[tCamNum].tCamPsMode, POWER_NORMAL_MODE, POWER_NORMAL_MODE);
			UI_CHK_CAMSYS(tUI_CamStatus[tCamNum].tREC_Mode, REC_RECMODE_MAX, REC_MANUAL);
			UI_CHK_CAMSYS(tUI_CamStatus[tCamNum].tREC_Resolution, RECRES_MAX, RECRES_HD);
			UI_CHK_CAMSYS(tUI_CamStatus[tCamNum].tPHOTO_Func, PHOTOFUNC_MAX, PHOTOFUNC_OFF);
			UI_CHK_CAMSYS(tUI_CamStatus[tCamNum].tPHOTO_Resolution, PHOTORES_MAX, PHOTORES_3M);
			UI_CHK_CAMIMGSET(tUI_CamStatus[tCamNum].tCamImgFlip, CAMIMGFLIP_ENABLE, CAMIMGFLIP_DISABLE);
			UI_CHK_CAMIMGSET(tUI_CamStatus[tCamNum].tCamImgMirror, CAMIMGMIRROR_ENABLE, CAMIMGMIRROR_DISABLE);	
		}
		else
		{
			tUI_CamStatus[tCamNum].ulCAM_ID = INVALID_ID;
			UI_ResetDevSetting(tCamNum);
		}
	}
}
//------------------------------------------------------------------------------
void UI_UpdateDevStatusInfo(void)
{
	uint32_t ulUI_SFAddr = pSF_Info->ulSize - (UI_SF_START_SECTOR * pSF_Info->ulSecSize);
	UI_DeviceStatusInfo_t tUI_DevStsInfo = {{0}, {0}, {0}, {0}};

	memcpy(tUI_DevStsInfo.cbUI_DevStsTag, SF_AP_UI_SECTOR_TAG, sizeof(tUI_DevStsInfo.cbUI_DevStsTag) - 1);
	memcpy(tUI_DevStsInfo.cbUI_FwVersion, SN937XX_FW_VERSION, sizeof(tUI_DevStsInfo.cbUI_FwVersion) - 1);
	memcpy(tUI_DevStsInfo.tCAM_StatusInfo, tUI_CamStatus, (CAM_4T * sizeof(UI_CamStatus_t)));
	memcpy(&tUI_DevStsInfo.tCU_SettingInfo, &tUI_CuSetting, sizeof(UI_CUSetting_t));
	SF_DisableWrProtect();
	SF_Erase(SF_SE, ulUI_SFAddr, pSF_Info->ulSecSize, 1);
	SF_Write(ulUI_SFAddr, sizeof(UI_DeviceStatusInfo_t), (uint8_t *)&tUI_DevStsInfo);
	SF_EnableWrProtect();
}
//------------------------------------------------------------------------------
void UI_TimerEventStart(uint32_t ulTime_ms, void *pvRegCb)
{
	TIMER_SETUP_t tUI_TimerParam;

	tUI_TimerParam.tCLK 		= TIMER_CLK_EXTCLK;
	tUI_TimerParam.ulTmLoad 	= 10000 * ulTime_ms;
	tUI_TimerParam.ulTmCounter 	= tUI_TimerParam.ulTmLoad;
	tUI_TimerParam.ulTmMatch1 	= tUI_TimerParam.ulTmLoad + 1;
	tUI_TimerParam.ulTmMatch2 	= tUI_TimerParam.ulTmLoad + 1;
	tUI_TimerParam.tOF 			= TIMER_OF_ENABLE;
	tUI_TimerParam.tDIR 		= TIMER_DOWN_CNT;
	tUI_TimerParam.tEM 			= TIMER_CB;
	tUI_TimerParam.pvEvent 		= pvRegCb;
	TIMER_Start(TIMER2_1, tUI_TimerParam);
}
//------------------------------------------------------------------------------
void UI_TimerEventStop(void)
{
	TIMER_Stop(TIMER2_1);
}
//------------------------------------------------------------------------------
void UI_ScanModeTimerEvent(void)
{
	UI_Event_t tScanEvent;
	osMessageQId *pUI_ScanEventQH = NULL;

	UI_TimerEventStop();
	tScanEvent.tEventType = SCANMODE_EVENT;
	tScanEvent.pvEvent 	  = NULL;
	pUI_ScanEventQH 	  = pUI_GetEventQueueHandle();
    osMessagePut(*pUI_ScanEventQH, &tScanEvent, 0);
}
//------------------------------------------------------------------------------
void UI_SetupScanModeTimer(uint8_t ubTimerEn)
{
	ubUI_ScanStartFlag = ubTimerEn;
	if(TRUE == ubTimerEn)
		UI_TimerEventStart((tUI_CuSetting.AutoScan.ubScanDuty * 1000), UI_ScanModeTimerEvent);
	else
		UI_TimerEventStop();
}
//------------------------------------------------------------------------------
void UI_EnableScanMode(void)
{
//	if(PS_ADOONLY_MODE != tUI_CuSetting.tPsMode)
		UI_CheckCameraSource4SV();
	UI_SetupScanModeTimer(TRUE);
}
//------------------------------------------------------------------------------
void UI_DisableScanMode(void)
{
	if(FALSE == ubUI_ScanStartFlag)
		return;
	UI_SetupScanModeTimer(FALSE);
}
//------------------------------------------------------------------------------
void UI_ScanModeExec(void)
{
	UI_CamNum_t tSearchCam = tCamViewSel.tCamViewPool[0];
	uint8_t ubSearchCnt;

	tSearchCam = tCamViewSel.tCamViewPool[0];	//! (PS_ADOONLY_MODE == tUI_CuSetting.tPsMode)?tUI_CuSetting.tAdoSrcCamNum:tCamViewSel.tCamViewPool[0];
	for(ubSearchCnt = 0; ubSearchCnt < tUI_CuSetting.ubTotalCamNum; ubSearchCnt++)
	{
		tSearchCam = ((tSearchCam + 1) >= ((DISPLAY_2T1R == tUI_CuSetting.ubTotalCamNum)?CAM_2T:CAM_4T))?CAM1:((UI_CamNum_t)(tSearchCam + 1));
		if(NO_CAM == tUI_CuSetting.AutoScan.tAutoScanCamPool[tSearchCam])
			continue;
		if((CAMS_2T_BWMODE == tUI_CuSetting.tCamsBwMode) && (tSearchCam > CAM2))
			continue;
		if(((CAMS_1T30_BWMODE == tUI_CuSetting.tCamsBwMode) ||
			(CAMS_1T60_BWMODE == tUI_CuSetting.tCamsBwMode)) && (tSearchCam > CAM1))
			continue;
		if(tUI_CamStatus[tSearchCam].ulCAM_ID != INVALID_ID)
		{
			if(0)	//! (PS_ADOONLY_MODE == tUI_CuSetting.tPsMode)
			{
				UI_SwitchAudioSource(tSearchCam);
				break;
			}
			else if(tSearchCam != tCamViewSel.tCamViewPool[0])
			{
				tCamViewSel.tCamViewType	= SCAN_VIEW;
				tCamViewSel.tCamViewPool[0] = tSearchCam;
				UI_SwitchCameraSource();
				break;
			}
		}
	}
	UI_SetupScanModeTimer(TRUE);
}
//------------------------------------------------------------------------------
UI_Result_t UI_CheckCameraSource4SV(void)
{
	UI_CamNum_t tCamViewNum;

	for(tCamViewNum = CAM1; tCamViewNum < tUI_CuSetting.ubTotalCamNum; tCamViewNum++)
	{
		if(NO_CAM == tUI_CuSetting.AutoScan.tAutoScanCamPool[tCamViewNum])
			continue;
		if(tUI_CamStatus[tCamViewNum].ulCAM_ID != INVALID_ID)
		{
			tCamViewSel.tCamViewType	= SCAN_VIEW;
			tCamViewSel.tCamViewPool[0] = tCamViewNum;
			tUI_CuSetting.tAdoSrcCamNum = tCamViewNum;
			return rUI_SUCCESS;
		}
	}
	return rUI_FAIL;
}
//------------------------------------------------------------------------------
void UI_SwitchCameraSource(void)
{
	APP_EventMsg_t tUI_SwitchBuMsg = {0};

	tUI_SwitchBuMsg.ubAPP_Event 	 = APP_VIEWTYPECHG_EVENT;
	tUI_SwitchBuMsg.ubAPP_Message[0] = 3;		//! Message Length	
	tUI_SwitchBuMsg.ubAPP_Message[1] = tCamViewSel.tCamViewType;
	tUI_SwitchBuMsg.ubAPP_Message[2] = tCamViewSel.tCamViewPool[0];
	tUI_SwitchBuMsg.ubAPP_Message[3] = tCamViewSel.tCamViewPool[1];
	tUI_SwitchBuMsg.ubAPP_Message[4] = ((SINGLE_VIEW == tCamViewSel.tCamViewType) || (SCAN_VIEW == tCamViewSel.tCamViewType))?tUI_CamStatus[tCamViewSel.tCamViewPool[0]].tCamConnSts:0;
	UI_SendMessageToAPP(&tUI_SwitchBuMsg);
	if((tCamViewSel.tCamViewType == SINGLE_VIEW) || (tCamViewSel.tCamViewType == SCAN_VIEW))
		tUI_CuSetting.tAdoSrcCamNum = tCamViewSel.tCamViewPool[0];
}
//------------------------------------------------------------------------------
void UI_SwitchAudioSource(UI_CamNum_t tCamNum)
{
	APP_EventMsg_t tUI_SwitchAdoSrcMsg = {0};

	tUI_CuSetting.tAdoSrcCamNum			 = tCamNum;
	tUI_SwitchAdoSrcMsg.ubAPP_Event 	 = APP_ADOSRCSEL_EVENT;
	tUI_SwitchAdoSrcMsg.ubAPP_Message[0] = 1;		//! Message Length
	tUI_SwitchAdoSrcMsg.ubAPP_Message[1] = tUI_CuSetting.tAdoSrcCamNum;
	tUI_SwitchAdoSrcMsg.ubAPP_Message[2] = TRUE;	//! (PS_ADOONLY_MODE == tUI_CuSetting.tPsMode)?FALSE:TRUE;
	UI_SendMessageToAPP(&tUI_SwitchAdoSrcMsg);
}
//------------------------------------------------------------------------------
void UI_SwitchViewTypeByTrxBwMode(UI_CamsBwMode_t tBwMode)
{
	tUI_CuSetting.tCamsBwMode = tBwMode;
//	UI_UpdateDevStatusInfo();
	switch(tUI_CuSetting.tCamsBwMode)
	{
		case CAMS_1T30_BWMODE:
		case CAMS_1T60_BWMODE:
			tCamViewSel.tCamViewType = SINGLE_VIEW;
			tCamViewSel.tCamViewPool[0] = CAM1;			
			break;
		case CAMS_2T_BWMODE:
			tCamViewSel.tCamViewType = DUAL_VIEW;
			tCamViewSel.tCamViewPool[0] = CAM1;
			tCamViewSel.tCamViewPool[1] = CAM2;
			break;
		case CAMS_4T_BWMODE:
			tCamViewSel.tCamViewType = H_VIEW;
			break;
		default:
			return;
	}
	UI_SwitchCameraSource();
}
//------------------------------------------------------------------------------
void UI_PairingControl(UI_ArrowKey_t tArrowKey)
{
	APP_EventMsg_t tUI_PairMessage = {0};

	tUI_PairMessage.ubAPP_Event = APP_PAIRING_STOP_EVENT;
	UI_SendMessageToAPP(&tUI_PairMessage);
}
//------------------------------------------------------------------------------
void UI_FwUpgViaSdCard(void)
{
}
//------------------------------------------------------------------------------
void UI_DispAhdLedState (uint8_t ubIdx)
{
	switch (ubIdx)
	{
		case 0:
			CAM1_LED_IO(1);
			CAM2_LED_IO(0);
			CAM3_LED_IO(0);
			CAM4_LED_IO(0);
			break;
		case 1:
			CAM1_LED_IO(0);
			CAM2_LED_IO(1);
			CAM3_LED_IO(0);
			CAM4_LED_IO(0);
			break;
		case 2:
			CAM1_LED_IO(0);
			CAM2_LED_IO(0);
			CAM3_LED_IO(1);
			CAM4_LED_IO(0);
			break;	
		case 3:
			CAM1_LED_IO(0);
			CAM2_LED_IO(0);
			CAM3_LED_IO(0);
			CAM4_LED_IO(1);
			break;
		case 4:
			CAM1_LED_IO(0);
			CAM2_LED_IO(0);
			CAM3_LED_IO(1);
			CAM4_LED_IO(1);
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_SwitchCamAhdType (UI_CamNum_t tCamNum, UI_ChangeDispState_t tRxState)
{
	KNL_ROLE	tKnlRole;
	KNL_SRC		tKnlSrc;	
	uint16_t   uwKnlSrcH, uwKnlSrcV;
	uint16_t   uwCamMaxH, uwCamMaxV;
	
	tKnlRole = APP_GetSTANumMappingTable(tCamNum)->tKNL_StaNum; 
	tKnlSrc = VDO_GetSourceNumber(KNL_MAIN_PATH, tKnlRole);

	switch(tUI_CamAhdType[tCamNum].tRes)
	{
		case UI_RES_1080:
		#ifdef A7130
			uwCamMaxH = HD_WIDTH;
			uwCamMaxV = HD_HEIGHT;
		#else
			uwCamMaxH = FHD_WIDTH;
			uwCamMaxV = FHD_HEIGHT;
		#endif
			break;
		case UI_RES_720:
			uwCamMaxH = HD_WIDTH;
			uwCamMaxV = HD_HEIGHT;
			break;
		case UI_RES_480:
			uwCamMaxH = VGA_WIDTH;
			uwCamMaxV = VGA_HEIGHT;
			break;
		default:
			return;
	}
	if (UI_DISP_CAM == tUI_CuSetting.tOutputAHD.tPacket)
	{
		uwKnlSrcH = uwCamMaxH;
		uwKnlSrcV = uwCamMaxV;
	}
	else
	{
		uwKnlSrcH = (VDO_MAIN_H_SIZE(VDO_DISP_TYPE) < uwLCD_GetLcdHoSize())?VDO_MAIN_H_SIZE(VDO_DISP_TYPE):
													  uwLCD_GetLcdHoSize();
		uwKnlSrcV = (VDO_MAIN_V_SIZE(VDO_DISP_TYPE) < uwLCD_GetLcdVoSize())?VDO_MAIN_V_SIZE(VDO_DISP_TYPE):
													  uwLCD_GetLcdVoSize();	
		if (uwKnlSrcV & 0xF)
			uwKnlSrcV = (uwKnlSrcV + 0x10) & (~0xF);
		KNL_SetVdoH(tKnlSrc, uwKnlSrcH);
		KNL_SetVdoV(tKnlSrc, uwKnlSrcV);
	}
	if (UI_DISP_STATE_CHANGE == tRxState ||
		uwCamMaxH < uwKnlSrcH || uwCamMaxV < uwKnlSrcH)
	{
		uwKnlSrcH = (uwKnlSrcH < uwCamMaxH)?uwKnlSrcH:uwCamMaxH;
		uwKnlSrcV = (uwKnlSrcV < uwCamMaxV)?uwKnlSrcV:uwCamMaxV;
	}
	//! Reset Video;	
	printf("%s %s %d call VDO_SwitchAhdType=========\n", __FILE__, __func__, __LINE__);
	VDO_SwitchAhdType(KNL_DISP_SINGLE, &tKnlRole, uwKnlSrcH, uwKnlSrcV);
}
//------------------------------------------------------------------------------
void UI_ReInitAhdOutput (void)
{
#if defined( BSP_D_SNCC71_TP2915_AHD_RX_V3 )
	uint8_t ubX, ubY;
	VDO_Stop();
	UI_UpdateOSD(UI_CLEAR_ALL, NULL);
	osDelay(50);
	AHD_TP2915_DacSuspend();
	LCD_UnInit();
	LCD_RESET_O(0);
	osDelay(10);
	LCD_RESET_O(1);
	osDelay(10);
	AHD_TP2915_SetOutputType(ubUI_AhdOutputType(&tUI_CuSetting.tOutputAHD));
	LCD_Init(LCD_LCD_PANEL);
	UI_GetOsdSft(&ubX, &ubY);
	OSD_ResetOsdStart(ubX, ubY);
	UI_UpdateOSD(UI_DRAW_DISP, NULL);
	UI_SwitchCamAhdType(CAM1, UI_DISP_STATE_CHANGE);
	KNL_VdoDisplaySetting();
	KNL_VdoDisplayParamUpdate();
	LCD_Start();
	UI_RemoveLostLinkLogo();
	VDO_Start();
#endif
}
//------------------------------------------------------------------------------
void UI_RecvCamAhdType (TWC_TAG tRecv_StaNum, uint8_t *pData)
{
	UI_CamNum_t tCamNum = NO_CAM;	
	
	APP_TwcTagMap2CamNum(tRecv_StaNum, tCamNum);
	if (tUI_CamAhdType[tCamNum].tRes != (UI_DispResType_t)pData[0] ||
		tUI_CamAhdType[tCamNum].tPacket != (UI_DispPacketType_t)pData[1] ||
		tUI_CamAhdType[tCamNum].tBTS != (UI_BTS_Type_t)pData[2])
	{
		tUI_CamAhdType[tCamNum].tRes	= (UI_DispResType_t)pData[0];
		tUI_CamAhdType[tCamNum].tPacket = (UI_DispPacketType_t)pData[1];
		tUI_CamAhdType[tCamNum].tBTS	= (UI_BTS_Type_t)pData[2];	
		if (UI_DISP_CAM == tUI_CuSetting.tOutputAHD.tPacket)
			UI_ReInitAhdOutput();
		else
			UI_SwitchCamAhdType(tCamNum, UI_DISP_STATE_KEEP);	
	}
}
//------------------------------------------------------------------------------
void UI_Pair (UI_CamNum_t tCam, UI_DisplayLocation_t tLocation)
{
	APP_EventMsg_t tUI_PairMessage = {0};
	UI_DisableScanMode();
	tUI_State = UI_PAIRING_STATE;
	tUI_PairMessage.ubAPP_Event 	 = APP_PAIRING_START_EVENT;
	tUI_PairMessage.ubAPP_Message[0] = 2;		//! Message Length
	tUI_PairMessage.ubAPP_Message[1] = tPairInfo.tPairSelCam = tCam;
	tUI_PairMessage.ubAPP_Message[2] = tPairInfo.tDispLocation = tLocation;
	tUI_PairMessage.ubAPP_Message[3] = FALSE;
	UI_SendMessageToAPP(&tUI_PairMessage);
	CAM1_LED_IO(0);
	CAM2_LED_IO(0);
	CAM3_LED_IO(0);
	CAM4_LED_IO(0);
}
//------------------------------------------------------------------------------
void UI_PairKey(void)
{
	switch (tUI_State)
	{
		case UI_DISPLAY_STATE:
		case UI_DISP_PACKET_STATE:
		case UI_DISP_RES_STATE:
		case UI_DISP_DATE_STATE:
			tPairInfo.tPairSelCam = CAM1;		
			UI_DispAhdLedState(tPairInfo.tPairSelCam);
		#if (DISPLAY_4T1R == DISPLAY_MODE)
			tUI_State = UI_PAIR_SEL_CAM_STATE;
			break;
		case UI_PAIR_SEL_CAM_STATE:			
		#endif
			UI_Pair(tPairInfo.tPairSelCam, DISP_1T);
			break;
		case UI_PAIRING_STATE:
		{
			APP_EventMsg_t tUI_PairMessage = {0};
			tUI_PairMessage.ubAPP_Event = APP_PAIRING_STOP_EVENT;
			UI_SendMessageToAPP(&tUI_PairMessage);
			break;
		}
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_Key1(void)
{
#if (DISPLAY_4T1R == DISPLAY_MODE)	
	if (UI_PAIR_SEL_CAM_STATE == tUI_State)
	{
		++tPairInfo.tPairSelCam;
		if (CAM_4T == tPairInfo.tPairSelCam)
			tPairInfo.tPairSelCam = CAM1;
		UI_DispAhdLedState(tPairInfo.tPairSelCam);
	}
#endif
}
//------------------------------------------------------------------------------
uint8_t ubUI_GetFebLastDay (void)
{
	uint16_t uwYear = 1000 * tUI_AHDosdStatus.ubNextDate[1] +
					   100 * tUI_AHDosdStatus.ubNextDate[2] +
						10 * tUI_AHDosdStatus.ubNextDate[3] +
							 tUI_AHDosdStatus.ubNextDate[4];
	return ((!(uwYear & 3) && uwYear % 100) || !(uwYear % 400))?29:28;
}
//------------------------------------------------------------------------------
void UI_CheckMaxDay (void)
{
	uint8_t ubMonth = 10 * tUI_AHDosdStatus.ubNextDate[5] +
						   tUI_AHDosdStatus.ubNextDate[6];
	uint8_t ubDay = 10 * tUI_AHDosdStatus.ubNextDate[7] +  
						 tUI_AHDosdStatus.ubNextDate[8];
	uint8_t ubMaxDay[12] = {31, ubUI_GetFebLastDay(), 31, 30, 31, 30,
							31, 31, 30, 31, 30, 31};
	if (ubDay > ubMaxDay[ubMonth - 1])
		ubDay = ubMaxDay[ubMonth - 1];
	tUI_AHDosdStatus.ubNextDate[7] = ubDay / 10;
	tUI_AHDosdStatus.ubNextDate[8] = ubDay % 10;
}
//------------------------------------------------------------------------------
void UI_CheckMonth(uint8_t ubIdx, UI_SetDate_t tMode)
{
	uint8_t ubSft = (5 == ubIdx)?10:1;
	uint8_t ubMonth = 10 * tUI_AHDosdStatus.ubNextDate[5] +
						   tUI_AHDosdStatus.ubNextDate[6];
	if (UI_DATE_RISE == tMode)
	{
		if (2 < ubMonth && 10 == ubSft)
		{
			tUI_AHDosdStatus.ubNextDate[5] = 1;
			tUI_AHDosdStatus.ubNextDate[6] = 0;
			return;
		}
		else
		{
			ubMonth += ubSft;
			if (12 < ubMonth)
			{
				tUI_AHDosdStatus.ubNextDate[5] = 0;
				tUI_AHDosdStatus.ubNextDate[6] = 1;
				UI_SetDate(4, tMode);
				return;
			}
		}
	}
	else
	{
		if (11 > ubMonth && 10 == ubSft)
		{
			tUI_AHDosdStatus.ubNextDate[5] = 0;
			tUI_AHDosdStatus.ubNextDate[6] = 1;
			return;
		}
		else
		{
			ubMonth -= ubSft;
			if (!ubMonth)
			{
				tUI_AHDosdStatus.ubNextDate[5] = 1;
				tUI_AHDosdStatus.ubNextDate[6] = 2;
				UI_SetDate(4, tMode);
				return;
			}
		}			
	}
	tUI_AHDosdStatus.ubNextDate[5] = ubMonth / 10;
	tUI_AHDosdStatus.ubNextDate[6] = ubMonth % 10;
	UI_CheckMaxDay();
}
//------------------------------------------------------------------------------
void UI_CheckDay(uint8_t ubIdx, UI_SetDate_t tMode)
{
	uint8_t ubSft = (7 == ubIdx)?10:1;
	uint8_t ubMonth = 10 * tUI_AHDosdStatus.ubNextDate[5] +
						   tUI_AHDosdStatus.ubNextDate[6];
	uint8_t ubDay = 10 * tUI_AHDosdStatus.ubNextDate[7] +  
						 tUI_AHDosdStatus.ubNextDate[8];
	uint8_t ubMaxDay[12] = {31, ubUI_GetFebLastDay(), 31, 30, 31, 30,
							31, 31, 30, 31, 30, 31};
	if (UI_DATE_RISE == tMode)
	{
		ubDay += ubSft;
		if (ubDay > ubMaxDay[ubMonth - 1])
		{
			if (1 == ubSft)
			{
				tUI_AHDosdStatus.ubNextDate[7] = 0;
				tUI_AHDosdStatus.ubNextDate[8] = 1;
				UI_SetDate(6, tMode);
				return;
			}
			ubDay = ubMaxDay[ubMonth - 1];
		}
	}
	else
	{
		if (ubSft >= ubDay)
		{
			if (10 == ubSft)
			{
				tUI_AHDosdStatus.ubNextDate[7] = 0;
				tUI_AHDosdStatus.ubNextDate[8] = 1;
				return;
			}
			else
			{
				ubDay = ubMaxDay[ubMonth - 1];
				tUI_AHDosdStatus.ubNextDate[7] = 3;
				tUI_AHDosdStatus.ubNextDate[8] = 1;
				UI_SetDate(6, tMode);
				return;
			}
		}
		ubDay -= ubSft;
	}
	tUI_AHDosdStatus.ubNextDate[7] = ubDay / 10;
	tUI_AHDosdStatus.ubNextDate[8] = ubDay % 10;	
}
//------------------------------------------------------------------------------
void UI_SetDate(uint8_t ubIdx, UI_SetDate_t tMode)
{
	uint8_t ubMax, ubMin;
	switch (ubIdx)
	{
		case 13:
		case 11:
			ubMax = 5;
			ubMin = 0;			
			break;
		case 10:
			ubMax = (2 == tUI_AHDosdStatus.ubNextDate[9])?3:9;
			ubMin = 0;			
			break;
		case 9:
			ubMax = 2;
			ubMin = 0;			
			break;
		case 8:
		case 7:
			UI_CheckDay(ubIdx, tMode);
			return;
		case 6:
		case 5:
			UI_CheckMonth(ubIdx, tMode);
			return;
		default:
			ubMax = 9;
			ubMin = 0;			
			break;
	}
	if (UI_DATE_RISE == tMode)
	{
		if (ubMax == tUI_AHDosdStatus.ubNextDate[ubIdx])
		{
			tUI_AHDosdStatus.ubNextDate[ubIdx] = ubMin;
			UI_SetDate(ubIdx - 1, tMode);
		}
		else
			++tUI_AHDosdStatus.ubNextDate[ubIdx];
	}
	else if (ubMin == tUI_AHDosdStatus.ubNextDate[ubIdx])
	{
		tUI_AHDosdStatus.ubNextDate[ubIdx] = ubMax;
		UI_SetDate(ubIdx - 1, tMode);
	}
	else
		--tUI_AHDosdStatus.ubNextDate[ubIdx];
	switch (ubIdx)
	{
		case 1:
		case 2:
		case 3:
		case 4:			
			UI_CheckMaxDay();
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_DispPacket(void)
{
	switch (tUI_State)
	{
		case UI_DISPLAY_STATE:
		case UI_DISP_RES_STATE:
		case UI_DISP_DATE_STATE:
			POWER_LED_IO = 0;
			UI_DispAhdLedState(tUI_CuSetting.tOutputAHD.tPacket);
			tUI_AHDosdStatus.ubDispCnt = 0;
			tUI_AHDosdStatus.ubDispNum = 1;
			if (UI_DISP_CAM == tUI_CuSetting.tOutputAHD.tPacket && 
				UI_DISP_CVBS != tUI_CamAhdType[0].tPacket)
			{
				tUI_AHDosdStatus.ubDispNum = 3;
				if (UI_DISP_UNKNOW != tUI_CamAhdType[0].tPacket)
				{
					tUI_AHDosdStatus.uwDispIdx[0] = OSD1IMG_ICON_MAX * 
												   tUI_AHDosdStatus.tRes +
												   tUI_CamAhdType[0].tPacket +
													OSD1IMG_AHD;
					tUI_AHDosdStatus.uwDispIdx[1] = OSD1IMG_ICON_MAX * 
												   tUI_AHDosdStatus.tRes +
												   tUI_CamAhdType[0].tRes +
													OSD1IMG_720P - UI_RES_720;
					tUI_AHDosdStatus.uwDispIdx[2] = OSD1IMG_ICON_MAX * 
												   tUI_AHDosdStatus.tRes +
												   tUI_CamAhdType[0].tBTS +
													OSD1IMG_25;
				}
			}
			else if (UI_DISP_CVBS == tUI_CuSetting.tOutputAHD.tPacket ||
					 UI_DISP_CAM == tUI_CuSetting.tOutputAHD.tPacket)
				tUI_AHDosdStatus.uwDispIdx[0] = OSD1IMG_ICON_MAX * 
											   tUI_AHDosdStatus.tRes +
											   tUI_CuSetting.tOutputAHD.tBTS +
												OSD1IMG_PAL;
			else
				tUI_AHDosdStatus.uwDispIdx[0] = OSD1IMG_ICON_MAX * 
											   tUI_AHDosdStatus.tRes +
											   tUI_CuSetting.tOutputAHD.tPacket + 
												OSD1IMG_AHD;
			UI_UpdateOSD(UI_DRAW_DISP, NULL);
			tUI_State = UI_DISP_PACKET_STATE;
			break;
		case UI_DISP_PACKET_STATE:
			POWER_LED_IO = 1;
			CAM1_LED_IO(0);
			CAM2_LED_IO(0);
			CAM3_LED_IO(0);
			CAM4_LED_IO(0);
			tUI_State = UI_DISPLAY_STATE;
			if (UI_DISP_STATE_CHANGE == tUI_ChangeDispState)
			{
				switch (tUI_CuSetting.tOutputAHD.tPacket)
				{
					case UI_DISP_AHD:
					case UI_DISP_TVI:
						if (UI_RES_480 == tUI_CuSetting.tOutputAHD.tRes)
							tUI_CuSetting.tOutputAHD.tRes = UI_RES_720;
						break;
					case UI_DISP_CVBS:
						if (UI_RES_480 != tUI_CuSetting.tOutputAHD.tRes)
							tUI_CuSetting.tOutputAHD.tRes = UI_RES_480;
					default:
						break;
				}
				UI_UpdateDevStatusInfo();
				UI_ReInitAhdOutput();
				tUI_ChangeDispState = UI_DISP_STATE_KEEP;				
			}
			else
				UI_UpdateOSD(UI_ERASE_DISP, NULL);
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_Key2(void)
{
	switch (tUI_State)
	{
		case UI_DISP_PACKET_STATE:
		{
			tUI_AHDosdStatus.ubDispCnt = 0;
			tUI_AHDosdStatus.ubDispNum = 1;
			tUI_AHDosdStatus.uwDispIdx[0] = OSD1IMG_ICON_MAX * tUI_AHDosdStatus.tRes;
			switch (tUI_CuSetting.tOutputAHD.tPacket)
			{
				case UI_DISP_AHD:
					tUI_CuSetting.tOutputAHD.tPacket = UI_DISP_TVI;
					tUI_AHDosdStatus.uwDispIdx[0] += OSD1IMG_TVI;
					tUI_ChangeDispState = UI_DISP_STATE_CHANGE;					
					break;
				case UI_DISP_TVI:
					tUI_CuSetting.tOutputAHD.tPacket = UI_DISP_CVBS;
					//tUI_CuSetting.tOutputAHD.tRes = UI_RES_480;
					tUI_AHDosdStatus.uwDispIdx[0] += 
					(UI_BTS_NTSC == tUI_CuSetting.tOutputAHD.tBTS)?OSD1IMG_NTSC:
																   OSD1IMG_PAL;
					tUI_ChangeDispState	= UI_DISP_STATE_CHANGE;
					break;
				case UI_DISP_CVBS:
					tUI_CuSetting.tOutputAHD.tPacket = UI_DISP_CAM;
					//tUI_CuSetting.tOutputAHD.tRes = UI_RES_720;					
					tUI_ChangeDispState	= UI_DISP_STATE_CHANGE;					
					if (UI_DISP_UNKNOW == tUI_CamAhdType[0].tPacket)
						UI_UpdateOSD(UI_ERASE_DISP, NULL);
					else if (UI_DISP_CVBS != tUI_CamAhdType[0].tPacket)
					{
						tUI_AHDosdStatus.ubDispNum = 3;
						tUI_AHDosdStatus.uwDispIdx[0] += OSD1IMG_AHD + 
														tUI_CamAhdType[0].tPacket;
						tUI_AHDosdStatus.uwDispIdx[1] = OSD1IMG_ICON_MAX * 
														tUI_AHDosdStatus.tRes +
														tUI_CamAhdType[0].tRes +
														OSD1IMG_720P - UI_RES_720;						
						tUI_AHDosdStatus.uwDispIdx[2] = OSD1IMG_ICON_MAX * 
														tUI_AHDosdStatus.tRes +
														tUI_CamAhdType[0].tBTS +
														OSD1IMG_25;
					}
					else
						tUI_AHDosdStatus.uwDispIdx[0] += tUI_CuSetting.tOutputAHD.tBTS +
														 OSD1IMG_PAL;
					break;
				case UI_DISP_CAM:
					tUI_CuSetting.tOutputAHD.tPacket = UI_DISP_AHD;
					tUI_AHDosdStatus.uwDispIdx[0] += OSD1IMG_AHD;
					tUI_ChangeDispState	= UI_DISP_STATE_CHANGE;					
				default:
					break;					
			}
			UI_UpdateOSD(UI_DRAW_DISP, NULL);
			UI_DispAhdLedState(tUI_CuSetting.tOutputAHD.tPacket);
			break;
		}
		case UI_DISP_DATE_STATE:
			UI_SetDate(tUI_AHDosdStatus.ubNextDate[0], UI_DATE_FALL);
			UI_UpdateOSD(UI_DRAW_DATE, NULL);
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_DispRes(void)
{
	switch (tUI_State)
	{
		case UI_DISPLAY_STATE:
		case UI_DISP_PACKET_STATE:
		case UI_DISP_DATE_STATE:
			POWER_LED_IO = 0;
			UI_DispAhdLedState(tUI_CuSetting.tOutputAHD.tRes);
			tUI_AHDosdStatus.ubDispCnt = 0;
			tUI_AHDosdStatus.ubDispNum = 1;
			tUI_AHDosdStatus.uwDispIdx[0] = OSD1IMG_ICON_MAX * tUI_AHDosdStatus.tRes;
			if (UI_DISP_CAM == tUI_CuSetting.tOutputAHD.tPacket && 
				UI_DISP_CVBS != tUI_CamAhdType[0].tPacket)
			{
				tUI_AHDosdStatus.ubDispNum = 3;
				tUI_AHDosdStatus.uwDispIdx[0] += OSD1IMG_AHD + 
												tUI_CamAhdType[0].tPacket;
				tUI_AHDosdStatus.uwDispIdx[1] = OSD1IMG_ICON_MAX * 
											   tUI_AHDosdStatus.tRes +
											   tUI_CamAhdType[0].tRes +
												OSD1IMG_720P - UI_RES_720;
				tUI_AHDosdStatus.uwDispIdx[2] = OSD1IMG_ICON_MAX * 
											   tUI_AHDosdStatus.tRes +
											   tUI_CamAhdType[0].tBTS +
												OSD1IMG_25;
			}
			else if (UI_DISP_CVBS == tUI_CuSetting.tOutputAHD.tPacket ||
					 UI_DISP_CAM == tUI_CuSetting.tOutputAHD.tPacket)
				tUI_AHDosdStatus.uwDispIdx[0] += tUI_CuSetting.tOutputAHD.tBTS +
																OSD1IMG_PAL;
			else
				tUI_AHDosdStatus.uwDispIdx[0] += tUI_CuSetting.tOutputAHD.tRes -
												  UI_RES_720 + OSD1IMG_720P;
			UI_UpdateOSD(UI_DRAW_DISP, NULL);
			tUI_State = UI_DISP_RES_STATE;
			break;
		case UI_DISP_RES_STATE:
			POWER_LED_IO = 1;
			CAM1_LED_IO(0);
			CAM2_LED_IO(0);
			CAM3_LED_IO(0);
			CAM4_LED_IO(0);
			tUI_State = UI_DISPLAY_STATE;
			if (UI_DISP_STATE_CHANGE == tUI_ChangeDispState)
			{
				UI_UpdateDevStatusInfo();
				UI_ReInitAhdOutput();
				tUI_ChangeDispState = UI_DISP_STATE_KEEP;				
			}
			else
				UI_UpdateOSD(UI_ERASE_DISP, NULL);
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_Key3(void)
{
	switch (tUI_State)
	{
		case UI_DISP_RES_STATE:
			tUI_AHDosdStatus.ubDispCnt = 0;		
			tUI_AHDosdStatus.ubDispNum = 1;
			tUI_AHDosdStatus.uwDispIdx[0] = OSD1IMG_ICON_MAX * tUI_AHDosdStatus.tRes;
			if (UI_DISP_CAM != tUI_CuSetting.tOutputAHD.tPacket)
			{
				
				switch (tUI_CuSetting.tOutputAHD.tRes)
				{
					case UI_RES_1080:
						tUI_CuSetting.tOutputAHD.tRes = UI_RES_720;
						tUI_AHDosdStatus.uwDispIdx[0] += OSD1IMG_720P;
						tUI_ChangeDispState			  = UI_DISP_STATE_CHANGE;
						break;
					case UI_RES_720:
						tUI_CuSetting.tOutputAHD.tRes = UI_RES_1080;
						tUI_AHDosdStatus.uwDispIdx[0] += OSD1IMG_1080P;
						tUI_ChangeDispState			  = UI_DISP_STATE_CHANGE;
						break;
					default:
						return;
				}
				UI_DispAhdLedState(tUI_CuSetting.tOutputAHD.tRes);
			}
			else
			{
				UI_DispAhdLedState(tUI_CamAhdType[CAM1].tRes);
				if (UI_DISP_CVBS != tUI_CamAhdType[0].tPacket)
				{
					tUI_AHDosdStatus.ubDispNum = 3;
					tUI_AHDosdStatus.uwDispIdx[0] += OSD1IMG_AHD + 
													tUI_CamAhdType[0].tPacket;
					tUI_AHDosdStatus.uwDispIdx[1] = OSD1IMG_ICON_MAX * 
												   tUI_AHDosdStatus.tRes +
												   tUI_CamAhdType[0].tRes +
													OSD1IMG_720P - UI_RES_720;
					tUI_AHDosdStatus.uwDispIdx[2] = OSD1IMG_ICON_MAX * 
												   tUI_AHDosdStatus.tRes +
												   tUI_CamAhdType[0].tBTS +
													OSD1IMG_25;
				}
				else
					tUI_AHDosdStatus.uwDispIdx[0] += tUI_CuSetting.tOutputAHD.tBTS +
																	OSD1IMG_PAL;
			}
			UI_UpdateOSD(UI_DRAW_DISP, NULL);
			break;
		case UI_DISP_DATE_STATE:
			UI_SetDate(tUI_AHDosdStatus.ubNextDate[0], UI_DATE_RISE);
			UI_UpdateOSD(UI_DRAW_DATE, NULL);
		default:				
			break;
	}
}
//------------------------------------------------------------------------------
void UI_Date(void)
{
	switch (tUI_State)
	{
		case UI_DISPLAY_STATE:
			memset(&tUI_AHDosdStatus.ubOsdDate, 0xFF, 
			 sizeof(tUI_AHDosdStatus.ubOsdDate));			
			tUI_AHDosdStatus.ubNextDate[0] = 1;
			tUI_AHDosdStatus.ubNextDate[1] = 2;
			tUI_AHDosdStatus.ubNextDate[2] = 0;
			tUI_AHDosdStatus.ubNextDate[3] = 1;
			tUI_AHDosdStatus.ubNextDate[4] = 9;
			tUI_AHDosdStatus.ubNextDate[5] = 1;
			tUI_AHDosdStatus.ubNextDate[6] = 2;
			tUI_AHDosdStatus.ubNextDate[7] = 0;
			tUI_AHDosdStatus.ubNextDate[8] = 5;
			tUI_AHDosdStatus.ubNextDate[9] = 1;
			tUI_AHDosdStatus.ubNextDate[10] = 0;
			tUI_AHDosdStatus.ubNextDate[11] = 4;
			tUI_AHDosdStatus.ubNextDate[12] = 5;
			tUI_AHDosdStatus.ubNextDate[13] = 1;
			tUI_AHDosdStatus.ubNextDate[14] = 6;
			UI_UpdateOSD(UI_DRAW_DATE, NULL);
			tUI_State = UI_DISP_DATE_STATE;
			break;
		case UI_DISP_DATE_STATE:
			UI_UpdateOSD(UI_ERASE_DATE, NULL);
			tUI_State = UI_DISPLAY_STATE;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_Key4(void)
{
	if (UI_DISP_DATE_STATE == tUI_State)
	{
		if (sizeof(tUI_AHDosdStatus.ubNextDate) - 1 == 
				   tUI_AHDosdStatus.ubNextDate[0])
			tUI_AHDosdStatus.ubNextDate[0] = 1;
		else
			++tUI_AHDosdStatus.ubNextDate[0];
		UI_UpdateOSD(UI_DRAW_DATE, NULL);
	}
}
//------------------------------------------------------------------------------
void UI_GetOsdSft (uint8_t *pubX, uint8_t *pubY)
{
	switch (tUI_AHDosdStatus.tRes)
	{
		case UI_RES_1080:
			pubX[0] = 28;
			pubY[0] = 44;
			break;
		case UI_RES_720:
			pubX[0] = 24;
			pubY[0] = 24;
			break;
		case UI_RES_480:
			pubX[0] = 40;
			pubY[0] = 24;
			break;
		default:
			pubX[0] = 0;
			pubY[0] = 0;
			break;
	}
}
//------------------------------------------------------------------------------
#define UI_COUNT_SFT	(73)
static void UI_SignalThread(void const *argument)
{
	static uint32_t ulCount = 0;
	while(1)
	{

		if (UI_DISP_CAM != tUI_CuSetting.tOutputAHD.tPacket)
		{
			if (GPIO->GPIO_I10 && UI_BTS_PAL == tUI_CuSetting.tOutputAHD.tBTS)
			{
				tUI_CuSetting.tOutputAHD.tBTS = UI_BTS_NTSC;
				tUI_CuSetting.tOutputAHD.tFPS = UI_FPS_30;
				UI_ReInitAhdOutput();
			}
			else if (!GPIO->GPIO_I10 && UI_BTS_NTSC == tUI_CuSetting.tOutputAHD.tBTS)
			{
				tUI_CuSetting.tOutputAHD.tBTS = UI_BTS_PAL;
				tUI_CuSetting.tOutputAHD.tFPS = UI_FPS_25;
				UI_ReInitAhdOutput();
			}
		}
		if (UI_DISPLAY_STATE == tUI_State)
		{
			if (ubUI_Signal[0] && !(ulCount % (UI_COUNT_SFT - ubUI_Signal[0])))
				CAM1_LED_IO(!GET_CAM1_LED_IO);
			if (ubUI_Signal[1] && !(ulCount % (UI_COUNT_SFT - ubUI_Signal[1])))
				CAM2_LED_IO(!GET_CAM2_LED_IO);
			if (ubUI_Signal[2] && !(ulCount % (UI_COUNT_SFT - ubUI_Signal[2])))
				CAM3_LED_IO(!GET_CAM3_LED_IO);
			if (ubUI_Signal[3] && !(ulCount % (UI_COUNT_SFT - ubUI_Signal[3])))
				CAM4_LED_IO(!GET_CAM4_LED_IO);			
		}
		++ulCount;
		osDelay(10);
	}
}
#endif
