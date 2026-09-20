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
	\brief		User Interface of BUC Center Unit (for High Speed Mode)
	\author		Hanyi Chiu
	\version	1.18
	\date		2021/12/01
	\copyright	Copyright (C) 2021 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#include "BSP.h"

#if (defined(BSP_SN93711_FHD_REC_RX_V4)||defined(BSP_D_SN93701_SSD2828_RX_V5) || defined(BSP_D_SN93701_TC358778_RX_V6))

#include <string.h>
#include "BSP.h"
#include "UI_BUCCU[HD].h"
#include "SF_API.h"
#include "EN_API.h"
#include "FWU_API.h"
#include "TIMER.h"
#include "VDO.h"
#include "Buzzer.h"
#include "PLY_API.h"
#include "REC_API.h"
#include "BB_API.h"
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
	{NULL,				0,			NULL,						NULL},
	{AKEY_MENU, 		20,			UI_MenuKey, 				BUZ_PlaySingleSound},
	{AKEY_MENU, 		0,			UI_CameraSettingMenu1Key,	BUZ_PlaySingleSound},
	{AKEY_UP, 			0,			UI_UpArrowKey,				BUZ_PlaySingleSound},
	{AKEY_UP, 			30,			UI_FwUpgViaSdCard,   		NULL},
	{AKEY_DOWN, 		0,			UI_DownArrowKey,			BUZ_PlaySingleSound},
	{AKEY_DOWN, 		50,			UI_EngModeKey,				BUZ_PlaySingleSound},
	{AKEY_LEFT, 		0,			UI_LeftArrowKey,			BUZ_PlaySingleSound},
	{AKEY_LEFT, 		30,			UI_LeftArrowLongKey,		BUZ_PlaySingleSound},
	{AKEY_RIGHT, 		0,			UI_RightArrowKey,			BUZ_PlaySingleSound},
	{AKEY_RIGHT, 		30,			UI_EnPerDebugMode,			BUZ_PlaySingleSound},
	{AKEY_ENTER, 		0,			UI_EnterKey,				BUZ_PlaySingleSound},
	{AKEY_ENTER, 		20,			UI_CameraSettingMenu2Key, 	BUZ_PlaySingleSound},
#ifdef S2019A
	{AKEY_PS,			20,			UI_SetSPRfWorkCh,			NULL},
	{AKEY_PTT,			0,			NULL,						NULL},
#else
#ifdef A7130
    #if (APP_DOORPHONE_ENABLE==0)
	{AKEY_PS,			0,			UI_CamPowerSaveKey,			BUZ_PlaySingleSound},
	{AKEY_PS,			20,			UI_CuPowerSaveKey,			BUZ_PlaySingleSound},
    {AKEY_PTT,			0,		    UI_PushTalkKey,				NULL},
    #else
	{AKEY_PS,			0,			UI_DP_KEY2,			        BUZ_PlaySingleSound},	
	{AKEY_PS,			20,			NULL,			            NULL},
    {AKEY_PTT,			0,			UI_DP_KEY1,			        BUZ_PlaySingleSound},
    {AKEY_PTT,			20,			NULL,			            NULL},
    #endif
#else
	{AKEY_PS,			0,			NULL,						NULL},
	{AKEY_PS,			20,			NULL,						NULL},
    {AKEY_PTT,			0,		    UI_PushTalkKey,				NULL},
#endif
#endif
	{PKEY_ID0, 			20,			UI_PowerKey,				NULL},
};
static UI_State_t tUI_State;
static APP_State_t tUI_SyncAppState;
static UI_CamStatus_t tUI_CamStatus[CAM_4T];
static UI_CUSetting_t tUI_CuSetting;
OSD_IMG_INFO tOsdImgInfo1,tOsdImgInfo2;

const static UI_MenuFuncPtr_t tUI_StateMap2MenuFunc[UI_STATE_MAX] =
{
	[UI_DISPLAY_STATE]			= UI_DisplayArrowKeyFunc,
	[UI_MAINMENU_STATE] 		= UI_Menu,
	[UI_SUBMENU_STATE]  		= UI_SubMenu,
	[UI_SUBSUBMENU_STATE]  		= UI_SubSubMenu,
	[UI_SUBSUBSUBMENU_STATE]  	= UI_SubSubSubMenu,
	[UI_CAM_SEL_STATE]			= UI_CameraSelection,
	[UI_SET_VDOMODE_STATE]		= UI_ChangeVideoMode,
	[UI_SET_ADOSRC_STATE]		= UI_ChangeAudioSource,
	[UI_SET_CUPSMODE_STATE] 	= UI_CuPowerSaveModeSelection,
	[UI_SET_CAMECOMODE_STATE] 	= UI_CamPowerSaveModeSelection,
	[UI_CAMSETTINGMENU_STATE]	= UI_CameraSettingMenu,
	[UI_SET_CAMCOLOR_STATE] 	= UI_CameraColorSetting,
	[UI_DPTZ_CONTROL_STATE]		= UI_DPTZ_Control,
	[UI_MD_WINDOW_STATE]		= UI_MD_Window,
	[UI_PAIRING_STATE]			= UI_PairingControl,
	[UI_DUALVIEW_CAMSEL_STATE]  = UI_CameraSelection4DualView,
	[UI_SDFWUPG_STATE]			= UI_FwUpgExecSel,
	[UI_RECFOLDER_SEL_STATE]	= UI_DCIMFolderSelection,
	[UI_RECFILES_SEL_STATE]		= UI_RecordFileSelection,
	[UI_RECPLAYLIST_STATE]      = UI_RecordPlayListSelection,
	[UI_RECPLAYADOSRC_SEL_STATE]= UI_RecordPlayAdoSrcSelection,
	[UI_PHOTOPLAYNRDY_STATE]	= NULL,
	[UI_PHOTOPLAYLIST_STATE]	= UI_PhotoPlayListSelection,
	[UI_SDCARDFMT_STATE]		= UI_SdCardFormatFunc,
	[UI_ENGMODE_STATE]			= UI_EngModeCtrl,
	[UI_SPRF_SEL_STATE]			= UI_sPRfChSelection,
};
static UI_MenuItem_t tUI_MenuItem;
static UI_SubMenuItem_t tUI_SubMenuItem[MENUITEM_MAX] =
{
	{CAMSSELCAM_ITEM,	CAMSITEM_MAX	},
	{PAIRCAM_ITEM,		PAIRITEM_MAX	},
	{RECMODE_ITEM,		RECITEM_MAX		},
	{PHOTOSELCAM_ITEM, 	PHOTOITEM_MAX	},
	{NULL},
	{NULL},
	{DATETIME_ITEM, 	SETTINGITEM_MAX	},
};
const UI_ReportFuncPtr_t tUiReportMap2Func[] =
{
	[UI_UPDATE_CAMSTS] 			= UI_UpdateCamStatus,
	[UI_VOX_TRIG]				= UI_VoxTrigger,
	[UI_MD_TRIG]				= UI_MDTrigger,
	[UI_VOICE_TRIG]				= UI_VoiceTrigger,
};

const ADO_R2R_VOL tUI_VOLTable[] = {R2R_VOL_n45DB, R2R_VOL_n32p4DB, R2R_VOL_n26p2DB, R2R_VOL_n21p4DB, R2R_VOL_n14p6DB, R2R_VOL_n8p2DB};

osSemaphoreId osUI_CuSemId;
static UI_SubMenuCamNum_t tCamSelect;
static UI_CamViewSelect_t tCamViewSel;
static UI_PairingInfo_t tPairInfo;
static UI_ThreadNotify_t tosUI_Notify;
static UI_DPTZParam_t tUI_DptzParam;
static UI_CamNum_t tUI_CamEcoCamNum;
static UI_CamNum_t tUI_CamNumSel;
static UI_SdSts_t tUI_SdCardSts;
static uint32_t ulUI_LogoIndex;
static uint32_t ulUI_MonitorPsFlag[CAM_4T];
static uint8_t ubUI_CuStartUpFlag;
static uint8_t ubUI_PttStartFlag;
static uint8_t ubUI_ResetPeriodFlag;
static uint8_t ubUI_FastStateFlag;
static uint8_t ubUI_ShowTimeFlag;
static uint8_t ubUI_ScanStartFlag;
static uint8_t ubUI_RecSubMenuFlag;
static uint8_t ubUI_DualViewExFlag;
static uint8_t ubUI_StopUpdateStsBarFlag;
uint8_t *pUI_CamConnectFlag[CAM_STSMAX];
static uint8_t ubUI_DisScanMdFunc;

//! Record
static void UI_OsdLoadingDisplayThread(void const *argument);
osThreadId osUI_OsdLdDispThdId;
osMessageQId osUI_OsdLdDispQueue;
osMessageQId osUI_RecRptQueue;
osSemaphoreId osUI_OsdLdStsUpdSem;
static UI_OsdLdDispSts_t tUI_OsdLdDispSts;
static UI_RecFoldersInfo_t pUI_RecFoldersInfo;
static UI_RecFilesInfo_t pUI_RecFilesInfo;
static UI_RecOsdImgDb_t tUI_RecOsdImgDB;
static OSD_IMGIDXARRARY_t tUI_RecOsdImgInfo;
static UI_RecPlayAct_t tUI_RecPlayAct;
static uint8_t ubUI_VdoRecChkFlag;
//! Functions Execute
static void UI_FuncsExecuteThread(void const *argument);
osThreadId osUI_FuncsExecThdId;
osMessageQId osUI_FuncsExecQue;
osMessageQId osUI_FuncsFinExecQue;
//! Performance Debug
#define MAX_DBG_ITEM	3
osMutexId osUI_PerDbgMutex;
static OSD_IMG_INFO tUI_PerDbgOsdImgInfo[17];
static uint32_t ulUI_FrameErrCnt[CAM_4T][MAX_DBG_ITEM];
static uint8_t ubUI_PerDebugEn;

typedef enum
{	
	TRX_DBG_FRMSEQ_IDX	= 0,
	TRX_DBG_BW_IDX,
	TRX_DBG_FPS_IDX,
}UI_TRX_DBG_IDX;

uint8_t ubUI_TrxDebugUpdateFlg[3]={1,1,1};//FrmSeq/Bw/Fps

//! For SPRF
static uint8_t ubUI_sPRfChSel;
#if (APP_DOORPHONE_ENABLE==1)
static uint8_t *pUI_DP_SyncAPPSt;
#endif
//------------------------------------------------------------------------------

UI_State_t tUI_GetState(void)
{
	return tUI_State;
}

UI_RecPlayAct_t tUI_GetRecPlayState(void)
{
	return tUI_RecPlayAct;
}
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
#if (APP_DOORPHONE_ENABLE==0)        
		if(((ubUI_KeyEventIdx) && (ubUI_KeyEventIdx < uwUiKeyEvent_Cnt)) ||
		   (ptKeyEvent->ubKeyID == AKEY_PTT))
#else
		if((ubUI_KeyEventIdx) && (ubUI_KeyEventIdx < uwUiKeyEvent_Cnt))
#endif
		{
#if (APP_DOORPHONE_ENABLE==0)
			if(ptKeyEvent->ubKeyID == AKEY_PTT)
				ubUI_PttStartFlag = FALSE;
#endif
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
				if(UiKeyEventMap[ubUI_KeyEventIdx].KeyEventFuncPtr)
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
#if (APP_DOORPHONE_ENABLE==0)             
			if((ptKeyEvent->uwKeyCnt) || (ptKeyEvent->ubKeyID == AKEY_PTT))
#else
			if(ptKeyEvent->uwKeyCnt)
#endif
			{
#if (APP_DOORPHONE_ENABLE==0)             
				if(ptKeyEvent->ubKeyID == AKEY_PTT)
					ubUI_PttStartFlag = TRUE;
#endif
				if(UiKeyEventMap[ubUI_KeyEventIdx].KeyEventFuncPtr)
				{
					UiKeyEventMap[ubUI_KeyEventIdx].KeyEventFuncPtr();
					if(UiKeyEventMap[ubUI_KeyEventIdx].pvKeyTone)
						UiKeyEventMap[ubUI_KeyEventIdx].pvKeyTone();
				}
				ubUI_KeyEventIdx = (ptKeyEvent->ubKeyID == AKEY_PTT)?ubUI_KeyEventIdx:0;
			}
		}
	}
}
//------------------------------------------------------------------------------
void UI_OnInitDialog(void)
{
	if(NULL == osUI_OsdLdDispQueue)
	{
		osMessageQDef(UiLdDispQue, 10, uint16_t);
		osUI_OsdLdDispQueue = osMessageCreate(osMessageQ(UiLdDispQue), NULL);
	}
	if(NULL == osUI_RecRptQueue)
	{
		osMessageQDef(UiRecRptQue, 1, uint8_t);
		osUI_RecRptQueue = osMessageCreate(osMessageQ(UiRecRptQue), NULL);
	}
	if(NULL == osUI_OsdLdDispThdId)
	{
		osThreadDef(UiOsdLdDisp, UI_OsdLoadingDisplayThread, THREAD_PRIO_UIEVENT_HANDLER, 1, 512);
		osUI_OsdLdDispThdId = osThreadCreate(osThread(UiOsdLdDisp), NULL);
	}
	if(NULL == osUI_FuncsExecQue)
	{
		osMessageQDef(UiFuncExecQue, 300, UI_FuncExecMsg_t);
		osUI_FuncsExecQue = osMessageCreate(osMessageQ(UiFuncExecQue), NULL);
	}
	if(NULL == osUI_FuncsFinExecQue)
	{
		osMessageQDef(UiFuncsFinExecQue, 1, uint8_t);
		osUI_FuncsFinExecQue = osMessageCreate(osMessageQ(UiFuncsFinExecQue), NULL);
	}	
	if(NULL == osUI_FuncsExecThdId)
	{
		osThreadDef(UiFuncsExec, UI_FuncsExecuteThread, osPriorityAboveNormal, 1, 8192);
		osUI_FuncsExecThdId = osThreadCreate(osThread(UiFuncsExec), NULL);
	}

	if ((DISPLAY_MODE != DISPLAY_1T1R) && wRTC_ReadUserRam(RTC_RECORD_PWRSTS_ADDR) == RTC_WATCHDOG_CHK_TAG)
	{
		printd(DBG_CriticalLvl, "PWR STS: Keep\n");
		tCamViewSel.tCamViewType    = (UI_CamViewType_t)wRTC_ReadUserRam(RTC_RECORD_VIEW_MODE_ADDR);
		tCamViewSel.tCamViewPool[0] = (UI_CamNum_t)(wRTC_ReadUserRam(RTC_RECORD_VIEW_CAM_ADDR) >> 4);
		tCamViewSel.tCamViewPool[1] = (UI_CamNum_t)(wRTC_ReadUserRam(RTC_RECORD_VIEW_CAM_ADDR) & 0x0f);
	}
	else
	{
		tCamViewSel.tCamViewType    = (VDO_DISP_TYPE == KNL_DISP_H)?H_VIEW:
									  (VDO_DISP_TYPE == KNL_DISP_QUAD)?QUAD_VIEW:
									  (VDO_DISP_TYPE == KNL_DISP_DUAL_C)?DUAL_VIEW:
									  (VDO_DISP_SCAN == TRUE)?SCAN_VIEW:SINGLE_VIEW;
		tCamViewSel.tCamViewPool[0] = ((VDO_DISP_TYPE == KNL_DISP_QUAD) || (VDO_DISP_TYPE == KNL_DISP_H))?CAM_4T:CAM1;
		tCamViewSel.tCamViewPool[1] = (VDO_DISP_TYPE == KNL_DISP_DUAL_C)?CAM2:NO_CAM;
		RTC_WriteUserRam(RTC_RECORD_VIEW_MODE_ADDR, tCamViewSel.tCamViewType);	
		RTC_WriteUserRam(RTC_RECORD_VIEW_CAM_ADDR, (tCamViewSel.tCamViewPool[0] << 4) | tCamViewSel.tCamViewPool[1]);
	}

#if (BSP_RTC_TIMER_SEL == RTC_TIMER_INTERNAL)
	if(iRTC_SetBaseCalendar((RTC_Calendar_t *)(&tUI_CuSetting.tSysCalendar)) != RTC_OK)
		printd(DBG_ErrorLvl, "Calendar base setting fail!\n");

	if (((wRTC_ReadUserRam(RTC_RECORD_PWRSTS_ADDR) & 0xF) != RTC_PWRSTS_KEEP_TAG) &&
		((wRTC_ReadUserRam(RTC_RECORD_PWRSTS_ADDR) & 0xF) != RTC_WATCHDOG_CHK_TAG))
		RTC_SetCalendar((RTC_Calendar_t *)(&tUI_CuSetting.tSysCalendar));
#elif (BSP_RTC_TIMER_SEL == RTC_TIMER_EXTERNAL)
	RTC_PCF85063A_Calendar_t tCalendar;

	RTC_PCF85063A_GetCalendar((RTC_PCF85063A_Calendar_t *)(&tCalendar));
	tUI_CuSetting.tSysCalendar.uwYear 	= tCalendar.uwYear;
	tUI_CuSetting.tSysCalendar.ubMonth 	= tCalendar.ubMonth;
	tUI_CuSetting.tSysCalendar.ubDate	= tCalendar.ubDate;
	tUI_CuSetting.tSysCalendar.ubHour	= tCalendar.ubHour;
	tUI_CuSetting.tSysCalendar.ubMin	= tCalendar.ubMin;
	tUI_CuSetting.tSysCalendar.ubSec	= tCalendar.ubSec;
#endif
	RTC_WriteUserRam(RTC_RECORD_PWRSTS_ADDR, ((wRTC_ReadUserRam(RTC_RECORD_PWRSTS_ADDR) & 0xF0) | RTC_WATCHDOG_CHK_TAG));

	OSD_LogoJpeg(ulUI_LogoIndex);
	GPIO->GPIO_O0 	= 0;
	GPIO->GPIO_O13 	= 0;
	BUZ_PlayPowerOnSound();
	ubUI_sPRfChSel = ubUI_sPRfChSel;
}
//------------------------------------------------------------------------------
#if (APP_ADO_FUNC_ENABLE == 1)
KNL_ROLE PcConn_AdoRole;
#endif
void UI_ShowPcCnnPic(void)
{
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
}
//------------------------------------------------------------------------------
void UI_PcConn_SdCardPlugout(void)
{
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
}
//------------------------------------------------------------------------------
void UI_StateReset(void)
{
	osSemaphoreDef(UiCuSem);
	osUI_CuSemId = osSemaphoreCreate(osSemaphore(UiCuSem), 1);
	osSemaphoreDef(UiOsdLdStsSem);
	osUI_OsdLdStsUpdSem = osSemaphoreCreate(osSemaphore(UiOsdLdStsSem), 1);
	tosUI_Notify.thread_id	  	 = NULL;
	tosUI_Notify.iSignals	  	 = 0;
	ubUI_CuStartUpFlag		  	 = FALSE;
	ubUI_ResetPeriodFlag		 = FALSE;
	ubUI_FastStateFlag		 	 = FALSE;
	ubUI_ShowTimeFlag		 	 = FALSE;
	ubUI_PttStartFlag		  	 = TRUE;
	ubUI_ScanStartFlag    	  	 = FALSE;
	ubUI_DisScanMdFunc			 = FALSE;
	ubUI_DualViewExFlag		  	 = FALSE;
	ubUI_StopUpdateStsBarFlag 	 = FALSE;
	tUI_SdCardSts				 = UI_SD_CFM;
	osUI_OsdLdDispQueue		  	 = NULL;
	osUI_OsdLdDispThdId		  	 = NULL;
	osUI_RecRptQueue			 = NULL;
	tUI_OsdLdDispSts		  	 = UI_OSDLDDISP_OFF;
	osUI_FuncsExecThdId			 = NULL;
	osUI_FuncsExecQue			 = NULL;
	osUI_FuncsFinExecQue		 = NULL;
	ubUI_RecSubMenuFlag		  	 = FALSE;
	memset(&pUI_RecFoldersInfo, 0, sizeof(UI_RecFoldersInfo_t));
	memset(&pUI_RecFilesInfo,   0, sizeof(UI_RecFilesInfo_t));
	tUI_RecPlayAct.tRecAct 		 = UI_REC_STOP;
	tUI_RecPlayAct.tPlaySts 	 = UI_RECFILE_STOP;
	tUI_RecPlayAct.tSimFld       = KNL_REAL_FLD;
	ubUI_VdoRecChkFlag			 = TRUE;
	ubUI_PerDebugEn				 = FALSE;
	ubUI_sPRfChSel				 = 0;
	osMutexDef(UiPerDbgMutex);
	osUI_PerDbgMutex 			 = osMutexCreate(osMutex(UiPerDbgMutex));
	tUI_State 		 		  	 = UI_DISPLAY_STATE;
	tUI_SyncAppState		  	 = APP_STATE_NULL;
	ulUI_LogoIndex			  	 = OSDLOGO_BOOT;
	tUI_MenuItem.ubItemIdx 	  	 = CAMERAS_ITEM;
	tUI_MenuItem.ubItemPreIdx 	 = CAMERAS_ITEM;
	UI_ResetSubMenuInfo();
	UI_ResetSubSubMenuInfo();
	if(tTWC_RegTransCbFunc(TWC_UI_SETTING, UI_RecvCamResponse, UI_RecvCamRequest) != TWC_SUCCESS)
		printd(DBG_ErrorLvl, "UI Setting 2-way command fail!\n");
	UI_LoadDevStatusInfo();

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
	OSD_IMG_INFO tOsdImgInfo;
	static UI_State_t tUI_PreState;

	switch(pFWU_StsRpt->ubAPP_Report[0])
	{
		case FWU_UPG_INPROGRESS:
            if(UI_REC_START == tUI_RecPlayAct.tRecAct)
                UI_VideoRecordingExec(UI_REC_STOP);
			if(TRUE == ubUI_PerDebugEn)
				UI_EnPerDebugMode();
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
			tUI_PreState = tUI_State;
			tUI_State = UI_FWUPG_STATE;
			break;
		case FWU_UPG_SUCCESS:
		case FWU_UPG_FAIL:
		case FWU_UPG_DEVTAG_FAIL:
		{
			uint16_t uwImgIdx = (FWU_UPG_SUCCESS == pFWU_StsRpt->ubAPP_Report[0])?1:0;

			tOSD_GetOsdImgInfor(1, OSD_IMG2, (OSD2IMG_FWUFAILED_ICON + uwImgIdx), 1, &tOsdImgInfo);
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			osDelay(2000);
			if(FWU_UPG_SUCCESS != pFWU_StsRpt->ubAPP_Report[0])
			{
				tOsdImgInfo.uwHSize  = uwOSD_GetHSize();
				tOsdImgInfo.uwVSize  = uwOSD_GetVSize();
				tOsdImgInfo.uwXStart = 0;
				tOsdImgInfo.uwYStart = 0;
				OSD_EraserImg1(&tOsdImgInfo);
			}
			ubUI_StopUpdateStsBarFlag = FALSE;
			tUI_State = tUI_PreState;
			break;
		}
		default:
		{
			uint8_t ubProgScaleIdx = (pFWU_StsRpt->ubAPP_Report[0] / pFWU_StsRpt->ubAPP_Report[1]);
			tOSD_GetOsdImgInfor(1, OSD_IMG2, (OSD2IMG_FWUPROG0P_ICON + ubProgScaleIdx), 1, &tOsdImgInfo);
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			if(100 == pFWU_StsRpt->ubAPP_Report[0])
				osDelay(1000);
			break;
		}
	}
}
//------------------------------------------------------------------------------
void UI_ChkPreviewCamSrc4UnBind(UI_CamNum_t tDelCam)
{
#if (defined(S2019A) || defined(RTC676x))
	UI_CamNum_t tSwCamNum;

	if(tCamViewSel.tCamViewPool[0] == tDelCam)
	{
		for(tSwCamNum = CAM1; tSwCamNum < tUI_CuSetting.ubTotalCamNum; tSwCamNum++)
		{
			if((tSwCamNum != tDelCam) &&
			   (tUI_CamStatus[tSwCamNum].ulCAM_ID != INVALID_ID))
			{
				if(SINGLE_VIEW == tCamViewSel.tCamViewType)
				{
					tCamViewSel.tCamViewPool[0] = tSwCamNum;
					tUI_CuSetting.tAdoSrcCamNum = tSwCamNum;
					UI_SwitchCameraSource();
					break;
				}
			}
		}
	}
#endif
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
			void (*UI_RptPairFunc[])(UI_Result_t) =
			{
				UI_ReportPairingResult,
				UI_ReportAppPairingResult,
			};

			if(TRUE == ubAppPairFlag)
			{
				tPairInfo.tPairSelCam   = (UI_CamNum_t)pAppStsRpt->ubAPP_Report[3];
				tPairInfo.tDispLocation = (UI_DisplayLocation_t)pAppStsRpt->ubAPP_Report[4];
			}
			UI_RptPairFunc[ubAppPairFlag](tPair_Result);
			if((rUI_SUCCESS == tPair_Result) &&
			   (tUI_CuSetting.tAdoSrcCamNum != tAppAdoSrcNum))
				pAppStsRpt->ubAPP_Report[1] = tUI_CuSetting.tAdoSrcCamNum;
			break;
		}
		case APP_LINKSTS_RPT:
			UI_ReportCamConnectionStatus(pAppStsRpt->ubAPP_Report);
			break;
		case APP_VWMODESTS_RPT:
			if (pAppStsRpt->ubAPP_Report[0] != SCAN_VIEW)
				UI_DisableScanMode();
			break;
		case APP_VOXMODESTS_RPT:
			ubUI_StopUpdateStsBarFlag = pAppStsRpt->ubAPP_Report[0];
			if(TRUE == ubUI_StopUpdateStsBarFlag)
				break;
			tUI_CuSetting.IconSts.ubDrawStsIconFlag = TRUE;
			UI_ClearStatusBarOsdIcon();
			UI_ClearCamConnectStatusFlag();
			UI_RemoveLostLinkLogo();
			break;
		case APP_PAIRUDCAM_PRT:
		{
			UI_CamNum_t tDelCam 	= (UI_CamNum_t)pAppStsRpt->ubAPP_Report[0];
			uint8_t ubAppRrefUiFlag = pAppStsRpt->ubAPP_Report[1];

			if(TRUE == ubAppRrefUiFlag)
			{
				tUI_CuSetting.IconSts.ubDrawStsIconFlag = TRUE;
				UI_ClearStatusBarOsdIcon();
				UI_ClearCamConnectStatusFlag();
			}
			UI_ChkPreviewCamSrc4UnBind(tDelCam);
			UI_UnBindCam(tDelCam);
			break;
		}
		case APP_DISPPAIRICON_RPT:
		{
			UI_DisplayAppPairingScreen();
			osSemaphoreRelease(osUI_CuSemId);
			return;
		}
		default:
			break;
	}
	if(pAppStsRpt->tAPP_State == APP_LINK_STATE)
	{
		if(tUI_SyncAppState != pAppStsRpt->tAPP_State)
			UI_RemoveLostLinkLogo();
		ubUI_ResetPeriodFlag = TRUE;
		if((SCAN_VIEW == tCamViewSel.tCamViewType) && (FALSE == ubUI_ScanStartFlag) && (FALSE == ubUI_DisScanMdFunc))
			UI_EnableScanMode();

	}
	if(FALSE == ubUI_CuStartUpFlag)
	{
		ADO_SetDacR2RVol(tUI_VOLTable[tUI_CuSetting.VolLvL.tVOL_UpdateLvL]);
		if(PS_VOX_MODE == tUI_CuSetting.tPsMode)
			UI_EnableVox();
        if((UI_RECORDING_MODE == tUI_CuSetting.tVdoMode) && (REC_LOOPING == tUI_CuSetting.RecInfo.tREC_Mode) &&
		   (POWER_NORMAL_MODE == tUI_CuSetting.tPsMode)  && (SCAN_VIEW != tCamViewSel.tCamViewType))
			UI_VideoRecordingExec(UI_REC_START);
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
	UI_CLEAR_THREADCNT(tUI_CuSetting.IconSts.ubClearThdCntFlag, *pThreadCnt);
	switch(tUI_SyncAppState)
	{
		case APP_IDLE_STATE:
			ubUI_SendMsg2AppFlag = TRUE;
			break;
		case APP_LOSTLINK_STATE:
			ubUI_SendMsg2AppFlag = TRUE;
#if (APP_DOORPHONE_ENABLE==1)
            UI_DP_UpdateIcon();
            #if (APP_SD_FUNC_ENABLE && APP_REC_FUNC_ENABLE)
            if(pUI_DP_SyncAPPSt[CAM1] == APP_DP_IDLE && pUI_DP_SyncAPPSt[CAM2] == APP_DP_IDLE && 
                (UI_RECORDING_MODE == tUI_CuSetting.tVdoMode)&&(UI_REC_START == tUI_RecPlayAct.tRecAct))
                UI_VideoRecordingExec(UI_REC_STOP);
            #endif
#endif
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
#if (APP_DOORPHONE_ENABLE==1)
			    UI_DP_UpdateIcon();
                #if (APP_SD_FUNC_ENABLE && APP_REC_FUNC_ENABLE)
                if((REC_LOOPING == tUI_CuSetting.RecInfo.tREC_Mode) && 
                    (UI_RECORDING_MODE == tUI_CuSetting.tVdoMode)&&(UI_REC_STOP == tUI_RecPlayAct.tRecAct))
                    UI_VideoRecordingExec(UI_REC_START);
                #endif
#endif
				UI_RedrawStatusBar(pThreadCnt);
				(*pThreadCnt)++;
				goto END_UPDATESTS;
			}
			break;
		case APP_PAIRING_STATE:
			UI_DrawPairingStatusIcon();
			osSemaphoreRelease(osUI_CuSemId);
			return;
		default:
			break;
	}
	*pThreadCnt 	    					= 0;
	tUI_CuSetting.IconSts.ubDrawStsIconFlag = (UI_SHOWSTSICON_STATE == tUI_State)?tUI_CuSetting.IconSts.ubDrawStsIconFlag:FALSE;
END_UPDATESTS:
	UI_UpdateBriLvlIcon();
	UI_UpdateVolLvlIcon();
	UI_UpdateRecStsIcon();
	UI_UpdateWarningNoteIcon();
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
			tCamViewSel.tCamViewPool[2] = NO_CAM;
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
			break;
		case TRIPLE_2L1R_VIEW:
		case TRIPLE_1L2R_VIEW:
		case TRIPLE_2T1B_VIEW:
		case TRIPLE_1T2B_VIEW:
		case TRIPLE_3COL_VIEW:
		case V3_3T_VIEW:
			for(tCamNum = CAM1; tCamNum <= (CAM4-1); tCamNum++)
			{
				tCamViewSel.tCamViewPool[tCamNum] = tCamNum;
				tUI_ViewTypeParam.ubAPP_Message[tCamNum] = tCamViewSel.tCamViewPool[tCamNum];
			}			
			break;
		default:
			return NULL;
	}
	tUI_ViewTypeParam.ubAPP_Message[0] = tCamViewSel.tCamViewPool[0];
	tUI_ViewTypeParam.ubAPP_Message[1] = tCamViewSel.tCamViewPool[1];
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
	SIGNAL_LED_IO(0);
	RTC_WriteUserRam(RTC_RECORD_PWRSTS_ADDR, RTC_PWRSTS_KEEP_TAG);
	RTC_SetGPO_1(0, RTC_PullDownEnable);
	if(UI_REC_START == tUI_RecPlayAct.tRecAct)
		UI_VideoRecordingExec(UI_REC_STOP);
	printd(DBG_Debug1Lvl, "Power OFF!\n");
	RTC_PowerDisable();
	while(1);
}
//------------------------------------------------------------------------------
void UI_MenuKey(void)
{
	switch(tUI_State)
	{
		case UI_DISPLAY_STATE:
			tUI_State = UI_MAINMENU_STATE;
			tUI_MenuItem.ubItemPreIdx = CAMERAS_ITEM;
			tUI_MenuItem.ubItemIdx 	  = CAMERAS_ITEM;
			UI_DrawMenuPage();
			break;
		case UI_MAINMENU_STATE:
		{
			OSD_IMG_INFO tOsdInfo;

			tOsdInfo.uwHSize  = uwOSD_GetHSize();
			tOsdInfo.uwVSize  = uwOSD_GetVSize();
			tOsdInfo.uwXStart = 0;
			tOsdInfo.uwYStart = 0;
			OSD_EraserImg1(&tOsdInfo);
			tUI_CuSetting.IconSts.ubDrawStsIconFlag = FALSE;
			if(FALSE == tUI_CuSetting.IconSts.ubShowLostLogoFlag)
				UI_DrawCUStatusIcon();
			tUI_State = UI_DISPLAY_STATE;
			break;
		}
		case UI_SUBMENU_STATE:
		{
			tUI_State = UI_MAINMENU_STATE;
			UI_ResetSubMenuInfo();
			if(TRUE == ubUI_FastStateFlag)
			{
				OSD_IMG_INFO tOsdInfo;

				ubUI_FastStateFlag = FALSE;
				UI_ClearCamConnectStatusFlag();
				tOsdInfo.uwHSize  = uwOSD_GetHSize();
				tOsdInfo.uwVSize  = uwOSD_GetVSize();
				tOsdInfo.uwXStart = 0;
				tOsdInfo.uwYStart = 0;
				OSD_EraserImg1(&tOsdInfo);
				tUI_State = UI_DISPLAY_STATE;
				break;
			}
			ubUI_RecSubMenuFlag = FALSE;
			UI_DrawMenuPage();
			break;
		}
		case UI_SUBSUBSUBMENU_STATE:
		case UI_CAM_SEL_STATE:
		case UI_SET_VDOMODE_STATE:
		case UI_SET_ADOSRC_STATE:
		case UI_SET_CUPSMODE_STATE:
		case UI_SET_CAMECOMODE_STATE:
		case UI_ENGMODE_STATE:
		case UI_CAMSETTINGMENU_STATE:
		case UI_SET_CAMCOLOR_STATE:
		case UI_DPTZ_CONTROL_STATE:
		case UI_MD_WINDOW_STATE:
		case UI_DUALVIEW_CAMSEL_STATE:
		case UI_SDFWUPG_STATE:
		case UI_RECFILES_SEL_STATE:
		case UI_RECFOLDER_SEL_STATE:
		case UI_RECPLAYLIST_STATE:
		case UI_RECPLAYADOSRC_SEL_STATE:
		case UI_PHOTOPLAYLIST_STATE:
		case UI_SDCARDFMT_STATE:
			if(tUI_StateMap2MenuFunc[tUI_State].pvFuncPtr)
				tUI_StateMap2MenuFunc[tUI_State].pvFuncPtr(EXIT_ARROW);
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_UpArrowKey(void)
{
	switch(tUI_State)
	{
		case UI_DISPLAY_STATE:
		case UI_MAINMENU_STATE:
		case UI_SUBMENU_STATE:
		case UI_SUBSUBMENU_STATE:
		case UI_SUBSUBSUBMENU_STATE:
		case UI_SET_CAMCOLOR_STATE:
		case UI_DPTZ_CONTROL_STATE:
		case UI_MD_WINDOW_STATE:
		case UI_DUALVIEW_CAMSEL_STATE:
		case UI_SET_ADOSRC_STATE:
		case UI_RECFILES_SEL_STATE:
		case UI_RECFOLDER_SEL_STATE:
		case UI_ENGMODE_STATE:
		case UI_SPRF_SEL_STATE:
			if(tUI_StateMap2MenuFunc[tUI_State].pvFuncPtr)
				tUI_StateMap2MenuFunc[tUI_State].pvFuncPtr(UP_ARROW);
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_DownArrowKey(void)
{
	switch(tUI_State)
	{
		case UI_DISPLAY_STATE:
		case UI_MAINMENU_STATE:
		case UI_SUBMENU_STATE:
		case UI_SUBSUBMENU_STATE:
		case UI_SUBSUBSUBMENU_STATE:
		case UI_CAM_SEL_STATE:
		case UI_SET_ADOSRC_STATE:
		case UI_SET_CAMCOLOR_STATE:
		case UI_DPTZ_CONTROL_STATE:
		case UI_MD_WINDOW_STATE:
		case UI_DUALVIEW_CAMSEL_STATE:
		case UI_RECFILES_SEL_STATE:
		case UI_RECFOLDER_SEL_STATE:
		case UI_ENGMODE_STATE:
		case UI_SPRF_SEL_STATE:
			if(tUI_StateMap2MenuFunc[tUI_State].pvFuncPtr)
				tUI_StateMap2MenuFunc[tUI_State].pvFuncPtr(DOWN_ARROW);
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_LeftArrowKey(void)
{
	switch(tUI_State)
	{
		case UI_DISPLAY_STATE:
		case UI_MAINMENU_STATE:
		case UI_SUBMENU_STATE:
		case UI_SUBSUBMENU_STATE:
		case UI_SUBSUBSUBMENU_STATE:
		case UI_CAM_SEL_STATE:
		case UI_SET_VDOMODE_STATE:
		case UI_SET_ADOSRC_STATE:
		case UI_SET_CUPSMODE_STATE:
		case UI_SET_CAMECOMODE_STATE:
		case UI_CAMSETTINGMENU_STATE:
		case UI_SET_CAMCOLOR_STATE:
		case UI_DPTZ_CONTROL_STATE:
		case UI_MD_WINDOW_STATE:
		case UI_SDFWUPG_STATE:
		case UI_RECFOLDER_SEL_STATE:
		case UI_RECFILES_SEL_STATE:
		case UI_RECPLAYLIST_STATE:
		case UI_RECPLAYADOSRC_SEL_STATE:
		case UI_SDCARDFMT_STATE:
			if(tUI_StateMap2MenuFunc[tUI_State].pvFuncPtr)
				tUI_StateMap2MenuFunc[tUI_State].pvFuncPtr(LEFT_ARROW);
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_RightArrowKey(void)
{
	switch(tUI_State)
	{
		case UI_DISPLAY_STATE:
		case UI_MAINMENU_STATE:
		case UI_SUBMENU_STATE:
		case UI_SUBSUBMENU_STATE:
		case UI_SUBSUBSUBMENU_STATE:
		case UI_CAM_SEL_STATE:
		case UI_SET_VDOMODE_STATE:
		case UI_SET_ADOSRC_STATE:
		case UI_SET_CUPSMODE_STATE:
		case UI_SET_CAMECOMODE_STATE:
		case UI_CAMSETTINGMENU_STATE:
		case UI_SET_CAMCOLOR_STATE:
		case UI_DPTZ_CONTROL_STATE:
		case UI_MD_WINDOW_STATE:
		case UI_SDFWUPG_STATE:
		case UI_RECFILES_SEL_STATE:
		case UI_RECFOLDER_SEL_STATE:
		case UI_RECPLAYLIST_STATE:
		case UI_RECPLAYADOSRC_SEL_STATE:
		case UI_SDCARDFMT_STATE:
			if(tUI_StateMap2MenuFunc[tUI_State].pvFuncPtr)
				tUI_StateMap2MenuFunc[tUI_State].pvFuncPtr(RIGHT_ARROW);
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_EnterKey(void)
{
	switch(tUI_State)
	{
		case UI_DISPLAY_STATE:
		case UI_MAINMENU_STATE:
		case UI_SUBMENU_STATE:
		case UI_SUBSUBMENU_STATE:
		case UI_SUBSUBSUBMENU_STATE:
		case UI_CAM_SEL_STATE:
		case UI_SET_VDOMODE_STATE:
		case UI_SET_ADOSRC_STATE:
		case UI_SET_CUPSMODE_STATE:
		case UI_SET_CAMECOMODE_STATE:
		case UI_CAMSETTINGMENU_STATE:
		case UI_DPTZ_CONTROL_STATE:
		case UI_MD_WINDOW_STATE:
		case UI_DUALVIEW_CAMSEL_STATE:
		case UI_SDFWUPG_STATE:
		case UI_RECFILES_SEL_STATE:
		case UI_RECFOLDER_SEL_STATE:
		case UI_RECPLAYLIST_STATE:
		case UI_RECPLAYADOSRC_SEL_STATE:
		case UI_SDCARDFMT_STATE:
		case UI_ENGMODE_STATE:
		case UI_PAIRING_STATE:
		case UI_SPRF_SEL_STATE:
			if(tUI_StateMap2MenuFunc[tUI_State].pvFuncPtr)
				tUI_StateMap2MenuFunc[tUI_State].pvFuncPtr(ENTER_ARROW);
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_ShowColorSettingValue(uint8_t ubValue)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint16_t ubYStart = 0;
	uint8_t ubUnits = 0, ubTens = 0, ubHunds = 0;

	ubHunds = ubValue / 100;
	ubTens  = (ubValue - (ubHunds * 100)) / 10;
	ubUnits = ubValue - (ubHunds * 100 + ubTens * 10);
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_COLORVALUEMASK_ICON, 1, &tOsdImgInfo);
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	if(ubHunds)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_ENG_NUM0+ubTens, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = 628;
		tOsdImgInfo.uwYStart = ubYStart = 530 - tOsdImgInfo.uwVSize;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_ENG_NUM0+ubHunds, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = 628;
		tOsdImgInfo.uwYStart = 530;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_ENG_NUM0+ubUnits, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = 628;
		tOsdImgInfo.uwYStart = ubYStart - tOsdImgInfo.uwVSize;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	}
	else if(ubTens)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_ENG_NUM0+ubTens, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = 628;
		tOsdImgInfo.uwYStart = ubYStart = 543 - tOsdImgInfo.uwVSize;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_ENG_NUM0+ubUnits, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = 628;
		tOsdImgInfo.uwYStart = ubYStart - tOsdImgInfo.uwVSize;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	}
	else
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_ENG_NUM0+ubUnits, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = 628;
		tOsdImgInfo.uwYStart = 530 - tOsdImgInfo.uwVSize;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	}
}
//------------------------------------------------------------------------------
void UI_CameraSettingMenu1Key(void)
{
	if(UI_DISPLAY_STATE != tUI_State)
	{
		UI_MenuKey();
		return;
	}
#ifdef RTC676x
	if(DISPLAY_1T1R != tUI_CuSetting.ubTotalCamNum)
#else
	if((DISPLAY_1T1R != tUI_CuSetting.ubTotalCamNum) && (APP_LINK_STATE == tUI_SyncAppState))
#endif
	{
//		tCamPreViewSel.tCamViewType = tCamViewSel.tCamViewType;
		UI_CameraSelectionKey();
		return;
	}
#if APP_FS_FILE_LIST_STYLE
	KNL_ThmShowInfo.ubInFldListFlg = 1;
#endif	
	UI_DrawDCIMFolderMenu();
}
//------------------------------------------------------------------------------
void UI_CameraSettingMenu2Key(void)
{
	if((APP_LOSTLINK_STATE == tUI_SyncAppState) ||
	   (SINGLE_VIEW != tCamViewSel.tCamViewType) ||
	   (UI_DISPLAY_STATE != tUI_State))
		return;

	UI_DrawCameraSettingMenu(UI_CAMFUNC_SETUP);
}
//------------------------------------------------------------------------------
void UI_DrawColorSettingMenu(void)
{
	OSD_IMG_INFO tOsdImgInfo;

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_COLORSET_BG, 1, &tOsdImgInfo);
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_COLORRGB_ICON, 1, &tOsdImgInfo);
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_COLORBL_ITEM, 1, &tOsdImgInfo);
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_COLORSETUPNOR_ICON, 1, &tOsdImgInfo);
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_COLORSETDNNOR_ICON, 1, &tOsdImgInfo);
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	UI_ShowColorSettingValue(tUI_CamStatus[tCamViewSel.tCamViewPool[0]].tCamColorParam.ubColorBL);
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_COLORRIGHT_ICON, 1, &tOsdImgInfo);
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}
//------------------------------------------------------------------------------
void UI_DrawMDSettingScreen(void)
{
#define MD_H_WINDOWSIZE		48
#define MD_V_WINDOWSIZE		64
	OSD_IMG_INFO tOsdImgInfo[3];
	uint32_t ulLcd_HSize  = uwLCD_GetLcdHoSize();
	uint32_t ulLcd_VSize  = uwLCD_GetLcdVoSize();
	uint8_t ubMD_V_WinNum = ulLcd_VSize / MD_V_WINDOWSIZE;
	uint8_t ubMD_H_WinNum = ulLcd_HSize / MD_H_WINDOWSIZE;
	uint8_t ubMD_Idx;

	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_MDROWLINE, 2, &tOsdImgInfo[0]);
	for(ubMD_Idx = 0; ubMD_Idx < ubMD_H_WinNum; ubMD_Idx++)
	{
		tOsdImgInfo[0].uwXStart = (ubMD_Idx * MD_H_WINDOWSIZE);
		tOSD_Img1(&tOsdImgInfo[0], OSD_QUEUE);
	}
	tOsdImgInfo[0].uwXStart = ulLcd_HSize - 5;
	tOSD_Img1(&tOsdImgInfo[0], OSD_QUEUE);
	for(ubMD_Idx = 0; ubMD_Idx < ubMD_V_WinNum; ubMD_Idx++)
	{
		tOsdImgInfo[1].uwYStart = (ubMD_Idx * MD_V_WINDOWSIZE);
		tOSD_Img1(&tOsdImgInfo[1], OSD_QUEUE);
	}
	tOsdImgInfo[1].uwYStart = ulLcd_VSize - 5;
	tOSD_Img1(&tOsdImgInfo[1], OSD_QUEUE);
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MDBLOCK_ICON, 1, &tOsdImgInfo[2]);
	tOSD_Img2(&tOsdImgInfo[2], OSD_UPDATE);
	OSD_Weight(OSD_WEIGHT_6DIV8);
}
//------------------------------------------------------------------------------
void UI_DrawCameraSettingMenu(UI_CameraSettingMenu_t tCamSetMenu)
{
	OSD_IMG_INFO tOsdImgInfo;

	switch(tCamSetMenu)
	{
		case UI_CAMISP_SETUP:
			tUI_State = UI_SUBMENU_STATE;
//			tOsdImgInfo.uwXStart = 0;
//			tOsdImgInfo.uwYStart = 0;
//			tOsdImgInfo.uwHSize  = 100;
//			tOsdImgInfo.uwVSize  = uwLCD_GetLcdVoSize();
//			OSD_EraserImg1(&tOsdImgInfo);
			OSD_Weight(OSD_WEIGHT_7DIV8);
			tUI_MenuItem.ubItemIdx = CAMERAS_ITEM;
			UI_DrawSubMenuPage(CAMERAS_ITEM);
			ubUI_FastStateFlag = TRUE;
			break;
		case UI_CAMFUNC_SETUP:
		{
			OSD_IMG_INFO tCamSetOsdImgInfo[6];

			tUI_State = UI_CAMSETTINGMENU_STATE;
			OSD_Weight(OSD_WEIGHT_8DIV8);
			tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_SUBMENU, 1, &tOsdImgInfo);
			tOSD_Img1(&tOsdImgInfo, OSD_QUEUE);
			if(tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_CAMSCOLORMENUNOR_ITEM, 6, &tCamSetOsdImgInfo[0]) != OSD_OK)
			{
				printd(DBG_ErrorLvl, "Load OSD Image FAIL, pls check (%d) !\n", __LINE__);
				return;
			}
			tOSD_Img2(&tCamSetOsdImgInfo[1], OSD_QUEUE);
			tOSD_Img2(&tCamSetOsdImgInfo[2], OSD_QUEUE);
			tOSD_Img2(&tCamSetOsdImgInfo[4], OSD_QUEUE);
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_CAMS_SUBMENUICON, 1, &tOsdImgInfo);
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			break;
		}
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_CameraSettingMenu(UI_ArrowKey_t tArrowKey)
{
	static UI_CameraSettingItem_t tUI_CamSetItem = UI_COLOR_ITEM;
	UI_CameraSettingItem_t tUI_PrevCamSetItem = UI_DPTZ_ITEM;
	OSD_IMG_INFO tOsdImgInfo;
	uint16_t uwOsdImgIdx[3] = {OSD2IMG_CAMSCOLORMENUNOR_ITEM, OSD2IMG_CAMSDPTZMENUNOR_ITEM, OSD2IMG_CAMSMDV2MENUNOR_ITEM};

	switch(tArrowKey)
	{
		case LEFT_ARROW:
			if(UI_COLOR_ITEM == tUI_CamSetItem)
				return;
			tUI_PrevCamSetItem = tUI_CamSetItem;
			--tUI_CamSetItem;
			break;
		case RIGHT_ARROW:
			if(UI_MD_ITEM == tUI_CamSetItem)
				return;
			tUI_PrevCamSetItem = tUI_CamSetItem;
			++tUI_CamSetItem;
			break;
		case ENTER_ARROW:
		case EXIT_ARROW:
			UI_ClearCamConnectStatusFlag();
			tOsdImgInfo.uwHSize  = uwOSD_GetHSize();
			tOsdImgInfo.uwVSize  = uwOSD_GetVSize();
			tOsdImgInfo.uwXStart = 0;
			tOsdImgInfo.uwYStart = 0;
			OSD_EraserImg1(&tOsdImgInfo);
			if(ENTER_ARROW == tArrowKey)
			{
				if(UI_COLOR_ITEM == tUI_CamSetItem)
				{
					UI_DrawColorSettingMenu();
					tUI_State = UI_SET_CAMCOLOR_STATE;
					return;
				}
				if(UI_DPTZ_ITEM == tUI_CamSetItem)
				{
					uint32_t ulLcd_HSize = uwLCD_GetLcdHoSize();
					uint32_t ulLcd_VSize = uwLCD_GetLcdVoSize();
					uint8_t ubArrowNum;

					tUI_DptzParam.tScaleParam							 = UI_SCALEUP_2X;
					tUI_DptzParam.tUI_LcdCropParam.tChRes.uwChInputHsize = ulLcd_HSize;
					tUI_DptzParam.tUI_LcdCropParam.tChRes.uwChInputVsize = ulLcd_VSize;
					tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHsize    = ulLcd_HSize/tUI_DptzParam.tScaleParam;
					tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVsize    = ulLcd_VSize/tUI_DptzParam.tScaleParam;
					tUI_DptzParam.tUI_LcdCropParam.uwLcdOutputHsize	   	 = ulLcd_HSize;
					tUI_DptzParam.tUI_LcdCropParam.uwLcdOutputVsize	   	 = ulLcd_VSize;
					tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHstart   = (ulLcd_HSize - tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHsize)/2;
					tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVstart   = (ulLcd_VSize - tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVsize)/2;
					tLCD_DynamicOneChCropScale(&tUI_DptzParam.tUI_LcdCropParam);
					for(ubArrowNum = 0; ubArrowNum < 4; ubArrowNum++)
					{
						tOSD_GetOsdImgInfor(1, OSD_IMG2, (OSD2IMG_DPTZUPARROWNOR_ICON+(ubArrowNum*2)), 1, &tOsdImgInfo);
						tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
					}
					tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DPTZZOOMMSG_ICON, 1, &tOsdImgInfo);
					tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
					tUI_CamSetItem = UI_COLOR_ITEM;
					tUI_State = UI_DPTZ_CONTROL_STATE;
					return;
				}
				if(UI_MD_ITEM == tUI_CamSetItem)
				{
					UI_DrawMDSettingScreen();
					tUI_CamSetItem = UI_COLOR_ITEM;
					tUI_State = UI_MD_WINDOW_STATE;
					return;
				}
			}
			tUI_CamSetItem = UI_COLOR_ITEM;
			tUI_State = UI_DISPLAY_STATE;
			return;
		default:
			return;
	}
	tOSD_GetOsdImgInfor(1, OSD_IMG2, (uwOsdImgIdx[tUI_CamSetItem]+UI_ICON_HIGHLIGHT), 1, &tOsdImgInfo);
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	tOSD_GetOsdImgInfor(1, OSD_IMG2, uwOsdImgIdx[tUI_PrevCamSetItem], 1, &tOsdImgInfo);
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}
//------------------------------------------------------------------------------
void UI_CameraColorSetting(UI_ArrowKey_t tArrowKey)
{
	static uint8_t ubUI_ColorSetItem = UI_IMGBL_SETTING;
	OSD_IMG_INFO tOsdImgInfo;
	UI_CUReqCmd_t tCamSetColorCmd;
	uint8_t *pUI_ColorParm[4] = {(uint8_t *)&tUI_CamStatus[tCamViewSel.tCamViewPool[0]].tCamColorParam.ubColorBL,
								 (uint8_t *)&tUI_CamStatus[tCamViewSel.tCamViewPool[0]].tCamColorParam.ubColorContrast,
								 (uint8_t *)&tUI_CamStatus[tCamViewSel.tCamViewPool[0]].tCamColorParam.ubColorSaturation,
								 (uint8_t *)&tUI_CamStatus[tCamViewSel.tCamViewPool[0]].tCamColorParam.ubColorHue};

	tCamSetColorCmd.tDS_CamNum 				= tCamViewSel.tCamViewPool[0];
	tCamSetColorCmd.ubCmd[UI_TWC_TYPE]		= UI_SETTING;
	tCamSetColorCmd.ubCmd[UI_SETTING_ITEM]  = UI_IMGPROC_SETTING;
	switch(tArrowKey)
	{
		case LEFT_ARROW:
			if(UI_IMGBL_SETTING == ubUI_ColorSetItem)
				return;
			if(UI_IMGHUE_SETTING == ubUI_ColorSetItem)
			{
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_COLORRIGHTMASK_ICON, 1, &tOsdImgInfo);
				tOsdImgInfo.uwYStart = 1181;
				tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_COLORRIGHT_ICON, 1, &tOsdImgInfo);
				tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
			}
			--ubUI_ColorSetItem;
			break;
		case RIGHT_ARROW:
			if(UI_IMGHUE_SETTING == ubUI_ColorSetItem)
				return;
			if(++ubUI_ColorSetItem == UI_IMGHUE_SETTING)
			{
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_COLORRIGHTMASK_ICON, 1, &tOsdImgInfo);
				tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_COLORLEFT_ICON, 1, &tOsdImgInfo);
				tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
			}
			break;
		case DOWN_ARROW:
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_COLORSETDNHL_ICON, 1, &tOsdImgInfo);
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			if(*pUI_ColorParm[ubUI_ColorSetItem-UI_IMGBL_SETTING] > 0)
			{
				tCamSetColorCmd.ubCmd[UI_SETTING_DATA]   = ubUI_ColorSetItem;
				tCamSetColorCmd.ubCmd[UI_SETTING_DATA+1] = --(*pUI_ColorParm[ubUI_ColorSetItem-UI_IMGBL_SETTING]);
				tCamSetColorCmd.ubCmd_Len				 = 4;
				if(UI_SendRequestToCAM(osThreadGetId(), &tCamSetColorCmd) == rUI_SUCCESS)
				{
					UI_ShowColorSettingValue(*pUI_ColorParm[ubUI_ColorSetItem-UI_IMGBL_SETTING]);
					UI_UpdateDevStatusInfo();
				}
				else
					++(*pUI_ColorParm[ubUI_ColorSetItem-UI_IMGBL_SETTING]);
			}
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_COLORSETDNNOR_ICON, 1, &tOsdImgInfo);
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			return;
		case UP_ARROW:
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_COLORSETUPHL_ICON, 1, &tOsdImgInfo);
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			if(*pUI_ColorParm[ubUI_ColorSetItem-UI_IMGBL_SETTING] < 127)
			{
				tCamSetColorCmd.ubCmd[UI_SETTING_DATA]   = ubUI_ColorSetItem;
				tCamSetColorCmd.ubCmd[UI_SETTING_DATA+1] = ++(*pUI_ColorParm[ubUI_ColorSetItem-UI_IMGBL_SETTING]);
				tCamSetColorCmd.ubCmd_Len				 = 4;
				if(UI_SendRequestToCAM(osThreadGetId(), &tCamSetColorCmd) == rUI_SUCCESS)
				{
					UI_ShowColorSettingValue(*pUI_ColorParm[ubUI_ColorSetItem-UI_IMGBL_SETTING]);
					UI_UpdateDevStatusInfo();
				}
				else
					--(*pUI_ColorParm[ubUI_ColorSetItem-UI_IMGBL_SETTING]);
			}
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_COLORSETUPNOR_ICON, 1, &tOsdImgInfo);
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			return;
		case EXIT_ARROW:
			UI_ClearCamConnectStatusFlag();
			tOsdImgInfo.uwXStart = 580;
			tOsdImgInfo.uwYStart = 0;
			tOsdImgInfo.uwHSize  = 140;
			tOsdImgInfo.uwVSize  = uwLCD_GetLcdVoSize();
			OSD_EraserImg2(&tOsdImgInfo);
			ubUI_ColorSetItem = UI_IMGBL_SETTING;
			tUI_State = UI_DISPLAY_STATE;
			return;
		default:
			return;
	}
	UI_ShowColorSettingValue(*pUI_ColorParm[ubUI_ColorSetItem-UI_IMGBL_SETTING]);
	tOSD_GetOsdImgInfor(1, OSD_IMG2, (OSD2IMG_COLORBL_ITEM + (ubUI_ColorSetItem - UI_IMGBL_SETTING)), 1, &tOsdImgInfo);
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}
//------------------------------------------------------------------------------
UI_Result_t UI_DPTZ_KeyPress(uint8_t ubKeyID, uint8_t ubKeyMapIdx)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint16_t uwUI_ArrowOsdImgIdx[] = {[AKEY_UP]    = OSD2IMG_DPTZUPARROWHL_ICON,
							          [AKEY_DOWN]  = OSD2IMG_DPTZDNARROWHL_ICON,
									  [AKEY_LEFT]  = OSD2IMG_DPTZLEFTARROWHL_ICON,
		                              [AKEY_RIGHT] = OSD2IMG_DPTZRIGHTARROWHL_ICON};

	if(ubKeyID == UiKeyEventMap[ubKeyMapIdx].ubKeyID)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, uwUI_ArrowOsdImgIdx[ubKeyID], 1, &tOsdImgInfo);
		tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
		return rUI_SUCCESS;
	}
	return rUI_FAIL;
}
//------------------------------------------------------------------------------
void UI_DPTZ_KeyRelease(uint8_t ubKeyID)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint16_t uwUI_ArrowOsdImgIdx[] = {[AKEY_UP]    = OSD2IMG_DPTZUPARROWNOR_ICON,
							          [AKEY_DOWN]  = OSD2IMG_DPTZDNARROWNOR_ICON,
									  [AKEY_LEFT]  = OSD2IMG_DPTZLEFTARROWNOR_ICON,
		                              [AKEY_RIGHT] = OSD2IMG_DPTZRIGHTARROWNOR_ICON};

	tOSD_GetOsdImgInfor(1, OSD_IMG2, uwUI_ArrowOsdImgIdx[ubKeyID], 1, &tOsdImgInfo);
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}
//------------------------------------------------------------------------------
void UI_DPTZ_Control(UI_ArrowKey_t tArrowKey)
{
#define PT_STEP		10
	OSD_IMG_INFO tOsdImgInfo;
	uint8_t ubArrowNum;

	switch(tArrowKey)
	{
		case UP_ARROW:
			if(tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHstart == 0)
				return;
			tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHstart -= PT_STEP;
			break;
		case DOWN_ARROW:
			if((tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHstart + 
			    tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHsize) >= tUI_DptzParam.tUI_LcdCropParam.uwLcdOutputHsize)
				return;
			tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHstart += PT_STEP;
			break;
		case LEFT_ARROW:
			if((tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVstart + 
			    tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVsize) >= tUI_DptzParam.tUI_LcdCropParam.uwLcdOutputVsize)
				return;
			tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVstart += PT_STEP;
			break;
		case RIGHT_ARROW:
			if(tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVstart == 0)
				return;
			tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVstart -= PT_STEP;
			break;
		case ENTER_ARROW:
			if(tUI_DptzParam.tScaleParam == UI_SCALEUP_2X)
			{
				tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHsize /= UI_SCALEUP_2X;
				tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVsize /= UI_SCALEUP_2X;
				tUI_DptzParam.tScaleParam = UI_SCALEUP_4X;
			}
			else if(tUI_DptzParam.tScaleParam == UI_SCALEUP_4X)
			{
				uint16_t uwCropHsize     = tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHsize*UI_SCALEUP_2X;
				uint16_t uwCropVsize     = tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVsize*UI_SCALEUP_2X;
				uint16_t uwPrevCropHsize = tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHsize;
				uint16_t uwPrevCropVsize = tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVsize;
				if(tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHstart < ((uwCropHsize - uwPrevCropHsize)/2))
					tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHstart = 0;
				if((tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVstart + ((uwCropVsize - uwPrevCropVsize)/2)) >= uwCropVsize)
					tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVstart = uwCropVsize;
				tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHsize = uwCropHsize;
				tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVsize = uwCropVsize;
				tUI_DptzParam.tScaleParam = UI_SCALEUP_2X;
			}
			break;
		case EXIT_ARROW:
		{
			UI_ClearStatusBarOsdIcon();
			for(ubArrowNum = 0; ubArrowNum < 4; ubArrowNum++)
			{
				if(ubArrowNum < 2)
				{
					tOsdImgInfo.uwXStart = 300;
					tOsdImgInfo.uwYStart = 40 + (ubArrowNum * 1080);
				}
				else
				{
					tOsdImgInfo.uwXStart = 40 + ((ubArrowNum - 2) * 570);
					tOsdImgInfo.uwYStart = 590;
				}
				tOsdImgInfo.uwHSize  = 100;
				tOsdImgInfo.uwVSize  = 110;
				OSD_EraserImg2(&tOsdImgInfo);
			}
			tOsdImgInfo.uwXStart = 0;
			tOsdImgInfo.uwYStart = 20;
			tOsdImgInfo.uwHSize  = 50;
			tOsdImgInfo.uwVSize  = 125;
			OSD_EraserImg2(&tOsdImgInfo);
			tUI_DptzParam.tScaleParam						   = UI_SCALEUP_2X;
			tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHstart = 0;
			tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVstart = 0;
			tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropHsize  = uwLCD_GetLcdHoSize();
			tUI_DptzParam.tUI_LcdCropParam.tChRes.uwCropVsize  = uwLCD_GetLcdVoSize();
			tLCD_DynamicOneChCropScale(&tUI_DptzParam.tUI_LcdCropParam);
			tUI_State = UI_DISPLAY_STATE;
			return;
		}
		default:
			return;
	}
	tLCD_DynamicOneChCropScale(&tUI_DptzParam.tUI_LcdCropParam);
}
//------------------------------------------------------------------------------
void UI_MD_Window(UI_ArrowKey_t tArrowKey)
{
	uint32_t ulLcd_HSize = uwLCD_GetLcdHoSize();
	uint32_t ulLcd_VSize = uwLCD_GetLcdVoSize();
	uint8_t ubMD_V_WinNum = ulLcd_VSize / MD_V_WINDOWSIZE;
	uint8_t ubMD_H_WinNum = ulLcd_HSize / MD_H_WINDOWSIZE;
	static uint16_t uwMD_StartIdx = 0;
	static uint8_t ubMD_DownIdx = 0, ubMD_RightIdx = 0, ubMD_RightCnt = 0, ubMD_DownCnt = 0;
	static uint8_t ubMD_1stFlag = FALSE, ubMD_2ndFlag = FALSE;
	uint8_t ubMD_PrevDownIdx, ubMD_PrevRightIdx, i;
	OSD_IMG_INFO tOsdImgInfo[2];

	ubMD_PrevDownIdx  = ubMD_DownIdx;
	ubMD_PrevRightIdx = ubMD_RightIdx;
	switch(tArrowKey)
	{
		case UP_ARROW:
			if(((TRUE == ubMD_1stFlag) && (FALSE == ubMD_2ndFlag)) ||
			   ((TRUE == ubMD_2ndFlag) && !ubMD_DownCnt))
				return;
			if(ubMD_DownIdx == 0)
				return;
			--ubMD_DownIdx;
			if(TRUE == ubMD_2ndFlag)
				--ubMD_DownCnt;
			break;
		case DOWN_ARROW:
			if((TRUE == ubMD_1stFlag) && (FALSE == ubMD_2ndFlag))
				return;
			if((ubMD_DownIdx + 1) == ubMD_H_WinNum)
				return;
			++ubMD_DownIdx;
			if(TRUE == ubMD_2ndFlag)
				++ubMD_DownCnt;
			break;
		case LEFT_ARROW:
			if(((TRUE == ubMD_1stFlag) && !ubMD_RightCnt) || (TRUE == ubMD_2ndFlag))
				return;
			if(ubMD_RightIdx == 0)
				return;
			--ubMD_RightIdx;
			if(TRUE == ubMD_1stFlag)
				--ubMD_RightCnt;
			break;
		case RIGHT_ARROW:
			if(((ubMD_RightIdx + 1) == ubMD_V_WinNum) || (TRUE == ubMD_2ndFlag))
				return;
			++ubMD_RightIdx;
			if(TRUE == ubMD_1stFlag)
				++ubMD_RightCnt;
			break;
		case ENTER_ARROW:
			if(TRUE == ubMD_2ndFlag)
			{
				UI_CUReqCmd_t tMdCmd;

				tMdCmd.tDS_CamNum 				= tCamViewSel.tCamViewPool[0];
				tMdCmd.ubCmd[UI_TWC_TYPE]		= UI_SETTING;
				tMdCmd.ubCmd[UI_SETTING_ITEM]   = UI_MD_SETTING;
				tMdCmd.ubCmd[UI_SETTING_DATA]   = (uwMD_StartIdx & 0xFF);
				tMdCmd.ubCmd[UI_SETTING_DATA+1] = (uwMD_StartIdx >> 8);
				tMdCmd.ubCmd[UI_SETTING_DATA+2] = ubMD_RightCnt;
				tMdCmd.ubCmd[UI_SETTING_DATA+3] = ubMD_DownCnt;
				tMdCmd.ubCmd_Len  				= 6;
				if(UI_SendRequestToCAM(osThreadGetId(), &tMdCmd) != rUI_SUCCESS)
				{
					printd(DBG_ErrorLvl, "MD Setting Fail !\n");
					return;
				}
			}
			else
			{
				if(FALSE == ubMD_1stFlag)
				{
					uwMD_StartIdx = ((ubMD_DownIdx * ubMD_V_WinNum) + ubMD_RightIdx);
					ubMD_1stFlag  = TRUE;
				}
				else if(FALSE == ubMD_2ndFlag)
				{
					ubMD_2ndFlag  = TRUE;
				}
				return;
			}
		case EXIT_ARROW:
			tUI_CuSetting.IconSts.ubDrawStsIconFlag = FALSE;
			uwMD_StartIdx = ubMD_DownIdx  = ubMD_RightIdx = 0;
			ubMD_RightCnt = ubMD_DownCnt = 0;
			ubMD_1stFlag  = ubMD_2ndFlag = FALSE;
			tOsdImgInfo[1].uwXStart = 0;
			tOsdImgInfo[1].uwYStart = 0;
			tOsdImgInfo[1].uwHSize  = ulLcd_HSize;
			tOsdImgInfo[1].uwVSize  = ulLcd_VSize;
			OSD_EraserImg1(&tOsdImgInfo[1]);
			tUI_State = UI_DISPLAY_STATE;
			return;
		default:
			return;
	}
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MDBLOCK_ICON, 1, &tOsdImgInfo[0]);
	if(TRUE == ubMD_2ndFlag)
	{
		uint16_t uwTmpYStart = 0;

		if(UP_ARROW == tArrowKey)
		{
			tOsdImgInfo[1].uwHSize  = tOsdImgInfo[0].uwHSize;
			tOsdImgInfo[1].uwVSize  = tOsdImgInfo[0].uwVSize;
			tOsdImgInfo[1].uwXStart = tOsdImgInfo[0].uwXStart + (ubMD_PrevDownIdx * MD_H_WINDOWSIZE);
			for(i = 0; i <= ubMD_RightCnt; i++)
			{
				tOsdImgInfo[1].uwYStart = tOsdImgInfo[0].uwYStart - ((ubMD_PrevRightIdx - i) * MD_V_WINDOWSIZE);
				OSD_EraserImg2(&tOsdImgInfo[1]);
			}
		}
		tOsdImgInfo[0].uwXStart += (ubMD_DownIdx * MD_H_WINDOWSIZE);
		for(i = 0; i <= ubMD_RightCnt; i++)
		{
			uwTmpYStart = tOsdImgInfo[0].uwYStart;
			tOsdImgInfo[0].uwYStart -= ((ubMD_RightIdx - i) * MD_V_WINDOWSIZE);
			tOSD_Img2(&tOsdImgInfo[0], (i == ubMD_RightCnt)?OSD_UPDATE:OSD_QUEUE);
			tOsdImgInfo[0].uwYStart = uwTmpYStart;
		}
		return;
	}
	if((FALSE == ubMD_1stFlag) || (LEFT_ARROW == tArrowKey))
	{
		tOsdImgInfo[1].uwXStart = tOsdImgInfo[0].uwXStart + (ubMD_PrevDownIdx * MD_H_WINDOWSIZE);
		tOsdImgInfo[1].uwYStart = tOsdImgInfo[0].uwYStart - (ubMD_PrevRightIdx * MD_V_WINDOWSIZE);
		tOsdImgInfo[1].uwHSize  = tOsdImgInfo[0].uwHSize;
		tOsdImgInfo[1].uwVSize  = tOsdImgInfo[0].uwVSize;
		OSD_EraserImg2(&tOsdImgInfo[1]);
	}
	tOsdImgInfo[0].uwXStart += (ubMD_DownIdx * MD_H_WINDOWSIZE);
	tOsdImgInfo[0].uwYStart -= (ubMD_RightIdx * MD_V_WINDOWSIZE);
	tOSD_Img2(&tOsdImgInfo[0], OSD_UPDATE);
}
//------------------------------------------------------------------------------
void UI_CameraSelectionKey(void)
{
	OSD_IMG_INFO tFixLocateImgInfo;	
	OSD_IMG_INFO tOsdImgInfo[(OSD2IMG_SELCAM2TDISABLE_ICON-OSD2IMG_SELCAM1ONLINE_ICON)+1] = {0};
	uint16_t uwDisplayImgIdx = 0, uwStartIdx;
	uint16_t uwXOffset  = 0;
	uint16_t uwYOffset  = (DISPLAY_2T1R == tUI_CuSetting.ubTotalCamNum)?230:0;
	uint8_t ubSelItem;
	UI_CamNum_t tCamNum;
	static uint8_t ubUI_UpdateCamSelFlag = FALSE;

	if(tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SELCAM1ONLINE_ICON, (OSD2IMG_SELCAM2TDISABLE_ICON-OSD2IMG_SELCAM1ONLINE_ICON)+1, &tOsdImgInfo[0]) != OSD_OK)
	{
		printd(DBG_ErrorLvl, "Load OSD Image FAIL, pls check (%d) !\n", __LINE__);
		return;
	}
	if(FALSE == ubUI_UpdateCamSelFlag)
	{
		tUI_CamNumSel = (DISPLAY_4T1R == tUI_CuSetting.ubTotalCamNum)?(UI_CamNum_t)((VDO_DISP_TYPE == KNL_DISP_H)?H_TYPE_ITEM:QUAD_TYPE_ITEM):(UI_CamNum_t)tUI_CuSetting.ubTotalCamNum;
		ubUI_UpdateCamSelFlag = TRUE;
	}
	for(tCamNum = CAM1; tCamNum < tUI_CuSetting.ubTotalCamNum; tCamNum++)
	{
	#ifdef RTC676x
		uwDisplayImgIdx = (((tCamNum == CAM4)?(tCamNum+1):tCamNum)*2) +
                          ((tUI_CamStatus[tCamNum].ulCAM_ID == INVALID_ID)?1:0);
	#else
		uwDisplayImgIdx = (((tCamNum == CAM4)?(tCamNum+1):tCamNum)*2) +
                          (((tUI_CamStatus[tCamNum].ulCAM_ID == INVALID_ID) || (tUI_CamStatus[tCamNum].tCamConnSts == CAM_OFFLINE))?1:0);
	#endif            
		tOsdImgInfo[uwDisplayImgIdx].uwYStart -= uwYOffset;
		tOSD_Img2(&tOsdImgInfo[uwDisplayImgIdx], OSD_QUEUE);		
	}
	if (DISPLAY_4T1R == tUI_CuSetting.ubTotalCamNum) {
		OSD_IMG_INFO tOsdImg4TInfo[4];

		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SEL4TDUALONLINE_ICON, 4, &tOsdImg4TInfo[0]);
		uwStartIdx = (VDO_DISP_TYPE == KNL_DISP_H)?OSD2IMG_SELCAM4T_H_ENABLE_ICON:OSD2IMG_SELCAM4TENABLE_ICON;
		uwDisplayImgIdx = (uwStartIdx - OSD2IMG_SELCAM1ONLINE_ICON);
		
		tOSD_Img2(&tOsdImgInfo[uwDisplayImgIdx], OSD_QUEUE);
		tOSD_Img2(&tOsdImg4TInfo[((tUI_CuSetting.ubPairedCamNum >= 1)?0:1)], OSD_QUEUE);
		tOSD_Img2(&tOsdImg4TInfo[((tUI_CuSetting.ubPairedCamNum > 1)?2:3)], OSD_QUEUE);
	
		if(VDO_DISP_TYPE == KNL_DISP_3T_3C)		
		{			
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SELCAMTRIPTENABLE_ICON, 1, &tFixLocateImgInfo);			
			tOSD_Img2(&tFixLocateImgInfo, OSD_QUEUE);
		}
		else
		{
			tOSD_Img2(&tOsdImgInfo[uwDisplayImgIdx], OSD_QUEUE);
		}
		
		
		tOSD_Img2(&tOsdImg4TInfo[((tUI_CuSetting.ubPairedCamNum >= 1)?0:1)], OSD_QUEUE);
		tOSD_Img2(&tOsdImg4TInfo[((tUI_CuSetting.ubPairedCamNum > 1)?2:3)], OSD_QUEUE);
		
		
		ubSelItem = (tCamViewSel.tCamViewType == H_VIEW)?H_TYPE_ITEM:
					(tCamViewSel.tCamViewType == QUAD_VIEW)?QUAD_TYPE_ITEM:
		            (tCamViewSel.tCamViewType == DUAL_VIEW)?DUAL_TYPE_ITEM:
					(tCamViewSel.tCamViewType == SCAN_VIEW)?SCAN_TYPE_ITEM:tCamViewSel.tCamViewPool[0];
		tUI_CamNumSel = (UI_CamNum_t)ubSelItem;
	} else if(DISPLAY_2T1R == tUI_CuSetting.ubTotalCamNum) {
		uwStartIdx = OSD2IMG_SELCAM2TENABLE_ICON;
		uwDisplayImgIdx = (uwStartIdx - OSD2IMG_SELCAM1ONLINE_ICON);
		tOsdImgInfo[uwDisplayImgIdx].uwYStart -= 80;
		tOSD_Img2(&tOsdImgInfo[uwDisplayImgIdx], OSD_QUEUE);
		ubSelItem = (tCamViewSel.tCamViewType == DUAL_VIEW)?CAM_2T:tCamViewSel.tCamViewPool[0];
		tUI_CamNumSel = (UI_CamNum_t)ubSelItem;
	} else {
		ubSelItem = tCamViewSel.tCamViewPool[0];
	}
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SELCAMHL_ICON, 1, &tOsdImgInfo[0]);
	tOsdImgInfo[0].uwXStart += uwXOffset;
	tOsdImgInfo[0].uwYStart -= ((ubSelItem*111) + uwYOffset);
	tOSD_Img2(&tOsdImgInfo[0], OSD_UPDATE);
	tUI_State = UI_CAM_SEL_STATE;
	if(UI_REC_START == tUI_RecPlayAct.tRecAct)
	{
		OSD_IMG_INFO tRecOsdImgInfo[3];

		osDelay(20);
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_RECORDINGNACT_ICON, 3, &tRecOsdImgInfo);
		tOSD_Img2(&tRecOsdImgInfo[1], OSD_QUEUE);
		tOSD_Img2(&tRecOsdImgInfo[2], OSD_UPDATE);
	}
}
//------------------------------------------------------------------------------
void UI_CameraSelection(UI_ArrowKey_t tArrowKey)
{
	UI_CamNum_t tPreCamNum = tUI_CamNumSel;
	OSD_IMG_INFO tOsdImgInfo;
	uint16_t uwXOffset  = 0;
	uint16_t uwYOffset  = (DISPLAY_2T1R == tUI_CuSetting.ubTotalCamNum)?230:0;
	UI_Result_t tUI_ChkResult = rUI_SUCCESS;
	UI_CamViewType_t tCamViewTypeSel;
	uint8_t ubWarnNoteShowFlag = FALSE;

	if(DISPLAY_1T1R == tUI_CuSetting.ubTotalCamNum)
		return;

	tPreCamNum = tUI_CamNumSel;
	switch(tArrowKey)
	{
		case LEFT_ARROW:
		case RIGHT_ARROW:
			tUI_CamNumSel = UI_ChangeSelectCamNum4UiMenu(&tPreCamNum, &tArrowKey);
			if(tUI_CamNumSel == NO_CAM)
			{
				tUI_CamNumSel = tPreCamNum;
				break;
			}
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SELCAMNOR_ICON, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart += uwXOffset;
			tOsdImgInfo.uwYStart -= ((tPreCamNum*111) + uwYOffset);
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SELCAMHL_ICON, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart += uwXOffset;
			tOsdImgInfo.uwYStart -= ((tUI_CamNumSel*111) + uwYOffset);
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			break;
		case DOWN_ARROW:
#if APP_FS_FILE_LIST_STYLE
			KNL_ThmShowInfo.ubInFldListFlg = 1;
#endif		
			if((SINGLE_VIEW == tCamViewSel.tCamViewType) || (SCAN_VIEW == tCamViewSel.tCamViewType))
			{
				UI_DrawDCIMFolderMenu();
			}
			else
			{
				tOsdImgInfo.uwXStart  = 100;
				tOsdImgInfo.uwYStart  = (DISPLAY_2T1R == tUI_CuSetting.ubTotalCamNum)?400:0;
				tOsdImgInfo.uwHSize   = (DISPLAY_2T1R == tUI_CuSetting.ubTotalCamNum)?300:255;
				tOsdImgInfo.uwVSize   = (DISPLAY_2T1R == tUI_CuSetting.ubTotalCamNum)?500:uwLCD_GetLcdVoSize();
				OSD_EraserImg2(&tOsdImgInfo);
				UI_ChangeAudioSourceKey();
			}
			break;
		case ENTER_ARROW:
			tCamViewTypeSel = (DISPLAY_4T1R == tUI_CuSetting.ubTotalCamNum)?((tUI_CamNumSel == H_TYPE_ITEM) && (VDO_DISP_TYPE == KNL_DISP_H))?H_VIEW:
																		    ( tUI_CamNumSel == QUAD_TYPE_ITEM)?QUAD_VIEW:
																	        ( tUI_CamNumSel == DUAL_TYPE_ITEM)?DUAL_VIEW:/*tCamPreViewSel.tCamViewType:*/
																		    ( tUI_CamNumSel == SCAN_TYPE_ITEM)?SCAN_VIEW:SINGLE_VIEW:
							  (DISPLAY_2T1R == tUI_CuSetting.ubTotalCamNum)?( tUI_CamNumSel == CAM_2T)?DUAL_VIEW:SINGLE_VIEW:SINGLE_VIEW;
				
		//#if (APP_DUAL_HOST_ENABLE == 1)	//Support "SINGLE_VIEW/SCAN_VIEW/DUAL_VIEW/V3_3T_VIEW" View Type
		#if ((APP_DUAL_HOST_ENABLE == 1) && (VDO_DISP_TYPE == KNL_DISP_3T_3C))
			if((tCamViewTypeSel == SINGLE_VIEW)||(tCamViewTypeSel == SCAN_VIEW)||(tCamViewTypeSel == DUAL_VIEW))				
			{
			}
			else
			{
				tCamViewTypeSel = V3_3T_VIEW;
			}
		#endif
		
		#if (VDO_DATAPATH_MODE == VDO_PATH_ONLY_LIVECAM)
			if(UI_REC_START == tUI_RecPlayAct.tRecAct)
			{
				ubWarnNoteShowFlag = TRUE;
			}
			else
		#endif
			{
				if((tUI_CamNumSel == SCAN_TYPE_ITEM) && (tCamViewSel.tCamViewType == SCAN_VIEW))
					goto EXIT_CAMSELECT_MENU;
				if(SCAN_VIEW == tCamViewSel.tCamViewType)
					UI_DisableScanMode();
				if(DISPLAY_2T1R == tUI_CuSetting.ubTotalCamNum)
				{
					tCamViewSel.tCamViewPool[0] = (tUI_CamNumSel == CAM_2T)?(DISP_LEFT == tUI_CamStatus[CAM1].tCamDispLocation)?CAM1:CAM2:tUI_CamNumSel;
					tCamViewSel.tCamViewPool[1] = ((tCamViewSel.tCamViewPool[0] + 1) > CAM2)?CAM1:CAM2;
				}
				else
				{
					UI_CamNum_t tNextCamNum;

					tCamViewSel.tCamViewPool[0] = (DISPLAY_4T1R == tUI_CuSetting.ubTotalCamNum)?(tUI_CamNumSel == DUAL_TYPE_ITEM)?tCamViewSel.tCamViewPool[0]:tUI_CamNumSel:CAM1;
					tNextCamNum = ((tUI_CamNumSel + 1) > CAM4)?CAM1:(UI_CamNum_t)(tUI_CamNumSel + 1);
					tCamViewSel.tCamViewPool[1] = (DISPLAY_4T1R == tUI_CuSetting.ubTotalCamNum)?(tUI_CamNumSel == DUAL_TYPE_ITEM)?tCamViewSel.tCamViewPool[1]:tNextCamNum:tNextCamNum;
				}
				if(DUAL_TYPE_ITEM == tUI_CamNumSel)
				{
					UI_CameraSelection4DualView(ENTER_ARROW);
					break;
				}
				else
					ubUI_DualViewExFlag = FALSE;
				tCamViewSel.tCamViewType = tCamViewTypeSel;
				if(SCAN_VIEW == tCamViewSel.tCamViewType)
				{
					tCamViewSel.tCamViewPool[0] = CAM1;
					tUI_ChkResult = UI_CheckCameraSource4SV();
				#if (VDO_DATAPATH_MODE == VDO_PATH_ONLY_LIVECAM)
					if((rUI_SUCCESS == tUI_ChkResult) && (UI_PHOTOCAP_MODE != tUI_CuSetting.tVdoMode))
					{
						tUI_CuSetting.RecInfo.tREC_Mode = REC_OFF;
						tUI_CuSetting.tVdoMode = UI_PHOTOCAP_MODE;
						UI_UpdateDevStatusInfo();
					}
				#endif
				}
				if(rUI_SUCCESS == tUI_ChkResult)
					UI_SwitchCameraSource();
			}
		case EXIT_ARROW:
EXIT_CAMSELECT_MENU:
			tOsdImgInfo.uwXStart  = 100;
			tOsdImgInfo.uwYStart  = 0;
			tOsdImgInfo.uwHSize   = 255;
			tOsdImgInfo.uwVSize   = uwLCD_GetLcdVoSize();
			if(ENTER_ARROW == tArrowKey)
			{
				if(TRUE == ubWarnNoteShowFlag)
				{
					OSD_EraserImg2(&tOsdImgInfo);
					tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_RECORDINGWARN_ICON, 1, &tOsdImgInfo);
					osDelay(80);
					tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
					tUI_CuSetting.WarnIcon.tWarnNote 	   = UI_WARN_RECMODE;
					tUI_CuSetting.WarnIcon.ubWarnUpdateCnt = UI_WARNINGNOTE_PERIOD;
					tUI_State = UI_SHOWSTSICON_STATE;
					ubWarnNoteShowFlag = FALSE;
					break;
				}
				else
				{
					tUI_CuSetting.IconSts.ubDrawStsIconFlag = TRUE;
					UI_ClearStatusBarOsdIcon();
				}
			}
			UI_ClearCamConnectStatusFlag();
			OSD_EraserImg2(&tOsdImgInfo);
			tUI_State = UI_DISPLAY_STATE;
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_CameraSelection4DualView(UI_ArrowKey_t tArrowKey)
{
	static UI_DualViewCamSel_t tCamSelIdx = DUAL_CAM1_CAM2;
	OSD_IMG_INFO tDualSelOsdImgInfo[8];
	UI_Result_t tUI_ChkResult = rUI_SUCCESS;
	uint16_t uwUI_DualCamSelIdx = 0;
	uint8_t ubUI_CamSelTable[DUAL_VIEWCAMSEL_MAX] = {[DUAL_CAM1_CAM2] = 0x1,
													 [DUAL_CAM1_CAM3] = 0x2,												    
													 [DUAL_CAM1_CAM4] = 0x3,													
													 [DUAL_CAM2_CAM1] = 0x10,
													 [DUAL_CAM2_CAM3] = 0x12,													
													 [DUAL_CAM2_CAM4] = 0x13,													
													 [DUAL_CAM3_CAM1] = 0x20,
													 [DUAL_CAM3_CAM2] = 0x21,													
													 [DUAL_CAM3_CAM4] = 0x23,
													 [DUAL_CAM4_CAM1] = 0x30,
													 [DUAL_CAM4_CAM2] = 0x31,
													 [DUAL_CAM4_CAM3] = 0x32													
													};
	if(UI_DUALVIEW_CAMSEL_STATE != tUI_State)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DUALVIEWSELWIN, 8, &tDualSelOsdImgInfo[0]);
		tOSD_Img2(&tDualSelOsdImgInfo[0], OSD_QUEUE);
		uwUI_DualCamSelIdx = (TRUE == ubUI_DualViewExFlag)?((ubUI_CamSelTable[tCamSelIdx] >> 4) + 1):1;
		tOSD_Img2(&tDualSelOsdImgInfo[uwUI_DualCamSelIdx], OSD_QUEUE);
		uwUI_DualCamSelIdx = (TRUE == ubUI_DualViewExFlag)?((ubUI_CamSelTable[tCamSelIdx] & 0xF) + 1):2;
		tDualSelOsdImgInfo[uwUI_DualCamSelIdx].uwYStart -= 48;
		tOSD_Img2(&tDualSelOsdImgInfo[uwUI_DualCamSelIdx], OSD_QUEUE);
		tCamSelIdx = (TRUE == ubUI_DualViewExFlag)?tCamSelIdx:DUAL_CAM1_CAM2;
		tOSD_Img2(&tDualSelOsdImgInfo[6], OSD_QUEUE);
		tOSD_Img2(&tDualSelOsdImgInfo[7], OSD_UPDATE);
		tUI_State = UI_DUALVIEW_CAMSEL_STATE;
		return;
	}
	switch(tArrowKey)
	{
		case UP_ARROW:
			if(DUAL_CAM1_CAM2 == tCamSelIdx)
				return;
			--tCamSelIdx;
			break;
		case DOWN_ARROW:	
			if(DUAL_CAM4_CAM3 == tCamSelIdx)
				return;		
			++tCamSelIdx;
			break;
		case ENTER_ARROW:
			tUI_ChkResult = ((tCamViewSel.tCamViewPool[0] == (UI_CamNum_t)(ubUI_CamSelTable[tCamSelIdx] >> 4)) &&
							 (tCamViewSel.tCamViewPool[1] == (UI_CamNum_t)(ubUI_CamSelTable[tCamSelIdx] & 0xF)) &&
		                     (DUAL_VIEW == tCamViewSel.tCamViewType))?rUI_FAIL:rUI_SUCCESS;
			tCamViewSel.tCamViewType = DUAL_VIEW;
			if(rUI_SUCCESS == tUI_ChkResult)
			{
				tCamViewSel.tCamViewPool[0] = (UI_CamNum_t)(ubUI_CamSelTable[tCamSelIdx] >> 4);
				tCamViewSel.tCamViewPool[1] = (UI_CamNum_t)(ubUI_CamSelTable[tCamSelIdx] & 0xF);
				UI_SwitchCameraSource();
				ubUI_DualViewExFlag = TRUE;
			}
			tUI_CuSetting.IconSts.ubDrawStsIconFlag = TRUE;
			UI_ClearStatusBarOsdIcon();
			UI_ClearCamConnectStatusFlag();
			tDualSelOsdImgInfo[0].uwXStart	= 50;
			tDualSelOsdImgInfo[0].uwYStart	= 0;
			tDualSelOsdImgInfo[0].uwHSize	= 305;
			tDualSelOsdImgInfo[0].uwVSize	= uwLCD_GetLcdVoSize();
			OSD_EraserImg2(&tDualSelOsdImgInfo[0]);
			tUI_State = UI_DISPLAY_STATE;
			return;
		case EXIT_ARROW:
			tDualSelOsdImgInfo[0].uwXStart  = 50;
			tDualSelOsdImgInfo[0].uwYStart  = 0;
			tDualSelOsdImgInfo[0].uwHSize   = 100;
			tDualSelOsdImgInfo[0].uwVSize   = uwLCD_GetLcdVoSize();
			OSD_EraserImg2(&tDualSelOsdImgInfo[0]);
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DUALVIEWSELDELWIN, 1, &tDualSelOsdImgInfo[0]);
			tOSD_Img2(&tDualSelOsdImgInfo[0], OSD_UPDATE);
			tUI_State = UI_CAM_SEL_STATE;
			return;
		default:
			break;
	}
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DUALVIEWSELCAM1, 4, &tDualSelOsdImgInfo[0]);
	uwUI_DualCamSelIdx = (ubUI_CamSelTable[tCamSelIdx] >> 4);
	tOSD_Img2(&tDualSelOsdImgInfo[uwUI_DualCamSelIdx], OSD_QUEUE);
	uwUI_DualCamSelIdx = (ubUI_CamSelTable[tCamSelIdx] & 0xF);
	tDualSelOsdImgInfo[uwUI_DualCamSelIdx].uwYStart -= 48;
	tOSD_Img2(&tDualSelOsdImgInfo[uwUI_DualCamSelIdx], OSD_UPDATE);
}
//------------------------------------------------------------------------------
void UI_ChangeAudioSourceKey(void)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint16_t uwDisp2TImgIdx[4] = {OSD2IMG_SELADOCAM1ONLINE_ICON,   OSD2IMG_SELADOCAM1ONLINE_ICON,
								  OSD2IMG_SELADOCAM2_1ONLINE_ICON, OSD2IMG_SELADOCAM2_1OFFLINE_ICON};
	uint16_t uwDisp4TImgIdx[8] = {OSD2IMG_SELADOCAM1ONLINE_ICON, OSD2IMG_SELADOCAM1OFFLINE_ICON,
								  OSD2IMG_SELADOCAM2ONLINE_ICON, OSD2IMG_SELADOCAM2OFFLINE_ICON,
								  OSD2IMG_SELADOCAM3ONLINE_ICON, OSD2IMG_SELADOCAM3OFFLINE_ICON,
								  OSD2IMG_SELADOCAM4ONLINE_ICON, OSD2IMG_SELADOCAM4OFFLINE_ICON};
	uint16_t uwDisplayImgIdx;
	uint16_t uwXOffset = 0, uwYOffset = (DISPLAY_2T1R == tUI_CuSetting.ubTotalCamNum)?150:0;
	uint16_t uwYItemOffset = 150;
	UI_CamNum_t tCamNum;

	if((APP_LOSTLINK_STATE == tUI_SyncAppState) || (tUI_State != UI_CAM_SEL_STATE) ||
	   (DISPLAY_1T1R == tUI_CuSetting.ubTotalCamNum))
		return;

	for(tCamNum = CAM1; tCamNum < tUI_CuSetting.ubTotalCamNum; tCamNum++)
	{
		uwDisplayImgIdx  = (DISPLAY_2T1R == tUI_CuSetting.ubTotalCamNum)?uwDisp2TImgIdx[tCamNum*2]:uwDisp4TImgIdx[tCamNum*2];
		uwDisplayImgIdx += (((tUI_CamStatus[tCamNum].ulCAM_ID == INVALID_ID) || (tUI_CamStatus[tCamNum].tCamConnSts == CAM_OFFLINE))?1:0);
		tOSD_GetOsdImgInfor(1, OSD_IMG2, uwDisplayImgIdx, 1, &tOsdImgInfo);
		tOsdImgInfo.uwYStart -= uwYOffset;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	}
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SELCAMHL_ICON, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart += uwXOffset;
	tOsdImgInfo.uwYStart -= ((tUI_CuSetting.tAdoSrcCamNum*111) + uwYOffset + uwYItemOffset);
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
	tUI_State = UI_SET_ADOSRC_STATE;
}
//------------------------------------------------------------------------------
void UI_ChangeAudioSource(UI_ArrowKey_t tArrowKey)
{
	static UI_CamNum_t tAdoCamNumSel;
	static uint8_t ubUI_UpdateAdoCamSelFlag = FALSE;
	UI_CamNum_t tPreAdoCamNum = tAdoCamNumSel;
	uint16_t uwXOffset = 0, uwYOffset = (DISPLAY_2T1R == tUI_CuSetting.ubTotalCamNum)?150:0;
	uint16_t uwYItemOffset = 150;
	uint8_t ubCamNum;
	OSD_IMG_INFO tOsdImgInfo;

	if(FALSE == ubUI_UpdateAdoCamSelFlag)
	{
		tAdoCamNumSel = tUI_CuSetting.tAdoSrcCamNum;
		ubUI_UpdateAdoCamSelFlag = TRUE;
	}
	tPreAdoCamNum = tAdoCamNumSel;
	switch(tArrowKey)
	{
		case UP_ARROW:
			UI_CameraSelectionKey();
			return;
		case DOWN_ARROW:
			if(SINGLE_VIEW != tCamViewSel.tCamViewType)
				UI_DrawDCIMFolderMenu();	//!UI_DrawCameraSettingMenu(UI_CAMISP_SETUP);
			ubUI_UpdateAdoCamSelFlag = FALSE;
			return;
		case LEFT_ARROW:
			if(tAdoCamNumSel == CAM1)
				return;
			for(ubCamNum = tAdoCamNumSel; ubCamNum > CAM1; ubCamNum--)
			{
				if((tUI_CamStatus[ubCamNum - 1].ulCAM_ID != INVALID_ID) &&
				   (tUI_CamStatus[ubCamNum - 1].tCamConnSts == CAM_ONLINE))
				{
					tAdoCamNumSel = (UI_CamNum_t)(ubCamNum - 1);
					break;
				}
				if((ubCamNum - 1) == CAM1)
					return;
			}
			break;
		case RIGHT_ARROW:
			if((tAdoCamNumSel + 1) >= tUI_CuSetting.ubTotalCamNum)
				return;
			for(ubCamNum = (tAdoCamNumSel + 1); ubCamNum < tUI_CuSetting.ubTotalCamNum; ubCamNum++)
			{
				if((tUI_CamStatus[ubCamNum].ulCAM_ID != INVALID_ID) &&
				   (tUI_CamStatus[ubCamNum].tCamConnSts == CAM_ONLINE))
				{
					tAdoCamNumSel = (UI_CamNum_t)ubCamNum;
					break;
				}
			}
			if(ubCamNum == tUI_CuSetting.ubTotalCamNum)
				return;
			break;
		case ENTER_ARROW:
			UI_SwitchAudioSource(tAdoCamNumSel);
			UI_UpdateDevStatusInfo();
		case EXIT_ARROW:
			if(ENTER_ARROW == tArrowKey)
				UI_ClearStatusBarOsdIcon();
			UI_ClearCamConnectStatusFlag();
			tOsdImgInfo.uwXStart  = 100;
			tOsdImgInfo.uwYStart  = (DISPLAY_2T1R == tUI_CuSetting.ubTotalCamNum)?450:0;
			tOsdImgInfo.uwHSize   = (DISPLAY_2T1R == tUI_CuSetting.ubTotalCamNum)?300:255;
			tOsdImgInfo.uwVSize   = (DISPLAY_2T1R == tUI_CuSetting.ubTotalCamNum)?335:uwLCD_GetLcdVoSize();
			OSD_EraserImg2(&tOsdImgInfo);
			ubUI_UpdateAdoCamSelFlag = FALSE;
			tUI_State = UI_DISPLAY_STATE;
			return;
		default:
			return;
	}
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SELCAMNOR_ICON, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart += uwXOffset;
	tOsdImgInfo.uwYStart -= ((tPreAdoCamNum*111) + uwYOffset + uwYItemOffset);
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SELCAMHL_ICON, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart += uwXOffset;
	tOsdImgInfo.uwYStart -= ((tAdoCamNumSel*111) + uwYOffset + uwYItemOffset);
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}
//------------------------------------------------------------------------------
#define UI_MENUICON_NUM		7
#define UI_WRICON_OFFSET	2
//------------------------------------------------------------------------------
void UI_ChangeVideoMode(UI_ArrowKey_t tArrowKey)
{
	static UI_VdoModeList_t tUI_VdoModeSet = UI_VDOPHOTO_MODE;
	static UI_VdoModeList_t tVdoModeSel = UI_VDOPHOTO_MODE;
	static uint8_t ubUI_UpdateVdoModeFlag = FALSE;
	UI_VdoModeList_t tPreVdoMode = UI_VDOPHOTO_MODE;
	uint16_t uwVdoModeOsdImg[UI_VDOMODELIST_MAX] = {OSD2IMG_RECLOOPMODENOR_ICON, OSD2IMG_RECMANUMODENOR_ICON,
												    OSD2IMG_RECTRIGMODENOR_ICON, OSD2IMG_PHOTOSHOOTMODENOR_ICON};
	OSD_IMG_INFO tOsdImgInfo;

	if(FALSE == ubUI_UpdateVdoModeFlag)
	{
		tUI_VdoModeSet = tVdoModeSel = tPreVdoMode = (UI_PHOTOCAP_MODE == tUI_CuSetting.tVdoMode)?UI_VDOPHOTO_MODE:
												     (REC_LOOPING == tUI_CuSetting.RecInfo.tREC_Mode)?UI_VDORECLOOP_MODE:
												     (REC_MANUAL  == tUI_CuSetting.RecInfo.tREC_Mode)?UI_VDORECMANU_MODE:UI_VDORECTRIG_MODE;
		ubUI_UpdateVdoModeFlag = TRUE;
	}
	switch(tArrowKey)
	{
		case LEFT_ARROW:
			if(tVdoModeSel == UI_VDORECLOOP_MODE)
				return;
			tPreVdoMode = tVdoModeSel;
			tVdoModeSel--;
			break;
		case RIGHT_ARROW:
			if(tVdoModeSel == UI_VDOPHOTO_MODE)
				return;
			tPreVdoMode = tVdoModeSel;
			tVdoModeSel++;
			break;
		case ENTER_ARROW:
			if(tVdoModeSel == UI_VDORECTRIG_MODE)
				return;
		#if (VDO_DATAPATH_MODE == VDO_PATH_ONLY_LIVECAM)
			if((tUI_VdoModeSet != tVdoModeSel) &&
			   ((UI_VDOPHOTO_MODE == tVdoModeSel) || (SCAN_VIEW != tCamViewSel.tCamViewType)))
		#else
			if(tUI_VdoModeSet != tVdoModeSel)
		#endif
			{
			#if (!APP_SD_FUNC_ENABLE || !APP_REC_FUNC_ENABLE)
				if((UI_VDORECLOOP_MODE == tVdoModeSel) || (UI_VDORECMANU_MODE == tVdoModeSel))
					return;
			#endif
			#if !APP_PHOTOGRAPH_FUNC_ENABLE
				if(UI_VDOPHOTO_MODE == tVdoModeSel)
					return;
			#endif
				if(UI_RECORDING_MODE == tUI_CuSetting.tVdoMode)
				{
					ubUI_VdoRecChkFlag = ((UI_REC_STOP == tUI_RecPlayAct.tRecAct) && (REC_LOOPING == tUI_CuSetting.RecInfo.tREC_Mode))?FALSE:TRUE;
					UI_VideoRecordingExec(UI_REC_STOP);
				}
				tUI_CuSetting.RecInfo.tREC_Mode = (UI_VDORECLOOP_MODE == tVdoModeSel)?REC_LOOPING:
												  (UI_VDORECMANU_MODE == tVdoModeSel)?REC_MANUAL:
												  (UI_VDORECTRIG_MODE == tVdoModeSel)?REC_TRIGGER:REC_OFF;
				tUI_CuSetting.tVdoMode = (REC_OFF == tUI_CuSetting.RecInfo.tREC_Mode)?UI_PHOTOCAP_MODE:UI_RECORDING_MODE;
				if((UI_RECORDING_MODE == tUI_CuSetting.tVdoMode) && (REC_LOOPING == tUI_CuSetting.RecInfo.tREC_Mode))
					UI_VideoRecordingExec(UI_REC_START);
                else
                    KNL_SetRecordFunc(KNL_RECORDFUNC_DISABLE);
				tUI_VdoModeSet = tVdoModeSel;
				UI_UpdateDevStatusInfo();
			}
		case EXIT_ARROW:
			if(UI_SDCARDFMT_STATE != tUI_State)
			{
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PHOTOSHOOTMODEHL_ICON, 1, &tOsdImgInfo);
				tOsdImgInfo.uwHSize = 110;
				tOsdImgInfo.uwVSize = 650;
				OSD_EraserImg2(&tOsdImgInfo);
			}
			tUI_State = (UI_SET_VDOMODE_STATE == tUI_State)?UI_DISPLAY_STATE:tUI_State;
			ubUI_UpdateVdoModeFlag = FALSE;
			tUI_CuSetting.IconSts.ubDrawStsIconFlag = TRUE;
			return;
		default:
			return;
	}
	tOSD_GetOsdImgInfor(1, OSD_IMG2, (uwVdoModeOsdImg[tPreVdoMode]+UI_ICON_HIGHLIGHT), 1, &tOsdImgInfo);
	OSD_EraserImg2(&tOsdImgInfo);
	UI_DrawHLandNormalIcon(uwVdoModeOsdImg[tPreVdoMode], (uwVdoModeOsdImg[tVdoModeSel]+UI_ICON_HIGHLIGHT));
}
//------------------------------------------------------------------------------
void UI_CuPowerSaveKey(void)
{
	OSD_IMG_INFO tOsdImgInfo[5];

#if (defined(BUC_CU) && APP_DUAL_HOST_ENABLE)
	tOsdImgInfo[0] = tOsdImgInfo[0];	//Avoid Warning
	printf("Not Support CuPowerSave Mode\r\n");
	return;
#else	
	if((APP_LOSTLINK_STATE == tUI_SyncAppState)
	|| (tUI_State != UI_DISPLAY_STATE)
//	|| (DISPLAY_1T1R != tUI_CuSetting.ubTotalCamNum)
	|| (PS_VOX_MODE == tUI_CuSetting.tPsMode))
		return;

	if(tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_VOXOPT_ICON, 5, &tOsdImgInfo[0]) != OSD_OK)
	{
		printd(DBG_ErrorLvl, "Load OSD Image FAIL, pls check (%d) !\n", __LINE__);
		return;
	}
	tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);
	tOSD_Img2(&tOsdImgInfo[1], OSD_QUEUE);
	tOSD_Img2(&tOsdImgInfo[2], OSD_QUEUE);
	tOSD_Img2(&tOsdImgInfo[4], OSD_UPDATE);
	tUI_State = UI_SET_CUPSMODE_STATE;
#endif
}
//------------------------------------------------------------------------------
UI_Result_t UI_SetupPuVoxMode(void)
{
	UI_CUReqCmd_t tPsCmd;
	UI_CamNum_t tCamNum;
	UI_Result_t tVoxRet = rUI_FAIL, tBuNotifyRet = rUI_SUCCESS;

//justin 2020.07.02
#if (defined(BUC_CU) && APP_DUAL_HOST_ENABLE)	
	for(tCamNum = CAM1; tCamNum < KNL_WIRELESS_CAM_NUM; tCamNum++)
#else
	for(tCamNum = CAM1; tCamNum < tUI_CuSetting.ubTotalCamNum; tCamNum++)
#endif
	{
		if(PS_ECO_MODE == tUI_CamStatus[tCamNum].tCamPsMode)
			continue;
		if(CAM_OFFLINE == tUI_CamStatus[tCamNum].tCamConnSts)
			continue;
		tPsCmd.tDS_CamNum 				= tCamNum;
		tPsCmd.ubCmd[UI_TWC_TYPE]		= UI_SETTING;
		tPsCmd.ubCmd[UI_SETTING_ITEM]   = UI_VOXMODE_SETTING;
		tPsCmd.ubCmd[UI_SETTING_DATA]   = PS_VOX_MODE;
		tPsCmd.ubCmd_Len  				= 3;
		tBuNotifyRet = UI_SendRequestToCAM(osThreadGetId(), &tPsCmd);
		if(rUI_SUCCESS == tBuNotifyRet)
			tVoxRet = rUI_SUCCESS;
		else
			printd(DBG_ErrorLvl, "CAM%d:VOX Notify Fail !\n", (tCamNum + 1));
		tUI_CamStatus[tCamNum].tCamPsMode = PS_VOX_MODE;
	}
	if(rUI_SUCCESS == tVoxRet)
		UI_EnableVox();
	return tVoxRet;
}
//------------------------------------------------------------------------------
UI_Result_t UI_SetupPuAdoOnlyMode(void)
{
	APP_EventMsg_t tUI_PsMessage = {0};

	LCDBL_ENABLE(UI_DISABLE);
	UI_DisableScanMode();
	tUI_PsMessage.ubAPP_Event 	   = APP_POWERSAVE_EVENT;
	tUI_PsMessage.ubAPP_Message[0] = 2;		//! Message Length
	tUI_PsMessage.ubAPP_Message[1] = PS_ADOONLY_MODE;
	tUI_PsMessage.ubAPP_Message[2] = TRUE;
	UI_SendMessageToAPP(&tUI_PsMessage);
	tUI_CuSetting.tPsMode = PS_ADOONLY_MODE;
	UI_EnableScanMode();

	return rUI_SUCCESS;
}
//------------------------------------------------------------------------------
UI_Result_t UI_SetupPuWorMode(void)
{
	UI_CUReqCmd_t tPsCmd;
	UI_CamNum_t tCamNum;
	UI_Result_t tWorRet = rUI_FAIL, tBuNotifyRet = rUI_SUCCESS;

	tPsCmd.ubCmd[UI_TWC_TYPE]		= UI_SETTING;
	tPsCmd.ubCmd[UI_SETTING_ITEM]   = UI_WORMODE_SETTING;
	tPsCmd.ubCmd[UI_SETTING_DATA]   = PS_WOR_MODE;
	tPsCmd.ubCmd_Len  				= 3;
	
	//justin 2020.07.01
#if (defined(BUC_CU) && APP_DUAL_HOST_ENABLE)
	for(tCamNum = CAM1; tCamNum < KNL_WIRELESS_CAM_NUM; tCamNum++)
#else	
	for(tCamNum = CAM1; tCamNum < tUI_CuSetting.ubTotalCamNum; tCamNum++)
#endif
	{
		if(CAM_OFFLINE == tUI_CamStatus[tCamNum].tCamConnSts)
			continue;
		tPsCmd.tDS_CamNum = tCamNum;
		tBuNotifyRet = UI_SendRequestToCAM(osThreadGetId(), &tPsCmd);
		if(rUI_SUCCESS == tBuNotifyRet)
			tWorRet = rUI_SUCCESS;
		else
			printd(DBG_ErrorLvl, "CAM%d:WOR Setting Fail !\n", (tCamNum + 1));
	}
	if(rUI_SUCCESS == tWorRet)
	{
		APP_EventMsg_t tUI_PsMessage = {0};

		tUI_CuSetting.tPsMode = PS_WOR_MODE;
		UI_UpdateDevStatusInfo();
		tUI_PsMessage.ubAPP_Event 	    = APP_POWERSAVE_EVENT;
		tUI_PsMessage.ubAPP_Message[0]  = 3;		//! Message Length
		tUI_PsMessage.ubAPP_Message[1]  = PS_WOR_MODE;
		tUI_PsMessage.ubAPP_Message[2]  = FALSE;
		tUI_PsMessage.ubAPP_Message[3]  = CAM1;
		UI_SendMessageToAPP(&tUI_PsMessage);
	}
	return rUI_SUCCESS;
}
//------------------------------------------------------------------------------
static uint8_t ubUI_WakeUpFromPsFlag = FALSE;
uint8_t ubUI_ClrFlg = FALSE;
UI_Result_t UI_SetupCamEcoMode(UI_CamNum_t tECO_CamNum)
{	
	APP_EventMsg_t tUI_PsMessage = {0};

	tUI_PsMessage.ubAPP_Event 	    = APP_POWERSAVE_EVENT;
	tUI_PsMessage.ubAPP_Message[0]  = 4;		//! Message Length
	tUI_PsMessage.ubAPP_Message[1]  = PS_ECO_MODE;
	if((POWER_NORMAL_MODE == tUI_CamStatus[tECO_CamNum].tCamPsMode) &&
	   (FALSE == ulUI_MonitorPsFlag[tECO_CamNum]))
	{
		UI_CUReqCmd_t tPsCmd;

		tPsCmd.tDS_CamNum 				= tECO_CamNum;
		tPsCmd.ubCmd[UI_TWC_TYPE]		= UI_SETTING;
		tPsCmd.ubCmd[UI_SETTING_ITEM]   = UI_ECOMODE_SETTING;
		tPsCmd.ubCmd[UI_SETTING_DATA]   = PS_ECO_MODE;
		tPsCmd.ubCmd_Len  				= 3;
		if(tUI_CamStatus[tECO_CamNum].tCamConnSts == CAM_ONLINE)
		{
			UI_SendRequestToCAM(NULL, &tPsCmd);
			tUI_CamStatus[tECO_CamNum].tCamPsMode = PS_ECO_MODE;
			UI_UpdateDevStatusInfo();
			tUI_PsMessage.ubAPP_Message[2]  = FALSE;
			tUI_PsMessage.ubAPP_Message[3]  = tECO_CamNum;
			tUI_PsMessage.ubAPP_Message[4]  = FALSE;
			UI_SendMessageToAPP(&tUI_PsMessage);
		#if (defined(BUC_CU) && APP_DUAL_HOST_ENABLE)	
			ubUI_ClrFlg = TRUE;
		#endif
		}
	}
	else if(PS_ECO_MODE == tUI_CamStatus[tECO_CamNum].tCamPsMode)
	{
		tUI_PsMessage.ubAPP_Message[2]  = TRUE;
		tUI_PsMessage.ubAPP_Message[3]  = tECO_CamNum;
		tUI_PsMessage.ubAPP_Message[4]  = TRUE;
		UI_SendMessageToAPP(&tUI_PsMessage);
		ubUI_WakeUpFromPsFlag = TRUE;
	}
	
	return rUI_SUCCESS;
}
//------------------------------------------------------------------------------
void UI_CamPowerSaveKey(void)
{
#if (defined(BUC_CU) && APP_DUAL_HOST_ENABLE)	
	uint32_t ulIdTemp;
#endif
	
	OSD_IMG_INFO tOsdImgInfo;
	uint16_t uwDisp2TImgIdx[4] = {OSD2IMG_SELECOCAM1ONLINE_ICON,   OSD2IMG_SELECOCAM1ONLINE_ICON,
								  OSD2IMG_SELECOCAM2_1ONLINE_ICON, OSD2IMG_SELECOCAM2_1OFFLINE_ICON};
	uint16_t uwDisp4TImgIdx[8] = {OSD2IMG_SELECOCAM1ONLINE_ICON, OSD2IMG_SELECOCAM1OFFLINE_ICON,
								  OSD2IMG_SELECOCAM2ONLINE_ICON, OSD2IMG_SELECOCAM2OFFLINE_ICON,
								  OSD2IMG_SELECOCAM3ONLINE_ICON, OSD2IMG_SELECOCAM3OFFLINE_ICON,
								  OSD2IMG_SELECOCAM4ONLINE_ICON, OSD2IMG_SELECOCAM4OFFLINE_ICON};
	uint16_t uwDisplayImgIdx;
	uint16_t uwXOffset = 5, uwYOffset = (DISPLAY_2T1R == tUI_CuSetting.ubTotalCamNum)?155:0;
	uint16_t uwYItemOffset = 150, uwYEcoOffset = 150;
	UI_CamNum_t tCamNum;

	if(PS_VOX_MODE == tUI_CuSetting.tPsMode)
	{
		UI_DisableVox();
		return;
	}
	if(PS_ADOONLY_MODE == tUI_CuSetting.tPsMode)
	{
		UI_DisableCuAdoOnlyMode();
		return;
	}
#if (APP_DOORPHONE_ENABLE==0)
	if((UI_DISPLAY_STATE != tUI_State) ||
	   (TRUE == tUI_CuSetting.IconSts.ubShowLostLogoFlag))
		return;
	if((DISPLAY_1T1R == tUI_CuSetting.ubTotalCamNum) ||
	   (SINGLE_VIEW == tCamViewSel.tCamViewType))
	{
		UI_SetupCamEcoMode(tCamViewSel.tCamViewPool[0]);
		return;
	}
#endif
	tUI_CamEcoCamNum = NO_CAM;
	
//justin 2020.07.02
#if (defined(BUC_CU) && APP_DUAL_HOST_ENABLE)		
	for(tCamNum = CAM1; tCamNum < tUI_CuSetting.ubTotalCamNum; tCamNum++)
	{
		uwYEcoOffset     = 0;
		uwDisplayImgIdx  = (DISPLAY_2T1R == tUI_CuSetting.ubTotalCamNum)?uwDisp2TImgIdx[tCamNum*2]:uwDisp4TImgIdx[tCamNum*2];		
		
		if(KNL_WIRELESS_CAM_NUM == 1)
		{
			if(tCamNum == CAM1)
				ulIdTemp = tUI_CamStatus[tCamNum].ulCAM_ID;
			else
				ulIdTemp = INVALID_ID;
		}
		else
		{
			ulIdTemp = tUI_CamStatus[tCamNum].ulCAM_ID;
		}
		
		//if((tUI_CamStatus[tCamNum].ulCAM_ID == INVALID_ID) ||
		if((ulIdTemp == INVALID_ID) ||
		   ((tUI_CamStatus[tCamNum].tCamConnSts == CAM_OFFLINE) && (PS_ECO_MODE != tUI_CamStatus[tCamNum].tCamPsMode)))
		{
			uwDisplayImgIdx = ((DISPLAY_2T1R == tUI_CuSetting.ubTotalCamNum) && (CAM2 == tCamNum))?OSD2IMG_SELCAM2_1OFFLINE_ICON:(uwDisplayImgIdx+1);
		}
		else
		{
			if(PS_ECO_MODE == tUI_CamStatus[tCamNum].tCamPsMode)
			{
				uwDisplayImgIdx = ((DISPLAY_2T1R == tUI_CuSetting.ubTotalCamNum) && (CAM2 == tCamNum))?OSD2IMG_SELCAM2_1ONLINE_ICON:(uwDisplayImgIdx-32);
				uwYEcoOffset    = (((DISPLAY_2T1R == tUI_CuSetting.ubTotalCamNum) && (CAM2 == tCamNum)) ||
								   ((DISPLAY_4T1R == tUI_CuSetting.ubTotalCamNum) && (CAM4 == tCamNum)))?0:150;
			}
			tUI_CamEcoCamNum = (NO_CAM == tUI_CamEcoCamNum)?tCamNum:tUI_CamEcoCamNum;
		}
		tOSD_GetOsdImgInfor(1, OSD_IMG2, uwDisplayImgIdx, 1, &tOsdImgInfo);	
		tOsdImgInfo.uwYStart -= (uwYOffset + uwYEcoOffset);
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	}	
#else	
	for(tCamNum = CAM1; tCamNum < tUI_CuSetting.ubTotalCamNum; tCamNum++)
	{
		uwYEcoOffset     = 0;
		uwDisplayImgIdx  = (DISPLAY_2T1R == tUI_CuSetting.ubTotalCamNum)?uwDisp2TImgIdx[tCamNum*2]:uwDisp4TImgIdx[tCamNum*2];		
		
		if((tUI_CamStatus[tCamNum].ulCAM_ID == INVALID_ID) ||
		   ((tUI_CamStatus[tCamNum].tCamConnSts == CAM_OFFLINE) && (PS_ECO_MODE != tUI_CamStatus[tCamNum].tCamPsMode)))
		{
			uwDisplayImgIdx = ((DISPLAY_2T1R == tUI_CuSetting.ubTotalCamNum) && (CAM2 == tCamNum))?OSD2IMG_SELCAM2_1OFFLINE_ICON:(uwDisplayImgIdx+1);
		}
		else
		{
			if(PS_ECO_MODE == tUI_CamStatus[tCamNum].tCamPsMode)
			{
				uwDisplayImgIdx = ((DISPLAY_2T1R == tUI_CuSetting.ubTotalCamNum) && (CAM2 == tCamNum))?OSD2IMG_SELCAM2_1ONLINE_ICON:(uwDisplayImgIdx-32);
				uwYEcoOffset    = (((DISPLAY_2T1R == tUI_CuSetting.ubTotalCamNum) && (CAM2 == tCamNum)) ||
								   ((DISPLAY_4T1R == tUI_CuSetting.ubTotalCamNum) && (CAM4 == tCamNum)))?0:150;
			}
			tUI_CamEcoCamNum = (NO_CAM == tUI_CamEcoCamNum)?tCamNum:tUI_CamEcoCamNum;
		}
		tOSD_GetOsdImgInfor(1, OSD_IMG2, uwDisplayImgIdx, 1, &tOsdImgInfo);	
		tOsdImgInfo.uwYStart -= (uwYOffset + uwYEcoOffset);
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	}
#endif
	if(NO_CAM != tUI_CamEcoCamNum)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SELCAMHL_ICON, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart += uwXOffset;
		tOsdImgInfo.uwYStart -= ((tUI_CamEcoCamNum*111) + uwYOffset + uwYItemOffset);
		tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
	}
	tUI_State = UI_SET_CAMECOMODE_STATE;
}
//------------------------------------------------------------------------------
void UI_CuPowerSaveModeSelection(UI_ArrowKey_t tArrowKey)
{
	OSD_IMG_INFO tOsdImgInfo;
	static UI_PowerSaveMode_t tUI_PsMode = PS_VOX_MODE;
	UI_PowerSaveMode_t tUI_PrePsMode = PS_WOR_MODE;

	switch(tArrowKey)
	{
		case LEFT_ARROW:
			if(PS_VOX_MODE == tUI_PsMode)
				return;
			tUI_PrePsMode = tUI_PsMode;
			tUI_PsMode--;
			break;
		case RIGHT_ARROW:
			if(PS_WOR_MODE == tUI_PsMode)
				return;
			tUI_PrePsMode = tUI_PsMode;			
			tUI_PsMode++;
			break;
		case ENTER_ARROW:
            if((tUI_PsMode <= PS_WOR_MODE) && (UI_RECORDING_MODE == tUI_CuSetting.tVdoMode) && (UI_REC_START == tUI_RecPlayAct.tRecAct))
                UI_VideoRecordingExec(UI_REC_STOP);
			if(PS_VOX_MODE == tUI_PsMode)
			{
				tUI_PsMode = (rUI_SUCCESS != UI_SetupPuVoxMode())?POWER_NORMAL_MODE:tUI_PsMode;
			}
			else if(PS_ADOONLY_MODE == tUI_PsMode)
			{
				UI_SetupPuAdoOnlyMode();
			}
			else if(PS_WOR_MODE == tUI_PsMode)
			{
				UI_SetupPuWorMode();
			}
		case EXIT_ARROW:
			UI_ClearCamConnectStatusFlag();
			tOsdImgInfo.uwXStart = 153;
			tOsdImgInfo.uwYStart = 300;
			tOsdImgInfo.uwHSize  = 210;
			tOsdImgInfo.uwVSize  = 600;
			OSD_EraserImg2(&tOsdImgInfo);
			tUI_State = (ENTER_ARROW == tArrowKey)?(PS_VOX_MODE == tUI_PsMode)?UI_VOXPS_STATE:
												   (PS_ADOONLY_MODE == tUI_PsMode)?UI_ADOONLYPS_STATE:UI_DISPLAY_STATE:UI_DISPLAY_STATE;
			tUI_PsMode = PS_VOX_MODE;
			return;
		default:
			return;
	}
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PSSELECTNOR_ICON, 1, &tOsdImgInfo);
	tOsdImgInfo.uwYStart -= (tUI_PrePsMode * 183);
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PSSELECTHL_ICON, 1, &tOsdImgInfo);
	tOsdImgInfo.uwYStart -= (tUI_PsMode * 183);
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}
//------------------------------------------------------------------------------
void UI_CamPowerSaveModeSelection(UI_ArrowKey_t tArrowKey)
{
	OSD_IMG_INFO tOsdImgInfo;
	UI_CamNum_t tEcoPrevCamNum = tUI_CamEcoCamNum;
	uint16_t uwXOffset = 5, uwYOffset = (DISPLAY_2T1R == tUI_CuSetting.ubTotalCamNum)?155:0;
	uint16_t uwYItemOffset = 150;

	switch(tArrowKey)
	{
		case LEFT_ARROW:
		case RIGHT_ARROW:
			tUI_CamEcoCamNum = UI_ChangeSelectCamNum4UiMenu(&tEcoPrevCamNum, &tArrowKey);
			if(tUI_CamEcoCamNum >= tUI_CuSetting.ubTotalCamNum)
			{
				tUI_CamEcoCamNum = tEcoPrevCamNum;
				return;
			}
			break;
		case ENTER_ARROW:
			if(UI_SetupCamEcoMode(tUI_CamEcoCamNum) == rUI_FAIL)
				return;
		case EXIT_ARROW:
			tUI_CuSetting.IconSts.ubDrawStsIconFlag = TRUE;
			if(DISPLAY_4T1R == tUI_CuSetting.ubTotalCamNum)
				UI_ClearCamConnectStatusFlag();
			tOsdImgInfo.uwXStart  = 150;
			tOsdImgInfo.uwYStart  = (DISPLAY_2T1R == tUI_CuSetting.ubTotalCamNum)?450:400;
			tOsdImgInfo.uwHSize   = (DISPLAY_2T1R == tUI_CuSetting.ubTotalCamNum)?300:210;
			tOsdImgInfo.uwVSize   = (DISPLAY_2T1R == tUI_CuSetting.ubTotalCamNum)?335:500;
			OSD_EraserImg2(&tOsdImgInfo);
			if(ENTER_ARROW == tArrowKey)
				osDelay(800);
			tUI_State = UI_DISPLAY_STATE;
			return;
		default:
			return;
	}
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SELCAMNOR_ICON, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart += uwXOffset;
	tOsdImgInfo.uwYStart -= ((tEcoPrevCamNum*111) + uwYOffset + uwYItemOffset);
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SELCAMHL_ICON, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart += uwXOffset;
	tOsdImgInfo.uwYStart -= ((tUI_CamEcoCamNum*111) + uwYOffset + uwYItemOffset);
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}
//------------------------------------------------------------------------------
void UI_UpdatePushTalkIcon(void)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint32_t ulLcd_HSize  = uwLCD_GetLcdHoSize();
	uint32_t ulLcd_VSize  = uwLCD_GetLcdVoSize();
	uint16_t uwXOffset[7] = {[DISP_UPPER_LEFT] = 0,               [DISP_UPPER_RIGHT] = 0,
							 [DISP_LOWER_LEFT] = (ulLcd_HSize/2), [DISP_LOWER_RIGHT] = (ulLcd_HSize/2),
							 [DISP_LEFT] 	   = 0,				  [DISP_RIGHT] 		 = 0};
	uint16_t uwYOffset[7] = {[DISP_UPPER_LEFT] = 0,               [DISP_UPPER_RIGHT] = (ulLcd_VSize/2),
							 [DISP_LOWER_LEFT] = 0, 			  [DISP_LOWER_RIGHT] = (ulLcd_VSize/2),
							 [DISP_LEFT] 	   = 0, 		      [DISP_RIGHT] 		 = (ulLcd_VSize/2)};
	UI_DisplayLocation_t tUI_DispLoc;

	tUI_DispLoc = ((tCamViewSel.tCamViewType == SINGLE_VIEW) ||
				   (tCamViewSel.tCamViewType == SCAN_VIEW))?DISP_LOWER_RIGHT:tUI_CamStatus[tUI_CuSetting.tAdoSrcCamNum].tCamDispLocation;
	if(tCamViewSel.tCamViewType == DUAL_VIEW)
	{
		if((tUI_CuSetting.tAdoSrcCamNum != tCamViewSel.tCamViewPool[0]) && (tUI_CuSetting.tAdoSrcCamNum != tCamViewSel.tCamViewPool[1]))
			return;
//		tUI_DispLoc = (tUI_CuSetting.tAdoSrcCamNum == tCamViewSel.tCamViewPool[0])?DISP_LEFT:DISP_RIGHT;
        tUI_DispLoc = tUI_CamStatus[tUI_CuSetting.tAdoSrcCamNum].tCamDispLocation;
	}
	if(OSD_OK != tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PTTMIC_ICON, 1, &tOsdImgInfo))
		return;
	if(H_VIEW == tCamViewSel.tCamViewType)
	{
		tOsdImgInfo.uwXStart += ((DISP_H_L == tUI_DispLoc) || (DISP_H_R == tUI_DispLoc) || (DISP_H_CL == tUI_DispLoc))?(ulLcd_HSize/2):0;
		tOsdImgInfo.uwYStart  = ulLcd_VSize - ((DISP_H_L == tUI_DispLoc)?310:(DISP_H_R == tUI_DispLoc)?1270:((DISP_H_CU == tUI_DispLoc) || (DISP_H_CL == tUI_DispLoc))?950:0);
	}
	else
	{
		tOsdImgInfo.uwXStart += (uwXOffset[tUI_DispLoc] + ((tCamViewSel.tCamViewType == DUAL_VIEW)?(ulLcd_HSize/2):0));
		tOsdImgInfo.uwYStart -= uwYOffset[tUI_DispLoc];
	}
	switch(ubUI_PttStartFlag)
	{
		case TRUE:
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			break;
		case FALSE:
			OSD_EraserImg2(&tOsdImgInfo);
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_PushTalkKey(void)
{
	APP_EventMsg_t tUI_PttMessage = {0};

	if(APP_LOSTLINK_STATE == tUI_SyncAppState)
		return;
	tUI_PttMessage.ubAPP_Event 	 	= APP_PTT_EVENT;
	tUI_PttMessage.ubAPP_Message[0] = 1;		//! Message Length
	tUI_PttMessage.ubAPP_Message[1] = ubUI_PttStartFlag;
	UI_SendMessageToAPP(&tUI_PttMessage);
	SPEAKER_EN(((TRUE == ubUI_PttStartFlag)?UI_DISABLE:UI_ENABLE));
	if(UI_DISPLAY_STATE == tUI_State)
		UI_UpdatePushTalkIcon();
}
//------------------------------------------------------------------------------
void UI_DisplayArrowKeyFunc(UI_ArrowKey_t tArrowKey)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint16_t uwOsdImgIdx = 0;

	if(LCD_JPEG_ENABLE == tLCD_GetJpegDecoderStatus())
		return;

	switch(tArrowKey)
	{
		case LEFT_ARROW:
		{
			tUI_CuSetting.VolLvL.ubVOL_UpdateCnt = UI_UPDATEVOLLVL_PERIOD;
			if(tUI_CuSetting.VolLvL.tVOL_UpdateLvL == VOL_LVL0)
			{
				uwOsdImgIdx = OSD2IMG_VOLUME_LVL0_ICON;
				break;
			}
			uwOsdImgIdx = OSD2IMG_VOLUME_LVL0_ICON+(--tUI_CuSetting.VolLvL.tVOL_UpdateLvL);
			if(tUI_CuSetting.VolLvL.tVOL_UpdateLvL == VOL_LVL0)
			{
				ADO_SetDacMute(DAC_MR_0p5DB_1SAMPLE, ADO_ON);
				break;
			}
            ADO_SetDacR2RVol(tUI_VOLTable[tUI_CuSetting.VolLvL.tVOL_UpdateLvL]);
			break;
		}
		case RIGHT_ARROW:
		{
			tUI_CuSetting.VolLvL.ubVOL_UpdateCnt = UI_UPDATEVOLLVL_PERIOD;
			if(tUI_CuSetting.VolLvL.tVOL_UpdateLvL == VOL_LVL5)
			{
				uwOsdImgIdx = OSD2IMG_VOLUME_LVL5_ICON;
				break;
			}
			if(tUI_CuSetting.VolLvL.tVOL_UpdateLvL == VOL_LVL0)
				ADO_SetDacMute(DAC_MR_0p5DB_1SAMPLE, ADO_OFF);
			uwOsdImgIdx = OSD2IMG_VOLUME_LVL0_ICON+(++tUI_CuSetting.VolLvL.tVOL_UpdateLvL);
            ADO_SetDacR2RVol(tUI_VOLTable[tUI_CuSetting.VolLvL.tVOL_UpdateLvL]);
			break;
		}
		case UP_ARROW:
		{
			uint32_t ulUI_BLTable[] = {0x150, 0x230, 0x400, 0x600, 0x0800, 0xA00};
			tUI_CuSetting.BriLvL.ubBL_UpdateCnt = UI_UPDATEBRILVL_PERIOD;
			if(tUI_CuSetting.BriLvL.tBL_UpdateLvL == BL_LVL5)
			{
				uwOsdImgIdx = OSD2IMG_BRIGHT_LVL5_ICON;
				break;
			}
			uwOsdImgIdx = OSD2IMG_BRIGHT_LVL0_ICON+(++tUI_CuSetting.BriLvL.tBL_UpdateLvL);
			LCD_BACKLIGHT_CTRL(ulUI_BLTable[tUI_CuSetting.BriLvL.tBL_UpdateLvL]);
			break;
		}
		case DOWN_ARROW:
		{
			uint32_t ulUI_BLTable[] = {0x150, 0x230, 0x400, 0x600, 0x0800, 0xA00};
			tUI_CuSetting.BriLvL.ubBL_UpdateCnt = UI_UPDATEBRILVL_PERIOD;
			if(tUI_CuSetting.BriLvL.tBL_UpdateLvL == BL_LVL0)
			{
				uwOsdImgIdx = OSD2IMG_BRIGHT_LVL0_ICON;
				break;
			}
			uwOsdImgIdx = OSD2IMG_BRIGHT_LVL0_ICON+(--tUI_CuSetting.BriLvL.tBL_UpdateLvL);
			LCD_BACKLIGHT_CTRL(ulUI_BLTable[tUI_CuSetting.BriLvL.tBL_UpdateLvL]);
			break;
		}
		case ENTER_ARROW:
			if(UI_PHOTOCAP_MODE == tUI_CuSetting.tVdoMode)
			{
				KNL_RecordAct_t tUI_CapAct = {KNL_RECORDFUNC_DISABLE, NULL,};

				tUI_CapAct.tRecordFunc  	= KNL_PHOTO_CAPTURE;
				tUI_CapAct.pRecordStsNtyCb 	= UI_PhotoCaptureFinish;
				tKNL_ExecRecordFunc(tUI_CapAct);
			}
            else if((UI_RECORDING_MODE == tUI_CuSetting.tVdoMode) && (REC_MANUAL == tUI_CuSetting.RecInfo.tREC_Mode ))
            {
                UI_VideoRecordingExec(((UI_REC_START == tUI_RecPlayAct.tRecAct)?UI_REC_STOP:UI_REC_START));
            }
			return;
		default:
			return;
	}
	tOSD_GetOsdImgInfor(1, OSD_IMG2, uwOsdImgIdx, 1, &tOsdImgInfo);
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
	tUI_State = UI_SHOWSTSICON_STATE;
}
//------------------------------------------------------------------------------
void UI_DrawMenuPage(void)
{
	uint16_t uwMenuOsdImg[UI_MENUICON_NUM] = {OSD2IMG_CAMNOR_MENUICON,      OSD2IMG_PAIRNOR_MENUICON, OSD2IMG_RECNOR_MENUICON,
											  OSD2IMG_PHOTONOR_MENUICON,    OSD2IMG_PLYNOR_MENUICON,  OSD2IMG_PSNOR_MENUICON,
											  OSD2IMG_SETTINGNOR_MENUICON};
	uint16_t uwDisplayImgIdx = 0;
	uint8_t i;
	OSD_IMG_INFO tOsdImgInfo[UI_MENUICON_NUM*4];

	UI_ClearCamConnectStatusFlag();
	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_MENU, 1, &tOsdImgInfo[0]);
	tOsdImgInfo[0].uwXStart = tOsdImgInfo[0].uwYStart = 0;
	tOSD_Img1(&tOsdImgInfo[0], OSD_QUEUE);
	uwMenuOsdImg[tUI_MenuItem.ubItemIdx] += UI_ICON_HIGHLIGHT;
	if(tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_CAMNOR_MENUICON, (UI_MENUICON_NUM*4), &tOsdImgInfo[0]) != OSD_OK)
	{
		printd(DBG_ErrorLvl, "Load OSD Image FAIL, pls check (%d) !\n", __LINE__);
		return;
	}
	for(i = 0; i < UI_MENUICON_NUM; i++)
	{
		uwDisplayImgIdx = uwMenuOsdImg[i] - OSD2IMG_CAMNOR_MENUICON;
		tOSD_Img2(&tOsdImgInfo[uwDisplayImgIdx], OSD_QUEUE);
		uwDisplayImgIdx += UI_WRICON_OFFSET;
		tOSD_Img2(&tOsdImgInfo[uwDisplayImgIdx], OSD_QUEUE);
	}
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MENUWR_ICON, 1, &tOsdImgInfo[0]);	
	tOSD_Img2(&tOsdImgInfo[0], OSD_UPDATE);
	OSD_Weight(OSD_WEIGHT_8DIV8);
}
//------------------------------------------------------------------------------
void UI_Menu(UI_ArrowKey_t tArrowKey)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint16_t uwMenuOsdImg[UI_MENUICON_NUM] = {OSD2IMG_CAMNOR_MENUICON,      OSD2IMG_PAIRNOR_MENUICON, OSD2IMG_RECNOR_MENUICON,
											  OSD2IMG_PHOTONOR_MENUICON,    OSD2IMG_PLYNOR_MENUICON,  OSD2IMG_PSNOR_MENUICON,
											  OSD2IMG_SETTINGNOR_MENUICON};
	switch(tArrowKey)
	{
		case UP_ARROW:
			if(tUI_MenuItem.ubItemIdx < PLAYBACK_ITEM)
				return;
			tUI_MenuItem.ubItemPreIdx = tUI_MenuItem.ubItemIdx;
			tUI_MenuItem.ubItemIdx 	 -= PLAYBACK_ITEM;
			break;
		case DOWN_ARROW:
			if(tUI_MenuItem.ubItemIdx >= PHOTO_ITEM)
				return;
			tUI_MenuItem.ubItemPreIdx = tUI_MenuItem.ubItemIdx;
			tUI_MenuItem.ubItemIdx 	 += PLAYBACK_ITEM;
			break;
		case LEFT_ARROW:
			if((tUI_MenuItem.ubItemIdx == CAMERAS_ITEM) ||
			   (tUI_MenuItem.ubItemIdx == PLAYBACK_ITEM))
				return;
			tUI_MenuItem.ubItemPreIdx = tUI_MenuItem.ubItemIdx;
			tUI_MenuItem.ubItemIdx--;
			break;
		case RIGHT_ARROW:
			if((tUI_MenuItem.ubItemIdx == PHOTO_ITEM) ||
			   (tUI_MenuItem.ubItemIdx == SETTING_ITEM))
				return;
			tUI_MenuItem.ubItemPreIdx = tUI_MenuItem.ubItemIdx;
			tUI_MenuItem.ubItemIdx++;
			break;
		case ENTER_ARROW:
			//! Check tUI_MenuItem.ubItemIdx
			//! Draw Sub menu page (if record and photo page, check select camera number
			UI_SubMenu(ENTER_ARROW);
		default:
			return;
	}
	tOSD_GetOsdImgInfor(1, OSD_IMG2, uwMenuOsdImg[tUI_MenuItem.ubItemPreIdx], 1, &tOsdImgInfo);		
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	tOSD_GetOsdImgInfor(1, OSD_IMG2, uwMenuOsdImg[tUI_MenuItem.ubItemPreIdx]+UI_WRICON_OFFSET, 1, &tOsdImgInfo);			
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	//! Draw highlight icon
	uwMenuOsdImg[tUI_MenuItem.ubItemIdx] += UI_ICON_HIGHLIGHT;
	tOSD_GetOsdImgInfor(1, OSD_IMG2, uwMenuOsdImg[tUI_MenuItem.ubItemIdx], 1, &tOsdImgInfo);
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	tOSD_GetOsdImgInfor(1, OSD_IMG2, uwMenuOsdImg[tUI_MenuItem.ubItemIdx]+UI_WRICON_OFFSET, 1, &tOsdImgInfo);
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}
//------------------------------------------------------------------------------
#define CAMS_SET_ITEM_OFFSET	50
void UI_DrawCamsSubMenuPage(void)
{
	UI_CamNum_t tCamSelNum = tCamSelect.tCamNum4CamSetSub, tCamNum;
	OSD_IMG_INFO tOsdImgInfo[26];
	uint16_t uwCamOsdImg[CAM_4T] = {OSD2IMG_RECCAM1NOR_ICON, OSD2IMG_RECCAM2NOR_ICON, OSD2IMG_RECCAM3NOR_ICON, OSD2IMG_RECCAM4NOR_ICON};
	uint16_t uwDisplayImgIdx[8]  = {OSD2IMG_ANRNOR_ITEM,   OSD2IMG_THREEDNRNOR_ITEM, OSD2IMG_vLDSNOR_ITEM, 
	                                OSD2IMG_BUAECNOR_ITEM, OSD2IMG_DISNOR_ITEM, OSD2IMG_CBRNOR_ITEM,
								    OSD2IMG_CONDENSENOR_ITEM, OSD2IMG_FLICKERNOR_ITEM};
	uint16_t uwCamModeImgIdx[7]  = {OSD2IMG_CAMSOFFWR_ICON + tUI_CamStatus[tCamNum].tCamAnrMode,
								    OSD2IMG_CAMSOFFWR_ICON + tUI_CamStatus[tCamNum].tCam3DNRMode,
								    OSD2IMG_CAMSOFFWR_ICON + tUI_CamStatus[tCamNum].tCamvLDCMode,
								    OSD2IMG_CAMSOFFWR_ICON + tUI_CamStatus[tCamNum].tCamAecMode,
								    OSD2IMG_CAMSOFFWR_ICON + tUI_CamStatus[tCamNum].tCamDisMode,
									OSD2IMG_CAMSOFFWR_ICON + tUI_CamStatus[tCamNum].tCamCbrMode,
									OSD2IMG_CAMSOFFWR_ICON + tUI_CamStatus[tCamNum].tCamCondenseMode};
									
	uint16_t uwCamImgIdx, uwModeImgIdx, uwXStart[2];
	uint8_t i;

	if(tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_RECCAM1NOR_ICON, (tUI_CuSetting.ubTotalCamNum*2), &tOsdImgInfo[0]) != OSD_OK)
	{
		printd(DBG_ErrorLvl, "Load OSD Image FAIL, pls check (%d) !\n", __LINE__);
		return;
	}
	uwCamOsdImg[tCamSelNum] += UI_ICON_HIGHLIGHT;
	for(tCamNum = CAM1; tCamNum < tUI_CuSetting.ubTotalCamNum; tCamNum++)
	{
		uwCamImgIdx = uwCamOsdImg[tCamNum] - OSD2IMG_RECCAM1NOR_ICON;
		tOSD_Img2(&tOsdImgInfo[uwCamImgIdx], OSD_QUEUE);
	}
	if(tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_CAMSOPTMARKNOR_ICON, (OSD2IMG_FLICKERHL_ITEM-OSD2IMG_CAMSOPTMARKNOR_ICON+1), &tOsdImgInfo[0]) != OSD_OK)
	{
		printd(DBG_ErrorLvl, "Load OSD Image FAIL, pls check (%d) !\n", __LINE__);
		return;
	}
	uwXStart[0] = tOsdImgInfo[0].uwXStart;
	for(i = 0; i < (CAMSITEM_MAX - 2); i++)
	{
		uwDisplayImgIdx[i] -= OSD2IMG_ANRNOR_ITEM;
		tOSD_Img2(&tOsdImgInfo[uwDisplayImgIdx[i] + 10], OSD_QUEUE);
		uwModeImgIdx = uwCamModeImgIdx[i] - OSD2IMG_CAMSOPTMARKNOR_ICON;
		uwXStart[1]  = tOsdImgInfo[uwModeImgIdx].uwXStart;
		tOsdImgInfo[uwModeImgIdx].uwXStart += (i * CAMS_SET_ITEM_OFFSET);
		tOSD_Img2(&tOsdImgInfo[uwModeImgIdx], OSD_QUEUE);
		tOsdImgInfo[uwModeImgIdx].uwXStart  = uwXStart[1];
		tOsdImgInfo[0].uwXStart += ( i * CAMS_SET_ITEM_OFFSET);
		tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);
		tOsdImgInfo[0].uwXStart  = uwXStart[0];
	}
	uwDisplayImgIdx[7] -= OSD2IMG_ANRNOR_ITEM;	//! CAMSFLICKER_ITEM - 1
	tOSD_Img2(&tOsdImgInfo[uwDisplayImgIdx[7] + 10], OSD_QUEUE);
	tOSD_Img2(&tOsdImgInfo[((tUI_CamStatus[tCamSelNum].tCamFlicker == CAMFLICKER_50HZ)?0:1) + 6], OSD_QUEUE);
	tOsdImgInfo[0].uwXStart += (CAMS_SET_ITEM_OFFSET * 7);
	tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_CAMS_SUBMENUICON, 1, &tOsdImgInfo[0]);
	tOSD_Img2(&tOsdImgInfo[0], OSD_UPDATE);
}
//------------------------------------------------------------------------------
void UI_DrawPairingSubMenuPage(void)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint16_t uwSubMenuItemOsdImg[PAIRITEM_MAX] = {OSD2IMG_PAIRCAMHL_ITEM, OSD2IMG_DELCAMNOR_ITEM};
	uint8_t i;
	for(i = 0; i < PAIRITEM_MAX; i++)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, uwSubMenuItemOsdImg[i], 1, &tOsdImgInfo);		
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	}
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PAIR_SUBMENUICON, 1, &tOsdImgInfo);	
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}
//------------------------------------------------------------------------------
void UI_DrawRecordSubMenuPage(void)
{
	OSD_IMG_INFO tOsdImgInfo[15];
	uint16_t uwDisplayImgIdx = 0;

	if(tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SDFNOR_ITEM, 7, &tOsdImgInfo[0]) != OSD_OK)
	{
		printd(DBG_ErrorLvl, "Load OSD Image FAIL, pls check (%d) !\n", __LINE__);
		return;
	}
	tOSD_Img2(&tOsdImgInfo[2], OSD_QUEUE);
	tOSD_Img2(&tOsdImgInfo[4], OSD_QUEUE);
	tOSD_Img2(&tOsdImgInfo[5], OSD_QUEUE);
	tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);
	if(tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_REC1LOOPWR_ICON, 8, &tOsdImgInfo[7]) != OSD_OK)
	{
		printd(DBG_ErrorLvl, "Load OSD Image FAIL, pls check (%d) !\n", __LINE__);
		return;
	}
	uwDisplayImgIdx = 7 + tUI_CuSetting.RecInfo.tREC_Mode;
	tOSD_Img2(&tOsdImgInfo[uwDisplayImgIdx], OSD_QUEUE);
	uwDisplayImgIdx = 11 + tUI_CuSetting.RecInfo.tREC_Time;
	tOSD_Img2(&tOsdImgInfo[uwDisplayImgIdx], OSD_QUEUE);
	if(tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_REC1OPTMARKNOR_ICON, 2, &tOsdImgInfo[0]) != OSD_OK)
	{
		printd(DBG_ErrorLvl, "Load OSD Image FAIL, pls check (%d) !\n", __LINE__);
		return;
	}
	tOSD_Img2(&tOsdImgInfo[1], OSD_QUEUE);
	tOsdImgInfo[0].uwXStart += 0x40;
	tOSD_Img2(&tOsdImgInfo[0], OSD_UPDATE);
}
//------------------------------------------------------------------------------
void UI_DrawPhotoSubMenuPage(void)
{
	uint16_t uwCamOsdImg[CAM_4T] = {OSD2IMG_RECCAM1NOR_ICON, OSD2IMG_RECCAM2NOR_ICON, OSD2IMG_RECCAM3NOR_ICON, OSD2IMG_RECCAM4NOR_ICON};
	OSD_IMG_INFO tOsdImgInfo[OSD2IMG_PRES12MHL_ICON - OSD2IMG_PHOTO_SUBMENUICON + 1] = {0};
	UI_CamNum_t tCamSelNum = tCamSelect.tCamNum4PhotoSub, tCamNum;
	uint16_t uwDisplayImgIdx[5]  = {0, (OSD2IMG_PHOTOMODENOR_ITEM-OSD2IMG_PHOTO_SUBMENUICON), ((OSD2IMG_PHOTOOFFWR_ICON-OSD2IMG_PHOTO_SUBMENUICON)+tUI_CamStatus[tCamSelNum].tPHOTO_Func),
								   (OSD2IMG_PRESNOR_ITEM-OSD2IMG_PHOTO_SUBMENUICON), ((OSD2IMG_PRES3MWR_ICON-OSD2IMG_PHOTO_SUBMENUICON)+tUI_CamStatus[tCamSelNum].tPHOTO_Resolution)};
	uint8_t i;

	if(tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_RECCAM1NOR_ICON, (CAM_4T*2), &tOsdImgInfo[0]) != OSD_OK)
	{
		printd(DBG_ErrorLvl, "Load OSD Image FAIL, pls check (%d) !\n", __LINE__);
		return;
	}
	uwCamOsdImg[tCamSelNum] += UI_ICON_HIGHLIGHT;
	for(tCamNum = CAM1; tCamNum < CAM_4T; tCamNum++)
	{
		uwDisplayImgIdx[0] = uwCamOsdImg[tCamNum] - OSD2IMG_RECCAM1NOR_ICON;
		tOSD_Img2(&tOsdImgInfo[uwDisplayImgIdx[0]], OSD_QUEUE);
	}
	if(tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PHOTO_SUBMENUICON, (OSD2IMG_PRES12MHL_ICON-OSD2IMG_PHOTO_SUBMENUICON+1), &tOsdImgInfo[0]) != OSD_OK)
	{
		printd(DBG_ErrorLvl, "Load OSD Image FAIL, pls check (%d) !\n", __LINE__);
		return;
	}
	for(i = 1; i < 6; i++)
		tOSD_Img2(&tOsdImgInfo[uwDisplayImgIdx[i]], OSD_QUEUE);
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_OPTMARKNOR_ICON, 1, &tOsdImgInfo[1]);
	tOSD_Img2(&tOsdImgInfo[1], OSD_QUEUE);
	tOsdImgInfo[1].uwXStart += 0x40;
	tOSD_Img2(&tOsdImgInfo[1], OSD_QUEUE);
	tOSD_Img2(&tOsdImgInfo[0], OSD_UPDATE);
}
//------------------------------------------------------------------------------
void UI_DrawPlaybackSubMenuPage(void)
{
	ubUI_RecSubMenuFlag = TRUE;
	UI_DrawDCIMFolderMenu();
}
//------------------------------------------------------------------------------
void UI_DrawPowerSaveSubMenuPage(void)
{
	OSD_IMG_INFO tOsdImgInfo;
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PS_SUBMENUICON, 1, &tOsdImgInfo);
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}
//------------------------------------------------------------------------------
void UI_DrawSettingSubMenuPage(void)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint16_t uwSubMenuItemOsdImg[SETTINGITEM_MAX] = {OSD2IMG_DTHL_ITEM, OSD2IMG_AECNOR_ITEM, OSD2IMG_CCANOR_ITEM,
	                                                 OSD2IMG_STORAGENOR_ITEM, OSD2IMG_LANGUAGENOR_ITEM, OSD2IMG_DEFUNOR_ITEM, OSD2IMG_SWUSBDMODENOR_ITEM};
	uint8_t i;
	for(i = 0; i < SETTINGITEM_MAX; i++)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, uwSubMenuItemOsdImg[i], 1, &tOsdImgInfo);
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	}
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_AECOFFWR_ICON+tUI_CuSetting.ubAEC_Mode, 1, &tOsdImgInfo);
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_AECOFFWR_ICON+tUI_CuSetting.ubCCA_Mode, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart += 65;
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_STORAGATBU_ICON+tUI_CuSetting.ubSTORAGE_Mode, 1, &tOsdImgInfo);
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_OPTMARKNOR_ICON, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart -= 0xE;
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	tOsdImgInfo.uwXStart += (0xE + 0x32);
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SETTING_SUBMENUICON, 1, &tOsdImgInfo);
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}
//------------------------------------------------------------------------------
void UI_DrawSubMenuPage(UI_MenuItemList_t MenuItem)
{
	const static UI_DrawSubMenuFuncPtr_t DrawSubMenuFunc[MENUITEM_MAX] =
	{
		UI_DrawCamsSubMenuPage,
		UI_DrawPairingSubMenuPage,
		UI_DrawRecordSubMenuPage,
		NULL,						//! UI_DrawPhotoSubMenuPage,
		UI_DrawPlaybackSubMenuPage,
		UI_DrawPowerSaveSubMenuPage,
		UI_DrawSettingSubMenuPage
	};
	OSD_IMG_INFO tOsdImgInfo;

	if(NULL == DrawSubMenuFunc[MenuItem].pvFuncPtr)
		return;
	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_SUBMENU, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart = 0;
	tOsdImgInfo.uwYStart = 0;
	tOSD_Img1(&tOsdImgInfo, OSD_QUEUE);
	DrawSubMenuFunc[MenuItem].pvFuncPtr();
	tUI_State = UI_SUBMENU_STATE;
}
//------------------------------------------------------------------------------
void UI_SubMenu(UI_ArrowKey_t tArrowKey)
{
	const static UI_MenuFuncPtr_t SubMenuFunc[MENUITEM_MAX] =
	{
		UI_CameraSettingSubMenuPage,
		UI_PairingSubMenuPage,
		UI_RecordSubMenuPage,
		NULL,
		UI_PlaybackSubMenuPage,
		UI_PowerSaveSubMenuPage,
		UI_SettingSubMenuPage,
	};
	if(SubMenuFunc[tUI_MenuItem.ubItemIdx].pvFuncPtr)
		SubMenuFunc[tUI_MenuItem.ubItemIdx].pvFuncPtr(tArrowKey);
}
//------------------------------------------------------------------------------
void UI_SubSubMenu(UI_ArrowKey_t tArrowKey)
{
	const static UI_MenuFuncPtr_t SubSubMenuFunc[MENUITEM_MAX] =
	{
		NULL,
		UI_PairingSubSubMenuPage,
		UI_RecordSubSubMenuPage,
		UI_PhotoSubSubMenuPage,
		NULL,
		NULL,
		UI_SettingSubSubMenuPage
	};
	if(SubSubMenuFunc[tUI_MenuItem.ubItemIdx].pvFuncPtr)
		SubSubMenuFunc[tUI_MenuItem.ubItemIdx].pvFuncPtr(tArrowKey);
}
//------------------------------------------------------------------------------
void UI_SubSubSubMenu(UI_ArrowKey_t tArrowKey)
{
	switch(tUI_MenuItem.ubItemIdx)
	{
		case PAIRING_ITEM:
			UI_PairingSubSubSubMenuPage(tArrowKey);
			break;
		case SETTING_ITEM:
			UI_SettingSysDateTimeSubSubMenuPage(tArrowKey);
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_CameraSettingSubMenuPage(UI_ArrowKey_t tArrowKey)
{
	UI_CamsSubMenuItemList_t tSubMenuItem = (UI_CamsSubMenuItemList_t)tUI_SubMenuItem[CAMERAS_ITEM].tSubMenuInfo.ubItemIdx;
	UI_MenuAct_t tMenuAct;
	OSD_IMG_INFO tOsdImgInfo[8];
	UI_CamNum_t tCamSelNum;
	UI_CamsSetMode_t tCamsModeSts[CAMSITEM_MAX];

	if(tUI_State == UI_MAINMENU_STATE)
	{
		//! Draw Cameras sub menu page
		UI_DrawSubMenuPage(CAMERAS_ITEM);
		return;
	}
	if(tSubMenuItem == CAMSSELCAM_ITEM)
	{
		if((tArrowKey == LEFT_ARROW) || (tArrowKey == RIGHT_ARROW))
		{
			UI_CamNum_t tPreCamNum = tCamSelect.tCamNum4CamSetSub;
			UI_CamNum_t tNexCamNum = NO_CAM;
			tNexCamNum = UI_ChangeSelectCamNum4UiMenu(&tCamSelect.tCamNum4CamSetSub, &tArrowKey);
			if(tNexCamNum < tUI_CuSetting.ubTotalCamNum)
			{
				uint16_t uwTmpXStart = 0;
				uint8_t i;
				//! Change camera number
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_RECCAM1NOR_ICON+(tPreCamNum*2), 1, &tOsdImgInfo[0]);
				tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_RECCAM1HL_ICON+(tNexCamNum*2), 1, &tOsdImgInfo[0]);
				tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_CAMSOFFWR_ICON, 2, &tOsdImgInfo[0]);
				tCamsModeSts[CAMSANR_ITEM] 		= tUI_CamStatus[tNexCamNum].tCamAnrMode;
				tCamsModeSts[CAMS3DNR_ITEM] 	= tUI_CamStatus[tNexCamNum].tCam3DNRMode;
				tCamsModeSts[CAMSvLDS_ITEM] 	= tUI_CamStatus[tNexCamNum].tCamvLDCMode;
				tCamsModeSts[CAMSAEC_ITEM] 		= tUI_CamStatus[tNexCamNum].tCamAecMode;
				tCamsModeSts[CAMSDIS_ITEM] 		= tUI_CamStatus[tNexCamNum].tCamDisMode;
				tCamsModeSts[CAMSCBR_ITEM] 		= tUI_CamStatus[tNexCamNum].tCamCbrMode;
				tCamsModeSts[CAMSCONDENSE_ITEM] = tUI_CamStatus[tNexCamNum].tCamCondenseMode;
				for(i = CAMSANR_ITEM; i <= CAMSCONDENSE_ITEM; i++)
				{
					uwTmpXStart = tOsdImgInfo[tCamsModeSts[i]].uwXStart;
					tOsdImgInfo[tCamsModeSts[i]].uwXStart += (i - CAMSANR_ITEM) * CAMS_SET_ITEM_OFFSET;
					tOSD_Img2(&tOsdImgInfo[tCamsModeSts[i]], OSD_QUEUE);
					tOsdImgInfo[tCamsModeSts[i]].uwXStart = uwTmpXStart;
				}
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_CAMS50WR_ICON, 2, &tOsdImgInfo[0]);
				tOSD_Img2(&tOsdImgInfo[(tUI_CamStatus[tNexCamNum].tCamFlicker == CAMFLICKER_50HZ)?0:1], OSD_UPDATE);
				tCamSelect.tCamNum4CamSetSub = tNexCamNum;
			}
			else if((tArrowKey == LEFT_ARROW) && (TRUE == ubUI_FastStateFlag))
			{
				OSD_IMG_INFO tOsdInfo;

				tOsdInfo.uwHSize  = 600;
				tOsdInfo.uwVSize  = 1180;
				tOsdInfo.uwXStart = 100;
				tOsdInfo.uwYStart = 50;
				OSD_EraserImg2(&tOsdInfo);
				ubUI_FastStateFlag = FALSE;
				UI_DrawDCIMFolderMenu();
			}
			return;
		}
	}
	tCamSelNum = tCamSelect.tCamNum4CamSetSub;
	if((tUI_CamStatus[tCamSelNum].ulCAM_ID == INVALID_ID) ||
	   (tUI_CamStatus[tCamSelNum].tCamConnSts == CAM_OFFLINE))
		return;
	tCamsModeSts[CAMSANR_ITEM] 		= tUI_CamStatus[tCamSelNum].tCamAnrMode;
	tCamsModeSts[CAMS3DNR_ITEM] 	= tUI_CamStatus[tCamSelNum].tCam3DNRMode;
	tCamsModeSts[CAMSvLDS_ITEM] 	= tUI_CamStatus[tCamSelNum].tCamvLDCMode;
	tCamsModeSts[CAMSAEC_ITEM] 		= tUI_CamStatus[tCamSelNum].tCamAecMode;
	tCamsModeSts[CAMSDIS_ITEM] 		= tUI_CamStatus[tCamSelNum].tCamDisMode;
	tCamsModeSts[CAMSCBR_ITEM] 		= tUI_CamStatus[tCamSelNum].tCamCbrMode;
	tCamsModeSts[CAMSCONDENSE_ITEM] = tUI_CamStatus[tCamSelNum].tCamCondenseMode;
	tMenuAct = UI_KeyEventMap2SubMenuInfo(&tArrowKey, &tUI_SubMenuItem[CAMERAS_ITEM]);
	switch(tMenuAct)
	{
		case DRAW_HIGHLIGHT_MENUICON:
		{
			uint16_t uwSubMenuItemOsdImg[CAMSITEM_MAX] = {OSD2IMG_ANRNOR_ITEM,  OSD2IMG_ANRNOR_ITEM,      OSD2IMG_THREEDNRNOR_ITEM,
														  OSD2IMG_vLDSNOR_ITEM, OSD2IMG_BUAECNOR_ITEM,    OSD2IMG_DISNOR_ITEM, 
														  OSD2IMG_CBRNOR_ITEM,  OSD2IMG_CONDENSENOR_ITEM, OSD2IMG_FLICKERNOR_ITEM};
			uint8_t ubSubMenuItemPreIdx = tUI_SubMenuItem[CAMERAS_ITEM].tSubMenuInfo.ubItemPreIdx;
			uint8_t ubSubMenuItemIdx 	= tUI_SubMenuItem[CAMERAS_ITEM].tSubMenuInfo.ubItemIdx;

			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_CAMSOFFWR_ICON, 8, &tOsdImgInfo[0]);
			if((ubSubMenuItemIdx != CAMSANR_ITEM) || (ubSubMenuItemPreIdx == CAMS3DNR_ITEM))
			{
				if(ubSubMenuItemPreIdx == CAMSFLICKER_ITEM)
				{
					tOSD_Img2(&tOsdImgInfo[(4 + ((tUI_CamStatus[tCamSelNum].tCamFlicker == CAMFLICKER_50HZ)?0:1))], OSD_QUEUE);
				}
				else
				{
					tOsdImgInfo[tCamsModeSts[ubSubMenuItemPreIdx]].uwXStart += (ubSubMenuItemPreIdx - 1) * CAMS_SET_ITEM_OFFSET;
					tOSD_Img2(&tOsdImgInfo[tCamsModeSts[ubSubMenuItemPreIdx]], OSD_QUEUE);
				}
			}
			if(ubSubMenuItemIdx > CAMSSELCAM_ITEM)
			{
				if(ubSubMenuItemIdx == CAMSFLICKER_ITEM)
				{
					tOSD_Img2(&tOsdImgInfo[(6 + ((tUI_CamStatus[tCamSelNum].tCamFlicker == CAMFLICKER_50HZ)?0:1))], OSD_QUEUE);
				}
				else
				{
					tOsdImgInfo[tCamsModeSts[ubSubMenuItemIdx] + 2].uwXStart += (ubSubMenuItemIdx - 1) * CAMS_SET_ITEM_OFFSET;
					tOSD_Img2(&tOsdImgInfo[tCamsModeSts[ubSubMenuItemIdx] + 2], OSD_QUEUE);
				}
			}
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_CAMSOPTMARKNOR_ICON, 2, &tOsdImgInfo[0]);
			if((ubSubMenuItemIdx != CAMSANR_ITEM) || (ubSubMenuItemPreIdx == CAMS3DNR_ITEM))
			{
				tOsdImgInfo[0].uwXStart += (ubSubMenuItemPreIdx - 1) * CAMS_SET_ITEM_OFFSET;
				tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);
			}
			if(ubSubMenuItemIdx > CAMSSELCAM_ITEM)
			{
				tOsdImgInfo[1].uwXStart += (ubSubMenuItemIdx - 1) * CAMS_SET_ITEM_OFFSET;
				tOSD_Img2(&tOsdImgInfo[1], OSD_QUEUE);
			}
			UI_DrawHLandNormalIcon(uwSubMenuItemOsdImg[ubSubMenuItemPreIdx], (uwSubMenuItemOsdImg[ubSubMenuItemIdx] + ((!ubSubMenuItemIdx)?0:UI_ICON_HIGHLIGHT)));
			break;
		}
		case DRAW_MENUPAGE:
			if(tArrowKey == ENTER_ARROW)
			{
				UI_CUReqCmd_t tCamSetCmd;
				UI_CamsSetMode_t tCamsSetMode;
				UI_CamFlicker_t tCamsFlicker = tUI_CamStatus[tCamSelNum].tCamFlicker;
				uint8_t ubSubMenuItemIdx = tUI_SubMenuItem[CAMERAS_ITEM].tSubMenuInfo.ubItemIdx;
				uint8_t tUI_CamsSetItem[CAMSITEM_MAX] = {UI_ADOANR_SETTING,  UI_ADOANR_SETTING, UI_IMG3DNR_SETTING,
														 UI_IMGvLDC_SETTING, UI_ADOAEC_SETTING, UI_IMGDIS_SETTING, 
														 UI_IMGCBR_SETTING, UI_IMGCONDENSE_SETTING, UI_FLICKER_SETTING};
				UI_CamsSetMode_t *pCamsModeSts[CAMSITEM_MAX] = {&tUI_CamStatus[tCamSelNum].tCamAnrMode, &tUI_CamStatus[tCamSelNum].tCamAnrMode,
																&tUI_CamStatus[tCamSelNum].tCam3DNRMode, &tUI_CamStatus[tCamSelNum].tCamvLDCMode,
																&tUI_CamStatus[tCamSelNum].tCamAecMode, &tUI_CamStatus[tCamSelNum].tCamDisMode,
															    &tUI_CamStatus[tCamSelNum].tCamCbrMode, &tUI_CamStatus[tCamSelNum].tCamCondenseMode};

				if((ubSubMenuItemIdx == CAMSSELCAM_ITEM) || (ubSubMenuItemIdx == CAMSDIS_ITEM) ||				   
				   (ubSubMenuItemIdx == CAMSCBR_ITEM) || (ubSubMenuItemIdx == CAMSCONDENSE_ITEM))
					break;
				tCamSetCmd.tDS_CamNum 				= tCamSelNum;
				tCamSetCmd.ubCmd[UI_TWC_TYPE]		= UI_SETTING;
				tCamSetCmd.ubCmd[UI_SETTING_ITEM]   = (ubSubMenuItemIdx == CAMSANR_ITEM)?UI_ADOANR_SETTING:(ubSubMenuItemIdx == CAMSAEC_ITEM)?UI_ADOAEC_SETTING:UI_IMGPROC_SETTING;
				if(ubSubMenuItemIdx == CAMSFLICKER_ITEM)
				{
					tCamsFlicker  						= (CAMFLICKER_50HZ == tUI_CamStatus[tCamSelNum].tCamFlicker)?CAMFLICKER_60HZ:CAMFLICKER_50HZ;
					tCamSetCmd.ubCmd[UI_SETTING_DATA]   = UI_FLICKER_SETTING;
					tCamSetCmd.ubCmd[UI_SETTING_DATA+1] = tCamsFlicker;
				}
				else
				{
					tCamsSetMode  						= (CAMSET_OFF == tCamsModeSts[ubSubMenuItemIdx])?CAMSET_ON:CAMSET_OFF;
					tCamSetCmd.ubCmd[UI_SETTING_DATA]   = ((ubSubMenuItemIdx == CAMSANR_ITEM) || (ubSubMenuItemIdx == CAMSAEC_ITEM))?tCamsSetMode:tUI_CamsSetItem[ubSubMenuItemIdx];
					tCamSetCmd.ubCmd[UI_SETTING_DATA+1] = ((ubSubMenuItemIdx == CAMSANR_ITEM) || (ubSubMenuItemIdx == CAMSAEC_ITEM))?0:tCamsSetMode;
				}
				tCamSetCmd.ubCmd_Len  				= ((ubSubMenuItemIdx == CAMSANR_ITEM) || (ubSubMenuItemIdx == CAMSAEC_ITEM))?3:4;
				if(ubSubMenuItemIdx == CAMSFLICKER_ITEM)
				{
					tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_CAMS50HL_ICON, 2, &tOsdImgInfo[0]);
					tOSD_Img2(&tOsdImgInfo[tCamsFlicker], OSD_UPDATE);
				}
				else
				{
					tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_CAMSOFFHL_ICON, 2, &tOsdImgInfo[0]);
					tOsdImgInfo[tCamsSetMode].uwXStart += (ubSubMenuItemIdx - 1) * CAMS_SET_ITEM_OFFSET;
					tOSD_Img2(&tOsdImgInfo[tCamsSetMode], OSD_UPDATE);
				}
				if(UI_SendRequestToCAM(osThreadGetId(), &tCamSetCmd) != rUI_SUCCESS)
				{
					printd(DBG_ErrorLvl, "Camera Setting fail !\n");
					if(ubSubMenuItemIdx == CAMSFLICKER_ITEM)
					{
						tOSD_Img2(&tOsdImgInfo[tCamsFlicker], OSD_UPDATE);
					}
					else
					{
						tOsdImgInfo[tCamsModeSts[ubSubMenuItemIdx]].uwXStart += (ubSubMenuItemIdx - 1) * CAMS_SET_ITEM_OFFSET;
						tOSD_Img2(&tOsdImgInfo[tCamsModeSts[ubSubMenuItemIdx]], OSD_UPDATE);
					}
					return;
				}
				*pCamsModeSts[ubSubMenuItemIdx] 		= tCamsSetMode;
				tUI_CamStatus[tCamSelNum].tCamFlicker   = tCamsFlicker;
				UI_UpdateDevStatusInfo();
			}
			break;
		case EXIT_MENUFUNC:
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_PairingSubMenuPage(UI_ArrowKey_t tArrowKey)
{
	UI_MenuAct_t tMenuAct;

	if(tUI_State == UI_MAINMENU_STATE)
	{
		//! Draw Cameras sub menu page
		UI_DrawSubMenuPage(PAIRING_ITEM);
		return;
	}
	tMenuAct = UI_KeyEventMap2SubMenuInfo(&tArrowKey, &tUI_SubMenuItem[PAIRING_ITEM]);
	switch(tMenuAct)
	{
		case DRAW_HIGHLIGHT_MENUICON:
		{
			uint16_t uwSubMenuItemOsdImg[PAIRITEM_MAX] = {OSD2IMG_PAIRCAMNOR_ITEM, OSD2IMG_DELCAMNOR_ITEM};
			uint8_t ubSubMenuItemPreIdx = tUI_SubMenuItem[PAIRING_ITEM].tSubMenuInfo.ubItemPreIdx;
			uint8_t ubSubMenuItemIdx = tUI_SubMenuItem[PAIRING_ITEM].tSubMenuInfo.ubItemIdx;
			UI_DrawHLandNormalIcon(uwSubMenuItemOsdImg[ubSubMenuItemPreIdx], (uwSubMenuItemOsdImg[ubSubMenuItemIdx] + UI_ICON_HIGHLIGHT));
			break;
		}
		case DRAW_MENUPAGE:
		{
			//! Draw sub sub menu page
			OSD_IMG_INFO tOsdImgInfo, tCamRdyOsdImgInfo = {0};
			uint16_t uwSubMenuItemOsdImg[CAM_4T] = {OSD2IMG_PAIRCAM1NOR_ICON, OSD2IMG_PAIRCAM2NOR_ICON, OSD2IMG_PAIRCAM3NOR_ICON, OSD2IMG_PAIRCAM4NOR_ICON};
			uint8_t ubBuNum = tUI_CuSetting.ubTotalCamNum, i, ubUI_FirstHL = FALSE;
			uint8_t ubSubMenuItemIdx = tUI_SubMenuItem[PAIRING_ITEM].tSubMenuInfo.ubItemIdx;
			uint16_t uwIconOffset = (ubSubMenuItemIdx == DELCAM_ITEM)?80:0;
			uint16_t uwRDY_XStart = 0, uwOsdImgIdx;

			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PAIRRDYMASK_ICON, 1, &tCamRdyOsdImgInfo);
			uwRDY_XStart = tCamRdyOsdImgInfo.uwXStart;
			
		//justin 2020.07.09
		#if (defined(BUC_CU) && APP_DUAL_HOST_ENABLE)
				ubBuNum = KNL_WIRELESS_CAM_NUM;				
		#endif
				
			if(ubSubMenuItemIdx == PAIRCAM_ITEM)
			{
				uint16_t uwItemOffset = ubBuNum * 55;
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DELCAMNOR_ITEM, 1, &tOsdImgInfo);
				OSD_EraserImg2(&tOsdImgInfo);
				tOsdImgInfo.uwXStart += uwItemOffset;
				tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
				for(i = CAM1; i < ubBuNum; i++)
				{
					if(tUI_CamStatus[i].ulCAM_ID == INVALID_ID)
					{
						if(FALSE == ubUI_FirstHL)
						{
							uwSubMenuItemOsdImg[i] += UI_ICON_HIGHLIGHT;
							ubUI_FirstHL = TRUE;
						}
						tOSD_GetOsdImgInfor(1, OSD_IMG2, uwSubMenuItemOsdImg[i], 1, &tOsdImgInfo);
					}
					else
					{
						uwOsdImgIdx = uwSubMenuItemOsdImg[i]+UI_ICON_READY;
						if(FALSE == ubUI_FirstHL)
						{
							uwOsdImgIdx  = uwSubMenuItemOsdImg[i]+UI_ICON_HIGHLIGHT;
							ubUI_FirstHL = TRUE;
						}
						tCamRdyOsdImgInfo.uwXStart = uwRDY_XStart + (i*55);
						tOSD_Img2(&tCamRdyOsdImgInfo, OSD_QUEUE);
						tOSD_GetOsdImgInfor(1, OSD_IMG2, uwOsdImgIdx, 1, &tOsdImgInfo);
					}
					tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
				}
			}
			if(ubSubMenuItemIdx == DELCAM_ITEM)
			{
				for(i = CAM1; i < ubBuNum; i++)
				{
					tOSD_GetOsdImgInfor(1, OSD_IMG2, (uwSubMenuItemOsdImg[i]+((i == CAM1)?UI_ICON_HIGHLIGHT:(tUI_CamStatus[i].ulCAM_ID == INVALID_ID)?UI_ICON_NORMAL:UI_ICON_READY)), 1, &tOsdImgInfo);
					tOsdImgInfo.uwXStart += uwIconOffset;
					tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
					if(tUI_CamStatus[i].ulCAM_ID != INVALID_ID)
					{
						tCamRdyOsdImgInfo.uwXStart = uwIconOffset + uwRDY_XStart + (i*55);
						tOSD_Img2(&tCamRdyOsdImgInfo, OSD_QUEUE);						
					}
				}
			}
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PAIROPTMASK_ICON, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart += uwIconOffset;
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			tUI_State = UI_SUBSUBMENU_STATE;
			break;
		}
		case EXIT_MENUFUNC:
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_PairingUpdateSubSubMenuItemIndex(UI_SubMenuItem_t *ptSubSubMenuItem, uint8_t *Update_Flag)
{
	ptSubSubMenuItem->ubFirstItem 			 = CAM1;
	ptSubSubMenuItem->tSubMenuInfo.ubItemIdx = CAM1;
	*Update_Flag = TRUE;
}
//------------------------------------------------------------------------------
void UI_PairingDrawSubSubMenuItem(UI_PairSubMenuItemList_t *ptSubMenuItem, UI_SubMenuItem_t *ptSubSubMenuItem)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint16_t uwSubSubMenuItemOsdImg[CAM_4T] = {OSD2IMG_PAIRCAM1NOR_ICON, OSD2IMG_PAIRCAM2NOR_ICON, OSD2IMG_PAIRCAM3NOR_ICON, OSD2IMG_PAIRCAM4NOR_ICON};
	uint16_t uwIconOffset = (*ptSubMenuItem == DELCAM_ITEM)?80:0, uwXStart = 0;
	uint8_t ubSubSubMenuItemPreIdx = ptSubSubMenuItem->tSubMenuInfo.ubItemPreIdx;
	uint8_t ubSubSubMenuItemIdx    = ptSubSubMenuItem->tSubMenuInfo.ubItemIdx;
	switch(*ptSubMenuItem)
	{
		case PAIRCAM_ITEM:
		case DELCAM_ITEM:
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PAIROPTMASK_ICON, 1, &tOsdImgInfo);
			uwXStart = tOsdImgInfo.uwXStart;
			tOsdImgInfo.uwXStart += (uwIconOffset + (ubSubSubMenuItemPreIdx * 55));
			OSD_EraserImg2(&tOsdImgInfo);
			tOsdImgInfo.uwXStart = uwXStart + (uwIconOffset + (ubSubSubMenuItemIdx * 55));
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
			tOSD_GetOsdImgInfor(1, OSD_IMG2, (uwSubSubMenuItemOsdImg[ubSubSubMenuItemPreIdx]+((tUI_CamStatus[ubSubSubMenuItemPreIdx].ulCAM_ID != INVALID_ID)?UI_ICON_READY:UI_ICON_NORMAL)), 1, &tOsdImgInfo);								
			tOsdImgInfo.uwXStart += uwIconOffset;
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
			tOSD_GetOsdImgInfor(1, OSD_IMG2, uwSubSubMenuItemOsdImg[ubSubSubMenuItemIdx]+UI_ICON_HIGHLIGHT, 1, &tOsdImgInfo);											
			tOsdImgInfo.uwXStart += uwIconOffset;
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_PairingSubSubMenuPage(UI_ArrowKey_t tArrowKey)
{
	static UI_PairSubSubMenuItem_t tPairSubSubMenuItem =
	{
		{
		   { 0, CAM_4T, { 0, 0 } },
		   { 0, CAM_4T, { 0, 0 } },
		},
	};
	static uint8_t ubUI_PairStsUpdateFlag = FALSE;
	UI_PairSubMenuItemList_t tSubMenuItem = (UI_PairSubMenuItemList_t)tUI_SubMenuItem[PAIRING_ITEM].tSubMenuInfo.ubItemIdx;
	UI_MenuAct_t tMenuAct;
	OSD_IMG_INFO tOsdImgInfo;

	if((FALSE == ubUI_PairStsUpdateFlag) && (tSubMenuItem == PAIRCAM_ITEM))
		UI_PairingUpdateSubSubMenuItemIndex(&tPairSubSubMenuItem.tPairS[tSubMenuItem], &ubUI_PairStsUpdateFlag);
	
	//justin 2020.07.09
	tPairSubSubMenuItem.tPairS[tSubMenuItem].ubItemCount = tUI_CuSetting.ubTotalCamNum;
#if (defined(BUC_CU) && APP_DUAL_HOST_ENABLE)
	tPairSubSubMenuItem.tPairS[tSubMenuItem].ubItemCount = KNL_WIRELESS_CAM_NUM;
#endif
	
	tMenuAct = UI_KeyEventMap2SubSubMenuInfo(&tArrowKey, &tPairSubSubMenuItem.tPairS[tSubMenuItem]);
	switch(tMenuAct)
	{
		case DRAW_HIGHLIGHT_MENUICON:
			UI_PairingDrawSubSubMenuItem(&tSubMenuItem, &tPairSubSubMenuItem.tPairS[tSubMenuItem]);
			break;
		case EXECUTE_MENUFUNC:
		{
			if(tSubMenuItem == PAIRCAM_ITEM)
			{
				if(tUI_CuSetting.ubTotalCamNum == DISPLAY_1T1R)
				{
					APP_EventMsg_t tUI_PairMessage = {0};

					tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PAIRINGSINGLE_ICON, 1, &tOsdImgInfo);
					tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
					tUI_PairMessage.ubAPP_Event 	 = APP_PAIRING_START_EVENT;
					tUI_PairMessage.ubAPP_Message[0] = 2;		//! Message Length
					tUI_PairMessage.ubAPP_Message[1] = tPairInfo.tPairSelCam = CAM1;
					tUI_PairMessage.ubAPP_Message[2] = DISP_1T;
					tUI_PairMessage.ubAPP_Message[3] = FALSE;
					UI_SendMessageToAPP(&tUI_PairMessage);
					tPairInfo.ubDrawFlag 			 = TRUE;
					UI_DisableScanMode();
					tUI_State = UI_PAIRING_STATE;
				}
				else if(tUI_CuSetting.ubTotalCamNum == DISPLAY_2T1R)
				{
					tPairInfo.tDispLocation = DISP_LEFT;
					tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PAIRLEFTWINDOW_ICON, 1, &tOsdImgInfo);
					tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
					tPairInfo.tPairSelCam = (UI_CamNum_t)tPairSubSubMenuItem.tPairS[tSubMenuItem].tSubMenuInfo.ubItemIdx;
					tUI_State = UI_SUBSUBSUBMENU_STATE;
				}
				else if(tUI_CuSetting.ubTotalCamNum == DISPLAY_4T1R)
				{
					tPairInfo.tDispLocation = (KNL_DISP_H == VDO_DISP_TYPE)?DISP_H_L:DISP_UPPER_LEFT;
				//justin 2020.07.09
				#if (defined(BUC_CU) && APP_DUAL_HOST_ENABLE)
					if(VDO_DISP_TYPE == KNL_DISP_3T_3C)
					{
						tPairInfo.tDispLocation = DISP_UPPER_LEFT;
						tOSD_GetOsdImgInfor(1, OSD_IMG2,OSD2IMG_PAIRTRIPWINDOW_ICON, 1, &tOsdImgInfo);
					}
					else
					{
						tOSD_GetOsdImgInfor(1, OSD_IMG2, (KNL_DISP_H == VDO_DISP_TYPE)?OSD2IMG_PAIRHLWINDOW_ICON:OSD2IMG_PAIRLUWINDOW_ICON, 1, &tOsdImgInfo);
					}
				#else					
					tOSD_GetOsdImgInfor(1, OSD_IMG2, (KNL_DISP_H == VDO_DISP_TYPE)?OSD2IMG_PAIRHLWINDOW_ICON:OSD2IMG_PAIRLUWINDOW_ICON, 1, &tOsdImgInfo);
				#endif
					
					tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
				
				//justin 2020.07.09		
				#if (defined(BUC_CU) && APP_DUAL_HOST_ENABLE)
					if(KNL_WIRELESS_CAM_NUM == 1)
						tPairInfo.tPairSelCam = CAM1;						
				#else
					tPairInfo.tPairSelCam = (UI_CamNum_t)tPairSubSubMenuItem.tPairS[tSubMenuItem].tSubMenuInfo.ubItemIdx;
				#endif
					tUI_State = UI_SUBSUBSUBMENU_STATE;
				}
			}
			if(tSubMenuItem == DELCAM_ITEM)
			{
				OSD_IMG_INFO tDelOsdInfo;
				uint16_t uwSubMenuItemOsdImg[CAM_4T] = {OSD2IMG_PAIRCAM1HL_ICON, OSD2IMG_PAIRCAM2HL_ICON, OSD2IMG_PAIRCAM3HL_ICON, OSD2IMG_PAIRCAM4HL_ICON};
				uint16_t uwIconOffset = 80;
				UI_CamNum_t tUI_DelCam = (UI_CamNum_t)tPairSubSubMenuItem.tPairS[tSubMenuItem].tSubMenuInfo.ubItemIdx, tSwCamNum;

				if(INVALID_ID == tUI_CamStatus[tUI_DelCam].ulCAM_ID)
					break;
				tUI_CuSetting.tAdoSrcCamNum  = (tUI_CuSetting.tAdoSrcCamNum == tUI_DelCam)?NO_CAM:tUI_CuSetting.tAdoSrcCamNum;
				if(tCamViewSel.tCamViewPool[0] == tUI_DelCam)
				{
					for(tSwCamNum = CAM1; tSwCamNum < tUI_CuSetting.ubTotalCamNum; tSwCamNum++)
					{
					#ifdef RTC676x
						if((tSwCamNum != tUI_DelCam) &&
						   (tUI_CamStatus[tSwCamNum].ulCAM_ID != INVALID_ID))
					#else
						if((tSwCamNum != tUI_DelCam) &&
						   (tUI_CamStatus[tSwCamNum].ulCAM_ID != INVALID_ID) &&
						   (tUI_CamStatus[tSwCamNum].tCamConnSts == CAM_ONLINE))
					#endif
						{
							UI_ClearCamConnectStatusFlag();
							if((SINGLE_VIEW == tCamViewSel.tCamViewType) &&
							   (tCamViewSel.tCamViewPool[0] != tSwCamNum))
							{
								tCamViewSel.tCamViewPool[0] = tSwCamNum;
								tUI_CuSetting.tAdoSrcCamNum = tSwCamNum;
								UI_SwitchCameraSource();
								break;
							}
							else
							{
								UI_SwitchAudioSource(tSwCamNum);
								break;
							}
						}
					}
				}
				UI_UnBindCam(tUI_DelCam);
				tOSD_GetOsdImgInfor(1, OSD_IMG2, uwSubMenuItemOsdImg[tUI_DelCam], 1, &tOsdImgInfo);
				tDelOsdInfo.uwHSize  = 50;
				tDelOsdInfo.uwVSize  = 80;
				tDelOsdInfo.uwXStart = tOsdImgInfo.uwXStart + uwIconOffset;
				tDelOsdInfo.uwYStart = tOsdImgInfo.uwYStart - 80;
				OSD_EraserImg2(&tDelOsdInfo);
				tOsdImgInfo.uwXStart += uwIconOffset;
				tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			}
			break;
		}
		case EXIT_MENUFUNC:
		{
			OSD_IMG_INFO tOsdImgInfo;
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PAIRCAM1NOR_ICON, 1, &tOsdImgInfo);
			if(tSubMenuItem == PAIRCAM_ITEM)
			{
				tOsdImgInfo.uwHSize   = 335;
				tOsdImgInfo.uwVSize   = 450;
				tOsdImgInfo.uwYStart -= 80;
				OSD_EraserImg2(&tOsdImgInfo);
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DELCAMNOR_ITEM, 1, &tOsdImgInfo);
				tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			}
			if(tSubMenuItem == DELCAM_ITEM)
			{
				tOsdImgInfo.uwHSize   = 335;
				tOsdImgInfo.uwVSize   = 330;
				tOsdImgInfo.uwXStart += 80;
				tOsdImgInfo.uwYStart -= 80;
				OSD_EraserImg2(&tOsdImgInfo);
			}
			memset(&tPairSubSubMenuItem.tPairS[tSubMenuItem].tSubMenuInfo, 0, sizeof(UI_MenuItem_t));
			ubUI_PairStsUpdateFlag = FALSE;
			tUI_State = UI_SUBMENU_STATE;
			break;
		}
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_PairingSubSubSubMenuPage(UI_ArrowKey_t tArrowKey)
{
#if (defined(BUC_CU) && APP_DUAL_HOST_ENABLE)
	OSD_IMG_INFO tFixLocateImgInfo;
#endif
	OSD_IMG_INFO tOsdImgInfo;
	uint16_t uwPairWinOsdImgIdx;

	switch(tArrowKey)
	{
		case UP_ARROW:
			if(tUI_CuSetting.ubTotalCamNum == DISPLAY_4T1R)
			{
				if((tPairInfo.tDispLocation == DISP_UPPER_LEFT) || (tPairInfo.tDispLocation == DISP_UPPER_RIGHT))
					return;
				if((tPairInfo.tDispLocation == DISP_H_L) || (tPairInfo.tDispLocation == DISP_H_R) || (tPairInfo.tDispLocation == DISP_H_CU))
					return;				
				tPairInfo.tDispLocation = (KNL_DISP_H == VDO_DISP_TYPE)?DISP_H_CU:(tPairInfo.tDispLocation == DISP_LOWER_LEFT)?DISP_UPPER_LEFT:DISP_UPPER_RIGHT;
				break;
			}
			return;
		case DOWN_ARROW:
			if(tUI_CuSetting.ubTotalCamNum == DISPLAY_4T1R)
			{
				if((tPairInfo.tDispLocation == DISP_LOWER_LEFT) || (tPairInfo.tDispLocation == DISP_LOWER_RIGHT))
					return;
				if((tPairInfo.tDispLocation == DISP_H_L) || (tPairInfo.tDispLocation == DISP_H_R) || (tPairInfo.tDispLocation == DISP_H_CL))
					return;	
				tPairInfo.tDispLocation = (KNL_DISP_H == VDO_DISP_TYPE)?DISP_H_CL:(tPairInfo.tDispLocation == DISP_UPPER_LEFT)?DISP_LOWER_LEFT:DISP_LOWER_RIGHT;
				break;
			}
			return;
		case LEFT_ARROW:
			if(tUI_CuSetting.ubTotalCamNum == DISPLAY_4T1R)
			{
				if((tPairInfo.tDispLocation == DISP_UPPER_LEFT) || (tPairInfo.tDispLocation == DISP_LOWER_LEFT))
					return;
				if(tPairInfo.tDispLocation == DISP_H_L)
					return;
				tPairInfo.tDispLocation = (KNL_DISP_H == VDO_DISP_TYPE)?(tPairInfo.tDispLocation == DISP_H_R)?DISP_H_CU:DISP_H_L:
				                                                        (tPairInfo.tDispLocation == DISP_UPPER_RIGHT)?DISP_UPPER_LEFT:DISP_LOWER_LEFT;
			}
			else if(tUI_CuSetting.ubTotalCamNum == DISPLAY_2T1R)
			{
				if(tPairInfo.tDispLocation == DISP_LEFT)
					return;
				tPairInfo.tDispLocation = DISP_LEFT;
			}
			break;
		case RIGHT_ARROW:
			if(tUI_CuSetting.ubTotalCamNum == DISPLAY_4T1R)
			{
				if((tPairInfo.tDispLocation == DISP_UPPER_RIGHT) || (tPairInfo.tDispLocation == DISP_LOWER_RIGHT))
					return;
				if(tPairInfo.tDispLocation == DISP_H_R)
					return;
				tPairInfo.tDispLocation = (KNL_DISP_H == VDO_DISP_TYPE)?(tPairInfo.tDispLocation == DISP_H_L)?DISP_H_CU:DISP_H_R:
														 			    (tPairInfo.tDispLocation == DISP_UPPER_LEFT)?DISP_UPPER_RIGHT:DISP_LOWER_RIGHT;
			}
			else if(tUI_CuSetting.ubTotalCamNum == DISPLAY_2T1R)
			{
				if(tPairInfo.tDispLocation == DISP_RIGHT)
					return;
				tPairInfo.tDispLocation = DISP_RIGHT;
			}
			break;
		case ENTER_ARROW:
		{
			APP_EventMsg_t tUI_PairMessage = {0};
			uint16_t uwIconXoffset = (tUI_CuSetting.ubTotalCamNum == DISPLAY_4T1R)?((tPairInfo.tDispLocation == DISP_LOWER_LEFT) || (tPairInfo.tDispLocation == DISP_LOWER_RIGHT))?88:0:
									 (tUI_CuSetting.ubTotalCamNum == DISPLAY_2T1R)?40:0;
			uint16_t uwIconYoffset = (tUI_CuSetting.ubTotalCamNum == DISPLAY_4T1R)?((tPairInfo.tDispLocation == DISP_UPPER_RIGHT) || (tPairInfo.tDispLocation == DISP_LOWER_RIGHT))?130:0:
									 (tUI_CuSetting.ubTotalCamNum == DISPLAY_2T1R)?(tPairInfo.tDispLocation == DISP_RIGHT)?130:0:0;
			uint16_t uwPairIconOsdImgIdx = OSD2IMG_PAIRINGMULTI_ICON;

			uwIconXoffset = ((tUI_CuSetting.ubTotalCamNum == DISPLAY_4T1R) && (KNL_DISP_H == VDO_DISP_TYPE))?(tPairInfo.tDispLocation == DISP_H_CL)?89:0:uwIconXoffset;
			uwIconYoffset = ((tUI_CuSetting.ubTotalCamNum == DISPLAY_4T1R) && (KNL_DISP_H == VDO_DISP_TYPE))?(tPairInfo.tDispLocation == DISP_H_R)?186:0:uwIconYoffset;
			uwPairIconOsdImgIdx = ((tUI_CuSetting.ubTotalCamNum == DISPLAY_4T1R) && (KNL_DISP_H == VDO_DISP_TYPE))?
							      ((tPairInfo.tDispLocation == DISP_H_L) || (tPairInfo.tDispLocation == DISP_H_R))?OSD2IMG_PAIRINGHS_STS1_ICON:OSD2IMG_PAIRINGC_STS1_ICON:uwPairIconOsdImgIdx;
			tOSD_GetOsdImgInfor(1, OSD_IMG2, uwPairIconOsdImgIdx, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart += uwIconXoffset;
			tOsdImgInfo.uwYStart -= uwIconYoffset;
			
		#if (defined(BUC_CU) && APP_DUAL_HOST_ENABLE)	
			memcpy(&tFixLocateImgInfo,&tOsdImgInfo,sizeof(OSD_IMG_INFO));		
			tFixLocateImgInfo.uwXStart = 200;
			tFixLocateImgInfo.uwYStart = 341;
			tOSD_Img2(&tFixLocateImgInfo, OSD_UPDATE);
		#else			
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
		#endif
			tUI_PairMessage.ubAPP_Event 	 = APP_PAIRING_START_EVENT;
			tUI_PairMessage.ubAPP_Message[0] = 2;		//! Message Length
		#if (defined(BUC_CU) && APP_DUAL_HOST_ENABLE && (KNL_WIRELESS_CAM_NUM == 1))
			tUI_PairMessage.ubAPP_Message[1] = CAM1;
			tUI_PairMessage.ubAPP_Message[2] = DISP_UPPER_LEFT;
		#else
			tUI_PairMessage.ubAPP_Message[1] = tPairInfo.tPairSelCam;
			tUI_PairMessage.ubAPP_Message[2] = tPairInfo.tDispLocation;
		#endif			
			tUI_PairMessage.ubAPP_Message[3] = FALSE;
			UI_SendMessageToAPP(&tUI_PairMessage);
			tPairInfo.ubDrawFlag 			 = TRUE;
			UI_DisableScanMode();
			tUI_State = UI_PAIRING_STATE;
			return;
		}
		case EXIT_ARROW:
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PAIRLUWINDOW_ICON, 1, &tOsdImgInfo);			
			OSD_EraserImg2(&tOsdImgInfo);
			tUI_State = UI_SUBSUBMENU_STATE;
			return;
		default:
			return;
	}
	if(KNL_DISP_H == VDO_DISP_TYPE)
		uwPairWinOsdImgIdx = OSD2IMG_PAIRHLWINDOW_ICON + (tPairInfo.tDispLocation - DISP_H_L);
	else if(KNL_DISP_DUAL_C == VDO_DISP_TYPE)
	    uwPairWinOsdImgIdx = OSD2IMG_PAIRLEFTWINDOW_ICON + (tPairInfo.tDispLocation - DISP_LEFT);
	else
		uwPairWinOsdImgIdx = OSD2IMG_PAIRLUWINDOW_ICON+tPairInfo.tDispLocation;
	
//justin 2020.07.07
#if (defined(BUC_CU) && APP_DUAL_HOST_ENABLE)
	tPairInfo.tDispLocation = DISP_UPPER_LEFT;
	//uwPairWinOsdImgIdx = OSD2IMG_PAIRLUWINDOW_ICON;
	uwPairWinOsdImgIdx = OSD2IMG_PAIRTRIPWINDOW_ICON;
#endif
	tOSD_GetOsdImgInfor(1, OSD_IMG2, uwPairWinOsdImgIdx, 1, &tOsdImgInfo);	
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}
//------------------------------------------------------------------------------
#define PAIRING_ICON_NUM	2
void UI_DrawPairingStatusIcon(void)
{
#if (defined(BUC_CU) && APP_DUAL_HOST_ENABLE)
	static OSD_IMG_INFO tFixLocateImgInfo;	
#endif
	static OSD_IMG_INFO tPairOsdImgInfo[PAIRING_ICON_NUM] = {0};
	uint16_t uwPairIconOsdImgIdx;
	uint16_t uwIconXoffset = 0;
	uint16_t uwIconYoffset = 0;
	uint16_t uwIconXStart  = 0;
	uint16_t uwIconYStart  = 0;
	static uint8_t ubIconOffset = 0;

	if(FALSE == tPairInfo.ubDrawFlag)
		return;
	if(FALSE == tUI_CuSetting.IconSts.ubRdPairIconFlag)
	{
		uwPairIconOsdImgIdx = (tUI_CuSetting.ubTotalCamNum == DISPLAY_1T1R)?OSD2IMG_PAIRINGSINGLE_ICON:
							  (tUI_CuSetting.ubTotalCamNum == DISPLAY_2T1R)?OSD2IMG_PAIRINGMULTI_ICON:
							  (tUI_CuSetting.ubTotalCamNum == DISPLAY_4T1R)?OSD2IMG_PAIRINGMULTI_ICON:OSD2IMG_PAIRINGSINGLE_ICON;
		uwPairIconOsdImgIdx = ((tUI_CuSetting.ubTotalCamNum == DISPLAY_4T1R) && (KNL_DISP_H == VDO_DISP_TYPE))?
							  ((tPairInfo.tDispLocation == DISP_H_L) || (tPairInfo.tDispLocation == DISP_H_R))?OSD2IMG_PAIRINGHS_STS1_ICON:OSD2IMG_PAIRINGC_STS1_ICON:uwPairIconOsdImgIdx;
		if(tOSD_GetOsdImgInfor(1, OSD_IMG2, uwPairIconOsdImgIdx, PAIRING_ICON_NUM, &tPairOsdImgInfo[0]) != OSD_OK)
		{
			printd(DBG_ErrorLvl, "Load OSD Image FAIL, pls check (%d) !\n", __LINE__);
			return;
		}
		tUI_CuSetting.IconSts.ubRdPairIconFlag 	= TRUE;
		ubIconOffset     						= 1;
	}
	uwIconXoffset = (tUI_CuSetting.ubTotalCamNum == DISPLAY_4T1R)?((tPairInfo.tDispLocation == DISP_LOWER_LEFT) || (tPairInfo.tDispLocation == DISP_LOWER_RIGHT))?88:0:
					(tUI_CuSetting.ubTotalCamNum == DISPLAY_2T1R)?40:0;
	uwIconXoffset = ((tUI_CuSetting.ubTotalCamNum == DISPLAY_4T1R) && (KNL_DISP_H == VDO_DISP_TYPE))?(tPairInfo.tDispLocation == DISP_H_CL)?89:0:uwIconXoffset;
	uwIconYoffset = (tUI_CuSetting.ubTotalCamNum == DISPLAY_4T1R)?((tPairInfo.tDispLocation == DISP_UPPER_RIGHT) || (tPairInfo.tDispLocation == DISP_LOWER_RIGHT))?130:0:
					(tUI_CuSetting.ubTotalCamNum == DISPLAY_2T1R)?(tPairInfo.tDispLocation == DISP_RIGHT)?130:0:0;
	uwIconYoffset = ((tUI_CuSetting.ubTotalCamNum == DISPLAY_4T1R) && (KNL_DISP_H == VDO_DISP_TYPE))?(tPairInfo.tDispLocation == DISP_H_R)?186:0:uwIconYoffset;
	uwIconXStart = tPairOsdImgInfo[ubIconOffset].uwXStart;
	uwIconYStart = tPairOsdImgInfo[ubIconOffset].uwYStart;
	tPairOsdImgInfo[ubIconOffset].uwXStart += uwIconXoffset;
	tPairOsdImgInfo[ubIconOffset].uwYStart -= uwIconYoffset;
	
#if (defined(BUC_CU) && APP_DUAL_HOST_ENABLE)	
	memcpy(&tFixLocateImgInfo,&tPairOsdImgInfo[ubIconOffset],sizeof(OSD_IMG_INFO));		
	tFixLocateImgInfo.uwXStart = 200;
	tFixLocateImgInfo.uwYStart = 341;
	tOSD_Img2(&tFixLocateImgInfo, OSD_UPDATE);
#else	
	tOSD_Img2(&tPairOsdImgInfo[ubIconOffset], OSD_UPDATE);
#endif
	tPairOsdImgInfo[ubIconOffset].uwXStart = uwIconXStart;
	tPairOsdImgInfo[ubIconOffset].uwYStart = uwIconYStart;
	ubIconOffset = (++ubIconOffset == PAIRING_ICON_NUM)?0:ubIconOffset;
}
//------------------------------------------------------------------------------
void UI_ReportPairingResult(UI_Result_t tResult)
{
	OSD_IMG_INFO tOsdImgInfo, tEraseOsdImgInfo;
	APP_EventMsg_t tUI_UnindBuMsg = {0};
	UI_CamNum_t tCamNum;
	uint16_t uwPairIconOsdImgIdx = 0;
	uint16_t uwIconXStart  = 0;

	tPairInfo.ubDrawFlag = FALSE;
	uwPairIconOsdImgIdx = (tUI_CuSetting.ubTotalCamNum == DISPLAY_1T1R)?OSD2IMG_PAIRINGSINGLE_ICON:OSD2IMG_PAIRLUWINDOW_ICON;
	uwIconXStart		= (tUI_CuSetting.ubTotalCamNum == DISPLAY_1T1R)?(tPairInfo.tPairSelCam*55):0;
	tOSD_GetOsdImgInfor(1, OSD_IMG2, uwPairIconOsdImgIdx, 1, &tEraseOsdImgInfo);
	tEraseOsdImgInfo.uwXStart += uwIconXStart;
	switch(tResult)
	{
		case rUI_SUCCESS:
			if((DISPLAY_4T1R == tUI_CuSetting.ubTotalCamNum) ||
			   (DISPLAY_2T1R == tUI_CuSetting.ubTotalCamNum))
			{
				if(!tUI_CuSetting.ubPairedCamNum)
				{
					if((SINGLE_VIEW == tCamViewSel.tCamViewType) || (DUAL_VIEW == tCamViewSel.tCamViewType))
					{
						tCamViewSel.tCamViewPool[0] = tPairInfo.tPairSelCam;
						tCamViewSel.tCamViewPool[1] = ((tPairInfo.tPairSelCam + 1) >= ((DISPLAY_2T1R == tUI_CuSetting.ubTotalCamNum)?CAM_2T:CAM_4T))?CAM1:
													   ((UI_CamNum_t)(tPairInfo.tPairSelCam + 1));
						if((DISPLAY_4T1R == tUI_CuSetting.ubTotalCamNum) && (DUAL_VIEW == tCamViewSel.tCamViewType))
							UI_SwitchCameraSource();
					}
				}
				else if((DISPLAY_4T1R == tUI_CuSetting.ubTotalCamNum) && (DUAL_VIEW == tCamViewSel.tCamViewType))
				{
					UI_CamNum_t tCamPoolNum = tCamViewSel.tCamViewPool[1];

					if((INVALID_ID == tUI_CamStatus[tCamPoolNum].ulCAM_ID) && (tCamPoolNum != tPairInfo.tPairSelCam))
					{
						tCamViewSel.tCamViewPool[1] = tPairInfo.tPairSelCam;
						UI_SwitchCameraSource();
					}
				}
			}
			tUI_CuSetting.ubPairedCamNum += (tUI_CuSetting.ubPairedCamNum >= tUI_CuSetting.ubTotalCamNum)?0:1;
			tUI_CamStatus[tPairInfo.tPairSelCam].ulCAM_ID = tPairInfo.tPairSelCam;
			tUI_CamStatus[tPairInfo.tPairSelCam].tCamDispLocation = tPairInfo.tDispLocation;
			for(tCamNum = CAM1; tCamNum < tUI_CuSetting.ubTotalCamNum; tCamNum++)
			{
				if(tCamNum == tPairInfo.tPairSelCam)
					continue;
				if((INVALID_ID != tUI_CamStatus[tCamNum].ulCAM_ID) &&
				   (tUI_CamStatus[tCamNum].tCamDispLocation == tPairInfo.tDispLocation))
				{
					tUI_UnindBuMsg.ubAPP_Event 		= APP_UNBIND_CAM_EVENT;
					tUI_UnindBuMsg.ubAPP_Message[0] = 1;		//! Message Length
					tUI_UnindBuMsg.ubAPP_Message[1] = tCamNum;
					UI_SendMessageToAPP(&tUI_UnindBuMsg);
					tUI_CamStatus[tCamNum].ulCAM_ID    = INVALID_ID;
					tUI_CamStatus[tCamNum].tCamConnSts = CAM_OFFLINE;
					tUI_CuSetting.ubPairedCamNum -= (tUI_CuSetting.ubPairedCamNum == 0)?0:1;
					tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PAIRRDYMASK_ICON, 1, &tOsdImgInfo);
					tOsdImgInfo.uwXStart += (tCamNum*55);
					OSD_EraserImg2(&tOsdImgInfo);
					tOSD_GetOsdImgInfor(1, OSD_IMG2, ((OSD2IMG_PAIRCAM1NOR_ICON+(tCamNum*3))), 1, &tOsdImgInfo);
					tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
					UI_ResetDevSetting(tCamNum);
					UI_ChkPreviewCamSrc4UnBind(tCamNum);
				}
			}
			if((DISPLAY_1T1R == tUI_CuSetting.ubTotalCamNum) ||
			   (NO_CAM == tUI_CuSetting.tAdoSrcCamNum) ||
			   (INVALID_ID == tUI_CamStatus[tUI_CuSetting.tAdoSrcCamNum].ulCAM_ID))
				tUI_CuSetting.tAdoSrcCamNum = tPairInfo.tPairSelCam;
			OSD_EraserImg2(&tEraseOsdImgInfo);
			tOSD_GetOsdImgInfor(1, OSD_IMG2, ((OSD2IMG_PAIRCAM1NOR_ICON+(tPairInfo.tPairSelCam*3))+UI_ICON_READY), 1, &tOsdImgInfo);
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PAIRRDYMASK_ICON, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart += (tPairInfo.tPairSelCam*55);			
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			UI_PairingSubSubMenuPage(DOWN_ARROW);
			UI_ResetDevSetting(tPairInfo.tPairSelCam);
			UI_UpdateDevStatusInfo();
			break;
		case rUI_FAIL:
			OSD_EraserImg2(&tEraseOsdImgInfo);
			break;
		default:
			break;
	}
	tUI_State = UI_SUBSUBMENU_STATE;
}
//------------------------------------------------------------------------------
void UI_ReportAppPairingResult(UI_Result_t tResult)
{
	UI_CamNum_t tCamNum;
	OSD_IMG_INFO tOsdImgInfo;

	tOsdImgInfo.uwHSize  = uwOSD_GetHSize();
	tOsdImgInfo.uwVSize  = uwOSD_GetVSize();
	tOsdImgInfo.uwXStart = 0;
	tOsdImgInfo.uwYStart = 0;
	OSD_EraserImg1(&tOsdImgInfo);
	tUI_CuSetting.IconSts.ubDrawStsIconFlag = TRUE;
	UI_ClearCamConnectStatusFlag();
	switch(tResult)
	{
		case rUI_SUCCESS:
			tUI_CuSetting.ubPairedCamNum += (tUI_CuSetting.ubPairedCamNum >= tUI_CuSetting.ubTotalCamNum)?0:1;
			tUI_CamStatus[tPairInfo.tPairSelCam].ulCAM_ID = tPairInfo.tPairSelCam;
			tUI_CamStatus[tPairInfo.tPairSelCam].tCamDispLocation = tPairInfo.tDispLocation;
			for(tCamNum = CAM1; tCamNum < tUI_CuSetting.ubTotalCamNum; tCamNum++)
			{
				if(tCamNum == tPairInfo.tPairSelCam)
					continue;
				if((INVALID_ID != tUI_CamStatus[tCamNum].ulCAM_ID) &&
				   (tUI_CamStatus[tCamNum].tCamDispLocation == tPairInfo.tDispLocation))
				{
					APP_EventMsg_t tUI_UnindBuMsg = {0};

					tUI_UnindBuMsg.ubAPP_Event 		= APP_UNBIND_CAM_EVENT;
					tUI_UnindBuMsg.ubAPP_Message[0] = 1;		//! Message Length
					tUI_UnindBuMsg.ubAPP_Message[1] = tCamNum;
					UI_SendMessageToAPP(&tUI_UnindBuMsg);
					tUI_CamStatus[tCamNum].ulCAM_ID    = INVALID_ID;
					tUI_CamStatus[tCamNum].tCamConnSts = CAM_OFFLINE;
					tUI_CuSetting.ubPairedCamNum -= (tUI_CuSetting.ubPairedCamNum == 0)?0:1;
					UI_ResetDevSetting(tCamNum);
				}
			}
			if((DISPLAY_1T1R == tUI_CuSetting.ubTotalCamNum) ||
			   (NO_CAM == tUI_CuSetting.tAdoSrcCamNum) ||
			   (INVALID_ID == tUI_CamStatus[tUI_CuSetting.tAdoSrcCamNum].ulCAM_ID))
				tUI_CuSetting.tAdoSrcCamNum = tPairInfo.tPairSelCam;
			UI_ResetDevSetting(tPairInfo.tPairSelCam);
			UI_UpdateDevStatusInfo();
			break;
		default:
			break;
	}
	ubUI_StopUpdateStsBarFlag = FALSE;
	tUI_State = UI_DISPLAY_STATE;
}
//------------------------------------------------------------------------------
void UI_SetLdDispStatus(UI_OsdLdDispSts_t tDispSts)
{
	osSemaphoreWait(osUI_OsdLdStsUpdSem, osWaitForever);
	tUI_OsdLdDispSts = tDispSts;
	osSemaphoreRelease(osUI_OsdLdStsUpdSem);
}
//------------------------------------------------------------------------------
UI_OsdLdDispSts_t UI_GetLdDispStatus(void)
{
	UI_OsdLdDispSts_t tDispSts;

	osSemaphoreWait(osUI_OsdLdStsUpdSem, osWaitForever);
	tDispSts = tUI_OsdLdDispSts;
	osSemaphoreRelease(osUI_OsdLdStsUpdSem);
	return tDispSts;
}
//------------------------------------------------------------------------------
void UI_ClearRecordStatusOsdImg(void)
{
	OSD_IMG_INFO tRecEreOsdImgInfo;

	tRecEreOsdImgInfo.uwHSize  = 50;
	tRecEreOsdImgInfo.uwVSize  = 150;
	tRecEreOsdImgInfo.uwXStart = 40;
	tRecEreOsdImgInfo.uwYStart = uwOSD_GetVSize() - 150;
	OSD_EraserImg2(&tRecEreOsdImgInfo);
}
//------------------------------------------------------------------------------
static void UI_OsdLoadingDisplayThread(void const *argument)
{
	uint32_t ulUI_OsdLdWaitTickTime = osWaitForever;
	uint16_t uwUI_OsdLdDispSte;
	uint8_t ubLdOsdIdx = 0;
	uint8_t ubUI_RecChgImgFlag = TRUE;
	uint8_t ubFlag = 0;
	OSD_IMG_INFO tLdOsdImg[17], tWorkOsdImg[8], tRecOsdImgInfo[3];
	OSD_RESULT tLdOsdImgRdRet = OSD_OK, tWorkOsdImgRdRet = OSD_OK, tRecOsdImgRdRet = OSD_OK;

	tLdOsdImgRdRet 	 = tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_LOADING1_P1_ICON, 16, &tLdOsdImg);
	tWorkOsdImgRdRet = tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_WORKING_P1_ICON, 8, &tWorkOsdImg);
	tRecOsdImgRdRet  = tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_RECORDINGNACT_ICON, 3, &tRecOsdImgInfo);
	if((OSD_OK != tLdOsdImgRdRet) || (OSD_OK != tWorkOsdImgRdRet) || (OSD_OK != tRecOsdImgRdRet))
		printd(DBG_ErrorLvl, "[%d:%d:%d]Load OSD Image FAIL, pls check (%d) !\n", tLdOsdImgRdRet, tWorkOsdImgRdRet, tRecOsdImgRdRet, __LINE__);
	while(1)
	{
		osMessageGet(osUI_OsdLdDispQueue, &uwUI_OsdLdDispSte, ulUI_OsdLdWaitTickTime);

		switch(uwUI_OsdLdDispSte)
		{
			case UI_SEARCH_DCIMFOLDER:
			case UI_SEARCH_RECFILES:
				while(UI_OSDLDDISP_ON == UI_GetLdDispStatus())
				{
					tOSD_Img2(&tLdOsdImg[ubLdOsdIdx++], OSD_UPDATE);
					ubLdOsdIdx = (ubLdOsdIdx >= 16)?0:ubLdOsdIdx;
					osDelay(250);
				}
				tLdOsdImg[16].uwHSize  = 200;
				tLdOsdImg[16].uwVSize  = 250;
				tLdOsdImg[16].uwXStart = 275;
				tLdOsdImg[16].uwYStart = 550;
				OSD_EraserImg2(&tLdOsdImg[16]);
				uwUI_OsdLdDispSte = UI_LD_DEFU;
				break;
			case UI_SDCARD_FORMAT:
			{
				uint16_t uwTmpXStart, uwTmpYStart;
				ubLdOsdIdx = 0;
				uwTmpXStart = tWorkOsdImg[ubLdOsdIdx].uwXStart;
				uwTmpYStart = tWorkOsdImg[ubLdOsdIdx].uwYStart;
				while(UI_OSDLDDISP_ON == UI_GetLdDispStatus())
				{
					if(UI_SD_NRDY == tUI_SdCardSts)
					{
						tWorkOsdImg[ubLdOsdIdx].uwXStart = 353;
						tWorkOsdImg[ubLdOsdIdx].uwYStart = 563;
					}
					tOSD_Img2(&tWorkOsdImg[ubLdOsdIdx], OSD_UPDATE);
					tWorkOsdImg[ubLdOsdIdx].uwXStart = uwTmpXStart;
					tWorkOsdImg[ubLdOsdIdx].uwYStart = uwTmpYStart;
					ubLdOsdIdx = (++ubLdOsdIdx >= 8)?0:ubLdOsdIdx;
					osDelay(250);
				}
				uwUI_OsdLdDispSte = UI_LD_DEFU;
				break;
			}
			default:
				if(((APP_LINK_STATE == tUI_SyncAppState)) && (UI_DISPLAY_STATE == tUI_State) && (LCD_JPEG_DISABLE == tLCD_GetJpegDecoderStatus()))
				{
					if(UI_REC_START == tUI_RecPlayAct.tRecAct)
					{
						tOSD_Img2(((FALSE == ubUI_RecChgImgFlag)?&tRecOsdImgInfo[0]:&tRecOsdImgInfo[1]), OSD_QUEUE);
						ubUI_RecChgImgFlag = (FALSE == ubUI_RecChgImgFlag)?TRUE:FALSE;
						if (!ubFlag)
						{
							tOSD_Img2(&tRecOsdImgInfo[2], OSD_UPDATE);
							ubFlag = 1;
						}
						else
							OSD_UpdateQueueBuf();
						ulUI_OsdLdWaitTickTime = 500;
					}
					else
					{
						UI_ClearRecordStatusOsdImg();
						ubUI_RecChgImgFlag	   = FALSE;
						ubFlag = 0;
						ulUI_OsdLdWaitTickTime = osWaitForever;
					}
				}
				else
				{
					ulUI_OsdLdWaitTickTime = 200;
					ubUI_RecChgImgFlag 	   = TRUE;
					ubFlag = 0;
				}
				break;
		}
	}
}
//------------------------------------------------------------------------------
static void UI_FuncsExecuteThread(void const *argument)
{
	UI_FuncExecMsg_t tUI_FuncsExecMsg;
	uint8_t ubUI_FuncsExecRet = rUI_SUCCESS;
	while(1)
	{
		osMessageGet(osUI_FuncsExecQue, &tUI_FuncsExecMsg, osWaitForever);
		switch(tUI_FuncsExecMsg.uwFunc)
		{
			case UI_SDCARDFMT_ACT:
			{
				uint16_t uwLdState = UI_SDCARD_FORMAT;

				if(UI_SD_NRDY == tUI_SdCardSts)
				{
					OSD_IMG_INFO tOsdImgInfo;

					tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SDFING_BG, 1, &tOsdImgInfo);
					tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
				}
				UI_SetLdDispStatus(UI_OSDLDDISP_ON);
				osMessagePut(osUI_OsdLdDispQueue, &uwLdState, 0);
				ubUI_FuncsExecRet = (FORMAT_FAIL == FS_MediaFormat(KNL_GetFsMedia()))?rUI_FAIL:rUI_SUCCESS;
				UI_SetLdDispStatus(UI_OSDLDDISP_OFF);
				if((rUI_SUCCESS == ubUI_FuncsExecRet) &&
				   ((UI_RECORDING_MODE == tUI_CuSetting.tVdoMode) && (REC_LOOPING == tUI_CuSetting.RecInfo.tREC_Mode)))
					UI_VideoRecordingExec(UI_REC_START);
				break;
			}
			case UI_PERDBGRPT_ACT:
				UI_DisplayTrxInfo(tUI_FuncsExecMsg);
				continue;
			default:
				continue;
		}
		osMessagePut(osUI_FuncsFinExecQue, &ubUI_FuncsExecRet, 0);
	}
}
//------------------------------------------------------------------------------
void UI_SetRecImgColor(UI_RecImgColor_t tColorNum)
{
	static UI_RecImgColor_t tUI_RecImgColor = UI_RECIMG_DEFU;
	uint8_t i;
	
	if(tUI_RecImgColor == tColorNum)
		return;
	for(i = 0; i < 10; i++)
		tUI_RecOsdImgDB.uwUI_RecNumArray[i] = (tColorNum <= UI_RECIMG_COLOR5)?((OSD2IMG_FILE_C1_NUM0 + (tColorNum * 10)) + i):
																			   (OSD2IMG_FILE_C6_NUM0 + ((tColorNum - UI_RECIMG_COLOR6) * 66) + i);
	tUI_RecOsdImgInfo.pNumImgIdxArray = tUI_RecOsdImgDB.uwUI_RecNumArray;
	for(i = 0; i < 26; i++)
	{
		tUI_RecOsdImgDB.uwUI_RecUpperLetterArray[i] = (tColorNum <= UI_RECIMG_COLOR5)?((OSD2IMG_FILE_C1_UPA + (tColorNum * 26)) + i):
																				       (OSD2IMG_FILE_C6_UPA + ((tColorNum - UI_RECIMG_COLOR6) * 66) + i);
		tUI_RecOsdImgDB.uwUI_RecLowerLetterArray[i] = (tColorNum <= UI_RECIMG_COLOR5)?((OSD2IMG_FILE_C1_LWA + (tColorNum * 26)) + i):
																					   (OSD2IMG_FILE_C6_LWA + ((tColorNum - UI_RECIMG_COLOR6) * 66) + i);
	}
	tUI_RecOsdImgInfo.pUpperLetterImgIdxArray = tUI_RecOsdImgDB.uwUI_RecUpperLetterArray;
	tUI_RecOsdImgInfo.pLowerLetterImgIdxArray = tUI_RecOsdImgDB.uwUI_RecLowerLetterArray;
	for(i = 0; i < 4; i++)
		tUI_RecOsdImgDB.uwUI_RecSymbolArray[i] = (tColorNum <= UI_RECIMG_COLOR5)?((OSD2IMG_FILE_C1_COLON + (tColorNum * 4)) + i):
																				  (OSD2IMG_FILE_C6_COLON + ((tColorNum - UI_RECIMG_COLOR6) * 66) + i);
	tUI_RecOsdImgInfo.pSymbolImgIdxArray = tUI_RecOsdImgDB.uwUI_RecSymbolArray;
	tUI_RecImgColor = tColorNum;
}	
//------------------------------------------------------------------------------
void UI_SearchDCIMFolder(void)
{
	uint16_t uwLdState = UI_SEARCH_DCIMFOLDER;
	
	UI_SetLdDispStatus(UI_OSDLDDISP_ON);
	osMessagePut(osUI_OsdLdDispQueue, &uwLdState, 0);
	osDelay(200);
	pUI_RecFoldersInfo.uwTotalRecFolderNum = ulKNL_GetSortingFolders(KNL_REAL_FLD,SORT_BY_NAME_DESCENDING,&pUI_RecFoldersInfo.tRecFolderInfo[0]);
	UI_SetLdDispStatus(UI_OSDLDDISP_OFF);
	osDelay(350);
}
//------------------------------------------------------------------------------
void UI_ListDCIMFolderInfo(uint16_t uwStartFileIdx, uint16_t uwEndFileIdx, OSD_UPDATE_TYP tUpdateMode)
{
	OSD_IMG_INFO tRecOsdImgInfo[2];
	uint16_t uwRecXStart[2] = {0, 0}, uwRecYStart[2] = {0, 0};
	uint8_t ubRecFolderIdx, ubOpenOsdIdx = 0;
	uint16_t uwIdx;

	if(tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_REC_FOLDERCLOSE_ICON, 2, &tRecOsdImgInfo[0]) != OSD_OK)
	{
		printd(DBG_ErrorLvl, "Load OSD Image FAIL, pls check (%d) !\n", __LINE__);
		return;
	}
	uwRecXStart[0] = tRecOsdImgInfo[0].uwXStart;
	uwRecYStart[0] = tRecOsdImgInfo[0].uwYStart;
	uwRecXStart[1] = tRecOsdImgInfo[1].uwXStart;
	uwRecYStart[1] = tRecOsdImgInfo[1].uwYStart;
	for(uwIdx = uwStartFileIdx; uwIdx < uwEndFileIdx; uwIdx++)
	{
		UI_SetRecImgColor((pUI_RecFoldersInfo.uwRecFolderSelIdx == uwIdx)?UI_RECIMG_COLOR6:UI_RECIMG_COLOR5);
		ubOpenOsdIdx = (pUI_RecFoldersInfo.uwRecFolderSelIdx == uwIdx)?1:0;
		ubRecFolderIdx = uwIdx % REC_FOLDER_LIST_MAXNUM;
		tRecOsdImgInfo[ubOpenOsdIdx].uwYStart -= ((ubRecFolderIdx % 5) * 185);
		tRecOsdImgInfo[ubOpenOsdIdx].uwXStart += ((ubRecFolderIdx / 5) * 265);
		tOSD_Img2(&tRecOsdImgInfo[ubOpenOsdIdx], OSD_QUEUE);
		OSD_ImagePrintf(OSD_IMG_ROTATION_90, (275 + ((ubRecFolderIdx % 5) * 185)), (263 + ((ubRecFolderIdx / 5) * 265)), tUI_RecOsdImgInfo, tUpdateMode, pUI_RecFoldersInfo.tRecFolderInfo[uwIdx].FldName.chName);
		tRecOsdImgInfo[ubOpenOsdIdx].uwXStart = uwRecXStart[ubOpenOsdIdx];
		tRecOsdImgInfo[ubOpenOsdIdx].uwYStart = uwRecYStart[ubOpenOsdIdx];
	}
}
//------------------------------------------------------------------------------
void UI_DrawDCIMFolderMenu(void)
{
	OSD_IMG_INFO tRecBgOsdImgInfo, tRecOsdImgInfo;

	if(tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_SUBMENU, 1, &tRecBgOsdImgInfo) != OSD_OK)
	{
		printd(DBG_ErrorLvl, "Load OSD Image FAIL, pls check (%d) !\n", __LINE__);
		return;
	}
	if(tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_FILE_SUBMENUICON, 1, &tRecOsdImgInfo) != OSD_OK)
	{
		printd(DBG_ErrorLvl, "Load OSD Image FAIL, pls check (%d) !\n", __LINE__);
		return;
	}
	tUI_State = (TRUE == ubUI_RecSubMenuFlag)?UI_SUBMENU_STATE:UI_RECFOLDER_SEL_STATE;
	OSD_Weight(OSD_WEIGHT_8DIV8);
	tRecBgOsdImgInfo.uwXStart = 0;
	tRecBgOsdImgInfo.uwYStart = 0;
#if !APP_FS_FILE_LIST_STYLE
	tOSD_Img1(&tRecBgOsdImgInfo, OSD_QUEUE);
#else
	if(KNL_ThmShowInfo.ubInFldListFlg==1)
	{
		KNL_ThmShowInfo.ubInFldListFlg = 0;
		tOSD_Img1(&tRecBgOsdImgInfo, OSD_QUEUE);
	}
#endif	
	tOSD_Img2(&tRecOsdImgInfo, OSD_UPDATE);
	if(SCAN_VIEW == tCamViewSel.tCamViewType)
	{
		ubUI_DisScanMdFunc = TRUE;
		UI_DisableScanMode();
	}
	UI_SearchDCIMFolder();
	if(pUI_RecFoldersInfo.uwTotalRecFolderNum)
		UI_ListDCIMFolderInfo(0, ((pUI_RecFoldersInfo.uwTotalRecFolderNum < REC_FOLDER_LIST_MAXNUM)?pUI_RecFoldersInfo.uwTotalRecFolderNum:REC_FOLDER_LIST_MAXNUM), OSD_UPDATE);
}
//------------------------------------------------------------------------------
void UI_DCIMFolderSelection(UI_ArrowKey_t tArrowKey)
{
	OSD_IMG_INFO tFolderSelOsdImgInfo[2], tOsdImgInfo;
	uint8_t ubUI_RecFolderIdx = 0, ubUI_PrevRecFolderIdx = 0;
	uint8_t ubRefreshFldIconFlag = FALSE, ubRefreshFldIdx, ubFldChkIdx;
	uint16_t uwRecSelIdx;

	if((!pUI_RecFoldersInfo.uwTotalRecFolderNum) && (EXIT_ARROW != tArrowKey))
		return;

	if(tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_REC_FOLDERCLOSE_ICON, 2, &tFolderSelOsdImgInfo[0]) != OSD_OK)
	{
		printd(DBG_ErrorLvl, "Load OSD Image FAIL, pls check (%d) !\n", __LINE__);
		return;
	}
	uwRecSelIdx = pUI_RecFoldersInfo.uwRecFolderSelIdx;
	ubUI_RecFolderIdx = (uwRecSelIdx % REC_FOLDER_LIST_MAXNUM);
	switch(tArrowKey)
	{
		case LEFT_ARROW:
			if(!(ubUI_RecFolderIdx % 5))
				return;
			ubUI_PrevRecFolderIdx = ubUI_RecFolderIdx;
			ubUI_RecFolderIdx     = (ubUI_PrevRecFolderIdx - 1);
			--pUI_RecFoldersInfo.uwRecFolderSelIdx;
			break;
		case RIGHT_ARROW:
			if((((ubUI_RecFolderIdx % 5) + 1) >= 5) || ((uwRecSelIdx + 1) >= pUI_RecFoldersInfo.uwTotalRecFolderNum)) 
				return;
			ubUI_PrevRecFolderIdx = ubUI_RecFolderIdx;
			ubUI_RecFolderIdx     = (ubUI_PrevRecFolderIdx + 1);
			++pUI_RecFoldersInfo.uwRecFolderSelIdx;
			break;
		case UP_ARROW:
			if(pUI_RecFoldersInfo.uwRecFolderSelIdx < 5)
				return;
			ubUI_PrevRecFolderIdx = ubUI_RecFolderIdx;
			ubUI_RecFolderIdx    -= 5;
			ubRefreshFldIdx = (pUI_RecFoldersInfo.uwRecFolderSelIdx > REC_FOLDER_LIST_MAXNUM)?((pUI_RecFoldersInfo.uwRecFolderSelIdx % REC_FOLDER_LIST_MAXNUM) >= 5)?0:(pUI_RecFoldersInfo.uwRecFolderSelIdx % REC_FOLDER_LIST_MAXNUM):0;
			ubRefreshFldIconFlag  = (!((pUI_RecFoldersInfo.uwRecFolderSelIdx - ubRefreshFldIdx) % REC_FOLDER_LIST_MAXNUM))?TRUE:FALSE;
			pUI_RecFoldersInfo.uwRecFolderSelIdx -= 5;
			break;
		case DOWN_ARROW:
			ubFldChkIdx = (ubUI_RecFolderIdx / 5)?(uwRecSelIdx - (uwRecSelIdx % 5) + 5):(uwRecSelIdx + 5);
        	if(ubFldChkIdx >= pUI_RecFoldersInfo.uwTotalRecFolderNum)
				return;
			ubUI_PrevRecFolderIdx = ubUI_RecFolderIdx;
			ubUI_RecFolderIdx    += 5;
			pUI_RecFoldersInfo.uwRecFolderSelIdx = ((pUI_RecFoldersInfo.uwRecFolderSelIdx + 5) >= pUI_RecFoldersInfo.uwTotalRecFolderNum)?
													 (((pUI_RecFoldersInfo.uwRecFolderSelIdx + 5) / REC_FOLDER_LIST_MAXNUM) * REC_FOLDER_LIST_MAXNUM):(pUI_RecFoldersInfo.uwRecFolderSelIdx + 5);
			ubRefreshFldIdx = (pUI_RecFoldersInfo.uwRecFolderSelIdx > REC_FOLDER_LIST_MAXNUM)?((pUI_RecFoldersInfo.uwRecFolderSelIdx % REC_FOLDER_LIST_MAXNUM) >= 5)?0:(pUI_RecFoldersInfo.uwRecFolderSelIdx % REC_FOLDER_LIST_MAXNUM):0;
			ubRefreshFldIconFlag  = (!((pUI_RecFoldersInfo.uwRecFolderSelIdx - ubRefreshFldIdx) % REC_FOLDER_LIST_MAXNUM))?TRUE:FALSE;
			break;
		case ENTER_ARROW:
#if !APP_FS_FILE_LIST_STYLE
			tOsdImgInfo.uwHSize  = 600;
			tOsdImgInfo.uwVSize  = 955;
			tOsdImgInfo.uwXStart = 100;
			tOsdImgInfo.uwYStart = 50;
			OSD_EraserImg2(&tOsdImgInfo);
#else
            tOsdImgInfo.uwHSize  = 0;
			tOsdImgInfo.uwVSize  = 0;
			tOsdImgInfo.uwXStart = 720;
			tOsdImgInfo.uwYStart = 1280;            
            OSD_EraserImg2(&tOsdImgInfo);
            UI_ClearOsdImage();
            if((UI_RECORDING_MODE == tUI_CuSetting.tVdoMode) && (UI_REC_START == tUI_RecPlayAct.tRecAct))
            {
                UI_VideoRecordingExec(UI_REC_STOP);
            }
            KNL_ThumbnailSwtichView();
#endif
			UI_DrawRecordFileMenu();
			return;
		case EXIT_ARROW:
			ubUI_DisScanMdFunc = FALSE;
			pUI_RecFoldersInfo.uwRecFolderSelIdx = 0;
			if(APP_LOSTLINK_STATE == tUI_SyncAppState)
			{
				tOsdImgInfo.uwHSize  = uwOSD_GetHSize();
				tOsdImgInfo.uwVSize  = uwOSD_GetVSize();
				tOsdImgInfo.uwXStart = 0;
				tOsdImgInfo.uwYStart = 0;
				OSD_EraserImg1(&tOsdImgInfo);
				UI_ClearCamConnectStatusFlag();
				tUI_State = UI_DISPLAY_STATE;
				return;
			}
			tOsdImgInfo.uwHSize  = 600;
			tOsdImgInfo.uwVSize  = 1180;
			tOsdImgInfo.uwXStart = 100;
			tOsdImgInfo.uwYStart = 50;
			OSD_EraserImg2(&tOsdImgInfo);
			UI_DrawCameraSettingMenu(UI_CAMISP_SETUP);
			return;
		default:
			return;
	}
	if(TRUE == ubRefreshFldIconFlag)
	{
		uint16_t uwFldStartIdx = 0, uwFldEndIdx = 0;

		tOsdImgInfo.uwHSize  = 600;
		tOsdImgInfo.uwVSize  = 955;
		tOsdImgInfo.uwXStart = 100;
		tOsdImgInfo.uwYStart = 50;
		OSD_EraserImg2(&tOsdImgInfo);
		uwFldStartIdx = (pUI_RecFoldersInfo.uwRecFolderSelIdx / REC_FOLDER_LIST_MAXNUM) * REC_FOLDER_LIST_MAXNUM;
		if(DOWN_ARROW == tArrowKey)
		{
			uwFldEndIdx = pUI_RecFoldersInfo.uwTotalRecFolderNum - pUI_RecFoldersInfo.uwRecFolderSelIdx;
			uwFldEndIdx = (uwFldEndIdx < REC_FOLDER_LIST_MAXNUM)?pUI_RecFoldersInfo.uwTotalRecFolderNum:(uwFldStartIdx + REC_FOLDER_LIST_MAXNUM);
		}
		else
		{
			uwFldEndIdx = uwFldStartIdx + REC_FOLDER_LIST_MAXNUM;
			ubRefreshFldIdx = pUI_RecFoldersInfo.uwRecFolderSelIdx;			
		}
		if(ubRefreshFldIdx)
		{
			ubUI_PrevRecFolderIdx = 0;
			ubUI_RecFolderIdx 	  = ubRefreshFldIdx % REC_FOLDER_LIST_MAXNUM;
			ubRefreshFldIconFlag  = FALSE;
		}
		UI_ListDCIMFolderInfo(uwFldStartIdx, uwFldEndIdx, (ubRefreshFldIdx)?OSD_QUEUE:OSD_UPDATE);
	}
	if(FALSE == ubRefreshFldIconFlag)
	{
		uwRecSelIdx = (pUI_RecFoldersInfo.uwRecFolderSelIdx / REC_FOLDER_LIST_MAXNUM) * REC_FOLDER_LIST_MAXNUM;
		UI_SetRecImgColor(UI_RECIMG_COLOR5);
		tFolderSelOsdImgInfo[0].uwYStart -= ((ubUI_PrevRecFolderIdx % 5) * 185);
		tFolderSelOsdImgInfo[0].uwXStart += ((ubUI_PrevRecFolderIdx / 5) * 265);
		tOSD_Img2(&tFolderSelOsdImgInfo[0], OSD_QUEUE);
		OSD_ImagePrintf(OSD_IMG_ROTATION_90, (275 + ((ubUI_PrevRecFolderIdx % 5) * 185)), (263 + ((ubUI_PrevRecFolderIdx / 5) * 265)),
						tUI_RecOsdImgInfo, OSD_QUEUE, pUI_RecFoldersInfo.tRecFolderInfo[ubUI_PrevRecFolderIdx + uwRecSelIdx].FldName.chName);
		UI_SetRecImgColor(UI_RECIMG_COLOR6);
		tFolderSelOsdImgInfo[1].uwYStart -= ((ubUI_RecFolderIdx % 5) * 185);
		tFolderSelOsdImgInfo[1].uwXStart += ((ubUI_RecFolderIdx / 5) * 265);
		tOSD_Img2(&tFolderSelOsdImgInfo[1], OSD_QUEUE);
		OSD_ImagePrintf(OSD_IMG_ROTATION_90, (275 + ((ubUI_RecFolderIdx % 5) * 185)), (263 + ((ubUI_RecFolderIdx / 5) * 265)),
						tUI_RecOsdImgInfo, OSD_UPDATE, pUI_RecFoldersInfo.tRecFolderInfo[ubUI_RecFolderIdx + uwRecSelIdx].FldName.chName);
	}
}
//------------------------------------------------------------------------------
void UI_StopPlayRecordFile(uint8_t ubPlayRet)
{
	KNL_RecordAct_t tUI_PlayAct = {KNL_RECORDFUNC_DISABLE, NULL,};

	switch(ubPlayRet)
	{
		case KNL_VDOPLAY_STOP:
            if(tUI_RecPlayAct.tPlaySts != UI_RECFILE_STOP)
            {
    			tUI_RecPlayAct.tPlaySts = UI_RECFILE_STOP;
    			UI_RecordPlayListSelection(DOWN_ARROW);
            }
			break;
		default:
			if(UI_RECFILE_PLAY == tUI_RecPlayAct.tPlaySts)
			{
				printd(DBG_ErrorLvl, "Video Play Err !\n");
				tUI_PlayAct.tRecordFunc = KNL_RECORDFUNC_DISABLE;
				tUI_PlayAct.pRecordStsNtyCb = NULL;
				tKNL_ExecRecordFunc(tUI_PlayAct);
				UI_RecordPlayListSelection(EXIT_ARROW);
			}
			else if(UI_RECFILES_SEL_STATE == tUI_State)
			{
				OSD_IMG_INFO tRecPlayOsdImgInfo;

				tUI_PlayAct.tRecordFunc = KNL_RECORDFUNC_DISABLE;
				tUI_PlayAct.pRecordStsNtyCb = NULL;
				tKNL_ExecRecordFunc(tUI_PlayAct);
				tRecPlayOsdImgInfo.uwHSize  = 415;
				tRecPlayOsdImgInfo.uwVSize  = 955;
				tRecPlayOsdImgInfo.uwXStart = 195;
				tRecPlayOsdImgInfo.uwYStart = 50;
				OSD_EraserImg2(&tRecPlayOsdImgInfo);
				UI_DrawRecordFileMenu();
			}
			break;
	}
}
//------------------------------------------------------------------------------
void UI_PlayRecordFile(uint16_t uwRecFileIndex)
{
    KNL_RecordAct_t tUI_PlayAct = {KNL_RECORDFUNC_DISABLE, NULL,};
    uint16_t uwIdx;
    uint8_t ubPlayIdx = 0;

    if(ubPLY_GetOpMode() == PLY_MODE_R)
    {
        printd(DBG_ErrorLvl, "PLY Fail(Now Recording) \n");
        return;
    }

	if(!memcmp(pUI_RecFilesInfo.tRecFilesInfo[uwRecFileIndex].HidnFileInfo.FileName.chExt, "JPG", 3))
	{
		tUI_PlayAct.tRecordFunc  	= KNL_PHOTO_PLAY;
		tUI_PlayAct.pRecordStsNtyCb = UI_PhotoPlayFinish;
		memset(&tUI_PlayAct.tPhotoPlayInfo.FileName, 0, sizeof(tUI_PlayAct.tPhotoPlayInfo.FileName));
		memcpy(&tUI_PlayAct.tPhotoPlayInfo.FileName, &pUI_RecFilesInfo.tRecFilesInfo[uwRecFileIndex].HidnFileInfo.FileName, sizeof(pUI_RecFilesInfo.tRecFilesInfo[uwRecFileIndex].HidnFileInfo.FileName));
		tUI_PlayAct.tPhotoPlayInfo.SrcNum =(FS_SRC_NUM)(pUI_RecFilesInfo.tRecFilesInfo[uwRecFileIndex].HidnFileInfo.SrcNum + FS_JPG_SRC_0);
		tUI_PlayAct.tPhotoPlayInfo.ulFirstClus = pUI_RecFilesInfo.tRecFilesInfo[uwRecFileIndex].HidnFileInfo.ulFirstClus;
		tUI_PlayAct.tPhotoPlayInfo.ulFileSize = pUI_RecFilesInfo.tRecFilesInfo[uwRecFileIndex].HidnFileInfo.ullFileSize;
		tUI_PlayAct.tPhotoPlayInfo.NoFatChainFlag = pUI_RecFilesInfo.tRecFilesInfo[uwRecFileIndex].HidnFileInfo.NoFatChainFlag;
		if(KNL_OK == tKNL_ExecRecordFunc(tUI_PlayAct))
			tUI_State = UI_PHOTOPLAYNRDY_STATE;
		else
			printd(DBG_ErrorLvl, "Photo play err !\n");
	}
	else if(!memcmp(pUI_RecFilesInfo.tRecFilesInfo[uwRecFileIndex].HidnFileInfo.FileName.chExt, "MP4", 3))
	{
		OSD_IMG_INFO tRecPlayOsdImgInfo[12];
		KNL_ROLE tRecRoleNum;
		UI_CamNum_t tRecCamNum;
		uint8_t ubImgIdx = 0;

		if(tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_RECPLAYLISTBG, 1, &tRecPlayOsdImgInfo[0]) != OSD_OK)
		{
			printd(DBG_ErrorLvl, "Load OSD Image FAIL, pls check (%d) !\n", __LINE__);
			return;
		}
		if(tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_REC_PAUSENOR_ICON, 10, &tRecPlayOsdImgInfo[1]) != OSD_OK)
		{
			printd(DBG_ErrorLvl, "Load OSD Image FAIL, pls check (%d) !\n", __LINE__);
			return;
		}
		if(UI_RECPLAYLIST_STATE == tUI_State)
		{
		    tUI_RecPlayAct.tPlaySts = UI_RECFILE_PLAY;
            ubPLY_Jump(PLY_JUMP_RESTART);
			KNL_ResetLcdChannel();
			tOSD_Img2(&tRecPlayOsdImgInfo[2], OSD_UPDATE);
		}
		else
		{
			for(tRecCamNum = CAM1; tRecCamNum <= CAM4; tRecCamNum++)
				tUI_RecPlayAct.tVdoPlayCam[tRecCamNum] = NO_CAM;
			tRecRoleNum = VDO_KNLSrcNumMap2KNLRoleNum((KNL_SRC)pUI_RecFilesInfo.tRecFilesInfo[uwRecFileIndex].HidnFileInfo.SrcNum);
			if(KNL_NONE != tRecRoleNum)
			{
				APP_KNLRoleMap2CamNum(tRecRoleNum, tRecCamNum);
				tUI_RecPlayAct.tVdoPlayCam[tRecCamNum] = tRecCamNum;
				tUI_RecPlayAct.tAdoPlayCam = tRecCamNum;
				ubPLY_AdoChannelSet(tUI_RecPlayAct.tAdoPlayCam);
			}
			tUI_PlayAct.tRecordFunc  	= KNL_VIDEO_PLAY;
			tUI_PlayAct.pRecordStsNtyCb = UI_StopPlayRecordFile;
			tUI_PlayAct.tPlayDispTye	= (KNL_DISP_TYPE)pUI_RecFilesInfo.tRecFilesInfo[uwRecFileIndex].HidnFileInfo.SubHidnInfo.ubPreviewMode;
			tUI_RecPlayAct.ubPlayMode	= tUI_PlayAct.tPlayDispTye;
			if(KNL_DISP_SINGLE == tUI_PlayAct.tPlayDispTye)
			{
				tUI_PlayAct.ulVideoPlayIdx[0] = uwRecFileIndex;
				tUI_PlayAct.ubPlayFileNum = 1;
			}
			else
			{
				tUI_PlayAct.ubPlayFileNum = 0;
				for(uwIdx = 0; uwIdx < pUI_RecFilesInfo.uwTotalRecFileNum; uwIdx++)
				{
					if(tUI_PlayAct.ubPlayFileNum)
					{
						 if(++ubPlayIdx > 4)
							break;
					}
					if(pUI_RecFilesInfo.tRecFilesInfo[uwIdx].HidnFileInfo.uwGroupIdx == pUI_RecFilesInfo.tRecFilesInfo[uwRecFileIndex].HidnFileInfo.uwGroupIdx)
					{
						tRecRoleNum = VDO_KNLSrcNumMap2KNLRoleNum((KNL_SRC)pUI_RecFilesInfo.tRecFilesInfo[uwIdx].HidnFileInfo.SrcNum);
						if(KNL_NONE != tRecRoleNum)
						{
							APP_KNLRoleMap2CamNum(tRecRoleNum, tRecCamNum);
							tUI_RecPlayAct.tVdoPlayCam[tRecCamNum] = tRecCamNum;
						}
						tUI_PlayAct.ulVideoPlayIdx[tUI_PlayAct.ubPlayFileNum++] = uwIdx;
					}
				}
			}
			UI_DisableScanMode();
			if(KNL_OK == tKNL_ExecRecordFunc(tUI_PlayAct))
			{
				tUI_RecPlayAct.tPlaySts = UI_RECFILE_PLAY;
				tUI_State = UI_RECPLAYLIST_STATE;
				tRecPlayOsdImgInfo[11].uwHSize  = uwOSD_GetHSize();
				tRecPlayOsdImgInfo[11].uwVSize  = uwOSD_GetVSize();
				tRecPlayOsdImgInfo[11].uwXStart = 0;
				tRecPlayOsdImgInfo[11].uwYStart = 0;
				OSD_EraserImg1(&tRecPlayOsdImgInfo[11]);
				OSD_Weight(OSD_WEIGHT_6DIV8);
				tOSD_Img1(&tRecPlayOsdImgInfo[0], OSD_QUEUE);
				for(ubImgIdx = 3; ubImgIdx < 10; ubImgIdx+=2)
					tOSD_Img2(&tRecPlayOsdImgInfo[ubImgIdx], OSD_QUEUE);
				tOSD_Img2(&tRecPlayOsdImgInfo[2], OSD_UPDATE);
			}
			else
				tUI_RecPlayAct.tPlaySts = UI_RECFILE_STOP;
		}
	}
}
//------------------------------------------------------------------------------
uint8_t UI_DeleteRecordFile(uint16_t uwRecFileIndex)
{
    FS_KNL_MANUAL_DEL_PROCESS_t tUI_DelAct; 
    uint8_t ubFsTimeout = 150;
	FS_MEDIA_SEL MediaSel;

	MediaSel = KNL_GetFsMedia();
	
	tUI_DelAct.MediaSel = MediaSel;
    memset(&tUI_DelAct.FldName, 0, sizeof(tUI_DelAct.FldName));
    memset(&tUI_DelAct.FileName, 0, sizeof(tUI_DelAct.FileName));
	memcpy(&tUI_DelAct.FldName, &pUI_RecFoldersInfo.tRecFolderInfo[pUI_RecFoldersInfo.uwRecFolderSelIdx].FldName, sizeof(pUI_RecFoldersInfo.tRecFolderInfo[pUI_RecFoldersInfo.uwRecFolderSelIdx].FldName));
	memcpy(&tUI_DelAct.FileName, &pUI_RecFilesInfo.tRecFilesInfo[uwRecFileIndex].HidnFileInfo.FileName, sizeof(pUI_RecFilesInfo.tRecFilesInfo[uwRecFileIndex].HidnFileInfo.FileName));
    tUI_DelAct.FilePath = pUI_RecFilesInfo.tRecFilesInfo[uwRecFileIndex].HidnFileInfo.FilePath;
	if( memcmp(tUI_DelAct.FldName.chName,"EMG", 3)==0 )
		tUI_DelAct.FilePath = FILE_PATH3;
	else if( memcmp(tUI_DelAct.FldName.chName,"TIMELAPS", 8)==0 )
		tUI_DelAct.FilePath = FILE_PATH4;
	else	
    FS_ManualDeleteFile(&tUI_DelAct);
    while(FS_ChkManualDelStatus(MediaSel) != FS_MANUAL_DEL_OK)
	{
		osDelay(20);
		if(!--ubFsTimeout)
		{
			printd(DBG_ErrorLvl, "Delete File Err !!\n");
			return FALSE;
		}
	}
    return TRUE;
}
//------------------------------------------------------------------------------
void UI_PhotoPlayListSelection(UI_ArrowKey_t tArrowKey)
{
	KNL_RecordAct_t tUI_PlayAct = {KNL_RECORDFUNC_DISABLE, NULL,};
#if !APP_FS_FILE_LIST_STYLE
	OSD_IMG_INFO tRecPlayOsdImgInfo[2];
#else
    OSD_IMG_INFO tOsdImgInfo;
#endif

	switch(tArrowKey)
	{
		case EXIT_ARROW:
#if !APP_FS_FILE_LIST_STYLE
			tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_SUBMENU, 1, &tRecPlayOsdImgInfo[0]);
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_FILE_SUBMENUICON, 1, &tRecPlayOsdImgInfo[1]);
			tOSD_Img1(&tRecPlayOsdImgInfo[0], OSD_QUEUE);
			tOSD_Img2(&tRecPlayOsdImgInfo[1], OSD_QUEUE);
#else
            tOsdImgInfo.uwXStart = 0;
            tOsdImgInfo.uwYStart = 0;
            tOsdImgInfo.uwHSize  = 720;
            tOsdImgInfo.uwVSize  = 1280;   
            OSD_EraserImg2(&tOsdImgInfo);
#endif
			UI_DrawRecordFileMenu();
			tUI_PlayAct.tRecordFunc = KNL_RECORDFUNC_DISABLE;
			tUI_PlayAct.pRecordStsNtyCb = NULL;
			tKNL_ExecRecordFunc(tUI_PlayAct);
#if !APP_FS_FILE_LIST_STYLE
			if(TRUE == tUI_CuSetting.IconSts.ubShowLostLogoFlag)
				OSD_LogoJpeg(OSDLOGO_LOSTLINK);
#else
			tUI_CuSetting.IconSts.ubShowLostLogoFlag = FALSE;
#endif
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_DrawRecordPlayAudioSource(void)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint16_t uwDisp2TImgIdx[4] = {OSD2IMG_SELADOCAM1ONLINE_ICON,   OSD2IMG_SELADOCAM1ONLINE_ICON,
								  OSD2IMG_SELADOCAM2_1ONLINE_ICON, OSD2IMG_SELADOCAM2_1OFFLINE_ICON};
	uint16_t uwDisp4TImgIdx[8] = {OSD2IMG_SELADOCAM1ONLINE_ICON, OSD2IMG_SELADOCAM1OFFLINE_ICON,
								  OSD2IMG_SELADOCAM2ONLINE_ICON, OSD2IMG_SELADOCAM2OFFLINE_ICON,
								  OSD2IMG_SELADOCAM3ONLINE_ICON, OSD2IMG_SELADOCAM3OFFLINE_ICON,
								  OSD2IMG_SELADOCAM4ONLINE_ICON, OSD2IMG_SELADOCAM4OFFLINE_ICON};
	uint16_t uwDisplayImgIdx;
	uint16_t uwXOffset = 0, uwYOffset;
	uint16_t uwYItemOffset = 150;
	UI_CamNum_t tCamNum;

	uwYOffset = (DISPLAY_2T1R == tUI_CuSetting.ubTotalCamNum)?150:0;
	for(tCamNum = CAM1; tCamNum < tUI_CuSetting.ubTotalCamNum; tCamNum++)
	{
		uwDisplayImgIdx  = (DISPLAY_2T1R == tUI_CuSetting.ubTotalCamNum)?uwDisp2TImgIdx[tCamNum*2]:uwDisp4TImgIdx[tCamNum*2];
		uwDisplayImgIdx += ((NO_CAM == tUI_RecPlayAct.tVdoPlayCam[tCamNum])?1:0);
		tOSD_GetOsdImgInfor(1, OSD_IMG2, uwDisplayImgIdx, 1, &tOsdImgInfo);
		tOsdImgInfo.uwYStart -= uwYOffset;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	}
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SELCAMHL_ICON, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart += uwXOffset;
	tOsdImgInfo.uwYStart -= ((tUI_RecPlayAct.tAdoPlayCam*111) + uwYOffset + uwYItemOffset);
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
	tUI_State = UI_RECPLAYADOSRC_SEL_STATE;
}
//------------------------------------------------------------------------------
void UI_RecordPlayListSelection(UI_ArrowKey_t tArrowKey)
{
	static UI_RecPlayListItem_t tUI_RecPlayListItem = UI_RECPLAYPAUSE_ITEM;
	static UI_RecPlayStatus_t tUI_RecPlaySts = UI_RECFILE_PLAY;
	static uint16_t uwRecPlayListImgIdx[UI_RECPLAYLISTITEM_MAX] = {OSD2IMG_REC_SKIPBACKWARDNOR_ICON, OSD2IMG_REC_PAUSENOR_ICON,
											                       OSD2IMG_REC_SKIPFORWARDNOR_ICON, OSD2IMG_REC_ADOSRCSELNOR_ICON};
	UI_RecPlayListItem_t tPrevRecListItem = UI_RECPLAYPAUSE_ITEM;
	OSD_IMG_INFO tRecPlayOsdImgInfo[2];

	if(UI_RECPLAYLIST_STATE != tUI_State)
		return;
	switch(tArrowKey)
	{
		case LEFT_ARROW:
			if((UI_RECSKIPBKFWD_ITEM == tUI_RecPlayListItem)  || (UI_RECFILE_PAUSE == tUI_RecPlaySts) ||
			   (UI_RECFILE_PLAY != tUI_RecPlayAct.tPlaySts))
				return;
			tPrevRecListItem = tUI_RecPlayListItem;
			tUI_RecPlayListItem--;
			break;
		case RIGHT_ARROW:
			if(((tUI_RecPlayListItem + 1) > UI_RECADOSRCSEL_ITEM) || (UI_RECFILE_PAUSE == tUI_RecPlaySts) ||
			   (UI_RECFILE_PLAY != tUI_RecPlayAct.tPlaySts))
				return;
			tPrevRecListItem = tUI_RecPlayListItem;
			tUI_RecPlayListItem++;
			break;
		case ENTER_ARROW:
			if(UI_RECPLAYPAUSE_ITEM == tUI_RecPlayListItem)
			{
				if(UI_RECFILE_PLAY != tUI_RecPlayAct.tPlaySts)
				{
					if(UI_RECPLAYPAUSE_ITEM == tUI_RecPlayListItem)
					{
						uwRecPlayListImgIdx[UI_RECPLAYPAUSE_ITEM] = OSD2IMG_REC_PAUSENOR_ICON;
						UI_PlayRecordFile(pUI_RecFilesInfo.uwRecFileSelIdx);
					}
					return;
				}
				if(ubPLY_GetPauseStatus() == PLY_PAUSE_OFF)
				{
				    if(ubPLY_Pause(PLY_PAUSE_ON) == 0)
                        return;
					uwRecPlayListImgIdx[UI_RECPLAYPAUSE_ITEM] = OSD2IMG_REC_PLAYNOR_ICON;
					tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_REC_PLAYHL_ICON, 1, &tRecPlayOsdImgInfo[0]);
					tOSD_Img2(&tRecPlayOsdImgInfo[0], OSD_UPDATE);
					tUI_RecPlaySts = UI_RECFILE_PAUSE;
				}
				else if(ubPLY_GetPauseStatus() == PLY_PAUSE_ON)
				{
				    if(ubPLY_Pause(PLY_PAUSE_OFF) == 0)
                        return;
					uwRecPlayListImgIdx[UI_RECPLAYPAUSE_ITEM] = OSD2IMG_REC_PAUSENOR_ICON;
					tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_REC_PAUSEHL_ICON, 1, &tRecPlayOsdImgInfo[0]);
					tOSD_Img2(&tRecPlayOsdImgInfo[0], OSD_UPDATE);
					tUI_RecPlaySts = UI_RECFILE_PLAY;
				}
			}
            else if(UI_RECSKIPBKFWD_ITEM == tUI_RecPlayListItem)
			{
				ubPLY_Jump(PLY_JUMP_BWD);
			}
            else if(UI_RECSKIPFRFWD_ITEM == tUI_RecPlayListItem)
			{
				ubPLY_Jump(PLY_JUMP_FWD);
			}
			else if(UI_RECADOSRCSEL_ITEM == tUI_RecPlayListItem)
			{
				if(KNL_DISP_SINGLE != tUI_RecPlayAct.ubPlayMode)
					UI_DrawRecordPlayAudioSource();
			}
			return;
		case EXIT_ARROW:
			tUI_RecPlayAct.tPlaySts   = UI_RECFILE_STOP;
			KNL_VideoPlayStop();
			tRecPlayOsdImgInfo[0].uwHSize  = 100;
			tRecPlayOsdImgInfo[0].uwVSize  = uwOSD_GetVSize();
			tRecPlayOsdImgInfo[0].uwXStart = 620;
			tRecPlayOsdImgInfo[0].uwYStart = 0;
			OSD_EraserImg1(&tRecPlayOsdImgInfo[0]);
			tUI_RecPlaySts      = UI_RECFILE_PLAY;
			tUI_RecPlayListItem = UI_RECPLAYPAUSE_ITEM;
			uwRecPlayListImgIdx[UI_RECPLAYPAUSE_ITEM] = OSD2IMG_REC_PAUSENOR_ICON;
#if APP_FS_FILE_LIST_STYLE
			if(KNL_ThmShowInfo.ubEnFlg==0)
			{
				OSD_Weight(OSD_WEIGHT_8DIV8);
				tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_SUBMENU, 1, &tRecPlayOsdImgInfo[0]);
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_FILE_SUBMENUICON, 1, &tRecPlayOsdImgInfo[1]);
				tOSD_Img1(&tRecPlayOsdImgInfo[0], OSD_QUEUE);
				tOSD_Img2(&tRecPlayOsdImgInfo[1], OSD_UPDATE);
			}
			else if(KNL_ThmShowInfo.ubEnFlg==1)
			{
				KNL_ROLE CamRole = KNL_STA1;
				KNL_SetRecordFunc(KNL_RECORDFUNC_DISABLE);
				KNL_SetDispSrc(KNL_DISP_LOCATION1, KNL_SRC_1_MAIN);
				VDO_SwitchDisplayType(KNL_DISP_SINGLE, &CamRole);
			}
#else						
			OSD_Weight(OSD_WEIGHT_8DIV8);
			tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_SUBMENU, 1, &tRecPlayOsdImgInfo[0]);
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_FILE_SUBMENUICON, 1, &tRecPlayOsdImgInfo[1]);
			tOSD_Img1(&tRecPlayOsdImgInfo[0], OSD_QUEUE);
			tOSD_Img2(&tRecPlayOsdImgInfo[1], OSD_UPDATE);
#endif
			UI_DrawRecordFileMenu();
			return;
		case DOWN_ARROW:
			if(UI_RECFILE_PLAY != tUI_RecPlayAct.tPlaySts)
			{
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_REC_PLAYHL_ICON, 1, &tRecPlayOsdImgInfo[0]);
				if(UI_RECPLAYPAUSE_ITEM != tUI_RecPlayListItem)
				{
					tPrevRecListItem = tUI_RecPlayListItem;
					tUI_RecPlayListItem = UI_RECPLAYPAUSE_ITEM;
					uwRecPlayListImgIdx[UI_RECPLAYPAUSE_ITEM] = OSD2IMG_REC_PLAYNOR_ICON;
					tOSD_Img2(&tRecPlayOsdImgInfo[0], OSD_QUEUE);
					break;
				}
				else
					tOSD_Img2(&tRecPlayOsdImgInfo[0], OSD_UPDATE);
			}
		default:
			return;
	}
	UI_DrawHLandNormalIcon(uwRecPlayListImgIdx[tPrevRecListItem], (uwRecPlayListImgIdx[tUI_RecPlayListItem] + UI_ICON_HIGHLIGHT));
}
//------------------------------------------------------------------------------
void UI_RecordPlayAdoSrcSelection(UI_ArrowKey_t tArrowKey)
{
	UI_CamNum_t tPreRecAdoCamNum = tUI_RecPlayAct.tAdoPlayCam;
	uint16_t uwXOffset = 0, uwYOffset = (DISPLAY_2T1R == tUI_CuSetting.ubTotalCamNum)?150:0;
	uint16_t uwYItemOffset = 150;
	OSD_IMG_INFO tOsdImgInfo;
	uint8_t ubCamNum;

	tPreRecAdoCamNum = tUI_RecPlayAct.tAdoPlayCam;
	switch(tArrowKey)
	{
		case LEFT_ARROW:
			if(CAM1 == tUI_RecPlayAct.tAdoPlayCam)
				return;
			for(ubCamNum = tUI_RecPlayAct.tAdoPlayCam; ubCamNum > CAM1; ubCamNum--)
			{
				if(NO_CAM != tUI_RecPlayAct.tVdoPlayCam[ubCamNum - 1])
				{
					tUI_RecPlayAct.tAdoPlayCam = (UI_CamNum_t)(ubCamNum - 1);
					break;
				}
				if((ubCamNum - 1) == CAM1)
					return;
			}
			break;
		case RIGHT_ARROW:
			if((tUI_RecPlayAct.tAdoPlayCam + 1) >= tUI_CuSetting.ubTotalCamNum)
				return;
			for(ubCamNum = (tUI_RecPlayAct.tAdoPlayCam + 1); ubCamNum < tUI_CuSetting.ubTotalCamNum; ubCamNum++)
			{
				if(NO_CAM != tUI_RecPlayAct.tVdoPlayCam[ubCamNum])
				{
					tUI_RecPlayAct.tAdoPlayCam = (UI_CamNum_t)ubCamNum;
					break;
				}
			}
			if(ubCamNum == tUI_CuSetting.ubTotalCamNum)
				return;
			break;
		case ENTER_ARROW:
			ubPLY_AdoChannelSet(tUI_RecPlayAct.tAdoPlayCam);
		case EXIT_ARROW:
			tOsdImgInfo.uwXStart  = 100;
			tOsdImgInfo.uwYStart  = 100;
			tOsdImgInfo.uwHSize   = 255;
			tOsdImgInfo.uwVSize   = 800;
			OSD_EraserImg2(&tOsdImgInfo);
			tUI_State = UI_RECPLAYLIST_STATE;
			return;
		default:
			return;
	}
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SELCAMNOR_ICON, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart += uwXOffset;
	tOsdImgInfo.uwYStart -= ((tPreRecAdoCamNum*111) + uwYOffset + uwYItemOffset);
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SELCAMHL_ICON, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart += uwXOffset;
	tOsdImgInfo.uwYStart -= ((tUI_RecPlayAct.tAdoPlayCam*111) + uwYOffset + uwYItemOffset);
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}
//------------------------------------------------------------------------------
void UI_SearchRecordFile(uint16_t uwRecFolderIndex)
{
	uint16_t uwLdState = UI_SEARCH_RECFILES;

	uwRecFolderIndex = uwRecFolderIndex;
	UI_SetLdDispStatus(UI_OSDLDDISP_ON);
	osMessagePut(osUI_OsdLdDispQueue, &uwLdState, 0);
	osDelay(200);
	pUI_RecFilesInfo.uwTotalRecFileNum = uwKNL_GetSortingFiles(uwRecFolderIndex, SORT_BY_TIME_DESCENDING, &pUI_RecFilesInfo.tRecFilesInfo[0]);
   UI_SetLdDispStatus(UI_OSDLDDISP_OFF);
	osDelay(350);
}
//------------------------------------------------------------------------------
void UI_ListRecFileInfo(uint16_t uwStartFileIdx, uint16_t uwEndFileIdx, OSD_UPDATE_TYP tUpdateMode)
{
	UI_RecImgColor_t tRecCamColor[] = {[CAM1] = UI_RECIMG_COLOR1,
									   [CAM2] = UI_RECIMG_COLOR2,
									   [CAM3] = UI_RECIMG_COLOR3,
									   [CAM4] = UI_RECIMG_COLOR4,}, tRecColor;
	UI_CamNum_t tRecSelCamNum;
	KNL_ROLE tRecRoleNum;
#if !APP_FS_FILE_LIST_STYLE	
	OSD_IMG_INFO tAsteriskOsdImgInfo;
#endif
	uint16_t uwIdx;
	uint8_t ubRecGrpLNum, ubRecGrpRNum, ubGrpFlag = FALSE;
	int iGrpRIdx = 1;
	char cRecFileName[32];
#if !APP_FS_FILE_LIST_STYLE
	char cRecFileCreated[20];
#endif	    

	for(uwIdx = uwStartFileIdx; uwIdx < uwEndFileIdx; uwIdx++)
	{
		ubRecGrpLNum = pUI_RecFilesInfo.tRecFilesInfo[uwIdx].HidnFileInfo.uwGroupIdx;
		iGrpRIdx     = uwIdx + ((uwIdx)?-1:1);
		ubRecGrpRNum = pUI_RecFilesInfo.tRecFilesInfo[iGrpRIdx].HidnFileInfo.uwGroupIdx;
		ubGrpFlag	 = (ubRecGrpLNum == ubRecGrpRNum)?TRUE:FALSE;
		if((uwIdx) && (FALSE == ubGrpFlag))
		{
			ubRecGrpRNum = pUI_RecFilesInfo.tRecFilesInfo[uwIdx + 1].HidnFileInfo.uwGroupIdx;
			ubGrpFlag	 = (ubRecGrpLNum == ubRecGrpRNum)?TRUE:FALSE;
		}
		tRecRoleNum = VDO_KNLSrcNumMap2KNLRoleNum((KNL_SRC)pUI_RecFilesInfo.tRecFilesInfo[uwIdx].HidnFileInfo.SrcNum);
		if(KNL_NONE == tRecRoleNum)
		{
			printd(DBG_ErrorLvl, "Get kernel role info Err!\n");
			tRecRoleNum = KNL_STA1;
		}
		APP_KNLRoleMap2CamNum(tRecRoleNum, tRecSelCamNum);
		tRecColor = (TRUE == ubGrpFlag)?UI_RECIMG_COLOR5:tRecCamColor[tRecSelCamNum];
		UI_SetRecImgColor(tRecColor);
		memset(cRecFileName, 0, sizeof(cRecFileName));
		snprintf(cRecFileName, sizeof(cRecFileName), "%s.%s", pUI_RecFilesInfo.tRecFilesInfo[uwIdx].HidnFileInfo.FileName.chName, pUI_RecFilesInfo.tRecFilesInfo[uwIdx].HidnFileInfo.FileName.chExt);
		cRecFileName[(pUI_RecFilesInfo.tRecFilesInfo[uwIdx].HidnFileInfo.FileName.ubLen + 4)] = 0;
#if APP_FS_FILE_LIST_STYLE
		if( memcmp(pUI_RecFilesInfo.tRecFilesInfo[uwIdx].HidnFileInfo.FileName.chExt, "MP4", 3)==0 )
		{
			KNL_SRC tVDO_KNLSrcNum;
			uint8_t ubDataPathSetupFlg = 0;
			
			KNL_ThmShowInfo.uwHSize_Now = pUI_RecFilesInfo.tRecFilesInfo[uwIdx].HidnFileInfo.SubHidnInfo.uwRes_HSize;
			KNL_ThmShowInfo.uwVSize_Now = pUI_RecFilesInfo.tRecFilesInfo[uwIdx].HidnFileInfo.SubHidnInfo.uwRes_VSize;
			
			if(KNL_ThmShowInfo.ubVdoDataPathSetUpFirstFlg==0)
			{
				KNL_ThmShowInfo.ubVdoDataPathSetUpFirstFlg = 1;
				ubDataPathSetupFlg = 1;
			}
			else if(KNL_ThmShowInfo.ubVdoDataPathSetUpFirstFlg==1)
			{
				if(!( KNL_ThmShowInfo.uwHSize_Now==KNL_ThmShowInfo.uwHSize_Pre && 
					  KNL_ThmShowInfo.uwVSize_Now==KNL_ThmShowInfo.uwVSize_Pre ))
				{
					ubDataPathSetupFlg = 1;
				}
			}
			
			if(ubDataPathSetupFlg)
			{
				VDO_Stop();
				KNL_VdoPathReset();
				tVDO_KNLSrcNum = VDO_GetSourceNumber(KNL_MAIN_PATH, KNL_STA1);
				KNL_SetVdoResolution(tVDO_KNLSrcNum, KNL_ThmShowInfo.uwHSize_Now, KNL_ThmShowInfo.uwVSize_Now);
				VDO_DataPathSetup(KNL_STA1, VDO_MAIN_SRC);
				KNL_ImageDecodeSetup(tVDO_KNLSrcNum);
				KNL_VdoStart(tVDO_KNLSrcNum);
				
				KNL_ThmShowInfo.uwHSize_Pre = KNL_ThmShowInfo.uwHSize_Now;
				KNL_ThmShowInfo.uwVSize_Pre = KNL_ThmShowInfo.uwVSize_Now;
			}
		}
		KNL_ShowingThm(uwIdx%REC_FILE_LIST_MAXNUM, &pUI_RecFilesInfo.tRecFilesInfo[uwIdx]);
#else		
		OSD_ImagePrintf(OSD_IMG_ROTATION_90, 325, 195 + ((uwIdx % REC_FILE_LIST_MAXNUM) * 40 + 5), tUI_RecOsdImgInfo, OSD_QUEUE, cRecFileName);
#if (APP_PHOTO_STORE_SEL == APP_FS_MEDIA_TYPE_SF)
		ubGrpFlag = FALSE;
#endif
		if(TRUE == ubGrpFlag)
		{
			char cListGrp[9];

			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_FILE_C5_ASTERISK, 1, &tAsteriskOsdImgInfo);
			tAsteriskOsdImgInfo.uwYStart = 280;	//! uwLCD_GetLcdVoSize() - 980 - 20
			tAsteriskOsdImgInfo.uwXStart = 195 + ((uwIdx % REC_FILE_LIST_MAXNUM) * 40 + 5);
			tOSD_Img2(&tAsteriskOsdImgInfo, OSD_QUEUE);
			snprintf(cListGrp, sizeof(cListGrp), "Grp%d", pUI_RecFilesInfo.tRecFilesInfo[uwIdx].HidnFileInfo.uwGroupIdx);
			OSD_ImagePrintf(OSD_IMG_ROTATION_90, 1000, 195 + ((uwIdx % REC_FILE_LIST_MAXNUM) * 40 + 5), tUI_RecOsdImgInfo, OSD_QUEUE, cListGrp);
		}
		memset(cRecFileCreated, 0, sizeof(cRecFileCreated));
		snprintf(cRecFileCreated, sizeof(cRecFileCreated), "%02d-%02d-%02d %02d:%02d", pUI_RecFilesInfo.tRecFilesInfo[uwIdx].HidnFileInfo.CreTime.uwYear, pUI_RecFilesInfo.tRecFilesInfo[uwIdx].HidnFileInfo.CreTime.ubMonth, pUI_RecFilesInfo.tRecFilesInfo[uwIdx].HidnFileInfo.CreTime.ubDay,
																					   pUI_RecFilesInfo.tRecFilesInfo[uwIdx].HidnFileInfo.CreTime.ubHour, pUI_RecFilesInfo.tRecFilesInfo[uwIdx].HidnFileInfo.CreTime.ubMin);
		OSD_ImagePrintf(OSD_IMG_ROTATION_90, 643, 195 + ((uwIdx % REC_FILE_LIST_MAXNUM) * 40 + 5), tUI_RecOsdImgInfo, ((uwIdx + 1) == uwEndFileIdx)?tUpdateMode:OSD_QUEUE, cRecFileCreated);
#endif
	}
}
//------------------------------------------------------------------------------
#if APP_FS_FILE_LIST_STYLE
void UI_RecordFileThmShowXYSetting(uint8_t ubOpIdx, OSD_IMG_INFO *Info)
{
	Info->uwXStart += (ubOpIdx/3)*150;
	Info->uwYStart -= ((ubOpIdx%3)*(pUI_RecFilesInfo.tRecFilesInfo[ubOpIdx].HidnFileInfo.FileName.ubLen+4)*25);	
}
void UI_DrawFileTime(uint16_t uwIdx)
{
	char cRecFileCreated[60];
	memset(cRecFileCreated, 0, sizeof(cRecFileCreated));
	UI_SetRecImgColor(UI_RECIMG_COLOR5);
	snprintf(cRecFileCreated, sizeof(cRecFileCreated), "%s.%s  %02d.%02d.%04d  %02d:%02d:%02d", 
		pUI_RecFilesInfo.tRecFilesInfo[uwIdx].HidnFileInfo.FileName.chName,
		pUI_RecFilesInfo.tRecFilesInfo[uwIdx].HidnFileInfo.FileName.chExt,
		pUI_RecFilesInfo.tRecFilesInfo[uwIdx].HidnFileInfo.CreTime.ubMonth, 
		pUI_RecFilesInfo.tRecFilesInfo[uwIdx].HidnFileInfo.CreTime.ubDay,
		pUI_RecFilesInfo.tRecFilesInfo[uwIdx].HidnFileInfo.CreTime.uwYear, 
		pUI_RecFilesInfo.tRecFilesInfo[uwIdx].HidnFileInfo.CreTime.ubHour, 
		pUI_RecFilesInfo.tRecFilesInfo[uwIdx].HidnFileInfo.CreTime.ubMin, 
		pUI_RecFilesInfo.tRecFilesInfo[uwIdx].HidnFileInfo.CreTime.ubSec);
	OSD_ImagePrintf(OSD_IMG_ROTATION_90, 20, 650, tUI_RecOsdImgInfo, OSD_UPDATE, cRecFileCreated);
}
void UI_DrawFileIdx(uint16_t uwIdx)
{
	char cRecFileNum[10];
	memset(cRecFileNum, 0, sizeof(cRecFileNum));
	UI_SetRecImgColor(UI_RECIMG_COLOR5);
	snprintf(cRecFileNum, sizeof(cRecFileNum), "%04d-%04d",uwIdx+1, pUI_RecFilesInfo.uwTotalRecFileNum);
	OSD_ImagePrintf(OSD_IMG_ROTATION_90, 780, 650, tUI_RecOsdImgInfo, OSD_UPDATE, cRecFileNum);
}
#endif
//------------------------------------------------------------------------------
void UI_DrawRecordFileMenu(void)
{
	OSD_IMG_INFO tOsdImgInfo[9];
	uint16_t uwFileStartIdx = 0, uwFileEndIdx = 0;
#if !APP_FS_FILE_LIST_STYLE
	uint8_t ubIdx;
#endif

	if(tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_FILE_CAM1, 9, &tOsdImgInfo[0]) != OSD_OK)
	{
		printd(DBG_ErrorLvl, "Load OSD Image FAIL, pls check (%d) !\n", __LINE__);
		return;
	}
	UI_SearchRecordFile(pUI_RecFoldersInfo.uwRecFolderSelIdx);
#if !APP_FS_FILE_LIST_STYLE
	for(ubIdx = 0; ubIdx < 8; ubIdx++)
		tOSD_Img2(&tOsdImgInfo[ubIdx], ((!pUI_RecFilesInfo.uwTotalRecFileNum) && (ubIdx == 7))?OSD_UPDATE:OSD_QUEUE);
#endif
	if(pUI_RecFilesInfo.uwTotalRecFileNum)
	{
		uwFileStartIdx = (pUI_RecFilesInfo.uwRecFileSelIdx / REC_FILE_LIST_MAXNUM) * REC_FILE_LIST_MAXNUM;
		uwFileEndIdx   = pUI_RecFilesInfo.uwTotalRecFileNum - uwFileStartIdx;
		uwFileEndIdx   = (uwFileEndIdx < REC_FILE_LIST_MAXNUM)?pUI_RecFilesInfo.uwTotalRecFileNum:(uwFileStartIdx + REC_FILE_LIST_MAXNUM);
#if APP_FS_FILE_LIST_STYLE
		tOsdImgInfo[8].uwXStart = 143;
		tOsdImgInfo[8].uwYStart = 1020;        
        UI_RecordFileThmShowXYSetting(pUI_RecFilesInfo.uwRecFileSelIdx%REC_FILE_LIST_MAXNUM, &tOsdImgInfo[8]);
        tOSD_Img2(&tOsdImgInfo[8], OSD_QUEUE);
        
        KNL_ThmShowInfo.ubLcdDispAddrKeepFlg = 0;
        KNL_ThmShowInfo.ubVdoDataPathSetUpFirstFlg = 0;
        
        KNL_SetRecordFunc(KNL_RECORDFUNC_DISABLE);
        
        KNL_FillWhiteOnLcd(720, 1280);
#else
		tOsdImgInfo[8].uwXStart += ((pUI_RecFilesInfo.uwRecFileSelIdx % REC_FILE_LIST_MAXNUM) * 40);
		tOSD_Img2(&tOsdImgInfo[8], OSD_QUEUE);
#endif
		UI_ListRecFileInfo(uwFileStartIdx, uwFileEndIdx, OSD_UPDATE);
#if APP_FS_FILE_LIST_STYLE
        UI_DrawFileTime(pUI_RecFilesInfo.uwRecFileSelIdx);
        UI_DrawFileIdx(pUI_RecFilesInfo.uwRecFileSelIdx);
#endif
	}
#if APP_FS_FILE_LIST_STYLE
    else
    {
        KNL_ThmShowInfo.ubLcdDispAddrKeepFlg = 0;
        KNL_ThmShowInfo.ubVdoDataPathSetUpFirstFlg = 0;
        KNL_SetRecordFunc(KNL_RECORDFUNC_DISABLE);
        KNL_LcdDisplaySetting();
        if(KNL_ThmShowInfo.ubLcdDispAddrKeepFlg==0)
        {
            KNL_ThmShowInfo.ubLcdDispAddrKeepFlg = 1;
            KNL_ThmShowInfo.ulLcdDispCurAddr = ulKNL_GetLcdDispAddr(KNL_SRC_1_MAIN);
            KNL_ThmShowInfo.ulLcdDispKeepAddr = KNL_ThmShowInfo.ulLcdDispCurAddr;
        }
        else if(KNL_ThmShowInfo.ubLcdDispAddrKeepFlg==1)
        {
            KNL_ThmShowInfo.ulLcdDispCurAddr = KNL_ThmShowInfo.ulLcdDispKeepAddr;
        }
        KNL_FillWhiteOnLcd(720, 1280);
        KNL_ActiveLcdDispBuf(KNL_SRC_1_MAIN);
    }
#endif
	tUI_State = UI_RECFILES_SEL_STATE;
}
//------------------------------------------------------------------------------
void UI_RecordFileSelection(UI_ArrowKey_t tArrowKey)
{
	OSD_IMG_INFO tFileSelOsdImgInfo[3];
	OSD_IMG_INFO tOsdImgInfo;
	uint16_t ubUI_RecFileIdx, ubUI_PrevRecFileIdx = 0;
	static uint8_t ubUI_RecFileDelFlag = FALSE;
	uint8_t ubUI_RecFileDelRdy = FALSE;
	KNL_RecordAct_t tUI_PlayAct = {KNL_RECORDFUNC_DISABLE, NULL,};

	if((!pUI_RecFilesInfo.uwTotalRecFileNum) && (EXIT_ARROW != tArrowKey))
		return;

	if(tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_FILE_FILESELECT, 3, &tFileSelOsdImgInfo[0]) != OSD_OK)
	{
		printd(DBG_ErrorLvl, "Load OSD Image FAIL, pls check (%d) !\n", __LINE__);
		return;
	}
#if APP_FS_FILE_LIST_STYLE
    tFileSelOsdImgInfo[0].uwXStart = tFileSelOsdImgInfo[1].uwXStart = tFileSelOsdImgInfo[2].uwXStart = 143;
    tFileSelOsdImgInfo[0].uwYStart = tFileSelOsdImgInfo[1].uwYStart = tFileSelOsdImgInfo[2].uwYStart = 1020;
#endif
	switch(tArrowKey)
	{
		case RIGHT_ARROW:
			if(FALSE == ubUI_RecFileDelFlag)
				return;
#if APP_FS_FILE_LIST_STYLE
            UI_RecordFileThmShowXYSetting(pUI_RecFilesInfo.uwRecFileSelIdx%REC_FILE_LIST_MAXNUM, &tFileSelOsdImgInfo[0]);
#else
			tFileSelOsdImgInfo[0].uwXStart += ((pUI_RecFilesInfo.uwRecFileSelIdx % REC_FILE_LIST_MAXNUM) * 40);
#endif
			tOSD_Img2(&tFileSelOsdImgInfo[0], OSD_UPDATE);
			ubUI_RecFileDelFlag = FALSE;
			return;
		case LEFT_ARROW:
			if(FALSE == ubUI_RecFileDelFlag)
			{
#if APP_FS_FILE_LIST_STYLE
                UI_RecordFileThmShowXYSetting(pUI_RecFilesInfo.uwRecFileSelIdx%REC_FILE_LIST_MAXNUM, &tFileSelOsdImgInfo[2]);
#else	
				tFileSelOsdImgInfo[2].uwXStart += ((pUI_RecFilesInfo.uwRecFileSelIdx % REC_FILE_LIST_MAXNUM) * 40);
#endif
				tOSD_Img2(&tFileSelOsdImgInfo[2], OSD_UPDATE);
			}
			ubUI_RecFileDelFlag = TRUE;
			return;
		case UP_ARROW:
			ubUI_RecFileDelFlag = FALSE;
			if(!pUI_RecFilesInfo.uwRecFileSelIdx)
				return;
			ubUI_PrevRecFileIdx = (pUI_RecFilesInfo.uwRecFileSelIdx % REC_FILE_LIST_MAXNUM);
			ubUI_RecFileIdx		= (--pUI_RecFilesInfo.uwRecFileSelIdx % REC_FILE_LIST_MAXNUM);
			break;
		case DOWN_ARROW:
			ubUI_RecFileDelFlag = FALSE;
			if((pUI_RecFilesInfo.uwRecFileSelIdx + 1) >= pUI_RecFilesInfo.uwTotalRecFileNum)
				return;
			ubUI_PrevRecFileIdx = (pUI_RecFilesInfo.uwRecFileSelIdx % REC_FILE_LIST_MAXNUM);
			ubUI_RecFileIdx		= (++pUI_RecFilesInfo.uwRecFileSelIdx % REC_FILE_LIST_MAXNUM);
			break;
		case ENTER_ARROW:
			if(TRUE == ubUI_RecFileDelFlag)
			{
				//! Delete Record File
				ubUI_RecFileDelRdy = UI_DeleteRecordFile(pUI_RecFilesInfo.uwRecFileSelIdx);
			}
			else
			{
				UI_PlayRecordFile(pUI_RecFilesInfo.uwRecFileSelIdx);
				return;
			}
		case EXIT_ARROW:
#if !APP_FS_FILE_LIST_STYLE		
			tOsdImgInfo.uwHSize  = 610;
			tOsdImgInfo.uwVSize  = 955;
			tOsdImgInfo.uwXStart = 100;
			tOsdImgInfo.uwYStart = 50;
#else
            tOsdImgInfo.uwHSize  = 720;
            tOsdImgInfo.uwVSize  = 1280;
            tOsdImgInfo.uwXStart = 0;
            tOsdImgInfo.uwYStart = 0;            
#endif
			OSD_EraserImg2(&tOsdImgInfo);
			ubUI_RecFileDelFlag = FALSE;
			if(TRUE == ubUI_RecFileDelRdy)
			{
                if(pUI_RecFilesInfo.uwRecFileSelIdx)
                {
                    ubUI_PrevRecFileIdx = (pUI_RecFilesInfo.uwRecFileSelIdx % REC_FILE_LIST_MAXNUM);
                    ubUI_RecFileIdx		= (--pUI_RecFilesInfo.uwRecFileSelIdx % REC_FILE_LIST_MAXNUM);
                }
				UI_DrawRecordFileMenu();
				return;
			}
#if APP_FS_FILE_LIST_STYLE
            KNL_ThmShowInfo.ubEnFlg = 0;
            KNL_ThmShowInfo.ubDispRevertFlg = 1;
            KNL_ResetLcdChannel();
#endif
            KNL_RevertDisplayMode();
            if(tKNL_GetRecordFunc() != KNL_RECORDFUNC_LOOP && tKNL_GetRecordFunc() != KNL_RECORDFUNC_MANU)
            {
    			tUI_PlayAct.tRecordFunc = KNL_RECORDFUNC_DISABLE;
                tUI_PlayAct.pRecordStsNtyCb = NULL;
    			tKNL_ExecRecordFunc(tUI_PlayAct);
            }
			pUI_RecFoldersInfo.uwRecFolderSelIdx = 0;
			pUI_RecFilesInfo.uwRecFileSelIdx = 0;
#if APP_FS_FILE_LIST_STYLE
			KNL_ThmShowInfo.ubInFldListFlg = 1;
			KNL_ThmShowInfo.ubDispRevertFlg = 0;
#endif			
			UI_DrawDCIMFolderMenu();
			tUI_CuSetting.IconSts.ubShowLostLogoFlag = FALSE;
			return;
		default:
			return;
	}
	if((((!(pUI_RecFilesInfo.uwRecFileSelIdx % REC_FILE_LIST_MAXNUM)) && (pUI_RecFilesInfo.uwRecFileSelIdx) && (ubUI_PrevRecFileIdx == REC_FILE_LIST_MAXNUM - 1))) ||
	   ((ubUI_PrevRecFileIdx == 0) && (ubUI_RecFileIdx == REC_FILE_LIST_MAXNUM - 1)))
	{
		uint16_t uwFileStartIdx = 0, uwFileEndIdx = 0;
#if !APP_FS_FILE_LIST_STYLE
		tOsdImgInfo.uwHSize  = 415;
		tOsdImgInfo.uwVSize  = 955;
		tOsdImgInfo.uwXStart = 195;
		tOsdImgInfo.uwYStart = 50;
		OSD_EraserImg2(&tOsdImgInfo);
#else
		tOsdImgInfo.uwXStart = 0;
		tOsdImgInfo.uwYStart = 0;
		tOsdImgInfo.uwHSize  = 720;
		tOsdImgInfo.uwVSize  = 1280;		
		OSD_EraserImg2(&tOsdImgInfo);
		KNL_FillWhiteOnLcd(720, 1280);
#endif		
		if(DOWN_ARROW == tArrowKey)
		{
			uwFileStartIdx = pUI_RecFilesInfo.uwRecFileSelIdx;
			uwFileEndIdx   = pUI_RecFilesInfo.uwTotalRecFileNum - pUI_RecFilesInfo.uwRecFileSelIdx;
			uwFileEndIdx   = (uwFileEndIdx < REC_FILE_LIST_MAXNUM)?pUI_RecFilesInfo.uwTotalRecFileNum:(uwFileStartIdx + REC_FILE_LIST_MAXNUM);
		}
		else
		{
			uwFileStartIdx = (pUI_RecFilesInfo.uwRecFileSelIdx / REC_FILE_LIST_MAXNUM) * REC_FILE_LIST_MAXNUM;
			uwFileEndIdx   = pUI_RecFilesInfo.uwRecFileSelIdx + 1;
		}
		UI_ListRecFileInfo(uwFileStartIdx, uwFileEndIdx, OSD_QUEUE);
	}
#if APP_FS_FILE_LIST_STYLE
	UI_RecordFileThmShowXYSetting(ubUI_RecFileIdx, &tFileSelOsdImgInfo[0]); 
    UI_RecordFileThmShowXYSetting(ubUI_PrevRecFileIdx, &tFileSelOsdImgInfo[1]);
#else	
	tFileSelOsdImgInfo[0].uwXStart += (ubUI_RecFileIdx * 40);
	tFileSelOsdImgInfo[1].uwXStart += (ubUI_PrevRecFileIdx * 40);
#endif
	tOSD_Img2(&tFileSelOsdImgInfo[0], OSD_QUEUE);
	tOSD_Img2(&tFileSelOsdImgInfo[1], OSD_UPDATE);
#if APP_FS_FILE_LIST_STYLE
	UI_DrawFileTime(pUI_RecFilesInfo.uwRecFileSelIdx);
	UI_DrawFileIdx(pUI_RecFilesInfo.uwRecFileSelIdx);
#endif	
}
//------------------------------------------------------------------------------
void UI_RecordSubMenuPage(UI_ArrowKey_t tArrowKey)
{
	UI_MenuAct_t tMenuAct;

	if(tUI_State == UI_MAINMENU_STATE)
	{
		//! Draw Record sub menu page
		//! Check Select Camera number
		UI_DrawSubMenuPage(RECORD_ITEM);
		return;
	}
	tMenuAct = UI_KeyEventMap2SubMenuInfo(&tArrowKey, &tUI_SubMenuItem[RECORD_ITEM]);
	switch(tMenuAct)
	{
		case DRAW_HIGHLIGHT_MENUICON:
		{
			uint16_t uwSubMenuItemOsdImg[RECITEM_MAX] = {OSD2IMG_REC1MODENOR_ITEM, OSD2IMG_REC1TIMENOR_ITEM, OSD2IMG_SDFNOR_ITEM};
			uint8_t ubSubMenuItemPreIdx = tUI_SubMenuItem[RECORD_ITEM].tSubMenuInfo.ubItemPreIdx;
			uint8_t ubSubMenuItemIdx = tUI_SubMenuItem[RECORD_ITEM].tSubMenuInfo.ubItemIdx;
			OSD_IMG_INFO tOsdImgInfo[2];

			UI_DrawHLandNormalIcon(uwSubMenuItemOsdImg[ubSubMenuItemPreIdx], (uwSubMenuItemOsdImg[ubSubMenuItemIdx] + UI_ICON_HIGHLIGHT));
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_REC1OPTMARKNOR_ICON, 2, &tOsdImgInfo[0]);
			if(ubSubMenuItemPreIdx != SDCARD_ITEM)
			{
				tOsdImgInfo[0].uwXStart += (ubSubMenuItemPreIdx * 0x40);
				tOSD_Img2(&tOsdImgInfo[0], (ubSubMenuItemIdx == SDCARD_ITEM)?OSD_UPDATE:OSD_QUEUE);
			}
			if(ubSubMenuItemIdx == SDCARD_ITEM)
				break;
			tOsdImgInfo[1].uwXStart += (ubSubMenuItemIdx * 0x40);
			tOSD_Img2(&tOsdImgInfo[1], OSD_UPDATE);
			break;
		}
		case DRAW_MENUPAGE:
		{
			OSD_IMG_INFO tOsdImgInfo;
			uint8_t ubSubMenuItemIdx = tUI_SubMenuItem[RECORD_ITEM].tSubMenuInfo.ubItemIdx;
			uint16_t uwOptMarkOffset = 0;
			uint8_t ubImgIdx;

			//! Draw sub sub menu page
			if(ubSubMenuItemIdx == RECMODE_ITEM)
			{
				uint16_t uwSubSubMenuItemOsdImg[REC_RECMODE_MAX] = {OSD2IMG_REC1LOOPNOR_ICON, OSD2IMG_REC1MANUNOR_ICON, OSD2IMG_REC1TRIGNOR_ICON, OSD2IMG_REC1OFFNOR_ICON};

				uwSubSubMenuItemOsdImg[tUI_CuSetting.RecInfo.tREC_Mode] += UI_ICON_HIGHLIGHT;
				tOsdImgInfo.uwHSize  = 335;
				tOsdImgInfo.uwVSize  = 250;
				tOsdImgInfo.uwXStart = 130;
				tOsdImgInfo.uwYStart = 300;
				OSD_EraserImg2(&tOsdImgInfo);
				for(ubImgIdx = 0; ubImgIdx < REC_RECMODE_MAX; ubImgIdx++)
				{
					tOSD_GetOsdImgInfor(1, OSD_IMG2, uwSubSubMenuItemOsdImg[ubImgIdx], 1, &tOsdImgInfo);
					tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
				}
			}
			else if(ubSubMenuItemIdx == RECTIME_ITEM)
			{
				uint16_t uwSubSubMenuItemOsdImg[RECTIME_MAX] = {OSD2IMG_REC1RT1MINNOR_ICON, OSD2IMG_REC1RT2MINNOR_ICON,
																OSD2IMG_REC1RT5MINNOR_ICON};
				uwSubSubMenuItemOsdImg[tUI_CuSetting.RecInfo.tREC_Time] += UI_ICON_HIGHLIGHT;
				tOsdImgInfo.uwHSize  = 335;
				tOsdImgInfo.uwVSize  = 250;
				tOsdImgInfo.uwXStart = 198;
				tOsdImgInfo.uwYStart = 300;
				OSD_EraserImg2(&tOsdImgInfo);
				for(ubImgIdx = 0; ubImgIdx < RECTIME_MAX; ubImgIdx++)
				{
					tOSD_GetOsdImgInfor(1, OSD_IMG2, uwSubSubMenuItemOsdImg[ubImgIdx], 1, &tOsdImgInfo);
					tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
				}
				uwOptMarkOffset = 0x40;
			}
			else if(ubSubMenuItemIdx == SDCARD_ITEM)
			{
				OSD_IMG_INFO tSdfOsdImgInfo[6];

				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SDFBG_ICON, 6, &tSdfOsdImgInfo);
				tOSD_Img2(&tSdfOsdImgInfo[0], OSD_QUEUE);
				tOSD_Img2(&tSdfOsdImgInfo[2], OSD_QUEUE);
				tOSD_Img2(&tSdfOsdImgInfo[3], OSD_UPDATE);
				tUI_SdCardSts = UI_SD_CFM;
				tUI_State = UI_SDCARDFMT_STATE;
				break;				
			}
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_REC1OPTMARKHL_ICON, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart += uwOptMarkOffset;
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			tUI_State = UI_SUBSUBMENU_STATE;
			break;
		}
		case EXIT_MENUFUNC:
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_RecordUpdateSubSubMenuItemIndex(UI_RecSubSubMenuItem_t *ptSubSubMenuItem, UI_RecordSubMenuItemList_t *tSubMenuItem, uint8_t *Update_Flag)
{
	switch(*tSubMenuItem)
	{
		case RECMODE_ITEM:
			ptSubSubMenuItem->tRecordS[*tSubMenuItem].tSubMenuInfo.ubItemIdx = tUI_CuSetting.RecInfo.tREC_Mode;
			break;
		case RECTIME_ITEM:
			ptSubSubMenuItem->tRecordS[*tSubMenuItem].tSubMenuInfo.ubItemIdx = tUI_CuSetting.RecInfo.tREC_Time;
			break;
		default:
			return;
	}
	*Update_Flag = TRUE;
}
//------------------------------------------------------------------------------
void UI_RecordDrawSubSubMenuItem(UI_RecSubSubMenuItem_t *ptSubSubMenuItem, UI_RecordSubMenuItemList_t *tSubMenuItem)
{
	uint8_t ubSubSubMenuItemPreIdx = ptSubSubMenuItem->tRecordS[*tSubMenuItem].tSubMenuInfo.ubItemPreIdx;
	uint8_t ubSubSubMenuItemIdx = ptSubSubMenuItem->tRecordS[*tSubMenuItem].tSubMenuInfo.ubItemIdx;
	switch(*tSubMenuItem)
	{
		case RECMODE_ITEM:
		{
			uint16_t uwSubSubMenuItemOsdImg[REC_RECMODE_MAX] = {OSD2IMG_REC1LOOPNOR_ICON, OSD2IMG_REC1MANUNOR_ICON, OSD2IMG_REC1TRIGNOR_ICON, OSD2IMG_REC1OFFNOR_ICON};
			UI_DrawHLandNormalIcon(uwSubSubMenuItemOsdImg[ubSubSubMenuItemPreIdx], (uwSubSubMenuItemOsdImg[ubSubSubMenuItemIdx]+UI_ICON_HIGHLIGHT));
			break;
		}
		case RECTIME_ITEM:
		{
			uint16_t uwSubSubMenuItemOsdImg[RECTIME_MAX] = {OSD2IMG_REC1RT1MINNOR_ICON, OSD2IMG_REC1RT2MINNOR_ICON,
															OSD2IMG_REC1RT5MINNOR_ICON};
			UI_DrawHLandNormalIcon(uwSubSubMenuItemOsdImg[ubSubSubMenuItemPreIdx], (uwSubSubMenuItemOsdImg[ubSubSubMenuItemIdx]+UI_ICON_HIGHLIGHT));
			break;
		}
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_RecordSDFSubSubMenu(UI_ArrowKey_t tArrowKey)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint16_t uwSdfSelImgIdx[2] = {OSD2IMG_SDFNONOR_ICON, OSD2IMG_SDFYESNOR_ICON};
	static uint8_t ubUI_SdfSel = FALSE;
	uint8_t ubPrevSdfSel;
	FS_FMT_STATUS tSDF_Ret = FORMAT_FAIL;

	switch(tArrowKey)
	{
		case LEFT_ARROW:
			if(TRUE == ubUI_SdfSel)
				return;
			ubPrevSdfSel = ubUI_SdfSel;
			ubUI_SdfSel = TRUE;
			break;
		case RIGHT_ARROW:
			if(FALSE == ubUI_SdfSel)
				return;
			ubPrevSdfSel = ubUI_SdfSel;
			ubUI_SdfSel = FALSE;
			break;
		case ENTER_ARROW:
			if(TRUE == ubUI_SdfSel)
            {
				if(UI_REC_START == tUI_RecPlayAct.tRecAct)
					UI_VideoRecordingExec(UI_REC_STOP);
            }
		case EXIT_ARROW:
			tOsdImgInfo.uwHSize  = 310;
			tOsdImgInfo.uwVSize  = 600;
			tOsdImgInfo.uwXStart = 310;
			tOsdImgInfo.uwYStart = 400;
			OSD_EraserImg2(&tOsdImgInfo);
			if((ENTER_ARROW == tArrowKey) && (TRUE == ubUI_SdfSel))
			{
				OSD_IMG_INFO tSdfOsdImgInfo[3];
				UI_FuncExecMsg_t tWorkAct;
				uint8_t ubSdFmtRet;

				tWorkAct.uwFunc = UI_SDCARDFMT_ACT;
				osMessagePut(osUI_FuncsExecQue, &tWorkAct, 0);
				osMessageGet(osUI_FuncsFinExecQue, &ubSdFmtRet, osWaitForever);
				tSDF_Ret = (rUI_SUCCESS == ubSdFmtRet)?FORMAT_OK:FORMAT_FAIL;
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SDFRET_BG, 3, &tSdfOsdImgInfo);
				tOSD_Img2(&tSdfOsdImgInfo[0], OSD_QUEUE);
				tOSD_Img2(&tSdfOsdImgInfo[1+tSDF_Ret], OSD_UPDATE);
				osDelay(1000);
				OSD_EraserImg2(&tOsdImgInfo);
			}
			ubUI_SdfSel = FALSE;
			tUI_State 	= UI_SUBMENU_STATE;
			return;
		default:
			return;
	}
	UI_DrawHLandNormalIcon(uwSdfSelImgIdx[ubPrevSdfSel], (uwSdfSelImgIdx[ubUI_SdfSel]+UI_ICON_HIGHLIGHT));
}
//------------------------------------------------------------------------------
void UI_RecordSubSubMenuPage(UI_ArrowKey_t tArrowKey)
{
	static UI_RecSubSubMenuItem_t tRecSubSubMenuItem = 
	{
		{
		   { 0, REC_RECMODE_MAX },{ 0, RECTIME_MAX }, { 0, 2 }
		},
	};
	static uint8_t ubUI_RecStsUpdateFlag = FALSE;
	UI_RecordSubMenuItemList_t tSubMenuItem = (UI_RecordSubMenuItemList_t)tUI_SubMenuItem[RECORD_ITEM].tSubMenuInfo.ubItemIdx;
	UI_MenuAct_t tMenuAct;

	if(FALSE == ubUI_RecStsUpdateFlag)
		UI_RecordUpdateSubSubMenuItemIndex(&tRecSubSubMenuItem, &tSubMenuItem, &ubUI_RecStsUpdateFlag);
	tMenuAct = UI_KeyEventMap2SubSubMenuInfo(&tArrowKey, &tRecSubSubMenuItem.tRecordS[tSubMenuItem]);
	switch(tMenuAct)
	{
		case DRAW_HIGHLIGHT_MENUICON:
			UI_RecordDrawSubSubMenuItem(&tRecSubSubMenuItem, &tSubMenuItem);
			break;
		case EXECUTE_MENUFUNC:
		{
			uint8_t *pUI_Mode = (tSubMenuItem == RECMODE_ITEM)?(uint8_t *)&tUI_CuSetting.RecInfo.tREC_Mode:
					            (tSubMenuItem == RECTIME_ITEM)?(uint8_t *)&tUI_CuSetting.RecInfo.tREC_Time:NULL;
			if(pUI_Mode)
			{
				uint8_t ubItemIdx = tRecSubSubMenuItem.tRecordS[tSubMenuItem].tSubMenuInfo.ubItemIdx;

				if(*pUI_Mode != ubItemIdx)
				{
					if((tSubMenuItem == RECMODE_ITEM) && (REC_TRIGGER != ubItemIdx))
					{
					#if APP_SD_FUNC_ENABLE
						#if !APP_REC_FUNC_ENABLE
						if((UI_VDORECLOOP_MODE == ubItemIdx) || (UI_VDORECMANU_MODE == ubItemIdx))
							break;
						#endif
                        if(UI_REC_START == tUI_RecPlayAct.tRecAct)
							UI_VideoRecordingExec(UI_REC_STOP);
						*pUI_Mode = ubItemIdx;
						tUI_CuSetting.tVdoMode = (REC_OFF == *pUI_Mode)?UI_PHOTOCAP_MODE:UI_RECORDING_MODE;
						UI_UpdateDevStatusInfo();
						if((UI_RECORDING_MODE == tUI_CuSetting.tVdoMode) && (REC_LOOPING == tUI_CuSetting.RecInfo.tREC_Mode))
						{
							UI_VideoRecordingExec(UI_REC_START);
							if((tUI_CuSetting.ubVdoRecStsCnt) || (UI_SDCARDFMT_STATE == tUI_State))
								break;
						}
                        else
                            KNL_SetRecordFunc(KNL_RECORDFUNC_DISABLE);
					#else
						break;
					#endif
					}
					else if(tSubMenuItem == RECTIME_ITEM)
					{
					#if (APP_SD_FUNC_ENABLE && APP_REC_FUNC_ENABLE)
						UI_RecordingAct_t tCurRecAct;

						tCurRecAct = tUI_RecPlayAct.tRecAct;
						if(UI_REC_START == tUI_RecPlayAct.tRecAct)
							UI_VideoRecordingExec(UI_REC_STOP);
						*pUI_Mode = ubItemIdx;
						if(tUI_CuSetting.RecInfo.tREC_Time == RECTIME_1MIN)
							REC_TimeSet(0,60);
						else if(tUI_CuSetting.RecInfo.tREC_Time == RECTIME_3MIN)
							REC_TimeSet(0,180);
						else if(tUI_CuSetting.RecInfo.tREC_Time == RECTIME_5MIN)
							REC_TimeSet(0,300);
						UI_UpdateDevStatusInfo();
						if(UI_REC_START == tCurRecAct)
							UI_VideoRecordingExec(UI_REC_START);
					#else
						break;
					#endif
					}
				}
			}
		}
		case EXIT_MENUFUNC:
		{
			OSD_IMG_INFO tOsdImgInfo[2];

			tOsdImgInfo[0].uwHSize  = 335;
			tOsdImgInfo[0].uwVSize  = 250;
			tOsdImgInfo[0].uwXStart = 110;
			tOsdImgInfo[0].uwYStart = 300;
			OSD_EraserImg2(&tOsdImgInfo[0]);
			tOSD_GetOsdImgInfor(1, OSD_IMG2, (OSD2IMG_REC1LOOPWR_ICON+tUI_CuSetting.RecInfo.tREC_Mode), 1, &tOsdImgInfo[0]);
			tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);
			tOSD_GetOsdImgInfor(1, OSD_IMG2, (OSD2IMG_REC1RT1MINWR_ICON+tUI_CuSetting.RecInfo.tREC_Time), 1, &tOsdImgInfo[0]);
			tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_REC1OPTMARKNOR_ICON, 2, &tOsdImgInfo[0]);
			tOsdImgInfo[0].uwXStart += (((tSubMenuItem == RECMODE_ITEM)?1:0) * 0x40);
			tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);
			tOsdImgInfo[1].uwXStart += (((tSubMenuItem == RECMODE_ITEM)?0:1) * 0x40);
			tOSD_Img2(&tOsdImgInfo[1], OSD_UPDATE);
			ubUI_RecStsUpdateFlag = FALSE;
			tUI_State = UI_SUBMENU_STATE;
			break;
		}
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_PhotoSubMenuPage(UI_ArrowKey_t tArrowKey)
{
	if(tUI_State == UI_MAINMENU_STATE)
	{
		ubUI_RecSubMenuFlag = TRUE;
		UI_DrawDCIMFolderMenu();
		return;
	}
	UI_DCIMFolderSelection(tArrowKey);
}
//------------------------------------------------------------------------------
void UI_PhotoUpdateSubSubMenuItemIndex(UI_PhotoSubSubMenuItem_t *ptSubSubMenuItem, UI_PhotoSubMenuItemList_t *tSubMenuItem, uint8_t *Update_Flag)
{
	UI_CamNum_t tCamNum = tCamSelect.tCamNum4PhotoSub;
	switch(*tSubMenuItem)
	{
		case PHOTOFUNC_ITEM:
			ptSubSubMenuItem->tPhotoS[tCamNum][*tSubMenuItem].tSubMenuInfo.ubItemIdx = tUI_CamStatus[tCamNum].tPHOTO_Func;
			break;
		case PHOTORES_ITEM:
			ptSubSubMenuItem->tPhotoS[tCamNum][*tSubMenuItem].tSubMenuInfo.ubItemIdx = tUI_CamStatus[tCamNum].tPHOTO_Resolution;
			break;
		default:
			return;
	}
	*Update_Flag = TRUE;
}
//------------------------------------------------------------------------------
void UI_PhotoDrawSubSubMenuItem(UI_PhotoSubSubMenuItem_t *ptSubSubMenuItem, UI_PhotoSubMenuItemList_t *tSubMenuItem)
{
	UI_CamNum_t tCamNum = tCamSelect.tCamNum4PhotoSub;
	uint8_t ubSubSubMenuItemPreIdx = ptSubSubMenuItem->tPhotoS[tCamNum][*tSubMenuItem].tSubMenuInfo.ubItemPreIdx;
	uint8_t ubSubSubMenuItemIdx = ptSubSubMenuItem->tPhotoS[tCamNum][*tSubMenuItem].tSubMenuInfo.ubItemIdx;
	switch(*tSubMenuItem)
	{
		case PHOTOFUNC_ITEM:
			break;
		case PHOTORES_ITEM:
		{
			uint16_t uwSubSubMenuItemOsdImg[PHOTORES_MAX] = {OSD2IMG_PRES3MNOR_ICON, OSD2IMG_PRES5MNOR_ICON, OSD2IMG_PRES12MNOR_ICON};
			UI_DrawHLandNormalIcon(uwSubSubMenuItemOsdImg[ubSubSubMenuItemPreIdx], (uwSubSubMenuItemOsdImg[ubSubSubMenuItemIdx]+UI_ICON_HIGHLIGHT));
			break;
		}
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_PhotoSubSubMenuPage(UI_ArrowKey_t tArrowKey)
{
	static UI_PhotoSubSubMenuItem_t tPhotoSubSubMenuItem = 
	{
		{
		   {{ 0, 0 },{ 0, PHOTOFUNC_MAX },{ 0, PHOTORES_MAX }},
		   {{ 0, 0 },{ 0, PHOTOFUNC_MAX },{ 0, PHOTORES_MAX }},
		   {{ 0, 0 },{ 0, PHOTOFUNC_MAX },{ 0, PHOTORES_MAX }},
		   {{ 0, 0 },{ 0, PHOTOFUNC_MAX },{ 0, PHOTORES_MAX }}
		},
	};
	static uint8_t ubUI_PhotoStsUpdateFlag = FALSE;
	UI_PhotoSubMenuItemList_t tSubMenuItem = (UI_PhotoSubMenuItemList_t)tUI_SubMenuItem[PHOTO_ITEM].tSubMenuInfo.ubItemIdx;
	UI_CamNum_t tCamNum 				   = tCamSelect.tCamNum4PhotoSub;
	UI_MenuAct_t tMenuAct;

	if(FALSE == ubUI_PhotoStsUpdateFlag)
		UI_PhotoUpdateSubSubMenuItemIndex(&tPhotoSubSubMenuItem, &tSubMenuItem, &ubUI_PhotoStsUpdateFlag);
	tMenuAct = UI_KeyEventMap2SubSubMenuInfo(&tArrowKey, &tPhotoSubSubMenuItem.tPhotoS[tCamNum][tSubMenuItem]);
	switch(tMenuAct)
	{
		case DRAW_HIGHLIGHT_MENUICON:
			UI_PhotoDrawSubSubMenuItem(&tPhotoSubSubMenuItem, &tSubMenuItem);
			break;
		case EXECUTE_MENUFUNC:
		{
			UI_CUReqCmd_t   tPhotoCmd;
			UI_PhotoFunction_t tPHOTO_Func = (tUI_CamStatus[tCamNum].tPHOTO_Func == PHOTOFUNC_OFF)?PHOTOFUNC_ON:PHOTOFUNC_OFF;

			if((tSubMenuItem != PHOTOFUNC_ITEM) && (tSubMenuItem != PHOTORES_ITEM))
				break;
			tPhotoCmd.tDS_CamNum = tCamNum;
			tPhotoCmd.ubCmd[UI_TWC_TYPE]	 = UI_SETTING;
			tPhotoCmd.ubCmd[UI_SETTING_ITEM] = (tSubMenuItem == PHOTOFUNC_ITEM)?PHOTOFUNC_ITEM:PHOTORES_ITEM;
			tPhotoCmd.ubCmd[UI_SETTING_DATA] = (tSubMenuItem == PHOTOFUNC_ITEM)?tPHOTO_Func:
												tPhotoSubSubMenuItem.tPhotoS[tCamNum][tSubMenuItem].tSubMenuInfo.ubItemIdx;
			tPhotoCmd.ubCmd_Len  = 3;
			if(UI_SendRequestToCAM(osThreadGetId(), &tPhotoCmd) == rUI_SUCCESS)
			{
				if(tSubMenuItem == PHOTOFUNC_ITEM)
					tUI_CamStatus[tCamNum].tPHOTO_Func = tPHOTO_Func;
				if(tSubMenuItem == PHOTORES_ITEM)
					tUI_CamStatus[tCamNum].tPHOTO_Resolution = (UI_PhotoResolution_t)tPhotoSubSubMenuItem.tPhotoS[tCamNum][tSubMenuItem].tSubMenuInfo.ubItemIdx;
			}
		}
		case EXIT_MENUFUNC:
		{
			OSD_IMG_INFO tOsdImgInfo;

			if(tSubMenuItem == PHOTOFUNC_ITEM)
			{
				tOSD_GetOsdImgInfor(1, OSD_IMG2, (OSD2IMG_PHOTOOFFWR_ICON+tUI_CamStatus[tCamNum].tPHOTO_Func), 1, &tOsdImgInfo);
				tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_OPTMARKNOR_ICON, 1, &tOsdImgInfo);
				tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			}
			if(tSubMenuItem == PHOTORES_ITEM)
			{
				tOsdImgInfo.uwHSize  = 335;
				tOsdImgInfo.uwVSize  = 250;
				tOsdImgInfo.uwXStart = 285;
				tOsdImgInfo.uwYStart = 312;
				OSD_EraserImg2(&tOsdImgInfo);
				tOSD_GetOsdImgInfor(1, OSD_IMG2, (OSD2IMG_PRES3MWR_ICON+tUI_CamStatus[tCamNum].tPHOTO_Resolution), 1, &tOsdImgInfo);
				tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_OPTMARKNOR_ICON, 1, &tOsdImgInfo);
				tOsdImgInfo.uwXStart += 0x40;
				tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);				
			}
			ubUI_PhotoStsUpdateFlag = FALSE;
			tUI_State = UI_SUBMENU_STATE;
			break;
		}
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_PlaybackSubMenuPage(UI_ArrowKey_t tArrowKey)
{
	if(UI_MAINMENU_STATE == tUI_State)
	{
		UI_DrawSubMenuPage(PLAYBACK_ITEM);
		return;
	}
	UI_DCIMFolderSelection(tArrowKey);
}
//------------------------------------------------------------------------------
void UI_PowerSaveSubMenuPage(UI_ArrowKey_t tArrowKey)
{
	if(UI_MAINMENU_STATE == tUI_State)
	{
		UI_DrawSubMenuPage(PS_ITEM);
		return;
	}
}
//------------------------------------------------------------------------------
void UI_DrawSysDateTime(UI_CalendarItem_t tShowItem, UI_IconType_t tIconType, RTC_Calendar_t *ptSysCalendar)
{
	static OSD_IMG_INFO tNumOsdImgArrayInfo[22] = {0};
	static uint8_t ubUI_RdNumOsdImgFlag = FALSE;
	uint8_t ubDateOffset[2] = {0}, i;

	if(FALSE == ubUI_RdNumOsdImgFlag)
	{
		uint16_t uwSubSubMenuItemOsdImg[22] = {OSD2IMG_NUMBER0NOR_ICON, OSD2IMG_NUMBER0HL_ICON,
											   OSD2IMG_NUMBER1NOR_ICON, OSD2IMG_NUMBER1HL_ICON,
											   OSD2IMG_NUMBER2NOR_ICON, OSD2IMG_NUMBER2HL_ICON,
											   OSD2IMG_NUMBER3NOR_ICON, OSD2IMG_NUMBER3HL_ICON,
											   OSD2IMG_NUMBER4NOR_ICON, OSD2IMG_NUMBER4HL_ICON,
											   OSD2IMG_NUMBER5NOR_ICON, OSD2IMG_NUMBER5HL_ICON,
											   OSD2IMG_NUMBER6NOR_ICON, OSD2IMG_NUMBER6HL_ICON,
											   OSD2IMG_NUMBER7NOR_ICON, OSD2IMG_NUMBER7HL_ICON,
											   OSD2IMG_NUMBER8NOR_ICON, OSD2IMG_NUMBER8HL_ICON,
											   OSD2IMG_NUMBER9NOR_ICON, OSD2IMG_NUMBER9HL_ICON,
											   OSD2IMG_COLONNOR_ICON, OSD2IMG_DIVISIONNOR_ICON};
		tOSD_GetOsdImgInfor(1, OSD_IMG2, uwSubSubMenuItemOsdImg[21], 1, &tNumOsdImgArrayInfo[21]);
		tOSD_GetOsdImgInfor(1, OSD_IMG2, uwSubSubMenuItemOsdImg[0], 21, &tNumOsdImgArrayInfo[0]);
		ubUI_RdNumOsdImgFlag = TRUE;
	}
	switch(tShowItem)
	{
		case ALLCALE_ITEM:
		case YEAR_ITEM:
			tNumOsdImgArrayInfo[4+tIconType].uwXStart = 170;
			tNumOsdImgArrayInfo[4+tIconType].uwYStart = 390;
			tOSD_Img2(&tNumOsdImgArrayInfo[4+tIconType], OSD_QUEUE);
			tNumOsdImgArrayInfo[tIconType].uwXStart = 170;
			tNumOsdImgArrayInfo[tIconType].uwYStart = 360;
			tOSD_Img2(&tNumOsdImgArrayInfo[tIconType], OSD_QUEUE);
			ubDateOffset[0] = (ptSysCalendar->uwYear - 2000) / 10;
			ubDateOffset[1] = (ptSysCalendar->uwYear - 2000) - (ubDateOffset[0] * 10);
			for(i = 0; i < 2; i++)
			{
				tNumOsdImgArrayInfo[(ubDateOffset[i]*2)+tIconType].uwXStart = 170;
				tNumOsdImgArrayInfo[(ubDateOffset[i]*2)+tIconType].uwYStart = 330 - (i * 30);
				tOSD_Img2(&tNumOsdImgArrayInfo[(ubDateOffset[i]*2)+tIconType], (i)?(tShowItem != ALLCALE_ITEM)?OSD_UPDATE:OSD_QUEUE:OSD_QUEUE);
			}
			if(tShowItem != ALLCALE_ITEM)
				break;
		case MONTH_ITEM:
			ubDateOffset[0] = ptSysCalendar->ubMonth / 10;
			ubDateOffset[1] = ptSysCalendar->ubMonth - (ubDateOffset[0] * 10);
			for(i = 0; i < 2; i++)
			{
				tNumOsdImgArrayInfo[(ubDateOffset[i]*2)+tIconType].uwXStart = 170;
				tNumOsdImgArrayInfo[(ubDateOffset[i]*2)+tIconType].uwYStart = 248 - (i * 30);
				tOSD_Img2(&tNumOsdImgArrayInfo[(ubDateOffset[i]*2)+tIconType], (i)?(tShowItem != ALLCALE_ITEM)?OSD_UPDATE:OSD_QUEUE:OSD_QUEUE);
			}
			if(tShowItem != ALLCALE_ITEM)
				break;
		case DATE_ITEM:
			ubDateOffset[0] = ptSysCalendar->ubDate / 10;
			ubDateOffset[1] = ptSysCalendar->ubDate - (ubDateOffset[0] * 10);
			for(i = 0; i < 2; i++)
			{
				tNumOsdImgArrayInfo[(ubDateOffset[i]*2)+tIconType].uwXStart = 170;
				tNumOsdImgArrayInfo[(ubDateOffset[i]*2)+tIconType].uwYStart = 166 - (i * 30);
				tOSD_Img2(&tNumOsdImgArrayInfo[(ubDateOffset[i]*2)+tIconType], (i)?(tShowItem != ALLCALE_ITEM)?OSD_UPDATE:OSD_QUEUE:OSD_QUEUE);
			}
			if(tShowItem != ALLCALE_ITEM)
				break;
		case HOUR_ITEM:
			ubDateOffset[0] = ptSysCalendar->ubHour / 10;
			ubDateOffset[1] = ptSysCalendar->ubHour - (ubDateOffset[0] * 10);
			for(i = 0; i < 2; i++)
			{
				tNumOsdImgArrayInfo[(ubDateOffset[i]*2)+tIconType].uwXStart = 220;
				tNumOsdImgArrayInfo[(ubDateOffset[i]*2)+tIconType].uwYStart = 390 - (i * 30);
				tOSD_Img2(&tNumOsdImgArrayInfo[(ubDateOffset[i]*2)+tIconType], (i)?(tShowItem != ALLCALE_ITEM)?OSD_UPDATE:OSD_QUEUE:OSD_QUEUE);
			}
			if(tShowItem != ALLCALE_ITEM)
				break;
		case MIN_ITEM:
			ubDateOffset[0] = ptSysCalendar->ubMin / 10;
			ubDateOffset[1] = ptSysCalendar->ubMin - (ubDateOffset[0] * 10);
			for(i = 0; i < 2; i++)
			{
				tNumOsdImgArrayInfo[(ubDateOffset[i]*2)+tIconType].uwXStart = 220;
				tNumOsdImgArrayInfo[(ubDateOffset[i]*2)+tIconType].uwYStart = 306 - (i * 30);
				tOSD_Img2(&tNumOsdImgArrayInfo[(ubDateOffset[i]*2)+tIconType], (i)?(tShowItem != ALLCALE_ITEM)?OSD_UPDATE:OSD_QUEUE:OSD_QUEUE);
			}
			if(tShowItem != ALLCALE_ITEM)
				break;
		case SEC_ITEM:
			ubDateOffset[0] = ptSysCalendar->ubSec / 10;
			ubDateOffset[1] = ptSysCalendar->ubSec - (ubDateOffset[0] * 10);
			for(i = 0; i < 2; i++)
			{
				tNumOsdImgArrayInfo[(ubDateOffset[i]*2)+tIconType].uwXStart = 220;
				tNumOsdImgArrayInfo[(ubDateOffset[i]*2)+tIconType].uwYStart = 222 - (i * 30);
				tOSD_Img2(&tNumOsdImgArrayInfo[(ubDateOffset[i]*2)+tIconType], (i)?(tShowItem != ALLCALE_ITEM)?OSD_UPDATE:OSD_QUEUE:OSD_QUEUE);
			}
			break;
		default:
			return;
	}
	if(tShowItem == ALLCALE_ITEM)
	{
		tNumOsdImgArrayInfo[21].uwXStart = 170;
		tNumOsdImgArrayInfo[21].uwYStart = 278;
		tOSD_Img2(&tNumOsdImgArrayInfo[21], OSD_QUEUE);
		
		tNumOsdImgArrayInfo[21].uwXStart = 170;
		tNumOsdImgArrayInfo[21].uwYStart = 196;
		tOSD_Img2(&tNumOsdImgArrayInfo[21], OSD_QUEUE);
		
		tNumOsdImgArrayInfo[20].uwXStart = 220;
		tNumOsdImgArrayInfo[20].uwYStart = 336;
		tOSD_Img2(&tNumOsdImgArrayInfo[20], OSD_QUEUE);
		
		tNumOsdImgArrayInfo[20].uwXStart = 220;
		tNumOsdImgArrayInfo[20].uwYStart = 252;
		tOSD_Img2(&tNumOsdImgArrayInfo[20], OSD_UPDATE);
	}
}
//------------------------------------------------------------------------------
int UI_DrawSettingSubSubMenuPage(UI_SettingSubMenuItemList_t *tSubMenuItem)
{
	OSD_IMG_INFO tOsdImgInfo;
	int iRet = -1;
	uint8_t i;

	tOsdImgInfo.uwHSize  = 335;
	tOsdImgInfo.uwVSize  = 250;
	tOsdImgInfo.uwYStart = 312;
	switch(*tSubMenuItem)
	{
		case DATETIME_ITEM:
		{
			tOsdImgInfo.uwXStart = 150;
			OSD_EraserImg2(&tOsdImgInfo);
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DT_SUBMENUICON, 1, &tOsdImgInfo);
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
		#if (BSP_RTC_TIMER_SEL == RTC_TIMER_EXTERNAL)
			RTC_PCF85063A_Calendar_t tCalendar;

			RTC_PCF85063A_GetCalendar((RTC_PCF85063A_Calendar_t *)(&tCalendar));
			tUI_CuSetting.tSysCalendar.uwYear 	= tCalendar.uwYear;
			tUI_CuSetting.tSysCalendar.ubMonth 	= tCalendar.ubMonth;
			tUI_CuSetting.tSysCalendar.ubDate	= tCalendar.ubDate;
			tUI_CuSetting.tSysCalendar.ubHour	= tCalendar.ubHour;
			tUI_CuSetting.tSysCalendar.ubMin	= tCalendar.ubMin;
			tUI_CuSetting.tSysCalendar.ubSec	= tCalendar.ubSec;
		#endif
			UI_DrawSysDateTime(ALLCALE_ITEM, UI_ICON_NORMAL, (RTC_Calendar_t *)(&tUI_CuSetting.tSysCalendar));
			break;
		}
		case AECSET_ITEM:
		{
			uint16_t uwSubSubMenuItemOsdImg[AECFUNC_MAX] = {OSD2IMG_AECOFFNOR_ICON, OSD2IMG_AECONNOR_ICON};
			uwSubSubMenuItemOsdImg[tUI_CuSetting.ubAEC_Mode] += UI_ICON_HIGHLIGHT;
			tOsdImgInfo.uwXStart = 215;
			OSD_EraserImg2(&tOsdImgInfo);
			for(i = 0; i < AECFUNC_MAX; i++)
			{
				tOSD_GetOsdImgInfor(1, OSD_IMG2, uwSubSubMenuItemOsdImg[i], 1, &tOsdImgInfo);
				tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
			}
			iRet = 0;
			break;
			}
		case CCASET_ITEM:
		{
			uint16_t uwSubSubMenuItemOsdImg[CCAMODE_MAX] = {OSD2IMG_AECOFFNOR_ICON, OSD2IMG_AECONNOR_ICON};
			uwSubSubMenuItemOsdImg[tUI_CuSetting.ubCCA_Mode] += UI_ICON_HIGHLIGHT;
			tOsdImgInfo.uwXStart = 275;
			OSD_EraserImg2(&tOsdImgInfo);
			for(i = 0; i < CCAMODE_MAX; i++)
			{
				tOSD_GetOsdImgInfor(1, OSD_IMG2, uwSubSubMenuItemOsdImg[i], 1, &tOsdImgInfo);
				tOsdImgInfo.uwXStart += 65;
				tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
			}
			iRet = 0x40;
			break;
		}
		case DEFUSET_ITEM:
		{
			uint16_t uwSubSubMenuItemOsdImg[2] = {OSD2IMG_DEFUNOTHL_ICON, OSD2IMG_DEFUEXECNOR_ICON};

			for(i = 0; i < 2; i++)
			{
				tOSD_GetOsdImgInfor(1, OSD_IMG2, uwSubSubMenuItemOsdImg[i], 1, &tOsdImgInfo);
				tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
			}
			iRet = 0x100;
			break;
		}
		case SWUSBDMODE_ITEM:
		{
			uint16_t uwSubSubMenuItemOsdImg[2] = {OSD2IMG_FWUDISABLEHL_ICON, OSD2IMG_FWUENNOR_ICON};

			for(i = 0; i < 2; i++)
			{
				tOSD_GetOsdImgInfor(1, OSD_IMG2, uwSubSubMenuItemOsdImg[i], 1, &tOsdImgInfo);
				tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
			}
			iRet = 0x140;
			break;
		}
		default:
			break;
	}
	return iRet;
}
//------------------------------------------------------------------------------
void UI_SettingSubMenuPage(UI_ArrowKey_t tArrowKey)
{
	UI_MenuAct_t tMenuAct;

	if(tUI_State == UI_MAINMENU_STATE)
	{
		//! Draw Cameras sub menu page
		UI_DrawSubMenuPage(SETTING_ITEM);
		return;
	}
	tMenuAct = UI_KeyEventMap2SubMenuInfo(&tArrowKey, &tUI_SubMenuItem[SETTING_ITEM]);
	switch(tMenuAct)
	{
		case DRAW_HIGHLIGHT_MENUICON:
		{
			uint16_t uwSubMenuItemOsdImg[SETTINGITEM_MAX] = {OSD2IMG_DTNOR_ITEM, OSD2IMG_AECNOR_ITEM, OSD2IMG_CCANOR_ITEM,
															 OSD2IMG_STORAGENOR_ITEM, OSD2IMG_LANGUAGENOR_ITEM, OSD2IMG_DEFUNOR_ITEM, OSD2IMG_SWUSBDMODENOR_ITEM};
			uint8_t ubSubMenuItemPreIdx = tUI_SubMenuItem[SETTING_ITEM].tSubMenuInfo.ubItemPreIdx;
			uint8_t ubSubMenuItemIdx = tUI_SubMenuItem[SETTING_ITEM].tSubMenuInfo.ubItemIdx;
			if(ubSubMenuItemIdx == STORAGESTS_ITEM)
			{
				ubSubMenuItemIdx = (ubSubMenuItemPreIdx > ubSubMenuItemIdx)?--tUI_SubMenuItem[SETTING_ITEM].tSubMenuInfo.ubItemIdx:
																		    ++tUI_SubMenuItem[SETTING_ITEM].tSubMenuInfo.ubItemIdx;
				tUI_SubMenuItem[SETTING_ITEM].tSubMenuInfo.ubItemPreIdx = (ubSubMenuItemPreIdx > ubSubMenuItemIdx)?--tUI_SubMenuItem[SETTING_ITEM].tSubMenuInfo.ubItemPreIdx:
																												   ++tUI_SubMenuItem[SETTING_ITEM].tSubMenuInfo.ubItemPreIdx;
			}
			UI_DrawHLandNormalIcon(uwSubMenuItemOsdImg[ubSubMenuItemPreIdx], (uwSubMenuItemOsdImg[ubSubMenuItemIdx] + UI_ICON_HIGHLIGHT));
			break;
		}
		case DRAW_MENUPAGE:
		{
			//! Draw sub sub menu page
			UI_SettingSubMenuItemList_t tSubMenuItemIdx = (UI_SettingSubMenuItemList_t)tUI_SubMenuItem[SETTING_ITEM].tSubMenuInfo.ubItemIdx;
			OSD_IMG_INFO tOsdImgInfo;
			int iOptMarkOffset = UI_DrawSettingSubSubMenuPage(&tSubMenuItemIdx);

			if(iOptMarkOffset != -1)
			{
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_OPTMARKHL_ICON, 1, &tOsdImgInfo);
				tOsdImgInfo.uwXStart = (0xDA + iOptMarkOffset);
				tOsdImgInfo.uwYStart = 0x205;
				tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			}
			tUI_State = UI_SUBSUBMENU_STATE;
			break;
		}
		case EXIT_MENUFUNC:
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_SettingUpdateSubSubMenuItemIndex(UI_SettingSubSubMenuItem_t *ptSubSubMenuItem, UI_SettingSubMenuItemList_t *tSubMenuItem, uint8_t *Update_Flag)
{
	switch(*tSubMenuItem)
	{
		case AECSET_ITEM:
			ptSubSubMenuItem->tSettingS[*tSubMenuItem].tSubMenuInfo.ubItemIdx = tUI_CuSetting.ubAEC_Mode;
			break;
		case CCASET_ITEM:
			ptSubSubMenuItem->tSettingS[*tSubMenuItem].tSubMenuInfo.ubItemIdx = tUI_CuSetting.ubCCA_Mode;
			break;
		default:
			return;
	}
	*Update_Flag = TRUE;
}
//------------------------------------------------------------------------------
void UI_SettingDrawSubSubMenuItem(UI_SettingSubSubMenuItem_t *ptSubSubMenuItem, UI_SettingSubMenuItemList_t *tSubMenuItem)
{
	uint8_t ubSubSubMenuItemPreIdx = ptSubSubMenuItem->tSettingS[*tSubMenuItem].tSubMenuInfo.ubItemPreIdx;
	uint8_t ubSubSubMenuItemIdx = ptSubSubMenuItem->tSettingS[*tSubMenuItem].tSubMenuInfo.ubItemIdx;
	switch(*tSubMenuItem)
	{
		case AECSET_ITEM:
		{
			uint16_t uwSubSubMenuItemOsdImg[AECFUNC_MAX] = {OSD2IMG_AECOFFNOR_ICON, OSD2IMG_AECONNOR_ICON};
			UI_DrawHLandNormalIcon(uwSubSubMenuItemOsdImg[ubSubSubMenuItemPreIdx], (uwSubSubMenuItemOsdImg[ubSubSubMenuItemIdx]+UI_ICON_HIGHLIGHT));
			break;
		}
		case CCASET_ITEM:
		{
			OSD_IMG_INFO tOsdImgInfo;
			uint16_t uwSubSubMenuItemOsdImg[AECFUNC_MAX] = {OSD2IMG_AECOFFNOR_ICON, OSD2IMG_AECONNOR_ICON};

			tOSD_GetOsdImgInfor(1, OSD_IMG2, uwSubSubMenuItemOsdImg[ubSubSubMenuItemPreIdx], 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart += 65;
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
			tOSD_GetOsdImgInfor(1, OSD_IMG2, (uwSubSubMenuItemOsdImg[ubSubSubMenuItemIdx]+UI_ICON_HIGHLIGHT), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart += 65;
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			break;
		}
		case DEFUSET_ITEM:
		{
			uint16_t uwSubSubMenuItemOsdImg[2] = {OSD2IMG_DEFUNOTNOR_ICON, OSD2IMG_DEFUEXECNOR_ICON};

			UI_DrawHLandNormalIcon(uwSubSubMenuItemOsdImg[ubSubSubMenuItemPreIdx], (uwSubSubMenuItemOsdImg[ubSubSubMenuItemIdx]+UI_ICON_HIGHLIGHT));
			break;
		}
		case SWUSBDMODE_ITEM:
		{
			uint16_t uwSubSubMenuItemOsdImg[2] = {OSD2IMG_FWUDISABLENOR_ICON, OSD2IMG_FWUENNOR_ICON};

			UI_DrawHLandNormalIcon(uwSubSubMenuItemOsdImg[ubSubSubMenuItemPreIdx], (uwSubSubMenuItemOsdImg[ubSubSubMenuItemIdx]+UI_ICON_HIGHLIGHT));
			break;
		}
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_SettingSubSubMenuPage(UI_ArrowKey_t tArrowKey)
{
	static UI_SettingSubSubMenuItem_t tSettingSubSubMenuItem = 
	{
		{
		   { 0, 0 			},
		   { 0, AECFUNC_MAX },
		   { 0, CCAMODE_MAX },
		   { 0, 0 			},
		   { 0, 0 			},
		   { 0, 2 			},
		   { 0, 2 			},
		},
	};
	static uint8_t ubUI_SettingStsUpdateFlag = FALSE;
	UI_SettingSubMenuItemList_t tSubMenuItem = (UI_SettingSubMenuItemList_t)tUI_SubMenuItem[SETTING_ITEM].tSubMenuInfo.ubItemIdx;
	UI_MenuAct_t tMenuAct;
	OSD_IMG_INFO tOsdImgInfo;

	if(FALSE == ubUI_SettingStsUpdateFlag)
		UI_SettingUpdateSubSubMenuItemIndex(&tSettingSubSubMenuItem, &tSubMenuItem, &ubUI_SettingStsUpdateFlag);
	tMenuAct = UI_KeyEventMap2SubSubMenuInfo(&tArrowKey, &tSettingSubSubMenuItem.tSettingS[tSubMenuItem]);
	switch(tMenuAct)
	{
		case DRAW_HIGHLIGHT_MENUICON:
			UI_SettingDrawSubSubMenuItem(&tSettingSubSubMenuItem, &tSubMenuItem);
			break;
		case EXECUTE_MENUFUNC:
			if(tSubMenuItem == SWUSBDMODE_ITEM)
			{
				if(tSettingSubSubMenuItem.tSettingS[SWUSBDMODE_ITEM].tSubMenuInfo.ubItemIdx)
					FWU_Enable();
				else
					FWU_Disable();
				tSettingSubSubMenuItem.tSettingS[SWUSBDMODE_ITEM].tSubMenuInfo.ubItemIdx = 0;
			}
		case EXIT_MENUFUNC:
			tUI_State = UI_SUBMENU_STATE;
			if((tSubMenuItem == DEFUSET_ITEM) || (tSubMenuItem == SWUSBDMODE_ITEM))
			{
				tOsdImgInfo.uwHSize  = 200;
				tOsdImgInfo.uwVSize  = 250;
				tOsdImgInfo.uwXStart = 450 + ((tSubMenuItem == SWUSBDMODE_ITEM)?60:0);
				tOsdImgInfo.uwYStart = 312;
				OSD_EraserImg2(&tOsdImgInfo);
				break;
			}
			
			if(tSubMenuItem == AECSET_ITEM)
            {
				
				tUI_CuSetting.ubAEC_Mode = tSettingSubSubMenuItem.tSettingS[AECSET_ITEM].tSubMenuInfo.ubItemIdx;	
				
				if(tUI_CuSetting.ubAEC_Mode == 0)
				{
					ADO_Noise_Process_Type(NOISE_DISABLE,AEC_NR_16kHZ);
				}
				else
				{
					ADO_Noise_Process_Type(NOISE_AEC,AEC_NR_16kHZ);
				}
                                //AEC
				
            }
		
			tOsdImgInfo.uwHSize  = 350;
			tOsdImgInfo.uwVSize  = 480;
			tOsdImgInfo.uwXStart = 100;
			tOsdImgInfo.uwYStart = 100;
			OSD_EraserImg2(&tOsdImgInfo);
			//! Change camera setting
			tOSD_GetOsdImgInfor(1, OSD_IMG2, (OSD2IMG_AECOFFWR_ICON+tUI_CuSetting.ubAEC_Mode), 1, &tOsdImgInfo);
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
			tOSD_GetOsdImgInfor(1, OSD_IMG2, (OSD2IMG_AECOFFWR_ICON+tUI_CuSetting.ubCCA_Mode), 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart += 65;
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_STORAGATBU_ICON+tUI_CuSetting.ubSTORAGE_Mode, 1, &tOsdImgInfo);
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_OPTMARKNOR_ICON, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart -= 0xE;
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
			tOsdImgInfo.uwXStart += (0xE + 0x32);
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			ubUI_SettingStsUpdateFlag = FALSE;
			break;
		default:
			if((tSubMenuItem == DATETIME_ITEM) && (tArrowKey == RIGHT_ARROW))
			{
				UI_DrawSysDateTime(YEAR_ITEM, UI_ICON_HIGHLIGHT, (RTC_Calendar_t *)(&tUI_CuSetting.tSysCalendar));
				tUI_State = UI_SUBSUBSUBMENU_STATE;
			}
			break;
	}
}
//------------------------------------------------------------------------------
void UI_SettingSysDateTimeSubSubMenuPage(UI_ArrowKey_t tArrowKey)
{
	static UI_SettingSubSubSubItem_t tSysDtSubSubSubItem = {YEAR_ITEM, YEAR_ITEM};
	static RTC_Calendar_t tUI_SetSysCalendar;
	static uint8_t ubUI_SetSysDtFlag = FALSE;
	uint16_t *pDT_YearNum;
	uint8_t *pDT_Num[ALLCALE_ITEM];
	uint16_t uwDT_MaxYearNum = 2098, uwDT_MinYearNum = 2015;
	uint8_t ubDT_MaxNum[ALLCALE_ITEM] = {0, 12, 31, 23, 59, 59};
	uint8_t ubDT_MinNum[ALLCALE_ITEM] = {0,  1,  1,  0,  0,  0};
	uint8_t ubDT_UpdateItem = tSysDtSubSubSubItem.ubItemIdx;

	if(FALSE == ubUI_SetSysDtFlag)
	{
		memcpy((RTC_Calendar_t *)&tUI_SetSysCalendar, (RTC_Calendar_t *)&tUI_CuSetting.tSysCalendar, sizeof(RTC_Calendar_t));
		ubUI_SetSysDtFlag	= TRUE;
	}
	pDT_YearNum 		= (uint16_t *)&tUI_SetSysCalendar.uwYear;
	pDT_Num[MONTH_ITEM] = (uint8_t *)&tUI_SetSysCalendar.ubMonth;
	pDT_Num[DATE_ITEM] 	= (uint8_t *)&tUI_SetSysCalendar.ubDate;
	pDT_Num[HOUR_ITEM] 	= (uint8_t *)&tUI_SetSysCalendar.ubHour;
	pDT_Num[MIN_ITEM] 	= (uint8_t *)&tUI_SetSysCalendar.ubMin;
	pDT_Num[SEC_ITEM] 	= (uint8_t *)&tUI_SetSysCalendar.ubSec;
	switch(tArrowKey)
	{
		case UP_ARROW:
			if(ubDT_UpdateItem == YEAR_ITEM)
			{
				if((NULL == pDT_YearNum) || (*pDT_YearNum >= uwDT_MaxYearNum))
					return;
				(*pDT_YearNum)++;
			}
			else
			{
				if((NULL == pDT_Num[ubDT_UpdateItem]) || (*pDT_Num[ubDT_UpdateItem] >= ubDT_MaxNum[ubDT_UpdateItem]))
					return;
				(*pDT_Num[ubDT_UpdateItem])++;
			}
			UI_DrawSysDateTime((UI_CalendarItem_t)ubDT_UpdateItem, UI_ICON_HIGHLIGHT, (RTC_Calendar_t *)(&tUI_SetSysCalendar));
			return;
		case DOWN_ARROW:
			if(ubDT_UpdateItem == YEAR_ITEM)
			{
				if((NULL == pDT_YearNum) || (*pDT_YearNum == uwDT_MinYearNum))
					return;
				(*pDT_YearNum)--;
			}
			else
			{
				if((NULL == pDT_Num[ubDT_UpdateItem]) || (*pDT_Num[ubDT_UpdateItem] == ubDT_MinNum[ubDT_UpdateItem]))
					return;
				(*pDT_Num[ubDT_UpdateItem])--;
			}
			UI_DrawSysDateTime((UI_CalendarItem_t)ubDT_UpdateItem, UI_ICON_HIGHLIGHT, (RTC_Calendar_t *)(&tUI_SetSysCalendar));
			return;
		case ENTER_ARROW:
		case LEFT_ARROW:
			if((tSysDtSubSubSubItem.ubItemIdx == YEAR_ITEM) || (ENTER_ARROW == tArrowKey))
			{
				if(memcmp((RTC_Calendar_t *)&tUI_SetSysCalendar, (RTC_Calendar_t *)&tUI_CuSetting.tSysCalendar, sizeof(RTC_Calendar_t)))
				{
					memcpy((RTC_Calendar_t *)&tUI_CuSetting.tSysCalendar, (RTC_Calendar_t *)&tUI_SetSysCalendar, sizeof(RTC_Calendar_t));
				#if (BSP_RTC_TIMER_SEL == RTC_TIMER_INTERNAL)
					if(iRTC_SetBaseCalendar((RTC_Calendar_t *)(&tUI_CuSetting.tSysCalendar)) != RTC_OK)
						printd(DBG_ErrorLvl, "Calendar base setting fail !\n");
					else
						RTC_SetCalendar((RTC_Calendar_t *)(&tUI_CuSetting.tSysCalendar));
				#elif (BSP_RTC_TIMER_SEL == RTC_TIMER_EXTERNAL)
					RTC_PCF85063A_Calendar_t tCalendar;

					tCalendar.uwYear  = tUI_CuSetting.tSysCalendar.uwYear;
					tCalendar.ubMonth = tUI_CuSetting.tSysCalendar.ubMonth;
					tCalendar.ubDate  = tUI_CuSetting.tSysCalendar.ubDate;
					tCalendar.ubHour  = tUI_CuSetting.tSysCalendar.ubHour;
					tCalendar.ubMin   = tUI_CuSetting.tSysCalendar.ubMin;
					tCalendar.ubSec   = tUI_CuSetting.tSysCalendar.ubSec;
					RTC_PCF85063A_SetCalendar((RTC_PCF85063A_Calendar_t *)(&tCalendar));
				#endif
					UI_UpdateDevStatusInfo();
					UI_DrawSysDateTime((UI_CalendarItem_t)tSysDtSubSubSubItem.ubItemIdx, UI_ICON_NORMAL, (RTC_Calendar_t *)(&tUI_CuSetting.tSysCalendar));
				}
#if APP_TIMESTAMP_FUNC_ENABLE
                KNL_SyncTimeStamp2TX();
#endif                
				tSysDtSubSubSubItem.ubItemPreIdx = YEAR_ITEM;
				tSysDtSubSubSubItem.ubItemIdx    = YEAR_ITEM;
				ubUI_SetSysDtFlag = FALSE;
				tUI_State = UI_SUBSUBMENU_STATE;
				return;
			}
			tSysDtSubSubSubItem.ubItemPreIdx = tSysDtSubSubSubItem.ubItemIdx;
			tSysDtSubSubSubItem.ubItemIdx--;
			break;
		case RIGHT_ARROW:
			if(tSysDtSubSubSubItem.ubItemIdx == SEC_ITEM)
				return;
			tSysDtSubSubSubItem.ubItemPreIdx = tSysDtSubSubSubItem.ubItemIdx;
			tSysDtSubSubSubItem.ubItemIdx++;
			break;
		default:
			return;
	}
	UI_DrawSysDateTime((UI_CalendarItem_t)tSysDtSubSubSubItem.ubItemIdx,    UI_ICON_HIGHLIGHT, (RTC_Calendar_t *)(&tUI_SetSysCalendar));
	UI_DrawSysDateTime((UI_CalendarItem_t)tSysDtSubSubSubItem.ubItemPreIdx, UI_ICON_NORMAL,    (RTC_Calendar_t *)(&tUI_SetSysCalendar));
}
//------------------------------------------------------------------------------
void UI_ResetSubMenuInfo(void)
{
	uint8_t i;

	for(i = 0; i < MENUITEM_MAX; i++)
		memset(&tUI_SubMenuItem[i].tSubMenuInfo, 0, sizeof(UI_MenuItem_t));
}
//------------------------------------------------------------------------------
void UI_ResetSubSubMenuInfo(void)
{
}
//------------------------------------------------------------------------------
UI_CamNum_t UI_ChangeSelectCamNum4UiMenu(UI_CamNum_t *tCurrentCamNum, UI_ArrowKey_t *ptArrowKey)
{
	UI_CamNum_t tChangeCamNum = NO_CAM;
	UI_CamNum_t tCamNum 	  = (UI_CamNum_t)*tCurrentCamNum;
	UI_CamNum_t tMaxCamNum 	  = (DISPLAY_4T1R == tUI_CuSetting.ubTotalCamNum)?CAM_4T:
	                            (DISPLAY_2T1R == tUI_CuSetting.ubTotalCamNum)?CAM_2T:CAM_4T;

	if(tCamNum > ((DISPLAY_4T1R == tUI_CuSetting.ubTotalCamNum)?((VDO_DISP_TYPE == KNL_DISP_H)?H_TYPE_ITEM:QUAD_TYPE_ITEM):tMaxCamNum))
		return NO_CAM;
	switch(*ptArrowKey)
	{
		case LEFT_ARROW:
			if(tCamNum == CAM1)
				return NO_CAM;
			if(DISPLAY_4T1R == tUI_CuSetting.ubTotalCamNum)
			{
				if((tUI_CuSetting.ubPairedCamNum > 1) && (tCamNum > DUAL_TYPE_ITEM))
					return (UI_CamNum_t)(tCamNum-1);
				else if((tUI_CuSetting.ubPairedCamNum == 1) && (tCamNum > DUAL_TYPE_ITEM))
					return (UI_CamNum_t)(tCamNum-2);
				if(tCamNum == DUAL_TYPE_ITEM)
					tCamNum = CAM_4T;
			}
			for(;tCamNum > CAM1; tCamNum--)
			{
			#ifdef RTC676x
				if(tUI_CamStatus[tCamNum-1].ulCAM_ID != INVALID_ID)
			#else
				if((tUI_CamStatus[tCamNum-1].ulCAM_ID != INVALID_ID) &&
				   (tUI_CamStatus[tCamNum-1].tCamConnSts == CAM_ONLINE))
			#endif
				{
					tChangeCamNum = (UI_CamNum_t)(tCamNum-1);
					break;
				}			
			}
			break;
		case RIGHT_ARROW:
			if((tCamNum+1) >= tMaxCamNum)
			{
				if(DISPLAY_4T1R == tUI_CuSetting.ubTotalCamNum)
					return ((tCamNum+1) > ((VDO_DISP_TYPE == KNL_DISP_H)?H_TYPE_ITEM:QUAD_TYPE_ITEM))?NO_CAM:(tUI_CuSetting.ubPairedCamNum > 1)?((UI_CamNum_t)(tCamNum+1)):(UI_CamNum_t)((VDO_DISP_TYPE == KNL_DISP_H)?H_TYPE_ITEM:QUAD_TYPE_ITEM);
				else
					return tMaxCamNum;
			}
			tChangeCamNum = tMaxCamNum;
			for(;tCamNum < (tMaxCamNum - 1); tCamNum++)
			{
			#ifdef RTC676x
				if(tUI_CamStatus[tCamNum+1].ulCAM_ID != INVALID_ID)
			#else
				if((tUI_CamStatus[tCamNum+1].ulCAM_ID != INVALID_ID) &&
				   (tUI_CamStatus[tCamNum+1].tCamConnSts == CAM_ONLINE))
			#endif
				{
					tChangeCamNum = (UI_CamNum_t)(tCamNum+1);
					break;
				}
			}
			if((DISPLAY_4T1R == tUI_CuSetting.ubTotalCamNum) && (tChangeCamNum == tMaxCamNum))
				tChangeCamNum = (tUI_CuSetting.ubPairedCamNum >= 1)?(UI_CamNum_t)DUAL_TYPE_ITEM:(UI_CamNum_t)((VDO_DISP_TYPE == KNL_DISP_H)?H_TYPE_ITEM:QUAD_TYPE_ITEM);
			break;
		default:
			break;
	}
	return tChangeCamNum;
}
//------------------------------------------------------------------------------
void UI_DrawHLandNormalIcon(uint16_t uwNormalOsdImgIdx, uint16_t uwHighLigthOsdImgIdx)
{
	OSD_IMG_INFO tOsdImgInfo;

	//! Draw normal item
	tOSD_GetOsdImgInfor(1, OSD_IMG2, uwNormalOsdImgIdx, 1, &tOsdImgInfo);
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	//! Draw highlight item
	tOSD_GetOsdImgInfor(1, OSD_IMG2, uwHighLigthOsdImgIdx, 1, &tOsdImgInfo);
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}
//------------------------------------------------------------------------------
UI_MenuAct_t UI_KeyEventMap2SubMenuInfo(UI_ArrowKey_t *ptArrowKey, UI_SubMenuItem_t *ptSubMenu)
{
	uint8_t ubCurrentSubMenuItemIdx = ptSubMenu->tSubMenuInfo.ubItemIdx;
	uint8_t ubNextSubMenuItemIdx = 0;

	switch(*ptArrowKey)
	{
		case UP_ARROW:
			if(ubCurrentSubMenuItemIdx <= ptSubMenu->ubFirstItem)
				return NOT_ACTION;
			ubNextSubMenuItemIdx = ubCurrentSubMenuItemIdx - 1;
			break;
		case DOWN_ARROW:
			if((ubCurrentSubMenuItemIdx+1) >= ptSubMenu->ubItemCount)
				return NOT_ACTION;
			ubNextSubMenuItemIdx = ubCurrentSubMenuItemIdx + 1;
			break;
		case RIGHT_ARROW:
		case ENTER_ARROW:
			return DRAW_MENUPAGE;
		case EXIT_ARROW:
			return EXIT_MENUFUNC;
		default:
			return NOT_ACTION;
	}
	ptSubMenu->tSubMenuInfo.ubItemPreIdx = ubCurrentSubMenuItemIdx;
	ptSubMenu->tSubMenuInfo.ubItemIdx    = ubNextSubMenuItemIdx;
	return DRAW_HIGHLIGHT_MENUICON;
}
//------------------------------------------------------------------------------
UI_MenuAct_t UI_KeyEventMap2SubSubMenuInfo(UI_ArrowKey_t *ptArrowKey, UI_SubMenuItem_t *ptSubSubMenuItem)
{
	uint8_t ubCurrentSubSubMenuItemIdx = 0;
	uint8_t ubNextSubSubMenuItemIdx = 0;

	ubCurrentSubSubMenuItemIdx = ptSubSubMenuItem->tSubMenuInfo.ubItemIdx;
	switch(*ptArrowKey)
	{
		case UP_ARROW:
			if(ubCurrentSubSubMenuItemIdx <= ptSubSubMenuItem->ubFirstItem)
				return NOT_ACTION;
			ubNextSubSubMenuItemIdx = ubCurrentSubSubMenuItemIdx - 1;
			break;
		case DOWN_ARROW:
			if((ubCurrentSubSubMenuItemIdx+1) >= ptSubSubMenuItem->ubItemCount)
				return NOT_ACTION;
			ubNextSubSubMenuItemIdx = ubCurrentSubSubMenuItemIdx + 1;
			break;
		case ENTER_ARROW:
			return EXECUTE_MENUFUNC;
		case LEFT_ARROW:
			return EXIT_MENUFUNC;
		default:
			return NOT_ACTION;
	}
	ptSubSubMenuItem->tSubMenuInfo.ubItemPreIdx = ubCurrentSubSubMenuItemIdx;
	ptSubSubMenuItem->tSubMenuInfo.ubItemIdx    = ubNextSubSubMenuItemIdx;
	return DRAW_HIGHLIGHT_MENUICON;
}
//------------------------------------------------------------------------------
void UI_UpdateBriLvlIcon(void)
{
	if((UI_SHOWSTSICON_STATE != tUI_State) ||
	   (!tUI_CuSetting.BriLvL.ubBL_UpdateCnt))
		return;
	if(!(--tUI_CuSetting.BriLvL.ubBL_UpdateCnt))
	{
		OSD_IMG_INFO tOsdImgInfo;

		UI_ClearCamConnectStatusFlag();
		tOsdImgInfo.uwXStart = 100;
		tOsdImgInfo.uwYStart = 0;
		tOsdImgInfo.uwHSize  = 190;
		tOsdImgInfo.uwVSize  = 300;
		OSD_EraserImg2(&tOsdImgInfo);
		tUI_State = UI_DISPLAY_STATE;
	}
}
//------------------------------------------------------------------------------
void UI_UpdateVolLvlIcon(void)
{
	if((UI_SHOWSTSICON_STATE != tUI_State) ||
	   (!tUI_CuSetting.VolLvL.ubVOL_UpdateCnt))
		return;
	if(!(--tUI_CuSetting.VolLvL.ubVOL_UpdateCnt))
	{
		OSD_IMG_INFO tOsdImgInfo;

		UI_ClearCamConnectStatusFlag();
		tOsdImgInfo.uwXStart = 100;
		tOsdImgInfo.uwYStart = 0;
		tOsdImgInfo.uwHSize  = 190;
		tOsdImgInfo.uwVSize  = 300;
		OSD_EraserImg2(&tOsdImgInfo);
		tUI_State = UI_DISPLAY_STATE;
	}
}
//------------------------------------------------------------------------------
void UI_UpdateOsdImg4MultiView(UI_CamViewType_t tView_Type, OSD_RESULT(*pOsdImgFuncPtr)(OSD_IMG_INFO *, OSD_UPDATE_TYP), OSD_IMG_INFO *pOsdImgInfo)
{
	uint32_t ulLcd_HSize = uwLCD_GetLcdHoSize();
	uint32_t ulLcd_VSize = uwLCD_GetLcdVoSize();
	uint16_t uwOriXStart = pOsdImgInfo->uwXStart, uwOriYStart = pOsdImgInfo->uwYStart;

	if(H_VIEW == tView_Type)
		return;
	pOsdImgInfo->uwYStart -= (ulLcd_VSize/2);
	if(QUAD_VIEW == tView_Type)
	{
		pOsdImgFuncPtr(pOsdImgInfo, OSD_QUEUE);
		pOsdImgInfo->uwXStart += (ulLcd_HSize/2);
		pOsdImgFuncPtr(pOsdImgInfo, OSD_QUEUE);
		pOsdImgInfo->uwYStart = uwOriYStart;
	}
	pOsdImgFuncPtr(pOsdImgInfo, OSD_QUEUE);
	pOsdImgInfo->uwXStart = uwOriXStart;
	pOsdImgInfo->uwYStart = uwOriYStart;
	pOsdImgFuncPtr(pOsdImgInfo, OSD_UPDATE);
}
//------------------------------------------------------------------------------
void UI_DrawCUStatusIcon(void)
{
	OSD_IMG_INFO tCuOsdImgInfo;
	uint32_t ulViewMask_YStart = uwLCD_GetLcdVoSize()/2;
	UI_CamViewType_t tUI_CamViewType = tCamViewSel.tCamViewType;

	if(UI_SET_CAMECOMODE_STATE == tUI_State)
		return;
	switch(tUI_CamViewType)
	{
		case SINGLE_VIEW:
		case SCAN_VIEW:
			if(TRUE == tUI_CuSetting.IconSts.ubDrawStsIconFlag)
				break;
			tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_HDSTATUSMASK, 1, &tCuOsdImgInfo);
			tOSD_Img1(&tCuOsdImgInfo, OSD_QUEUE);
			OSD_Weight(OSD_WEIGHT_6DIV8);
			tUI_CuSetting.IconSts.ubDrawStsIconFlag = TRUE;
			break;
		case DUAL_VIEW:
		case QUAD_VIEW:
			if(TRUE == tUI_CuSetting.IconSts.ubDrawStsIconFlag)
				break;
			tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_QUALSTATUSMASK, 1, &tCuOsdImgInfo);
			UI_UpdateOsdImg4MultiView(tCamViewSel.tCamViewType, tOSD_Img1, &tCuOsdImgInfo);
			OSD_Weight(OSD_WEIGHT_6DIV8);
			tUI_CuSetting.IconSts.ubDrawStsIconFlag = TRUE;
			break;
		case H_VIEW:
			if(TRUE == tUI_CuSetting.IconSts.ubDrawStsIconFlag)
				break;
			tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_HVIEWSTATUSMASK, 1, &tCuOsdImgInfo);
			tOSD_Img1(&tCuOsdImgInfo, OSD_QUEUE);
			tCuOsdImgInfo.uwYStart = 0;
			tOSD_Img1(&tCuOsdImgInfo, OSD_QUEUE);
			tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_QUALSTATUSMASK, 1, &tCuOsdImgInfo);
			tCuOsdImgInfo.uwYStart -= (ulViewMask_YStart / 2);
			tOSD_Img1(&tCuOsdImgInfo, OSD_QUEUE);
			tCuOsdImgInfo.uwXStart += uwLCD_GetLcdHoSize()/2;
			tOSD_Img1(&tCuOsdImgInfo, OSD_UPDATE);
			OSD_Weight(OSD_WEIGHT_6DIV8);
			tUI_CuSetting.IconSts.ubDrawStsIconFlag = TRUE;
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_UpdateCamStatusOsdImg(OSD_IMG_INFO *pOsdImgInfo, OSD_UPDATE_TYP tUpdateMode, UI_OsdImgFnType_t tOsdImgFnType, UI_DisplayLocation_t tDispLoc)
{
	if( DISP_LOCATION_TYPE_MAX <= tDispLoc )
	{
		printd(DBG_ErrorLvl, "Display Location Error!\n");
		return;
	}
	uint32_t ulLcd_HSize  = uwLCD_GetLcdHoSize();
	uint32_t ulLcd_VSize  = uwLCD_GetLcdVoSize();
	uint16_t uwXOffset[11] = {[DISP_UPPER_LEFT] = 0,               [DISP_UPPER_RIGHT] = 0,
	                         [DISP_LOWER_LEFT]  = (ulLcd_HSize/2), [DISP_LOWER_RIGHT] = (ulLcd_HSize/2),
							 [DISP_LEFT] 	    = 0,			   [DISP_RIGHT] 	  = 0,
							 [DISP_H_L] 	    = 0,  			   [DISP_H_R] 		  = 0,
							 [DISP_H_CU] 	    = 0, 		       [DISP_H_CL] 		  = (ulLcd_HSize/2)};
	uint16_t uwYOffset[11] = {[DISP_UPPER_LEFT] = 0,               [DISP_UPPER_RIGHT] = (ulLcd_VSize/2),
	                         [DISP_LOWER_LEFT]  = 0, 			   [DISP_LOWER_RIGHT] = (ulLcd_VSize/2),
							 [DISP_LEFT] 	    = 0, 		       [DISP_RIGHT] 	  = (ulLcd_VSize/2),
							 [DISP_H_L] 	    = 0, 			   [DISP_H_R] 		  = 960,
							 [DISP_H_CU] 	    = 360, 		       [DISP_H_CL] 		  = 360};
	uint16_t uwXStart = 0, uwYStart = 0;

	uwXStart = pOsdImgInfo->uwXStart;
	uwYStart = pOsdImgInfo->uwYStart;
	pOsdImgInfo->uwXStart += uwXOffset[tDispLoc];
	pOsdImgInfo->uwYStart -= uwYOffset[tDispLoc];
	if(UI_OsdUpdate == tOsdImgFnType)
		tOSD_Img2(pOsdImgInfo, tUpdateMode);
	else if(UI_OsdErase == tOsdImgFnType)
		OSD_EraserImg2(pOsdImgInfo);
	pOsdImgInfo->uwXStart = uwXStart;
	pOsdImgInfo->uwYStart = uwYStart;
}
//------------------------------------------------------------------------------
void UI_ClearCamConnectStatusFlag(void)
{
	UI_CamNum_t tCamNum;

	for(tCamNum = CAM1; tCamNum < tUI_CuSetting.ubTotalCamNum; tCamNum++)
	{
		if(pUI_CamConnectFlag[0])
			*(pUI_CamConnectFlag[0]+tCamNum) = FALSE;
		if(pUI_CamConnectFlag[1])
			*(pUI_CamConnectFlag[1]+tCamNum) = FALSE;
	}
}
//------------------------------------------------------------------------------
#define X_START_ANR_TRIP		0
#define Y_START_ANR_TRIP		615
#define X_START_CAMNUM_TRIP		0
#define Y_START_CAMNUM_TRIP		575
#define X_START_ADOSRC_TRIP		0	
#define Y_START_ADOSRC_TRIP		536
#define X_START_ANT_TRIP		0
#define Y_START_ANT_TRIP		486
#define X_START_BAT_TRIP		0	
#define Y_START_BAT_TRIP		426	

void UI_RedrawCamConnectStatusIcon(UI_CamNum_t tCamNum)
{		
#if (defined(BUC_CU) && APP_DUAL_HOST_ENABLE)
	static OSD_IMG_INFO tFixLocateImgInfo;
#endif
	static OSD_IMG_INFO tCamNumOsdImgInfo[CAM_4T], tCamNumHvOsdImgInfo[CAM_4T];
	static OSD_IMG_INFO tAdoSrcOsdImgInfo, tMarkAdoOsdImgInfo;
	static OSD_IMG_INFO tAdoSrcHvOsdImgInfo, tMarkAdoHvOsdImgInfo;
	//! For H View
	static OSD_IMG_INFO tCamAntLvlOsdImgInfo[6], tCamBatLvlOsdImgInfo[6];
	static OSD_IMG_INFO tCamAntLvlHvOsdImgInfo[6], tCamBatLvlHvOsdImgInfo[6];
	static uint8_t ubUI_RdCamStsOsdImgFlag 	    = FALSE;
	static uint8_t ubUI_CamOnlineFlag[CAM_4T]   = {FALSE, FALSE, FALSE, FALSE};
	static uint8_t ubUI_CamOfflineFlag[CAM_4T]  = {FALSE, FALSE, FALSE, FALSE};
	static uint8_t ubUI_NoSignalOsdFlag[CAM_4T] = {FALSE, FALSE, FALSE, FALSE};
	uint16_t uwCamNumOsdIdx[CAM_4T] = {OSD2IMG_CAM1_STSICON, OSD2IMG_CAM2_STSICON,
									   OSD2IMG_CAM3_STSICON, OSD2IMG_CAM4_STSICON};
	//! For H View
	uint16_t uwCamNumHvOsdIdx[CAM_4T] = {OSD2IMG_CAM1_H_STSICON, OSD2IMG_CAM2_H_STSICON,
									     OSD2IMG_CAM3_H_STSICON, OSD2IMG_CAM4_H_STSICON};
	OSD_IMG_INFO tOsdImgInfo;
	UI_DisplayLocation_t tUI_DispLoc;
	uint16_t uwAntLvLIdx = ANT_NOSIGNAL, uwBatLvLIdx = BAT_LVL0;

	if(FALSE == ubUI_RdCamStsOsdImgFlag)
	{
		UI_CamNum_t tSelCamNum;

		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_ADOSRC_STSICON, 1, &tAdoSrcOsdImgInfo);
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_STSMASK_STSICON, 1, &tMarkAdoOsdImgInfo);
		for(tSelCamNum = CAM1; tSelCamNum < tUI_CuSetting.ubTotalCamNum; tSelCamNum++)
			tOSD_GetOsdImgInfor(1, OSD_IMG2, uwCamNumOsdIdx[tSelCamNum], 1, &tCamNumOsdImgInfo[tSelCamNum]);
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_ANTLVL5_STSICON, 6, &tCamAntLvlOsdImgInfo);
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_BATLVL4_STSICON, 6, &tCamBatLvlOsdImgInfo);

		//! For H View
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_ADOSRC_H_STSICON, 1, &tAdoSrcHvOsdImgInfo);
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_STSMASK_H_STSICON, 1, &tMarkAdoHvOsdImgInfo);
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_ANTLVL5_H_STSICON, 6, &tCamAntLvlHvOsdImgInfo);
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_BATLVL4_H_STSICON, 6, &tCamBatLvlHvOsdImgInfo);
		for(tSelCamNum = CAM1; tSelCamNum < tUI_CuSetting.ubTotalCamNum; tSelCamNum++)
			tOSD_GetOsdImgInfor(1, OSD_IMG2, uwCamNumHvOsdIdx[tSelCamNum], 1, &tCamNumHvOsdImgInfo[tSelCamNum]);

		pUI_CamConnectFlag[0] = &ubUI_CamOnlineFlag[0];
		pUI_CamConnectFlag[1] = &ubUI_CamOfflineFlag[0];
		ubUI_RdCamStsOsdImgFlag = TRUE;
	}
	if((TRUE == tUI_CuSetting.IconSts.ubShowLostLogoFlag) ||
	   (UI_MAINMENU_STATE == tUI_State))
	{
		ubUI_CamOnlineFlag[tCamNum]  = FALSE;
		ubUI_CamOfflineFlag[tCamNum] = FALSE;
		if(UI_MAINMENU_STATE == tUI_State)
			return;
		//! No Signal
		if((DISPLAY_1T1R != tUI_CuSetting.ubTotalCamNum) && (TRUE == ubUI_NoSignalOsdFlag[tCamNum]) &&
		   (UI_RECFILES_SEL_STATE != tUI_State))
			UI_RedrawNoSignalOsdIcon(tCamNum, UI_OsdErase);
		return;
	}
	if((DISPLAY_4T1R == tUI_CuSetting.ubTotalCamNum) && (tCamViewSel.tCamViewType == DUAL_VIEW))
		tUI_DispLoc = (tCamNum == tCamViewSel.tCamViewPool[0])?DISP_LEFT:DISP_RIGHT;
	else
		tUI_DispLoc = ((tCamViewSel.tCamViewType == SINGLE_VIEW) || (tCamViewSel.tCamViewType == SCAN_VIEW))?DISP_UPPER_RIGHT:tUI_CamStatus[tCamNum].tCamDispLocation;
	
	#if (defined(BUC_CU) && APP_DUAL_HOST_ENABLE)
		if(tCamViewSel.tCamViewType == V3_3T_VIEW)
		{
			tUI_DispLoc = DISP_UPPER_LEFT;
		}
	#endif	
	
	switch(tUI_CamStatus[tCamNum].tCamConnSts)
	{
		case CAM_ONLINE:
			uwAntLvLIdx = tUI_CamStatus[tCamNum].tCamAntLvl;
			uwBatLvLIdx = BAT_LVL4;
		//justin 2020.07.07
		#if (defined(BUC_CU) && APP_DUAL_HOST_ENABLE)
		#else
			if(TRUE == ubUI_CamOnlineFlag[tCamNum])
				break;
		#endif
			if(PS_ECO_MODE != tUI_CamStatus[tCamNum].tCamPsMode)
			{
				//! Mask No Signal
				if(TRUE == ubUI_NoSignalOsdFlag[tCamNum])
					UI_RedrawNoSignalOsdIcon(tCamNum, UI_OsdErase);
				//! ANR
				if(tUI_CuSetting.tAdoSrcCamNum == tCamNum)
				{
					uint16_t uwNROsdImgIdx = (tCamViewSel.tCamViewType == H_VIEW)?OSD2IMG_NROFF_H_ICON:OSD2IMG_NROFF_ICON;
					tOSD_GetOsdImgInfor(1, OSD_IMG2, uwNROsdImgIdx+tUI_CamStatus[tUI_CuSetting.tAdoSrcCamNum].tCamAnrMode, 1, &tOsdImgInfo);
				
				//justin 2020.06.30
				#if (defined(BUC_CU) && APP_DUAL_HOST_ENABLE)				
					if(tCamViewSel.tCamViewType == V3_3T_VIEW)
					{
						memcpy(&tFixLocateImgInfo,&tOsdImgInfo,sizeof(OSD_IMG_INFO));
						tFixLocateImgInfo.uwXStart = X_START_ANR_TRIP;
						tFixLocateImgInfo.uwYStart = Y_START_ANR_TRIP;
						UI_UpdateCamStatusOsdImg(&tFixLocateImgInfo, OSD_QUEUE, UI_OsdUpdate, tUI_DispLoc);
					}
					else
					{
						UI_UpdateCamStatusOsdImg(&tOsdImgInfo, OSD_QUEUE, UI_OsdUpdate, tUI_DispLoc);
					}
				#else					
					UI_UpdateCamStatusOsdImg(&tOsdImgInfo, OSD_QUEUE, UI_OsdUpdate, tUI_DispLoc);
				#endif
				}
			}
			
			//justin 2020.06.30
			#if (defined(BUC_CU) && APP_DUAL_HOST_ENABLE)
				if(tCamViewSel.tCamViewType == V3_3T_VIEW)
				{					
					memcpy(&tFixLocateImgInfo,&tCamNumOsdImgInfo,sizeof(OSD_IMG_INFO));
					tFixLocateImgInfo.uwXStart = X_START_CAMNUM_TRIP;
					tFixLocateImgInfo.uwYStart = Y_START_CAMNUM_TRIP;
					UI_UpdateCamStatusOsdImg(&tFixLocateImgInfo, OSD_QUEUE, UI_OsdUpdate, tUI_DispLoc);
			
				}
				else
				{
					UI_UpdateCamStatusOsdImg(((tCamViewSel.tCamViewType == H_VIEW)?&tCamNumHvOsdImgInfo[tCamNum]:&tCamNumOsdImgInfo[tCamNum]), OSD_QUEUE, UI_OsdUpdate, tUI_DispLoc);
				}
			#else
				//! Camera Number
				UI_UpdateCamStatusOsdImg(((tCamViewSel.tCamViewType == H_VIEW)?&tCamNumHvOsdImgInfo[tCamNum]:&tCamNumOsdImgInfo[tCamNum]), OSD_QUEUE, UI_OsdUpdate, tUI_DispLoc);
			#endif			
			
			#if (defined(BUC_CU) && APP_DUAL_HOST_ENABLE)
				if(tCamViewSel.tCamViewType == V3_3T_VIEW)
				{					
					memcpy(&tFixLocateImgInfo,&tAdoSrcOsdImgInfo,sizeof(OSD_IMG_INFO));
					tFixLocateImgInfo.uwXStart = X_START_ADOSRC_TRIP;
					tFixLocateImgInfo.uwYStart = Y_START_ADOSRC_TRIP;
					if(tUI_CuSetting.tAdoSrcCamNum == tCamNum)
						UI_UpdateCamStatusOsdImg(&tFixLocateImgInfo, OSD_QUEUE, UI_OsdUpdate, tUI_DispLoc);
				}
				else
				{
					if(tUI_CuSetting.tAdoSrcCamNum == tCamNum)
						UI_UpdateCamStatusOsdImg(((tCamViewSel.tCamViewType == H_VIEW)?&tAdoSrcHvOsdImgInfo:&tAdoSrcOsdImgInfo), OSD_QUEUE, UI_OsdUpdate, tUI_DispLoc);
				}
			#else
				//! Audio Source
				if(tUI_CuSetting.tAdoSrcCamNum == tCamNum)
					UI_UpdateCamStatusOsdImg(((tCamViewSel.tCamViewType == H_VIEW)?&tAdoSrcHvOsdImgInfo:&tAdoSrcOsdImgInfo), OSD_QUEUE, UI_OsdUpdate, tUI_DispLoc);
			#endif
				
			ubUI_NoSignalOsdFlag[tCamNum] 	= FALSE;
			ubUI_CamOnlineFlag[tCamNum]  	= TRUE;
			ubUI_CamOfflineFlag[tCamNum] 	= FALSE;
			break;
		case CAM_OFFLINE:
			if(TRUE == ubUI_CamOfflineFlag[tCamNum])
				return;
			
			//! ANR
			tOSD_GetOsdImgInfor(1, OSD_IMG2, (tCamViewSel.tCamViewType == H_VIEW)?OSD2IMG_NRMASK_H_ICON:OSD2IMG_NRMASK_ICON, 1, &tOsdImgInfo);
			
		#if (defined(BUC_CU) && APP_DUAL_HOST_ENABLE)	
			if(tCamViewSel.tCamViewType == V3_3T_VIEW)
			{
				memcpy(&tFixLocateImgInfo,&tOsdImgInfo,sizeof(OSD_IMG_INFO));
				tFixLocateImgInfo.uwXStart = X_START_ANR_TRIP;
				tFixLocateImgInfo.uwYStart = Y_START_ANR_TRIP;
				UI_UpdateCamStatusOsdImg(&tFixLocateImgInfo, OSD_QUEUE, UI_OsdUpdate, tUI_DispLoc);
			}
			else
			{
				UI_UpdateCamStatusOsdImg(&tOsdImgInfo, OSD_QUEUE, UI_OsdUpdate, tUI_DispLoc);
			}
		#else
			UI_UpdateCamStatusOsdImg(&tOsdImgInfo, OSD_QUEUE, UI_OsdUpdate, tUI_DispLoc);
		#endif						
			
			//! Camera Number
			//UI_UpdateCamStatusOsdImg(((tCamViewSel.tCamViewType == H_VIEW)?&tCamNumHvOsdImgInfo[tCamNum]:&tCamNumOsdImgInfo[tCamNum]), OSD_QUEUE, UI_OsdUpdate, tUI_DispLoc);
			//=====================================================
		#if (defined(BUC_CU) && APP_DUAL_HOST_ENABLE)
			if(tCamViewSel.tCamViewType == V3_3T_VIEW)
			{				
				memcpy(&tFixLocateImgInfo,&tCamNumOsdImgInfo,sizeof(OSD_IMG_INFO));
				tFixLocateImgInfo.uwXStart = X_START_CAMNUM_TRIP;
				tFixLocateImgInfo.uwYStart = Y_START_CAMNUM_TRIP;
				UI_UpdateCamStatusOsdImg(&tFixLocateImgInfo, OSD_QUEUE, UI_OsdUpdate, tUI_DispLoc);
		
			}
			else
			{
				UI_UpdateCamStatusOsdImg(((tCamViewSel.tCamViewType == H_VIEW)?&tCamNumHvOsdImgInfo[tCamNum]:&tCamNumOsdImgInfo[tCamNum]), OSD_QUEUE, UI_OsdUpdate, tUI_DispLoc);
			}
		#else
			//! Camera Number
			UI_UpdateCamStatusOsdImg(((tCamViewSel.tCamViewType == H_VIEW)?&tCamNumHvOsdImgInfo[tCamNum]:&tCamNumOsdImgInfo[tCamNum]), OSD_QUEUE, UI_OsdUpdate, tUI_DispLoc);
		#endif
			//=====================================================			
			
			//! Mask Audio Source
			//if(tUI_CuSetting.tAdoSrcCamNum == tCamNum)
			//	UI_UpdateCamStatusOsdImg(((tCamViewSel.tCamViewType == H_VIEW)?&tMarkAdoHvOsdImgInfo:&tMarkAdoOsdImgInfo), OSD_QUEUE, UI_OsdUpdate, tUI_DispLoc);
			//=====================================================
			#if (defined(BUC_CU) && APP_DUAL_HOST_ENABLE)
				if(tCamViewSel.tCamViewType == V3_3T_VIEW)
				{					
					memcpy(&tFixLocateImgInfo,&tAdoSrcOsdImgInfo,sizeof(OSD_IMG_INFO));
					tFixLocateImgInfo.uwXStart = X_START_ADOSRC_TRIP;
					tFixLocateImgInfo.uwYStart = Y_START_ADOSRC_TRIP;
					if(tUI_CuSetting.tAdoSrcCamNum == tCamNum)
						UI_UpdateCamStatusOsdImg(&tFixLocateImgInfo, OSD_QUEUE, UI_OsdUpdate, tUI_DispLoc);
				}
				else
				{
					//! Audio Source
					if(tUI_CuSetting.tAdoSrcCamNum == tCamNum)
						UI_UpdateCamStatusOsdImg(((tCamViewSel.tCamViewType == H_VIEW)?&tAdoSrcHvOsdImgInfo:&tAdoSrcOsdImgInfo), OSD_QUEUE, UI_OsdUpdate, tUI_DispLoc);
				}
			#else
				//! Audio Source
				if(tUI_CuSetting.tAdoSrcCamNum == tCamNum)
					UI_UpdateCamStatusOsdImg(((tCamViewSel.tCamViewType == H_VIEW)?&tAdoSrcHvOsdImgInfo:&tAdoSrcOsdImgInfo), OSD_QUEUE, UI_OsdUpdate, tUI_DispLoc);
			#endif
			//=====================================================
			
			//! No Signal
			if(PS_ECO_MODE != tUI_CamStatus[tCamNum].tCamPsMode)
				UI_RedrawNoSignalOsdIcon(tCamNum, UI_OsdUpdate);
			ubUI_NoSignalOsdFlag[tCamNum] 	= TRUE;
			ubUI_CamOfflineFlag[tCamNum] 	= TRUE;
			ubUI_CamOnlineFlag[tCamNum]  	= FALSE;
			break;
		default:
			break;
	}
	if(TRUE == ubUI_PerDebugEn)
		UI_OsdDisplayFrmErrItem(tUI_DispLoc);
	
#if (defined(BUC_CU) && APP_DUAL_HOST_ENABLE)
	if(tCamViewSel.tCamViewType == V3_3T_VIEW)	//@Trip-View
	{		
		memcpy(&tFixLocateImgInfo,&tCamAntLvlOsdImgInfo[uwAntLvLIdx],sizeof(OSD_IMG_INFO));
		tFixLocateImgInfo.uwXStart = X_START_ANT_TRIP;
		tFixLocateImgInfo.uwYStart = Y_START_ANT_TRIP;
		UI_UpdateCamStatusOsdImg(&tFixLocateImgInfo, OSD_QUEUE, UI_OsdUpdate, tUI_DispLoc);
	}
	else										//@Single/Scan/Dual-View
	{
		//justin 2020.07.08
		if(KNL_WIRELESS_CAM_NUM == 1)
		{
			if(tCamNum == CAM1)
				UI_UpdateCamStatusOsdImg(((tCamViewSel.tCamViewType == H_VIEW)?&tCamAntLvlHvOsdImgInfo[uwAntLvLIdx]:&tCamAntLvlOsdImgInfo[uwAntLvLIdx]), OSD_QUEUE, UI_OsdUpdate, tUI_DispLoc);
		}
	}
#else
	//! Antenna Level
	UI_UpdateCamStatusOsdImg(((tCamViewSel.tCamViewType == H_VIEW)?&tCamAntLvlHvOsdImgInfo[uwAntLvLIdx]:&tCamAntLvlOsdImgInfo[uwAntLvLIdx]), OSD_QUEUE, UI_OsdUpdate, tUI_DispLoc);
#endif
	
#if (defined(BUC_CU) && APP_DUAL_HOST_ENABLE)
	if(tCamViewSel.tCamViewType == V3_3T_VIEW)
	{		
		memcpy(&tFixLocateImgInfo,&tCamBatLvlOsdImgInfo[uwBatLvLIdx],sizeof(OSD_IMG_INFO));
		tFixLocateImgInfo.uwXStart = X_START_BAT_TRIP;
		tFixLocateImgInfo.uwYStart = Y_START_BAT_TRIP;
		UI_UpdateCamStatusOsdImg(&tFixLocateImgInfo, OSD_UPDATE, UI_OsdUpdate, tUI_DispLoc);
	}
	else
	{
		UI_UpdateCamStatusOsdImg(((tCamViewSel.tCamViewType == H_VIEW)?&tCamBatLvlHvOsdImgInfo[uwBatLvLIdx]:&tCamBatLvlOsdImgInfo[uwBatLvLIdx]), OSD_UPDATE, UI_OsdUpdate, tUI_DispLoc);			
	}
#else
	//! Battery Level
	UI_UpdateCamStatusOsdImg(((tCamViewSel.tCamViewType == H_VIEW)?&tCamBatLvlHvOsdImgInfo[uwBatLvLIdx]:&tCamBatLvlOsdImgInfo[uwBatLvLIdx]), OSD_UPDATE, UI_OsdUpdate, tUI_DispLoc);
#endif
}
//------------------------------------------------------------------------------
void UI_DrawCAMStatusIcon(void)
{
	OSD_IMG_INFO tOsdImgInfo;
	UI_CamNum_t tCamNum, tDrawCamNum;
	UI_CamViewType_t tUI_CamViewType = tCamViewSel.tCamViewType;
	uint8_t ubUI_TotalBuNum = (tCamViewSel.tCamViewType == DUAL_VIEW)?CAM_2T:tUI_CuSetting.ubTotalCamNum;

	switch(tUI_CamViewType)
	{
		case SINGLE_VIEW:
		case SCAN_VIEW:
			if(tUI_CamStatus[tCamViewSel.tCamViewPool[0]].ulCAM_ID != INVALID_ID)
				UI_RedrawCamConnectStatusIcon(tCamViewSel.tCamViewPool[0]);
			if(PS_ECO_MODE == tUI_CamStatus[tCamViewSel.tCamViewPool[0]].tCamPsMode)
			{
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_ECOSTS_ICON, 1, &tOsdImgInfo);
				UI_UpdateCamStatusOsdImg(&tOsdImgInfo, OSD_QUEUE, UI_OsdUpdate, DISP_UPPER_RIGHT);
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_IMAGE_PAUSE_ICON, 1, &tOsdImgInfo);
				tOsdImgInfo.uwXStart += 180;
				tOsdImgInfo.uwYStart -= 321;
				tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			}
			else
			{
				if((tUI_CamViewType == SCAN_VIEW) && (TRUE == (*(pUI_CamConnectFlag[0]+tCamViewSel.tCamViewPool[0]))))
				{
					tOsdImgInfo.uwXStart = 280;
					tOsdImgInfo.uwYStart = 480;
					tOsdImgInfo.uwHSize  = 160;
					tOsdImgInfo.uwVSize  = 310;
					OSD_EraserImg2(&tOsdImgInfo);
				}
			}
			break;
		case DUAL_VIEW:
		case QUAD_VIEW:
		case H_VIEW:
			for(tCamNum = CAM1; tCamNum < ubUI_TotalBuNum; tCamNum++)
			{
				tDrawCamNum = tCamNum;
				if((DISPLAY_4T1R == tUI_CuSetting.ubTotalCamNum) && (tCamViewSel.tCamViewType == DUAL_VIEW))
					tDrawCamNum = (tCamNum == CAM1)?tCamViewSel.tCamViewPool[0]:tCamViewSel.tCamViewPool[1];
				if(tUI_CamStatus[tDrawCamNum].ulCAM_ID != INVALID_ID)
					UI_RedrawCamConnectStatusIcon(tDrawCamNum);
				if(PS_ECO_MODE == tUI_CamStatus[tDrawCamNum].tCamPsMode)
				{
					UI_DisplayLocation_t tUI_DispLoc;

					if(UI_SET_CAMECOMODE_STATE == tUI_State)
						break;
					tUI_DispLoc = (DUAL_VIEW == tCamViewSel.tCamViewType)?((tDrawCamNum == tCamViewSel.tCamViewPool[0])?DISP_LEFT:DISP_RIGHT):tUI_CamStatus[tDrawCamNum].tCamDispLocation;
					tOSD_GetOsdImgInfor(1, OSD_IMG2, (H_VIEW == tUI_CamViewType)?OSD2IMG_ECOSTS_H_ICON:OSD2IMG_ECOSTS_ICON, 1, &tOsdImgInfo);
					UI_UpdateCamStatusOsdImg(&tOsdImgInfo, OSD_QUEUE, UI_OsdUpdate, tUI_DispLoc);
					if(H_VIEW == tUI_CamViewType)
					{
						tOSD_GetOsdImgInfor(1, OSD_IMG2, ((DISP_H_L == tUI_DispLoc) || (DISP_H_R == tUI_DispLoc))?OSD2IMG_IMGPAUSE_HS_ICON:OSD2IMG_IMGPAUSE_C_ICON, 1, &tOsdImgInfo);
						tOsdImgInfo.uwXStart += (DISP_H_CL == tUI_DispLoc)?(uwLCD_GetLcdHoSize() / 2):0;
						tOsdImgInfo.uwYStart -= (DISP_H_R == tUI_DispLoc)?960:0;
						tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
					}
					else
					{
						tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_IMAGE_PAUSE_ICON, 1, &tOsdImgInfo);
						tOsdImgInfo.uwXStart += (tCamViewSel.tCamViewType == DUAL_VIEW)?180:(tCamViewSel.tCamViewType == QUAD_VIEW)?30:0;
						UI_UpdateCamStatusOsdImg(&tOsdImgInfo, OSD_UPDATE, UI_OsdUpdate, tUI_DispLoc);
					}
				}
			}
			break;
		case V3_3T_VIEW:			
			for(tCamNum = CAM1; tCamNum < 1; tCamNum++)
			{
				tDrawCamNum = tCamNum;
				if((DISPLAY_4T1R == tUI_CuSetting.ubTotalCamNum) && (tCamViewSel.tCamViewType == DUAL_VIEW))
					tDrawCamNum = (tCamNum == CAM1)?tCamViewSel.tCamViewPool[0]:tCamViewSel.tCamViewPool[1];
				if(tUI_CamStatus[tDrawCamNum].ulCAM_ID != INVALID_ID)
					UI_RedrawCamConnectStatusIcon(tDrawCamNum);
				if(PS_ECO_MODE == tUI_CamStatus[tDrawCamNum].tCamPsMode)
				{
					UI_DisplayLocation_t tUI_DispLoc;

					if(UI_SET_CAMECOMODE_STATE == tUI_State)
						break;
					tUI_DispLoc = (DUAL_VIEW == tCamViewSel.tCamViewType)?((tDrawCamNum == tCamViewSel.tCamViewPool[0])?DISP_LEFT:DISP_RIGHT):tUI_CamStatus[tDrawCamNum].tCamDispLocation;
					tOSD_GetOsdImgInfor(1, OSD_IMG2, (H_VIEW == tUI_CamViewType)?OSD2IMG_ECOSTS_H_ICON:OSD2IMG_ECOSTS_ICON, 1, &tOsdImgInfo);
					
					tOsdImgInfo.uwXStart = 300;
					tOsdImgInfo.uwYStart = 590;
					
					tOsdImgInfo1.uwXStart 	= tOsdImgInfo.uwXStart;
					tOsdImgInfo1.uwYStart 	= tOsdImgInfo.uwYStart;
					tOsdImgInfo1.uwHSize 	= tOsdImgInfo.uwHSize;
					tOsdImgInfo1.uwVSize 	= tOsdImgInfo.uwVSize;
					
					UI_UpdateCamStatusOsdImg(&tOsdImgInfo, OSD_QUEUE, UI_OsdUpdate, tUI_DispLoc);
					
//					printf("Delay(2 Sec)\r\n");
//					osDelay(2000);
//					
//					OSD_EraserImg2(&tOsdImgInfo);
					
//					if(H_VIEW == tUI_CamViewType)
//					{
//						tOSD_GetOsdImgInfor(1, OSD_IMG2, ((DISP_H_L == tUI_DispLoc) || (DISP_H_R == tUI_DispLoc))?OSD2IMG_IMGPAUSE_HS_ICON:OSD2IMG_IMGPAUSE_C_ICON, 1, &tOsdImgInfo);
//						tOsdImgInfo.uwXStart += (DISP_H_CL == tUI_DispLoc)?(uwLCD_GetLcdHoSize() / 2):0;
//						tOsdImgInfo.uwYStart -= (DISP_H_R == tUI_DispLoc)?960:0;
//						tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
//					}
//					else
//					{
//						tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_IMAGE_PAUSE_ICON, 1, &tOsdImgInfo);
//						tOsdImgInfo.uwXStart += (tCamViewSel.tCamViewType == DUAL_VIEW)?180:(tCamViewSel.tCamViewType == QUAD_VIEW)?30:0;
//						UI_UpdateCamStatusOsdImg(&tOsdImgInfo, OSD_UPDATE, UI_OsdUpdate, tUI_DispLoc);
//					}
					
					tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_IMAGE_PAUSE_ICON, 1, &tOsdImgInfo);
					tOsdImgInfo.uwXStart += (tCamViewSel.tCamViewType == DUAL_VIEW)?180:(tCamViewSel.tCamViewType == QUAD_VIEW)?30:0;
					
					tOsdImgInfo.uwXStart = 360;
					tOsdImgInfo.uwYStart = 590;
					
					tOsdImgInfo2.uwXStart 	= tOsdImgInfo.uwXStart;
					tOsdImgInfo2.uwYStart 	= tOsdImgInfo.uwYStart;
					tOsdImgInfo2.uwHSize 	= tOsdImgInfo.uwHSize;
					tOsdImgInfo2.uwVSize 	= tOsdImgInfo.uwVSize;					
					
					UI_UpdateCamStatusOsdImg(&tOsdImgInfo, OSD_UPDATE, UI_OsdUpdate, tUI_DispLoc);
					
//					printf("Delay(2 Sec)\r\n");
//					osDelay(2000);
//					OSD_EraserImg2(&tOsdImgInfo);
				}
				else
				{
					#if (defined(BUC_CU) && APP_DUAL_HOST_ENABLE)	
					if(ubUI_ClrFlg == TRUE)
					{
						ubUI_ClrFlg = FALSE;		
						printf("Clear ECO\r\n");			
						OSD_EraserImg2(&tOsdImgInfo1);
						OSD_EraserImg2(&tOsdImgInfo2);			
					}
					#endif
		
//					printf("Clear(1)\r\n");
//					OSD_EraserImg2(&tOsdImgInfo1);
//					printf("Clear(2)\r\n");
//					OSD_EraserImg2(&tOsdImgInfo2);
				}
			}
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_RemoveLostLinkLogo(void)
{
	tUI_CuSetting.IconSts.ubShowLostLogoFlag = FALSE;
}
//------------------------------------------------------------------------------
void UI_ShowLostLinkLogo(uint16_t *pThreadCnt)
{
	UI_CamNum_t tCamNum;
	uint16_t uwUI_LostPeriod = (FALSE == ubUI_ResetPeriodFlag)?(UI_SHOWLOSTLOGO_PERIOD * 2):UI_SHOWLOSTLOGO_PERIOD;

	if(TRUE == ubUI_ResetPeriodFlag)
	{
		switch(tUI_CuSetting.tPsMode)
		{
			case PS_VOX_MODE:
				UI_DisableVox();
				break;
			case PS_ADOONLY_MODE:
				UI_DisableCuAdoOnlyMode();
				break;
			default:
				break;
		}
	}
	for(tCamNum = CAM1; tCamNum < tUI_CuSetting.ubTotalCamNum; tCamNum++)
	{
		if(PS_ECO_MODE == tUI_CamStatus[tCamNum].tCamPsMode)
		{
			if((GLB->LCD_FUNC_DIS == 1) ||
			   (TRUE == ubUI_StopUpdateStsBarFlag))
				return;
			UI_DrawCUStatusIcon();
			UI_DrawCAMStatusIcon();
			return;
		}
	}
	if((FALSE == tUI_CuSetting.IconSts.ubShowLostLogoFlag) && (*pThreadCnt == uwUI_LostPeriod))
	{
		UI_CamNum_t tCamNum;

		tUI_CuSetting.IconSts.ubShowLostLogoFlag = TRUE;
		ubUI_ResetPeriodFlag = TRUE;
		if(TRUE == ubUI_StopUpdateStsBarFlag)
			return;
		switch(tUI_State)
		{
		    case UI_RECPLAYLIST_STATE:
			case UI_PHOTOPLAYLIST_STATE:
			case UI_PAIRING_STATE:
		#if APP_FS_FILE_LIST_STYLE
			case UI_RECFILES_SEL_STATE:
		#endif
				return;
			case UI_MAINMENU_STATE:
			case UI_SUBSUBSUBMENU_STATE:
			case UI_RECFOLDER_SEL_STATE:
		#if !APP_FS_FILE_LIST_STYLE
			case UI_RECFILES_SEL_STATE:
		#endif
			case UI_SDCARDFMT_STATE:
				break;
			case UI_SUBMENU_STATE:
			case UI_SUBSUBMENU_STATE:
				if(FALSE == ubUI_FastStateFlag)
					break;
			default:
			{
				OSD_IMG_INFO tOsdInfo;

				for(tCamNum = CAM1; tCamNum < tUI_CuSetting.ubTotalCamNum; tCamNum++)
					UI_RedrawCamConnectStatusIcon(tCamNum);
				tUI_CuSetting.IconSts.ubDrawStsIconFlag = TRUE;
				tOsdInfo.uwHSize  = uwOSD_GetHSize();
				tOsdInfo.uwVSize  = uwOSD_GetVSize();
				tOsdInfo.uwXStart = 0;
				tOsdInfo.uwYStart = 0;
				OSD_EraserImg1(&tOsdInfo);
				tUI_State = UI_DISPLAY_STATE;
				break;
			}
		}
		tLCD_JpegDecodeDisable();
		OSD_LogoJpeg(OSDLOGO_LOSTLINK);
	}
}
//------------------------------------------------------------------------------
void UI_RedrawNoSignalOsdIcon(UI_CamNum_t tCamNum, UI_OsdImgFnType_t tOsdImgFnType)
{
	OSD_IMG_INFO tOsdImgInfo;
	UI_DisplayLocation_t tUI_DispLoc;

	switch(tCamViewSel.tCamViewType)
	{
		case H_VIEW:
		{
			uint16_t uwHvNoSignOsdImgIdx;
			uint16_t uwHvXOffset, uwHvYOffset;

			tUI_DispLoc = tUI_CamStatus[tCamNum].tCamDispLocation;
			uwHvNoSignOsdImgIdx = ((DISP_H_L == tUI_DispLoc) || (DISP_H_R == tUI_DispLoc))?OSD2IMG_NOSIGNAL_HS_STSICON:OSD2IMG_NOSIGNAL_C_STSICON;
			uwHvXOffset = (DISP_H_CL == tUI_DispLoc)?(uwLCD_GetLcdHoSize() / 2):0;
			uwHvYOffset = (DISP_H_R == tUI_DispLoc)?960:0;
			tOSD_GetOsdImgInfor(1, OSD_IMG2, uwHvNoSignOsdImgIdx, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart += uwHvXOffset;
			tOsdImgInfo.uwYStart -= uwHvYOffset;
			if(UI_OsdUpdate == tOsdImgFnType)
				tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
			else if(UI_OsdErase == tOsdImgFnType)
				OSD_EraserImg2(&tOsdImgInfo);
			break;
		}
		//justin 2020.06.30
		case V3_3T_VIEW:
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_NOSIGNAL_STSICON, 1, &tOsdImgInfo);

			tOsdImgInfo.uwXStart = 320;
			tOsdImgInfo.uwYStart = 492;
			tUI_DispLoc = DISP_UPPER_LEFT;			
			
			UI_UpdateCamStatusOsdImg(&tOsdImgInfo, OSD_QUEUE, tOsdImgFnType, tUI_DispLoc);
			break;
		default:
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_NOSIGNAL_STSICON, 1, &tOsdImgInfo);
			if ((tCamViewSel.tCamViewType == SINGLE_VIEW) || (tCamViewSel.tCamViewType == SCAN_VIEW))
			{
				tOsdImgInfo.uwXStart += 192;
				tOsdImgInfo.uwYStart -= 355;
				tUI_DispLoc = DISP_UPPER_LEFT;
			}
			else
			{
				tOsdImgInfo.uwXStart += (tCamViewSel.tCamViewType == DUAL_VIEW)?180:10;
				tOsdImgInfo.uwYStart -= 40;
				tUI_DispLoc = ((DISPLAY_4T1R == tUI_CuSetting.ubTotalCamNum) && (tCamViewSel.tCamViewType == DUAL_VIEW))?((tCamNum == tCamViewSel.tCamViewPool[0])?DISP_LEFT:DISP_RIGHT):tUI_CamStatus[tCamNum].tCamDispLocation;
			}
			UI_UpdateCamStatusOsdImg(&tOsdImgInfo, OSD_QUEUE, tOsdImgFnType, tUI_DispLoc);
			break;
	}
}
//------------------------------------------------------------------------------
void UI_ClearStatusBarOsdIcon(void)
{
	uint32_t ulLcd_HSize = (uwLCD_GetLcdHoSize() / 2);
	uint32_t ulLcd_VSize = uwLCD_GetLcdVoSize();
	OSD_IMG_INFO tOsdImgInfo = {0};

	if(FALSE == tUI_CuSetting.IconSts.ubDrawStsIconFlag)
		return;

	tOsdImgInfo.uwHSize = 100;
	tOsdImgInfo.uwVSize = ulLcd_VSize;
	OSD_EraserImg1(&tOsdImgInfo);
	if(DISPLAY_4T1R == tUI_CuSetting.ubTotalCamNum)
	{
		tOsdImgInfo.uwXStart += ulLcd_HSize;
		OSD_EraserImg1(&tOsdImgInfo);
	}
	tUI_CuSetting.IconSts.ubDrawStsIconFlag = FALSE;
}
//------------------------------------------------------------------------------
void UI_RedrawStatusBar(uint16_t *pThreadCnt)
{
	if(TRUE == ubUI_StopUpdateStsBarFlag)
		return;
	if(LCD_JPEG_ENABLE == tLCD_GetJpegDecoderStatus())
	{
		*pThreadCnt = (UI_UPDATESTS_PERIOD - 1);
		return;
	}
	if((*pThreadCnt % UI_UPDATESTS_PERIOD) == 0)
	{
		UI_DrawCUStatusIcon();
		UI_DrawCAMStatusIcon();
	}
	if(TRUE == ubUI_ShowTimeFlag)
		UI_ShowSysTime();
}
//------------------------------------------------------------------------------
void UI_ChangeBuPsModeToNormalMode(UI_CamNum_t tPS_CamNum)
{
	OSD_IMG_INFO tOsdImgInfo;
	UI_DisplayLocation_t tUI_DispLoc;

	tUI_DispLoc = ((tCamViewSel.tCamViewType == SINGLE_VIEW) || (tCamViewSel.tCamViewType == SCAN_VIEW))?DISP_UPPER_RIGHT:
	               (DUAL_VIEW == tCamViewSel.tCamViewType)?((tPS_CamNum == tCamViewSel.tCamViewPool[0])?DISP_LEFT:DISP_RIGHT):tUI_CamStatus[tPS_CamNum].tCamDispLocation;
	if(tCamViewSel.tCamViewType != SCAN_VIEW)
	{
	#if (defined(BUC_CU) && APP_DUAL_HOST_ENABLE)
		if(tCamViewSel.tCamViewType == SINGLE_VIEW)		
		{
			tOSD_GetOsdImgInfor(1, OSD_IMG2, (tCamViewSel.tCamViewType == H_VIEW)?OSD2IMG_NRMASK_H_ICON:OSD2IMG_NRMASK_ICON, 1, &tOsdImgInfo);
			UI_UpdateCamStatusOsdImg(&tOsdImgInfo, OSD_UPDATE, UI_OsdUpdate, tUI_DispLoc);
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_IMAGE_PAUSE_ICON, 1, &tOsdImgInfo);			
			tOsdImgInfo.uwXStart += 180;
			tOsdImgInfo.uwYStart -= 321;
			OSD_EraserImg2(&tOsdImgInfo);			
		}
	#else			
		tOSD_GetOsdImgInfor(1, OSD_IMG2, (tCamViewSel.tCamViewType == H_VIEW)?OSD2IMG_NRMASK_H_ICON:OSD2IMG_NRMASK_ICON, 1, &tOsdImgInfo);
		UI_UpdateCamStatusOsdImg(&tOsdImgInfo, OSD_UPDATE, UI_OsdUpdate, tUI_DispLoc);
		if(tCamViewSel.tCamViewType == H_VIEW)
		{
			tOSD_GetOsdImgInfor(1, OSD_IMG2, ((DISP_H_L == tUI_DispLoc) || (DISP_H_R == tUI_DispLoc))?OSD2IMG_IMGPAUSE_HS_ICON:OSD2IMG_IMGPAUSE_C_ICON, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart += (DISP_H_CL == tUI_DispLoc)?(uwLCD_GetLcdHoSize() / 2):0;
			tOsdImgInfo.uwYStart -= (DISP_H_R == tUI_DispLoc)?960:0;
			OSD_EraserImg2(&tOsdImgInfo);
		}
		else
		{
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_IMAGE_PAUSE_ICON, 1, &tOsdImgInfo);
			if (tCamViewSel.tCamViewType == SINGLE_VIEW)
			{
				tOsdImgInfo.uwXStart += 180;
				tOsdImgInfo.uwYStart -= 321;
				OSD_EraserImg2(&tOsdImgInfo);
			}
			else
			{
				tOsdImgInfo.uwXStart += (tCamViewSel.tCamViewType == DUAL_VIEW)?180:(tCamViewSel.tCamViewType == QUAD_VIEW)?30:0;
				UI_UpdateCamStatusOsdImg(&tOsdImgInfo, OSD_UPDATE, UI_OsdErase, tUI_DispLoc);
			}
		}
	#endif
	}
	if(pUI_CamConnectFlag[0])
		*(pUI_CamConnectFlag[0]+tPS_CamNum) = FALSE;
	if(pUI_CamConnectFlag[1])
		*(pUI_CamConnectFlag[1]+tPS_CamNum) = FALSE;
	tUI_CamStatus[tPS_CamNum].tCamPsMode = POWER_NORMAL_MODE;
	UI_UpdateDevStatusInfo();
}
//------------------------------------------------------------------------------
void UI_ReportCamConnectionStatus(void *pvConnectionSts)
{
	uint8_t *pCamConnSts = (uint8_t *)pvConnectionSts;
	UI_CamNum_t tCamNum;
	UI_PerMap2AntLvl_t tAntMap[] =
	{
		{ANT_NOSIGNAL, 		 10},
		{ANT_SIGNALLVL1, 	 20},
		{ANT_SIGNALLVL2, 	 40},
		{ANT_SIGNALLVL3, 	 60},
		{ANT_SIGNALLVL4, 	 80},
		{ANT_SIGNALLVL5, 	100},
	};
	uint8_t ubUI_AntLvlCnt = sizeof tAntMap / sizeof(UI_PerMap2AntLvl_t), ubIdx;
	static uint8_t ubUI_PsStsFlag = FALSE;
	static uint32_t ulUI_EcoStsCnt[CAM_4T] = {0, 0, 0, 0};
	static uint8_t ubUI_TotalCamEcoNum = 0, ubUI_WakeUpCamCnt = 0;

	for(tCamNum = CAM1; tCamNum < tUI_CuSetting.ubTotalCamNum; tCamNum++)
	{
		tUI_CamStatus[tCamNum].tCamConnSts 	= (pCamConnSts[APP_GetSTANumMappingTable(tCamNum)->tKNL_StaNum] == rLINK)?CAM_ONLINE:CAM_OFFLINE;
		if(PS_ECO_MODE == tUI_CamStatus[tCamNum].tCamPsMode)
		{
			switch(tUI_CamStatus[tCamNum].tCamConnSts)
			{
				case CAM_ONLINE:
					if(TRUE == ulUI_MonitorPsFlag[tCamNum])
					{
						APP_EventMsg_t tUI_PsMessage = {0};

						tUI_PsMessage.ubAPP_Event 	   = APP_POWERSAVE_EVENT;
						tUI_PsMessage.ubAPP_Message[0] = 4;		//! Message Length
						tUI_PsMessage.ubAPP_Message[1] = PS_ECO_MODE;
						tUI_PsMessage.ubAPP_Message[2] = FALSE;
						tUI_PsMessage.ubAPP_Message[3] = tCamNum;
						tUI_PsMessage.ubAPP_Message[4] = TRUE;
						UI_SendMessageToAPP(&tUI_PsMessage);
						UI_ChangeBuPsModeToNormalMode(tCamNum);
						ulUI_MonitorPsFlag[tCamNum] = FALSE;
						if(ubUI_TotalCamEcoNum)
							--ubUI_TotalCamEcoNum;
					}
					else
					{
					#define	UI_CHKBUECOSTS_PERIOD (2000 / UI_TASK_PERIOD)
						if(++ulUI_EcoStsCnt[tCamNum] > UI_CHKBUECOSTS_PERIOD)
						{
							ulUI_EcoStsCnt[tCamNum] = 0;
							UI_ChangeBuPsModeToNormalMode(tCamNum);
						}
					}
					break;
				case CAM_OFFLINE:
					if(FALSE == ulUI_MonitorPsFlag[tCamNum])
					{
						ulUI_EcoStsCnt[tCamNum] = 0;
						ulUI_MonitorPsFlag[tCamNum] = TRUE;
						++ubUI_TotalCamEcoNum;
						ubUI_WakeUpCamCnt = 0;
					}
					tUI_CamStatus[tCamNum].tCamConnSts = CAM_ONLINE;
					if(FALSE == ubUI_PsStsFlag)
					{
						tLCD_JpegDecodeDisable();
						UI_DrawCUStatusIcon();
						UI_DrawCAMStatusIcon();
						ubUI_PsStsFlag = TRUE;
					}
					continue;
				default:
					break;
			}
		}
		else
			ulUI_EcoStsCnt[tCamNum] = 0;

#ifdef A7130
		if((BB_ENABLE_ALL_STA_WAKEUP == tKNL_GetWORMode()) && (TRUE == ubUI_WakeUpFromPsFlag))
		{
		#define	UI_PSWAKEUP_TIME (6000 / UI_TASK_PERIOD)
			if((!ubUI_TotalCamEcoNum) || (++ubUI_WakeUpCamCnt > UI_PSWAKEUP_TIME))
			{
				APP_EventMsg_t tUI_PsMessage = {0};

				tUI_PsMessage.ubAPP_Event 	   = APP_POWERSAVE_EVENT;
				tUI_PsMessage.ubAPP_Message[0] = 4;
				tUI_PsMessage.ubAPP_Message[1] = POWER_NORMAL_MODE;
				tUI_PsMessage.ubAPP_Message[2] = FALSE;
				tUI_PsMessage.ubAPP_Message[3] = CAM1;
				UI_SendMessageToAPP(&tUI_PsMessage);
				ubUI_WakeUpFromPsFlag = FALSE;
				ubUI_TotalCamEcoNum	  = 0;
				ubUI_WakeUpCamCnt	  = 0;
				printd(DBG_Debug1Lvl, "  _Stop Wake-Up !\n");
			}
		}
#else
		ubUI_WakeUpFromPsFlag 	= ubUI_WakeUpFromPsFlag;
		ubUI_TotalCamEcoNum		= ubUI_TotalCamEcoNum;
		ubUI_WakeUpCamCnt		= ubUI_WakeUpCamCnt;
#endif

		tUI_CamStatus[tCamNum].tCamAntLvl = ANT_NOSIGNAL;
		printd(DBG_Debug3Lvl, "PER: %d\n", pCamConnSts[(APP_GetSTANumMappingTable(tCamNum)->tKNL_StaNum) + 4]);
		for(ubIdx = 0; ubIdx < ubUI_AntLvlCnt; ubIdx++)
		{
			if((pCamConnSts[(APP_GetSTANumMappingTable(tCamNum)->tKNL_StaNum) + 4]) <= tAntMap[ubIdx].ubPerValue)
			{
				tUI_CamStatus[tCamNum].tCamAntLvl = tAntMap[ubIdx].tAntLvl;
				break;
			}
		}
	}
}
//------------------------------------------------------------------------------
void UI_UpdateCamStatus(UI_CamNum_t tCamNum, void *pvStatus)
{
//	uint8_t *pUI_BuSts = (uint8_t *)pvStatus;

//	if(tCamNum > CAM4)
//		return;

//	tUI_CamStatus[tCamNum].tCamBatLvl = pUI_BuSts[0];
}
//------------------------------------------------------------------------------
void UI_LeftArrowLongKey(void)
{
	OSD_IMG_INFO tOsdImgInfo;
	UI_VdoModeList_t tVdoModeList, tVdoModeHL;

	if(UI_DISPLAY_STATE != tUI_State)
		return;

	tVdoModeHL = (UI_PHOTOCAP_MODE == tUI_CuSetting.tVdoMode)?UI_VDOPHOTO_MODE:
	             (REC_LOOPING == tUI_CuSetting.RecInfo.tREC_Mode)?UI_VDORECLOOP_MODE:
	             (REC_MANUAL  == tUI_CuSetting.RecInfo.tREC_Mode)?UI_VDORECMANU_MODE:UI_VDORECTRIG_MODE;
	for(tVdoModeList = UI_VDORECLOOP_MODE; tVdoModeList < UI_VDOMODELIST_MAX; tVdoModeList++)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, (OSD2IMG_RECLOOPMODENOR_ICON+(tVdoModeList*2)+((tVdoModeList == tVdoModeHL)?UI_ICON_HIGHLIGHT:0)), 1, &tOsdImgInfo);
		tOSD_Img2(&tOsdImgInfo, ((tVdoModeList+1) == UI_VDOMODELIST_MAX)?OSD_UPDATE:OSD_QUEUE);
	}
	tUI_State = UI_SET_VDOMODE_STATE;
}
//------------------------------------------------------------------------------
void UI_RightArrowLongKey(void)
{
	if(UI_DISPLAY_STATE != tUI_State)
		return;
	if(TRUE == ubUI_ShowTimeFlag)
	{
		OSD_IMG_INFO tOsdImgInfo;

		ubUI_ShowTimeFlag = FALSE;
		tOsdImgInfo.uwXStart = 670;
		tOsdImgInfo.uwYStart = 980;
		tOsdImgInfo.uwHSize  = 50;
		tOsdImgInfo.uwVSize  = 300;
		OSD_EraserImg2(&tOsdImgInfo);
		return;
	}
	ubUI_ShowTimeFlag = TRUE;
}
//------------------------------------------------------------------------------
void UI_ShowSysTime(void)
{
	static OSD_IMG_INFO tOsdImgInfo[11];
	static uint8_t ubUI_SysTimeFlag = FALSE;
	uint16_t uwYOffset = uwLCD_GetLcdVoSize();
	uint8_t *pDT_Num[3];
	uint8_t ubTen = 0, ubUnit = 0, ubSysTimeIdx = 0;
	RTC_Calendar_t tSysCalendar;

	if(FALSE == ubUI_SysTimeFlag)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_CK_NUM0, 11, &tOsdImgInfo[0]);
		ubUI_SysTimeFlag = TRUE;
	}
	RTC_GetCalendar((RTC_Calendar_t *)(&tSysCalendar));
	printd(DBG_Debug2Lvl, "Time: %02d:%02d:%02d\n", tSysCalendar.ubHour, tSysCalendar.ubMin, tSysCalendar.ubSec);
	if(tUI_CuSetting.tSysCalendar.ubSec == tSysCalendar.ubSec)
		return;
	pDT_Num[0] = (uint8_t *)(&tSysCalendar.ubHour);
	pDT_Num[1] = (uint8_t *)(&tSysCalendar.ubMin);
	pDT_Num[2] = (uint8_t *)(&tSysCalendar.ubSec);
	for(ubSysTimeIdx = 0; ubSysTimeIdx < 3; ubSysTimeIdx++)
	{
		ubTen  = *pDT_Num[ubSysTimeIdx] / 10;
		ubUnit = *pDT_Num[ubSysTimeIdx] - (ubTen * 10);
		tOsdImgInfo[ubTen].uwXStart  = 680;
		tOsdImgInfo[ubTen].uwYStart  = uwYOffset - tOsdImgInfo[ubTen].uwVSize;
		tOSD_Img2(&tOsdImgInfo[ubTen], OSD_QUEUE);
		tOsdImgInfo[ubUnit].uwXStart = 680;
		tOsdImgInfo[ubUnit].uwYStart = tOsdImgInfo[ubTen].uwYStart - tOsdImgInfo[ubUnit].uwVSize;
		tOSD_Img2(&tOsdImgInfo[ubUnit], (ubSysTimeIdx == 2)?OSD_UPDATE:OSD_QUEUE);
		if(ubSysTimeIdx != 2)
		{
			tOsdImgInfo[10].uwXStart = 680;
			tOsdImgInfo[10].uwYStart = tOsdImgInfo[ubUnit].uwYStart - tOsdImgInfo[10].uwVSize;
			tOSD_Img2(&tOsdImgInfo[10], OSD_QUEUE);
			uwYOffset = tOsdImgInfo[10].uwYStart;
		}
		tOsdImgInfo[ubTen].uwXStart  = 0;
		tOsdImgInfo[ubTen].uwYStart  = 0;
		tOsdImgInfo[ubUnit].uwXStart = 0;
		tOsdImgInfo[ubUnit].uwYStart = 0;
		tOsdImgInfo[10].uwXStart	 = 0;
		tOsdImgInfo[10].uwYStart	 = 0;
	}
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

//	if(DISPLAY_1T1R != tUI_CuSetting.ubTotalCamNum)
//	{
//		tUI_CuSetting.tPsMode = POWER_NORMAL_MODE;
//		UI_UpdateDevStatusInfo();
//		return;
//	}

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
#if (defined(BUC_CU) && APP_DUAL_HOST_ENABLE)	
	for(tCamNum = CAM1; tCamNum < KNL_WIRELESS_CAM_NUM; tCamNum++)
#else
	for(tCamNum = CAM1; tCamNum < tUI_CuSetting.ubTotalCamNum; tCamNum++)
#endif
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
	if((APP_LINK_STATE == tUI_SyncAppState) &&
	   (UI_RECORDING_MODE == tUI_CuSetting.tVdoMode) && (REC_LOOPING == tUI_CuSetting.RecInfo.tREC_Mode ))
		UI_VideoRecordingExec(UI_REC_START);
	tUI_State = UI_DISPLAY_STATE;
}
//------------------------------------------------------------------------------
void UI_DisableCuAdoOnlyMode(void)
{
	APP_EventMsg_t tUI_PsMessage = {0};

	UI_DisableScanMode();
	UI_ClearStatusBarOsdIcon();
	UI_ClearCamConnectStatusFlag();
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
		case SCAN_VIEW:
			if(tCamViewSel.tCamViewPool[0] != tUI_CuSetting.tAdoSrcCamNum)
				UI_SwitchAudioSource(tCamViewSel.tCamViewPool[0]);
			if(SINGLE_VIEW == tCamViewSel.tCamViewType)
				break;
			UI_EnableScanMode();
			break;
		default:
			break;
	}
	if((APP_LINK_STATE == tUI_SyncAppState) &&
	   (UI_RECORDING_MODE == tUI_CuSetting.tVdoMode) && (REC_LOOPING == tUI_CuSetting.RecInfo.tREC_Mode ))
		UI_VideoRecordingExec(UI_REC_START);
}
//------------------------------------------------------------------------------
void UI_MDTrigger(UI_CamNum_t tCamNum, void *pvTrig)
{
	OSD_IMG_INFO tOsdImgInfo;
	UI_DisplayLocation_t tUI_DispLoc;
	uint32_t ulLcd_HSize  = uwLCD_GetLcdHoSize();
	uint32_t ulLcd_VSize  = uwLCD_GetLcdVoSize();
	uint16_t uwXOffset[7] = {[DISP_UPPER_LEFT] = 0,               [DISP_UPPER_RIGHT] = 0,
	                         [DISP_LOWER_LEFT] = (ulLcd_HSize/2), [DISP_LOWER_RIGHT] = (ulLcd_HSize/2),
							 [DISP_LEFT] 	   = 0,				  [DISP_RIGHT] 		 = 0};
	uint16_t uwYOffset[7] = {[DISP_UPPER_LEFT] = 0,               [DISP_UPPER_RIGHT] = (ulLcd_VSize/2),
	                         [DISP_LOWER_LEFT] = 0, 			  [DISP_LOWER_RIGHT] = (ulLcd_VSize/2),
							 [DISP_LEFT] 	   = 0, 		      [DISP_RIGHT] 		 = (ulLcd_VSize/2)};

	if(tCamNum > CAM4)
		return;

	tUI_DispLoc = ((tCamViewSel.tCamViewType == SINGLE_VIEW) || (tCamViewSel.tCamViewType == SCAN_VIEW))?DISP_UPPER_LEFT:tUI_CamStatus[tCamNum].tCamDispLocation;
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_MDTRIG_ICON, 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart += uwXOffset[tUI_DispLoc];
	tOsdImgInfo.uwYStart -= uwYOffset[tUI_DispLoc];
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
	tUI_CuSetting.IconSts.ubDrawMdTrigFlag = TRUE;
}
//------------------------------------------------------------------------------
void UI_VoiceTrigger(UI_CamNum_t tCamNum, void *pvTrig)
{
	if(DISPLAY_1T1R == tUI_CuSetting.ubTotalCamNum)
		return;

	if(SCAN_VIEW == tCamViewSel.tCamViewType)
		UI_DisableScanMode();
	if(PS_VOX_MODE == tUI_CuSetting.tPsMode)
	{
		UI_DisableVox();
		osDelay(50);
	}
	tUI_CuSetting.IconSts.ubDrawStsIconFlag = TRUE;
	UI_ClearStatusBarOsdIcon();
	UI_ClearCamConnectStatusFlag();
	tUI_CamNumSel = tCamViewSel.tCamViewPool[0];
	tCamViewSel.tCamViewType = SINGLE_VIEW;
	tCamViewSel.tCamViewPool[0] = tCamNum;
	UI_SwitchCameraSource();	
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
	ulUI_MonitorPsFlag[tCamNum] = FALSE;
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
}
//------------------------------------------------------------------------------
void UI_LoadDevStatusInfo(void)
{
	uint32_t ulUI_SFAddr = pSF_Info->ulSize - (UI_SF_START_SECTOR * pSF_Info->ulSecSize);
	UI_DeviceStatusInfo_t tUI_DevStsInfo = {{0}, {0}, {0}, {0}};
	UI_CamNum_t tCamNum;

	SF_Read(ulUI_SFAddr, sizeof(UI_DeviceStatusInfo_t), (uint8_t *)&tUI_DevStsInfo);


#if ((APP_DUAL_HOST_ENABLE == 1) && defined(OP_AP))	
	tUI_DevStsInfo.tCAM_StatusInfo[1].ulCAM_ID = 0;	//for UVC Device1
	tUI_DevStsInfo.tCAM_StatusInfo[2].ulCAM_ID = 0;	//for UVC Device2	
#endif
	
	memcpy(tUI_CamStatus, tUI_DevStsInfo.tCAM_StatusInfo, (CAM_4T * sizeof(UI_CamStatus_t)));
	memcpy(&tUI_CuSetting, &tUI_DevStsInfo.tCU_SettingInfo, sizeof(UI_CUSetting_t));
	printd(DBG_InfoLvl, "UI TAG:%s\n",tUI_DevStsInfo.cbUI_DevStsTag);
	printd(DBG_InfoLvl, "UI VER:%s\n",tUI_DevStsInfo.cbUI_FwVersion);
	if((strncmp(tUI_DevStsInfo.cbUI_DevStsTag, SF_AP_UI_SECTOR_TAG, sizeof(tUI_DevStsInfo.cbUI_DevStsTag) - 1)) ||
	   (strncmp(tUI_DevStsInfo.cbUI_FwVersion, SN937XX_FW_VERSION,  sizeof(tUI_DevStsInfo.cbUI_FwVersion) - 1)))
	{
		printd(DBG_ErrorLvl, "UI Default Setting !\n");
		for(tCamNum = CAM1; tCamNum < CAM_4T; tCamNum++)
			memset(&tUI_CamStatus[tCamNum], 0xFF, sizeof(UI_CamStatus_t));
		memset(&tUI_CuSetting, 0xFF, sizeof(UI_CUSetting_t));
		UI_CLEAR_CALENDAR_TODEFU(tUI_CuSetting.tSysCalendar.uwYear, 2018);
		UI_CLEAR_CALENDAR_TODEFU(tUI_CuSetting.tSysCalendar.ubMonth,   1);
		UI_CLEAR_CALENDAR_TODEFU(tUI_CuSetting.tSysCalendar.ubDate,    1);
		UI_CLEAR_CALENDAR_TODEFU(tUI_CuSetting.tSysCalendar.ubHour,    0);
		UI_CLEAR_CALENDAR_TODEFU(tUI_CuSetting.tSysCalendar.ubMin,     0);
		UI_CLEAR_CALENDAR_TODEFU(tUI_CuSetting.tSysCalendar.ubSec,     0);
		UI_CLEAR_CALENDAR_TODEFU(tUI_CuSetting.RecInfo.tREC_Mode, REC_OFF);
		UI_CLEAR_CALENDAR_TODEFU(tUI_CuSetting.RecInfo.tREC_Time, RECTIME_5MIN);
        REC_TimeSet(0,300);
		tUI_CuSetting.tVdoMode = UI_PHOTOCAP_MODE;
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
	tUI_CuSetting.ubPairedCamNum				 = 0;
	UI_CHK_CUSYS(tUI_CuSetting.ubAEC_Mode, AECFUNC_MAX, AECFUNC_OFF);
	UI_CHK_CUSYS(tUI_CuSetting.ubCCA_Mode, CCAMODE_MAX, CCAFUNC_OFF);
	UI_CHK_CUSYS(tUI_CuSetting.tPsMode, POWER_NORMAL_MODE, POWER_NORMAL_MODE);
	UI_CHK_CUSYS(tUI_CuSetting.RecInfo.tREC_Mode, REC_RECMODE_MAX, REC_OFF);
	UI_CHK_CUSYS(tUI_CuSetting.RecInfo.tREC_Time, RECTIME_MAX, RECTIME_5MIN);
	UI_CHK_CUSYS(tUI_CuSetting.tVdoMode, UI_VDOMODE_MAX, UI_PHOTOCAP_MODE);
	if(UI_PHOTOCAP_MODE == tUI_CuSetting.tVdoMode)
		tUI_CuSetting.RecInfo.tREC_Mode = REC_OFF;

	if(PS_WOR_MODE == tUI_CuSetting.tPsMode)
	{
		ulUI_LogoIndex = OSDLOGO_WORBOOT;
		tUI_CuSetting.tPsMode = POWER_NORMAL_MODE;
		UI_UpdateDevStatusInfo();
	}

    if(tUI_CuSetting.RecInfo.tREC_Time == RECTIME_1MIN)
        REC_TimeSet(0,60);
    else if(tUI_CuSetting.RecInfo.tREC_Time == RECTIME_3MIN)
        REC_TimeSet(0,180);
    else if(tUI_CuSetting.RecInfo.tREC_Time == RECTIME_5MIN)
        REC_TimeSet(0,300);

	for(tCamNum = CAM1; tCamNum < CAM_4T; tCamNum++)
	{
		if ((strncmp(tUI_DevStsInfo.cbUI_DevStsTag, SF_AP_UI_SECTOR_TAG, sizeof(tUI_DevStsInfo.cbUI_DevStsTag) - 1) == 0)
		&& (strncmp(tUI_DevStsInfo.cbUI_FwVersion, SN937XX_FW_VERSION, sizeof(tUI_DevStsInfo.cbUI_FwVersion) - 1) == 0)) {
			ulUI_MonitorPsFlag[tCamNum]   = FALSE;
			tUI_CamStatus[tCamNum].tCamConnSts = CAM_ONLINE;
			if(tCamNum >= tUI_CuSetting.ubTotalCamNum)
				tUI_CamStatus[tCamNum].ulCAM_ID = INVALID_ID;
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
		} else {
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
		UI_TimerEventStart(10000, UI_ScanModeTimerEvent);
	else
		UI_TimerEventStop();
}
//------------------------------------------------------------------------------
void UI_EnableScanMode(void)
{
	if(PS_ADOONLY_MODE != tUI_CuSetting.tPsMode)
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

	tSearchCam = (PS_ADOONLY_MODE == tUI_CuSetting.tPsMode)?tUI_CuSetting.tAdoSrcCamNum:tCamViewSel.tCamViewPool[0];
	for(ubSearchCnt = 0; ubSearchCnt < tUI_CuSetting.ubTotalCamNum; ubSearchCnt++)
	{
		tSearchCam = ((tSearchCam + 1) >= ((DISPLAY_2T1R == tUI_CuSetting.ubTotalCamNum)?CAM_2T:CAM_4T))?CAM1:((UI_CamNum_t)(tSearchCam + 1));
		if((tUI_CamStatus[tSearchCam].ulCAM_ID != INVALID_ID) &&
		   (tUI_CamStatus[tSearchCam].tCamConnSts == CAM_ONLINE) &&
		   (tUI_CamStatus[tSearchCam].tCamPsMode == POWER_NORMAL_MODE))
		{
			if(PS_ADOONLY_MODE == tUI_CuSetting.tPsMode)
			{
				UI_SwitchAudioSource(tSearchCam);
				break;
			}
			else if(tSearchCam != tCamViewSel.tCamViewPool[0])
			{
				tCamViewSel.tCamViewType	= SCAN_VIEW;
				tCamViewSel.tCamViewPool[0] = tSearchCam;
				UI_SwitchCameraSource();
				UI_ClearCamConnectStatusFlag();
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

	if((tUI_CamStatus[tCamViewSel.tCamViewPool[0]].ulCAM_ID != INVALID_ID) &&
	   (tUI_CamStatus[tCamViewSel.tCamViewPool[0]].tCamConnSts == CAM_ONLINE))
		return rUI_SUCCESS;

	for(tCamViewNum = CAM1; tCamViewNum < tUI_CuSetting.ubTotalCamNum; tCamViewNum++)
	{
		if((tUI_CamStatus[tCamViewNum].ulCAM_ID != INVALID_ID) &&
	       (tUI_CamStatus[tCamViewNum].tCamConnSts == CAM_ONLINE))
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
	if ((tCamViewSel.tCamViewType == SINGLE_VIEW) || (tCamViewSel.tCamViewType == SCAN_VIEW))
	{		
	#if (defined(BUC_CU) && APP_DUAL_HOST_ENABLE)
		if(KNL_WIRELESS_CAM_NUM == 1)
			tUI_CuSetting.tAdoSrcCamNum = CAM1;
	#else
		tUI_CuSetting.tAdoSrcCamNum = tCamViewSel.tCamViewPool[0];
	#endif
	}
#ifdef RTC676x
	UI_RemoveLostLinkLogo();
#endif
	RTC_WriteUserRam(RTC_RECORD_VIEW_MODE_ADDR, tCamViewSel.tCamViewType);	
	RTC_WriteUserRam(RTC_RECORD_VIEW_CAM_ADDR, (tCamViewSel.tCamViewPool[0] << 4) | tCamViewSel.tCamViewPool[1]);
}
//------------------------------------------------------------------------------
void UI_SwitchAudioSource(UI_CamNum_t tCamNum)
{
	APP_EventMsg_t tUI_SwitchAdoSrcMsg = {0};

	tUI_CuSetting.tAdoSrcCamNum			 = tCamNum;
	tUI_SwitchAdoSrcMsg.ubAPP_Event 	 = APP_ADOSRCSEL_EVENT;
	tUI_SwitchAdoSrcMsg.ubAPP_Message[0] = 1;		//! Message Length
	tUI_SwitchAdoSrcMsg.ubAPP_Message[1] = tUI_CuSetting.tAdoSrcCamNum;
	tUI_SwitchAdoSrcMsg.ubAPP_Message[2] = (PS_ADOONLY_MODE == tUI_CuSetting.tPsMode)?FALSE:TRUE;
	UI_SendMessageToAPP(&tUI_SwitchAdoSrcMsg);
}
//------------------------------------------------------------------------------
void UI_EngModeKey(void)
{
#ifdef A7130
	OSD_IMG_INFO tOsdImgInfo;
	OSD_IMGIDXARRARY_t tUI_EngOsdImg;
	uint16_t uwUI_NumArray[10];
	uint16_t uwUI_UpperLetterArray[26];
	uint16_t uwUI_LowerLetterArray[26];
	uint16_t uwUI_SymbolArray[4];
	uint8_t i;

	if(UI_DISPLAY_STATE != tUI_State)
		return;
	tOsdImgInfo.uwXStart = 670;
	tOsdImgInfo.uwYStart = 980;
	tOsdImgInfo.uwHSize  = 50;
	tOsdImgInfo.uwVSize  = 300;
	OSD_EraserImg2(&tOsdImgInfo);
	tOsdImgInfo.uwXStart = 0;
	tOsdImgInfo.uwYStart = 0;
	tOsdImgInfo.uwHSize  = 40;
	tOsdImgInfo.uwVSize  = uwLCD_GetLcdVoSize();
	OSD_EraserImg1(&tOsdImgInfo);
	for(i = 0; i < 10; i++)
		uwUI_NumArray[i] = OSD2IMG_ENG_D0 + i;
	tUI_EngOsdImg.pNumImgIdxArray = uwUI_NumArray;
	for(i = 0; i < 26; i++)
	{
		uwUI_UpperLetterArray[i] = OSD2IMG_ENG_UPA + i;
		uwUI_LowerLetterArray[i] = OSD2IMG_ENG_LWA + i;
	}
	tUI_EngOsdImg.pUpperLetterImgIdxArray = uwUI_UpperLetterArray;
	tUI_EngOsdImg.pLowerLetterImgIdxArray = uwUI_LowerLetterArray;
	for(i = 0; i < 4; i++)
		uwUI_SymbolArray[i] = OSD2IMG_ENG_COLONSYM + i;
	tUI_EngOsdImg.pSymbolImgIdxArray = uwUI_SymbolArray;
	EN_SetupOsdImgRotate(OSD_IMG_ROTATION_90);
	EN_SetupOsdImgInfo(&tUI_EngOsdImg);
	EN_OpenEnMode(TRUE);
	tUI_State = UI_ENGMODE_STATE;
#endif
}
//------------------------------------------------------------------------------
void UI_EngModeCtrl(UI_ArrowKey_t tArrowKey)
{
#ifdef A7130
	pvUiFuncPtr UI_EngFuncPrt[] = {[UP_ARROW] 	 = EN_UpKey,
								   [DOWN_ARROW]	 = EN_DownKey,
								   NULL, NULL,
								   [ENTER_ARROW] = EN_EnterKey,
								   NULL};
	switch(tArrowKey)
	{
		case EXIT_ARROW:
		{
			EN_OpenEnMode(FALSE);
			UI_ClearCamConnectStatusFlag();
			tUI_State = UI_DISPLAY_STATE;
			break;
		}
		default:
			if(UI_EngFuncPrt[tArrowKey])
				UI_EngFuncPrt[tArrowKey]();
			break;
	}
#endif
}
//------------------------------------------------------------------------------
void UI_DisplayAppPairingScreen(void)
{
	OSD_IMG_INFO tOsdImgInfo;

	ubUI_StopUpdateStsBarFlag = TRUE;
	tOsdImgInfo.uwHSize  = uwOSD_GetHSize();
	tOsdImgInfo.uwVSize  = uwOSD_GetVSize();
	tOsdImgInfo.uwXStart = 0;
	tOsdImgInfo.uwYStart = 0;
	OSD_EraserImg1(&tOsdImgInfo);
	OSD_Weight(OSD_WEIGHT_8DIV8);
	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_APPPAIRBG_ICON, 1, &tOsdImgInfo);
	tOSD_Img1(&tOsdImgInfo, OSD_UPDATE);
	tUI_State = UI_PAIRING_STATE;
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
	OSD_IMG_INFO tOsdImgInfo[6];

	if(UI_DISPLAY_STATE != tUI_State)
		return;

	if(tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_FWUSDUPGDIAG_ICON, 5, &tOsdImgInfo[0]) != OSD_OK)
	{
		printd(DBG_ErrorLvl, "Load OSD Image FAIL, pls check (%d) !\n", __LINE__);
		return;
	}
	tOsdImgInfo[5].uwHSize  = uwOSD_GetHSize();
	tOsdImgInfo[5].uwVSize  = uwOSD_GetVSize();
	tOsdImgInfo[5].uwXStart = 0;
	tOsdImgInfo[5].uwYStart = 0;
	OSD_EraserImg1(&tOsdImgInfo[5]);
	OSD_Weight(OSD_WEIGHT_8DIV8);
	tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);
	tOSD_Img2(&tOsdImgInfo[1], OSD_QUEUE);
	tOSD_Img2(&tOsdImgInfo[4], OSD_UPDATE);
	tUI_State = UI_SDFWUPG_STATE;
}
//------------------------------------------------------------------------------
void UI_FwUpgExecSel(UI_ArrowKey_t tArrowKey)
{
	static uint8_t ubUI_FwuSelIdx = TRUE;
	uint8_t ubPrevFwuSelIdx = TRUE;
	uint16_t uwFwuSelImgIdx[2] = {OSD2IMG_FWUSDUPGNONOR_ICON, OSD2IMG_FWUSDUPGYESNOR_ICON};
	switch(tArrowKey)
	{
		case LEFT_ARROW:
			if(TRUE == ubUI_FwuSelIdx)
				return;
			ubPrevFwuSelIdx = ubUI_FwuSelIdx;
			ubUI_FwuSelIdx = TRUE;
			break;
		case RIGHT_ARROW:
			if(FALSE == ubUI_FwuSelIdx)
				return;
			ubPrevFwuSelIdx = ubUI_FwuSelIdx;
			ubUI_FwuSelIdx = FALSE;
			break;
		case ENTER_ARROW:
			if(TRUE == ubUI_FwuSelIdx)
			{
				UI_Event_t tSdFwUpgEvent;
				osMessageQId *pUI_EventQH = NULL;

				tSdFwUpgEvent.tEventType = FWUPG_EVENT;
				tSdFwUpgEvent.pvEvent  	 = NULL;
				pUI_EventQH 	  	 	 = pUI_GetEventQueueHandle();
				osMessagePut(*pUI_EventQH, &tSdFwUpgEvent, 0);
				ubUI_FwuSelIdx = TRUE;
				tUI_State = UI_DISPLAY_STATE;
				return;
			}
		case EXIT_ARROW:
		{
			OSD_IMG_INFO tOsdImgInfo;

			tUI_CuSetting.IconSts.ubDrawStsIconFlag = TRUE;
			UI_ClearStatusBarOsdIcon();
			UI_ClearCamConnectStatusFlag();
			OSD_Weight(OSD_WEIGHT_6DIV8);
			tOsdImgInfo.uwHSize  = uwOSD_GetHSize();
			tOsdImgInfo.uwVSize  = uwOSD_GetVSize();
			tOsdImgInfo.uwXStart = 0;
			tOsdImgInfo.uwYStart = 0;
			OSD_EraserImg1(&tOsdImgInfo);
			ubUI_FwuSelIdx = TRUE;
			tUI_State = UI_DISPLAY_STATE;
			return;
		}
		default:
			return;
	}
	UI_DrawHLandNormalIcon(uwFwuSelImgIdx[ubPrevFwuSelIdx], (uwFwuSelImgIdx[ubUI_FwuSelIdx]+UI_ICON_HIGHLIGHT));
}
//------------------------------------------------------------------------------
void UI_UpdateRecStsIcon(void)
{
	if((UI_SHOWSTSICON_STATE != tUI_State) ||
	   (!tUI_CuSetting.ubVdoRecStsCnt))
		return;
	if(!(--tUI_CuSetting.ubVdoRecStsCnt))
	{
		OSD_IMG_INFO tOsdImgInfo;

		UI_ClearCamConnectStatusFlag();
		tOsdImgInfo.uwXStart = 100;
		tOsdImgInfo.uwYStart = 0;
		tOsdImgInfo.uwHSize  = 190;
		tOsdImgInfo.uwVSize  = 300;
		OSD_EraserImg2(&tOsdImgInfo);
		tUI_State = UI_DISPLAY_STATE;
	}
}
//------------------------------------------------------------------------------
void UI_UpdateWarningNoteIcon(void)
{
	if((UI_SHOWSTSICON_STATE != tUI_State) ||
	   (!tUI_CuSetting.WarnIcon.ubWarnUpdateCnt))
		return;
	if(!(--tUI_CuSetting.WarnIcon.ubWarnUpdateCnt))
	{
		OSD_IMG_INFO tOsdImgInfo;

		UI_ClearCamConnectStatusFlag();
		tOsdImgInfo.uwXStart = 160;
		tOsdImgInfo.uwYStart = 100;
		tOsdImgInfo.uwHSize  = 150;
		tOsdImgInfo.uwVSize  = 800;
		OSD_EraserImg2(&tOsdImgInfo);
		tUI_State = UI_DISPLAY_STATE;
	}
}
//------------------------------------------------------------------------------
void UI_SdCardFormatMenu(UI_ArrowKey_t tArrowKey)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint16_t uwSdfSelImgIdx[2] = {OSD2IMG_SDFNONOR_ICON, OSD2IMG_SDFYESNOR_ICON};
	static uint8_t ubUI_SdfItmSel = FALSE;
	uint8_t ubPrevSdfSel;
	FS_FMT_STATUS tSDF_Ret = FORMAT_FAIL;

	switch(tArrowKey)
	{
		case LEFT_ARROW:
			if(TRUE == ubUI_SdfItmSel)
				return;
			ubPrevSdfSel = ubUI_SdfItmSel;
			ubUI_SdfItmSel = TRUE;
			break;
		case RIGHT_ARROW:
			if(FALSE == ubUI_SdfItmSel)
				return;
			ubPrevSdfSel = ubUI_SdfItmSel;
			ubUI_SdfItmSel = FALSE;
			break;
		case ENTER_ARROW:
			if(TRUE == ubUI_SdfItmSel)
            {
				OSD_IMG_INFO tSdfOsdImgInfo[3];
				UI_FuncExecMsg_t tWorkAct;
				uint8_t ubSdFmtRet;

				tWorkAct.uwFunc = UI_SDCARDFMT_ACT;
				osMessagePut(osUI_FuncsExecQue, &tWorkAct, 0);
				osMessageGet(osUI_FuncsFinExecQue, &ubSdFmtRet, osWaitForever);
				tSDF_Ret = (rUI_SUCCESS == ubSdFmtRet)?FORMAT_OK:FORMAT_FAIL;
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SDFRET_BG, 3, &tSdfOsdImgInfo);
				tSdfOsdImgInfo[0].uwXStart = 260;
				tSdfOsdImgInfo[0].uwYStart = 473;
				tSdfOsdImgInfo[1+tSDF_Ret].uwXStart = 336;
				tSdfOsdImgInfo[1+tSDF_Ret].uwYStart = 499;
				tOSD_Img2(&tSdfOsdImgInfo[0], OSD_QUEUE);
				tOSD_Img2(&tSdfOsdImgInfo[1+tSDF_Ret], OSD_UPDATE);
				osDelay(1000);
            }
		case EXIT_ARROW:
			UI_ClearCamConnectStatusFlag();
			OSD_Weight(OSD_WEIGHT_6DIV8);
			tOsdImgInfo.uwHSize  = 300;
			tOsdImgInfo.uwVSize  = 600;
			tOsdImgInfo.uwXStart = 220;
			tOsdImgInfo.uwYStart = 400;
			OSD_EraserImg2(&tOsdImgInfo);
			if(UI_REC_START == tUI_RecPlayAct.tRecAct)
			{
				tUI_RecPlayAct.tRecAct = UI_REC_STOP;
				if((TRUE == ubUI_SdfItmSel) && (FORMAT_OK == tSDF_Ret))
					UI_VideoRecordingExec(UI_REC_START);
			}
			ubUI_SdfItmSel = FALSE;
			tUI_State = UI_DISPLAY_STATE;
			return;
		default:
			return;
	}
	tOSD_GetOsdImgInfor(1, OSD_IMG2, uwSdfSelImgIdx[ubPrevSdfSel], 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart = 405;
	tOsdImgInfo.uwYStart = (TRUE == ubPrevSdfSel)?670:470;
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	tOSD_GetOsdImgInfor(1, OSD_IMG2, (uwSdfSelImgIdx[ubUI_SdfItmSel]+UI_ICON_HIGHLIGHT), 1, &tOsdImgInfo);
	tOsdImgInfo.uwXStart = 405;
	tOsdImgInfo.uwYStart = (TRUE == ubUI_SdfItmSel)?670:470;
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
}
//------------------------------------------------------------------------------
void UI_SdCardFormatFunc(UI_ArrowKey_t tArrowKey)
{
	switch(tUI_SdCardSts)
	{
		case UI_SD_NRDY:
			UI_SdCardFormatMenu(tArrowKey);
			break;
		case UI_SD_CFM:
			UI_RecordSDFSubSubMenu(tArrowKey);
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_SdCardFsFmtErrMenu(void)
{
	OSD_IMG_INFO tOsdImgInfo[6];

	tUI_CuSetting.IconSts.ubDrawStsIconFlag = TRUE;
	UI_ClearStatusBarOsdIcon();
	tUI_State = UI_SDCARDFMT_STATE;
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SDFBG_ICON, 6, &tOsdImgInfo);
	tOsdImgInfo[0].uwXStart = 230;
	tOsdImgInfo[0].uwYStart = 416;
	tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);
	tOsdImgInfo[2].uwXStart = 405;
	tOsdImgInfo[2].uwYStart = 470;
	tOSD_Img2(&tOsdImgInfo[2], OSD_QUEUE);
	OSD_Weight(OSD_WEIGHT_8DIV8);
	tOsdImgInfo[3].uwXStart = 405;
	tOsdImgInfo[3].uwYStart = 670;
	tOSD_Img2(&tOsdImgInfo[3], OSD_UPDATE);
	tUI_SdCardSts = UI_SD_NRDY;
}
//------------------------------------------------------------------------------
void UI_PhotoCaptureFinish(uint8_t ubPhotoCapRet)
{
	OSD_IMG_INFO tOsdImgInfo;
	KNL_Status_t tPhotoCapSts = (KNL_Status_t)ubPhotoCapRet;

	if(KNL_ErrorFsFmt == tPhotoCapSts)
	{
		UI_SdCardFsFmtErrMenu();
		return;
	}
	tOSD_GetOsdImgInfor(1, OSD_IMG2, (OSD2IMG_PHOTOCAPOK_ICON + tPhotoCapSts), 1, &tOsdImgInfo);
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
	tUI_CuSetting.ubVdoRecStsCnt = UI_PHOTOGRAPHSTS_PERIOD;
	tUI_State = UI_SHOWSTSICON_STATE;
}
//------------------------------------------------------------------------------
void UI_PhotoPlayFinish(uint8_t ubPhtoCapRet)
{
	if(!ubPhtoCapRet)
	{
		OSD_IMG_INFO tOsdImgInfo;

		tOsdImgInfo.uwHSize  = uwOSD_GetHSize();
		tOsdImgInfo.uwVSize  = uwOSD_GetVSize();
		tOsdImgInfo.uwXStart = 0;
		tOsdImgInfo.uwYStart = 0;
		OSD_EraserImg1(&tOsdImgInfo);
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PHOTOPLAY_ICON, 1, &tOsdImgInfo);
		tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
		tUI_State = UI_PHOTOPLAYLIST_STATE;
	}
	else
	{
		printd(DBG_ErrorLvl, "Play Err !\n");
		tUI_State = UI_RECFILES_SEL_STATE;
	}
}
//------------------------------------------------------------------------------
void UI_VideoRecordingStsRpt(uint8_t ubRecSts)
{
	switch((KNL_Status_t)ubRecSts)
	{
		case KNL_ErrorNoCard:
        case KNL_ErrorTimeout:
		case KNL_ErrorCardNRdy:
		{
            uint8_t ubRecRpt = ubRecSts;
			OSD_IMG_INFO tOsdImgInfo;

			if(UI_DISPLAY_STATE != tUI_State)
				break;
			if((UI_SUBSUBMENU_STATE == tUI_State) || (UI_SUBMENU_STATE == tUI_State))
			{
				tOsdImgInfo.uwHSize  = uwOSD_GetHSize();
				tOsdImgInfo.uwVSize  = uwOSD_GetVSize();
				tOsdImgInfo.uwXStart = 0;
				tOsdImgInfo.uwYStart = 0;
				OSD_EraserImg1(&tOsdImgInfo);
				osDelay(100);
			}
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PHOTOCAPNOCARD_ICON, 1, &tOsdImgInfo);
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			tUI_CuSetting.ubVdoRecStsCnt = UI_RECGRAPHSTS_PERIOD;
			tUI_State = UI_SHOWSTSICON_STATE;
			if(UI_REC_START == tUI_RecPlayAct.tRecAct)
			{
                osMessagePut(osUI_RecRptQueue, &ubRecRpt, 0);
				UI_ClearRecordStatusOsdImg();
			}
			break;
		}
		case KNL_ErrorFsFmt:
			if((UI_SUBSUBMENU_STATE == tUI_State) || (UI_SUBMENU_STATE == tUI_State))
			{
				OSD_IMG_INFO tOsdImgInfo;

				tOsdImgInfo.uwHSize  = uwOSD_GetHSize();
				tOsdImgInfo.uwVSize  = uwOSD_GetVSize();
				tOsdImgInfo.uwXStart = 0;
				tOsdImgInfo.uwYStart = 0;
				OSD_EraserImg1(&tOsdImgInfo);
				osDelay(100);
			}
			UI_SdCardFsFmtErrMenu();
			break;
        case KNL_VDOREC_STOP_TIMEOUT: 
        {
			uint8_t ubRecRpt = ubRecSts;
            tUI_RecPlayAct.tRecAct = UI_REC_STOP;
			osMessagePut(osUI_RecRptQueue, &ubRecRpt, 0);
            break;
		}
		case KNL_VDOREC_STOP:
		{
			uint8_t ubRecRpt = ubRecSts;
            tUI_RecPlayAct.tRecAct = UI_REC_STOP;
			osMessagePut(osUI_RecRptQueue, &ubRecRpt, 0);
			break;
		}
		case KNL_VDOREC_START:
		{
			uint16_t uwLdState = UI_LD_DEFU;

			tUI_RecPlayAct.tRecAct = UI_REC_START;
			if(UI_RECORDING_MODE != tUI_CuSetting.tVdoMode)
			{
				ubUI_VdoRecChkFlag = FALSE;
				UI_VideoRecordingExec(UI_REC_STOP);
				break;
			}
			osMessagePut(osUI_OsdLdDispQueue, &uwLdState, 0);
			break;
		}
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_VideoRecordingExec(UI_RecordingAct_t tRecAct)
{
	KNL_RecordAct_t tUI_RecAct = {KNL_RECORDFUNC_DISABLE, NULL,};
	KNL_Status_t tUI_RecSts;

	if((TRUE == ubUI_VdoRecChkFlag) &&
	   ((tUI_RecPlayAct.tRecAct == tRecAct) || (UI_PHOTOCAP_MODE == tUI_CuSetting.tVdoMode)))
		return;

	tUI_RecAct.tRecordFunc     = ((REC_LOOPING == tUI_CuSetting.RecInfo.tREC_Mode) && (UI_REC_START == tRecAct))?KNL_RECORDFUNC_LOOP:KNL_RECORDFUNC_MANU;
	tUI_RecAct.pRecordStsNtyCb = UI_VideoRecordingStsRpt;
    tUI_RecAct.ubRecOnceStopCBFlag = NULL;
	tUI_RecSts = tKNL_ExecRecordFunc(tUI_RecAct);
	if(KNL_ERR != tUI_RecSts)
	{
        tUI_RecPlayAct.tRecAct = tRecAct;
		if(UI_REC_STOP == tUI_RecPlayAct.tRecAct)
		{
			uint8_t ubRecRpt;
            osMessageReset(osUI_RecRptQueue);
			osMessageGet(osUI_RecRptQueue, &ubRecRpt, osWaitForever);
		}
	}
	ubUI_VdoRecChkFlag = TRUE;
}
//------------------------------------------------------------------------------
//! Performance Debug
void UI_OsdDisplayFrmErrItem(UI_DisplayLocation_t tDispLoc)
{	
#if (defined(BUC_CU) && APP_DUAL_HOST_ENABLE)
	uint16_t uwFixXStart[MAX_DBG_ITEM];
	uint16_t uwFixYStart[MAX_DBG_ITEM];
#endif
	
#define DBGDISP_ICON_OFFSET 40
	uint16_t uwLcd_HSize  = uwOSD_GetHSize();
	uint16_t uwLcd_VSize  = uwOSD_GetVSize();
	uint16_t uwXOffset[11] = {[DISP_UPPER_LEFT] = 0,               [DISP_UPPER_RIGHT] = 0,
	                         [DISP_LOWER_LEFT]  = (uwLcd_HSize/2), [DISP_LOWER_RIGHT] = (uwLcd_HSize/2),
							 [DISP_LEFT] 	    = 0,			   [DISP_RIGHT] 	  = 0,
							 [DISP_H_L] 	    = 0,  			   [DISP_H_R] 		  = 0,
							 [DISP_H_CU] 	    = 0, 		       [DISP_H_CL] 		  = (uwLcd_HSize/2)};
	uint16_t uwYOffset[11] = {[DISP_UPPER_LEFT] = 0,               [DISP_UPPER_RIGHT] = (uwLcd_VSize/2),
	                         [DISP_LOWER_LEFT]  = 0, 			   [DISP_LOWER_RIGHT] = (uwLcd_VSize/2),
							 [DISP_LEFT] 	    = 0, 		       [DISP_RIGHT] 	  = (uwLcd_VSize/2),
							 [DISP_H_L] 	    = 0, 			   [DISP_H_R] 		  = 960,
							 [DISP_H_CU] 	    = 360, 		       [DISP_H_CL] 		  = 360};
	uint16_t uwXStart = 0, uwYStart = 0;
	uint8_t ubDbgItem;
							 
#if (defined(BUC_CU) && APP_DUAL_HOST_ENABLE)
	uwXOffset[0] = uwXOffset[0];	
	uwYOffset[0] = uwYOffset[0];
	uwXStart = uwXStart;
	uwYStart = uwYStart;
	ubDbgItem = ubDbgItem;
#endif

	tDispLoc = ((tCamViewSel.tCamViewType == SINGLE_VIEW) || (tCamViewSel.tCamViewType == SCAN_VIEW))?DISP_UPPER_LEFT:tDispLoc;	
	
//justin 2020.07.07
#if (defined(BUC_CU) && APP_DUAL_HOST_ENABLE)
	tDispLoc = DISP_UPPER_LEFT;	
	uwFixXStart[0] = 180;
	uwFixYStart[0] = 740;
							 
	uwFixXStart[1] = 220;
	uwFixYStart[1] = 740;
							 
	uwFixXStart[2] = 260;
	uwFixYStart[2] = 740;
	
	tUI_PerDbgOsdImgInfo[11+0].uwXStart = uwFixXStart[0];
	tUI_PerDbgOsdImgInfo[11+0].uwYStart = uwFixYStart[0];
	tOSD_Img2(&tUI_PerDbgOsdImgInfo[11+0], OSD_QUEUE);
	
	tUI_PerDbgOsdImgInfo[11+1].uwXStart = uwFixXStart[1];
	tUI_PerDbgOsdImgInfo[11+1].uwYStart = uwFixYStart[1];
	tOSD_Img2(&tUI_PerDbgOsdImgInfo[11+1], OSD_QUEUE);
	
	tUI_PerDbgOsdImgInfo[11+2].uwXStart = uwFixXStart[2];
	tUI_PerDbgOsdImgInfo[11+2].uwYStart = uwFixYStart[2];
	tOSD_Img2(&tUI_PerDbgOsdImgInfo[11+2], OSD_QUEUE);
#else							 
	for(ubDbgItem = 0; ubDbgItem < MAX_DBG_ITEM; ubDbgItem++)
	{		
		uwXStart = tUI_PerDbgOsdImgInfo[11+ubDbgItem].uwXStart;
		uwYStart = tUI_PerDbgOsdImgInfo[11+ubDbgItem].uwYStart;
		tUI_PerDbgOsdImgInfo[11+ubDbgItem].uwXStart += uwXOffset[tDispLoc];
		tUI_PerDbgOsdImgInfo[11+ubDbgItem].uwYStart -= uwYOffset[tDispLoc];
		tOSD_Img2(&tUI_PerDbgOsdImgInfo[11+ubDbgItem], OSD_QUEUE);
		tUI_PerDbgOsdImgInfo[11+ubDbgItem].uwXStart = uwXStart;
		tUI_PerDbgOsdImgInfo[11+ubDbgItem].uwYStart = uwYStart;
	}
#endif
}
//------------------------------------------------------------------------------
void UI_OsdDisplayFrameSeqFunc(uint8_t ubCamNum, uint16_t uwFrameSeq, uint16_t uwXPos, uint16_t uwYPos)
{
/*
	static uint8_t ubFrameSeq[CAM4 + 1][4]={{0xFF, 0xFF, 0xFF, 0xFF},
											{0xFF, 0xFF, 0xFF, 0xFF},
											{0xFF, 0xFF, 0xFF, 0xFF},
											{0xFF, 0xFF, 0xFF, 0xFF}};
	uint8_t ubTen = 0, ubUnit = 0, ubHun = 0, ubThus = 0;
	uint16_t uwXStart = 0, uwYStart = 0;

	if(UI_DISPLAY_STATE != tUI_State)
	{
		memset(ubFrameSeq, 0xFF, sizeof(ubFrameSeq));
		return;
	}
	osMutexWait(osUI_PerDbgMutex, osWaitForever);
	ubThus =  uwFrameSeq / 1000;
//	ubHun  = (uwFrameSeq - (ubThus * 1000)) / 100;
//	ubTen  = (uwFrameSeq - ((ubThus * 1000) + (ubHun * 100))) / 10;
//	ubUnit = (uwFrameSeq - ((ubThus * 1000) + (ubHun * 100) + (ubTen * 10)));
	ubHun  = (uwFrameSeq % 1000) / 100;
	ubTen  = (uwFrameSeq % 100) / 10;
	ubUnit = uwFrameSeq % 10;
	uwXStart = 60 + uwXPos;	
	tUI_PerDbgOsdImgInfo[ubThus].uwYStart  = uwOSD_GetVSize() - uwYPos - tUI_PerDbgOsdImgInfo[ubThus].uwVSize;
	if (ubThus != ubFrameSeq[ubCamNum][3])
	{
		tUI_PerDbgOsdImgInfo[ubThus].uwXStart  = uwXStart;
		tOSD_Img2(&tUI_PerDbgOsdImgInfo[ubThus], OSD_QUEUE);
		ubFrameSeq[ubCamNum][3] = ubThus;
	}
	uwYStart = tUI_PerDbgOsdImgInfo[ubThus].uwYStart;
//	tUI_PerDbgOsdImgInfo[ubThus].uwXStart  = 0;
//	tUI_PerDbgOsdImgInfo[ubThus].uwYStart  = 0;	
	tUI_PerDbgOsdImgInfo[ubHun].uwYStart  = uwYStart - tUI_PerDbgOsdImgInfo[ubThus].uwVSize;
	if (ubHun != ubFrameSeq[ubCamNum][2])
	{
		tUI_PerDbgOsdImgInfo[ubHun].uwXStart  = uwXStart;
		tOSD_Img2(&tUI_PerDbgOsdImgInfo[ubHun], OSD_QUEUE);
		ubFrameSeq[ubCamNum][2] = ubHun;
	}
	uwYStart = tUI_PerDbgOsdImgInfo[ubHun].uwYStart;
//	tUI_PerDbgOsdImgInfo[ubHun].uwXStart  = 0;
//	tUI_PerDbgOsdImgInfo[ubHun].uwYStart  = 0;	
	tUI_PerDbgOsdImgInfo[ubTen].uwYStart  = uwYStart - tUI_PerDbgOsdImgInfo[ubHun].uwVSize;
	if (ubTen != ubFrameSeq[ubCamNum][1])
	{
		tUI_PerDbgOsdImgInfo[ubTen].uwXStart  = uwXStart;
		tOSD_Img2(&tUI_PerDbgOsdImgInfo[ubTen], OSD_QUEUE);
		ubFrameSeq[ubCamNum][1] = ubTen;
	}
	uwYStart = tUI_PerDbgOsdImgInfo[ubTen].uwYStart;
//	tUI_PerDbgOsdImgInfo[ubTen].uwXStart  = 0;
//	tUI_PerDbgOsdImgInfo[ubTen].uwYStart  = 0;	
	tUI_PerDbgOsdImgInfo[ubUnit].uwXStart = uwXStart;
	tUI_PerDbgOsdImgInfo[ubUnit].uwYStart = uwYStart - tUI_PerDbgOsdImgInfo[ubTen].uwVSize;
	tOSD_Img2(&tUI_PerDbgOsdImgInfo[ubUnit], OSD_UPDATE);
//	tUI_PerDbgOsdImgInfo[ubUnit].uwXStart = 0;
//	tUI_PerDbgOsdImgInfo[ubUnit].uwYStart = 0;
	osMutexRelease(osUI_PerDbgMutex);
*/
	OSD_IMG_INFO tFixLocateImgInfo;
	static uint8_t ubFrameSeq[CAM4 + 1][4]={{0xFF, 0xFF, 0xFF, 0xFF},
											{0xFF, 0xFF, 0xFF, 0xFF},
											{0xFF, 0xFF, 0xFF, 0xFF},
											{0xFF, 0xFF, 0xFF, 0xFF}};
	uint8_t ubTen = 0, ubUnit = 0, ubHun = 0, ubThus = 0;
	uint16_t uwXStart = 0, uwYStart = 0;
											
	if((UI_DISPLAY_STATE != tUI_State) || ubUI_TrxDebugUpdateFlg[TRX_DBG_FRMSEQ_IDX])
	{
		if(ubUI_TrxDebugUpdateFlg[TRX_DBG_FRMSEQ_IDX])
			ubUI_TrxDebugUpdateFlg[TRX_DBG_FRMSEQ_IDX] = 0;
		
		memset(ubFrameSeq, 0xFF, sizeof(ubFrameSeq));
		return;
	}
	
//#if (defined(BUC_CU) && APP_DUAL_HOST_ENABLE)
//	if(KNL_WIRELESS_CAM_NUM == 1)
//	{		
//		if(ubCamNum != 0)
//			return;			
//	}		
//#endif
	
	osMutexWait(osUI_PerDbgMutex, osWaitForever);
	ubThus =  uwFrameSeq / 1000;
//	ubHun  = (uwFrameSeq - (ubThus * 1000)) / 100;
//	ubTen  = (uwFrameSeq - ((ubThus * 1000) + (ubHun * 100))) / 10;
//	ubUnit = (uwFrameSeq - ((ubThus * 1000) + (ubHun * 100) + (ubTen * 10)));
	ubHun  = (uwFrameSeq % 1000) / 100;
	ubTen  = (uwFrameSeq % 100) / 10;
	ubUnit = uwFrameSeq % 10;
	uwXStart = 60 + uwXPos;	
	tUI_PerDbgOsdImgInfo[ubThus].uwYStart  = uwOSD_GetVSize() - uwYPos - tUI_PerDbgOsdImgInfo[ubThus].uwVSize;
	if (ubThus != ubFrameSeq[ubCamNum][3])
	{
		tUI_PerDbgOsdImgInfo[ubThus].uwXStart  = uwXStart;				
		
	#if (defined(BUC_CU) && APP_DUAL_HOST_ENABLE)
		memcpy(&tFixLocateImgInfo,&tUI_PerDbgOsdImgInfo[ubThus],sizeof(OSD_IMG_INFO));
		tFixLocateImgInfo.uwXStart	= 60;
		tFixLocateImgInfo.uwYStart 	= 830;
	#else	
		memcpy(&tFixLocateImgInfo,&tUI_PerDbgOsdImgInfo[ubThus],sizeof(OSD_IMG_INFO));
	#endif		
		tOSD_Img2(&tFixLocateImgInfo, OSD_QUEUE);
		
		
		ubFrameSeq[ubCamNum][3] = ubThus;
	}
	uwYStart = tUI_PerDbgOsdImgInfo[ubThus].uwYStart;
//	tUI_PerDbgOsdImgInfo[ubThus].uwXStart  = 0;
//	tUI_PerDbgOsdImgInfo[ubThus].uwYStart  = 0;	
	tUI_PerDbgOsdImgInfo[ubHun].uwYStart  = uwYStart - tUI_PerDbgOsdImgInfo[ubThus].uwVSize;
	if (ubHun != ubFrameSeq[ubCamNum][2])
	{
		tUI_PerDbgOsdImgInfo[ubHun].uwXStart  = uwXStart;		
		
	#if (defined(BUC_CU) && APP_DUAL_HOST_ENABLE)
		memcpy(&tFixLocateImgInfo,&tUI_PerDbgOsdImgInfo[ubHun],sizeof(OSD_IMG_INFO));
		tFixLocateImgInfo.uwXStart	= 60;
		tFixLocateImgInfo.uwYStart 	= 810;
	#else	
		memcpy(&tFixLocateImgInfo,&tUI_PerDbgOsdImgInfo[ubHun],sizeof(OSD_IMG_INFO));
	#endif		
		tOSD_Img2(&tFixLocateImgInfo, OSD_QUEUE);		
		
		ubFrameSeq[ubCamNum][2] = ubHun;
	}
	uwYStart = tUI_PerDbgOsdImgInfo[ubHun].uwYStart;
//	tUI_PerDbgOsdImgInfo[ubHun].uwXStart  = 0;
//	tUI_PerDbgOsdImgInfo[ubHun].uwYStart  = 0;	
	tUI_PerDbgOsdImgInfo[ubTen].uwYStart  = uwYStart - tUI_PerDbgOsdImgInfo[ubHun].uwVSize;
	if (ubTen != ubFrameSeq[ubCamNum][1])
	{
		tUI_PerDbgOsdImgInfo[ubTen].uwXStart  = uwXStart;		
		
	#if (defined(BUC_CU) && APP_DUAL_HOST_ENABLE)
		memcpy(&tFixLocateImgInfo,&tUI_PerDbgOsdImgInfo[ubTen],sizeof(OSD_IMG_INFO));
		tFixLocateImgInfo.uwXStart	= 60;
		tFixLocateImgInfo.uwYStart 	= 790;
	#else	
		memcpy(&tFixLocateImgInfo,&tUI_PerDbgOsdImgInfo[ubTen],sizeof(OSD_IMG_INFO));
	#endif		
		tOSD_Img2(&tFixLocateImgInfo, OSD_QUEUE);		
		
		ubFrameSeq[ubCamNum][1] = ubTen;
	}
	uwYStart = tUI_PerDbgOsdImgInfo[ubTen].uwYStart;
//	tUI_PerDbgOsdImgInfo[ubTen].uwXStart  = 0;
//	tUI_PerDbgOsdImgInfo[ubTen].uwYStart  = 0;	
	tUI_PerDbgOsdImgInfo[ubUnit].uwXStart = uwXStart;
	tUI_PerDbgOsdImgInfo[ubUnit].uwYStart = uwYStart - tUI_PerDbgOsdImgInfo[ubTen].uwVSize;

	//tOSD_Img2(&tUI_PerDbgOsdImgInfo[ubUnit], OSD_UPDATE);
#if (defined(BUC_CU) && APP_DUAL_HOST_ENABLE)
	memcpy(&tFixLocateImgInfo,&tUI_PerDbgOsdImgInfo[ubUnit],sizeof(OSD_IMG_INFO));
	tFixLocateImgInfo.uwXStart	= 60;
	tFixLocateImgInfo.uwYStart 	= 770;
#else	
	memcpy(&tFixLocateImgInfo,&tUI_PerDbgOsdImgInfo[ubUnit],sizeof(OSD_IMG_INFO));
#endif	
	tOSD_Img2(&tFixLocateImgInfo, OSD_UPDATE);	
	
//	tUI_PerDbgOsdImgInfo[ubUnit].uwXStart = 0;
//	tUI_PerDbgOsdImgInfo[ubUnit].uwYStart = 0;
	osMutexRelease(osUI_PerDbgMutex);
	
}
//------------------------------------------------------------------------------
void UI_OsdDisplayFpsFunc(uint8_t ubCamNum, uint16_t uwFps, uint16_t uwXPos, uint16_t uwYPos)
{
/*
	static uint8_t ubFps[CAM4 + 1][2]={{0xFF, 0xFF},
									   {0xFF, 0xFF},
									   {0xFF, 0xFF},
									   {0xFF, 0xFF}};
	uint8_t ubTen = 0, ubUnit = 0;
	uint16_t uwXStart = 0, uwYStart = 0;

	if(UI_DISPLAY_STATE != tUI_State)
	{
		memset(ubFps, 0xFF, sizeof(ubFps));
		return;
	}
	osMutexWait(osUI_PerDbgMutex, osWaitForever);
	uwXStart = 60 + uwXPos;
	tUI_PerDbgOsdImgInfo[10].uwYStart = (uwOSD_GetVSize() - uwYPos - tUI_PerDbgOsdImgInfo[10].uwVSize - 240);
	if (0xFF == ubFps[ubCamNum][1])
	{
		tUI_PerDbgOsdImgInfo[10].uwXStart = uwXStart;
		tOSD_Img2(&tUI_PerDbgOsdImgInfo[10], OSD_QUEUE);
	}
	uwYStart = tUI_PerDbgOsdImgInfo[10].uwYStart;
//	tUI_PerDbgOsdImgInfo[10].uwXStart = 0;
//	tUI_PerDbgOsdImgInfo[10].uwYStart = 0;
	ubTen  = uwFps / 10;
	ubUnit = uwFps % 10;
	tUI_PerDbgOsdImgInfo[ubTen].uwYStart  = uwYStart - tUI_PerDbgOsdImgInfo[ubTen].uwVSize;
	if (ubTen != ubFps[ubCamNum][1])
	{
		tUI_PerDbgOsdImgInfo[ubTen].uwXStart  = uwXStart;
		tOSD_Img2(&tUI_PerDbgOsdImgInfo[ubTen], OSD_QUEUE);
	}
	if (ubUnit != ubFps[ubCamNum][0])
	{
		uwYStart = tUI_PerDbgOsdImgInfo[ubTen].uwYStart;
	//	tUI_PerDbgOsdImgInfo[ubTen].uwXStart = 0;
	//	tUI_PerDbgOsdImgInfo[ubTen].uwYStart = 0;
		tUI_PerDbgOsdImgInfo[ubUnit].uwXStart = uwXStart;
		tUI_PerDbgOsdImgInfo[ubUnit].uwYStart = uwYStart - tUI_PerDbgOsdImgInfo[ubUnit].uwVSize;
		tOSD_Img2(&tUI_PerDbgOsdImgInfo[ubUnit], OSD_UPDATE);
	//	tUI_PerDbgOsdImgInfo[ubUnit].uwXStart = 0;
	//	tUI_PerDbgOsdImgInfo[ubUnit].uwYStart = 0;
		ubFps[ubCamNum][0] = ubUnit;
		ubFps[ubCamNum][1] = ubTen;
	}
	else if (ubTen != ubFps[ubCamNum][1])
	{
		OSD_UpdateQueueBuf();
		ubFps[ubCamNum][1] = ubTen;
	}
	osMutexRelease(osUI_PerDbgMutex);
*/
	
	OSD_IMG_INFO tFixLocateImgInfo;
	static uint8_t ubFps[CAM4 + 1][2]={{0xFF, 0xFF},
									   {0xFF, 0xFF},
									   {0xFF, 0xFF},
									   {0xFF, 0xFF}};
	uint8_t ubTen = 0, ubUnit = 0;
	uint16_t uwXStart = 0, uwYStart = 0;	
									   
	//if(UI_DISPLAY_STATE != tUI_State)
	if((UI_DISPLAY_STATE != tUI_State) || ubUI_TrxDebugUpdateFlg[TRX_DBG_FPS_IDX])
	{
		if(ubUI_TrxDebugUpdateFlg[TRX_DBG_FPS_IDX])
			ubUI_TrxDebugUpdateFlg[TRX_DBG_FPS_IDX] = 0;
		memset(ubFps, 0xFF, sizeof(ubFps));
		return;
	}
	
//#if (defined(BUC_CU) && APP_DUAL_HOST_ENABLE)
//	if(KNL_WIRELESS_CAM_NUM == 1)
//	{		
//		if(ubCamNum != 0)
//			return;			
//	}		
//#endif	
	
	osMutexWait(osUI_PerDbgMutex, osWaitForever);
	uwXStart = 60 + uwXPos;
	tUI_PerDbgOsdImgInfo[10].uwYStart = (uwOSD_GetVSize() - uwYPos - tUI_PerDbgOsdImgInfo[10].uwVSize - 240);
	if (0xFF == ubFps[ubCamNum][1])	
	{
		tUI_PerDbgOsdImgInfo[10].uwXStart = uwXStart;
	#if (defined(BUC_CU) && APP_DUAL_HOST_ENABLE)
		memcpy(&tFixLocateImgInfo,&tUI_PerDbgOsdImgInfo[10],sizeof(OSD_IMG_INFO));
		tFixLocateImgInfo.uwXStart	= 60;
		tFixLocateImgInfo.uwYStart 	= 630;
	#else		
		memcpy(&tFixLocateImgInfo,&tUI_PerDbgOsdImgInfo[10],sizeof(OSD_IMG_INFO));
	#endif		
		tOSD_Img2(&tFixLocateImgInfo, OSD_QUEUE);
	}
	uwYStart = tUI_PerDbgOsdImgInfo[10].uwYStart;
//	tUI_PerDbgOsdImgInfo[10].uwXStart = 0;
//	tUI_PerDbgOsdImgInfo[10].uwYStart = 0;
	ubTen  = uwFps / 10;
	ubUnit = uwFps % 10;
	tUI_PerDbgOsdImgInfo[ubTen].uwYStart  = uwYStart - tUI_PerDbgOsdImgInfo[ubTen].uwVSize;
	if (ubTen != ubFps[ubCamNum][1])	
	{
		tUI_PerDbgOsdImgInfo[ubTen].uwXStart  = uwXStart;
	#if (defined(BUC_CU) && APP_DUAL_HOST_ENABLE)
		memcpy(&tFixLocateImgInfo,&tUI_PerDbgOsdImgInfo[ubTen],sizeof(OSD_IMG_INFO));
		tFixLocateImgInfo.uwXStart	= 60;
		tFixLocateImgInfo.uwYStart 	= 610;
	#else		
		memcpy(&tFixLocateImgInfo,&tUI_PerDbgOsdImgInfo[ubTen],sizeof(OSD_IMG_INFO));
	#endif		
		tOSD_Img2(&tFixLocateImgInfo, OSD_QUEUE);
	}
	if (ubUnit != ubFps[ubCamNum][0])	
	{
		uwYStart = tUI_PerDbgOsdImgInfo[ubTen].uwYStart;
	//	tUI_PerDbgOsdImgInfo[ubTen].uwXStart = 0;
	//	tUI_PerDbgOsdImgInfo[ubTen].uwYStart = 0;
		tUI_PerDbgOsdImgInfo[ubUnit].uwXStart = uwXStart;
		tUI_PerDbgOsdImgInfo[ubUnit].uwYStart = uwYStart - tUI_PerDbgOsdImgInfo[ubUnit].uwVSize;
			
	#if (defined(BUC_CU) && APP_DUAL_HOST_ENABLE)
		memcpy(&tFixLocateImgInfo,&tUI_PerDbgOsdImgInfo[ubUnit],sizeof(OSD_IMG_INFO));
		tFixLocateImgInfo.uwXStart	= 60;
		tFixLocateImgInfo.uwYStart 	= 590;
	#else
		memcpy(&tFixLocateImgInfo,&tUI_PerDbgOsdImgInfo[ubUnit],sizeof(OSD_IMG_INFO));		
	#endif		
		tOSD_Img2(&tFixLocateImgInfo, OSD_UPDATE);		
		
	//	tUI_PerDbgOsdImgInfo[ubUnit].uwXStart = 0;
	//	tUI_PerDbgOsdImgInfo[ubUnit].uwYStart = 0;
		ubFps[ubCamNum][0] = ubUnit;
		ubFps[ubCamNum][1] = ubTen;
	}
	else if (ubTen != ubFps[ubCamNum][1])
	{
		OSD_UpdateQueueBuf();
		ubFps[ubCamNum][1] = ubTen;
	}
	osMutexRelease(osUI_PerDbgMutex);
}
//------------------------------------------------------------------------------
void UI_OsdDisplayRfBwFunc(uint8_t ubCamNum, uint16_t uwBw, uint16_t uwXPos, uint16_t uwYPos)
{
	OSD_IMG_INFO tFixLocateImgInfo;
	
	uint8_t ubHun = 0, ubTen = 0, ubUnit = 0;
	uint16_t uwXStart = 0, uwYStart = 0;	
	
	if(UI_DISPLAY_STATE != tUI_State)
		return;
	
//#if (defined(BUC_CU) && APP_DUAL_HOST_ENABLE)
//	if(KNL_WIRELESS_CAM_NUM == 1)
//	{		
//		if(ubCamNum != 0)
//			return;			
//	}		
//#endif
	
	osMutexWait(osUI_PerDbgMutex, osWaitForever);
	uwXStart = 60 + uwXPos;
	tUI_PerDbgOsdImgInfo[10].uwXStart = uwXStart;
	tUI_PerDbgOsdImgInfo[10].uwYStart = (uwOSD_GetVSize() - uwYPos - tUI_PerDbgOsdImgInfo[10].uwVSize - 135);
	
#if (defined(BUC_CU) && APP_DUAL_HOST_ENABLE)
	memcpy(&tFixLocateImgInfo,&tUI_PerDbgOsdImgInfo[10],sizeof(OSD_IMG_INFO));
	tFixLocateImgInfo.uwXStart	= 60;
	tFixLocateImgInfo.uwYStart 	= 730;
#else	
	memcpy(&tFixLocateImgInfo,&tUI_PerDbgOsdImgInfo[10],sizeof(OSD_IMG_INFO));
#endif		
	//tOSD_Img2(&tUI_PerDbgOsdImgInfo[10], OSD_QUEUE);
	tOSD_Img2(&tFixLocateImgInfo, OSD_QUEUE);
	
	uwYStart = tUI_PerDbgOsdImgInfo[10].uwYStart;
	tUI_PerDbgOsdImgInfo[10].uwXStart = 0;
	tUI_PerDbgOsdImgInfo[10].uwYStart = 0;
	ubHun  = uwBw / 100;
	ubTen  = (uwBw - (ubHun * 100)) / 10;
	ubUnit = uwBw - ((ubHun * 100) + (ubTen * 10));
	tUI_PerDbgOsdImgInfo[ubHun].uwXStart  = uwXStart;
	tUI_PerDbgOsdImgInfo[ubHun].uwYStart  = uwYStart - tUI_PerDbgOsdImgInfo[ubHun].uwVSize;
#if (defined(BUC_CU) && APP_DUAL_HOST_ENABLE)
	memcpy(&tFixLocateImgInfo,&tUI_PerDbgOsdImgInfo[ubHun],sizeof(OSD_IMG_INFO));
	tFixLocateImgInfo.uwXStart	= 60;
	tFixLocateImgInfo.uwYStart 	= 710;
#else	
	memcpy(&tFixLocateImgInfo,&tUI_PerDbgOsdImgInfo[ubHun],sizeof(OSD_IMG_INFO));
#endif	
	//tOSD_Img2(&tUI_PerDbgOsdImgInfo[ubHun], OSD_QUEUE);
	tOSD_Img2(&tFixLocateImgInfo, OSD_QUEUE);
	uwYStart = tUI_PerDbgOsdImgInfo[ubHun].uwYStart;
	tUI_PerDbgOsdImgInfo[ubHun].uwXStart  = 0;
	tUI_PerDbgOsdImgInfo[ubHun].uwYStart  = 0;
	tUI_PerDbgOsdImgInfo[ubTen].uwXStart  = uwXStart;
	tUI_PerDbgOsdImgInfo[ubTen].uwYStart  = uwYStart - tUI_PerDbgOsdImgInfo[ubTen].uwVSize;
#if (defined(BUC_CU) && APP_DUAL_HOST_ENABLE)
	memcpy(&tFixLocateImgInfo,&tUI_PerDbgOsdImgInfo[ubTen],sizeof(OSD_IMG_INFO));
	tFixLocateImgInfo.uwXStart	= 60;
	tFixLocateImgInfo.uwYStart 	= 690;
#else	
	memcpy(&tFixLocateImgInfo,&tUI_PerDbgOsdImgInfo[ubTen],sizeof(OSD_IMG_INFO));
#endif	
	//tOSD_Img2(&tUI_PerDbgOsdImgInfo[ubTen], OSD_QUEUE);
	tOSD_Img2(&tFixLocateImgInfo, OSD_QUEUE);
	uwYStart = tUI_PerDbgOsdImgInfo[ubTen].uwYStart;
	tUI_PerDbgOsdImgInfo[ubTen].uwXStart  = 0;
	tUI_PerDbgOsdImgInfo[ubTen].uwYStart  = 0;
	tUI_PerDbgOsdImgInfo[ubUnit].uwXStart = uwXStart;
	tUI_PerDbgOsdImgInfo[ubUnit].uwYStart = uwYStart - tUI_PerDbgOsdImgInfo[ubUnit].uwVSize;
#if (defined(BUC_CU) && APP_DUAL_HOST_ENABLE)
	memcpy(&tFixLocateImgInfo,&tUI_PerDbgOsdImgInfo[ubUnit],sizeof(OSD_IMG_INFO));
	tFixLocateImgInfo.uwXStart	= 60;
	tFixLocateImgInfo.uwYStart 	= 670;
#else	
	memcpy(&tFixLocateImgInfo,&tUI_PerDbgOsdImgInfo[ubUnit],sizeof(OSD_IMG_INFO));
#endif
	//tOSD_Img2(&tUI_PerDbgOsdImgInfo[ubUnit], OSD_UPDATE);
	tOSD_Img2(&tFixLocateImgInfo, OSD_UPDATE);
	tUI_PerDbgOsdImgInfo[ubUnit].uwXStart  = 0;
	tUI_PerDbgOsdImgInfo[ubUnit].uwYStart  = 0;
	osMutexRelease(osUI_PerDbgMutex);
}
//------------------------------------------------------------------------------
void UI_OsdDisplaySPRfStatus(uint8_t ubCamNum, uint16_t uwValue, uint16_t uwXPos, uint16_t uwYPos)
{
#ifdef S2019A
	uint8_t ubHun = 0, ubTen = 0, ubUnit = 0;
	uint8_t ubCh = 0, ubRssi = 0, ubPer = 0;
	uint16_t uwXStart = 0, uwYStart = 0;

	if(UI_DISPLAY_STATE != tUI_State)
		return;
	osMutexWait(osUI_PerDbgMutex, osWaitForever);
	//! CH
	ubCh = ubsPRF_GetWorkCh();
	uwXStart = 60 + uwXPos;
	tUI_PerDbgOsdImgInfo[10].uwXStart = uwXStart;
	tUI_PerDbgOsdImgInfo[10].uwYStart = (uwOSD_GetVSize() - uwYPos - tUI_PerDbgOsdImgInfo[10].uwVSize - 135);
	tOSD_Img2(&tUI_PerDbgOsdImgInfo[10], OSD_QUEUE);
	uwYStart = tUI_PerDbgOsdImgInfo[10].uwYStart;
	tUI_PerDbgOsdImgInfo[10].uwXStart = 0;
	tUI_PerDbgOsdImgInfo[10].uwYStart = 0;
	ubTen  = ubCh / 10;
	ubUnit = ubCh - (ubTen * 10);
	tUI_PerDbgOsdImgInfo[ubTen].uwXStart  = uwXStart;
	tUI_PerDbgOsdImgInfo[ubTen].uwYStart  = uwYStart - tUI_PerDbgOsdImgInfo[ubTen].uwVSize;
	tOSD_Img2(&tUI_PerDbgOsdImgInfo[ubTen], OSD_QUEUE);
	uwYStart = tUI_PerDbgOsdImgInfo[ubTen].uwYStart;
	tUI_PerDbgOsdImgInfo[ubTen].uwXStart  = 0;
	tUI_PerDbgOsdImgInfo[ubTen].uwYStart  = 0;
	tUI_PerDbgOsdImgInfo[ubUnit].uwXStart = uwXStart;
	tUI_PerDbgOsdImgInfo[ubUnit].uwYStart = uwYStart - tUI_PerDbgOsdImgInfo[ubUnit].uwVSize;
	tOSD_Img2(&tUI_PerDbgOsdImgInfo[ubUnit], OSD_UPDATE);
	tUI_PerDbgOsdImgInfo[ubUnit].uwXStart  = 0;
	tUI_PerDbgOsdImgInfo[ubUnit].uwYStart  = 0;
	//! RSSI
	ubRssi = tsPRF_GetRssi();
	uwXStart = 100 + uwXPos;
	tUI_PerDbgOsdImgInfo[10].uwXStart = uwXStart;
	tUI_PerDbgOsdImgInfo[10].uwYStart = (uwOSD_GetVSize() - uwYPos - tUI_PerDbgOsdImgInfo[10].uwVSize - 240);
	tOSD_Img2(&tUI_PerDbgOsdImgInfo[10], OSD_QUEUE);
	uwYStart = tUI_PerDbgOsdImgInfo[10].uwYStart;
	tUI_PerDbgOsdImgInfo[10].uwXStart = 0;
	tUI_PerDbgOsdImgInfo[10].uwYStart = 0;
	ubHun  = ubRssi / 100;
	ubTen  = (ubRssi - (ubHun * 100)) / 10;
	ubUnit = ubRssi - ((ubHun * 100) + (ubTen * 10));
	tUI_PerDbgOsdImgInfo[ubHun].uwXStart  = uwXStart;
	tUI_PerDbgOsdImgInfo[ubHun].uwYStart  = uwYStart - tUI_PerDbgOsdImgInfo[ubHun].uwVSize;
	tOSD_Img2(&tUI_PerDbgOsdImgInfo[ubHun], OSD_QUEUE);
	uwYStart = tUI_PerDbgOsdImgInfo[ubHun].uwYStart;
	tUI_PerDbgOsdImgInfo[ubHun].uwXStart  = 0;
	tUI_PerDbgOsdImgInfo[ubHun].uwYStart  = 0;
	tUI_PerDbgOsdImgInfo[ubTen].uwXStart  = uwXStart;
	tUI_PerDbgOsdImgInfo[ubTen].uwYStart  = uwYStart - tUI_PerDbgOsdImgInfo[ubTen].uwVSize;
	tOSD_Img2(&tUI_PerDbgOsdImgInfo[ubTen], OSD_QUEUE);
	uwYStart = tUI_PerDbgOsdImgInfo[ubTen].uwYStart;
	tUI_PerDbgOsdImgInfo[ubTen].uwXStart  = 0;
	tUI_PerDbgOsdImgInfo[ubTen].uwYStart  = 0;
	tUI_PerDbgOsdImgInfo[ubUnit].uwXStart = uwXStart;
	tUI_PerDbgOsdImgInfo[ubUnit].uwYStart = uwYStart - tUI_PerDbgOsdImgInfo[ubUnit].uwVSize;
	tOSD_Img2(&tUI_PerDbgOsdImgInfo[ubUnit], OSD_UPDATE);
	tUI_PerDbgOsdImgInfo[ubUnit].uwXStart  = 0;
	tUI_PerDbgOsdImgInfo[ubUnit].uwYStart  = 0;
	//! PER
	ubPer = tsPRF_GetPER((sPRF_DevId_t)(sPRF_STA1+ubCamNum));
	uwXStart = 140 + uwXPos;
	tUI_PerDbgOsdImgInfo[10].uwXStart = uwXStart;
	tUI_PerDbgOsdImgInfo[10].uwYStart = (uwOSD_GetVSize() - uwYPos - tUI_PerDbgOsdImgInfo[10].uwVSize - 240);
	tOSD_Img2(&tUI_PerDbgOsdImgInfo[10], OSD_QUEUE);
	uwYStart = tUI_PerDbgOsdImgInfo[10].uwYStart;
	tUI_PerDbgOsdImgInfo[10].uwXStart = 0;
	tUI_PerDbgOsdImgInfo[10].uwYStart = 0;
	ubHun  = ubPer / 100;
	ubTen  = (ubPer - (ubHun * 100)) / 10;
	ubUnit = ubPer - ((ubHun * 100) + (ubTen * 10));
	tUI_PerDbgOsdImgInfo[ubHun].uwXStart  = uwXStart;
	tUI_PerDbgOsdImgInfo[ubHun].uwYStart  = uwYStart - tUI_PerDbgOsdImgInfo[ubHun].uwVSize;
	tOSD_Img2(&tUI_PerDbgOsdImgInfo[ubHun], OSD_QUEUE);
	uwYStart = tUI_PerDbgOsdImgInfo[ubHun].uwYStart;
	tUI_PerDbgOsdImgInfo[ubHun].uwXStart  = 0;
	tUI_PerDbgOsdImgInfo[ubHun].uwYStart  = 0;
	tUI_PerDbgOsdImgInfo[ubTen].uwXStart  = uwXStart;
	tUI_PerDbgOsdImgInfo[ubTen].uwYStart  = uwYStart - tUI_PerDbgOsdImgInfo[ubTen].uwVSize;
	tOSD_Img2(&tUI_PerDbgOsdImgInfo[ubTen], OSD_QUEUE);
	uwYStart = tUI_PerDbgOsdImgInfo[ubTen].uwYStart;
	tUI_PerDbgOsdImgInfo[ubTen].uwXStart  = 0;
	tUI_PerDbgOsdImgInfo[ubTen].uwYStart  = 0;
	tUI_PerDbgOsdImgInfo[ubUnit].uwXStart = uwXStart;
	tUI_PerDbgOsdImgInfo[ubUnit].uwYStart = uwYStart - tUI_PerDbgOsdImgInfo[ubUnit].uwVSize;
	tOSD_Img2(&tUI_PerDbgOsdImgInfo[ubUnit], OSD_UPDATE);
	tUI_PerDbgOsdImgInfo[ubUnit].uwXStart  = 0;
	tUI_PerDbgOsdImgInfo[ubUnit].uwYStart  = 0;
	osMutexRelease(osUI_PerDbgMutex);
#endif
}
//------------------------------------------------------------------------------
void UI_OsdDisplayFrameErrType(uint8_t ubCamNum, uint16_t uwErrorType, uint16_t uwXPos, uint16_t uwYPos)
{
	uint8_t ubTen = 0, ubUnit = 0, ubHun = 0, ubThus = 0;
	uint16_t uwXStart = 0 ,uwYStart = 0;

	if(UI_DISPLAY_STATE != tUI_State)
		return;
	osMutexWait(osUI_PerDbgMutex, osWaitForever);
	ubThus = ulUI_FrameErrCnt[ubCamNum][uwErrorType] / 1000;
	ubHun  = (ulUI_FrameErrCnt[ubCamNum][uwErrorType] - (ubThus * 1000)) / 100;
	ubTen  = (ulUI_FrameErrCnt[ubCamNum][uwErrorType] - ((ubThus * 1000) + (ubHun * 100))) / 10;
	ubUnit = (ulUI_FrameErrCnt[ubCamNum][uwErrorType] - ((ubThus * 1000) + (ubHun * 100) + (ubTen * 10)));
	uwXStart = 105 + (uwErrorType * DBGDISP_ICON_OFFSET) + uwXPos;
	tUI_PerDbgOsdImgInfo[ubThus].uwXStart  = uwXStart;
	tUI_PerDbgOsdImgInfo[ubThus].uwYStart  = uwOSD_GetVSize() - uwYPos - tUI_PerDbgOsdImgInfo[ubThus].uwVSize - 40;

#if (defined(BUC_CU) && APP_DUAL_HOST_ENABLE)
	tUI_PerDbgOsdImgInfo[ubThus].uwYStart = 680;
#endif	
	tOSD_Img2(&tUI_PerDbgOsdImgInfo[ubThus], OSD_QUEUE);
	uwYStart = tUI_PerDbgOsdImgInfo[ubThus].uwYStart;
	tUI_PerDbgOsdImgInfo[ubThus].uwXStart  = 0;
	tUI_PerDbgOsdImgInfo[ubThus].uwYStart  = 0;
	tUI_PerDbgOsdImgInfo[ubHun].uwXStart   = uwXStart;
	tUI_PerDbgOsdImgInfo[ubHun].uwYStart   = uwYStart - tUI_PerDbgOsdImgInfo[ubHun].uwVSize;
#if (defined(BUC_CU) && APP_DUAL_HOST_ENABLE)
	tUI_PerDbgOsdImgInfo[ubThus].uwYStart = 680;
#endif
	tOSD_Img2(&tUI_PerDbgOsdImgInfo[ubHun], OSD_QUEUE);
	uwYStart = tUI_PerDbgOsdImgInfo[ubHun].uwYStart;
	tUI_PerDbgOsdImgInfo[ubHun].uwXStart   = 0;
	tUI_PerDbgOsdImgInfo[ubHun].uwYStart   = 0;
	tUI_PerDbgOsdImgInfo[ubTen].uwXStart   = uwXStart;
	tUI_PerDbgOsdImgInfo[ubTen].uwYStart   = uwYStart - tUI_PerDbgOsdImgInfo[ubTen].uwVSize;
#if (defined(BUC_CU) && APP_DUAL_HOST_ENABLE)
	tUI_PerDbgOsdImgInfo[ubThus].uwYStart = 680;
#endif
	tOSD_Img2(&tUI_PerDbgOsdImgInfo[ubTen], OSD_QUEUE);
	uwYStart = tUI_PerDbgOsdImgInfo[ubTen].uwYStart;
	tUI_PerDbgOsdImgInfo[ubTen].uwXStart   = 0;
	tUI_PerDbgOsdImgInfo[ubTen].uwYStart   = 0;
	tUI_PerDbgOsdImgInfo[ubUnit].uwXStart  = uwXStart;
	tUI_PerDbgOsdImgInfo[ubUnit].uwYStart  = uwYStart - tUI_PerDbgOsdImgInfo[ubUnit].uwVSize;
#if (defined(BUC_CU) && APP_DUAL_HOST_ENABLE)
	tUI_PerDbgOsdImgInfo[ubThus].uwYStart = 680;
#endif
	tOSD_Img2(&tUI_PerDbgOsdImgInfo[ubUnit], OSD_UPDATE);
	tUI_PerDbgOsdImgInfo[ubUnit].uwXStart   = 0;
	tUI_PerDbgOsdImgInfo[ubUnit].uwYStart   = 0;
	osMutexRelease(osUI_PerDbgMutex);
}
//------------------------------------------------------------------------------
void UI_DisplayTrxInfo(UI_FuncExecMsg_t tPerRpt)
{
	static void (*pvUI_PerRptFunc[])(uint8_t, uint16_t, uint16_t, uint16_t) =
	{
		[PER_FRMSTS_RPT]	= UI_OsdDisplayFrameErrType,
		[PER_FRMSEQ_RPT]	= UI_OsdDisplayFrameSeqFunc,
		[PER_FPS_RPT]		= UI_OsdDisplayFpsFunc,
		[PER_RFBW_RPT] 	   	= UI_OsdDisplayRfBwFunc,
		[PER_SPRFSTS_RPT]  	= UI_OsdDisplaySPRfStatus,
	};
	uint16_t uwLcd_HSize  = uwOSD_GetHSize();
	uint16_t uwLcd_VSize  = uwOSD_GetVSize();
	uint16_t uwXOffset[11] = {[DISP_UPPER_LEFT] = 0,               [DISP_UPPER_RIGHT] = 0,
	                         [DISP_LOWER_LEFT]  = (uwLcd_HSize/2), [DISP_LOWER_RIGHT] = (uwLcd_HSize/2),
							 [DISP_LEFT] 	    = 0,			   [DISP_RIGHT] 	  = 0,
							 [DISP_H_L] 	    = 0,  			   [DISP_H_R] 		  = 0,
							 [DISP_H_CU] 	    = 0, 		       [DISP_H_CL] 		  = (uwLcd_HSize/2)};
	uint16_t uwYOffset[11] = {[DISP_UPPER_LEFT] = 0,               [DISP_UPPER_RIGHT] = (uwLcd_VSize/2),
	                         [DISP_LOWER_LEFT]  = 0, 			   [DISP_LOWER_RIGHT] = (uwLcd_VSize/2),
							 [DISP_LEFT] 	    = 0, 		       [DISP_RIGHT] 	  = (uwLcd_VSize/2),
							 [DISP_H_L] 	    = 0, 			   [DISP_H_R] 		  = 960,
							 [DISP_H_CU] 	    = 360, 		       [DISP_H_CL] 		  = 360};
	UI_DisplayLocation_t tUI_DispLoc;

	if((SINGLE_VIEW == tCamViewSel.tCamViewType) && (((UI_CamNum_t)tPerRpt.ubCamNum != tCamViewSel.tCamViewPool[0])))
		return;
	if((DISPLAY_4T1R == tUI_CuSetting.ubTotalCamNum) && (tCamViewSel.tCamViewType == DUAL_VIEW))
		tUI_DispLoc = (tPerRpt.ubCamNum == tCamViewSel.tCamViewPool[0])?DISP_LEFT:DISP_RIGHT;
	else
		tUI_DispLoc = ((tCamViewSel.tCamViewType == SINGLE_VIEW) || (tCamViewSel.tCamViewType == SCAN_VIEW))?DISP_UPPER_LEFT:tUI_CamStatus[tPerRpt.ubCamNum].tCamDispLocation;
	if(pvUI_PerRptFunc[tPerRpt.ubType])
	{	
	#if (defined(BUC_CU) && APP_DUAL_HOST_ENABLE)		
		if(KNL_WIRELESS_CAM_NUM == 1)
		{
			if(tPerRpt.ubCamNum == 0)	//CAM1
				pvUI_PerRptFunc[tPerRpt.ubType](tPerRpt.ubCamNum, tPerRpt.uwFuncMsg, uwXOffset[tUI_DispLoc], uwYOffset[tUI_DispLoc]);	
		}
	#else
		pvUI_PerRptFunc[tPerRpt.ubType](tPerRpt.ubCamNum, tPerRpt.uwFuncMsg, uwXOffset[tUI_DispLoc], uwYOffset[tUI_DispLoc]);
	#endif
	}
}
//------------------------------------------------------------------------------
void UI_PerReportFunc(uint8_t ubRptType, uint8_t ubCamNum, uint16_t uwValue)
{
	UI_FuncExecMsg_t tWorkAct;

	if(INVALID_ID == tUI_CamStatus[ubCamNum].ulCAM_ID)
		return;
	switch(ubRptType)
	{
		case PER_FRMSTS_RPT:
		case PER_FRMSEQ_RPT:
		case PER_FPS_RPT:
		case PER_RFBW_RPT:
		case PER_SPRFSTS_RPT:
			if(PER_FRMSTS_RPT == ubRptType)
			{
				osMutexWait(osUI_PerDbgMutex, osWaitForever);
				++ulUI_FrameErrCnt[ubCamNum][uwValue];
				osMutexRelease(osUI_PerDbgMutex);
			}
			if((SINGLE_VIEW == tCamViewSel.tCamViewType) && (((UI_CamNum_t)ubCamNum != tCamViewSel.tCamViewPool[0])))
				return;
			tWorkAct.uwFunc 	= UI_PERDBGRPT_ACT;
			tWorkAct.ubCamNum 	= ubCamNum;
			tWorkAct.ubType 	= ubRptType;
			tWorkAct.uwFuncMsg  = uwValue;
			osMessagePut(osUI_FuncsExecQue, &tWorkAct, 0);
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_EnPerDebugMode(void)
{
	uint32_t ulIdTemp;
	static uint8_t ubUI_LdPerDbgOsdImgFlag = FALSE;
	UI_CamNum_t tCamNum;
	UI_CUReqCmd_t tPerDbgCmd;
	UI_Result_t tPerDbgNotifyRet = rUI_SUCCESS;
	uint8_t ubCnt;
	
	if(!ubUI_TrxDebugUpdateFlg[TRX_DBG_FRMSEQ_IDX])
		ubUI_TrxDebugUpdateFlg[TRX_DBG_FRMSEQ_IDX] = 1;
	if(!ubUI_TrxDebugUpdateFlg[TRX_DBG_BW_IDX])
		ubUI_TrxDebugUpdateFlg[TRX_DBG_BW_IDX] = 1;
	if(!ubUI_TrxDebugUpdateFlg[TRX_DBG_FPS_IDX])
		ubUI_TrxDebugUpdateFlg[TRX_DBG_FPS_IDX] = 1;

	if((APP_LOSTLINK_STATE == tUI_SyncAppState) ||
	   (UI_DISPLAY_STATE != tUI_State))
		return;
	for(tCamNum = CAM1; tCamNum < tUI_CuSetting.ubTotalCamNum; tCamNum++)
	{
		for(ubCnt = 0; ubCnt < MAX_DBG_ITEM; ubCnt++)
			ulUI_FrameErrCnt[tCamNum][ubCnt] = 0;
	}
	if(FALSE == ubUI_LdPerDbgOsdImgFlag)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_CK_NUM0,    11, &tUI_PerDbgOsdImgInfo[0]);
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_TRXERR1TYPE, 6, &tUI_PerDbgOsdImgInfo[11]);
		ubUI_LdPerDbgOsdImgFlag = TRUE;
	}
	ubUI_PerDebugEn	= !ubUI_PerDebugEn;
	KNL_EnPerDebugMode(((ubUI_PerDebugEn)?KNL_PERDBG_ON:KNL_PERDBG_OFF), UI_PerReportFunc);
	if(FALSE == ubUI_PerDebugEn)
	{
		OSD_IMG_INFO tOsdImgInfo;

		tOsdImgInfo.uwHSize  = uwOSD_GetHSize();
		tOsdImgInfo.uwVSize  = uwOSD_GetVSize();
		tOsdImgInfo.uwXStart = 0;
		tOsdImgInfo.uwYStart = 0;
		OSD_EraserImg1(&tOsdImgInfo);
		tUI_CuSetting.IconSts.ubDrawStsIconFlag = FALSE;
		UI_ClearCamConnectStatusFlag();
	}
	for(tCamNum = CAM1; tCamNum < tUI_CuSetting.ubTotalCamNum; tCamNum++)
	{
	#if (defined(BUC_CU) && APP_DUAL_HOST_ENABLE)
		if(tCamNum == CAM1)
			ulIdTemp = tUI_CamStatus[tCamNum].ulCAM_ID;
		else
			ulIdTemp = INVALID_ID;
	#else
		ulIdTemp = tUI_CamStatus[tCamNum].ulCAM_ID;
	#endif	
		
		//if((INVALID_ID == tUI_CamStatus[tCamNum].ulCAM_ID) ||
		if((INVALID_ID == ulIdTemp) ||
		   (CAM_OFFLINE == tUI_CamStatus[tCamNum].tCamConnSts))
			continue;
		tPerDbgCmd.tDS_CamNum 				= tCamNum;
		tPerDbgCmd.ubCmd[UI_TWC_TYPE]		= UI_SETTING;
		tPerDbgCmd.ubCmd[UI_SETTING_ITEM]   = UI_PERDBGMODE_SETTING;
		tPerDbgCmd.ubCmd[UI_SETTING_DATA]   = ubUI_PerDebugEn;
		tPerDbgCmd.ubCmd_Len  				= 3;
		tPerDbgNotifyRet = UI_SendRequestToCAM(osThreadGetId(), &tPerDbgCmd);
		if(rUI_SUCCESS != tPerDbgNotifyRet)
			printd(DBG_ErrorLvl, "CAM%d:Per Debug Notify Fail !\n", (tCamNum + 1));
	}
}
//------------------------------------------------------------------------------
#ifdef S2019A
void UI_SetSPRfWorkCh(void)
{
	OSD_IMG_INFO tChOsdImgInfo[11];
	uint8_t ubTen = 0, ubUnit = 0;

	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_CK_NUM0, 11, &tChOsdImgInfo[0]);
	tChOsdImgInfo[10].uwXStart = 500;
	tChOsdImgInfo[10].uwYStart = uwOSD_GetVSize() - tChOsdImgInfo[10].uwVSize;
	tOSD_Img2(&tChOsdImgInfo[10], OSD_QUEUE);
	ubTen  = ubUI_sPRfChSel / 10;
	ubUnit = ubUI_sPRfChSel - (ubTen * 10);
	tChOsdImgInfo[ubTen].uwXStart = 500;
	tChOsdImgInfo[ubTen].uwYStart = tChOsdImgInfo[10].uwYStart - tChOsdImgInfo[10].uwVSize;
	tOSD_Img2(&tChOsdImgInfo[ubTen], OSD_QUEUE);
	tChOsdImgInfo[ubUnit].uwXStart = 500;
	tChOsdImgInfo[ubUnit].uwYStart = tChOsdImgInfo[ubTen].uwYStart - tChOsdImgInfo[ubTen].uwVSize;
	tOSD_Img2(&tChOsdImgInfo[ubUnit], OSD_UPDATE);
	tUI_State = UI_SPRF_SEL_STATE;
}
#endif	//! End of #ifdef S2019A
//------------------------------------------------------------------------------
void UI_sPRfChSelection(UI_ArrowKey_t tArrowKey)
{
#ifdef S2019A
	static uint8_t ubUI_RecordsPRfCh = 0;
	OSD_IMG_INFO tChOsdImgInfo[11];
	uint8_t ubTen = 0, ubUnit = 0;

	switch(tArrowKey)
	{
		case UP_ARROW:
			if((ubUI_sPRfChSel + 1) > 14)
				return;
			ubUI_RecordsPRfCh = ubUI_sPRfChSel;
			ubUI_sPRfChSel++;
			break;
		case DOWN_ARROW:
			if(!ubUI_sPRfChSel)
				return;
			ubUI_RecordsPRfCh = ubUI_sPRfChSel;
			ubUI_sPRfChSel--;
			break;
		case ENTER_ARROW:
			if((ubUI_RecordsPRfCh == ubUI_sPRfChSel) && (ubUI_sPRfChSel))
				return;
			osMutexWait(osUI_PerDbgMutex, osWaitForever);
			if(!ubUI_sPRfChSel)
			{
				sPRF_EnAutoSwCh(TRUE);
				sPRF_SetWorkCh(1);
			}
			else
			{
				sPRF_EnAutoSwCh(FALSE);
				sPRF_SetWorkCh(ubUI_sPRfChSel);
			}
			UI_ClearCamConnectStatusFlag();
			tChOsdImgInfo[10].uwHSize  = 40;
			tChOsdImgInfo[10].uwVSize  = uwOSD_GetVSize();
			tChOsdImgInfo[10].uwXStart = 500;
			tChOsdImgInfo[10].uwYStart = 0;
			OSD_EraserImg2(&tChOsdImgInfo[10]);
			tUI_State = UI_DISPLAY_STATE;
			osMutexRelease(osUI_PerDbgMutex);
			return;
		default:
			return;
	}
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_CK_NUM0, 10, &tChOsdImgInfo[0]);
	ubTen  = ubUI_sPRfChSel / 10;
	ubUnit = ubUI_sPRfChSel - (ubTen * 10);
	tChOsdImgInfo[ubTen].uwXStart = 500;
	tChOsdImgInfo[ubTen].uwYStart = uwOSD_GetVSize() - 40;
	tOSD_Img2(&tChOsdImgInfo[ubTen], OSD_QUEUE);
	tChOsdImgInfo[ubUnit].uwXStart = 500;
	tChOsdImgInfo[ubUnit].uwYStart = tChOsdImgInfo[ubTen].uwYStart - tChOsdImgInfo[ubTen].uwVSize;
	tOSD_Img2(&tChOsdImgInfo[ubUnit], OSD_UPDATE);
#endif
}
#if (APP_DOORPHONE_ENABLE==1)
void UI_DP_SycAPPStatus(uint8_t *st)
{
    pUI_DP_SyncAPPSt = (uint8_t*)st;
}
//------------------------------------------------------------------------------
void UI_DP_UpdateIcon(void)
{
    static uint8_t ubUI_DPUpdateIconCnt=5;
    if(--ubUI_DPUpdateIconCnt)   return;
    ubUI_DPUpdateIconCnt = 10;

    if(tUI_SyncAppState == APP_LINK_STATE)
    {
        for(UI_CamNum_t tCamNum = CAM1; tCamNum < tUI_CuSetting.ubTotalCamNum; tCamNum++)
        {
            if(pUI_DP_SyncAPPSt[tCamNum] == APP_DP_CALL)
                UI_DP_SHOWStatusIcon(tCamNum,UI_RECIMG_COLOR4,1);
            else if(pUI_DP_SyncAPPSt[tCamNum] == APP_DP_TALK)
                UI_DP_SHOWStatusIcon(tCamNum,UI_RECIMG_COLOR4,2);
            else if(pUI_DP_SyncAPPSt[tCamNum] == APP_DP_MONITOR)
                UI_DP_SHOWStatusIcon(tCamNum,UI_RECIMG_COLOR4,3);
            else if(pUI_DP_SyncAPPSt[tCamNum] == APP_DP_WAITMONITOR)
                UI_DP_SHOWStatusIcon(tCamNum,UI_RECIMG_COLOR4,4);
            else
                UI_DP_SHOWStatusIcon(tCamNum,UI_RECIMG_COLOR4,0);
        }
    }
    else if(tUI_SyncAppState == APP_LOSTLINK_STATE)
    {
        for(UI_CamNum_t tCamNum = CAM1; tCamNum < tUI_CuSetting.ubTotalCamNum; tCamNum++)
        {
            if(pUI_DP_SyncAPPSt[tCamNum] == APP_DP_WAITMONITOR)
                UI_DP_SHOWStatusIcon(tCamNum,UI_RECIMG_COLOR4,4);
            if(pUI_DP_SyncAPPSt[tCamNum] == APP_DP_IDLE)
                UI_DP_SHOWStatusIcon(tCamNum,UI_RECIMG_COLOR4,5);
        }
    }
    
    if(pUI_DP_SyncAPPSt[CAM1] == APP_DP_TALK || pUI_DP_SyncAPPSt[CAM2] == APP_DP_TALK)  ubUI_PttStartFlag = 1;
    else    ubUI_PttStartFlag = 0;
    
    UI_UpdatePushTalkIcon();
}

void UI_DP_SHOWStatusIcon(UI_CamNum_t tCamNum ,UI_RecImgColor_t tColorNum,uint8_t ubMode)
{
	uint32_t ulLcd_HSize  = uwLCD_GetLcdHoSize();
	uint32_t ulLcd_VSize  = uwLCD_GetLcdVoSize();
	uint16_t uwXOffset[7] = {[DISP_UPPER_LEFT] = 0,               [DISP_UPPER_RIGHT] = 0,
							 [DISP_LOWER_LEFT] = (ulLcd_HSize/2), [DISP_LOWER_RIGHT] = (ulLcd_HSize/2),
							 [DISP_LEFT] 	   = 400,			  [DISP_RIGHT] 		 = 400};
	uint16_t uwYOffset[7] = {[DISP_UPPER_LEFT] = 0,               [DISP_UPPER_RIGHT] = (ulLcd_VSize/2),
							 [DISP_LOWER_LEFT] = 0, 			  [DISP_LOWER_RIGHT] = (ulLcd_VSize/2),
							 [DISP_LEFT] 	   = 50, 		      [DISP_RIGHT] 		 = 50};
	UI_DisplayLocation_t tUI_DispLoc;
    uint16_t uwXStart=0,uwYStart=0;
    OSD_IMG_INFO tOsdImgInfo;

	tUI_DispLoc = ((tCamViewSel.tCamViewType == SINGLE_VIEW) || (tCamViewSel.tCamViewType == SCAN_VIEW))?DISP_LOWER_RIGHT:tUI_CamStatus[tCamNum].tCamDispLocation;

    if(tCamViewSel.tCamViewType == SINGLE_VIEW)
    {
        if(tCamNum != tUI_CuSetting.tAdoSrcCamNum) return;
    
        uwXStart += (ulLcd_VSize/2) + 400;
        uwYStart += 50;
    }
    else if(tCamViewSel.tCamViewType == DUAL_VIEW)
	{
        uwXStart += ((tUI_DispLoc==DISP_RIGHT)?(ulLcd_VSize/2):0) + uwXOffset[tUI_DispLoc];
        uwYStart += uwYOffset[tUI_DispLoc];
	}
    UI_SetRecImgColor(tColorNum);
	switch(ubMode)
	{
	    case 0:
            OSD_ImagePrintf(OSD_IMG_ROTATION_90, uwXStart, uwYStart, tUI_RecOsdImgInfo, OSD_UPDATE, "IDLE   ");
			break;
		case 1:
            OSD_ImagePrintf(OSD_IMG_ROTATION_90, uwXStart, uwYStart, tUI_RecOsdImgInfo, OSD_UPDATE, "RINGING");
			break;
		case 2:
            OSD_ImagePrintf(OSD_IMG_ROTATION_90, uwXStart, uwYStart, tUI_RecOsdImgInfo, OSD_UPDATE, "TALKING");
			break;
		case 3:
            OSD_ImagePrintf(OSD_IMG_ROTATION_90, uwXStart, uwYStart, tUI_RecOsdImgInfo, OSD_UPDATE, "MONITOR");
			break;         
        case 4:
            OSD_ImagePrintf(OSD_IMG_ROTATION_90, uwXStart, uwYStart, tUI_RecOsdImgInfo, OSD_UPDATE, "BUSY...");
			break;
        case 5:
    		tOsdImgInfo.uwHSize  = 50;
    		tOsdImgInfo.uwVSize  = (ulLcd_VSize/2);
    		tOsdImgInfo.uwXStart = uwYStart;
    		tOsdImgInfo.uwYStart = (tUI_DispLoc==DISP_RIGHT)?0:(ulLcd_VSize/2);
    		OSD_EraserImg2(&tOsdImgInfo);            
            break;
		default:
			break;
	}
}

void UI_DP_KEY1(void)
{
    UI_CamNum_t tCamNum = CAM1;
    if(pUI_DP_SyncAPPSt[tCamNum] == APP_DP_IDLE || pUI_DP_SyncAPPSt[tCamNum] == APP_DP_WAITMONITOR)
        UI_DP_Monitor(tCamNum);
    else if(pUI_DP_SyncAPPSt[tCamNum] == APP_DP_CALL)
        UI_DP_Answer(tCamNum);
    else if(pUI_DP_SyncAPPSt[tCamNum] == APP_DP_MONITOR || pUI_DP_SyncAPPSt[tCamNum] == APP_DP_TALK)
        UI_DP_Handup(tCamNum);
}

void UI_DP_KEY2(void)
{
    UI_CamNum_t tCamNum = CAM2;
    if(pUI_DP_SyncAPPSt[tCamNum] == APP_DP_IDLE || pUI_DP_SyncAPPSt[tCamNum] == APP_DP_WAITMONITOR)
        UI_DP_Monitor(tCamNum);
    else if(pUI_DP_SyncAPPSt[tCamNum] == APP_DP_CALL)
        UI_DP_Answer(tCamNum);
    else if(pUI_DP_SyncAPPSt[tCamNum] == APP_DP_MONITOR || pUI_DP_SyncAPPSt[tCamNum] == APP_DP_TALK)
        UI_DP_Handup(tCamNum);
}

UI_Result_t UI_DP_Monitor(UI_CamNum_t tCamNum)
{
	APP_EventMsg_t tUI_DP_Message = {0};
#if(APP_DP_TXPS_MODE==1)
	tUI_DP_Message.ubAPP_Event 	    = APP_POWERSAVE_EVENT;
	tUI_DP_Message.ubAPP_Message[0]  = 4;		//! Message Length
	tUI_DP_Message.ubAPP_Message[1]  = PS_ECO_MODE;
	tUI_DP_Message.ubAPP_Message[2]  = TRUE;
	tUI_DP_Message.ubAPP_Message[3]  = tCamNum;
	tUI_DP_Message.ubAPP_Message[4]  = TRUE;
	UI_SendMessageToAPP(&tUI_DP_Message);
	ubUI_WakeUpFromPsFlag = TRUE;
#else
    tUI_DP_Message.ubAPP_Event = APP_DP_EVENT;
	tUI_DP_Message.ubAPP_Message[0] = 2;		//! Message Length
	tUI_DP_Message.ubAPP_Message[1] = APP_DP_CMD_MONITORREQ;
    tUI_DP_Message.ubAPP_Message[2] = tCamNum;
	UI_SendMessageToAPP(&tUI_DP_Message);    
#endif
	return rUI_SUCCESS;
}
//------------------------------------------------------------------------------
void UI_DP_Answer(UI_CamNum_t tCamNum)
{
	APP_EventMsg_t tUI_DP_Message = {0};

	if(APP_LOSTLINK_STATE == tUI_SyncAppState)
		return;

    if(UI_DISPLAY_STATE == tUI_State)
    {
        ubUI_PttStartFlag = 0;
		UI_UpdatePushTalkIcon();
    }
    ubUI_PttStartFlag = 1;
    tUI_CuSetting.tAdoSrcCamNum = tCamNum;
	tUI_DP_Message.ubAPP_Event = APP_DP_EVENT;
	tUI_DP_Message.ubAPP_Message[0] = 3;		//! Message Length
	tUI_DP_Message.ubAPP_Message[1] = APP_DP_CMD_TALK;
    tUI_DP_Message.ubAPP_Message[2] = tCamNum;
    tUI_DP_Message.ubAPP_Message[3] = ubUI_PttStartFlag;    
	UI_SendMessageToAPP(&tUI_DP_Message);
}

void UI_DP_Handup(UI_CamNum_t tCamNum)
{
	APP_EventMsg_t tUI_DP_Message = {0};
    uint8_t ubUI_SWAdoFg=0;

	if(APP_LOSTLINK_STATE == tUI_SyncAppState)
		return;

    ubUI_PttStartFlag = 0;
    UI_UpdatePushTalkIcon();
    if( tCamNum == CAM1 && pUI_DP_SyncAPPSt[CAM2] == APP_DP_TALK )
    {
        ubUI_PttStartFlag = 1;
        ubUI_SWAdoFg = 1;
        tUI_CuSetting.tAdoSrcCamNum = CAM2;
    }
    else if(tCamNum == CAM2 && pUI_DP_SyncAPPSt[CAM1] == APP_DP_TALK)
    {
        ubUI_PttStartFlag = 1;
        ubUI_SWAdoFg = 1;
        tUI_CuSetting.tAdoSrcCamNum = CAM1;
    }
    
	tUI_DP_Message.ubAPP_Event = APP_DP_EVENT;
	tUI_DP_Message.ubAPP_Message[0] = 4;		//! Message Length
	tUI_DP_Message.ubAPP_Message[1] = APP_DP_CMD_HANDUP;
    tUI_DP_Message.ubAPP_Message[2] = tCamNum;
    tUI_DP_Message.ubAPP_Message[3] = ubUI_SWAdoFg;
    tUI_DP_Message.ubAPP_Message[4] = tUI_CuSetting.tAdoSrcCamNum;
	UI_SendMessageToAPP(&tUI_DP_Message);
}
#endif
#endif	//! End of #if (defined(BSP_SN93711_FHD_REC_RX_V4)||defined(BSP_D_SN93701_SSD2828_RX_V5))
