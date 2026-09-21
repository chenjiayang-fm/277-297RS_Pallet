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
	\version	1.3
	\date		2020/06/24
	\copyright	Copyright (C) 2020 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#include "BSP.h"

#ifdef BSP_D_SNCC71_GM8285C_RX_V2

#include <string.h>
#include "UI_BUCCU[WSVGA].h"
#include "SF_API.h"
#include "EN_API.h"
#include "FWU_API.h"
#include "TIMER.h"
#include "VDO.h"
#include "Buzzer.h"
#include "PLY_API.h"
#include "REC_API.h"
#include <math.h>
#include "UART4AI.h"

#define osUI_SIGNALS	0x66
#define AI_LAMP_UNKNOWN 2 // 灯控应答失败时状态未知，下次继续同步开关状态。

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
	{AKEY_MENU, 		0,			UI_MenuKey, 				NULL},
	{AKEY_LEFT, 		0,			UI_LeftArrowKey,			NULL},
	{AKEY_RIGHT,		0,			UI_RightArrowKey,			NULL},
	{AKEY_OK,			0,			UI_EnterKey,				NULL},
	{AKEY_SEL,			0,			UI_SelKey,					NULL},
	{AKEY_POWER,		0,			UI_PowerKey,				NULL},
	{AKEY_POWER,		20, 		UI_PowerLongKey,			NULL},
	//20201222 IR
	{REMO_MENU, 		0,			UI_MenuKey, 				NULL},
	{REMO_OK,			0,			UI_EnterKey,				NULL},
	{REMO_LEFT, 		0,			UI_LeftArrowKey,			NULL},
	{REMO_RIGHT,		0,			UI_RightArrowKey,			NULL},
	{REMO_POWER,		0,			UI_PowerKey,				NULL},
	{REMO_PAIRING,		0,			UI_PairingKey,				NULL},
	{REMO_SEL,			0,			UI_SelKey,					NULL},
	{REMO_MIRROR,		0,			UI_MirrorKey,				NULL},
	{REMO_MUTE, 		0,			UI_MuteKey, 				NULL},
	{REMO_VERSION, 		0,			UI_VersionKey, 				NULL},
	{REMO_POWER, 		20,			UI_PowerLongKey, 			NULL},
	
};
UI_State_t tUI_State;
APP_State_t tUI_SyncAppState;
UI_CamStatus_t tUI_CamStatus[CAM_4T];
UI_CUSetting_t tUI_CuSetting;
uint8_t ISPlaying_wav = 0;
uint8_t Playwav_Flag = 0;
volatile uint32_t Playwav_Count = 0;
static uint8_t Playwav_Switch = 0;
volatile uint8_t ubAIAlarmLevel[4] = {0};
volatile uint16_t uwAIAlarmTimeout[4] = {0};
volatile uint16_t uwAILampTimeout[4] = {0};
static uint8_t ubAILaserState[4] = {0};
static uint8_t ubAILedState[4] = {0};
volatile uint8_t ubAIConfigSync = TRUE;
static volatile uint16_t uwAIConfigSyncCount = 0;
static osMutexId osAIConfigMutex;
static osMutexId osUI_CamCmdMutex; // 共用 tosUI_Notify 的摄像头命令必须串行发送并等待应答。
uint8_t tUI_RecCutOFF[4] = 0;
//BSD RANGE
//UI_ParkinglinePoint_t tUI_ParkinglinePoint[4]={0};

extern uint8_t UI_GetCurrentVolumeLevel(void);

const static UI_MenuFuncPtr_t tUI_StateMap2MenuFunc[UI_STATE_MAX] =
{
	[UI_DISPLAY_STATE]				= UI_DisplayArrowKeyFunc,
	[UI_MAINMENU_STATE] 			= UI_HomeMenu_Key,
	[UI_SUBMENU_STATE]				= UI_SubKeyMenu,
	[UI_SUBSUBMENU_STATE]			= UI_SubSubKeyMenu,
	[UI_SUBSUBSUBMENU_STATE]		= UI_SubSubSubKeyMenu,
	[UI_SUBSUBSUBSUBMENU_STATE] 	= UI_SubSubSubSubKeyMenu,
	[UI_SET_VOLUME_STATE]			= UI_VolumeMenuPage,
	[UI_SET_CUPSMODE_STATE] 		= NULL,
	[UI_SET_CAMECOMODE_STATE]		= NULL,
	[UI_CAMSETTINGMENU_STATE]		= NULL, 
	[UI_SET_CAMCOLOR_STATE] 		= NULL,//UI_SUBSUBMENU_STATE
	[UI_DPTZ_CONTROL_STATE] 		= NULL,
	[UI_MD_WINDOW_STATE]			= NULL, 
	[UI_PAIRING_STATE]				= UI_PairingControl,	
	[UI_SDFWUPG_STATE]				= NULL,
	
	[UI_RECFOLDER_SEL_STATE]		= NULL,//UI_SUBMENU_STATE
	[UI_RECFILES_SEL_STATE] 		= NULL,//UI_SUBSUBMENU_STATE
	[UI_RECPLAYLIST_STATE]			= NULL,//UI_SUBSUBSUBMENU_STATE
	[UI_RECPLAYADOSRC_SEL_STATE]	= NULL,
	[UI_RECPLAYDISPTYPE_SEL_STATE]	= UI_PlayDispTypeSelection,

	[UI_PHOTOPLAYNRDY_STATE]		= NULL,
	[UI_PHOTOPLAYLIST_STATE]		= NULL,
	[UI_SDCARDFMT_STATE]			= NULL,//UI_SUBSUBMENU_STATE
	[UI_ENGMODE_STATE]				= UI_EngModeCtrl,
	[UI_SHOW_SDCARD_INFO_STATE] 	= UI_ShowSdCardInfo,
	[UI_SHOW_TX_VERSION_STATE] 		= UI_ShowTxVersion,
};


//-----------------------------------------------TOUCH FUNCTION 20201230-----------------------------------------------------
const static UI_MenuTouchFuncPtr_t tUI_StateMap2TouchMenuFunc[UI_STATE_MAX] =
{
	[UI_DISPLAY_STATE]				= UI_DisplayTouchFunc,
	[UI_MAINMENU_STATE] 			= UI_HomeMenuTouchFunc,
	[UI_SUBMENU_STATE]				= UI_SubTouchMenu,
	[UI_SUBSUBMENU_STATE]			= UI_SubSubTouchMenu,
	[UI_SUBSUBSUBMENU_STATE]		= UI_SubSubSubTouchMenu,
	[UI_SUBSUBSUBSUBMENU_STATE] 	= UI_SubSubSubSubTouchMenu,
	[UI_SET_VOLUME_STATE]			= UI_VolumeMenuTouchFunc,
	[UI_SET_CUPSMODE_STATE] 		= NULL,
	[UI_SET_CAMECOMODE_STATE]		= NULL,
	[UI_CAMSETTINGMENU_STATE]		= NULL, 
	[UI_SET_CAMCOLOR_STATE] 		= NULL,
	[UI_DPTZ_CONTROL_STATE] 		= NULL,
	[UI_MD_WINDOW_STATE]			= NULL, // UI_MD_Window
	[UI_PAIRING_STATE]				= UI_PairingTouchControl,
	[UI_SDFWUPG_STATE]				= NULL,
	[UI_RECFOLDER_SEL_STATE]		= NULL,
	[UI_RECFILES_SEL_STATE] 		= NULL,
	[UI_RECPLAYLIST_STATE]			= NULL,
	[UI_RECPLAYADOSRC_SEL_STATE]	= NULL,
	[UI_RECPLAYDISPTYPE_SEL_STATE]	= UI_PlayDispTypeTouchSelection,
	[UI_PHOTOPLAYNRDY_STATE]		= NULL,
	[UI_PHOTOPLAYLIST_STATE]		= NULL,
	[UI_SDCARDFMT_STATE]			= NULL,
	[UI_ENGMODE_STATE]				= NULL,
	[UI_SHOW_SDCARD_INFO_STATE] 	= UI_ShowSdCardInfoTouchFunc,
	[UI_SHOW_TX_VERSION_STATE] 		= UI_ShowTxVersionTouchFunc,
};

UI_MenuItem_t tUI_MenuItem;
//------------------------------------------------------------------------------sub
UI_SubMenuItem_t tUI_SubMenuItem[MENUITEM_MAX] =
{
	{0,	5	},			//ado sel
	{CAM1,VDO_VIEW_MAX	},		//vdo sel
	{PAIRINGCAM1_ITEM,	PAIRING_ITEM_MAX	},//pairing
	{NULL,				NULL},//record
	{NULL,				NULL			},//playback
	{CAMERASET_ITEM, 	SETTINGITEM_MAX},//system
	{POWER_SCREENOFF,	POWER_ITEM_MAX}//power
};
//------------------------------------------------------------------------------sub sub 

UI_SubMenuItem_t tSystemSubSubMenuItem = 
{
	DIMMER_ITEM,SYSTEMITEM_MAX,{0,0}
};
UI_SubMenuItem_t tDualSubSubMenuItem = 
{
	DUAL_LEFT_CAM1,DUAL_MAX,{0,0}
};
UI_SubMenuItem_t tQuadSubSubMenuItem = 
{
	QUAD_UPPER_LEFT_CAM1,QUAD_MAX,{0,0}
};

UI_SubMenuItem_t tPowerOnSubSubMenuItem = 
{
	POWERON_CAM1,POWERON_MAX - 1,{0,0}
};

UI_SettingSubSubMenuItem_t tSettingSubSubMenuItem =
{
	{
	   { CAM1, CAM_4T, { 0, 0 } },      		//camera 
	   { RECTIME_1MIN_ITEM, RECITEM_MAX, { 0, 0 } }, //record
	   { DIMMER_ITEM, SYSTEMITEM_MAX, { 0, 0 } },//system
	   { 0, 0, { 0, 0 } },		//dual
	   { 0, 0, { 0, 0 } }, //quad
	   { TRIGGER_DELAY_CAM1, TRIGGER_MAX, { 0, 0 } },	//trigger
	   { AUTOSCAN_ONOFF_CAM1, AUTOSCAN_MAX, { 0, 0 } },//auto scan
	   { 0,0, { 0, 0 } },//power on
	   { PARKINGLINE_CAM1_ONOFF, PARKINGLINE_MAX, { 0, 0 } },//parking line
	   { GUIDELINE_CAM1_ONOFF, GUIDELINE_MAX, { 0, 0 } },//Guide line
	   { 0, AI_MAX, { 0 , 0 }},//AI
	 },
};
//------------------------------------------------------------------------------sub sub sub
/********************camera************************/
uint8_t CameraSubSubSubRemoteMode = REMOTE_SELECT_ITEM;

static UI_SubMenuItem_t tCameraCam1MenuItem = 
{
	0,CAMERA_MAX,{0,0}
};
static UI_SubMenuItem_t tCameraCam2MenuItem = 
{
	0,CAMERA_MAX,{0,0}
};
static UI_SubMenuItem_t tCameraCam3MenuItem = 
{
	0,CAMERA_MAX,{0,0}
};
static UI_SubMenuItem_t tCameraCam4MenuItem = 
{
	0,CAMERA_MAX,{0,0}
};

UI_SubMenuItem_t *tCameraSubSubSubMenuItem[] = 
{
	&tCameraCam1MenuItem,
	&tCameraCam2MenuItem,
	&tCameraCam3MenuItem,
	&tCameraCam4MenuItem	
};

/********************system************************/
uint8_t SystemSubSubSubRemoteMode = REMOTE_SELECT_ITEM;

UI_SubMenuItem_t tSystemSubSubSubMenuItem[] = 
{
	{ 0, 0, { 0, 0 } },
	{ 0, 0, { 0, 0 } },
	{ DAY_SLIDER, DIMMER_SLIDER_MAX, { 0, 0 } },
	{ 0, 0, { 0, 0 } },
	{ SYSTEM_UPGRADE_YES, SYSTEM_UPGRADE_MAX, { 0, 0 } },
	{ LANGUAGE_ENGLISH, LANGUAGE_MAX, { 0, 0 } },
	{ 0, 0, { 0, 0 } },
	{ 0, 0, { 0, 0 } },
	{ 0, 0, { 0, 0 } },
	{ 0, 0, { 0, 0 } },
	{ DATE_TIME_YEAR, DATE_TIME_MAX, { 0, 0 } },
};
/********************setting************************/
uint8_t SettingSubSubSubRemoteMode = REMOTE_SELECT_ITEM;

//------------------------------------------------------------------------------sub sub sub sub
/********************playback************************/

UI_MenuItem_t tUI_RecPlayListItem = {UI_RECPLAYPAUSE_ITEM,UI_RECPLAYPAUSE_ITEM};

UI_MenuItem_t tUI_PlyDispTypeSelItem;

UI_MenuItem_t tUI_RecPlayAdoSrcItem;


/********************date time************************/
UI_SubMenuItem_t tDateTimeSubSubSubSubMenuItem = 
{
	KEYBOARD_KEY1,KEYBOARD_KEY_MAX,{0,0}
};
/*******************AI*********************************/
uint8_t AISubSubSubRemoteMode = REMOTE_SELECT_ITEM;

UI_SubMenuItem_t tAISubSubSubMenuItem = 
{
	BSDRANGE_ENABLE_ITEM,AIITEM_MAX,{0,0}
};

const UI_ReportFuncPtr_t tUiReportMap2Func[] =
{
	[UI_UPDATE_CAMSTS] 			= UI_UpdateCamStatus,
	[UI_VOX_TRIG]				= NULL,
	[UI_MD_TRIG]				= NULL,
	[UI_VOICE_TRIG]				= UI_VoiceTrigger,
};

ADO_R2R_VOL tUI_VOLTable[] = {R2R_VOL_n45DB, R2R_VOL_n32p4DB, R2R_VOL_n26p2DB, R2R_VOL_n21p4DB, R2R_VOL_n14p6DB, R2R_VOL_n8p2DB};

osSemaphoreId osUI_CuSemId;
osSemaphoreId osUI_CuUiCtr;//����ʾSUB�����ʱ�򣬱����ź�sd����Щ�ᶯ̬ˢ�µ�ͼ��Ӱ��
osSemaphoreId osUI_CuRecCtr;
osSemaphoreId osUI_CuTriggerCtr;
osSemaphoreId osUI_PowerLightCtr;
osSemaphoreId osUI_UpdateStatusCtr;

UI_CamViewSelect_t tCamViewSel;
UI_PairingInfo_t tPairInfo;
static UI_ThreadNotify_t tosUI_Notify;
UI_CamNum_t tUI_ViewModeSel;	//vdo mode�����ѡ��?
UI_CamNum_t tUI_SaveViewNum;//��������������ת�Ķ�ָ��?

UI_CamNum_t tUI_AutoScanNumSel;//��ǰ�Զ�ɨ��ɨ���ĸ�����

uint8_t MenuOnFlag = FALSE;//��ʾ�����˵����?
uint8_t TriggerLock = FALSE;//ΪTRUE��ʾ��ǰ���ڴ���״̬
uint8_t TriggerSource = 0xff;//��ǰ����Դ

UI_CamNum_t DeskTopShowView;//DeskTop��ǰ��ʾ�Ļ��棬����tUI_ViewNumSel֮���Ի���Ҫ�ñ�������ΪtUI_ViewNumSel����ŵ�ǰѡ��view�����ܱ�

static uint8_t ubCamDiscCount[4] = {0};//��������ÿ��tx��ʧ���ӵĴ�����ֻ�м���ֵ�ﵽ�趨�ĲŻ�ˢno signal

UI_SdSts_t tUI_SdCardSts;
static uint8_t ubUI_CuStartUpFlag;


uint16_t uwUI_ViewSwitchTime;//�����л���ʱ

uint8_t ubUI_FinishViewSwitch;//�����л��Ƿ��Ѿ�����
uint8_t ubUI_RestartRec;//�����л����Ƿ�����¼��
uint8_t ubUI_ReOpenPanel;//�����л����Ƿ�����?


uint8_t ubUI_CuStandbyFlag;//����״̬
uint8_t ubUI_CuPowerOffFlag;//˯��

uint8_t ubUI_CuPowerDiscFlag;//��Դ�Ƿ�Ͽ�?
uint8_t ubUI_CuFWU_1126Flag = 0;
uint8_t ubUI_CuSemaphorewaitFlag = 0;//切换通道后，拿到锁之后去释放�?
uint8_t ubZoomPairingFlag = FALSE; //����pairing��ˢcamera��־
uint32_t ubZoomPairingCount = 1000; //����pairing��ˢcameraʱ�� ms



uint8_t ubUI_CurDimmerLvl;//��ǰ��������


static uint8_t ubUI_StopUpdateStsBarFlag;

UI_CharOsdImgDb_t tCharOsdImgDB;

uint32_t ulMenuAutoOffCount;//�˳��˵�����ʱ
uint32_t ulAutoScanCount;//�Զ�ɨ�軭�浹��ʱ
uint32_t ulPanelOffCount;//Ϣ������ʱ
uint32_t ulTriggerOverCount;//������������ʱ
uint32_t ulStartRecCompleteCount;//����¼����ɵ����?
uint16_t uwCurVideoPlayIdx[4];//��ǰ���ŵ���Ƶ

uint8_t ubAutoScanReady = TRUE;
uint8_t ubDrawBSDRange = 1;
uint8_t ubDrawingBSD = 0;
uint8_t Drawing_BoxFlag[4] = 0;

UI_CamNum_t TriggerBackUp_View;

static uint16_t uwTxType[4] = {0};
static uint8_t ubTxTypeCount[4] = {0};
osMutexId osUI_TxTypeMutex;
osMutexId osPlayVolumeFlag;
osMutexId osEnterVolumeFlag;


uint8_t ubUI_TouchPanelSts = TRUE;

//! Record
static void UI_OsdLoadingDisplayThread(void const *argument);
osThreadId osUI_OsdLdDispThdId;
osMessageQId osUI_OsdLdDispQueue;
osMessageQId osUI_RecRptQueue;
osSemaphoreId osUI_OsdLdStsUpdSem;
static UI_OsdLdDispSts_t tUI_OsdLdDispSts;
UI_RecFoldersInfo_t tUI_RecFoldersInfo;
UI_RecFilesInfo_t tUI_RecFilesInfo;
static UI_RecOsdImgDb_t tUI_RecOsdImgDB;
OSD_IMGIDXARRARY_t tUI_RecOsdImgInfo;
OSD_IMGIDXARRARY_t tUI_CharOsdImgInfo;
UI_RecPlayAct_t tUI_RecPlayAct;
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

#define UI_HVIEW_ICON_XOFFSET		756

char zoomchar[] = "-Z-";
char Laserchar[] = "LAS";
/*PELCO code*/
                                                                               // sync  addr  cmd1  cmd2  data1 data2 checksum
uint8_t pelco_out[]  = {/*0xAA, 0xFF, 0xE9, 0x4F, 0x00, 0x04, 0x00, 0x00, 0x07,*/ 0xFF, 0x00, 0x00, 0x40, 0x00, 0x00, 0x40};
uint8_t pelco_in[]   = {/*0xAA, 0xFF, 0xC1, 0x51, 0x00, 0x04, 0x00, 0x00, 0x07,*/ 0xFF, 0x00, 0x00, 0x20, 0x00, 0x00, 0x20};
uint8_t pelco_stop[] = {/*0xAA, 0xFF, 0xD9, 0x5B, 0x00, 0x04, 0x00, 0x00, 0x07,*/ 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

uint8_t pelco_near[]    = {0xFF, 0x91, 0x00, 0x00, 0x00, 0x00, 0x91};  
uint8_t pelco_far[]     = {0xFF, 0x91, 0x00, 0x01, 0x00, 0x00, 0x92};
uint8_t pelco_auto[]    = {0xFF, 0x90, 0x00, 0x00, 0x00, 0x00, 0x90}; 
uint8_t pelco_restart[] = {0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF};

uint8_t pelco_digital_zoom[7] = {0xFF, 0x92, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8_t pelco_BSD[]   = {0xFF, 0xA0, 0x00, 0x00, 0x00, 0x00, 0xA0, \
                         0xFF, 0xA0, 0x00, 0x01, 0x00, 0x00, 0xA1};
uint8_t pelco_Trace[] = {0xFF, 0xB0, 0x00, 0x00, 0x00, 0x00, 0xB0, \
	                     0xFF, 0xB0, 0x00, 0x01, 0x00, 0x00, 0xB1};

uint8_t CheckSensorStatus[] = {0xFF, 0x41, 0x00, 0x00, 0x00, 0x00, 0x41};
uint8_t CheckAlarmRoi[]     = {0xFF, 0x43, 0x00, 0x00, 0x00, 0x00, 0x43};
uint8_t CheckPictureMirrorFlip[]     = {0xFF, 0x47, 0x00, 0x00, 0x00, 0x00, 0x47};


uint8_t Picture_Mirror_Flip[] = {0xFF, 0x46, 0x00, 0x00, 0x00, 0x00, 0x46};

#define BACKLIGHT_ADC_VALUE 124

//------------------------------------------------------------------------------
//20201229
void UI_TouchEventExec(void *pvTouchEvent)
{
	TOUCH_EVENT_t *ptTouchEvent = (TOUCH_EVENT_t*)pvTouchEvent;

	if(tUI_StateMap2TouchMenuFunc[tUI_State].pvFuncPtr)
		tUI_StateMap2TouchMenuFunc[tUI_State].pvFuncPtr(ptTouchEvent);
}

void UI_KeyEventExec(void *pvKeyEvent)
{
	static uint8_t ubUI_KeyEventIdx = 0;
	static uint8_t ubCount = 0;
	KEY_Event_t *ptKeyEvent;
	uint16_t uwUiKeyEvent_Cnt = 0, uwIdx;
	ptKeyEvent = (KEY_Event_t *)pvKeyEvent;
	uwUiKeyEvent_Cnt = sizeof UiKeyEventMap / sizeof(UI_KeyEventMap_t);
	if(ptKeyEvent->ubKeyAction == KEY_UP_ACT)
	{
		if((ubUI_KeyEventIdx) && (ubUI_KeyEventIdx < uwUiKeyEvent_Cnt))
		{
			if(UiKeyEventMap[ubUI_KeyEventIdx].KeyEventFuncPtr)
			{
//				if ((ptKeyEvent->ubKeyID >= AKEY_POWER && ptKeyEvent->ubKeyID <= AKEY_SEL) && (!ubUI_CuStandbyFlag))
//				{
//					BUZZER_DI(150);
//				}
				
				UiKeyEventMap[ubUI_KeyEventIdx].KeyEventFuncPtr();
				if(UiKeyEventMap[ubUI_KeyEventIdx].pvKeyTone && tUI_CuSetting.ubBuzzer)
					UiKeyEventMap[ubUI_KeyEventIdx].pvKeyTone();
			}
		}
		ubUI_KeyEventIdx = 0;
		ubCount = 0;
		return;
	}
	for(uwIdx = 1; uwIdx < uwUiKeyEvent_Cnt; uwIdx++)
	{
		//��ѹʱ��ﵽ�趨ֵ�ͻ���룬������趨ֵ��?
		if((ptKeyEvent->ubKeyID  == UiKeyEventMap[uwIdx].ubKeyID) &&
		   (ptKeyEvent->uwKeyCnt == UiKeyEventMap[uwIdx].uwKeyCnt))
		{	
			printf("KEY ID is :0x%x\n",ptKeyEvent->ubKeyID);
			ubUI_KeyEventIdx = uwIdx;
			if((ptKeyEvent->uwKeyCnt) && (UiKeyEventMap[ubUI_KeyEventIdx].KeyEventFuncPtr))//������������Ӧ
			{
				UiKeyEventMap[ubUI_KeyEventIdx].KeyEventFuncPtr();
				if(UiKeyEventMap[ubUI_KeyEventIdx].pvKeyTone && tUI_CuSetting.ubBuzzer)
					UiKeyEventMap[ubUI_KeyEventIdx].pvKeyTone();
				ubUI_KeyEventIdx = 0;
			}
		}
		
	}
	//���Ҽ�����
	if((ptKeyEvent->ubKeyID == AKEY_LEFT || ptKeyEvent->ubKeyID == AKEY_RIGHT) && SendIrCodeFlag == TRUE)
	{
		if(ubCount < 5)//�ӳ�500ms����Ӧ����
			ubCount ++;
		else
		{
			for(uwIdx = 1; uwIdx < uwUiKeyEvent_Cnt; uwIdx++)
			{
				if(ptKeyEvent->ubKeyID	== UiKeyEventMap[uwIdx].ubKeyID)
				{			
					if(UiKeyEventMap[ubUI_KeyEventIdx].KeyEventFuncPtr)
					{
						UiKeyEventMap[ubUI_KeyEventIdx].KeyEventFuncPtr();
					}
				}
				
			}
		}
	}
}

//------------------------------------------------------------------------------

void UI_1MsTimer(void)
{
	if(ulMenuAutoOffCount && (tUI_State != UI_PAIRING_STATE) 
		&& !((tUI_State == UI_SUBSUBSUBMENU_STATE || tUI_State == UI_SUBSUBSUBSUBMENU_STATE) && tUI_MenuItem.ubItemIdx == PLAYBACK_ITEM))
	{
		ulMenuAutoOffCount --;
	}
	
	if(ulAutoScanCount && !MenuOnFlag && !ubUI_CuStandbyFlag && !TriggerLock && !ubUI_CuPowerDiscFlag)// && ubUI_FinishViewSwitch)
	{
		ulAutoScanCount --;
	}
	if(ulPanelOffCount && !MenuOnFlag && !TriggerLock)
	{
		ulPanelOffCount --;
	}
	if(ulTriggerOverCount && TriggerLock && !MenuOnFlag && ubUI_FinishViewSwitch && !ubUI_CuPowerDiscFlag)
	{
		ulTriggerOverCount --;
	}
	if(ulStartRecCompleteCount)
	{
		ulStartRecCompleteCount --;
		if(ulStartRecCompleteCount == 0)
		osSemaphoreRelease(osUI_CuRecCtr);
	}
	// 毫秒回调只计时，音频播放和串口发送放到 UI 任务中执行。
	if(Playwav_Count < PLAYWAV_COUNT)
		Playwav_Count++;
	if(uwAIConfigSyncCount)
		uwAIConfigSyncCount--;
	for(uint8_t i = 0; i < CAM_4T; i++)
	{
		if(uwAIAlarmTimeout[i])
			uwAIAlarmTimeout[i]--;
		if(uwAILampTimeout[i])
			uwAILampTimeout[i]--;
	}


	if(ubZoomPairingFlag && ubZoomPairingCount)
	{
		ubZoomPairingCount --;
	}

//	if(ubPwrUartFlag && ubPwrUartCount)
//	{
//		ubPwrUartCount --;
//	}
	
	if((ubUI_FinishViewSwitch == FALSE) && (tUI_RecPlayAct.tRecAct == UI_REC_STOP))
	{
		uwUI_ViewSwitchTime ++;
		if(uwUI_ViewSwitchTime >= 5000)//ͨ���л�ʱ���������?s������
		{
			UI_Event_t tRebootEvent;
			osMessageQId *pUI_RebootEventQH = NULL;
			//超过5s没切换通道成功就重�?
			tRebootEvent.tEventType = REBOOT_EVENT;
			tRebootEvent.pvEvent 	  = NULL;
			pUI_RebootEventQH 	  = pUI_GetEventQueueHandle();
			osMessagePut(*pUI_RebootEventQH, &tRebootEvent, 0);
			
		}
	}
	else
		uwUI_ViewSwitchTime = 0;
}

//------------------------------------------------------------------------------
void UI_1MSTimerInit(void)
{
	TIMER_SETUP_t TmSetup;

	TmSetup.tCLK 		= TIMER_CLK_EXTCLK;
	TmSetup.ulTmLoad 	= 10000;
	TmSetup.ulTmCounter = TmSetup.ulTmLoad;
	TmSetup.ulTmMatch1 	= TmSetup.ulTmLoad + 1;
	TmSetup.ulTmMatch2 	= TmSetup.ulTmLoad + 1;
	TmSetup.tOF 		= TIMER_OF_ENABLE;
	TmSetup.tDIR 		= TIMER_DOWN_CNT;
	TmSetup.tEM 		= TIMER_CB;
	TmSetup.pvEvent 	= UI_1MsTimer;
	TIMER_Start(TIMER2_1, TmSetup);
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
		osThreadDef(UiFuncsExec, UI_FuncsExecuteThread, osPriorityNormal, 1, 8192);
		osUI_FuncsExecThdId = osThreadCreate(osThread(UiFuncsExec), NULL);
	}
	MenuOnFlag = FALSE;

	tCamViewSel.tCamViewType	= (VDO_DISP_TYPE == KNL_DISP_H)?H_VIEW:
								  (VDO_DISP_TYPE == KNL_DISP_QUAD)?QUAD_VIEW:
								  ((VDO_DISP_TYPE == KNL_DISP_DUAL_C) || (VDO_DISP_TYPE == KNL_DISP_DUAL_U))?DUAL_VIEW:SINGLE_VIEW;
	tCamViewSel.tCamViewPool[0] = ((VDO_DISP_TYPE == KNL_DISP_QUAD) || (VDO_DISP_TYPE == KNL_DISP_H))?CAM_4T:CAM1;
	tCamViewSel.tCamViewPool[1] = ((VDO_DISP_TYPE == KNL_DISP_DUAL_C) || (VDO_DISP_TYPE == KNL_DISP_DUAL_U))?CAM2:NO_CAM;


}

//------------------------------------------------------------------------------
void UI_StateReset(void)
{
	osMutexDef(AIConfigMutex);
	osAIConfigMutex = osMutexCreate(osMutex(AIConfigMutex));
	osMutexDef(UICamCmdMutex);
	osUI_CamCmdMutex = osMutexCreate(osMutex(UICamCmdMutex));

	osMutexDef(PlayVolumeFlag);
	osPlayVolumeFlag 	= osMutexCreate(osMutex(PlayVolumeFlag));

	osMutexDef(EnterVolumeFlag);
	osEnterVolumeFlag 	= osMutexCreate(osMutex(EnterVolumeFlag));

	osSemaphoreDef(UiCuSem);
	osUI_CuSemId = osSemaphoreCreate(osSemaphore(UiCuSem), 1);
	
	osSemaphoreDef(UiCuRecCtr);
	osUI_CuRecCtr = osSemaphoreCreate(osSemaphore(UiCuRecCtr), 1);
	
	osSemaphoreDef(UiCuTriggerCtr);
	osUI_CuTriggerCtr = osSemaphoreCreate(osSemaphore(UiCuTriggerCtr), 1);

	osSemaphoreDef(UiPowerLightCtr);
	osUI_PowerLightCtr = osSemaphoreCreate(osSemaphore(UiPowerLightCtr), 1);
	
	osSemaphoreDef(UiCuUiCtr);
	osUI_CuUiCtr = osSemaphoreCreate(osSemaphore(UiCuUiCtr), 1);

	osSemaphoreDef(UI_UpdateStatusCtr);
	osUI_UpdateStatusCtr = osSemaphoreCreate(osSemaphore(UI_UpdateStatusCtr), 1);

	osSemaphoreDef(UiOsdLdStsSem);
	osUI_OsdLdStsUpdSem = osSemaphoreCreate(osSemaphore(UiOsdLdStsSem), 1); 

	osMutexDef(UI_TxTypeSyncMutex);
	osUI_TxTypeMutex = osMutexCreate(osMutex(UI_TxTypeSyncMutex));
	
	tosUI_Notify.thread_id		 = NULL;
	tosUI_Notify.iSignals		 = 0;
	ubUI_CuStartUpFlag			 = FALSE;
	ubUI_StopUpdateStsBarFlag	 = FALSE;
	tUI_SdCardSts				 = UI_SD_CFM;
	osUI_OsdLdDispQueue 		 = NULL;
	osUI_OsdLdDispThdId 		 = NULL;
	osUI_RecRptQueue			 = NULL;
	tUI_OsdLdDispSts			 = UI_OSDLDDISP_OFF;
	osUI_FuncsExecThdId 		 = NULL;
	osUI_FuncsExecQue			 = NULL;
	osUI_FuncsFinExecQue		 = NULL;
	ubUI_FinishViewSwitch		 = TRUE;
	ubUI_RestartRec 			 = FALSE;
	ubUI_ReOpenPanel			 = FALSE;
	ubUI_CuStandbyFlag			 = FALSE;
	ubUI_CuPowerOffFlag 		 = FALSE;
	ubUI_CuPowerDiscFlag		 = FALSE;
	tUI_SaveViewNum 			 = QUALVIEW_ITEM;
	ulPanelOffCount 			 = PANELAUTOOFFCOUNT;
	memset(&tUI_RecFoldersInfo, 0, sizeof(UI_RecFoldersInfo_t));
	memset(&tUI_RecFilesInfo,	0, sizeof(UI_RecFilesInfo_t));
	tUI_RecPlayAct.tRecAct		 = UI_REC_STOP;
	tUI_RecPlayAct.tPlaySts 	 = UI_RECFILE_STOP;
	ubUI_VdoRecChkFlag			 = TRUE;
	ubUI_PerDebugEn 			 = FALSE;
	osMutexDef(UiPerDbgMutex);
	osUI_PerDbgMutex			 = osMutexCreate(osMutex(UiPerDbgMutex));
	tUI_State					 = UI_DISPLAY_STATE;
	tUI_SyncAppState			 = APP_STATE_NULL;
	if(tTWC_RegTransCbFunc(TWC_UI_SETTING, UI_RecvCamResponse, UI_RecvCamRequest) != TWC_SUCCESS)
		printd(DBG_ErrorLvl, "UI Setting 2-way command fail!\n");
	if(tTWC_RegTransCbFunc(TWC_Laser_CTRL, UI_RecvCamResponse, NULL) != TWC_SUCCESS) //Alan
		printd(DBG_ErrorLvl, "TWC_Laser_CTRL Setting 2-way command fail!\n");
	if(tTWC_RegTransCbFunc(TWC_Led_CTRL, UI_RecvCamResponse, NULL) != TWC_SUCCESS)
		printd(DBG_ErrorLvl, "TWC_Led_CTRL Setting 2-way command fail!\n");
	if(tTWC_RegTransCbFunc(TWC_TX_TYPE, NULL, UI_RecvTxType) != TWC_SUCCESS)
		printd(DBG_ErrorLvl, "Register TWC_TX_TYPE TWC Fail !\n");
	if(tTWC_RegTransCbFunc(TWC_TX_VERSION, NULL, UI_RecvTxVersion) != TWC_SUCCESS)
		printd(DBG_ErrorLvl, "Register TWC_TX_VERSION TWC Fail !\n");
	UI_LoadDevStatusInfo();
	UI_InitCharArray();
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

			ubUI_StopUpdateStsBarFlag = TRUE;
			OSD_Weight(OSD_WEIGHT_8DIV8);
			tOsdImgInfo.uwHSize  = uwOSD_GetHSize();
			tOsdImgInfo.uwVSize  = uwOSD_GetVSize();
			tOsdImgInfo.uwXStart = 0;
			tOsdImgInfo.uwYStart = 0;
			OSD_EraserImg1(&tOsdImgInfo);
			tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_FWUSTARTBG, 1, &tOsdImgInfo);
			tOSD_Img1(&tOsdImgInfo, OSD_UPDATE);
			KNL_ResetLcdChannel();
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
			/*
			uint8_t ubProgScaleIdx = (pFWU_StsRpt->ubAPP_Report[0] / pFWU_StsRpt->ubAPP_Report[1]);
			tOSD_GetOsdImgInfor(1, OSD_IMG2, (OSD2IMG_FWUPROG0P_ICON + ubProgScaleIdx), 1, &tOsdImgInfo);
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			if(100 == pFWU_StsRpt->ubAPP_Report[0])
				osDelay(1000);
			*/
			printf("pFWU_StsRpt->ubAPP_Report[0] = %d\n",pFWU_StsRpt->ubAPP_Report[0]);
			
			static uint16_t uwOldVal;
			static OSD_IMG_INFO tOsdImgInfo[3];
			if(pFWU_StsRpt->ubAPP_Report[0] == 0)
			{
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PROGRESS_BAR, 3, &tOsdImgInfo[0]);
				tOSD_Img2(&tOsdImgInfo[0], OSD_UPDATE);
				tOsdImgInfo[1].uwXStart = tOsdImgInfo[0].uwXStart + 2;
			}
			else
			{
				for(uint16_t i = uwOldVal;i < pFWU_StsRpt->ubAPP_Report[0];i ++)
				{
					tOSD_Img2(&tOsdImgInfo[1], OSD_UPDATE);
					tOsdImgInfo[1].uwXStart += 2;
				}
			}
			uwOldVal = pFWU_StsRpt->ubAPP_Report[0];
			break;
		}
	}
}
//------------------------------------------------------------------
void UI_HandlePreviewStatus(void *ptAppStsReport)
{
	static uint32_t ulDelayCheck = 5;
	uint8_t uChnCount = 0;
	if(ulDelayCheck)
	{
		ulDelayCheck --;
		return;
	}
	APP_StatusReport_t *pAppStsRpt = (APP_StatusReport_t *)ptAppStsReport;
	
	if(tCamViewSel.tCamViewType == SINGLE_VIEW)
		uChnCount = 1;
	else if(tCamViewSel.tCamViewType == DUAL_VIEW)
		uChnCount = 2;
	//else if(tCamViewSel.tCamViewType == V3_2T1B_VIEW)
	//	uChnCount = 3;
	else if(tCamViewSel.tCamViewType == QUAD_VIEW)
		uChnCount = 4;
	
	if(pAppStsRpt->tAPP_State != APP_PAIRING_STATE && tUI_State == UI_DISPLAY_STATE)
	{
 		for(uint8_t i = 0;i < uChnCount;i ++)
		{
			uint8_t uCamNum = tCamViewSel.tCamViewPool[i];
			if(tUI_CamStatus[uCamNum].ulFrameCount == pAppStsRpt->ulAPP_Report[uCamNum])
			{
				tUI_CamStatus[uCamNum].ulImageStuckCount ++;
				if(tUI_CamStatus[uCamNum].ulImageStuckCount == 3)//3��֡��û����㿨ͼ��?
				{
					LCD_ChDisable(LCD_CH0 + i);
					tUI_RecCutOFF[uCamNum] = 1;
					printf("---------LCD_ChDisable chn = %d\n",i);
					
				}
				if(tUI_CamStatus[uCamNum].ulImageStuckCount == 20)
				{
					if(pAppStsRpt->ulAPP_Report[4 + uCamNum] != 0)//��ͼ�ˣ�����ʵ��֡��û��Ϊ0�ʹ��������쳣��
					{
						UI_SwitchViewType(DeskTopShowView,TRUE);
						return;
					}
				}
			}
			else
			{
				//tUI_RecCutOFF[uCamNum] = 0;
				tUI_CamStatus[uCamNum].ulFrameCount = pAppStsRpt->ulAPP_Report[uCamNum];
				tUI_CamStatus[uCamNum].ulImageStuckCount = 0;
 			}
					
 		}
	}
}


uint8_t UI_UpdateLaserAndLedCtrl(UI_CamNum_t tCamNum)
{
	printf("UI_UpdateTxImgProc  tCamNum = %d\n",tCamNum);
	UI_CUReqCmd_t tCamSetCmd;
	tCamSetCmd.tDS_CamNum				= tCamNum;
	tCamSetCmd.ubCmd[UI_TWC_TYPE]		= UI_SETTING;
	tCamSetCmd.ubCmd[UI_SETTING_ITEM]	= UI_IMGPROC_SETTING;
	tCamSetCmd.ubCmd_Len				 = 4;

	tCamSetCmd.ubCmd[UI_SETTING_DATA+1] = tUI_CamStatus[tCamNum].tCamLaser;
	if(UI_SendLaserorLedToCAM(osThreadGetId(), &tCamSetCmd, TWC_Laser_CTRL) != rUI_SUCCESS)
		return 0;

	tCamSetCmd.ubCmd[UI_SETTING_DATA+1] = tUI_CamStatus[tCamNum].tCamLed;
	if(UI_SendLaserorLedToCAM(osThreadGetId(), &tCamSetCmd, TWC_Led_CTRL) != rUI_SUCCESS)
		return 0;

	return 1;
}



void UI_UpdateZOOMCamAndLaserCam(uint8_t ubFirstCall[][2])
{
	static uint8_t uFirstLinkZoomCameraFlag = 0;
//	printf("UI_UpdateZOOMCamAndLaserCam\n");
	for(uint8_t i=0;i<4;i++)
	{
		
		if(ubFirstCall[i][0] && (ubZoomPairingFlag && ubZoomPairingCount==0))
		{
			
//			if(DeskTopShowView < CAM_4T && ubFirstCall[i][1] != DeskTopShowView) break;
			if(tUI_CamStatus[(UI_CamNum_t)ubFirstCall[i][1]].tCamConnSts == CAM_ONLINE && tUI_CamStatus[(UI_CamNum_t)ubFirstCall[i][1]].cCamVersion[0] != '\0')
			{
				ubFirstCall[i][0] = 0;
//				if(UI_CheckTxVersion(tUI_CamStatus[(UI_CamNum_t)ubFirstCall[i][1]].cCamVersion,30,zoomchar,3))
//				{	
//					if(UI_UpdateBSDAndTraceCtrl((UI_CamNum_t)ubFirstCall[i][1]) != rUI_SUCCESS)
//					{//Alan  ��һ��������ͷ�������û�Ѳ�����һ��?
//						printf("\nCAMERA_BSD And CAMERA_Trace Setting Fail!!! !\n");
//					}
//					for(uint8_t i=0;i<4;i++) //zoom �ȸ�λһ�飬��Ȼcam2->cam3������
//					{
//						tUI_CuSetting.ubisZoomCameraFlag[i] = 0;
//					}
//					tUI_CuSetting.ubisZoomCameraFlag[(UI_CamNum_t)ubFirstCall[i][1]] = 1;
//					uFirstLinkZoomCameraFlag = 1;
//				}
				if(UI_CheckTxVersion(tUI_CamStatus[(UI_CamNum_t)ubFirstCall[i][1]].cCamVersion,30,Laserchar,3))
				{
					if(UI_UpdateLaserAndLedCtrl((UI_CamNum_t)ubFirstCall[i][1]) != rUI_SUCCESS)
					{
						printf("\nCAMERA_LASER And CAMERA_LED Setting Fail!!! !\n");
					}
				}
				if(!ubFirstCall[0][0] && !ubFirstCall[1][0] && !ubFirstCall[2][0] && !ubFirstCall[3][0])
				{
					ubZoomPairingFlag = FALSE;
					ubZoomPairingCount = 3000;
				}
			}
		}

//		printf(" not ubFirstCall i = %d\n", i);
//		if(tUI_CamStatus[i].ulCAM_ID == INVALID_ID)
//		{
//			tUI_CuSetting.ubisZoomCameraFlag[(UI_CamNum_t)i] = 0;
//		}
//		if (tUI_CamStatus[(UI_CamNum_t)i].tCamConnSts == CAM_ONLINE)
//		{
//			if(tUI_CamStatus[(UI_CamNum_t)i].cCamVersion[0] != '\0')
//				tUI_CuSetting.ubisZoomCameraFlag[(UI_CamNum_t)i] = 0;
//			if(UI_CheckTxVersion(tUI_CamStatus[(UI_CamNum_t)i].cCamVersion,30,zoomchar,3))
//			{
//				tUI_CuSetting.ubisZoomCameraFlag[(UI_CamNum_t)i] = 1;
//				if(ubisShowZoomflag && uFirstLinkZoomCameraFlag)
//				{
//					uFirstLinkZoomCameraFlag++;
//					if(UI_UpdateBSDAndTraceCtrl((UI_CamNum_t)i) != rUI_SUCCESS)
//					{//Alan  ��һ��������ͷ�������û�Ѳ�����һ��?
//						printf("\nCAMERA_BSD And CAMERA_Trace Setting Fail!!! !\n");
//					}
//					if(uFirstLinkZoomCameraFlag > 3) uFirstLinkZoomCameraFlag = 0;
//				}
//			}
//		}
	}
}


//------------------------------------------------------------------------------
void UI_UpdateAppStatus(void *ptAppStsReport)
{
	APP_StatusReport_t *pAppStsRpt = (APP_StatusReport_t *)ptAppStsReport;
	UI_CamNum_t tCamNum;
	static uint8_t 	ubFirstCall[4][2] = {0};

	switch(pAppStsRpt->tAPP_ReportType)
	{
		case APP_PAIRSTS_RPT:
		{
			UI_Result_t tPair_Result  = (UI_Result_t)pAppStsRpt->ubAPP_Report[0];
			uint8_t ubAppPairFlag	  = pAppStsRpt->ubAPP_Report[2];
			void (*UI_RptPairFunc[])(UI_Result_t) =
			{
				UI_ReportPairingResult,
				UI_ReportAppPairingResult,
			};

			if(TRUE == ubAppPairFlag)
			{
				tPairInfo.tPairSelCam	= (UI_CamNum_t)pAppStsRpt->ubAPP_Report[3];
			}
			UI_RptPairFunc[ubAppPairFlag](tPair_Result);
			tCamNum = (UI_CamNum_t)pAppStsRpt->ubAPP_Report[3];
			if(rUI_SUCCESS == tPair_Result)
			{
				tUI_CamStatus[tCamNum].tCamColorParam.ubColorBL = 50;
				tUI_CamStatus[tCamNum].tCamColorParam.ubColorContrast =	50;
				tUI_CamStatus[tCamNum].tCamColorParam.ubColorSaturation = 50;
				tUI_CamStatus[tCamNum].tCamColorParam.ubColorHue = 50;
				tUI_CamStatus[tCamNum].tCamImgFlip = CAMIMGFLIP_DISABLE;
				tUI_CamStatus[tCamNum].tCamImgMirror = CAMIMGMIRROR_DISABLE;
				tUI_CamStatus[tCamNum].tCamLaser = CAMLASER_ENABLE;
		        tUI_CamStatus[tCamNum].tCamLed = CAMLED_ENABLE;

				tUI_CuSetting.ubUpdateTxParam[tCamNum] = FALSE;

				tUI_CuSetting.bColorBLGain[tCamNum] = 0;
				tUI_CuSetting.bColorContrastGain[tCamNum] = 0;
				tUI_CuSetting.bColorSaturationGain[tCamNum] = 0;
				tUI_CuSetting.bColorHueGain[tCamNum] = 0;

				for(uint8_t i=0;i<4;i++)
				{
					if(!ubFirstCall[i][0])
					{
						ubFirstCall[i][0] = 1;
						ubFirstCall[i][1] = tCamNum;
						break;
					}
				}

				ubZoomPairingFlag  = TRUE;
				ubZoomPairingCount = 3000;
				
				UI_SetTxType(tCamNum,0);
				UI_UpdateDevStatusInfo();
			}
			UI_RefreshBandwidth();//������ֶԽ����û�д�����ͼ
			break;
		}
		case APP_LINKSTS_RPT:
			UI_ReportCamConnectionStatus(pAppStsRpt->ubAPP_Report);
			UI_HandlePreviewStatus(ptAppStsReport);
			UI_UpdateZOOMCamAndLaserCam(ubFirstCall);
			break;
		case APP_VWMODESTS_RPT:
			//�����л�����
			
			if(ubUI_ReOpenPanel)//��������
			{
				ubUI_ReOpenPanel = FALSE;
				if(!ubUI_CuStandbyFlag || (ubUI_CuStandbyFlag == TRUE && TriggerLock == TRUE))
					UI_PanelOn();
			}
			if(ubUI_RestartRec && !ubUI_CuPowerDiscFlag && (!ubUI_CuStandbyFlag || (ubUI_CuStandbyFlag && TriggerLock)))//����¼��
			{
				ubUI_RestartRec = FALSE;
				UI_VideoRecordingExec(UI_REC_START);
			}
			
			ubUI_FinishViewSwitch = TRUE;
			//printf("uwUI_ViewSwitchTime = %d\n",uwUI_ViewSwitchTime);
			uwUI_ViewSwitchTime = 0;
			printf("ubUI_FinishViewSwitch\n");
			//printf("UI_SwitchViewType osUI_UpdateStatusCtr  wait 222222 \n");
			if(!ubUI_CuPowerDiscFlag)
			{
				ubUI_CuSemaphorewaitFlag = 0;
				osSemaphoreRelease(osUI_CuTriggerCtr);
				osSemaphoreRelease(osUI_UpdateStatusCtr);
			}
			else
			{
				if(ubUI_CuSemaphorewaitFlag)
				{
					ubUI_CuSemaphorewaitFlag = 0;
					//printf("UI_SwitchViewType osUI_UpdateStatusCtr  wait 3333333 \n");
					osSemaphoreRelease(osUI_CuTriggerCtr);
					osSemaphoreRelease(osUI_UpdateStatusCtr);
				}
			}


			

			break;
		case APP_VOXMODESTS_RPT:
			ubUI_StopUpdateStsBarFlag = pAppStsRpt->ubAPP_Report[0];
			if(TRUE == ubUI_StopUpdateStsBarFlag)
				break;
			break;
		case APP_PAIRUDCAM_PRT:
		{
			UI_CamNum_t tDelCam 	= (UI_CamNum_t)pAppStsRpt->ubAPP_Report[0];
			UI_UnBindCam(tDelCam);
			//����Խӳɹ����?
			OSD_IMG_INFO tOsdImgInfo,tDelOsdInfo;
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PAIRRDYMASK_ICON, 1, &tDelOsdInfo);
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PAIRCAM1NOR_ICON + 2*tDelCam, 1, &tOsdImgInfo);
			tDelOsdInfo.uwXStart = tOsdImgInfo.uwXStart + (tOsdImgInfo.uwHSize - tDelOsdInfo.uwHSize)/2;
			OSD_EraserImg2(&tDelOsdInfo);
			
			UI_RefreshBandwidth();//������ֶԽ����û�д�����ͼ
			break;
		}
		case APP_DISPPAIRICON_RPT:
		{
			UI_DisplayAppPairingScreen();
			return;
		}
		case APP_TRXBWSWRET_RPT:
		{
			
			break;
		}
		default:
			break;
	}

	if(FALSE == ubUI_CuStartUpFlag)
	{
		ADO_SetDacR2RVol(R2R_VOL_n0DB);
		if((UI_RECORDING_MODE == tUI_CuSetting.tVdoMode) && (tUI_CuSetting.ubPowerOnRecord == TRUE))
		{
			UI_VideoRecordingExec(UI_REC_START);
		}

		if(tUI_ViewModeSel == SCANVIEW_ITEM)
		{
			ulAutoScanCount = tUI_CuSetting.AutoScanDuty[tUI_AutoScanNumSel]* 1000;
		}
		if(tUI_CuSetting.tPowerOnMode == POWERON_STANDBY 
			|| (tUI_CuSetting.tPowerOnMode == POWERON_LAST &&(tUI_CuSetting.tLastMode == LASTMODE_STANDBY || tUI_CuSetting.tLastMode == LASTMODE_SCAN_STANDBY)))
			UI_EnterStandby();
		
		ubUI_CuStartUpFlag = TRUE;
	}
	osSemaphoreWait(osUI_CuSemId, osWaitForever);
	tUI_CuSetting.IconSts.ubClearThdCntFlag = (tUI_SyncAppState == pAppStsRpt->tAPP_State)?FALSE:TRUE;
	tUI_SyncAppState = pAppStsRpt->tAPP_State;
	osSemaphoreRelease(osUI_CuSemId);
}
//------------------------------------------------------------------------------
void UI_CheckLightSensor(uint8_t ubFirstPanelOn)
{
	if(!tUI_CuSetting.ubAutoDimmer)
		return;
	uint16_t uwVal = 0;
	static uint8_t ubStatus = 0;//������ȵ�״�?
	static uint16_t uwStableTime = 0;
	uwVal = uwSADC_GetReport(SADC_CH4);

	if(uwVal < BACKLIGHT_ADC_VALUE)//�������ȵ�
	{
		if(ubStatus)
		{
			ubStatus = 0;
			uwStableTime = 0;
		}
		if(ubUI_CurDimmerLvl > tUI_CuSetting.ubAutoDimmer_MinVal)
		{
			uwStableTime ++;
			if(uwStableTime >= 10)
			{
				for(uint8_t i = ubUI_CurDimmerLvl;i >= tUI_CuSetting.ubAutoDimmer_MinVal;(i > 2)?(i -= 2):(i = tUI_CuSetting.ubAutoDimmer_MinVal))
				{
					ubUI_CurDimmerLvl = i;
					uint16_t ubValue = (uint16_t)UI_Map(ubUI_CurDimmerLvl ,0,99,LCD_PWM_MIN,LCD_PWM_MAX);
					SET_PANEL_PWM(ubValue);
					osDelay(3);
					if(i == tUI_CuSetting.ubAutoDimmer_MinVal)
						break;
				}
				uwStableTime = 0;
			}
		}
		else if(ubUI_CurDimmerLvl <= tUI_CuSetting.ubAutoDimmer_MinVal)
		{
			uwStableTime  = 0;
			ubUI_CurDimmerLvl = tUI_CuSetting.ubAutoDimmer_MinVal;
			uint16_t ubValue = (uint16_t)UI_Map(ubUI_CurDimmerLvl ,0,99,LCD_PWM_MIN,LCD_PWM_MAX);
			SET_PANEL_PWM(ubValue);
		}
	}
	else//�������ȸ�
	{
		if(!ubStatus)
		{
			ubStatus = 1;
			uwStableTime = 0;
		}
		if(ubUI_CurDimmerLvl < tUI_CuSetting.ubAutoDimmer_MaxVal)
		{
			if(ubFirstPanelOn)//�����н���
			{
				ubUI_CurDimmerLvl = tUI_CuSetting.ubAutoDimmer_MaxVal;
				uint16_t ubValue = (uint16_t)UI_Map(ubUI_CurDimmerLvl ,0,99,LCD_PWM_MIN,LCD_PWM_MAX);
				SET_PANEL_PWM(ubValue);
			}
			else
			{
				uwStableTime ++;
				if(uwStableTime >= 10)
				{
					for(uint8_t i = ubUI_CurDimmerLvl;i <= tUI_CuSetting.ubAutoDimmer_MaxVal;
						((i + 2)<=tUI_CuSetting.ubAutoDimmer_MaxVal)?(i += 2):(i = tUI_CuSetting.ubAutoDimmer_MaxVal))
					{
						ubUI_CurDimmerLvl = i;
						uint16_t ubValue = (uint16_t)UI_Map(ubUI_CurDimmerLvl ,0,99,LCD_PWM_MIN,LCD_PWM_MAX);
						SET_PANEL_PWM(ubValue);
						osDelay(3);
						if((i == tUI_CuSetting.ubAutoDimmer_MaxVal))
							break;
					}
					uwStableTime = 0;
				}
			}
		}
		else if(ubUI_CurDimmerLvl >= tUI_CuSetting.ubAutoDimmer_MaxVal)
		{
			uwStableTime  = 0;
			ubUI_CurDimmerLvl = tUI_CuSetting.ubAutoDimmer_MaxVal;
			uint16_t ubValue = (uint16_t)UI_Map(ubUI_CurDimmerLvl ,0,99,LCD_PWM_MIN,LCD_PWM_MAX);
			SET_PANEL_PWM(ubValue);

		}

	}
}
//------------------------------------------------------------------------------
void UI_PwrCtrl(void)
{
	OSD_IMG_INFO tOsdImgInfo;
	if(!ubUI_CuPowerDiscFlag)//��Դ��������
	{
		if(GET_PWR_REPORT)//��Դ�Ͽ�
		{
			ubUI_CuPowerDiscFlag = TRUE;
//			//! UART2 修改串口漏电问题
//			GLB->PADIO49 = 0; //UART2_TX
//			GLB->PADIO50 = 0; //UART2_RX
//			GPIO->GPIO_OE7 = 1;
//			GPIO->GPIO_OE8 = 1;
//			GPIO->GPIO_O7 = 0;
//			GPIO->GPIO_O8 = 0;
//			KNL_UsbdFwuFg = 1;
	
//			while(!UART1->TX_RDY);
//			UART1->RS_DATA = 'D';
			ADO_SetDacR2RVol(R2R_VOL_n45DB);//�ص�оƬ����Ƶ���?
			
			uint8_t ubValue = (uint16_t)UI_Map(0,0,99,LCD_PWM_MIN,LCD_PWM_MAX);//�����Ƚ������?
			SET_PANEL_PWM(ubValue);
			
			if(tUI_State == UI_SUBSUBSUBMENU_STATE &&  tUI_MenuItem.ubItemIdx == PLAYBACK_ITEM)//�����ǰ���ڲ�����Ƶ����Ҫ���˳����Ų��ܷ���?
			{
				UI_MenuKey();
			}
			if(tUI_State == UI_PAIRING_STATE)//�����ǰ���ڶԽӾ���ֹͣ�Խ�?
			{
				UI_EnterKey();
				osDelay(100);
			}
			if(UI_REC_START == tUI_RecPlayAct.tRecAct)
			{
				UI_VideoRecordingExec(UI_REC_STOP);
				ubUI_RestartRec = TRUE;
			}
			//��������

			printf("UI_PwrCtrl UI_BackToDesktop 11111\n");
			if(tUI_State != UI_DISPLAY_STATE && tUI_State != UI_MAINMENU_STATE)
				UI_BackToDesktop(TRUE);
			else
				UI_BackToDesktop(FALSE);


			OSD_Weight(OSD_WEIGHT_4DIV8);
			tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_POWER_DISCONNECT, 1, &tOsdImgInfo);
			tOSD_Img1(&tOsdImgInfo, OSD_UPDATE);
			UI_AudioOff();
		}
	}
	else
	{
		if(!GET_PWR_REPORT)//��Դ����������
		{
//			while(!UART1->TX_RDY);
//			UART1->RS_DATA = 'C';
//			ubPwrUartFlag = TRUE;
//			//5s开启串�?
//			ubPwrUartCount = 5000;
//			printf("time reset 5000 pwr!!!!!!!");

			if(ubUI_CuStandbyFlag && !TriggerLock)
			{
				UI_PanelOff();
				UI_AudioOff();
				UI_DisconnectTx();
			}
			else
			{				
				UI_PanelOn();
				UI_AudioOn();
				tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_POWER_CONNECT, 1, &tOsdImgInfo);
				tOSD_Img1(&tOsdImgInfo, OSD_UPDATE);
				if(ubUI_RestartRec)
				{
					UI_VideoRecordingExec(UI_REC_START);
					ubUI_RestartRec = FALSE;
				}

//				GLB->PADIO49 = 2; //UART2_TX
//				GLB->PADIO50 = 2; //UART2_RX
//				KNL_UsbdFwuFg = 0;
				osDelay(500);
		
			}
			OSD_Weight(OSD_WEIGHT_8DIV8);
			printf("UI_PwrCtrl UI_BackToDesktop 22222\n");
			UI_BackToDesktop(FALSE);
			ubUI_CuPowerDiscFlag = FALSE;
			
			ADO_SetDacR2RVol(R2R_VOL_n0DB);
			
		}
	}
}
//------------------------------------------------------------------------------
uint8_t isDrawNosignal[4] = {0};
static uint8_t Stop_WavchnFlag[4]={1,1,1,1};
uint8_t DrawingBox = 0;
uint8_t ClearingBox = 0;	

void UI_DrawNoSignalIcon(void)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint16_t uwCam_X[4] = {0};
	uint16_t uwCam_Y[4] = {0};
	uint8_t ubCamNum ;
	uint16_t uwLcd_HSize = uwLCD_GetLcdHoSize();
	uint16_t uwLcd_VSize = uwLCD_GetLcdVoSize();

	tOSD_GetOsdImgInfor(1, OSD_IMG2, (tUI_CuSetting.tLanguage == LANGUAGE_ENGLISH)?OSD2IMG_DESKTOP_NOSIGNAL_WORD:
									(tUI_CuSetting.tLanguage == LANGUAGE_GERMAN)?OSD2IMG_DESKTOP_NOSIGNAL_WORD_GER:
									(tUI_CuSetting.tLanguage == LANGUAGE_FRENCH)?OSD2IMG_DESKTOP_NOSIGNAL_WORD_FR:OSD2IMG_DESKTOP_NOSIGNAL_WORD_CHN, 1, &tOsdImgInfo);
	
	switch(tCamViewSel.tCamViewType)//����nosignal������ʾλ��
	{
		case SINGLE_VIEW:
			uwCam_X[tCamViewSel.tCamViewPool[0]] = tOsdImgInfo.uwXStart + uwLcd_HSize/8*3;
			uwCam_Y[tCamViewSel.tCamViewPool[0]] = tOsdImgInfo.uwYStart;
			ubCamNum = 1;
			break;
		case DUAL_VIEW:
			//left
			uwCam_X[tCamViewSel.tCamViewPool[0]] = tOsdImgInfo.uwXStart + uwLcd_HSize/8;
			uwCam_Y[tCamViewSel.tCamViewPool[0]] = tOsdImgInfo.uwYStart;
			//right
			uwCam_X[tCamViewSel.tCamViewPool[1]] = tOsdImgInfo.uwXStart + uwLcd_HSize/8*5;
			uwCam_Y[tCamViewSel.tCamViewPool[1]] = tOsdImgInfo.uwYStart;
			ubCamNum = 2;
			break;
		case QUAD_VIEW:
			//upper left
			uwCam_X[tCamViewSel.tCamViewPool[0]] = tOsdImgInfo.uwXStart + uwLcd_HSize/8;
			uwCam_Y[tCamViewSel.tCamViewPool[0]] = tOsdImgInfo.uwYStart - uwLcd_VSize/4;
			//upper right
			uwCam_X[tCamViewSel.tCamViewPool[1]] = tOsdImgInfo.uwXStart + uwLcd_HSize/8*5;
			uwCam_Y[tCamViewSel.tCamViewPool[1]] = tOsdImgInfo.uwYStart - uwLcd_VSize/4;
			//lower left
			uwCam_X[tCamViewSel.tCamViewPool[2]] = tOsdImgInfo.uwXStart + uwLcd_HSize/8;
			uwCam_Y[tCamViewSel.tCamViewPool[2]] = tOsdImgInfo.uwYStart + uwLcd_VSize/4;
			//lower right
			uwCam_X[tCamViewSel.tCamViewPool[3]] = tOsdImgInfo.uwXStart + uwLcd_HSize/8*5;
			uwCam_Y[tCamViewSel.tCamViewPool[3]] = tOsdImgInfo.uwYStart + uwLcd_VSize/4;
			
			ubCamNum = 4;
			break;

		default:
			break;
	}
	//��ʾ�����no signal
	for(uint8_t ubCam = 0;ubCam < ubCamNum;ubCam ++)
	{
		tOsdImgInfo.uwXStart = uwCam_X[tCamViewSel.tCamViewPool[ubCam]];
		tOsdImgInfo.uwYStart = uwCam_Y[tCamViewSel.tCamViewPool[ubCam]];
		if(tUI_CamStatus[tCamViewSel.tCamViewPool[ubCam]].tCamConnSts == CAM_OFFLINE)
		{
			ubCamDiscCount[tCamViewSel.tCamViewPool[ubCam]] ++;
			if(ubCamDiscCount[tCamViewSel.tCamViewPool[ubCam]] == 10)
			{
				ubCamDiscCount[tCamViewSel.tCamViewPool[ubCam]] = 0;
				tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
				isDrawNosignal[ubCam] = 1;
				//Stop_WavchnFlag[ubCam] = 1;
				//osMessagePut(UI_AI_clearBOXQueue, &ubCam, 0);
				Drawing_BoxFlag[ubCam] = 0;
				
			}
		}
		else
		{
			ubCamDiscCount[tCamViewSel.tCamViewPool[ubCam]] = 0;
			if(tCamViewSel.tCamViewType == SINGLE_VIEW)
			{
				if((tUI_CuSetting.GuideLineEnable[tCamViewSel.tCamViewPool[0]] || tUI_CuSetting.tParkingLineEnable[tCamViewSel.tCamViewPool[0]] == PARKING_LINE_ON) && tUI_CuSetting.ubIsEnableBSDRANGE[tCamViewSel.tCamViewPool[0]] == 0 && \
					tUI_CuSetting.ubDetectPeopleFlag[tCamViewSel.tCamViewPool[0]] == 0 && tUI_CuSetting.ubDetectCarFlag[tCamViewSel.tCamViewPool[0]] == 0)//��������
				{
					
					if(tUI_CuSetting.tParkingLineEnable[tCamViewSel.tCamViewPool[0]] == PARKING_LINE_ON)
					{
						UI_DrawParkingLine(tCamViewSel.tCamViewPool[0],OSD_UPDATE);
					}
					if(tUI_CuSetting.GuideLineEnable[tCamViewSel.tCamViewPool[0]])
					{
						//if(tUI_CuSetting.ubIsShowBSDBox == 0 || tUI_CuSetting.ubIsEnableBSD == 0 ||((tUI_CuSetting.ubDetectCarFlag==0) && (tUI_CuSetting.ubDetectPeopleFlag==0) ))
						{	
							OSD_EraserImg2_NoUpdate(&tOsdImgInfo);
							tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_GUIDELINE_SIZE1 + tUI_CuSetting.GuideLineSize[tCamViewSel.tCamViewPool[0]], 1, &tOsdImgInfo);
							tOsdImgInfo.uwXStart = (tUI_CuSetting.GuideLine_XY[tCamViewSel.tCamViewPool[0]][0] << 8) + tUI_CuSetting.GuideLine_XY[tCamViewSel.tCamViewPool[0]][1];
							tOsdImgInfo.uwYStart = (tUI_CuSetting.GuideLine_XY[tCamViewSel.tCamViewPool[0]][2] << 8) + tUI_CuSetting.GuideLine_XY[tCamViewSel.tCamViewPool[0]][3];
							tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
						}
					}
				}
				else
				{
					if (isDrawNosignal[ubCam] == 1)
					{
						OSD_EraserImg2(&tOsdImgInfo);
						isDrawNosignal[ubCam] = 0;
					}
				}
			}
			else
			{
				if (isDrawNosignal[ubCam] == 1)
				{
					OSD_EraserImg2(&tOsdImgInfo);
					isDrawNosignal[ubCam] = 0;
				}
			}
		}
	}
	
}

//------------------------------------------------------------------------------

void UI_DrawSdCardInfoIcon(void)
{
	static uint8_t ubShowSdinfo = TRUE;

	OSD_IMG_INFO tOsdImgInfo[2],tSdCardImgInfo[2],tReturnImgInfo,tEraserOsdImgInfo;
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_SDCARD_NO, 2, &tOsdImgInfo[0]);
	// 状态图标刷新时擦除穿过该区域的旧检测框，保留屏幕最外侧 4 像素。
	tEraserOsdImgInfo.uwXStart = 972;
	tEraserOsdImgInfo.uwYStart = 4;
	tEraserOsdImgInfo.uwHSize = 48;
	tEraserOsdImgInfo.uwVSize = 76;
	OSD_EraserImg2_NoUpdate(&tEraserOsdImgInfo);

	tOSD_GetOsdImgInfor(1, OSD_IMG2, (tUI_CuSetting.tLanguage == LANGUAGE_ENGLISH)?OSD2IMG_DESKTOP_SD_NEED_FORMAT:
									(tUI_CuSetting.tLanguage == LANGUAGE_GERMAN)?OSD2IMG_DESKTOP_SD_NEED_FORMAT_GER:
									(tUI_CuSetting.tLanguage == LANGUAGE_FRENCH)?OSD2IMG_DESKTOP_SD_NEED_FORMAT_FR:OSD2IMG_DESKTOP_SD_NEED_FORMAT_CHN, 2, &tSdCardImgInfo[0]);
	
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_RETURN_NOBACKGROUND_NOR, 1, &tReturnImgInfo);

	if(FS_ChkMediaExist(KNL_GetFsMedia()) == FS_MEDIA_OUT)
		OSD_EraserImg2(&tOsdImgInfo[0]);
	else 
	{
		if(FS_ChkMediaRdy(KNL_GetFsMedia()) != FS_MEDIA_RDY)
		{
			tOSD_Img2(&tOsdImgInfo[0], OSD_UPDATE);
			if(ubShowSdinfo == TRUE)
			{	
				if(FS_ChkMediaRdy(KNL_GetFsMedia()) == FS_MEDIA_NOT_RDY) //sd����û׼����
				{
					printf("sd card not ready to access\n");
					return;
				}
				UI_ClearOsdImageNoUpdate();
				
				if(ubUI_TouchPanelSts)
					tOSD_Img2(&tReturnImgInfo, OSD_QUEUE);
				
				if(FS_ChkMediaRdy(KNL_GetFsMedia()) == FS_MEDIA_RE_PLUGIN)
					tOSD_Img2(&tSdCardImgInfo[1], OSD_UPDATE);
				else if(FS_ChkMediaRdy(KNL_GetFsMedia()) == FS_MEDIA_GET_INFO_FAIL)
					tOSD_Img2(&tSdCardImgInfo[0], OSD_UPDATE);
			
				ubShowSdinfo = FALSE;
				MenuOnFlag = TRUE;
				ulMenuAutoOffCount = MENUAUTOOFFCOUNT;
				tUI_State = UI_SHOW_SDCARD_INFO_STATE;
			}
		}
		else
			tOSD_Img2(&tOsdImgInfo[1], OSD_UPDATE);
	}

}
//------------------------------------------------------------------------------
// FF CC：18 字节同时发送参数和四路算法选择，1126 仅在通道改变时重启。
void UI_SendAIConfigTo1126(void)
{
	uint8_t ubData[18] = {0xFF, 0xCC};
	uint8_t i, chn = CAM1;
	uint8_t ubLength = sizeof(ubData);
	uint16_t uwLine, uwCRC;
	UI_PalletConfigInfo_t *pConfig;
	for(i = 0; i < CAM_4T; i++)
	{
		ubData[12+i] = tUI_CamStatus[i].ubAIAlgorithm;
		if(ubData[12+i] == AI_ALGORITHM_PALLET)
			chn = i;
	}
	pConfig = &tUI_CamStatus[chn].tPalletConfig;
	// 引导线按原协议使用 1024x600 坐标，由 1126 换算到算法图像。
	uwLine = (tUI_CuSetting.GuideLine_XY[chn][2] << 8) + tUI_CuSetting.GuideLine_XY[chn][3] + 2;
	ubData[2] = ubLength;
	ubData[3] = chn;
	ubData[4] = uwLine & 0xFF;
	ubData[5] = uwLine >> 8;
	ubData[6] = pConfig->tDelayTurnOFF;
	ubData[7] = pConfig->tFlowFrameInterval;
	ubData[8] = pConfig->tRateRange;
	ubData[9] = pConfig->tFlowPauseDuration;
	ubData[10] = pConfig->tFlowRunDuration;
	ubData[11] = pConfig->tDectability;
	uwCRC = crc16_Gen((char *)&ubData[2], ubLength - 4);
	ubData[ubLength-2] = uwCRC & 0xFF;
	ubData[ubLength-1] = uwCRC >> 8;
	osMutexWait(osAIConfigMutex, osWaitForever);
	for(i = 0; i < ubLength; i++)
		UI_UART2_PutChar(ubData[i]);
	osMutexRelease(osAIConfigMutex);
	uwAIConfigSyncCount = 1000;
}

void UI_SendBSDRangeTo1126(UI_ParkinglinePoint_t tParkinglinePoint,uint8_t chn)
{
	uint8_t BSDRangeDate[24];
	uint16_t CRCtemp; 
//	UI_UART2_PutChar(0XFF);
//	UI_UART2_PutChar(0XBB);
	BSDRangeDate[0] = 22;//大小size
	BSDRangeDate[1] = chn;//通道�?
#if 0
	BSDRangeDate[2] = tUI_CuSetting.ParkingLine_XY[chn][0];
	BSDRangeDate[3] = tUI_CuSetting.ParkingLine_XY[chn][1];

	BSDRangeDate[4] = tUI_CuSetting.ParkingLine_XY[chn][2];
	BSDRangeDate[5] = tUI_CuSetting.ParkingLine_XY[chn][3];

	BSDRangeDate[6] = tUI_CuSetting.ParkingLineSize[chn];
	CRCtemp = crc16_Gen(BSDRangeDate, 7);
	printf("crc is %x\n",CRCtemp);
	BSDRangeDate[7] = CRCtemp&0x00FF;
	BSDRangeDate[8] = (CRCtemp>>8)&0x00FF;
	UI_UART2_PutChar(0XFF);
	UI_UART2_PutChar(0XBB);	
	for(uint8_t i=0; i < 9;i++)		
	{
		UI_UART2_PutChar(BSDRangeDate[i]);
		//printf("BSDRangeDate[%d] = %x!!!\n",i,BSDRangeDate[i]);
	}
#else
	//四点画框数据、两点location	
		BSDRangeDate[2] = tParkinglinePoint.tupLeft.xPoint&0x00FF;
		BSDRangeDate[3] = (tParkinglinePoint.tupLeft.xPoint>>8)&0x00FF;

		BSDRangeDate[4] = tParkinglinePoint.tupRight.xPoint&0x00FF;
		BSDRangeDate[5] = (tParkinglinePoint.tupRight.xPoint>>8)&0x00FF;
			
		BSDRangeDate[6] = tParkinglinePoint.tdownLeft.xPoint&0x00FF;
		BSDRangeDate[7] = (tParkinglinePoint.tdownLeft.xPoint>>8)&0x00FF;

		
		BSDRangeDate[8] = tParkinglinePoint.tdownRight.xPoint&0x00FF;
		BSDRangeDate[9] = (tParkinglinePoint.tdownRight.xPoint>>8)&0x00FF;

		BSDRangeDate[10] = tParkinglinePoint.tupRight.yPoint&0x00FF;
		BSDRangeDate[11] = (tParkinglinePoint.tupRight.yPoint>>8)&0x00FF;
			
		BSDRangeDate[12] = tUI_CuSetting.tLocation1[chn]&0x00FF;
		BSDRangeDate[13] = (tUI_CuSetting.tLocation1[chn]>>8)&0x00FF;	
		BSDRangeDate[14] = tUI_CuSetting.tLocation2[chn]&0x00FF;
		BSDRangeDate[15] = (tUI_CuSetting.tLocation2[chn]>>8)&0x00FF;

		BSDRangeDate[16] = tParkinglinePoint.tdownLeft.yPoint&0x00FF;
		BSDRangeDate[17] = (tParkinglinePoint.tdownLeft.yPoint>>8)&0x00FF;		

		CRCtemp = crc16_Gen(BSDRangeDate, 18);
		printf("crc is %x\n",CRCtemp);
		BSDRangeDate[18] = CRCtemp&0x00FF;
		BSDRangeDate[19] = (CRCtemp>>8)&0x00FF;
		osMutexWait(osAIConfigMutex, osWaitForever);
		UI_UART2_PutChar(0XFF);
		UI_UART2_PutChar(0XBB); 
		for(uint8_t i=0; i < 20;i++) 	
		{
			UI_UART2_PutChar(BSDRangeDate[i]);
			//printf("BSDRangeDate[%d] = %x!!!\n",i,BSDRangeDate[i]);
		}

		osMutexRelease(osAIConfigMutex);

#endif
}
void UI_DrawTxBatteryInfoIcon(void)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint16_t uwLcd_HSize = uwLCD_GetLcdHoSize();
	uint16_t uwLcd_VSize = uwLCD_GetLcdVoSize();
	switch(tCamViewSel.tCamViewType)
	{
		case SINGLE_VIEW:
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DESKTOP_BATTERY_NO + tUI_CamStatus[tCamViewSel.tCamViewPool[0]].tCamBatLvl, 1, &tOsdImgInfo);
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			break;
		case DUAL_VIEW:
			//----------------------left---------------------------------
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DESKTOP_BATTERY_NO + tUI_CamStatus[tCamViewSel.tCamViewPool[0]].tCamBatLvl, 1, &tOsdImgInfo);
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
			//---------------------right----------------------------------
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DESKTOP_BATTERY_NO + tUI_CamStatus[tCamViewSel.tCamViewPool[1]].tCamBatLvl, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart += uwLcd_HSize/2;
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			break;
		case QUAD_VIEW:
			//--------------------------------upper left------------------------------
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DESKTOP_BATTERY_NO + tUI_CamStatus[tCamViewSel.tCamViewPool[0]].tCamBatLvl, 1, &tOsdImgInfo);
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE); 
			//-------------------------------upper right---------------------------------
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DESKTOP_BATTERY_NO + tUI_CamStatus[tCamViewSel.tCamViewPool[1]].tCamBatLvl, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart += uwLcd_HSize/2;
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE); 
			//-----------------------------lower left------------------------------------
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DESKTOP_BATTERY_NO + tUI_CamStatus[tCamViewSel.tCamViewPool[2]].tCamBatLvl, 1, &tOsdImgInfo);
			tOsdImgInfo.uwYStart += uwLcd_VSize/2;
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE); 
			//---------------------------lower right-------------------------------------
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DESKTOP_BATTERY_NO + tUI_CamStatus[tCamViewSel.tCamViewPool[3]].tCamBatLvl, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart += uwLcd_HSize/2;
			tOsdImgInfo.uwYStart += uwLcd_VSize/2;
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			break;

		default:
			break;
	}
}
//-------------------------------BSD-----------------------------------------------
unsigned char point_div = 2;
unsigned char point_interval = 2;
void UI_DrawArbitrarilyLine(int xleft,int yleft,int xright,int yright,OSD_IMG_INFO * tOsdImgInfo, unsigned char FLAG,OSD_LAYER_TYP OSD_TYP)
{
	int xmid,ymid;
	int xlen,ylen;
	int dx,dy;
	static unsigned char div = 2;
	static unsigned char interval = 2; //点间�?
	static unsigned char point = 0; //要画几个�?
	unsigned char i;

	if(FLAG == 1)  
	{
		if(xright>xleft) 
		{
			xlen = xright - xleft;
		}else
		{
			xlen = xleft - xright;
		}

		if(yright>yleft)
		{
			ylen = yright - yleft;
		}else
		{
			ylen = yleft - yright;
		}
    	point = (sqrt(xlen * xlen + ylen * ylen)) / 4; //4个像素一个色�?
		//printf("point = %d\n",point);

		//printf("xlen:%d,ylen:%d\n",xlen,ylen);

		div = point_div;
		
		interval = point_interval;

		// if(point <= 24 || point >= 44)
		// 	interval = 7;
		// else interval = 6;

		//printf("current_div = %d,current_interval = %d\n",div,interval );


	}

	if(xright>xleft) 
	{
		xlen = xright - xleft;
		xmid = (xright-xleft)/div + xleft;
	}else
	{
		xlen = xleft - xright;
		xmid = xleft - (xleft-xright)/div ;
	}
	
	if(yright>yleft)
	{
		ylen = yright - yleft;
		ymid = (yright-yleft)/div + yleft;
	}else
	{
		ylen = yleft - yright;
		ymid = yleft - (yleft-yright)/div ;
	}


	tOsdImgInfo->uwXStart = xmid;
	tOsdImgInfo->uwYStart = ymid;
	//if(xlen < 12 && ylen < 12 ) return; //æŽ§åˆ¶æ–œçº¿çš„ç‚¹è·�ç¦»
	if((sqrt(xlen * xlen + ylen * ylen)) < interval * 2)  return;
	if(OSD_TYP == OSD_IMG1)
	{
		tOSD_Img1_NoCopy(tOsdImgInfo, OSD_QUEUE);
		UI_DrawArbitrarilyLine( xleft, yleft, xmid, ymid, tOsdImgInfo,0,OSD_IMG1);
		UI_DrawArbitrarilyLine( xmid, ymid, xright, yright, tOsdImgInfo,0,OSD_IMG1);
	}
	else
	{	
		tOSD_Img2(tOsdImgInfo, OSD_QUEUE);
		UI_DrawArbitrarilyLine( xleft, yleft, xmid, ymid, tOsdImgInfo,0,OSD_IMG2);
		UI_DrawArbitrarilyLine( xmid, ymid, xright, yright, tOsdImgInfo,0,OSD_IMG2);
	}

}


void UI_DrawHorizontalLine(int xleft,int yleft,int xright,int yright,OSD_IMG_INFO * tOsdImgInfo,OSD_LAYER_TYP OSD_TYP)
{
	int xmid,ymid;
	int xlen,ylen;
	if(xright>xleft) 
	{
		xlen = xright - xleft;
		xmid = (xright-xleft)/2 + xleft;
	}else
	{
		xlen = xleft - xright;
		xmid = (xleft-xright)/2 + xright;
	}
	if(yright>yleft)
	{
		ylen = yright - yleft;
		ymid = (yright-yleft)/2 + yleft;
	}else
	{
		ylen = yleft - yright;
		ymid = (yleft-yright)/2 + yright;
	}
	tOsdImgInfo->uwXStart = xmid;
	tOsdImgInfo->uwYStart = ymid;
	if(xlen < 4 && ylen < 4) return; //控制直线的点距离，因为点�?*4�?4表示画实�?
	if(OSD_TYP == OSD_IMG1)
	{
		tOSD_Img1_NoCopy(tOsdImgInfo, OSD_QUEUE);
		UI_DrawHorizontalLine( xleft, yleft, xmid, ymid, tOsdImgInfo,OSD_IMG1);
		UI_DrawHorizontalLine( xmid, ymid, xright, yright, tOsdImgInfo,OSD_IMG1);
	}
	else
	{	
		tOSD_Img2(tOsdImgInfo, OSD_QUEUE);
		UI_DrawHorizontalLine( xleft, yleft, xmid, ymid, tOsdImgInfo,OSD_IMG2);
		UI_DrawHorizontalLine( xmid, ymid, xright, yright, tOsdImgInfo,OSD_IMG2);
	}

}
void UI_AIClearPoint(uint8_t Index, uint8_t chn)
{
	OSD_IMG_INFO tOsdImgInfo1;
	uint8_t tUI_Select_SeeK;
	if(AILineRemoteMode == UnSelect)
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_AI_POINT_UNSELECT, 1, &tOsdImgInfo1);
	else
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_AI_POINT_SELECTED, 1, &tOsdImgInfo1);

	tUI_Select_SeeK = tOsdImgInfo1.uwHSize/2;

	switch(Index)
	{
		case PonitUpLeft:
			tOsdImgInfo1.uwXStart = tUI_CuSetting.tUI_AIDetectlinePoint[chn].tupLeft.xPoint - tUI_Select_SeeK;
			tOsdImgInfo1.uwYStart = tUI_CuSetting.tUI_AIDetectlinePoint[chn].tupLeft.yPoint - tUI_Select_SeeK;
			OSD_EraserImg2(&tOsdImgInfo1);
			break;
		case PonitUpRight:
			tOsdImgInfo1.uwXStart = tUI_CuSetting.tUI_AIDetectlinePoint[chn].tupRight.xPoint - tUI_Select_SeeK;
			tOsdImgInfo1.uwYStart = tUI_CuSetting.tUI_AIDetectlinePoint[chn].tupRight.yPoint - tUI_Select_SeeK;
			OSD_EraserImg2(&tOsdImgInfo1);
			break;
		case PonitDownLeft:
			tOsdImgInfo1.uwXStart = tUI_CuSetting.tUI_AIDetectlinePoint[chn].tdownLeft.xPoint - tUI_Select_SeeK;
			tOsdImgInfo1.uwYStart = tUI_CuSetting.tUI_AIDetectlinePoint[chn].tdownLeft.yPoint - tUI_Select_SeeK;
			OSD_EraserImg2(&tOsdImgInfo1);
			break;
		case PonitDownRight:
			tOsdImgInfo1.uwXStart = tUI_CuSetting.tUI_AIDetectlinePoint[chn].tdownRight.xPoint - tUI_Select_SeeK;
			tOsdImgInfo1.uwYStart = tUI_CuSetting.tUI_AIDetectlinePoint[chn].tdownRight.yPoint - tUI_Select_SeeK;
			OSD_EraserImg2(&tOsdImgInfo1);
			break;
		case SecondLine:
			tOsdImgInfo1.uwXStart = LINE2_Mid_XPoint - tUI_Select_SeeK;
			tOsdImgInfo1.uwYStart = LINE2_Mid_YPoint - tUI_Select_SeeK;
			OSD_EraserImg2(&tOsdImgInfo1);
			break;
		case ThirdLine:
			tOsdImgInfo1.uwXStart = LINE3_Mid_XPoint - tUI_Select_SeeK;
			tOsdImgInfo1.uwYStart = LINE3_Mid_YPoint - tUI_Select_SeeK;
			OSD_EraserImg2(&tOsdImgInfo1);
			break;
		case WholeArea:
			tOsdImgInfo1.uwXStart = (LINE3_Mid_XPoint - LINE2_Mid_XPoint)/2 + LINE2_Mid_XPoint - tUI_Select_SeeK;
			tOsdImgInfo1.uwYStart = (LINE3_Mid_YPoint - LINE2_Mid_YPoint)/2 + LINE2_Mid_YPoint - tUI_Select_SeeK;
			OSD_EraserImg2(&tOsdImgInfo1);
			break;

		default:
			break;	

	}


}

void UI_AIDrawPoint(uint8_t Index,uint8_t chn)
{
	OSD_IMG_INFO tOsdImgInfo1;
	int16_t X_temp,Y_temp;
	uint8_t tUI_Select_SeeK ;
	if(AILineRemoteMode == UnSelect)
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_AI_POINT_UNSELECT, 1, &tOsdImgInfo1);
	else
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_AI_POINT_SELECTED, 1, &tOsdImgInfo1);

	tUI_Select_SeeK = tOsdImgInfo1.uwHSize/2;	
	switch(Index)
	{
		case PonitUpLeft:
			X_temp = tUI_CuSetting.tUI_AIDetectlinePoint[chn].tupLeft.xPoint - tUI_Select_SeeK;
			Y_temp = tUI_CuSetting.tUI_AIDetectlinePoint[chn].tupLeft.yPoint - tUI_Select_SeeK;
			break;
		case PonitUpRight:
			X_temp = tUI_CuSetting.tUI_AIDetectlinePoint[chn].tupRight.xPoint - tUI_Select_SeeK;
			Y_temp = tUI_CuSetting.tUI_AIDetectlinePoint[chn].tupRight.yPoint - tUI_Select_SeeK;
			break;
		case PonitDownLeft:
			X_temp = tUI_CuSetting.tUI_AIDetectlinePoint[chn].tdownLeft.xPoint - tUI_Select_SeeK;
			Y_temp = tUI_CuSetting.tUI_AIDetectlinePoint[chn].tdownLeft.yPoint - tUI_Select_SeeK;
			break;
		case PonitDownRight:
			X_temp = tUI_CuSetting.tUI_AIDetectlinePoint[chn].tdownRight.xPoint - tUI_Select_SeeK;
			Y_temp = tUI_CuSetting.tUI_AIDetectlinePoint[chn].tdownRight.yPoint - tUI_Select_SeeK;
			break;
		case SecondLine:
			X_temp = LINE2_Mid_XPoint - tUI_Select_SeeK;
			Y_temp = LINE2_Mid_YPoint - tUI_Select_SeeK;
			break;
		case ThirdLine:
			X_temp = LINE3_Mid_XPoint - tUI_Select_SeeK;
			Y_temp = LINE3_Mid_YPoint - tUI_Select_SeeK;
			break;
		case WholeArea:
			X_temp = (LINE3_Mid_XPoint - LINE2_Mid_XPoint)/2 + LINE2_Mid_XPoint - tUI_Select_SeeK;
			Y_temp = (LINE3_Mid_YPoint - LINE2_Mid_YPoint)/2 + LINE2_Mid_YPoint - tUI_Select_SeeK;
			break;

		default:
			break;	
	}
	if(X_temp < 0)
	{
		X_temp = 0;
	}
	else if(X_temp > 1024 - tOsdImgInfo1.uwHSize)
	{
		X_temp = 1024 - tOsdImgInfo1.uwHSize;
	}
	if(Y_temp < 0)
	{
		Y_temp = 0;
	}
	else if(Y_temp > 600 - tOsdImgInfo1.uwVSize)
	{
		Y_temp = 600 - tOsdImgInfo1.uwVSize;
	}
	tOsdImgInfo1.uwXStart = X_temp;
	tOsdImgInfo1.uwYStart = Y_temp;

	tOSD_Img2(&tOsdImgInfo1, OSD_UPDATE);

	return;
}


void UI_AutoDrawBSD(uint8_t chn)
{
	OSD_IMG_INFO tOsdImgInfo1;	

////绿色区域	
//	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_BSD_RANGE_DOTS_GREEN, 1, &tOsdImgInfo1);
//	UI_DrawHorizontalLine(UI_AIDetectlinePoint[chn].tupLeft.xPoint,UI_AIDetectlinePoint[chn].tupLeft.yPoint,UI_AIDetectlinePoint[chn].tupRight.xPoint,UI_AIDetectlinePoint[chn].tupRight.yPoint,&tOsdImgInfo1);
//	UI_DrawArbitrarilyLine(UI_AIDetectlinePoint[chn].tupLeft.xPoint,UI_AIDetectlinePoint[chn].tupLeft.yPoint,UI_AIDetectlinePoint[chn].tmid_upLeft.xPoint,UI_AIDetectlinePoint[chn].tmid_upLeft.yPoint,&tOsdImgInfo1,1);
//	UI_DrawArbitrarilyLine(UI_AIDetectlinePoint[chn].tupRight.xPoint,UI_AIDetectlinePoint[chn].tupRight.yPoint,UI_AIDetectlinePoint[chn].tmid_upRight.xPoint,UI_AIDetectlinePoint[chn].tmid_upRight.yPoint,&tOsdImgInfo1,1);
////黄色区域
//	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_BSD_RANGE_DOTS_YELLOW, 1, &tOsdImgInfo1);
//	UI_DrawHorizontalLine(UI_AIDetectlinePoint[chn].tmid_upLeft.xPoint,UI_AIDetectlinePoint[chn].tmid_upLeft.yPoint,UI_AIDetectlinePoint[chn].tmid_upRight.xPoint,UI_AIDetectlinePoint[chn].tmid_upRight.yPoint,&tOsdImgInfo1);
//	UI_DrawArbitrarilyLine(UI_AIDetectlinePoint[chn].tmid_upLeft.xPoint,UI_AIDetectlinePoint[chn].tmid_upLeft.yPoint,UI_AIDetectlinePoint[chn].tmid_downLeft.xPoint,UI_AIDetectlinePoint[chn].tmid_downLeft.yPoint,&tOsdImgInfo1,1);
//	UI_DrawArbitrarilyLine(UI_AIDetectlinePoint[chn].tmid_upRight.xPoint,UI_AIDetectlinePoint[chn].tmid_upRight.yPoint,UI_AIDetectlinePoint[chn].tmid_downRight.xPoint,UI_AIDetectlinePoint[chn].tmid_downRight.yPoint,&tOsdImgInfo1,1);
////红色区域
//	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_BSD_RANGE_DOTS_RED, 1, &tOsdImgInfo1);
//	UI_DrawHorizontalLine(UI_AIDetectlinePoint[chn].tmid_downLeft.xPoint,UI_AIDetectlinePoint[chn].tmid_downLeft.yPoint,UI_AIDetectlinePoint[chn].tmid_downRight.xPoint,UI_AIDetectlinePoint[chn].tmid_downRight.yPoint,&tOsdImgInfo1);
//	UI_DrawArbitrarilyLine(UI_AIDetectlinePoint[chn].tmid_downLeft.xPoint,UI_AIDetectlinePoint[chn].tmid_downLeft.yPoint,UI_AIDetectlinePoint[chn].tdownLeft.xPoint,UI_AIDetectlinePoint[chn].tdownLeft.yPoint,&tOsdImgInfo1,1);
//	UI_DrawArbitrarilyLine(UI_AIDetectlinePoint[chn].tmid_downRight.xPoint,UI_AIDetectlinePoint[chn].tmid_downRight.yPoint,UI_AIDetectlinePoint[chn].tdownRight.xPoint,UI_AIDetectlinePoint[chn].tdownRight.yPoint,&tOsdImgInfo1,1);	
//	UI_DrawHorizontalLine(UI_AIDetectlinePoint[chn].tdownRight.xPoint,UI_AIDetectlinePoint[chn].tdownRight.yPoint,UI_AIDetectlinePoint[chn].tdownLeft.xPoint,UI_AIDetectlinePoint[chn].tdownLeft.yPoint,&tOsdImgInfo1);
#if 0 //通过八点画检测区域功�?
	//绿色区域	
		tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_BSD_RANGE_DOTS_GREEN, 1, &tOsdImgInfo1);
		UI_DrawHorizontalLine(tUI_CuSetting.tUI_AIDetectlinePoint[chn].tupLeft.xPoint,tUI_CuSetting.tUI_AIDetectlinePoint[chn].tupLeft.yPoint,tUI_CuSetting.tUI_AIDetectlinePoint[chn].tupRight.xPoint,tUI_CuSetting.tUI_AIDetectlinePoint[chn].tupRight.yPoint,&tOsdImgInfo1);
		UI_DrawArbitrarilyLine(tUI_CuSetting.tUI_AIDetectlinePoint[chn].tupLeft.xPoint,tUI_CuSetting.tUI_AIDetectlinePoint[chn].tupLeft.yPoint,tUI_CuSetting.tUI_AIDetectlinePoint[chn].tmid_upLeft.xPoint,tUI_CuSetting.tUI_AIDetectlinePoint[chn].tmid_upLeft.yPoint,&tOsdImgInfo1,1);
		UI_DrawArbitrarilyLine(tUI_CuSetting.tUI_AIDetectlinePoint[chn].tupRight.xPoint,tUI_CuSetting.tUI_AIDetectlinePoint[chn].tupRight.yPoint,tUI_CuSetting.tUI_AIDetectlinePoint[chn].tmid_upRight.xPoint,tUI_CuSetting.tUI_AIDetectlinePoint[chn].tmid_upRight.yPoint,&tOsdImgInfo1,1);
	//黄色区域
		tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_BSD_RANGE_DOTS_YELLOW, 1, &tOsdImgInfo1);
		UI_DrawHorizontalLine(tUI_CuSetting.tUI_AIDetectlinePoint[chn].tmid_upLeft.xPoint,tUI_CuSetting.tUI_AIDetectlinePoint[chn].tmid_upLeft.yPoint,tUI_CuSetting.tUI_AIDetectlinePoint[chn].tmid_upRight.xPoint,tUI_CuSetting.tUI_AIDetectlinePoint[chn].tmid_upRight.yPoint,&tOsdImgInfo1);
		UI_DrawArbitrarilyLine(tUI_CuSetting.tUI_AIDetectlinePoint[chn].tmid_upLeft.xPoint,tUI_CuSetting.tUI_AIDetectlinePoint[chn].tmid_upLeft.yPoint,tUI_CuSetting.tUI_AIDetectlinePoint[chn].tmid_downLeft.xPoint,tUI_CuSetting.tUI_AIDetectlinePoint[chn].tmid_downLeft.yPoint,&tOsdImgInfo1,1);
		UI_DrawArbitrarilyLine(tUI_CuSetting.tUI_AIDetectlinePoint[chn].tmid_upRight.xPoint,tUI_CuSetting.tUI_AIDetectlinePoint[chn].tmid_upRight.yPoint,tUI_CuSetting.tUI_AIDetectlinePoint[chn].tmid_downRight.xPoint,tUI_CuSetting.tUI_AIDetectlinePoint[chn].tmid_downRight.yPoint,&tOsdImgInfo1,1);
	//红色区域
		tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_BSD_RANGE_DOTS_RED, 1, &tOsdImgInfo1);
		UI_DrawHorizontalLine(tUI_CuSetting.tUI_AIDetectlinePoint[chn].tmid_downLeft.xPoint,tUI_CuSetting.tUI_AIDetectlinePoint[chn].tmid_downLeft.yPoint,tUI_CuSetting.tUI_AIDetectlinePoint[chn].tmid_downRight.xPoint,tUI_CuSetting.tUI_AIDetectlinePoint[chn].tmid_downRight.yPoint,&tOsdImgInfo1);
		UI_DrawArbitrarilyLine(tUI_CuSetting.tUI_AIDetectlinePoint[chn].tmid_downLeft.xPoint,tUI_CuSetting.tUI_AIDetectlinePoint[chn].tmid_downLeft.yPoint,tUI_CuSetting.tUI_AIDetectlinePoint[chn].tdownLeft.xPoint,tUI_CuSetting.tUI_AIDetectlinePoint[chn].tdownLeft.yPoint,&tOsdImgInfo1,1);
		UI_DrawArbitrarilyLine(tUI_CuSetting.tUI_AIDetectlinePoint[chn].tmid_downRight.xPoint,tUI_CuSetting.tUI_AIDetectlinePoint[chn].tmid_downRight.yPoint,tUI_CuSetting.tUI_AIDetectlinePoint[chn].tdownRight.xPoint,tUI_CuSetting.tUI_AIDetectlinePoint[chn].tdownRight.yPoint,&tOsdImgInfo1,1);	
		UI_DrawHorizontalLine(tUI_CuSetting.tUI_AIDetectlinePoint[chn].tdownRight.xPoint,tUI_CuSetting.tUI_AIDetectlinePoint[chn].tdownRight.yPoint,tUI_CuSetting.tUI_AIDetectlinePoint[chn].tdownLeft.xPoint,tUI_CuSetting.tUI_AIDetectlinePoint[chn].tdownLeft.yPoint,&tOsdImgInfo1);




	UI_AIDrawPoint(AILinePointIndex,chn);
#endif	
//	UI_DrawHorizontalLine(tUI_ParkinglinePoint.tupLeft.xPoint,tUI_ParkinglinePoint.tupLeft.yPoint,tUI_ParkinglinePoint.tupRight.xPoint,tUI_ParkinglinePoint.tupRight.yPoint,&tOsdImgInfo1);
//	//tOSD_Img1(&tOsdImgInfo1, OSD_UPDATE); //画线函数无OSD_UPDATE，需要额外OSD_UPDATE一�?
//	UI_DrawArbitrarilyLine(tUI_ParkinglinePoint.tdownLeft.xPoint,tUI_ParkinglinePoint.tdownLeft.yPoint,tUI_ParkinglinePoint.tupLeft.xPoint,tUI_ParkinglinePoint.tupLeft.yPoint,&tOsdImgInfo1);
//	//tOSD_Img1(&tOsdImgInfo1, OSD_UPDATE);
//	UI_DrawHorizontalLine(tUI_ParkinglinePoint.tdownLeft.xPoint,tUI_ParkinglinePoint.tdownLeft.yPoint,tUI_ParkinglinePoint.tdownRight.xPoint,tUI_ParkinglinePoint.tdownRight.yPoint,&tOsdImgInfo1);
//	//tOSD_Img1(&tOsdImgInfo1, OSD_UPDATE);
//	UI_DrawArbitrarilyLine(tUI_ParkinglinePoint.tupRight.xPoint,tUI_ParkinglinePoint.tupRight.yPoint,tUI_ParkinglinePoint.tdownRight.xPoint,tUI_ParkinglinePoint.tdownRight.yPoint,&tOsdImgInfo1);	
//	tOSD_Img2(&tOsdImgInfo1, OSD_UPDATE);
}

//-----------------------------------------------------add from pjw
//选择两个点，三等分画绿黄红的�?  Location1 : 第二根线位于整体�?x/24 ，Location2 : 第三根线位于整体�?x/24 ，越小越往上，越大越往�?
#define LineInterval 12  // 12等分
void UI_CONNECT_POINT3(UI_LinePoint_t tParkinglinePoint1 , UI_LinePoint_t tParkinglinePoint2 , UI_LinePoint_t tParkinglinePoint3, UI_LinePoint_t tParkinglinePoint4,uint8_t Location1,uint8_t Location2, OSD_LAYER_TYP OSD_TYP)
{
	OSD_IMG_INFO tOsdImgInfo1;	

	uint16_t  xlenL,ylenL;
	uint16_t  x1L,x2L;
	uint16_t  y1L,y2L;

	uint16_t  xlenR,ylenR;
	uint16_t  x1R,x2R;
	uint16_t  y1R,y2R;
	if(Location1 >= Location2)
		return;
	

	if(tParkinglinePoint2.xPoint > tParkinglinePoint1.xPoint)
	{
		xlenL = (tParkinglinePoint2.xPoint - tParkinglinePoint1.xPoint);
		x1L = (tParkinglinePoint1.xPoint + xlenL*Location1/LineInterval);
		x2L = (tParkinglinePoint1.xPoint + xlenL*Location2/LineInterval);
	}
	else
	{
		xlenL = (tParkinglinePoint1.xPoint - tParkinglinePoint2.xPoint);
		x2L = (tParkinglinePoint1.xPoint - xlenL*Location2/LineInterval);
		x1L = (tParkinglinePoint1.xPoint - xlenL*Location1/LineInterval);		
	}
	if(tParkinglinePoint2.yPoint > tParkinglinePoint1.yPoint)	
	{
		ylenL = (tParkinglinePoint2.yPoint - tParkinglinePoint1.yPoint);
		y1L = (tParkinglinePoint1.yPoint + ylenL*Location1/LineInterval);
		y2L = (tParkinglinePoint1.yPoint + ylenL*Location2/LineInterval);
	}
	else
	{
		ylenL = (tParkinglinePoint1.yPoint - tParkinglinePoint2.yPoint);
		y2L = (tParkinglinePoint2.yPoint + ylenL*Location1/LineInterval);
		y1L = (tParkinglinePoint2.yPoint + ylenL*Location2/LineInterval);
	}

	if(tParkinglinePoint4.xPoint > tParkinglinePoint3.xPoint)
	{
		xlenR = (tParkinglinePoint4.xPoint - tParkinglinePoint3.xPoint);
		x1R = (tParkinglinePoint3.xPoint + xlenR*Location1/LineInterval);
		x2R = (tParkinglinePoint3.xPoint + xlenR*Location2/LineInterval);
	}
	else
	{
		xlenR = (tParkinglinePoint3.xPoint - tParkinglinePoint4.xPoint);
		x2R = (tParkinglinePoint3.xPoint - xlenR*Location2/LineInterval);
		x1R = (tParkinglinePoint3.xPoint - xlenR*Location1/LineInterval);		
	}
	if(tParkinglinePoint4.yPoint > tParkinglinePoint3.yPoint)	
	{
		ylenR = (tParkinglinePoint4.yPoint - tParkinglinePoint3.yPoint);
		y1R = (tParkinglinePoint3.yPoint + ylenR*Location1/LineInterval);
		y2R = (tParkinglinePoint3.yPoint + ylenR*Location2/LineInterval);
	}
	else
	{
		ylenR = (tParkinglinePoint3.yPoint - tParkinglinePoint4.yPoint);
		y2R = (tParkinglinePoint4.yPoint + ylenR*Location1/LineInterval);
		y1R = (tParkinglinePoint4.yPoint + ylenR*Location2/LineInterval);
	}
	//更新第二三根线的中点左标
	LINE2_Mid_XPoint = (x1R - x1L)/2 + x1L;
	LINE2_Mid_YPoint = 	y1L;
	LINE3_Mid_XPoint = (x2R - x2L)/2 + x2L;
	LINE3_Mid_YPoint =  y2L;


	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_BSD_RANGE_DOTS_GREEN, 1, &tOsdImgInfo1);
	UI_DrawHorizontalLine(tParkinglinePoint1.xPoint,tParkinglinePoint1.yPoint,tParkinglinePoint3.xPoint,tParkinglinePoint3.yPoint,&tOsdImgInfo1,OSD_TYP);
	UI_DrawArbitrarilyLine(tParkinglinePoint1.xPoint,tParkinglinePoint1.yPoint,x1L,y1L,&tOsdImgInfo1,1,OSD_TYP);
	UI_DrawArbitrarilyLine(tParkinglinePoint3.xPoint,tParkinglinePoint3.yPoint,x1R,y1R,&tOsdImgInfo1,1,OSD_TYP);
	UI_DrawHorizontalLine(x1L,y1L,x1R,y1R,&tOsdImgInfo1,OSD_TYP);//第二�?

	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_BSD_RANGE_DOTS_YELLOW, 1, &tOsdImgInfo1);
	UI_DrawArbitrarilyLine(x1L,y1L,x2L,y2L,&tOsdImgInfo1,1,OSD_TYP);
	UI_DrawArbitrarilyLine(x1R,y1R,x2R,y2R,&tOsdImgInfo1,1,OSD_TYP);
	UI_DrawHorizontalLine(x2L,y2L,x2R,y2R,&tOsdImgInfo1,OSD_TYP);//第三�?

	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_BSD_RANGE_DOTS_RED, 1, &tOsdImgInfo1);
	UI_DrawArbitrarilyLine(x2L,y2L,tParkinglinePoint2.xPoint,tParkinglinePoint2.yPoint,&tOsdImgInfo1,1,OSD_TYP);
	UI_DrawArbitrarilyLine(x2R,y2R,tParkinglinePoint4.xPoint,tParkinglinePoint4.yPoint,&tOsdImgInfo1,1,OSD_TYP);
	UI_DrawHorizontalLine(tParkinglinePoint2.xPoint,tParkinglinePoint2.yPoint,tParkinglinePoint4.xPoint,tParkinglinePoint4.yPoint,&tOsdImgInfo1,OSD_TYP);


}

void UI_DRAW_BSDRANGE(UI_ParkinglinePoint_t tParkinglinePoint ,uint8_t location1 ,uint8_t location2,OSD_UPDATE_TYP UpdatType, OSD_LAYER_TYP OSD_TYP)
{
	//传入左上 左下 右上 右下 的点坐标 中间两根横线占y的比�?
	UI_CONNECT_POINT3(tParkinglinePoint.tupLeft, tParkinglinePoint.tdownLeft , tParkinglinePoint.tupRight, tParkinglinePoint.tdownRight, location1, location2,OSD_TYP);		
	if(UpdatType == OSD_UPDATE && OSD_TYP == OSD_IMG1)
		tOSD_Img1_Test();//防止刷图后消�?

	printf("draw BSD RANGE down\n");
}



//------------------------------------------------------------------------------
void UI_DrawBSDRange(void)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint16_t uwLcd_HSize = uwLCD_GetLcdHoSize();
	uint16_t uwLcd_VSize = uwLCD_GetLcdVoSize();
	uint8_t ubIsShow = 0;
	UI_ParkinglinePoint_t tBSDlinePoint = {0};
	MenuOnFlag = TRUE;
	switch(tCamViewSel.tCamViewType)
	{
		case SINGLE_VIEW:
			if(tUI_CuSetting.ubIsEnableBSDRANGE[tCamViewSel.tCamViewPool[0]] == TRUE)
			{
				UI_DRAW_BSDRANGE(tUI_CuSetting.tUI_AIDetectlinePoint[tCamViewSel.tCamViewPool[0]],tUI_CuSetting.tLocation1[tCamViewSel.tCamViewPool[0]],tUI_CuSetting.tLocation2[tCamViewSel.tCamViewPool[0]],OSD_UPDATE,OSD_IMG1);	
			}
			else
			{
				if(tUI_CuSetting.tParkingLineEnable[tCamViewSel.tCamViewPool[0]] == TRUE)
				{	
					UI_DrawParkingLine_OSD1(tCamViewSel.tCamViewPool[0],OSD_UPDATE);
				}
			}
				
			
			break;
		case DUAL_VIEW:
			//----------------------left---------------------------------
			//旧的检测区�?
			//UI_DrawParkingLine_OSD1_DUAL(tCamViewSel.tCamViewPool[0],OSD_UPDATE);
			//新的检测区�?
			if(tUI_CuSetting.ubIsEnableBSDRANGE[tCamViewSel.tCamViewPool[0]] == TRUE)
			{
				tBSDlinePoint.tupLeft.xPoint = tUI_CuSetting.tUI_AIDetectlinePoint[tCamViewSel.tCamViewPool[0]].tupLeft.xPoint/2;
				tBSDlinePoint.tupRight.xPoint = tUI_CuSetting.tUI_AIDetectlinePoint[tCamViewSel.tCamViewPool[0]].tupRight.xPoint/2;
				tBSDlinePoint.tdownLeft.xPoint = tUI_CuSetting.tUI_AIDetectlinePoint[tCamViewSel.tCamViewPool[0]].tdownLeft.xPoint/2;
				tBSDlinePoint.tdownRight.xPoint = tUI_CuSetting.tUI_AIDetectlinePoint[tCamViewSel.tCamViewPool[0]].tdownRight.xPoint/2;	

				tBSDlinePoint.tupLeft.yPoint = tUI_CuSetting.tUI_AIDetectlinePoint[tCamViewSel.tCamViewPool[0]].tupLeft.yPoint;
				tBSDlinePoint.tupRight.yPoint = tUI_CuSetting.tUI_AIDetectlinePoint[tCamViewSel.tCamViewPool[0]].tupRight.yPoint;
				tBSDlinePoint.tdownLeft.yPoint = tUI_CuSetting.tUI_AIDetectlinePoint[tCamViewSel.tCamViewPool[0]].tdownLeft.yPoint;
				tBSDlinePoint.tdownRight.yPoint = tUI_CuSetting.tUI_AIDetectlinePoint[tCamViewSel.tCamViewPool[0]].tdownRight.yPoint;
				
				UI_DRAW_BSDRANGE(tBSDlinePoint,tUI_CuSetting.tLocation1[tCamViewSel.tCamViewPool[0]],tUI_CuSetting.tLocation2[tCamViewSel.tCamViewPool[0]],OSD_QUEUE,OSD_IMG1);	
				ubIsShow = 1;

			}
			if(tUI_CuSetting.ubIsEnableBSDRANGE[tCamViewSel.tCamViewPool[1]] == TRUE)
			{
				tBSDlinePoint.tupLeft.xPoint = tUI_CuSetting.tUI_AIDetectlinePoint[tCamViewSel.tCamViewPool[1]].tupLeft.xPoint/2 + 512;
				tBSDlinePoint.tupRight.xPoint = tUI_CuSetting.tUI_AIDetectlinePoint[tCamViewSel.tCamViewPool[1]].tupRight.xPoint/2 + 512;
				tBSDlinePoint.tdownLeft.xPoint = tUI_CuSetting.tUI_AIDetectlinePoint[tCamViewSel.tCamViewPool[1]].tdownLeft.xPoint/2 + 512;
				tBSDlinePoint.tdownRight.xPoint = tUI_CuSetting.tUI_AIDetectlinePoint[tCamViewSel.tCamViewPool[1]].tdownRight.xPoint/2 + 512;	

				tBSDlinePoint.tupLeft.yPoint = tUI_CuSetting.tUI_AIDetectlinePoint[tCamViewSel.tCamViewPool[1]].tupLeft.yPoint;
				tBSDlinePoint.tupRight.yPoint = tUI_CuSetting.tUI_AIDetectlinePoint[tCamViewSel.tCamViewPool[1]].tupRight.yPoint;
				tBSDlinePoint.tdownLeft.yPoint = tUI_CuSetting.tUI_AIDetectlinePoint[tCamViewSel.tCamViewPool[1]].tdownLeft.yPoint;
				tBSDlinePoint.tdownRight.yPoint = tUI_CuSetting.tUI_AIDetectlinePoint[tCamViewSel.tCamViewPool[1]].tdownRight.yPoint;	
				UI_DRAW_BSDRANGE(tBSDlinePoint,tUI_CuSetting.tLocation1[tCamViewSel.tCamViewPool[1]],tUI_CuSetting.tLocation2[tCamViewSel.tCamViewPool[1]],OSD_UPDATE,OSD_IMG1);
				ubIsShow = 0;
			}
			else
			{
				if(ubIsShow == 1)
				{
					tOSD_Img1_Test();
					ubIsShow = 0;
				}
			}
//			tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_BSD_RANGE_DUAL, 1, &tOsdImgInfo);
//			if(tUI_CuSetting.tParkingLineEnable[tCamViewSel.tCamViewPool[0]] == TRUE)
//			{
//				tOSD_Img1(&tOsdImgInfo, OSD_QUEUE);
//				ubIsShow = 1;
//			}
//			//---------------------right----------------------------------
//			if(tUI_CuSetting.tParkingLineEnable[tCamViewSel.tCamViewPool[1]] == TRUE)
//			{
//				tOsdImgInfo.uwXStart += uwLcd_HSize/2;
//				tOSD_Img1(&tOsdImgInfo, OSD_UPDATE);
//			}
//			else
//			{
//				if (ubIsShow == 1)
//				{
//					tOSD_Img1(&tOsdImgInfo, OSD_UPDATE);
//				}
//			}
			break;
		case QUAD_VIEW:
			//UI_DrawParkingLine_OSD1_QUAL(tCamViewSel.tCamViewPool[3],OSD_UPDATE);
//新的检测区�?
//			//--------------------------------upper left------------------------------
			if(tUI_CuSetting.ubIsEnableBSDRANGE[tCamViewSel.tCamViewPool[0]] == TRUE)
			{
				tBSDlinePoint.tupLeft.xPoint = tUI_CuSetting.tUI_AIDetectlinePoint[tCamViewSel.tCamViewPool[0]].tupLeft.xPoint/2;
				tBSDlinePoint.tupRight.xPoint = tUI_CuSetting.tUI_AIDetectlinePoint[tCamViewSel.tCamViewPool[0]].tupRight.xPoint/2;
				tBSDlinePoint.tdownLeft.xPoint = tUI_CuSetting.tUI_AIDetectlinePoint[tCamViewSel.tCamViewPool[0]].tdownLeft.xPoint/2;
				tBSDlinePoint.tdownRight.xPoint = tUI_CuSetting.tUI_AIDetectlinePoint[tCamViewSel.tCamViewPool[0]].tdownRight.xPoint/2; 

				tBSDlinePoint.tupLeft.yPoint = tUI_CuSetting.tUI_AIDetectlinePoint[tCamViewSel.tCamViewPool[0]].tupLeft.yPoint/2;
				tBSDlinePoint.tupRight.yPoint = tUI_CuSetting.tUI_AIDetectlinePoint[tCamViewSel.tCamViewPool[0]].tupRight.yPoint/2;
				tBSDlinePoint.tdownLeft.yPoint = tUI_CuSetting.tUI_AIDetectlinePoint[tCamViewSel.tCamViewPool[0]].tdownLeft.yPoint/2;
				tBSDlinePoint.tdownRight.yPoint = tUI_CuSetting.tUI_AIDetectlinePoint[tCamViewSel.tCamViewPool[0]].tdownRight.yPoint/2;
				
				UI_DRAW_BSDRANGE(tBSDlinePoint,tUI_CuSetting.tLocation1[tCamViewSel.tCamViewPool[0]],tUI_CuSetting.tLocation2[tCamViewSel.tCamViewPool[0]],OSD_QUEUE,OSD_IMG1);	
				ubIsShow = 1;
			}
//			//--------------------------------upper right---------------------------------
			if(tUI_CuSetting.ubIsEnableBSDRANGE[tCamViewSel.tCamViewPool[1]] == TRUE)
			{
				tBSDlinePoint.tupLeft.xPoint = tUI_CuSetting.tUI_AIDetectlinePoint[tCamViewSel.tCamViewPool[1]].tupLeft.xPoint/2 + 512;
				tBSDlinePoint.tupRight.xPoint = tUI_CuSetting.tUI_AIDetectlinePoint[tCamViewSel.tCamViewPool[1]].tupRight.xPoint/2 + 512;
				tBSDlinePoint.tdownLeft.xPoint = tUI_CuSetting.tUI_AIDetectlinePoint[tCamViewSel.tCamViewPool[1]].tdownLeft.xPoint/2 + 512;
				tBSDlinePoint.tdownRight.xPoint = tUI_CuSetting.tUI_AIDetectlinePoint[tCamViewSel.tCamViewPool[1]].tdownRight.xPoint/2 + 512;	

				tBSDlinePoint.tupLeft.yPoint = tUI_CuSetting.tUI_AIDetectlinePoint[tCamViewSel.tCamViewPool[1]].tupLeft.yPoint/2;
				tBSDlinePoint.tupRight.yPoint = tUI_CuSetting.tUI_AIDetectlinePoint[tCamViewSel.tCamViewPool[1]].tupRight.yPoint/2;
				tBSDlinePoint.tdownLeft.yPoint = tUI_CuSetting.tUI_AIDetectlinePoint[tCamViewSel.tCamViewPool[1]].tdownLeft.yPoint/2;
				tBSDlinePoint.tdownRight.yPoint = tUI_CuSetting.tUI_AIDetectlinePoint[tCamViewSel.tCamViewPool[1]].tdownRight.yPoint/2;	
				
				UI_DRAW_BSDRANGE(tBSDlinePoint,tUI_CuSetting.tLocation1[tCamViewSel.tCamViewPool[1]],tUI_CuSetting.tLocation2[tCamViewSel.tCamViewPool[1]],OSD_QUEUE,OSD_IMG1);	
				ubIsShow = 1;
			}

//			//--------------------------------lower left------------------------------------
				if(tUI_CuSetting.ubIsEnableBSDRANGE[tCamViewSel.tCamViewPool[2]] == TRUE)
				{
					tBSDlinePoint.tupLeft.xPoint = tUI_CuSetting.tUI_AIDetectlinePoint[tCamViewSel.tCamViewPool[2]].tupLeft.xPoint/2;
					tBSDlinePoint.tupRight.xPoint = tUI_CuSetting.tUI_AIDetectlinePoint[tCamViewSel.tCamViewPool[2]].tupRight.xPoint/2;
					tBSDlinePoint.tdownLeft.xPoint = tUI_CuSetting.tUI_AIDetectlinePoint[tCamViewSel.tCamViewPool[2]].tdownLeft.xPoint/2;
					tBSDlinePoint.tdownRight.xPoint = tUI_CuSetting.tUI_AIDetectlinePoint[tCamViewSel.tCamViewPool[2]].tdownRight.xPoint/2;	

					tBSDlinePoint.tupLeft.yPoint = tUI_CuSetting.tUI_AIDetectlinePoint[tCamViewSel.tCamViewPool[2]].tupLeft.yPoint/2 + 300;
					tBSDlinePoint.tupRight.yPoint = tUI_CuSetting.tUI_AIDetectlinePoint[tCamViewSel.tCamViewPool[2]].tupRight.yPoint/2 + 300;
					tBSDlinePoint.tdownLeft.yPoint = tUI_CuSetting.tUI_AIDetectlinePoint[tCamViewSel.tCamViewPool[2]].tdownLeft.yPoint/2 + 300;
					tBSDlinePoint.tdownRight.yPoint = tUI_CuSetting.tUI_AIDetectlinePoint[tCamViewSel.tCamViewPool[2]].tdownRight.yPoint/2 + 300;	
					
					UI_DRAW_BSDRANGE(tBSDlinePoint,tUI_CuSetting.tLocation1[tCamViewSel.tCamViewPool[2]],tUI_CuSetting.tLocation2[tCamViewSel.tCamViewPool[2]],OSD_QUEUE,OSD_IMG1);	
					ubIsShow = 1;
				}
//			//--------------------------------lower right-------------------------------------
				if(tUI_CuSetting.ubIsEnableBSDRANGE[tCamViewSel.tCamViewPool[3]] == TRUE)
				{
					tBSDlinePoint.tupLeft.xPoint = tUI_CuSetting.tUI_AIDetectlinePoint[tCamViewSel.tCamViewPool[3]].tupLeft.xPoint/2 + 512;
					tBSDlinePoint.tupRight.xPoint = tUI_CuSetting.tUI_AIDetectlinePoint[tCamViewSel.tCamViewPool[3]].tupRight.xPoint/2 + 512;
					tBSDlinePoint.tdownLeft.xPoint = tUI_CuSetting.tUI_AIDetectlinePoint[tCamViewSel.tCamViewPool[3]].tdownLeft.xPoint/2 + 512;
					tBSDlinePoint.tdownRight.xPoint = tUI_CuSetting.tUI_AIDetectlinePoint[tCamViewSel.tCamViewPool[3]].tdownRight.xPoint/2 + 512;	

					tBSDlinePoint.tupLeft.yPoint = tUI_CuSetting.tUI_AIDetectlinePoint[tCamViewSel.tCamViewPool[3]].tupLeft.yPoint/2 + 300;
					tBSDlinePoint.tupRight.yPoint = tUI_CuSetting.tUI_AIDetectlinePoint[tCamViewSel.tCamViewPool[3]].tupRight.yPoint/2 + 300;
					tBSDlinePoint.tdownLeft.yPoint = tUI_CuSetting.tUI_AIDetectlinePoint[tCamViewSel.tCamViewPool[3]].tdownLeft.yPoint/2 + 300;
					tBSDlinePoint.tdownRight.yPoint = tUI_CuSetting.tUI_AIDetectlinePoint[tCamViewSel.tCamViewPool[3]].tdownRight.yPoint/2 + 300;	
					
					UI_DRAW_BSDRANGE(tBSDlinePoint,tUI_CuSetting.tLocation1[tCamViewSel.tCamViewPool[3]],tUI_CuSetting.tLocation2[tCamViewSel.tCamViewPool[3]],OSD_UPDATE,OSD_IMG1);	
					ubIsShow = 0;
				}
				else
				{
					if(ubIsShow == 1)
					{
						tOSD_Img1_Test();
						ubIsShow = 0;
					}
				}

			break;
		default:
			break;
	}
	MenuOnFlag = FALSE;
}

//------------------------------------------------------------------------------
void UI_DrawSignalInfoIcon(OSD_UPDATE_TYP update_type)
{
	OSD_IMG_INFO tOsdImgInfo,tEraserOsdImgInfo;
	uint16_t uwLcd_HSize = uwLCD_GetLcdHoSize();
	uint16_t uwLcd_VSize = uwLCD_GetLcdVoSize();
	//printf("uwLcd_HSize = %d,uwLcd_VSize = %d\n",uwLcd_HSize,uwLcd_VSize);
	switch(tCamViewSel.tCamViewType)
	{
		case SINGLE_VIEW:
			// 清除信号图标背景中的旧框线，再绘制当前信号图标。
			tEraserOsdImgInfo.uwXStart = 4;
			tEraserOsdImgInfo.uwYStart = 4;
			tEraserOsdImgInfo.uwHSize = 106;
			tEraserOsdImgInfo.uwVSize = 46;
			OSD_EraserImg2_NoUpdate(&tEraserOsdImgInfo);
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DESKTOP_SIGNAL_LEVEL0 + tUI_CamStatus[tCamViewSel.tCamViewPool[0]].tCamAntLvl, 1, &tOsdImgInfo);
			tOSD_Img2(&tOsdImgInfo, update_type);
			break;
		case DUAL_VIEW:
			// 清除信号图标背景中的旧框线，再绘制当前信号图标。
			tEraserOsdImgInfo.uwXStart = 4;
			tEraserOsdImgInfo.uwYStart = 4;
			tEraserOsdImgInfo.uwHSize = 106;
			tEraserOsdImgInfo.uwVSize = 46;
			OSD_EraserImg2_NoUpdate(&tEraserOsdImgInfo);
			tEraserOsdImgInfo.uwXStart = 518;
			tEraserOsdImgInfo.uwYStart = 4;
			tEraserOsdImgInfo.uwHSize = 106;
			tEraserOsdImgInfo.uwVSize = 46;
			OSD_EraserImg2_NoUpdate(&tEraserOsdImgInfo);
			//----------------------left---------------------------------
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DESKTOP_SIGNAL_LEVEL0 + tUI_CamStatus[tCamViewSel.tCamViewPool[0]].tCamAntLvl, 1, &tOsdImgInfo);
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
			//---------------------right----------------------------------
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DESKTOP_SIGNAL_LEVEL0 + tUI_CamStatus[tCamViewSel.tCamViewPool[1]].tCamAntLvl, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart += uwLcd_HSize/2;
			tOSD_Img2(&tOsdImgInfo, update_type);
			break;
		case QUAD_VIEW:
			// 清除信号图标背景中的旧框线，再绘制当前信号图标。
			tEraserOsdImgInfo.uwXStart = 4;
			tEraserOsdImgInfo.uwYStart = 4;
			tEraserOsdImgInfo.uwHSize = 106;
			tEraserOsdImgInfo.uwVSize = 46;
			OSD_EraserImg2_NoUpdate(&tEraserOsdImgInfo);
			tEraserOsdImgInfo.uwXStart = 518;
			tEraserOsdImgInfo.uwYStart = 4;
			tEraserOsdImgInfo.uwHSize = 106;
			tEraserOsdImgInfo.uwVSize = 46;
			OSD_EraserImg2_NoUpdate(&tEraserOsdImgInfo);
			tEraserOsdImgInfo.uwXStart = 4;
			tEraserOsdImgInfo.uwYStart = 306;
			tEraserOsdImgInfo.uwHSize = 106;
			tEraserOsdImgInfo.uwVSize = 46;
			OSD_EraserImg2_NoUpdate(&tEraserOsdImgInfo);
			tEraserOsdImgInfo.uwXStart = 518;
			tEraserOsdImgInfo.uwYStart = 306;
			tEraserOsdImgInfo.uwHSize = 106;
			tEraserOsdImgInfo.uwVSize = 46;
			OSD_EraserImg2_NoUpdate(&tEraserOsdImgInfo);
			//--------------------------------upper left------------------------------
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DESKTOP_SIGNAL_LEVEL0 + tUI_CamStatus[tCamViewSel.tCamViewPool[0]].tCamAntLvl, 1, &tOsdImgInfo);
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
			//-------------------------------upper right---------------------------------
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DESKTOP_SIGNAL_LEVEL0 + tUI_CamStatus[tCamViewSel.tCamViewPool[1]].tCamAntLvl, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart += uwLcd_HSize/2;
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
			//-----------------------------lower left------------------------------------
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DESKTOP_SIGNAL_LEVEL0 + tUI_CamStatus[tCamViewSel.tCamViewPool[2]].tCamAntLvl, 1, &tOsdImgInfo);
			//tOsdImgInfo.uwYStart = uwLcd_VSize - (tOsdImgInfo.uwYStart + tOsdImgInfo.uwVSize);
			tOsdImgInfo.uwYStart += uwLcd_VSize/2;
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
			//---------------------------lower right-------------------------------------
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DESKTOP_SIGNAL_LEVEL0 + tUI_CamStatus[tCamViewSel.tCamViewPool[3]].tCamAntLvl, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart += uwLcd_HSize/2;
			//tOsdImgInfo.uwYStart = uwLcd_VSize - (tOsdImgInfo.uwYStart + tOsdImgInfo.uwVSize);
			tOsdImgInfo.uwYStart += uwLcd_VSize/2;
			tOSD_Img2(&tOsdImgInfo, update_type);
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_DrawTxVersion(void)
{
	OSD_IMG_INFO tOsdImgInfo;
	osSemaphoreWait(osUI_CuUiCtr, osWaitForever);
	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_SUBMENU2, 1, &tOsdImgInfo);
	tOSD_Img1(&tOsdImgInfo, OSD_QUEUE);
	if(!ubUI_TouchPanelSts)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_RETURN_BLACK, 1, &tOsdImgInfo);
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	}
	for(UI_CamNum_t tCamNum = CAM1;tCamNum < CAM_4T;tCamNum ++)
	{
		OSD_ImagePrintf(OSD_IMG_ROTATION_0, 350, 200 + tCamNum*50, tUI_CharOsdImgInfo, OSD_UPDATE, (tCamNum == CAM4)?"CAM4:":(tCamNum == CAM3)?"CAM3:":(tCamNum == CAM2)?"CAM2:":"CAM1:");
		if(tUI_CamStatus[tCamNum].ulCAM_ID != INVALID_ID)
			OSD_ImagePrintf(OSD_IMG_ROTATION_0, 430, 200 + tCamNum*50, tUI_CharOsdImgInfo, OSD_UPDATE, tUI_CamStatus[tCamNum].cCamVersion);
	}
	tUI_State = UI_SHOW_TX_VERSION_STATE;
	osSemaphoreRelease(osUI_CuUiCtr);
}

void UI_UART2_PutChar(char ch)
{
	while(!UART2->TX_RDY);
	UART2->RS_DATA = ch;
//	ch = ch;
}
void UI_DrawCarFlag(uint8_t chn,OSD_IMG_INFO *tImgInfor)
{
	uint8_t View = 0;		
	//tImgInfor.uwXStart += 0;
	//tImgInfor.uwYStart += 260;
	
	if (DeskTopShowView == DUALVIEW_ITEM)
	{	
		View = 1;
		//tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_Car_DETECUED_FLAG, 1, &tImgInfor);
		tImgInfor[View].uwXStart = 550;
		tImgInfor[View].uwYStart = 10;	
		if (tUI_CamStatus[chn].tCamDispLocation_Dual == DISP_LEFT)
		{
			tImgInfor[View].uwXStart -= 1024/4;
		}
        else if (tUI_CamStatus[chn].tCamDispLocation_Dual == DISP_RIGHT)
		{
			tImgInfor[View].uwXStart += 1024/4;
		}
		else
		{
			return;
		}
	}
	else if (DeskTopShowView == QUALVIEW_ITEM)
	{
		View = 1;
		//tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_Car_DETECUED_FLAG_HALF, 1, &tImgInfor);
		tImgInfor[View].uwXStart = 500;
		tImgInfor[View].uwYStart = 10;	
		if (tUI_CamStatus[chn].tCamDispLocation_Quad == DISP_UPPER_LEFT)
		{
			tImgInfor[View].uwXStart -= 1024/4;
		}
        else if (tUI_CamStatus[chn].tCamDispLocation_Quad == DISP_UPPER_RIGHT)
		{
			tImgInfor[View].uwXStart += 1024/4;
		}
		else if (tUI_CamStatus[chn].tCamDispLocation_Quad == DISP_LOWER_LEFT)
		{
			tImgInfor[View].uwXStart -= 1024/4;
			tImgInfor[View].uwYStart += 600/2;
		}
        else if (tUI_CamStatus[chn].tCamDispLocation_Quad == DISP_LOWER_RIGHT)
		{
			tImgInfor[View].uwXStart += 1024/4;
			tImgInfor[View].uwYStart += 600/2;
		}
		else
		{
			return;
		}
	}
	else if(DeskTopShowView <= DUALVIEW_ITEM)
	{
		View = 0;
		//tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_Car_DETECUED_FLAG, 1, &tImgInfor);
		tImgInfor[View].uwXStart = 550;
		tImgInfor[View].uwYStart = 10;	
	}

	tOSD_Img2(&tImgInfor[View], OSD_UPDATE);
}
void UI_ClearCarFlag(uint8_t chn)
{
	OSD_IMG_INFO tImgInfor;
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_Car_DETECUED_FLAG, 1, &tImgInfor);
	tImgInfor.uwXStart = 550;
	tImgInfor.uwYStart = 10;	


	if (DeskTopShowView == DUALVIEW_ITEM)
	{
		tImgInfor.uwXStart = 550;
		if (tUI_CamStatus[chn].tCamDispLocation_Dual == DISP_LEFT)
		{
			tImgInfor.uwXStart -= 1024/4;
		}
        else if (tUI_CamStatus[chn].tCamDispLocation_Dual == DISP_RIGHT)
		{
			tImgInfor.uwXStart += 1024/4;
		}
		else
		{
			return;
		}
	}
	else if (DeskTopShowView == QUALVIEW_ITEM)
	{
		tImgInfor.uwXStart = 500;
		if (tUI_CamStatus[chn].tCamDispLocation_Quad == DISP_UPPER_LEFT)
		{
			tImgInfor.uwXStart -= 1024/4;
		}
        else if (tUI_CamStatus[chn].tCamDispLocation_Quad == DISP_UPPER_RIGHT)
		{
			tImgInfor.uwXStart += 1024/4;
		}
		else if (tUI_CamStatus[chn].tCamDispLocation_Quad == DISP_LOWER_LEFT)
		{
			tImgInfor.uwXStart -= 1024/4;
			tImgInfor.uwYStart += 600/2;
		}
        else if (tUI_CamStatus[chn].tCamDispLocation_Quad == DISP_LOWER_RIGHT)
		{
			tImgInfor.uwXStart += 1024/4;
			tImgInfor.uwYStart += 600/2;
		}
		else
		{
			return;
		}
	}

	// 车辆和栈板共用图标位置，两组资源均为 50x50 / 25x25。
	OSD_EraserImg2_NoUpdate(&tImgInfor);
}


void UI_DrawPersonFlag(uint8_t chn,OSD_IMG_INFO *tImgInfor)
{
	
	uint8_t View;
	//tImgInfor.uwXStart += 0;
	//tImgInfor.uwYStart += 260;

	if (DeskTopShowView == DUALVIEW_ITEM)
	{	
		View = 1;
		//tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PERSON_DETECUED_FLAG, 1, &tImgInfor);
		tImgInfor[View].uwXStart = 450;
		tImgInfor[View].uwYStart = 10;	
		if (tUI_CamStatus[chn].tCamDispLocation_Dual == DISP_LEFT)
		{
			tImgInfor[View].uwXStart -= 1024/4;
		}
        else if (tUI_CamStatus[chn].tCamDispLocation_Dual == DISP_RIGHT)
		{
			tImgInfor[View].uwXStart += 1024/4;
		}
		else
		{
			return;
		}
	}
	else if (DeskTopShowView == QUALVIEW_ITEM)
	{
		View = 1;
		//tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PERSON_DETECUED_FLAG_HALF, 1, &tImgInfor);
		tImgInfor[View].uwXStart = 450;
		tImgInfor[View].uwYStart = 10;	
		if (tUI_CamStatus[chn].tCamDispLocation_Quad == DISP_UPPER_LEFT)
		{
			tImgInfor[View].uwXStart -= 1024/4;
		}
        else if (tUI_CamStatus[chn].tCamDispLocation_Quad == DISP_UPPER_RIGHT)
		{
			tImgInfor[View].uwXStart += 1024/4;
		}
		else if (tUI_CamStatus[chn].tCamDispLocation_Quad == DISP_LOWER_LEFT)
		{
			tImgInfor[View].uwXStart -= 1024/4;
			tImgInfor[View].uwYStart += 600/2;
		}
        else if (tUI_CamStatus[chn].tCamDispLocation_Quad == DISP_LOWER_RIGHT)
		{
			tImgInfor[View].uwXStart += 1024/4;
			tImgInfor[View].uwYStart += 600/2;
		}
		else
		{
			return;
		}
	}
	else if(DeskTopShowView <= DUALVIEW_ITEM)
	{
		View = 0;
		//tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PERSON_DETECUED_FLAG, 1, &tImgInfor);
		tImgInfor[View].uwXStart = 450;
		tImgInfor[View].uwYStart = 10;	
	}

	tOSD_Img2(&tImgInfor[View], OSD_UPDATE);
}

void UI_ClearPersonFlag(uint8_t chn)
{
	OSD_IMG_INFO tImgInfor;
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PERSON_DETECUED_FLAG, 1, &tImgInfor);
	tImgInfor.uwXStart = 450;
	tImgInfor.uwYStart = 10;	


	if (DeskTopShowView == DUALVIEW_ITEM)
	{
		if (tUI_CamStatus[chn].tCamDispLocation_Dual == DISP_LEFT)
		{
			tImgInfor.uwXStart -= 1024/4;
		}
        else if (tUI_CamStatus[chn].tCamDispLocation_Dual == DISP_RIGHT)
		{
			tImgInfor.uwXStart += 1024/4;
		}
		else
		{
			return;
		}
	}
	else if (DeskTopShowView == QUALVIEW_ITEM)
	{
		if (tUI_CamStatus[chn].tCamDispLocation_Quad == DISP_UPPER_LEFT)
		{
			tImgInfor.uwXStart -= 1024/4;
		}
        else if (tUI_CamStatus[chn].tCamDispLocation_Quad == DISP_UPPER_RIGHT)
		{
			tImgInfor.uwXStart += 1024/4;
		}
		else if (tUI_CamStatus[chn].tCamDispLocation_Quad == DISP_LOWER_LEFT)
		{
			tImgInfor.uwXStart -= 1024/4;
			tImgInfor.uwYStart += 600/2;
		}
        else if (tUI_CamStatus[chn].tCamDispLocation_Quad == DISP_LOWER_RIGHT)
		{
			tImgInfor.uwXStart += 1024/4;
			tImgInfor.uwYStart += 600/2;
		}
		else
		{
			return;
		}
	}

	OSD_EraserImg2_NoUpdate(&tImgInfor);
}

//static uint8_t Stop_WavchnFlag[4]={1,1,1,1};
//-------------------------------------------------------------------------------------------------------
extern uint8_t ul_drawboxflag;
void UI_PlayWav(uint8_t index,uint8_t chn)
{
//WAV
#if 0
	if(tUI_CuSetting.ubIsEnableBSDALARM == 1)
	{	
		Stop_WavchnFlag[chn] = 0;
		if(DeskTopShowView == QUALVIEW_ITEM)
		{
			if(ISPlaying_wav == 0)
			{
				ADO_WavRepeat(index);
				ISPlaying_wav = 1;
				//printf("play wav!!!\n");
			}
		} 
		else if(DeskTopShowView == DUALVIEW_ITEM)
		{
			if((chn == tCamViewSel.tCamViewPool[0] || chn == tCamViewSel.tCamViewPool[1])&& ISPlaying_wav == 0)
			{		
				ADO_WavRepeat(index);
				ISPlaying_wav = 1;
				//printf("play wav!!!\n");
			}
		}
		else
		{
			if(chn == tCamViewSel.tCamViewPool[0]&&ISPlaying_wav == 0)
			{
				ADO_WavRepeat(index);
				ISPlaying_wav = 1;
				//printf("play wav!!!\n");
			}	
		}	
	}
#endif
}
void UI_StopWav(uint8_t index,uint8_t chn)
{
#if 0
	uint8_t WavDual_ChnTemp0,WavDual_ChnTemp1;
	Stop_WavchnFlag[chn] = 1;	
	if(DeskTopShowView == QUALVIEW_ITEM )//&& Stop_WavchnFlag[0] && Stop_WavchnFlag [1] && Stop_WavchnFlag[2] && Stop_WavchnFlag[3])
	{
		printf("qual stop wav:%d,%d,%d,%d!!!\n",Stop_WavchnFlag[0],Stop_WavchnFlag[1],Stop_WavchnFlag[2],Stop_WavchnFlag[3]);
		if(Stop_WavchnFlag[0] && Stop_WavchnFlag [1] && Stop_WavchnFlag[2] && Stop_WavchnFlag[3] &&ISPlaying_wav == 1)//四路没有检测到目标的时�?
		{
			ADO_WavStop();
			ISPlaying_wav = 0;
		}

	}
	else if (DeskTopShowView == DUALVIEW_ITEM)
	{
		WavDual_ChnTemp0 = tCamViewSel.tCamViewPool[0];
		WavDual_ChnTemp1 = tCamViewSel.tCamViewPool[1];
		if(Stop_WavchnFlag[WavDual_ChnTemp0] && Stop_WavchnFlag [WavDual_ChnTemp1])
		{
			ADO_WavStop();
			ISPlaying_wav = 0;
			printf("stop wav!!!\n");
		}

	}
	else 
	{
		if(ISPlaying_wav == 1)
		{
			ADO_WavStop();
			ISPlaying_wav = 0;
		}
	}


#endif
}
//-------------------------------------------------------------------------------------------------------

#define READ_RECT_LINE_LENGTH 12
#define AI_BOX_SPLIT_LINE_WIDTH 4

// 单画面、双画面和四画面的有效区域，分屏时保留中间 4 像素分割线。
static uint8_t UI_GetAIBoxArea(uint32_t chn, OSD_IMG_INFO *tArea)
{
	tArea->uwXStart = 0;
	tArea->uwYStart = 0;
	tArea->uwHSize = 1024;
	tArea->uwVSize = 600;
	if(DeskTopShowView < DUALVIEW_ITEM)
		return chn == tCamViewSel.tCamViewPool[0];
	if(DeskTopShowView == DUALVIEW_ITEM)
	{
		tArea->uwHSize = 512 - AI_BOX_SPLIT_LINE_WIDTH / 2;
		if(tUI_CamStatus[chn].tCamDispLocation_Dual == DISP_RIGHT)
			tArea->uwXStart = 512 + AI_BOX_SPLIT_LINE_WIDTH / 2;
		else if(tUI_CamStatus[chn].tCamDispLocation_Dual != DISP_LEFT)
			return FALSE;
	}
	else if(DeskTopShowView == QUALVIEW_ITEM)
	{
		tArea->uwHSize = 512 - AI_BOX_SPLIT_LINE_WIDTH / 2;
		tArea->uwVSize = 300 - AI_BOX_SPLIT_LINE_WIDTH / 2;
		switch(tUI_CamStatus[chn].tCamDispLocation_Quad)
		{
			case DISP_UPPER_LEFT:
				break;
			case DISP_UPPER_RIGHT:
				tArea->uwXStart = 512 + AI_BOX_SPLIT_LINE_WIDTH / 2;
				break;
			case DISP_LOWER_LEFT:
				tArea->uwYStart = 300 + AI_BOX_SPLIT_LINE_WIDTH / 2;
				break;
			case DISP_LOWER_RIGHT:
				tArea->uwXStart = 512 + AI_BOX_SPLIT_LINE_WIDTH / 2;
				tArea->uwYStart = 300 + AI_BOX_SPLIT_LINE_WIDTH / 2;
				break;
			default:
				return FALSE;
		}
	}
	else
		return FALSE;
	return TRUE;
}

// 过小的目标扩大 12 像素；靠近边缘时平移回本通道，超大框再裁到有效区域。
static void UI_FitAIBoxToSafeArea(Algo_Pos *pos, OSD_IMG_INFO *tArea)
{
	int32_t xMin = tArea->uwXStart;
	int32_t yMin = tArea->uwYStart;
	int32_t xMax = xMin + tArea->uwHSize - 1;
	int32_t yMax = yMin + tArea->uwVSize - 1;
	int32_t x1 = pos->x1, y1 = pos->y1, x2 = pos->x2, y2 = pos->y2;
	int32_t offset;

	if(x2 - x1 < READ_RECT_LINE_LENGTH)
	{
		if(xMax - x2 < READ_RECT_LINE_LENGTH)
			x1 -= READ_RECT_LINE_LENGTH;
		else
			x2 += READ_RECT_LINE_LENGTH;
	}
	if(y2 - y1 < READ_RECT_LINE_LENGTH)
	{
		if(yMax - y2 < READ_RECT_LINE_LENGTH)
			y1 -= READ_RECT_LINE_LENGTH;
		else
			y2 += READ_RECT_LINE_LENGTH;
	}
	if(x1 < xMin)
	{
		offset = xMin - x1;
		x1 += offset;
		x2 += offset;
	}
	if(x2 > xMax)
	{
		offset = x2 - xMax;
		x1 -= offset;
		x2 -= offset;
	}
	if(y1 < yMin)
	{
		offset = yMin - y1;
		y1 += offset;
		y2 += offset;
	}
	if(y2 > yMax)
	{
		offset = y2 - yMax;
		y1 -= offset;
		y2 -= offset;
	}
	if(x1 < xMin) x1 = xMin;
	if(y1 < yMin) y1 = yMin;
	pos->x1 = (uint16_t)x1;
	pos->y1 = (uint16_t)y1;
	pos->x2 = (uint16_t)x2;
	pos->y2 = (uint16_t)y2;
}

// 清框和重画共用同一范围，连同画面边缘一起清除，避免目标移动后留下边框。
// 顶部信号、录像和 SD 卡区域仍由各自的状态刷新函数维护。
static void UI_ClearAIBoxArea(OSD_IMG_INFO *tArea)
{
	OSD_IMG_INFO tEraserOsdImgInfo = *tArea;
	uint16_t topHeight = (tArea->uwXStart == 0 && tArea->uwYStart == 0)?80:50;
	uint16_t rightWidth = (tArea->uwXStart + tArea->uwHSize == 1024 && tArea->uwYStart == 0)?52:0;

	// 顶部图标之间的空白区域。
	tEraserOsdImgInfo.uwXStart = tArea->uwXStart + 110;
	tEraserOsdImgInfo.uwHSize = tArea->uwHSize - 110 - rightWidth;
	tEraserOsdImgInfo.uwVSize = topHeight;
	OSD_EraserImg2_NoUpdate(&tEraserOsdImgInfo);

	// 图标以下的整个通道区域。
	tEraserOsdImgInfo = *tArea;
	tEraserOsdImgInfo.uwYStart += topHeight;
	tEraserOsdImgInfo.uwVSize -= topHeight;
	OSD_EraserImg2_NoUpdate(&tEraserOsdImgInfo);

	// 顶部及两侧贴边的检测框也必须清除，不能只清画面内部。
	tEraserOsdImgInfo = *tArea;
	tEraserOsdImgInfo.uwVSize = 4;
	OSD_EraserImg2_NoUpdate(&tEraserOsdImgInfo);
	tEraserOsdImgInfo.uwYStart += 4;
	tEraserOsdImgInfo.uwHSize = 4;
	tEraserOsdImgInfo.uwVSize = topHeight - 4;
	OSD_EraserImg2_NoUpdate(&tEraserOsdImgInfo);
	tEraserOsdImgInfo.uwXStart = tArea->uwXStart + tArea->uwHSize - 4;
	OSD_EraserImg2_NoUpdate(&tEraserOsdImgInfo);
}

void UI_EventClearBox(uint32_t ClearBOX_chn)
{
	OSD_IMG_INFO tArea;
	if(ClearBOX_chn >= 4)
		return;
	osMutexWait(osEnterVolumeFlag, osWaitForever);
	if(MenuOnFlag == TRUE || !UI_GetAIBoxArea(ClearBOX_chn, &tArea))
	{
		osMutexRelease(osEnterVolumeFlag);
		return;
	}
	UI_ClearPersonFlag(ClearBOX_chn);
	UI_ClearCarFlag(ClearBOX_chn);
	UI_ClearAIBoxArea(&tArea);
	osMutexRelease(osEnterVolumeFlag);
}

//-------------------------------------------------------------------------------------------------------
void UI_EventDrawBox(Algo_Result showBox)
{
	OSD_IMG_INFO tArea, tBox;
	static uint8_t LoadJpegFlag = 0;
	static OSD_IMG_INFO tPD_OsdImagInfo[2],tCD_OsdImagInfo[2],tPallet_OsdImagInfo[2];

	if(showBox.chn >= 4)
		return;
	osMutexWait(osEnterVolumeFlag, osWaitForever);
	if(MenuOnFlag == TRUE || isDrawNosignal[showBox.chn] == 1 || ubUI_FinishViewSwitch == 0 ||
		tUI_State == UI_SET_VOLUME_STATE || !UI_GetAIBoxArea(showBox.chn, &tArea))
	{
		osMutexRelease(osEnterVolumeFlag);
		return;
	}
	if(LoadJpegFlag == 0)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PERSON_DETECUED_FLAG, 2, &tPD_OsdImagInfo[0]);
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_Car_DETECUED_FLAG, 2, &tCD_OsdImagInfo[0]);
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PALLET_DETECTED_FLAG, 2, &tPallet_OsdImagInfo[0]);
		LoadJpegFlag = 1;
	}

	Drawing_BoxFlag[showBox.chn] = 1;
	UI_ClearAIBoxArea(&tArea);
	if(ubDrawBSDRange == 1)
	{
		for(uint8_t i = 0; i < showBox.cnt && i < DATA_LENGTH_MAX; i++)
		{
			// 跳过串口中的反向坐标，正常小目标仍会扩大后显示。
			if(showBox.pos[i].x2 < showBox.pos[i].x1 || showBox.pos[i].y2 < showBox.pos[i].y1)
				continue;
			UI_FitAIBoxToSafeArea(&showBox.pos[i], &tArea);
			tBox.uwXStart = showBox.pos[i].x1;
			tBox.uwYStart = showBox.pos[i].y1;
			tBox.uwHSize = showBox.pos[i].x2 - showBox.pos[i].x1 + 1;
			tBox.uwVSize = showBox.pos[i].y2 - showBox.pos[i].y1 + 1;
			tOSD_Img2_DrawBox(&tBox, showBox.pos[i].alarm_type, OSD_QUEUE);
		}
	}
	Drawing_BoxFlag[showBox.chn] = 0;

	// 每帧先清除旧标志，避免目标类别改变后留下上一次图标。
	UI_ClearPersonFlag(showBox.chn);
	UI_ClearCarFlag(showBox.chn);
	if(showBox.P_OR_C & 0x01)
		UI_DrawPersonFlag(showBox.chn, tPD_OsdImagInfo);
	if(showBox.P_OR_C & 0x04)
		UI_DrawCarFlag(showBox.chn, tPallet_OsdImagInfo);
	else if(showBox.P_OR_C & 0x02)
		UI_DrawCarFlag(showBox.chn, tCD_OsdImagInfo);

	osMutexRelease(osEnterVolumeFlag);
}
//------------------------------------------------------------------------------
void UI_WakeUpTx(void)
{
//	APP_EventMsg_t tUI_Message = {0};
//	tUI_Message.ubAPP_Event 	 = APP_WAKEUPTX_EVENT;
//	tUI_Message.ubAPP_Message[0] = 1;		
//	uint8_t ubRole = 0;
//	
//	if(tCamViewSel.tCamViewType == SINGLE_VIEW)
//		ubRole = 1;
//	else if(tCamViewSel.tCamViewType == DUAL_VIEW)
//		ubRole = 2;
//	else
//		ubRole = 4;
//	
//	for(uint8_t i= 0;i < ubRole;i++)
//	{
//		if(tUI_CamStatus[tCamViewSel.tCamViewPool[i]].tCamConnSts == CAM_OFFLINE && tUI_CamStatus[tCamViewSel.tCamViewPool[i]].ulCAM_ID != INVALID_ID)
//		{
//			tUI_Message.ubAPP_Message[1] = tCamViewSel.tCamViewPool[i];
//			UI_SendMessageToAPP(&tUI_Message);
//			osDelay(20);
//		}
//	}	

}
//------------------------------------------------------------------------------
void UI_SleepTx(void)
{
	/*APP_EventMsg_t tUI_Message = {0};
	tUI_Message.ubAPP_Event 	 = APP_SLEEPTX_EVENT;
	tUI_Message.ubAPP_Message[0] = 1;		
	
	uint8_t ubRole = 0; 

	if(tCamViewSel.tCamViewType == SINGLE_VIEW)
		ubRole = 1;
	else if(tCamViewSel.tCamViewType == DUAL_VIEW)
		ubRole = 2;
	else
		ubRole = 4;
	
	for(uint8_t i= 0;i < ubRole;i++)
	{
		if(tUI_CamStatus[tCamViewSel.tCamViewPool[i]].tCamConnSts == CAM_ONLINE && tUI_CamStatus[tCamViewSel.tCamViewPool[i]].tCamBatLvl != BAT_NO)
		{
			tUI_Message.ubAPP_Message[1] = tCamViewSel.tCamViewPool[i];
			UI_SendMessageToAPP(&tUI_Message);
			osDelay(20);
		}
	}	*/
}
//------------------------------------------------------------------------------
void UI_DisconnectTx(void)
{
	APP_EventMsg_t tUI_Message = {0};
	tUI_Message.ubAPP_Event 	 = APP_DISCONNECT_EVENT;
	tUI_Message.ubAPP_Message[0] = 1;	
	UI_SendMessageToAPP(&tUI_Message);
}
//------------------------------------------------------------------------------
void UI_RefreshBandwidth(void)
{
	APP_EventMsg_t tUI_Message = {0};
	tUI_Message.ubAPP_Event 	 = APP_SETBANDWIDTH_EVENT;
	tUI_Message.ubAPP_Message[0] = 5;		//! Message Length	
	if(tCamViewSel.tCamViewType == SINGLE_VIEW)
		tUI_Message.ubAPP_Message[1] = 1;
	else if(tCamViewSel.tCamViewType == DUAL_VIEW)
		tUI_Message.ubAPP_Message[1] = 2;
	else
		tUI_Message.ubAPP_Message[1] = 4;
	
	tUI_Message.ubAPP_Message[2] = tCamViewSel.tCamViewPool[0];
	tUI_Message.ubAPP_Message[3] = tCamViewSel.tCamViewPool[1];
	tUI_Message.ubAPP_Message[4] = tCamViewSel.tCamViewPool[2];
	tUI_Message.ubAPP_Message[5] = tCamViewSel.tCamViewPool[3];
	printf("tCamViewSel.tCamViewPool[0] = %d,tCamViewSel.tCamViewPool[1] = %d,tCamViewSel.tCamViewPool[2] = %d,tCamViewSel.tCamViewPool[3] = %d\n",
		tCamViewSel.tCamViewPool[0],tCamViewSel.tCamViewPool[1],tCamViewSel.tCamViewPool[2],tCamViewSel.tCamViewPool[3]);
	UI_SendMessageToAPP(&tUI_Message);
}

//------------------------------------------------------------------------------
void UI_SetTxType(UI_CamNum_t tCamNum, uint16_t uwVal)
{
	osMutexWait(osUI_TxTypeMutex, osWaitForever);
	uwTxType[tCamNum] = uwVal;
	osMutexRelease(osUI_TxTypeMutex);
}
//------------------------------------------------------------------------------
uint16_t uwUI_GetTxType(UI_CamNum_t tCamNum)
{
	uint16_t uwVal;
	osMutexWait(osUI_TxTypeMutex, osWaitForever);
	uwVal = uwTxType[tCamNum];
	osMutexRelease(osUI_TxTypeMutex);
	return uwVal;
}
//------------------------------------------------------------------------------
void UI_RecvTxType (TWC_TAG tRecv_StaNum, uint8_t *pData)
{
	if(pData[0] != tRecv_StaNum)
		return;
	uint16_t uwVal;
	uwVal = (pData[1] << 8) + pData[2];
	UI_SetTxType((UI_CamNum_t)tRecv_StaNum,uwVal);
}
//------------------------------------------------------------------
uint8_t UI_UpdateTxImgProc(UI_CamNum_t tCamNum)
{
	printf("UI_UpdateTxImgProc  tCamNum = %d\n",tCamNum);
	uint8_t ubValue,ubResult = 0;
	UI_CUReqCmd_t tCamSetCmd;
	tCamSetCmd.tDS_CamNum				= tCamNum;
	tCamSetCmd.ubCmd[UI_TWC_TYPE]		= UI_SETTING;
	tCamSetCmd.ubCmd[UI_SETTING_ITEM]	= UI_IMGPROC_SETTING;
	tCamSetCmd.ubCmd_Len				 = 4;

		
	tUI_CuSetting.bColorBLGain[tCamNum] = 10;
	tUI_CuSetting.bColorContrastGain[tCamNum] = -15;
	tUI_CuSetting.bColorSaturationGain[tCamNum] = -10;
	tUI_CuSetting.bColorHueGain[tCamNum] = 10;
	
	ubValue = UI_Map(tUI_CamStatus[tCamNum].tCamColorParam.ubColorBL + tUI_CuSetting.bColorBLGain[tCamNum],0,99,0,127);
	tCamSetCmd.ubCmd[UI_SETTING_DATA]	 = UI_IMGBL_SETTING ;
	tCamSetCmd.ubCmd[UI_SETTING_DATA+1] = ubValue;

	if(UI_SendRequestToCAM(osThreadGetId(), &tCamSetCmd) != rUI_SUCCESS)
		return 0;


	ubValue = UI_Map(tUI_CamStatus[tCamNum].tCamColorParam.ubColorContrast + tUI_CuSetting.bColorContrastGain[tCamNum],0,99,0,127);
	tCamSetCmd.ubCmd[UI_SETTING_DATA]	 = UI_IMGCONTRAST_SETTING ;
	tCamSetCmd.ubCmd[UI_SETTING_DATA+1] = ubValue;

	if(UI_SendRequestToCAM(osThreadGetId(), &tCamSetCmd) != rUI_SUCCESS)
		return 0;

	ubValue = UI_Map(tUI_CamStatus[tCamNum].tCamColorParam.ubColorSaturation + tUI_CuSetting.bColorSaturationGain[tCamNum],0,99,0,127);
	tCamSetCmd.ubCmd[UI_SETTING_DATA]	 = UI_IMGSATURATION_SETTING ;
	tCamSetCmd.ubCmd[UI_SETTING_DATA+1] = ubValue;

	if(UI_SendRequestToCAM(osThreadGetId(), &tCamSetCmd) != rUI_SUCCESS)
		return 0;
	
	ubValue = UI_Map(tUI_CamStatus[tCamNum].tCamColorParam.ubColorHue + tUI_CuSetting.bColorHueGain[tCamNum],0,99,0,127);
	tCamSetCmd.ubCmd[UI_SETTING_DATA]	 = UI_IMGHUE_SETTING ;
	tCamSetCmd.ubCmd[UI_SETTING_DATA+1] = ubValue;

	if(UI_SendRequestToCAM(osThreadGetId(), &tCamSetCmd) != rUI_SUCCESS)
		return 0;
	

	tCamSetCmd.ubCmd[UI_SETTING_ITEM]   = UI_SAVEPARAMETERS;
	tCamSetCmd.ubCmd_Len				= 2;

	if(UI_SendRequestToCAM(osThreadGetId(), &tCamSetCmd) != rUI_SUCCESS)
		return 0;

	return 1;
	
}
//------------------------------------------------------------------
void UI_CheckTxType(void)//�����ǰ�ԽӵĲ���?79����������ͼ�����?
{
	uint8_t ubRole = 0;
	UI_CamNum_t tCamNum;
	if(tCamViewSel.tCamViewType == SINGLE_VIEW)
		ubRole = 1;
	else if(tCamViewSel.tCamViewType == DUAL_VIEW)
		ubRole = 2;
	else
		ubRole = 4;
	for(uint8_t i = 0;i < ubRole;i ++)
	{
		tCamNum = tCamViewSel.tCamViewPool[i];
		
		if(tUI_CuSetting.ubUpdateTxParam[tCamNum])
			continue;
		
		if(tUI_CamStatus[tCamNum].tCamConnSts == CAM_ONLINE)
		{
			if(uwUI_GetTxType(tCamNum) != 479)
				ubTxTypeCount[tCamNum] ++;
			else
			{
				tUI_CuSetting.ubUpdateTxParam[tCamNum] = TRUE;
				UI_UpdateDevStatusInfo();
			}
			
			if(ubTxTypeCount[tCamNum] == 5)
			{
				if(UI_UpdateTxImgProc(tCamNum))
				{
					tUI_CuSetting.ubUpdateTxParam[tCamNum] = TRUE;
					UI_UpdateDevStatusInfo();
				}
				else
					ubTxTypeCount[tCamNum] = 0;
			}
		}
		else
		{	
			ubTxTypeCount[tCamNum] = 0;
		}
	}	

}

//------------------------------------------------------------------
void UI_CheckAIVersion(void)
{
//	printf("KNL_UsbdFwuFg = %d receivedVersion = %d\n", KNL_UsbdFwuFg, strlen(receivedVersion));
	static uint8_t count = 0;
	if(count % 5 == 0)
	{
		if(!KNL_UsbdFwuFg && strlen(receivedVersion) == 0)
		{
			printf("UI_CheckAIVersion\n");
			osMutexWait(osAIConfigMutex, osWaitForever);
			UI_UART2_PutChar(0XFF);
			UI_UART2_PutChar(0XDD);
			osMutexRelease(osAIConfigMutex);
		}
	}
	count++;
}

//------------------------------------------------------------------------------
// 各通道独立保存报警等级，取最高等级播放，空帧只清除本通道。
static void UI_UpdateAIAlarm(void)
{
	uint8_t i, ubLevel = 0;
	for(i = 0; i < CAM_4T; i++)
	{
		if(uwAIAlarmTimeout[i] && tUI_CamStatus[i].tCamConnSts == CAM_ONLINE &&
			(tUI_CamStatus[i].ubAIAlgorithm == AI_ALGORITHM_PALLET ||
			(tUI_CuSetting.ubIsEnableBSDALARM[i] &&
			(tUI_CuSetting.ubDetectPeopleFlag[i] || tUI_CuSetting.ubDetectCarFlag[i]))) &&
			ubAIAlarmLevel[i] > ubLevel)
			ubLevel = ubAIAlarmLevel[i];
	}
	if(MenuOnFlag || ubUI_CuPowerDiscFlag || ubUI_CuStandbyFlag || KNL_UsbdFwuFg || ubAIConfigSync)
		ubLevel = 0;
	if(ubLevel == 0)
	{
		if(Playwav_Flag)
			ADO_WavStop();
		Playwav_Flag = 0;
		return;
	}
	if(!Playwav_Flag || Playwav_Switch != ubLevel - 1)
		Playwav_Count = PLAYWAV_COUNT;
	Playwav_Switch = ubLevel - 1;
	Playwav_Flag = 1;
	if(Playwav_Count >= PLAYWAV_COUNT && tADO_GetWavState() == ADO_WAV_IDLE)
	{
		ADO_WavPlay(Playwav_Switch);
		Playwav_Count = 0;
	}
}

//------------------------------------------------------------------------------
// 独立灯控任务检查最新状态；摄像头命令通过共用互斥锁与菜单命令串行。
void UI_UpdateAILamp(void)
{
	uint8_t i;
	UI_CUReqCmd_t tCamSetCmd;
	if(!ubUI_CuStartUpFlag)
		return;
	tCamSetCmd.ubCmd[UI_TWC_TYPE] = UI_SETTING;
	tCamSetCmd.ubCmd[UI_SETTING_ITEM] = UI_IMGPROC_SETTING;
	tCamSetCmd.ubCmd_Len = 4;
	for(i = 0; i < CAM_4T; i++)
	{
		// 切回 BSD、待机或算法重新同步时，撤销该路临时开灯请求。
		if(tUI_CamStatus[i].ubAIAlgorithm != AI_ALGORITHM_PALLET ||
			tUI_CamStatus[i].tCamConnSts != CAM_ONLINE || ubAIConfigSync ||
			ubUI_CuPowerDiscFlag || ubUI_CuStandbyFlag || KNL_UsbdFwuFg)
			uwAILampTimeout[i] = 0;
		if(tUI_CamStatus[i].tCamConnSts != CAM_ONLINE ||
			!UI_CheckTxVersion(tUI_CamStatus[i].cCamVersion,30,Laserchar,3))
			continue;
		tCamSetCmd.tDS_CamNum = (UI_CamNum_t)i;
		// 发送函数取得共用互斥锁后，重新计算最新开关状态并更新已确认状态。
		if(UI_SendLaserorLedToCAM(osThreadGetId(), &tCamSetCmd, TWC_Laser_CTRL) != rUI_SUCCESS)
			printf("AI LASER Setting Fail: chn=%u\n", i);
		if(UI_SendLaserorLedToCAM(osThreadGetId(), &tCamSetCmd, TWC_Led_CTRL) != rUI_SUCCESS)
			printf("AI LED Setting Fail: chn=%u\n", i);
	}
}

void UI_UpdateStatus(uint16_t *pThreadCnt)
{
	if(!ubUI_CuStartUpFlag)
		return;
	APP_EventMsg_t tUI_GetLinkStsMsg = {0};
	uint8_t ubUI_SendMsg2AppFlag = FALSE;
	OSD_IMG_INFO tEraserOsdImgInfo;
	//UI_PwrCtrl();
	
	//printf("UI_UpdateStatus wait osUI_CuUiCtr\n");
	osSemaphoreWait(osUI_CuUiCtr, osWaitForever);
	//printf("UI_UpdateStatus get osUI_CuUiCtr\n");
	if((tUI_State == UI_DISPLAY_STATE || tUI_State == UI_MAINMENU_STATE || tUI_State == UI_SET_VOLUME_STATE) && ubUI_CuPowerDiscFlag == FALSE)
	//if((tUI_State == UI_DISPLAY_STATE) && ubUI_CuPowerDiscFlag == FALSE)// && (tUI_State != UI_SET_VOLUME_STATE && tUI_CuSetting.ubIsShowBSDBox == 1))//&& KNL_UsbdFwuFg == 0)
	{

		if((*pThreadCnt % UI_UPDATESTS_PERIOD) == 0)
		{
				UI_ShowRecordingStatus(FALSE);
				UI_DrawSignalInfoIcon(OSD_QUEUE);
				if(tUI_State == UI_DISPLAY_STATE)
					UI_DrawNoSignalIcon();
				UI_DrawTxBatteryInfoIcon();
				
				UI_DrawSdCardInfoIcon();
		}	
		
		if((*pThreadCnt % UI_RECORDINGSTS_PERIOD) == 0)
		{
			UI_ShowRecordingStatus(TRUE);
		}
	}
//	if(Stop_WavchnFlag[0]&&Stop_WavchnFlag[1]&&Stop_WavchnFlag[2]&&Stop_WavchnFlag[3])
//	{
//		ISPlaying_wav = 0;
//		ADO_WavStop();
//	}
	UI_UpdateAIAlarm();
	if(ubAIConfigSync && !uwAIConfigSyncCount && !KNL_UsbdFwuFg)
		UI_SendAIConfigTo1126();
	UI_UpdateRecStsIcon();
	UI_UpdateWarningNoteIcon();
	//paly time
	if(tUI_MenuItem.ubItemIdx == PLAYBACK_ITEM && tUI_State == UI_SUBSUBSUBMENU_STATE)
	{
		UI_ShowPlayTime(ulPLY_GetSchedule(),ulPLY_GetTotalPlayTime(),830,515);
	}
	osSemaphoreRelease(osUI_CuUiCtr);
		
	//�Զ�����
	if((!ubUI_CuStandbyFlag || (ubUI_CuStandbyFlag && TriggerLock)) && !ubUI_CuPowerDiscFlag && ubUI_FinishViewSwitch)//防止低背光切换通道拉起来屏�?
		UI_CheckLightSensor(FALSE);

//	UI_CheckPwrUartCount();
	UI_CheckMenuOffCount();//�Զ��˳��˵�
	UI_CheckScanModeCount();//�Զ�ɨ��
	UI_CheckTriggerOverCount();
	UI_CheckTxType();
	UI_CheckAIVersion();
#if 0
	UI_CheckPanelOffCount();//�Զ�Ϣ��
#endif
//433唤醒模块
//	if((*pThreadCnt % UI_RF433CTR_PERIOD) == 0)
//	{
//		//if((!ubUI_CuStandbyFlag || (ubUI_CuStandbyFlag && TriggerLock)) && !ubUI_CuPowerDiscFlag && tUI_MenuItem.ubItemIdx != PLAYBACK_ITEM);
//			//UI_WakeUpTx();
//			
//		//if((ubUI_CuStandbyFlag || ubUI_CuPowerDiscFlag) && !TriggerLock)
//		//	UI_SleepTx();
//	}
	
	if((*pThreadCnt % UI_SYNCTIMESTAMP_PERIOD) == 0)
	{
		UI_SyncTimeStamp2TX();
	}
	
	
	osSemaphoreWait(osUI_CuSemId, osWaitForever);
	switch(tUI_SyncAppState)
	{
		case APP_IDLE_STATE:
			ubUI_SendMsg2AppFlag = TRUE;
			break;
		case APP_LOSTLINK_STATE:
			ubUI_SendMsg2AppFlag = TRUE;
			break;
		case APP_LINK_STATE:
			ubUI_SendMsg2AppFlag = TRUE;
			
			break;
		case APP_PAIRING_STATE:
			UI_DrawPairingStatusIcon();
			osSemaphoreRelease(osUI_CuSemId);
			return;
		default:
			break;
	}
	if(ubUI_SendMsg2AppFlag == TRUE)//����Ϣ��app���ȡtx�����ӵ�������?
	{
		if(tUI_MenuItem.ubItemIdx == PLAYBACK_ITEM && tUI_State > UI_MAINMENU_STATE)
		{
			osSemaphoreRelease(osUI_CuSemId);
			return;
		}
		tUI_GetLinkStsMsg.ubAPP_Event		= APP_LINKSTATUS_REPORT_EVENT;
		UI_SendMessageToAPP(&tUI_GetLinkStsMsg);
	}	
	
	osSemaphoreRelease(osUI_CuSemId);
	
	tUI_CuSetting.IconSts.ubRdPairIconFlag	= FALSE;
	(*pThreadCnt)++;
	if(*pThreadCnt == 0xffff)
		*pThreadCnt = 0;

}
void UI_MenuReturnEventExec(void *pvTriggerEvent)
{
	MenuOnFlag = FALSE;
	if(tUI_State == UI_SUBSUBMENU_STATE && tUI_MenuItem.ubItemIdx == PLAYBACK_ITEM)
	{
		UI_MenuKey();
	}
	UI_BackToDesktop(TRUE);
	ulPanelOffCount = PANELAUTOOFFCOUNT;
}
//------------------------------------------------------------------------------
void UI_DrawAIBoxHandles(Algo_Result *showbox)
{
	if(FALSE == ubUI_CuStartUpFlag || ubUI_CuPowerDiscFlag)
		return;
//	switch(ptEventPtr->tEventType)
//	{
//	
//		case DRAWBOX_EVENT:
//			if (tUI_State == UI_DISPLAY_STATE)
//			{
//				UI_EventDrawBox(ptEventPtr->pvEvent);
//			}
//			break;
//		case CLEARBOX_EVENT:
//			if (tUI_State == UI_DISPLAY_STATE)
//			{
//				//UI_EventClearBox(ptEventPtr->pvEvent);				
//			}
//			break;
//		default:
//			break;
//	}
	



	
}

//------------------------------------------------------------------------------
void UI_EventHandles(UI_Event_t *ptEventPtr)
{
	//printf("UI_EventHandles is %d\n",ptEventPtr->tEventType);
	if(FALSE == ubUI_CuStartUpFlag || ubUI_CuPowerDiscFlag || ubUI_CuFWU_1126Flag)
		return;
	switch(ptEventPtr->tEventType)
	{
		case AKEY_EVENT:
		case PKEY_EVENT:
			//20201221
		case IRKEY_EVENT:
			ulMenuAutoOffCount = MENUAUTOOFFCOUNT;
			ulPanelOffCount = PANELAUTOOFFCOUNT;
			UI_KeyEventExec(ptEventPtr->pvEvent);
			break;
		case SCANMODE_EVENT:
			UI_ScanModeExec();
			break;
		case FWUPG_EVENT:
			KNL_SDUpgradeFwFunc();
			break;
			//20210302
		case TRIGGER_EVENT:
			UI_TriggerEventExec(ptEventPtr->pvEvent);
			break;
			

		//20201229
		case TOUCH_EVENT:
			if(tUI_CuSetting.ubMenuLock )//�˵�����������Ӳ���Ӧ�κδ���?
				return;
			ulMenuAutoOffCount = MENUAUTOOFFCOUNT;
			ulPanelOffCount = PANELAUTOOFFCOUNT;
			UI_TouchEventExec(ptEventPtr->pvEvent);
			break;
		case MENUOFF_EVENT:
			UI_MenuReturnEventExec(ptEventPtr->pvEvent);
			break;	
		case GESTURE_EVENT:
			
			ulPanelOffCount = PANELAUTOOFFCOUNT;
			if(ubUI_CuStandbyFlag)
			{
				UI_WakeupStandby();
			}

			break;
#if 0
		case DRAWBOX_EVENT:
			if (tUI_State == UI_DISPLAY_STATE)
			{
				UI_EventDrawBox(ptEventPtr->pvEvent);
			}
			break;
		case CLEARBOX_EVENT:
			if (tUI_State == UI_DISPLAY_STATE)
			{
				UI_EventClearBox(ptEventPtr->pvEvent);
			}
			break;
#endif
		case REBOOT_EVENT:
			SYS_Reboot();
			break;
		
		case FWUSD_EVENT:
			//KNL_UsbdFwupgrade();
			break;
		default:
			break;
	}
	
}

//------------------------------------------------------------------------------
APP_EventMsg_t *tUI_ViewTypeSetup(UI_CamViewType_t *tViewType)
{
	static APP_EventMsg_t tUI_ViewTypeParam = {0};
	UI_CamNum_t tCamNum;
	if(tUI_CuSetting.tPowerOnMode <= POWERON_QUAD)
	{	
		UI_PanelOn();
		DeskTopShowView = tUI_ViewModeSel = tUI_CuSetting.tPowerOnMode; 	
		tUI_CuSetting.tLastMode = (UI_LastMode_t)tUI_CuSetting.tPowerOnMode;
	}
	else if(tUI_CuSetting.tPowerOnMode == POWERON_SCAN)
	{
		UI_PanelOn();
		tUI_ViewModeSel = SCANVIEW_ITEM;
		//�ҳ��Զ�ɨ���һ������?
		for(tCamNum = CAM1;tCamNum < AUTOSCAN_MAX/2;tCamNum ++)
		{
			if(tUI_CuSetting.AutoScanEnable[tCamNum])
				break;
		}

		DeskTopShowView = tCamNum;
		tUI_AutoScanNumSel = tCamNum;
		tUI_CuSetting.tLastMode = LASTMODE_SCAN;
	}
	else if(tUI_CuSetting.tPowerOnMode == POWERON_STANDBY)
	{
		UI_PanelOff();
		UI_AudioOff();
		DeskTopShowView = tUI_ViewModeSel = QUALVIEW_ITEM;
		tUI_CuSetting.tLastMode = LASTMODE_STANDBY;
		
	}
	else if(tUI_CuSetting.tPowerOnMode == POWERON_LAST)//��һ�ο��������ʾ��״�?
	{
		if(tUI_CuSetting.tLastMode <= LASTMODE_QUAD)
		{
			UI_PanelOn();
			DeskTopShowView = tUI_ViewModeSel = tUI_CuSetting.tLastMode;
		}
		else if(tUI_CuSetting.tLastMode == LASTMODE_SCAN)
		{
			UI_PanelOn();
			tUI_ViewModeSel = SCANVIEW_ITEM;
			tUI_AutoScanNumSel = DeskTopShowView = tUI_CuSetting.tLastView;
		}
		else if(tUI_CuSetting.tLastMode == LASTMODE_STANDBY)
		{
			UI_PanelOff();
			UI_AudioOff();
			DeskTopShowView = tUI_ViewModeSel = (UI_CamNum_t)tUI_CuSetting.tLastView;
		}
		else if(tUI_CuSetting.tLastMode == LASTMODE_SCAN_STANDBY)
		{
			UI_PanelOff();
			UI_AudioOff();
			tUI_ViewModeSel = SCANVIEW_ITEM;
			DeskTopShowView = (UI_CamNum_t)tUI_CuSetting.tLastView;
		}
	}
	else
	{
		UI_PanelOn();
		DeskTopShowView = tUI_ViewModeSel = POWERON_QUAD; 	
		tUI_CuSetting.tLastMode = LASTMODE_QUAD;
	}
	tUI_CuSetting.tLastView = DeskTopShowView;

	if(DeskTopShowView <= CAM4)
		tCamViewSel.tCamViewType = SINGLE_VIEW;
	else if(DeskTopShowView == DUALVIEW_ITEM)
		tCamViewSel.tCamViewType = DUAL_VIEW;
	else if(DeskTopShowView == QUALVIEW_ITEM)
		tCamViewSel.tCamViewType = QUAD_VIEW;
	
	*tViewType = tCamViewSel.tCamViewType;

	tCamViewSel.tCamViewPool[0] = NO_CAM;
	tCamViewSel.tCamViewPool[1] = NO_CAM;
	tCamViewSel.tCamViewPool[2] = NO_CAM;
	tCamViewSel.tCamViewPool[3] = NO_CAM;

	switch(tCamViewSel.tCamViewType)
	{
		case SINGLE_VIEW:
			tCamViewSel.tCamViewPool[0] = DeskTopShowView;
			tUI_ViewTypeParam.ubAPP_Message[0] = tCamViewSel.tCamViewPool[0];
			break;
		
		case DUAL_VIEW:
			for(tCamNum = CAM1;tCamNum <= CAM4;tCamNum++)
			{
				if(tUI_CamStatus[tCamNum].tCamDispLocation_Dual == DISP_LEFT)
					tCamViewSel.tCamViewPool[0] = tCamNum;
				if(tUI_CamStatus[tCamNum].tCamDispLocation_Dual == DISP_RIGHT)
					tCamViewSel.tCamViewPool[1] = tCamNum;
			}
			tUI_ViewTypeParam.ubAPP_Message[0] = tCamViewSel.tCamViewPool[0];
			tUI_ViewTypeParam.ubAPP_Message[1] = tCamViewSel.tCamViewPool[1];
			break;
			
		case QUAD_VIEW:
			
			for(tCamNum = CAM1; tCamNum <= CAM4; tCamNum++)
			{
				if(tUI_CamStatus[tCamNum].tCamDispLocation_Quad == DISP_UPPER_LEFT)
				{
					tUI_ViewTypeParam.ubAPP_Message[0] = tCamViewSel.tCamViewPool[0] = tCamNum;
				}
				else if(tUI_CamStatus[tCamNum].tCamDispLocation_Quad == DISP_UPPER_RIGHT)
				{
					tUI_ViewTypeParam.ubAPP_Message[1] = tCamViewSel.tCamViewPool[1] = tCamNum;
				}
				else if(tUI_CamStatus[tCamNum].tCamDispLocation_Quad == DISP_LOWER_LEFT)
				{
					tUI_ViewTypeParam.ubAPP_Message[2] = tCamViewSel.tCamViewPool[2] = tCamNum;
				}
				else if(tUI_CamStatus[tCamNum].tCamDispLocation_Quad == DISP_LOWER_RIGHT)
				{
					tUI_ViewTypeParam.ubAPP_Message[3] = tCamViewSel.tCamViewPool[3] = tCamNum;
				}
			}
			break;
		default:
			return NULL;
	}
	tUI_ViewTypeParam.ubAPP_Message[4] = UI_GetAudioSrc();
	tUI_ViewTypeParam.ubAPP_Message[6] = tUI_CuSetting.ubSD_Switch;
	//ubDrawBSDRange = 1;
	UI_DrawDesktopIcon();
	//ubDrawBSDRange = 0;

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
void UI_ClearOsdImageNoUpdate(void)
{
	OSD_IMG_INFO tOsdImgInfo;

	tOsdImgInfo.uwHSize  = uwOSD_GetHSize();
	tOsdImgInfo.uwVSize  = uwOSD_GetVSize();
	tOsdImgInfo.uwXStart = 0;
	tOsdImgInfo.uwYStart = 0;
	OSD_EraserImg1_NOUpdate(&tOsdImgInfo);
	//OSD_EraserImg1(&tOsdImgInfo);
	printf("clear!!!!!");
}
//------------------------------------------------------------------------------
void UI_PowerKey(void)
{
	if(TriggerLock || !ubUI_FinishViewSwitch)
		return;
	if(!ubUI_CuStandbyFlag)
	{
		if((tUI_State == UI_PAIRING_STATE) || (tUI_MenuItem.ubItemIdx == PLAYBACK_ITEM && tUI_State > UI_MAINMENU_STATE))//���ڶԽӻ��߽��벥�Ž��涼����power��
		{
			return;
		}
		
		UI_EnterStandby();
	}
	else
	{
		UI_WakeupStandby();
	}

}

//------------------------------------------------------------------------------
void UI_PowerLongKey(void)
{
	if(TriggerLock || ubUI_CuStandbyFlag)
		return;
	if(tUI_State == UI_DISPLAY_STATE)
	{
        BUZ_Call_DI(150);
		UI_MenuLock();
	}
}

//------------------------------------------------------------------------------
void UI_MenuKey(void)
{
	switch(tUI_State)
	{
		case UI_DISPLAY_STATE:
		case UI_MAINMENU_STATE:
		case UI_SUBMENU_STATE:
		case UI_SUBSUBMENU_STATE:
		case UI_SUBSUBSUBMENU_STATE:
		case UI_SUBSUBSUBSUBMENU_STATE:
		case UI_SET_VOLUME_STATE:
		case UI_SET_CUPSMODE_STATE:
		case UI_SET_CAMECOMODE_STATE:
		case UI_ENGMODE_STATE:
		case UI_CAMSETTINGMENU_STATE:
		case UI_SET_CAMCOLOR_STATE:
		case UI_DPTZ_CONTROL_STATE:
		case UI_MD_WINDOW_STATE:
		case UI_SDFWUPG_STATE:
		case UI_RECFILES_SEL_STATE:
		case UI_RECFOLDER_SEL_STATE:
		case UI_RECPLAYLIST_STATE:
		case UI_RECPLAYADOSRC_SEL_STATE:
		case UI_RECPLAYDISPTYPE_SEL_STATE:
		case UI_PHOTOPLAYLIST_STATE:
		case UI_SDCARDFMT_STATE:
		case UI_SHOW_SDCARD_INFO_STATE:
		case UI_SHOW_TX_VERSION_STATE:
			
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
		case UI_SET_VOLUME_STATE:
		case UI_RECFILES_SEL_STATE:
		case UI_RECFOLDER_SEL_STATE:
		case UI_ENGMODE_STATE:
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
		case UI_SET_VOLUME_STATE:
		case UI_SET_CAMCOLOR_STATE:
		case UI_DPTZ_CONTROL_STATE:
		case UI_MD_WINDOW_STATE:
		case UI_RECFILES_SEL_STATE:
		case UI_RECFOLDER_SEL_STATE:
		case UI_ENGMODE_STATE:
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
		case UI_SUBSUBSUBSUBMENU_STATE:
		case UI_SET_VOLUME_STATE:
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
		case UI_RECPLAYDISPTYPE_SEL_STATE:
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
		case UI_SUBSUBSUBSUBMENU_STATE:
		case UI_SET_VOLUME_STATE:
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
		case UI_RECPLAYDISPTYPE_SEL_STATE:
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
		case UI_SUBSUBSUBSUBMENU_STATE:
		case UI_SET_VOLUME_STATE:
		case UI_SET_CUPSMODE_STATE:
		case UI_SET_CAMECOMODE_STATE:
		case UI_CAMSETTINGMENU_STATE:
		case UI_DPTZ_CONTROL_STATE:
		case UI_MD_WINDOW_STATE:
		case UI_SDFWUPG_STATE:
		case UI_RECFILES_SEL_STATE:
		case UI_RECFOLDER_SEL_STATE:
		case UI_RECPLAYLIST_STATE:
		case UI_RECPLAYADOSRC_SEL_STATE:
		case UI_RECPLAYDISPTYPE_SEL_STATE:
		case UI_SDCARDFMT_STATE:
		case UI_ENGMODE_STATE:
		case UI_PAIRING_STATE:
			if(tUI_StateMap2MenuFunc[tUI_State].pvFuncPtr)
				tUI_StateMap2MenuFunc[tUI_State].pvFuncPtr(ENTER_ARROW);
			break;
		default:
			break;
	}
}
//-------------------------------------------------------------------------------
void UI_SelKey(void)
{
	switch(tUI_State)
	{
		case UI_DISPLAY_STATE:
		case UI_MAINMENU_STATE:
		case UI_SUBMENU_STATE:
		case UI_SUBSUBMENU_STATE:
		case UI_SUBSUBSUBMENU_STATE:
		case UI_SUBSUBSUBSUBMENU_STATE:
		case UI_SET_VOLUME_STATE:
		case UI_SET_CUPSMODE_STATE:
		case UI_SET_CAMECOMODE_STATE:
		case UI_CAMSETTINGMENU_STATE:
		case UI_DPTZ_CONTROL_STATE:
		case UI_MD_WINDOW_STATE:
		case UI_SDFWUPG_STATE:
		case UI_RECFILES_SEL_STATE:
		case UI_RECFOLDER_SEL_STATE:
		case UI_RECPLAYLIST_STATE:
		case UI_RECPLAYADOSRC_SEL_STATE:
		case UI_RECPLAYDISPTYPE_SEL_STATE:
		case UI_SDCARDFMT_STATE:
		case UI_ENGMODE_STATE:
			if(tUI_StateMap2MenuFunc[tUI_State].pvFuncPtr)
				tUI_StateMap2MenuFunc[tUI_State].pvFuncPtr(SELECT_ARROW);
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_PairingKey(void)
{
	switch(tUI_State)
	{
		case UI_DISPLAY_STATE:
			if(tUI_StateMap2MenuFunc[tUI_State].pvFuncPtr)
				tUI_StateMap2MenuFunc[tUI_State].pvFuncPtr(PAIRING_ARROW);
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_MirrorKey(void)
{
	switch(tUI_State)
	{
		case UI_DISPLAY_STATE:
			if(tUI_StateMap2MenuFunc[tUI_State].pvFuncPtr)
				tUI_StateMap2MenuFunc[tUI_State].pvFuncPtr(MIRROR_ARROW);
//				ADO_WavPlay(0);
//			//	osDelay(100);
////				ADO_WavStop();
//				printf("play 1111111111\N");
			break;
		default:
			break;
	}

}
//------------------------------------------------------------------------------

void UI_MuteKey(void)
{
	switch(tUI_State)
	{
		case UI_DISPLAY_STATE:
		case UI_SET_VOLUME_STATE:
			
			if(tUI_StateMap2MenuFunc[tUI_State].pvFuncPtr)
				tUI_StateMap2MenuFunc[tUI_State].pvFuncPtr(MUTE_ARROW);
			break;
		default:
			break;
	}

}
//------------------------------------------------------------------------------

void UI_VersionKey(void)
{
	switch(tUI_State)
	{
		case UI_DISPLAY_STATE:			
			if(tUI_StateMap2MenuFunc[tUI_State].pvFuncPtr)
				tUI_StateMap2MenuFunc[tUI_State].pvFuncPtr(VERSION_ARROW);

			break;
		default:
			break;
	}
}


//------------------------------------------------------------------------------
#define UI_MENUICON_NUM		7
#define UI_WRICON_OFFSET	2


//------------------------------------------------------------------------------
void UI_DisplayArrowKeyFunc(UI_ArrowKey_t tArrowKey)
{
	if(ubUI_CuStandbyFlag && !TriggerLock)
		return;
	uint8_t DrawboxCnt = 0;
	switch(tArrowKey)
	{
		case LEFT_ARROW:
			UI_DrawVolumeMenuPage();
			break;
		case RIGHT_ARROW:
			UI_DrawVolumeMenuPage();

			break;
		case MUTE_ARROW:
			tUI_CuSetting.ubMuteFlag = 1 - tUI_CuSetting.ubMuteFlag;
			UI_AudioOn();
			UI_DrawVolumeMenuPage();

			break;
		
		case EXIT_ARROW:
			
			if(!TriggerLock)//����ʱ��ֹ�����˵�
			{
				tUI_State = UI_MAINMENU_STATE;
				tUI_MenuItem.ubItemPreIdx = ADO_MODE_ITEM;
				tUI_MenuItem.ubItemIdx 	  = ADO_MODE_ITEM;
				MenuOnFlag = TRUE;
				while(1)
				{
					if((Drawing_BoxFlag[0] == 0 && Drawing_BoxFlag[1] == 0 && Drawing_BoxFlag[2] == 0 && Drawing_BoxFlag[3] == 0)\
						|| DrawboxCnt >= 5)
					{
						DrawboxCnt = 0;
						break;

					}
						
					else
					{
						osDelay(100);
						DrawboxCnt++;
						printf("wait draw box!!!!!!\n");
					}
					printf("wait draw box!!!!!!   1123\n");
				}
				DrawboxCnt = 0;
				//printf("UI_DisplayArrowKeyFunc osUI_CuUiCtrosSemaphoreWait!!!!!!   1111\n");
				osSemaphoreWait(osUI_CuUiCtr, osWaitForever);			
				UI_DrawMenuPage();
				osSemaphoreRelease(osUI_CuUiCtr);
				//printf("UI_DisplayArrowKeyFunc osUI_CuUiCtrosSemaphoreRelease!!!!!!   2222\n");
			}
			break;
		case ENTER_ARROW:
			if(UI_RECORDING_MODE == tUI_CuSetting.tVdoMode)
            {
                UI_VideoRecordingExec((UI_REC_START == tUI_RecPlayAct.tRecAct)?UI_REC_STOP:UI_REC_START);
            }
			break;
		case PAIRING_ARROW:
			if(TriggerLock)
				return;
			if(DeskTopShowView <= CAM4)//��������ܽ���Խ�
			{
				//ת���Խӽ���
				MenuOnFlag = TRUE;
				tUI_State = UI_MAINMENU_STATE;
				tUI_MenuItem.ubItemIdx = PAIRING_ITEM;
				UI_EnterKey();
				tUI_State = UI_SUBMENU_STATE;
				//ִ�жԽ�
				for(uint8_t i = 0;i < DeskTopShowView;i++)
					UI_RightArrowKey();
				UI_EnterKey();
			}
			break;
		case SELECT_ARROW:
			if((tUI_ViewModeSel == SCANVIEW_ITEM) || (ubUI_FinishViewSwitch == FALSE) || TriggerLock)
				return;
//			//wav
//			ADO_WavStop();
//			ISPlaying_wav = 0;
			
			if(tUI_ViewModeSel < QUALVIEW_ITEM)
				tUI_ViewModeSel ++;
			else
				tUI_ViewModeSel = CAM1;			
			DeskTopShowView = tUI_ViewModeSel;
			UI_SwitchViewType(tUI_ViewModeSel,TRUE);
			if(tUI_CuSetting.tPowerOnMode == POWERON_LAST)
				UI_SaveLastMode();
			
			UI_ClearOsdImage();
			//ubDrawBSDRange = 0;
			UI_DrawDesktopIcon();
			//ubDrawBSDRange = 1;
			break;
		case MIRROR_ARROW:
			if(DeskTopShowView <= CAM4)
			{
				if(tUI_CamStatus[DeskTopShowView].tCamConnSts == CAM_ONLINE)
				{
					uint8_t ubVal = tUI_CamStatus[DeskTopShowView].tCamImgFlip * 2 + tUI_CamStatus[DeskTopShowView].tCamImgMirror;//00 01 10 11
					if(ubVal < 3)
						ubVal ++;
					else
						ubVal = 0;
					tUI_CamStatus[DeskTopShowView].tCamImgFlip = ubVal >> 1;
					tUI_CamStatus[DeskTopShowView].tCamImgMirror = ubVal & 0x01;

					UI_CamNum_t tCamNum = DeskTopShowView;
					UI_CUReqCmd_t tCamSetCmd;
					tCamSetCmd.tDS_CamNum				= tCamNum;
					tCamSetCmd.ubCmd[UI_TWC_TYPE]		= UI_SETTING;
					tCamSetCmd.ubCmd[UI_SETTING_ITEM]	= UI_IMGPROC_SETTING;
					tCamSetCmd.ubCmd_Len				 = 4;

					tCamSetCmd.ubCmd[UI_SETTING_DATA]	= UI_IMGMIRROR_SETTING;
					tCamSetCmd.ubCmd[UI_SETTING_DATA+1] = tUI_CamStatus[tCamNum].tCamImgMirror;
					uint8_t ubTryCount = 5;
					while(--ubTryCount)
					{
						if(UI_SendRequestToCAM(osThreadGetId(), &tCamSetCmd) != rUI_SUCCESS)
						{
							printf("set UI_IMGMIRROR_SETTING Fail !\n");
							osDelay(100);	
						}
						else
							break;
					}

					tCamSetCmd.ubCmd[UI_SETTING_DATA]	= UI_IMGFLIP_SETTING;
					tCamSetCmd.ubCmd[UI_SETTING_DATA+1] = tUI_CamStatus[tCamNum].tCamImgFlip;
					ubTryCount = 5;
					while(--ubTryCount)
					{
						if(UI_SendRequestToCAM(osThreadGetId(), &tCamSetCmd) != rUI_SUCCESS)
						{
							printf("set UI_IMGFLIP_SETTING Fail !\n");
							osDelay(100);	
						}
						else
							break;
					}
					if(tUI_ViewModeSel == SCANVIEW_ITEM)
						ulAutoScanCount = tUI_CuSetting.AutoScanDuty[tUI_AutoScanNumSel]* 1000;
					UI_UpdateDevStatusInfo();
					UI_UpdateTxDevStatusInfo(tCamNum);
				}
				
			}
			
			break;
		
		case VERSION_ARROW:
			MenuOnFlag = TRUE;
			while(1)
			{
				if((Drawing_BoxFlag[0] == 0 && Drawing_BoxFlag[1] == 0 && Drawing_BoxFlag[2] == 0 && Drawing_BoxFlag[3] == 0)\
					|| DrawboxCnt >= 5)
				{
					DrawboxCnt = 0;
					break;

				}
					
				else
				{
					osDelay(100);
					DrawboxCnt++;
					printf("wait draw box!!!!!!\n");
				}
				
			}
			DrawboxCnt = 0;

			UI_DrawTxVersion();
			//UI_UART2_PutChar(0XFF);
			//UI_UART2_PutChar(0XDD);	
			//OSD_ImagePrintf(OSD_IMG_ROTATION_0, 350, 500, tUI_CharOsdImgInfo, OSD_UPDATE, "AI:");
			OSD_ImagePrintf(OSD_IMG_ROTATION_0, 450, 543, tUI_CharOsdImgInfo, OSD_UPDATE, receivedVersion);
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_DrawPairingStatusIcon(void)
{
	static OSD_IMG_INFO tPairOsdImgInfo[2] = {0},tCamsOsdImgInfo[8] = {0};
	static uint8_t ubIconOffset = 0;
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PAIRCAM1NOR_ICON, 8, &tCamsOsdImgInfo[0]);
	if(FALSE == tPairInfo.ubDrawFlag)
		return;
	if(FALSE == tUI_CuSetting.IconSts.ubRdPairIconFlag)
	{
		if(tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PAIRING_STS1_ICON, 2, &tPairOsdImgInfo[0]) != OSD_OK)
		{
			printd(DBG_ErrorLvl, "Load OSD Image FAIL, pls check (%d) !\n", __LINE__);
			return;
		}
		tUI_CuSetting.IconSts.ubRdPairIconFlag 	= TRUE;
		ubIconOffset     						= 1;
	}	

	tPairOsdImgInfo[ubIconOffset].uwXStart = 
		tCamsOsdImgInfo[2*tPairInfo.tPairSelCam].uwXStart + (tCamsOsdImgInfo[2*tPairInfo.tPairSelCam].uwHSize - tPairOsdImgInfo[ubIconOffset].uwHSize)/2;
	tOSD_Img2(&tPairOsdImgInfo[ubIconOffset], OSD_UPDATE);

	ubIconOffset = 1 - ubIconOffset;
}
//------------------------------------------------------------------------------
void UI_ReportPairingResult(UI_Result_t tResult)
{
	OSD_IMG_INFO tRdyOsdImgInfo, tEraseOsdImgInfo,tCamOsdImgInfo;

	tPairInfo.ubDrawFlag = FALSE;
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PAIRING_STS1_ICON, 1, &tEraseOsdImgInfo);
	tOSD_GetOsdImgInfor(1, OSD_IMG2, (((tUI_CuSetting.tLanguage != LANGUAGE_GERMAN)?OSD2IMG_PAIRCAM1NOR_ICON:
								OSD2IMG_PAIRCAM1NOR_ICON_GER)+(tPairInfo.tPairSelCam*2)+UI_ICON_HIGHLIGHT), 1, &tCamOsdImgInfo);
	switch(tResult)
	{
		case rUI_SUCCESS:
			tUI_CuSetting.ubPairedCamNum += (tUI_CuSetting.ubPairedCamNum >= tUI_CuSetting.ubTotalCamNum)?0:1;
			tUI_CamStatus[tPairInfo.tPairSelCam].ulCAM_ID = tPairInfo.tPairSelCam;
			
			
			tOSD_Img2(&tCamOsdImgInfo, OSD_QUEUE);
			
			tEraseOsdImgInfo.uwXStart = tCamOsdImgInfo.uwXStart + (tCamOsdImgInfo.uwHSize - tEraseOsdImgInfo.uwHSize)/2;
			OSD_EraserImg2(&tEraseOsdImgInfo);
			
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PAIRRDYMASK_ICON, 1, &tRdyOsdImgInfo);
			tRdyOsdImgInfo.uwXStart = tCamOsdImgInfo.uwXStart + (tCamOsdImgInfo.uwHSize - tRdyOsdImgInfo.uwHSize)/2;
			tOSD_Img2(&tRdyOsdImgInfo, OSD_UPDATE);
			
			UI_PairingSubMenuPage(RIGHT_ARROW);
			UI_UpdateDevStatusInfo();
			break;
		case rUI_FAIL:
			tEraseOsdImgInfo.uwXStart = tCamOsdImgInfo.uwXStart + (tCamOsdImgInfo.uwHSize - tEraseOsdImgInfo.uwHSize)/2;
			OSD_EraserImg2(&tEraseOsdImgInfo);
			if(tUI_CamStatus[tPairInfo.tPairSelCam].ulCAM_ID != INVALID_ID)
			{
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PAIRRDYMASK_ICON, 1, &tRdyOsdImgInfo);
				tRdyOsdImgInfo.uwXStart = tCamOsdImgInfo.uwXStart + (tCamOsdImgInfo.uwHSize - tRdyOsdImgInfo.uwHSize)/2;
				tOSD_Img2(&tRdyOsdImgInfo, OSD_UPDATE);

			}
			
			break;
		default:
			break;
	}
	tUI_State = UI_SUBMENU_STATE;
}
//------------------------------------------------------------------------------
void UI_ReportAppPairingResult(UI_Result_t tResult)
{
	UI_ClearOsdImage();
	switch(tResult)
	{
		case rUI_SUCCESS:
			tUI_CuSetting.ubPairedCamNum += (tUI_CuSetting.ubPairedCamNum >= tUI_CuSetting.ubTotalCamNum)?0:1;
			tUI_CamStatus[tPairInfo.tPairSelCam].ulCAM_ID = tPairInfo.tPairSelCam;
			
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
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_RECORDING_BLACK_ICON, 1, &tRecEreOsdImgInfo);
	OSD_EraserImg2(&tRecEreOsdImgInfo);
}
//------------------------------------------------------------------------------
static void UI_OsdLoadingDisplayThread(void const *argument)
{
	uint32_t ulUI_OsdLdWaitTickTime = osWaitForever;
	uint16_t uwUI_OsdLdDispSte;
	uint8_t ubLdOsdIdx = 0;
	OSD_IMG_INFO tWorkOsdImg[9], tRecOsdImgInfo[3];
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_WORKING_P1_ICON, 8, &tWorkOsdImg);

	while(1)
	{
		osMessageGet(osUI_OsdLdDispQueue, &uwUI_OsdLdDispSte, ulUI_OsdLdWaitTickTime);
		switch(uwUI_OsdLdDispSte)
		{
			case UI_SEARCH_DCIMFOLDER:
			case UI_SEARCH_RECFILES:
			{
				ubLdOsdIdx = 4;
				uint16_t uwTmpXStart, uwTmpYStart;
				uwTmpXStart = (OSD_WIDTH - tWorkOsdImg[ubLdOsdIdx].uwHSize)/2;
				uwTmpYStart = (OSD_HEIGHT - tWorkOsdImg[ubLdOsdIdx].uwVSize)/2;
				while(UI_OSDLDDISP_ON == UI_GetLdDispStatus())
				{
					tWorkOsdImg[ubLdOsdIdx].uwXStart = uwTmpXStart;
					tWorkOsdImg[ubLdOsdIdx].uwYStart = uwTmpYStart;
					tOSD_Img2(&tWorkOsdImg[ubLdOsdIdx++], OSD_UPDATE);
					ubLdOsdIdx = (ubLdOsdIdx >= 8)?4:ubLdOsdIdx;
					osDelay(200);
				}
				tWorkOsdImg[8].uwXStart = 420;
				tWorkOsdImg[8].uwYStart = 209;
				tWorkOsdImg[8].uwHSize  = 160;
				tWorkOsdImg[8].uwVSize  = 183;
				OSD_EraserImg2(&tWorkOsdImg[8]);
				uwUI_OsdLdDispSte = UI_LD_DEFU;
			}
				break;
			case UI_SDCARD_FORMAT:
			
				ubLdOsdIdx = 0;
				while(UI_OSDLDDISP_ON == UI_GetLdDispStatus())
				{
					tOSD_Img2(&tWorkOsdImg[ubLdOsdIdx], OSD_UPDATE);
					ubLdOsdIdx = (++ubLdOsdIdx >= 4)?0:ubLdOsdIdx;
					osDelay(200);
				}
				uwUI_OsdLdDispSte = UI_LD_DEFU;
				break;
			
			default:
				
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
		tUI_RecOsdImgDB.uwUI_RecNumArray[i] = OSD2IMG_FILE_C1_NUM0 + (tColorNum * 10) + i;
																			 
	tUI_RecOsdImgInfo.pNumImgIdxArray = tUI_RecOsdImgDB.uwUI_RecNumArray;
	for(i = 0; i < 26; i++)
	{
		tUI_RecOsdImgDB.uwUI_RecUpperLetterArray[i] = OSD2IMG_FILE_C1_UPA + (tColorNum * 26) + i;
																				      
		tUI_RecOsdImgDB.uwUI_RecLowerLetterArray[i] = OSD2IMG_FILE_C1_LWA + (tColorNum * 26) + i;
																					
	}
	tUI_RecOsdImgInfo.pUpperLetterImgIdxArray = tUI_RecOsdImgDB.uwUI_RecUpperLetterArray;
	tUI_RecOsdImgInfo.pLowerLetterImgIdxArray = tUI_RecOsdImgDB.uwUI_RecLowerLetterArray;
	for(i = 0; i < 4; i++)
		tUI_RecOsdImgDB.uwUI_RecSymbolArray[i] = (OSD2IMG_FILE_C1_COLON + (tColorNum * 4)) + i;
																				 
	tUI_RecOsdImgInfo.pSymbolImgIdxArray = tUI_RecOsdImgDB.uwUI_RecSymbolArray;
	tUI_RecImgColor = tColorNum;
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
void UI_UpdateOsdImg4MultiView(UI_CamViewType_t tView_Type, OSD_RESULT(*pOsdImgFuncPtr)(OSD_IMG_INFO *, OSD_UPDATE_TYP), OSD_IMG_INFO *pOsdImgInfo)
{
	uint16_t uwLcd_HSize = uwOSD_GetHSize();
	uint16_t uwLcd_VSize = uwOSD_GetVSize();
	uint16_t uwOriXStart = pOsdImgInfo->uwXStart, uwOriYStart = pOsdImgInfo->uwYStart;

	if(H_VIEW == tView_Type)
		return;
	pOsdImgInfo->uwXStart += (uwLcd_HSize/2);
	if(QUAD_VIEW == tView_Type)
	{
		pOsdImgFuncPtr(pOsdImgInfo, OSD_QUEUE);
		pOsdImgInfo->uwYStart += (uwLcd_VSize/2);
		pOsdImgFuncPtr(pOsdImgInfo, OSD_QUEUE);
		pOsdImgInfo->uwXStart = uwOriXStart;
	}
	pOsdImgFuncPtr(pOsdImgInfo, OSD_QUEUE);
	pOsdImgInfo->uwXStart = uwOriXStart;
	pOsdImgInfo->uwYStart = uwOriYStart;
	pOsdImgFuncPtr(pOsdImgInfo, OSD_UPDATE);
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

	UI_Map2BatLvl_t tBatMap[] = 
	{
		{BAT_NO, 	 0},
		{BAT_LVL0, 	 1},
		{BAT_LVL1, 	 2},
		{BAT_LVL2, 	 3},
		{BAT_LVL3, 	 4},
		{BAT_LVL4, 	 5},
		{BAT_FULL, 	 6},
		{BAT_CHARGE, 7},
	};
	
	uint8_t ubUI_AntLvlCnt = sizeof tAntMap / sizeof(UI_PerMap2AntLvl_t), ubIdx,ubUI_BatLvlCnt = sizeof tBatMap / sizeof(UI_Map2BatLvl_t);

	for(tCamNum = CAM1; tCamNum < tUI_CuSetting.ubTotalCamNum; tCamNum++)
	{
		UI_CamConnectStatus_t tOldCamConnSts = tUI_CamStatus[tCamNum].tCamConnSts;
		
		tUI_CamStatus[tCamNum].tCamConnSts 	= (pCamConnSts[APP_GetSTANumMappingTable(tCamNum)->tKNL_StaNum] == rLINK)?CAM_ONLINE:CAM_OFFLINE;
	
		if (tUI_CamStatus[tCamNum].tCamConnSts != tOldCamConnSts) 
		{
			if (tUI_CamStatus[tCamNum].tCamConnSts == CAM_ONLINE) 
			{
				if(tOldCamConnSts == CAM_OFFLINE)
				{
					printd(DBG_InfoLvl, ("CAM%d -> ONLINE\n"), tCamNum+1);
					
					printf("CAM%d -> ONLINE\n", tCamNum+1);
				}
			}
			else
			{
				if(tOldCamConnSts == CAM_ONLINE)
				{
					printd(DBG_InfoLvl, ("CAM%d -> OFFLINE\n"), tCamNum+1);
					printf("CAM%d -> OFFLINE\n", tCamNum+1);
					LCD_ChDisable(tCamNum);
					osMessagePut(UI_AI_clearBOXQueue, &tCamNum, 0);
				}
			}
		}

		//signal
		tUI_CamStatus[tCamNum].tCamAntLvl = ANT_NOSIGNAL;
		for(ubIdx = 0; ubIdx < ubUI_AntLvlCnt; ubIdx++)
		{
			if((pCamConnSts[(APP_GetSTANumMappingTable(tCamNum)->tKNL_StaNum) + 4]) <= tAntMap[ubIdx].ubPerValue)
			{
				tUI_CamStatus[tCamNum].tCamAntLvl = tAntMap[ubIdx].tAntLvl;
				break;
			}
		}
		//battery
		tUI_CamStatus[tCamNum].tCamBatLvl = BAT_NO;
		for(ubIdx = 0; ubIdx < ubUI_BatLvlCnt; ubIdx++)
		{
			if((pCamConnSts[(APP_GetSTANumMappingTable(tCamNum)->tKNL_StaNum) + 12]) == tBatMap[ubIdx].ubValue)
			{
				tUI_CamStatus[tCamNum].tCamBatLvl = tBatMap[ubIdx].tBatLvl;
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
void UI_UnBindCam(UI_CamNum_t tUI_DelCam)
{
	APP_EventMsg_t tUI_UnindBuMsg = {0};

	if(INVALID_ID == tUI_CamStatus[tUI_DelCam].ulCAM_ID)
		return;
	tUI_CamStatus[tUI_DelCam].ulCAM_ID 	  = INVALID_ID;
	tUI_CamStatus[tUI_DelCam].tCamConnSts = CAM_OFFLINE;
	tUI_CamStatus[tUI_DelCam].cCamVersion[0] = '\0';
	tUI_CuSetting.ubPairedCamNum -= (tUI_CuSetting.ubPairedCamNum == 0)?0:1;
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
			
			if(SINGLE_VIEW == tCamViewSel.tCamViewType)
				break;
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_MDTrigger(UI_CamNum_t tCamNum, void *pvTrig)
{
	printd(DBG_InfoLvl, ("%s: CAM%d\n"), __func__, tCamNum);
}
//------------------------------------------------------------------------------
void UI_VoiceTrigger(UI_CamNum_t tCamNum, void *pvTrig)
{
	if(DISPLAY_1T1R == tUI_CuSetting.ubTotalCamNum)
		return;
	tUI_ViewModeSel = tCamViewSel.tCamViewPool[0];
	tCamViewSel.tCamViewType = SINGLE_VIEW;
	tCamViewSel.tCamViewPool[0] = tCamNum;
	UI_SwitchCameraSource();	
	tUI_State = UI_DISPLAY_STATE;
}

//------------------------------------------------------------------------------
UI_Result_t UI_SendLaserorLedToCAM(osThreadId thread_id, UI_CUReqCmd_t *ptReqCmd,TWC_OPC Opc)
{
	UI_Result_t tReq_Result = rUI_SUCCESS;
	osEvent tReq_Event;
	APP_StaNumMap_t *pUI_CamNumMap = APP_GetSTANumMappingTable(ptReqCmd->tDS_CamNum);
	uint8_t ubUI_TwcRetry = 5;
	uint8_t *pubLampState = NULL;
	UI_CamNum_t tCamNum = ptReqCmd->tDS_CamNum;

	osMutexWait(osUI_CamCmdMutex, osWaitForever);
	if(thread_id == osUI_AILampThreadId)
	{
		pubLampState = (Opc == TWC_Laser_CTRL)?&ubAILaserState[tCamNum]:&ubAILedState[tCamNum];
		// 等锁期间菜单可能已改为常开，此时让手动设置接管，不能发送自动关灯。
		if((Opc == TWC_Laser_CTRL && tUI_CamStatus[tCamNum].tCamLaser == CAMLASER_ENABLE) ||
			(Opc == TWC_Led_CTRL && tUI_CamStatus[tCamNum].tCamLed == CAMLED_ENABLE))
		{
			*pubLampState = FALSE;
			osMutexRelease(osUI_CamCmdMutex);
			return rUI_SUCCESS;
		}
		if(tUI_CamStatus[tCamNum].tCamConnSts != CAM_ONLINE ||
			!UI_CheckTxVersion(tUI_CamStatus[tCamNum].cCamVersion,30,Laserchar,3))
		{
			osMutexRelease(osUI_CamCmdMutex);
			return rUI_FAIL;
		}
		// 等锁后再读倒计时与算法选择，避免发出已过期或旧通道的开灯命令。
		ptReqCmd->ubCmd[UI_SETTING_DATA+1] = uwAILampTimeout[tCamNum] &&
			tUI_CamStatus[tCamNum].ubAIAlgorithm == AI_ALGORITHM_PALLET &&
			!ubAIConfigSync && !ubUI_CuPowerDiscFlag && !ubUI_CuStandbyFlag && !KNL_UsbdFwuFg;
		if(*pubLampState == ptReqCmd->ubCmd[UI_SETTING_DATA+1])
		{
			osMutexRelease(osUI_CamCmdMutex);
			return rUI_SUCCESS;
		}
	}
	// 清掉本任务上一次命令遗留的应答；不使用灯控唤醒信号位。
	osSignalWait(osUI_SIGNALS, 0);

	tosUI_Notify.thread_id = thread_id;
	tosUI_Notify.iSignals  = osUI_SIGNALS;
	while(--ubUI_TwcRetry)
	{
		printf("UI_SendLaserorLedToCAM success ubUI_TwcRetry = %d\n", ubUI_TwcRetry);
		if(tTWC_Send(pUI_CamNumMap->tTWC_StaNum, Opc, &ptReqCmd->ubCmd[UI_SETTING_DATA+1], 1, 10) == TWC_SUCCESS)
		{
//			printf("UI_SendLaserorLedToCAM success ubUI_TwcRetry = %d\n", ubUI_TwcRetry);
			break;
		}
		
		osDelay(10);
	}
	if(!ubUI_TwcRetry)
	{
		tTWC_StopTwcSend(pUI_CamNumMap->tTWC_StaNum, Opc);
		tosUI_Notify.thread_id = NULL;
		if(pubLampState)
			*pubLampState = AI_LAMP_UNKNOWN;
		osMutexRelease(osUI_CamCmdMutex);
		return rUI_FAIL;
	}
	if(tosUI_Notify.thread_id != NULL)
	{
		tReq_Event = osSignalWait(tosUI_Notify.iSignals, UI_TWC_TIMEOUT);
		printf("tReq_Event.status = %d tReq_Event.value.signals = %d tosUI_Notify.iSignals = %d tosUI_Notify.tReportSts = %d\n", tReq_Event.status, tReq_Event.value.signals, tosUI_Notify.iSignals, tosUI_Notify.tReportSts);
		tReq_Result = (tReq_Event.status == osEventSignal)?((tReq_Event.value.signals & tosUI_Notify.iSignals) == tosUI_Notify.iSignals)?tosUI_Notify.tReportSts:rUI_FAIL:rUI_FAIL;
		tTWC_StopTwcSend(pUI_CamNumMap->tTWC_StaNum, Opc);
		tosUI_Notify.thread_id  = NULL;
		tosUI_Notify.iSignals   = NULL;
		tosUI_Notify.tReportSts = rUI_SUCCESS;
	}
	// 状态与命令共用一把锁，避免菜单关灯与算法状态回写交错。
	if(pubLampState)
		*pubLampState = (tReq_Result == rUI_SUCCESS)?ptReqCmd->ubCmd[UI_SETTING_DATA+1]:AI_LAMP_UNKNOWN;
	else if(tReq_Result == rUI_SUCCESS)
	{
		if(Opc == TWC_Laser_CTRL)
			ubAILaserState[tCamNum] = FALSE;
		else if(Opc == TWC_Led_CTRL)
			ubAILedState[tCamNum] = FALSE;
	}
	osMutexRelease(osUI_CamCmdMutex);
	return tReq_Result;
}


//------------------------------------------------------------------------------
UI_Result_t UI_SendRequestToCAM(osThreadId thread_id, UI_CUReqCmd_t *ptReqCmd)
{
	UI_Result_t tReq_Result = rUI_SUCCESS;
	osEvent tReq_Event;
	APP_StaNumMap_t *pUI_CamNumMap = APP_GetSTANumMappingTable(ptReqCmd->tDS_CamNum);
	uint8_t ubUI_TwcRetry = 5;

	// 与独立灯控任务共用发送及应答上下文，锁一直持有到本次事务结束。
	osMutexWait(osUI_CamCmdMutex, osWaitForever);
	osSignalWait(osUI_SIGNALS, 0);
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
		osMutexRelease(osUI_CamCmdMutex);
		return rUI_FAIL;
	}
	if(tosUI_Notify.thread_id != NULL)
	{
		tReq_Event = osSignalWait(tosUI_Notify.iSignals, UI_TWC_TIMEOUT);
		printf("tReq_Event.status = %d tReq_Event.value.signals = %d tosUI_Notify.iSignals = %d tosUI_Notify.tReportSts = %d\n", tReq_Event.status, tReq_Event.value.signals, tosUI_Notify.iSignals, tosUI_Notify.tReportSts);
		tReq_Result = (tReq_Event.status == osEventSignal)?((tReq_Event.value.signals & tosUI_Notify.iSignals) == tosUI_Notify.iSignals)?tosUI_Notify.tReportSts:rUI_FAIL:rUI_FAIL;
		tTWC_StopTwcSend(pUI_CamNumMap->tTWC_StaNum, TWC_UI_SETTING);
		tosUI_Notify.thread_id  = NULL;
		tosUI_Notify.iSignals   = NULL;
		tosUI_Notify.tReportSts = rUI_SUCCESS;
	}
	osMutexRelease(osUI_CamCmdMutex);
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
void UI_RecvTxVersion (TWC_TAG tRecv_StaNum, uint8_t *pData)
{
	memcpy(tUI_CamStatus[(UI_CamNum_t)tRecv_StaNum].cCamVersion,pData + 1, *pData);
}

//------------------------------------------------------------------------------
//�ж�TX�汾��
uint8_t UI_CheckTxVersion(char *pTxVersion, uint8_t ubTxVersionLen, char *pTxDst, uint8_t ubTxDst)
{
	int found = 0;
    for (int i = 0; pTxVersion[i] != '\0'; i++) {
        if (pTxVersion[i] == '\0' || pTxVersion[i] == ' ') {
            i++; // Skip spaces
        }
        if (pTxVersion[i] == pTxDst[0]) {
            found = 1;
            for (int j = 1; pTxDst[j] != '\0'; j++) {
                if (pTxVersion[i + j] != pTxDst[j]) {
                    found = 0;
                    break;
                }
            }
            if (found) {
                break;
            }
        }
    }
	return found;
}


//------------------------------------------------------------------------------
void UI_ResetUIParameter(void)
{
	printf("---------------------UI_ResetUIParameter--------------------------\n");
	UI_CamNum_t tCamNum;
	//UI_ParkinglinePoint_t tUI_ParkinglinePoint[4];
	uint16_t uwTemp;
	/****************************************tUI_CamStatus*******************************************/
	for(tCamNum = CAM1; tCamNum < CAM_4T; tCamNum++)
	{	
		
		tUI_CamStatus[tCamNum].tCamDispLocation_Quad = DISP_UPPER_LEFT + tCamNum;//quad
		
		if(tCamNum < 2)//dual
			tUI_CamStatus[tCamNum].tCamDispLocation_Dual = DISP_LEFT + tCamNum;
		else	
			tUI_CamStatus[tCamNum].tCamDispLocation_Dual = DISP_LOCATION_TYPE_MAX;

		tUI_CamStatus[tCamNum].tCamDispLocation = DISP_H_L + tCamNum;//H
		tUI_CamStatus[tCamNum].tCamPsMode = POWER_NORMAL_MODE;
		tUI_CamStatus[tCamNum].tCamAnrMode = CAMSET_OFF;
		tUI_CamStatus[tCamNum].tCam3DNRMode = CAMSET_OFF;
		tUI_CamStatus[tCamNum].tCamvLDCMode = CAMSET_OFF;
		tUI_CamStatus[tCamNum].tCamAecMode = CAMSET_OFF;
		tUI_CamStatus[tCamNum].tCamDisMode = CAMSET_OFF;
		tUI_CamStatus[tCamNum].tCamFlicker = CAMFLICKER_50HZ;
		tUI_CamStatus[tCamNum].tCamCbrMode = CAMSET_ON;
		tUI_CamStatus[tCamNum].tCamCondenseMode = CAMSET_OFF;
		tUI_CamStatus[tCamNum].tCamColorParam.ubColorBL = 50;
		tUI_CamStatus[tCamNum].tCamColorParam.ubColorContrast =	50;
		tUI_CamStatus[tCamNum].tCamColorParam.ubColorSaturation = 50;
		tUI_CamStatus[tCamNum].tCamColorParam.ubColorHue = 50;
		tUI_CamStatus[tCamNum].tCamImgFlip = CAMIMGFLIP_DISABLE;
		tUI_CamStatus[tCamNum].tCamImgMirror = CAMIMGMIRROR_DISABLE;
		tUI_CamStatus[tCamNum].tCamLaser = CAMLASER_ENABLE;
		tUI_CamStatus[tCamNum].tCamLed = CAMLED_ENABLE;
		tUI_CamStatus[tCamNum].ubVolumeLvl = 50;
		tUI_CamStatus[tCamNum].ubAIAlgorithm = 0;
		tUI_CamStatus[tCamNum].tPalletConfig.tDelayTurnOFF = 6;
		tUI_CamStatus[tCamNum].tPalletConfig.tFlowFrameInterval = 0;
		tUI_CamStatus[tCamNum].tPalletConfig.tRateRange = 2;
		tUI_CamStatus[tCamNum].tPalletConfig.tFlowPauseDuration = 3;
		tUI_CamStatus[tCamNum].tPalletConfig.tFlowRunDuration = 1;
		tUI_CamStatus[tCamNum].tPalletConfig.tDectability = 52;
		tUI_CamStatus[tCamNum].ubAIConfigVersion = UI_AI_CONFIG_VERSION;
		
		tUI_CuSetting.ubUpdateTxParam[tCamNum] = FALSE;
		tUI_CuSetting.bColorBLGain[tCamNum] = 0;
		tUI_CuSetting.bColorContrastGain[tCamNum] = 0;
		tUI_CuSetting.bColorSaturationGain[tCamNum] = 0;
		tUI_CuSetting.bColorHueGain[tCamNum] = 0;

	}
	/****************************************tUI_CuSetting*******************************************/

	tUI_CuSetting.AI_ubVolumeLvl = 50;

	//AutoScan
	for(tCamNum = 0; tCamNum < 7; tCamNum++)
	{
		tUI_CuSetting.AutoScanDuty[tCamNum] = 10;
		tUI_CuSetting.AutoScanEnable[tCamNum] = 1;
	}
	
	//Trigger
	for(tCamNum = CAM1; tCamNum < 5; tCamNum++)
	{
		tUI_CuSetting.TriggerDelay[tCamNum] = 2;
	}
	tUI_CuSetting.TriggerPriority[CAM1] = 1;
	tUI_CuSetting.TriggerPriority[CAM2] = 2;
	tUI_CuSetting.TriggerPriority[CAM3] = 3;
	tUI_CuSetting.TriggerPriority[CAM4] = 0;
	tUI_CuSetting.TriggerPriority[4] = 4;
	
	tUI_CuSetting.TriggerSplitView = QUALVIEW_ITEM;
	
	//PowerOn
	tUI_CuSetting.tOldPowerOnMode = tUI_CuSetting.tPowerOnMode = POWERON_QUAD;
	
	//parkingLine	
	for(tCamNum = CAM1; tCamNum <= CAM4; tCamNum++)
	{
		tUI_CuSetting.ParkingLineSize[tCamNum] = 5;
		tUI_CuSetting.tParkingLineEnable[tCamNum] = PARKING_LINE_AUTO;
		uwTemp = (1024 - 486)/2;
		tUI_CuSetting.ParkingLine_XY[tCamNum][0] = uwTemp >> 8;
		tUI_CuSetting.ParkingLine_XY[tCamNum][1] = (uint8_t)uwTemp;
		uwTemp = (600 - 295)/2;
		tUI_CuSetting.ParkingLine_XY[tCamNum][2] = uwTemp >> 8;
		tUI_CuSetting.ParkingLine_XY[tCamNum][3] = (uint8_t)uwTemp;
	}
	
	//record
	tUI_CuSetting.RecInfo.tREC_Time = RECTIME_5MIN;
	tUI_CuSetting.ubPowerOnRecord = 1;
	tUI_CuSetting.ubTimeStame = 1;
    //  0  FS_TZ_DST       = 0xD0,             //!< UTC-12:00, Dateline Standard Time
	//  1  FS_TZ_SST       = 0xD4,             //!< UTC-11:00, Samoa Standard Time
	//  2  FS_TZ_HST       = 0xD8,             //!< UTC-10:00, Hawaii Standard Time
	//  3  FS_TZ_ALASKAST  = 0xDC,             //!< UTC-09:00, Alaska Standard Time
	//  4  FS_TZ_PST       = 0xE0,             //!< UTC-08:00, Pacific Standard Time
	//  5  FS_TZ_MOUNTST   = 0xE4,             //!< UTC-07:00, Mountain Standard Time
	//  6  FS_TZ_CST       = 0xE8,             //!< UTC-06:00, Central Standard Time
	//  7  FS_TZ_EST       = 0xEC,             //!< UTC-05:00, Eastern Standard Time
	//  8  FS_TZ_ATLANTST  = 0xF0,             //!< UTC-04:00, Atlantic Standard time
	//  9  FS_TZ_NST       = 0xF2,             //!< UTC-03:30, Newfoundland Standard Time
	// 10  FS_TZ_GREENLANDST = 0xF4,           //!< UTC-03:00, Greenland Standard Time
	// 11  FS_TZ_MAST      = 0xF8,             //!< UTC-02:00, Mid-Atlantic Standard Time
	// 12  FS_TZ_AZORESST  = 0xFC              //!< UTC-01:00, Azores Standard Time
	// 13  FS_TZ_GMT       = 0x80,             //!< UTC=GMT, Greenwich Standard Time
	// 14  FS_TZ_CET       = 0x84,             //!< UTC+01:00, Central Europe Time
	// 15  FS_TZ_EEST      = 0x88,             //!< UTC+02:00, Eastern Europe Standard Time
	// 16  FS_TZ_MST       = 0x8C,             //!< UTC+03:00, Moscow Standard Time
	// 17  FS_TZ_AST       = 0x90,             //!< UTC+04:00, Arabian Standard Time
	// 18  FS_TZ_WAST      = 0x94,             //!< UTC+05:00, West Asia Standard Time
	// 19  FS_TZ_CAST      = 0x98,             //!< UTC+06:00, Central Asia Standard Time
	// 20  FS_TZ_NAST      = 0x9C,             //!< UTC+07:00, North Asia Standard Time
	// 21  FS_TZ_NAEST     = 0xA0,             //!< UTC+08:00, North Asia East Standard Time,GMT+08:00
	// 22  FS_TZ_TOKYOST   = 0xA4,             //!< UTC+09:00, Tokyo Standard Time
	// 23  FS_TZ_WPST      = 0xA8,             //!< UTC+10:00, West Pacific Standard Time
	// 24  FS_TZ_CPST      = 0xAC,             //!< UTC+11:00, Central Pacific Standard Time
	// 25  FS_TZ_NZST      = 0xB0,             //!< UTC+12:00, New Zealand Standard Time
	// 26  FS_TZ_TONGAST   = 0xB4,             //!< UTC+13:00, Tonga Standard Time
    //Time Zone
    tUI_CuSetting.ubtimezone = 6;

	// AI BSD Config
	//tUI_CuSetting.ubBSDBoxType = 1; // 改为BOX TYPE 0 1
	for(tCamNum = CAM1; tCamNum <= CAM4; tCamNum++)
	{	
		//tUI_CuSetting.ubIsEnableBSD[tCamNum] = 1;
		tUI_CuSetting.ubIsEnableBSDALARM[tCamNum] = 1;
		tUI_CuSetting.ubDetectPeopleFlag[tCamNum] = 1;
		tUI_CuSetting.ubDetectCarFlag[tCamNum] = 1;
		tUI_CuSetting.ubBSDTriggerOut[tCamNum]= 0;
		tUI_CuSetting.ubIsEnableBSDRANGE[tCamNum] = 1;
//		tUI_CuSetting.tUI_AIDetectlinePoint[tCamNum].tupLeft.xPoint = 400;
//		tUI_CuSetting.tUI_AIDetectlinePoint[tCamNum].tupLeft.yPoint = 100;
//		tUI_CuSetting.tUI_AIDetectlinePoint[tCamNum].tupRight.xPoint = 700;
//		tUI_CuSetting.tUI_AIDetectlinePoint[tCamNum].tupRight.yPoint = 100;
//		
//		tUI_CuSetting.tUI_AIDetectlinePoint[tCamNum].tmid_upLeft.xPoint = 350;
//		tUI_CuSetting.tUI_AIDetectlinePoint[tCamNum].tmid_upLeft.yPoint = 250;
//		tUI_CuSetting.tUI_AIDetectlinePoint[tCamNum].tmid_upRight.xPoint = 650;
//		tUI_CuSetting.tUI_AIDetectlinePoint[tCamNum].tmid_upRight.yPoint = 250;
//		
//		tUI_CuSetting.tUI_AIDetectlinePoint[tCamNum].tmid_downLeft.xPoint = 250;
//		tUI_CuSetting.tUI_AIDetectlinePoint[tCamNum].tmid_downLeft.yPoint = 350;
//		tUI_CuSetting.tUI_AIDetectlinePoint[tCamNum].tmid_downRight.xPoint = 750;
//		tUI_CuSetting.tUI_AIDetectlinePoint[tCamNum].tmid_downRight.yPoint = 350;
//		
//		tUI_CuSetting.tUI_AIDetectlinePoint[tCamNum].tdownLeft.xPoint = 160;
//		tUI_CuSetting.tUI_AIDetectlinePoint[tCamNum].tdownLeft.yPoint = 580;
//		tUI_CuSetting.tUI_AIDetectlinePoint[tCamNum].tdownRight.xPoint = 940;
//		tUI_CuSetting.tUI_AIDetectlinePoint[tCamNum].tdownRight.yPoint = 580;	
		tUI_CuSetting.tUI_AIDetectlinePoint[tCamNum] = (UI_ParkinglinePoint_t) 
		{
		    .tupLeft = {400, 100},
		    .tupRight = {700, 100},
		    .tmid_upLeft = {350, 250},
		    .tmid_upRight = {650, 250},
		    .tmid_downLeft = {250, 350},
		    .tmid_downRight = {750, 350},
		    .tdownLeft = {160, 580},
		    .tdownRight = {940, 580}
		};
	
		tUI_CuSetting.tLocation1[tCamNum] = 4;
		tUI_CuSetting.tLocation2[tCamNum] = 8;
//		{
//	        {300, 100}, {800, 100}, {250, 200}, {850, 200},
//	        {200, 300}, {900, 300}, {150, 550}, {950, 550}
//	    };
	}
	//backlight
	tUI_CuSetting.BriLvL.ulBL_UpdateLvL 	 = 99;
	//volume
	tUI_CuSetting.VolLvL.ulVOL_UpdateLvL	 = 50;
	
	//menu lock
	tUI_CuSetting.ubMenuLock = false;
	
	//hide channel name
	tUI_CuSetting.ubHideChannelName = true;
	
	//SD Switch
	tUI_CuSetting.ubSD_Switch = false;
	
	//buzzer
	tUI_CuSetting.ubBuzzer = true;

	//audio Source
	tUI_CuSetting.tAdoSrcCamNum_Dual = CamSource_CAM1;
	tUI_CuSetting.tAdoSrcCamNum_Single[0] = CamSource_CAM1;
	tUI_CuSetting.tAdoSrcCamNum_Single[1] = CamSource_CAM1;
	tUI_CuSetting.tAdoSrcCamNum_Single[2] = CamSource_CAM1;
	tUI_CuSetting.tAdoSrcCamNum_Single[3] = CamSource_CAM1;
	tUI_CuSetting.tAdoSrcCamNum_Quad = CamSource_CAM1;
	
	//auto dimmer
	tUI_CuSetting.ubAutoDimmer = FALSE;
	tUI_CuSetting.ubAutoDimmer_MinVal = 0;
	tUI_CuSetting.ubAutoDimmer_MaxVal = 99;

	//mute 
	tUI_CuSetting.ubMuteFlag = FALSE;

	//guideLine	
	for(tCamNum = CAM1; tCamNum <= CAM4; tCamNum++)
	{
		tUI_CuSetting.GuideLineSize[tCamNum] = 10;
		tUI_CuSetting.GuideLineEnable[tCamNum] = FALSE;
		uwTemp = (1024 - 704)/2;
		tUI_CuSetting.GuideLine_XY[tCamNum][0] = uwTemp >> 8;
		tUI_CuSetting.GuideLine_XY[tCamNum][1] = (uint8_t)uwTemp;
		uwTemp = (600 - 4)/2;
		tUI_CuSetting.GuideLine_XY[tCamNum][2] = uwTemp >> 8;
		tUI_CuSetting.GuideLine_XY[tCamNum][3] = (uint8_t)uwTemp;
	}

	tUI_CuSetting.tLanguage = LANGUAGE_ENGLISH;

	//tUI_ParkinglinePoint = {{300,100},{800,100},{250,200},{850,200},{200,300},{900,300},{150,550},{950,550}};
	//tUI_ParkinglinePoint[0].tdownLeft.xPoint = 300;

	UI_UpdateDevStatusInfo();
}
//------------------------------------------------------------------------------
uint8_t UI_CheckUIParameter(void)
{
	UI_CamNum_t tCamNum;
	uint8_t ubAIConfigUpdate = FALSE;
	uint8_t ubPalletSelected = FALSE;
	for(tCamNum = CAM1; tCamNum < CAM_4T; tCamNum++)
	{	
		UI_CHK_MYSYS(tUI_CamStatus[tCamNum].tCamDispLocation_Quad,DISP_LOWER_RIGHT + 1,DISP_UPPER_LEFT);
		
		if(tUI_CamStatus[tCamNum].tCamDispLocation_Dual != DISP_LOCATION_TYPE_MAX)
			UI_CHK_MYSYS(tUI_CamStatus[tCamNum].tCamDispLocation_Dual,DISP_RIGHT + 1,DISP_LEFT);
		
		
		UI_CHK_MYSYS(tUI_CamStatus[tCamNum].tCamDispLocation,DISP_H_CL + 1,DISP_H_L);
		UI_CHK_MYSYS(tUI_CamStatus[tCamNum].tCamPsMode,POWER_NORMAL_MODE + 1,POWER_NORMAL_MODE);
		UI_CHK_MYSYS(tUI_CamStatus[tCamNum].tCamAnrMode,CAMSET_ON + 1,CAMSET_OFF);
		UI_CHK_MYSYS(tUI_CamStatus[tCamNum].tCam3DNRMode,CAMSET_ON + 1,CAMSET_OFF);
		UI_CHK_MYSYS(tUI_CamStatus[tCamNum].tCamvLDCMode,CAMSET_ON + 1,CAMSET_OFF);
		UI_CHK_MYSYS(tUI_CamStatus[tCamNum].tCamAecMode,CAMSET_ON + 1,CAMSET_OFF);
		UI_CHK_MYSYS(tUI_CamStatus[tCamNum].tCamDisMode,CAMSET_ON + 1,CAMSET_OFF);
		UI_CHK_MYSYS(tUI_CamStatus[tCamNum].tCamFlicker,CAMFLICKER_60HZ + 1,CAMFLICKER_50HZ);
		UI_CHK_MYSYS(tUI_CamStatus[tCamNum].tCamCbrMode,CAMSET_ON + 1,CAMSET_ON);
		UI_CHK_MYSYS(tUI_CamStatus[tCamNum].tCamCondenseMode,CAMSET_ON + 1,CAMSET_OFF);

		UI_CHK_MYSYS(tUI_CamStatus[tCamNum].tCamColorParam.ubColorBL,99 + 1,50);
		UI_CHK_MYSYS(tUI_CamStatus[tCamNum].tCamColorParam.ubColorContrast,99 + 1,50);
		UI_CHK_MYSYS(tUI_CamStatus[tCamNum].tCamColorParam.ubColorSaturation,99 + 1,50);
		UI_CHK_MYSYS(tUI_CamStatus[tCamNum].tCamColorParam.ubColorHue,99 + 1,50);
		UI_CHK_MYSYS(tUI_CamStatus[tCamNum].tCamImgFlip,CAMIMGFLIP_ENABLE + 1,CAMIMGFLIP_DISABLE);
		UI_CHK_MYSYS(tUI_CamStatus[tCamNum].tCamImgMirror,CAMIMGFLIP_ENABLE + 1,CAMIMGFLIP_DISABLE);
		UI_CHK_MYSYS(tUI_CamStatus[tCamNum].tCamLaser,CAMLASER_ENABLE + 1,CAMLASER_ENABLE);  //Alan
		UI_CHK_MYSYS(tUI_CamStatus[tCamNum].tCamLed,CAMLED_ENABLE + 1,CAMLED_ENABLE);
		UI_CHK_MYSYS(tUI_CamStatus[tCamNum].ubVolumeLvl,99 + 1,50);

		UI_CHK_MYSYS(tUI_CuSetting.ubUpdateTxParam[tCamNum],TRUE + 1,FALSE);

		/* These eight bytes used to be reserved; keep the saved camera layout unchanged. */
		UI_PalletConfigInfo_t *pConfig = &tUI_CamStatus[tCamNum].tPalletConfig;
		if(tUI_CamStatus[tCamNum].ubAIConfigVersion != UI_AI_CONFIG_VERSION ||
			tUI_CamStatus[tCamNum].ubAIAlgorithm >= AI_ALGORITHM_COUNT ||
			pConfig->tDelayTurnOFF < AI_PALLET_DELAY_TURN_OFF_MIN || pConfig->tDelayTurnOFF > AI_PALLET_DELAY_TURN_OFF_MAX || pConfig->tDelayTurnOFF%AI_PALLET_DELAY_TURN_OFF_STEP ||
			pConfig->tFlowFrameInterval > AI_PALLET_FLOW_FRAME_INTERVAL_MAX || pConfig->tRateRange < AI_PALLET_RATE_RANGE_MIN || pConfig->tRateRange > AI_PALLET_RATE_RANGE_MAX ||
			pConfig->tFlowPauseDuration > AI_PALLET_FLOW_PAUSE_DURATION_MAX || pConfig->tFlowPauseDuration%AI_PALLET_FLOW_PAUSE_DURATION_STEP ||
			pConfig->tFlowRunDuration < AI_PALLET_FLOW_RUN_DURATION_MIN || pConfig->tFlowRunDuration > AI_PALLET_FLOW_RUN_DURATION_MAX || pConfig->tFlowRunDuration%AI_PALLET_FLOW_RUN_DURATION_STEP ||
			pConfig->tDectability > AI_PALLET_SENSITIVITY_MAX)
		{
			tUI_CamStatus[tCamNum].ubAIAlgorithm = 0;
			tUI_CamStatus[tCamNum].tPalletConfig.tDelayTurnOFF = 6;
			tUI_CamStatus[tCamNum].tPalletConfig.tFlowFrameInterval = 0;
			tUI_CamStatus[tCamNum].tPalletConfig.tRateRange = 2;
			tUI_CamStatus[tCamNum].tPalletConfig.tFlowPauseDuration = 3;
			tUI_CamStatus[tCamNum].tPalletConfig.tFlowRunDuration = 1;
			tUI_CamStatus[tCamNum].tPalletConfig.tDectability = 52;
			tUI_CamStatus[tCamNum].ubAIConfigVersion = UI_AI_CONFIG_VERSION;
			ubAIConfigUpdate = TRUE;
		}

		// 兼容已保存的旧配置：保留第一个栈板通道，其余恢复 BSD。
		if(tUI_CamStatus[tCamNum].ubAIAlgorithm == AI_ALGORITHM_PALLET)
		{
			if(ubPalletSelected)
			{
				tUI_CamStatus[tCamNum].ubAIAlgorithm = AI_ALGORITHM_BSD;
				ubAIConfigUpdate = TRUE;
			}
			ubPalletSelected = TRUE;
		}

	
	}

	UI_CHK_MYSYS(tUI_CuSetting.AI_ubVolumeLvl,99 + 1,50);

	//AutoScan
	for(tCamNum = 0; tCamNum < 6; tCamNum++)
	{
		UI_CHK_MYSYS(tUI_CuSetting.AutoScanDuty[tCamNum],60 + 1,10);
		UI_CHK_MYSYS(tUI_CuSetting.AutoScanEnable[tCamNum],TRUE + 1,TRUE);
	}
	
	//Trigger
	for(tCamNum = CAM1; tCamNum < 5; tCamNum++)
	{
		UI_CHK_MYSYS(tUI_CuSetting.TriggerDelay[tCamNum],60 + 1,2);
	}
	
	UI_CHK_MYSYS(tUI_CuSetting.TriggerPriority[CAM1],4 + 1,0);
	UI_CHK_MYSYS(tUI_CuSetting.TriggerPriority[CAM2],4 + 1,0);
	UI_CHK_MYSYS(tUI_CuSetting.TriggerPriority[CAM3],4 + 1,0);
	UI_CHK_MYSYS(tUI_CuSetting.TriggerPriority[CAM4],4 + 1,0);
	UI_CHK_MYSYS(tUI_CuSetting.TriggerPriority[4],4 + 1,0);
	
	UI_CHK_MYSYS(tUI_CuSetting.TriggerSplitView,QUALVIEW_ITEM + 1,QUALVIEW_ITEM);
		
	UI_CHK_MYSYS(tUI_CuSetting.tOldPowerOnMode,POWERON_MAX,POWERON_QUAD);
	UI_CHK_MYSYS(tUI_CuSetting.tPowerOnMode,POWERON_MAX,POWERON_QUAD);

	//parkingLine	
		for(tCamNum = CAM1; tCamNum <= CAM4; tCamNum++)
	{	
		UI_CHK_MYSYS(tUI_CuSetting.ParkingLineSize[tCamNum],11 + 1 ,3);
		UI_CHK_MYSYS(tUI_CuSetting.tParkingLineEnable[tCamNum],PARKING_LINE_MAX,PARKING_LINE_AUTO);
	}
	
	//record
	UI_CHK_MYSYS(tUI_CuSetting.RecInfo.tREC_Time,RECTIME_MAX,RECTIME_5MIN);
	UI_CHK_MYSYS(tUI_CuSetting.ubPowerOnRecord,TRUE + 1,TRUE);
	UI_CHK_MYSYS(tUI_CuSetting.ubTimeStame,TRUE + 1,TRUE);
    //  0  FS_TZ_DST       = 0xD0,             //!< UTC-12:00, Dateline Standard Time
	//  1  FS_TZ_SST       = 0xD4,             //!< UTC-11:00, Samoa Standard Time
	//  2  FS_TZ_HST       = 0xD8,             //!< UTC-10:00, Hawaii Standard Time
	//  3  FS_TZ_ALASKAST  = 0xDC,             //!< UTC-09:00, Alaska Standard Time
	//  4  FS_TZ_PST       = 0xE0,             //!< UTC-08:00, Pacific Standard Time
	//  5  FS_TZ_MOUNTST   = 0xE4,             //!< UTC-07:00, Mountain Standard Time
	//  6  FS_TZ_CST       = 0xE8,             //!< UTC-06:00, Central Standard Time
	//  7  FS_TZ_EST       = 0xEC,             //!< UTC-05:00, Eastern Standard Time
	//  8  FS_TZ_ATLANTST  = 0xF0,             //!< UTC-04:00, Atlantic Standard time
	//  9  FS_TZ_NST       = 0xF2,             //!< UTC-03:30, Newfoundland Standard Time
	// 10  FS_TZ_GREENLANDST = 0xF4,           //!< UTC-03:00, Greenland Standard Time
	// 11  FS_TZ_MAST      = 0xF8,             //!< UTC-02:00, Mid-Atlantic Standard Time
	// 12  FS_TZ_AZORESST  = 0xFC              //!< UTC-01:00, Azores Standard Time
	// 13  FS_TZ_GMT       = 0x80,             //!< UTC=GMT, Greenwich Standard Time
	// 14  FS_TZ_CET       = 0x84,             //!< UTC+01:00, Central Europe Time
	// 15  FS_TZ_EEST      = 0x88,             //!< UTC+02:00, Eastern Europe Standard Time
	// 16  FS_TZ_MST       = 0x8C,             //!< UTC+03:00, Moscow Standard Time
	// 17  FS_TZ_AST       = 0x90,             //!< UTC+04:00, Arabian Standard Time
	// 18  FS_TZ_WAST      = 0x94,             //!< UTC+05:00, West Asia Standard Time
	// 19  FS_TZ_CAST      = 0x98,             //!< UTC+06:00, Central Asia Standard Time
	// 20  FS_TZ_NAST      = 0x9C,             //!< UTC+07:00, North Asia Standard Time
	// 21  FS_TZ_NAEST     = 0xA0,             //!< UTC+08:00, North Asia East Standard Time,GMT+08:00
	// 22  FS_TZ_TOKYOST   = 0xA4,             //!< UTC+09:00, Tokyo Standard Time
	// 23  FS_TZ_WPST      = 0xA8,             //!< UTC+10:00, West Pacific Standard Time
	// 24  FS_TZ_CPST      = 0xAC,             //!< UTC+11:00, Central Pacific Standard Time
	// 25  FS_TZ_NZST      = 0xB0,             //!< UTC+12:00, New Zealand Standard Time
	// 26  FS_TZ_TONGAST   = 0xB4,             //!< UTC+13:00, Tonga Standard Time
    //time zone
	UI_CHK_MYSYS(tUI_CuSetting.ubtimezone,27, 21);

//	//show bsd box
//	UI_CHK_MYSYS(tUI_CuSetting.ubIsShowBSDBox, 1 + 1, 0);

	//backlight
	UI_CHK_MYSYS(tUI_CuSetting.BriLvL.ulBL_UpdateLvL,99 + 1,99);
	
	//volume
	UI_CHK_MYSYS(tUI_CuSetting.VolLvL.ulVOL_UpdateLvL,99 + 1,50);
	
	//menu lock	
	UI_CHK_MYSYS(tUI_CuSetting.ubMenuLock,TRUE + 1,FALSE);
	
	//hide channel name
	UI_CHK_MYSYS(tUI_CuSetting.ubHideChannelName,TRUE + 1,FALSE);

	//SD Switch
	UI_CHK_MYSYS(tUI_CuSetting.ubSD_Switch,TRUE + 1,FALSE);
	
	//buzzer
	UI_CHK_MYSYS(tUI_CuSetting.ubBuzzer,TRUE + 1,TRUE);

	//audio Source
	UI_CHK_MYSYS(tUI_CuSetting.tAdoSrcCamNum_Dual,CamSource_AI + 1,CamSource_CAM1);
	UI_CHK_MYSYS(tUI_CuSetting.tAdoSrcCamNum_Single[0],CamSource_AI+ 1,CamSource_CAM1);
	UI_CHK_MYSYS(tUI_CuSetting.tAdoSrcCamNum_Single[1],CamSource_AI+ 1,CamSource_CAM1);
	UI_CHK_MYSYS(tUI_CuSetting.tAdoSrcCamNum_Single[2],CamSource_AI+ 1,CamSource_CAM1);
	UI_CHK_MYSYS(tUI_CuSetting.tAdoSrcCamNum_Single[3],CamSource_AI+ 1,CamSource_CAM1);
	UI_CHK_MYSYS(tUI_CuSetting.tAdoSrcCamNum_Quad,CamSource_AI + 1,CamSource_CAM1);
	
	//auto dimmer
	UI_CHK_MYSYS(tUI_CuSetting.ubAutoDimmer,TRUE + 1,FALSE);
	UI_CHK_MYSYS(tUI_CuSetting.ubAutoDimmer_MinVal,99 + 1,0);
	UI_CHK_MYSYS(tUI_CuSetting.ubAutoDimmer_MaxVal,99 + 1,99);

	//mute 
	UI_CHK_MYSYS(tUI_CuSetting.ubMuteFlag,TRUE + 1,FALSE);

	//guideLine	
	for(tCamNum = CAM1; tCamNum <= CAM4; tCamNum++)
	{
		UI_CHK_MYSYS(tUI_CuSetting.GuideLineSize[tCamNum],17 + 1,10);
		UI_CHK_MYSYS(tUI_CuSetting.GuideLineEnable[tCamNum],TRUE + 1,FALSE);
	}
	
	//language
	UI_CHK_MYSYS(tUI_CuSetting.tLanguage,LANGUAGE_MAX,LANGUAGE_ENGLISH);
	if(ubAIConfigUpdate)
		UI_UpdateDevStatusInfo();
	return rUI_SUCCESS;
}
//------------------------------------------------------------------------------
extern bool ado_src_ai;
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
		printf("--------------------------------------------------------------------------------------------\n");
		printf("-----------------------------------device reset---------------------------------------------\n");
		printf("--------------------------------------------------------------------------------------------\n");
		/****************************************tUI_CamStatus*******************************************/
		UI_ResetUIParameter();
		
		SF_Read(ulUI_SFAddr, sizeof(UI_DeviceStatusInfo_t), (uint8_t *)&tUI_DevStsInfo);
		memcpy(tUI_CamStatus, tUI_DevStsInfo.tCAM_StatusInfo, (CAM_4T * sizeof(UI_CamStatus_t)));
		memcpy(&tUI_CuSetting, &tUI_DevStsInfo.tCU_SettingInfo, sizeof(UI_CUSetting_t));
		printd(DBG_InfoLvl, "UI TAG:%s\n",tUI_DevStsInfo.cbUI_DevStsTag);
		printd(DBG_InfoLvl, "UI VER:%s\n",tUI_DevStsInfo.cbUI_FwVersion);
	}
	//�������Ƿ���Ч
	if(UI_CheckUIParameter() == rUI_FAIL)
		UI_ResetUIParameter();
		
	tUI_CuSetting.ubTotalCamNum 			 = DISPLAY_MODE;

	tUI_CuSetting.IconSts.ubRdPairIconFlag   = FALSE;
	tUI_CuSetting.IconSts.ubClearThdCntFlag	 = FALSE;
	tUI_CuSetting.IconSts.ubShowLostLogoFlag = FALSE;
	tUI_CuSetting.IconSts.ubDrawMdTrigFlag 	 = FALSE;
	tUI_CuSetting.ubPairedCamNum			 = 0;
	tUI_CuSetting.tCamsBwMode				 = CAMS_4T_BWMODE;

	
	tUI_CuSetting.RecInfo.tREC_Mode = REC_LOOPING;
	tUI_CuSetting.tVdoMode = UI_RECORDING_MODE;

	
		
    if(tUI_CuSetting.RecInfo.tREC_Time == RECTIME_1MIN)
        REC_TimeSet(0,60);
    else if(tUI_CuSetting.RecInfo.tREC_Time == RECTIME_3MIN)
        REC_TimeSet(0,180);
    else if(tUI_CuSetting.RecInfo.tREC_Time == RECTIME_5MIN)
        REC_TimeSet(0,300);

	for(tCamNum = CAM1; tCamNum < CAM_4T; tCamNum++)
	{
		if(INVALID_ID != tUI_CamStatus[tCamNum].ulCAM_ID)
		{
			tUI_CuSetting.ubPairedCamNum += 1;
			tUI_CamStatus[tCamNum].tCamConnSts = CAM_OFFLINE;
		}
		tUI_CamStatus[tCamNum].cCamVersion[0] = '\0';
		tUI_CamStatus[tCamNum].ulFrameCount = 0;
		tUI_CamStatus[tCamNum].ulImageStuckCount = 0;
	}

	if((SINGLE_VIEW == tCamViewSel.tCamViewType && CamSource_AI == tUI_CuSetting.tAdoSrcCamNum_Single[tCamViewSel.tCamViewPool[0]]) ||
		(DUAL_VIEW == tCamViewSel.tCamViewType && CamSource_AI == tUI_CuSetting.tAdoSrcCamNum_Dual) ||
		(QUAD_VIEW == tCamViewSel.tCamViewType && CamSource_AI == tUI_CuSetting.tAdoSrcCamNum_Quad)) 
	{
		ado_src_ai = 1;
	}
}
//------------------------------------------------------------------------------
void UI_UpdateTxDevStatusInfo(UI_CamNum_t tCamNum)
{
	uint8_t ubTryCount;
	UI_CUReqCmd_t tCamSetCmd;
	tCamSetCmd.tDS_CamNum				= tCamNum;
	tCamSetCmd.ubCmd[UI_TWC_TYPE]		= UI_SETTING;
	tCamSetCmd.ubCmd[UI_SETTING_ITEM]	= UI_SAVEPARAMETERS;
	tCamSetCmd.ubCmd_Len				 = 4;
	
	ubTryCount = 5;
	while(--ubTryCount)
	{
		if(UI_SendRequestToCAM(osThreadGetId(), &tCamSetCmd) != rUI_SUCCESS)
		{
			printf("Camera UI_SAVEPARAMETERS Fail !\n");
			osDelay(20);	
		}
		else
			break;
	}
	printf("UI_UpdateTxDevStatusInfo tCamNum = %d success\n",tCamNum);
}
//------------------------------------------------------------------------------
void UI_UpdateDevStatusInfo(void)
{
	uint32_t ulUI_SFAddr = pSF_Info->ulSize - (UI_SF_START_SECTOR * pSF_Info->ulSecSize);
	UI_DeviceStatusInfo_t tUI_DevStsInfo = {{0}, {0}, {0}, {0}};
	
	//osMutexWait(APP_UpdateMutex, osWaitForever);
	memcpy(tUI_DevStsInfo.cbUI_DevStsTag, SF_AP_UI_SECTOR_TAG, sizeof(tUI_DevStsInfo.cbUI_DevStsTag) - 1);
	memcpy(tUI_DevStsInfo.cbUI_FwVersion, SN937XX_FW_VERSION, sizeof(tUI_DevStsInfo.cbUI_FwVersion) - 1);
	memcpy(tUI_DevStsInfo.tCAM_StatusInfo, tUI_CamStatus, (CAM_4T * sizeof(UI_CamStatus_t)));
	memcpy(&tUI_DevStsInfo.tCU_SettingInfo, &tUI_CuSetting, sizeof(UI_CUSetting_t));
	SF_DisableWrProtect();
	SF_Erase(SF_SE, ulUI_SFAddr, pSF_Info->ulSecSize, 1);
	SF_Write(ulUI_SFAddr, sizeof(UI_DeviceStatusInfo_t), (uint8_t *)&tUI_DevStsInfo);
	SF_EnableWrProtect();
	//osMutexRelease(APP_UpdateMutex);
}


//------------------------------------------------------------------------------
void UI_CheckScanModeCount(void)
{
	UI_Event_t tScanEvent;
	osMessageQId *pUI_ScanEventQH = NULL;
	if(ulAutoScanCount == 0 && ubAutoScanReady && !MenuOnFlag && !ubUI_CuStandbyFlag && !ubUI_CuPowerDiscFlag && !TriggerLock && (tUI_ViewModeSel == SCANVIEW_ITEM))
	{
		ubAutoScanReady = FALSE;
		tScanEvent.tEventType = SCANMODE_EVENT;
		tScanEvent.pvEvent 	  = NULL;
		pUI_ScanEventQH 	  = pUI_GetEventQueueHandle();
	    osMessagePut(*pUI_ScanEventQH, &tScanEvent, 0);
	}
}
//------------------------------------------------------------------------------
void UI_ScanModeExec(void)
{
	printf("UI_ScanModeExec\n");
	UI_CamNum_t tUI_ViewSel;
	
	if(MenuOnFlag || TriggerLock || ubUI_CuStandbyFlag || tUI_ViewModeSel != SCANVIEW_ITEM)
	{
		return;
	}
	
	tUI_AutoScanNumSel ++;
	if(tUI_AutoScanNumSel >= 6)
		tUI_AutoScanNumSel = (UI_CamNum_t)0; 
	for(tUI_ViewSel = tUI_AutoScanNumSel; tUI_ViewSel < 6; tUI_ViewSel++)
	{
		if(!tUI_CuSetting.AutoScanEnable[tUI_ViewSel])
		{
			//���һ�����涼û��ʹ�ܾͻص��ʼ�Ļ���?
			if(tUI_ViewSel == 5)
			{
				tUI_ViewSel = 0;
				if(!tUI_CuSetting.AutoScanEnable[tUI_ViewSel])
					continue;
			}
			else
				continue;
		}
		
		DeskTopShowView = tUI_ViewSel;
		UI_SwitchViewType(tUI_ViewSel,TRUE);
		if(tUI_CuSetting.tPowerOnMode == POWERON_LAST)
			UI_SaveLastMode();
		
		UI_ClearOsdImageNoUpdate();
		//ubDrawBSDRange = 0;
		UI_DrawDesktopIcon();
		//ubDrawBSDRange = 1;
		break;
		
	}
	tUI_AutoScanNumSel = tUI_ViewSel;
	printf("UI_ScanModeExec tUI_ViewSel = %d\n",tUI_ViewSel);
	ulAutoScanCount = tUI_CuSetting.AutoScanDuty[tUI_AutoScanNumSel]* 1000;
	ubAutoScanReady = TRUE;
}

//------------------------------------------------------------------------------
void UI_SwitchCameraSource(void)
{
	APP_EventMsg_t tUI_SwitchBuMsg = {0};

	for(uint8_t i = 0;i < 4;i++)
	{
		tUI_CamStatus[i].tCamConnSts = CAM_OFFLINE;
		ubCamDiscCount[i] = 0;
		tUI_CamStatus[i].tCamAntLvl = ANT_NOSIGNAL;
		tUI_CamStatus[i].ulFrameCount = 0;
		tUI_CamStatus[i].ulImageStuckCount = 0;
	}
	
	tUI_SwitchBuMsg.ubAPP_Event 	 = APP_VIEWTYPECHG_EVENT;
	tUI_SwitchBuMsg.ubAPP_Message[0] = 5;		//! Message Length	
	tUI_SwitchBuMsg.ubAPP_Message[1] = tCamViewSel.tCamViewType;
	tUI_SwitchBuMsg.ubAPP_Message[2] = tCamViewSel.tCamViewPool[0];
	tUI_SwitchBuMsg.ubAPP_Message[3] = tCamViewSel.tCamViewPool[1];
	tUI_SwitchBuMsg.ubAPP_Message[4] = tCamViewSel.tCamViewPool[2];
	tUI_SwitchBuMsg.ubAPP_Message[5] = tCamViewSel.tCamViewPool[3];
	tUI_SwitchBuMsg.ubAPP_Message[6] = tUI_CuSetting.ubSD_Switch;
	UI_SendMessageToAPP(&tUI_SwitchBuMsg);
	printf("---------------UI_SwitchCameraSource\n");

}

//------------------------------------------------------------------------------
void UI_SwitchAudioSource(UI_CamSource tCamNum)
{
	APP_EventMsg_t tUI_SwitchAdoSrcMsg = {0};

	tUI_SwitchAdoSrcMsg.ubAPP_Event 	 = APP_ADOSRCSEL_EVENT;
	tUI_SwitchAdoSrcMsg.ubAPP_Message[0] = 1;		//! Message Length
	tUI_SwitchAdoSrcMsg.ubAPP_Message[1] = tCamNum;
	tUI_SwitchAdoSrcMsg.ubAPP_Message[2] = FALSE;	//! (PS_ADOONLY_MODE == tUI_CuSetting.tPsMode)?FALSE:TRUE;
	UI_SendMessageToAPP(&tUI_SwitchAdoSrcMsg);
}
//------------------------------------------------------------------------------
void UI_DrawDesktopIcon(void)
{
	OSD_IMG_INFO tOsdImgInfo,tVoiceOsdImgInfo;
	uint16_t uwLcd_HSize = uwLCD_GetLcdHoSize();
	uint16_t uwLcd_VSize = uwLCD_GetLcdVoSize();
	UI_CamViewType_t tUI_CamViewType = tCamViewSel.tCamViewType;
	uint16_t uwCam1WordOsdIndex = (tUI_CuSetting.tLanguage != LANGUAGE_GERMAN)?OSD2IMG_DESKTOP_CAM1_WORD:OSD2IMG_DESKTOP_CAM1_WORD_GER;
	if (ubDrawBSDRange == 1)
	{
		UI_DrawBSDRange();
	}	
	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_DESKTOP_VOICE_MUTE + 1 - tUI_CuSetting.ubMuteFlag, 1, &tVoiceOsdImgInfo);
	switch(tUI_CamViewType)
	{
		case SINGLE_VIEW:
			printf("xxxxxx = %d\n", UI_GetAudioSrc());
			if(UI_GetAudioSrc() == CamSource_AI) {
				printf("!!!!!!!!!!!!!!!\n");
				if(!tUI_CuSetting.ubHideChannelName)
				{				
					tOSD_GetOsdImgInfor(1, OSD_IMG2, uwCam1WordOsdIndex + tCamViewSel.tCamViewPool[0], 1, &tOsdImgInfo);
					tOSD_Img1(&tOsdImgInfo, OSD_UPDATE);
				}
			} else {
				//voice
				tVoiceOsdImgInfo.uwXStart+= uwLcd_HSize/2;			
				if(!tUI_CuSetting.ubHideChannelName)
				{				
					tOSD_GetOsdImgInfor(1, OSD_IMG2, uwCam1WordOsdIndex + tCamViewSel.tCamViewPool[0], 1, &tOsdImgInfo);
					tOSD_Img1(&tOsdImgInfo, OSD_QUEUE);
				}
				tOSD_Img1(&tVoiceOsdImgInfo, OSD_UPDATE);
			}
			break;
		case DUAL_VIEW:
			if(tUI_CuSetting.tAdoSrcCamNum_Dual == tCamViewSel.tCamViewPool[0])//voice
				tOSD_Img1(&tVoiceOsdImgInfo, OSD_QUEUE);
			else if(tUI_CuSetting.tAdoSrcCamNum_Dual == tCamViewSel.tCamViewPool[1])
			{
				tVoiceOsdImgInfo.uwXStart += uwLcd_HSize/2;
				tOSD_Img1(&tVoiceOsdImgInfo, OSD_QUEUE);
			}
			
			if(!tUI_CuSetting.ubHideChannelName)
			{
				//LEFT
				tOSD_GetOsdImgInfor(1, OSD_IMG2, uwCam1WordOsdIndex + tCamViewSel.tCamViewPool[0], 1, &tOsdImgInfo);
				tOsdImgInfo.uwXStart -= uwLcd_HSize/4;
				tOSD_Img1(&tOsdImgInfo, OSD_QUEUE);
				//RIGHT
				tOSD_GetOsdImgInfor(1, OSD_IMG2, uwCam1WordOsdIndex + tCamViewSel.tCamViewPool[1], 1, &tOsdImgInfo);
				tOsdImgInfo.uwXStart += uwLcd_HSize/4;
				tOSD_Img1(&tOsdImgInfo, OSD_QUEUE);
			}
			//LINE
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DESKTOP_LINE_DUAL, 1, &tOsdImgInfo);
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
//			if (ubDrawBSDRange == 1)
//			{
//				tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
//			}
//			else
//			{
//				tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
//			}
			break;
		case QUAD_VIEW:
			if(tUI_CuSetting.tAdoSrcCamNum_Quad == tCamViewSel.tCamViewPool[0])//voice
			{
				tVoiceOsdImgInfo.uwYStart -= 600/2;//= uwLcd_VSize - (tVoiceOsdImgInfo.uwYStart + tVoiceOsdImgInfo.uwVSize);
				tOSD_Img1(&tVoiceOsdImgInfo, OSD_QUEUE);
			}			
			else if(tUI_CuSetting.tAdoSrcCamNum_Quad == tCamViewSel.tCamViewPool[1])
			{
				tVoiceOsdImgInfo.uwYStart -= uwLcd_VSize/2;
				tVoiceOsdImgInfo.uwXStart += uwLcd_HSize/2;
				tOSD_Img1(&tVoiceOsdImgInfo, OSD_QUEUE);
			}
			else if(tUI_CuSetting.tAdoSrcCamNum_Quad == tCamViewSel.tCamViewPool[2])
			{
				tOSD_Img1(&tVoiceOsdImgInfo, OSD_QUEUE);
			}
			else if(tUI_CuSetting.tAdoSrcCamNum_Quad == tCamViewSel.tCamViewPool[3])
			{
				tVoiceOsdImgInfo.uwXStart += uwLcd_HSize/2;
				tOSD_Img1(&tVoiceOsdImgInfo, OSD_QUEUE);
			}
			
			if(!tUI_CuSetting.ubHideChannelName)
			{
				//UPPER LEFT
				tOSD_GetOsdImgInfor(1, OSD_IMG2, uwCam1WordOsdIndex + tCamViewSel.tCamViewPool[0], 1, &tOsdImgInfo);
				tOsdImgInfo.uwXStart -= uwLcd_HSize/4;
				tOsdImgInfo.uwYStart -= uwLcd_VSize/2;
				tOSD_Img1(&tOsdImgInfo, OSD_QUEUE);
				//UPPER RIGHT
				tOSD_GetOsdImgInfor(1, OSD_IMG2, uwCam1WordOsdIndex + tCamViewSel.tCamViewPool[1], 1, &tOsdImgInfo);
				tOsdImgInfo.uwXStart += uwLcd_HSize/4;
				tOsdImgInfo.uwYStart -= uwLcd_VSize/2;
				tOSD_Img1(&tOsdImgInfo, OSD_QUEUE);
				//LOWER LEFT
				tOSD_GetOsdImgInfor(1, OSD_IMG2, uwCam1WordOsdIndex + tCamViewSel.tCamViewPool[2], 1, &tOsdImgInfo);
				tOsdImgInfo.uwXStart -= uwLcd_HSize/4;
				tOSD_Img1(&tOsdImgInfo, OSD_QUEUE);
				//LOWER RIGHT
				tOSD_GetOsdImgInfor(1, OSD_IMG2, uwCam1WordOsdIndex + tCamViewSel.tCamViewPool[3], 1, &tOsdImgInfo);
				tOsdImgInfo.uwXStart += uwLcd_HSize/4;
				tOSD_Img1(&tOsdImgInfo, OSD_QUEUE);
			}
			//LINE
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DESKTOP_LINE_QUAD_H, 1, &tOsdImgInfo);
			tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DESKTOP_LINE_QUAD_V, 1, &tOsdImgInfo);
			tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
//			if (ubDrawBSDRange == 1)
//			{
//				tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
//			}
//			else
//			{
//				tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
//			}
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void UI_DrawParkingLine(uint8_t ubCamNum,OSD_UPDATE_TYP tMode)
{
	OSD_IMG_INFO tOsdImgInfo_left,tOsdImgInfo_right,tOsdImgInfo_upper,tOsdImgInfo_lower;
	//left
	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_PARKINGLINE_LEFT, 1, &tOsdImgInfo_left);
	tOsdImgInfo_left.uwXStart = (tUI_CuSetting.ParkingLine_XY[ubCamNum][0] << 8) + tUI_CuSetting.ParkingLine_XY[ubCamNum][1];
	tOsdImgInfo_left.uwYStart = (tUI_CuSetting.ParkingLine_XY[ubCamNum][2] << 8) + tUI_CuSetting.ParkingLine_XY[ubCamNum][3];
	tOSD_Img2(&tOsdImgInfo_left, OSD_QUEUE);
	//upper
	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_PARKINGLINE_UPPER1 + tUI_CuSetting.ParkingLineSize[ubCamNum], 1, &tOsdImgInfo_upper);
	tOsdImgInfo_upper.uwXStart = tOsdImgInfo_left.uwXStart + tOsdImgInfo_left.uwHSize;
	tOsdImgInfo_upper.uwYStart = tOsdImgInfo_left.uwYStart;
	tOSD_Img2(&tOsdImgInfo_upper, OSD_QUEUE);
//	//mid upper
//	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_PARKINGLINE_UPPER1 + tUI_CuSetting.ParkingLineSize[ubCamNum], 1, &tOsdImgInfo_upper);
//	tOsdImgInfo_upper.uwXStart = tOsdImgInfo_left.uwXStart + tOsdImgInfo_left.uwHSize;
//	tOsdImgInfo_upper.uwYStart = tOsdImgInfo_left.uwYStart + tOsdImgInfo_left.uwVSize*1/3;
//	tOSD_Img2(&tOsdImgInfo_upper, OSD_QUEUE);
//
//	//mid lower
//	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_PARKINGLINE_UPPER1 + tUI_CuSetting.ParkingLineSize[ubCamNum], 1, &tOsdImgInfo_upper);
//	tOsdImgInfo_upper.uwXStart = tOsdImgInfo_left.uwXStart + tOsdImgInfo_left.uwHSize;
//	tOsdImgInfo_upper.uwYStart = tOsdImgInfo_left.uwYStart + tOsdImgInfo_left.uwVSize*2/3;
//	tOSD_Img2(&tOsdImgInfo_upper, OSD_QUEUE);
	
	//lower
	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_PARKINGLINE_LOWER1 + tUI_CuSetting.ParkingLineSize[ubCamNum], 1, &tOsdImgInfo_lower);
	tOsdImgInfo_lower.uwXStart = tOsdImgInfo_left.uwXStart + tOsdImgInfo_left.uwHSize;
	tOsdImgInfo_lower.uwYStart = tOsdImgInfo_left.uwYStart + tOsdImgInfo_left.uwVSize - tOsdImgInfo_lower.uwVSize; 
	tOSD_Img2(&tOsdImgInfo_lower, OSD_QUEUE);
	//right
	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_PARKINGLINE_RIGHT, 1, &tOsdImgInfo_right);
	tOsdImgInfo_right.uwXStart = tOsdImgInfo_left.uwXStart + tOsdImgInfo_left.uwHSize + tOsdImgInfo_upper.uwHSize;
	tOsdImgInfo_right.uwYStart = tOsdImgInfo_left.uwYStart;
	tOSD_Img2(&tOsdImgInfo_right, tMode);
	
}
//------------------------------------------------------------------------------
void UI_EraseParkingLine(uint8_t ubCamNum,OSD_UPDATE_TYP tMode)
{
	OSD_IMG_INFO tOsdImgInfo_left,tOsdImgInfo_right,tOsdImgInfo_upper,tOsdImgInfo_lower;
	//left
	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_PARKINGLINE_LEFT, 1, &tOsdImgInfo_left);
	tOsdImgInfo_left.uwXStart = (tUI_CuSetting.ParkingLine_XY[ubCamNum][0] << 8) + tUI_CuSetting.ParkingLine_XY[ubCamNum][1];
	tOsdImgInfo_left.uwYStart = (tUI_CuSetting.ParkingLine_XY[ubCamNum][2] << 8) + tUI_CuSetting.ParkingLine_XY[ubCamNum][3];
	OSD_EraserImg2_NoUpdate(&tOsdImgInfo_left);
	//upper
	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_PARKINGLINE_UPPER1 + tUI_CuSetting.ParkingLineSize[ubCamNum], 1, &tOsdImgInfo_upper);
	tOsdImgInfo_upper.uwXStart = tOsdImgInfo_left.uwXStart + tOsdImgInfo_left.uwHSize;
	tOsdImgInfo_upper.uwYStart = tOsdImgInfo_left.uwYStart;
	OSD_EraserImg2_NoUpdate(&tOsdImgInfo_upper);
//	//mid upper
//	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_PARKINGLINE_UPPER1 + tUI_CuSetting.ParkingLineSize[ubCamNum], 1, &tOsdImgInfo_upper);
//	tOsdImgInfo_upper.uwXStart = tOsdImgInfo_left.uwXStart + tOsdImgInfo_left.uwHSize;
//	tOsdImgInfo_upper.uwYStart = tOsdImgInfo_left.uwYStart + tOsdImgInfo_left.uwVSize*1/3;
//	OSD_EraserImg2_NoUpdate(&tOsdImgInfo_upper);
//
//	//mid lower
//	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_PARKINGLINE_UPPER1 + tUI_CuSetting.ParkingLineSize[ubCamNum], 1, &tOsdImgInfo_upper);
//	tOsdImgInfo_upper.uwXStart = tOsdImgInfo_left.uwXStart + tOsdImgInfo_left.uwHSize;
//	tOsdImgInfo_upper.uwYStart = tOsdImgInfo_left.uwYStart + tOsdImgInfo_left.uwVSize*2/3;
//	OSD_EraserImg2_NoUpdate(&tOsdImgInfo_upper);
	//lower
	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_PARKINGLINE_LOWER1 + tUI_CuSetting.ParkingLineSize[ubCamNum], 1, &tOsdImgInfo_lower);
	tOsdImgInfo_lower.uwXStart = tOsdImgInfo_left.uwXStart + tOsdImgInfo_left.uwHSize;
	tOsdImgInfo_lower.uwYStart = tOsdImgInfo_left.uwYStart + tOsdImgInfo_left.uwVSize - tOsdImgInfo_lower.uwVSize;
	OSD_EraserImg2_NoUpdate(&tOsdImgInfo_lower);
	//right
	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_PARKINGLINE_RIGHT, 1, &tOsdImgInfo_right);
	tOsdImgInfo_right.uwXStart = tOsdImgInfo_left.uwXStart + tOsdImgInfo_left.uwHSize + tOsdImgInfo_upper.uwHSize;
	tOsdImgInfo_right.uwYStart = tOsdImgInfo_left.uwYStart;
	if(tMode == OSD_UPDATE)
		OSD_EraserImg2(&tOsdImgInfo_right);
	else
		OSD_EraserImg2_NoUpdate(&tOsdImgInfo_right);

}
//------------------------------------------------------------------------------
void UI_DrawParkingLine_OSD1(uint8_t ubCamNum,OSD_UPDATE_TYP tMode)//防止被画框数据清掉，都用图层一会导致移动检测区域很�?
{
	OSD_IMG_INFO tOsdImgInfo_left,tOsdImgInfo_right,tOsdImgInfo_upper,tOsdImgInfo_lower;
	//left
	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_PARKINGLINE_LEFT, 1, &tOsdImgInfo_left);
	tOsdImgInfo_left.uwXStart = (tUI_CuSetting.ParkingLine_XY[ubCamNum][0] << 8) + tUI_CuSetting.ParkingLine_XY[ubCamNum][1];
	tOsdImgInfo_left.uwYStart = (tUI_CuSetting.ParkingLine_XY[ubCamNum][2] << 8) + tUI_CuSetting.ParkingLine_XY[ubCamNum][3];
	tOSD_Img1(&tOsdImgInfo_left, OSD_QUEUE);
	//upper
	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_PARKINGLINE_UPPER1 + tUI_CuSetting.ParkingLineSize[ubCamNum], 1, &tOsdImgInfo_upper);
	tOsdImgInfo_upper.uwXStart = tOsdImgInfo_left.uwXStart + tOsdImgInfo_left.uwHSize;
	tOsdImgInfo_upper.uwYStart = tOsdImgInfo_left.uwYStart;
	tOSD_Img1(&tOsdImgInfo_upper, OSD_QUEUE);
//	//mid upper
//	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_PARKINGLINE_UPPER1 + tUI_CuSetting.ParkingLineSize[ubCamNum], 1, &tOsdImgInfo_upper);
//	tOsdImgInfo_upper.uwXStart = tOsdImgInfo_left.uwXStart + tOsdImgInfo_left.uwHSize;
//	tOsdImgInfo_upper.uwYStart = tOsdImgInfo_left.uwYStart + tOsdImgInfo_left.uwVSize*1/3;
//	tOSD_Img1(&tOsdImgInfo_upper, OSD_QUEUE);
//	//mid lower
//	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_PARKINGLINE_UPPER1 + tUI_CuSetting.ParkingLineSize[ubCamNum], 1, &tOsdImgInfo_upper);
//	tOsdImgInfo_upper.uwXStart = tOsdImgInfo_left.uwXStart + tOsdImgInfo_left.uwHSize;
//	tOsdImgInfo_upper.uwYStart = tOsdImgInfo_left.uwYStart + tOsdImgInfo_left.uwVSize*2/3;
//	tOSD_Img1(&tOsdImgInfo_upper, OSD_QUEUE);
	//lower
	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_PARKINGLINE_LOWER1 + tUI_CuSetting.ParkingLineSize[ubCamNum], 1, &tOsdImgInfo_lower);
	tOsdImgInfo_lower.uwXStart = tOsdImgInfo_left.uwXStart + tOsdImgInfo_left.uwHSize;
	tOsdImgInfo_lower.uwYStart = tOsdImgInfo_left.uwYStart + tOsdImgInfo_left.uwVSize - tOsdImgInfo_lower.uwVSize; 
	tOSD_Img1(&tOsdImgInfo_lower, OSD_QUEUE);
	//right
	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_PARKINGLINE_RIGHT, 1, &tOsdImgInfo_right);
	tOsdImgInfo_right.uwXStart = tOsdImgInfo_left.uwXStart + tOsdImgInfo_left.uwHSize + tOsdImgInfo_upper.uwHSize;
	tOsdImgInfo_right.uwYStart = tOsdImgInfo_left.uwYStart;
	tOSD_Img1(&tOsdImgInfo_right, tMode);
	
}
//------------------------------------------------------------------------------
void UI_EraseParkingLine_OSD1(uint8_t ubCamNum,OSD_UPDATE_TYP tMode)
{
	OSD_IMG_INFO tOsdImgInfo_left,tOsdImgInfo_right,tOsdImgInfo_upper,tOsdImgInfo_lower;
	//left
	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_PARKINGLINE_LEFT, 1, &tOsdImgInfo_left);
	tOsdImgInfo_left.uwXStart = (tUI_CuSetting.ParkingLine_XY[ubCamNum][0] << 8) + tUI_CuSetting.ParkingLine_XY[ubCamNum][1];
	tOsdImgInfo_left.uwYStart = (tUI_CuSetting.ParkingLine_XY[ubCamNum][2] << 8) + tUI_CuSetting.ParkingLine_XY[ubCamNum][3];
	OSD_EraserImg1_NOUpdate(&tOsdImgInfo_left);
	//upper
	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_PARKINGLINE_UPPER1 + tUI_CuSetting.ParkingLineSize[ubCamNum], 1, &tOsdImgInfo_upper);
	tOsdImgInfo_upper.uwXStart = tOsdImgInfo_left.uwXStart + tOsdImgInfo_left.uwHSize;
	tOsdImgInfo_upper.uwYStart = tOsdImgInfo_left.uwYStart;
	OSD_EraserImg1_NOUpdate(&tOsdImgInfo_upper);
//	//mid upper
//	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_PARKINGLINE_UPPER1 + tUI_CuSetting.ParkingLineSize[ubCamNum], 1, &tOsdImgInfo_upper);
//	tOsdImgInfo_upper.uwXStart = tOsdImgInfo_left.uwXStart + tOsdImgInfo_left.uwHSize;
//	tOsdImgInfo_upper.uwYStart = tOsdImgInfo_left.uwYStart + tOsdImgInfo_left.uwVSize*1/3;
//	OSD_EraserImg1_NOUpdate(&tOsdImgInfo_upper);
//	//mid lower
//	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_PARKINGLINE_UPPER1 + tUI_CuSetting.ParkingLineSize[ubCamNum], 1, &tOsdImgInfo_upper);
//	tOsdImgInfo_upper.uwXStart = tOsdImgInfo_left.uwXStart + tOsdImgInfo_left.uwHSize;
//	tOsdImgInfo_upper.uwYStart = tOsdImgInfo_left.uwYStart + tOsdImgInfo_left.uwVSize*2/3;
//	OSD_EraserImg1_NOUpdate(&tOsdImgInfo_upper);
	
	//lower
	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_PARKINGLINE_LOWER1 + tUI_CuSetting.ParkingLineSize[ubCamNum], 1, &tOsdImgInfo_lower);
	tOsdImgInfo_lower.uwXStart = tOsdImgInfo_left.uwXStart + tOsdImgInfo_left.uwHSize;
	tOsdImgInfo_lower.uwYStart = tOsdImgInfo_left.uwYStart + tOsdImgInfo_left.uwVSize - tOsdImgInfo_lower.uwVSize;
	OSD_EraserImg1_NOUpdate(&tOsdImgInfo_lower);
	//right
	tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_PARKINGLINE_RIGHT, 1, &tOsdImgInfo_right);
	tOsdImgInfo_right.uwXStart = tOsdImgInfo_left.uwXStart + tOsdImgInfo_left.uwHSize + tOsdImgInfo_upper.uwHSize;
	tOsdImgInfo_right.uwYStart = tOsdImgInfo_left.uwYStart;
	if(tMode == OSD_UPDATE)
		OSD_EraserImg1(&tOsdImgInfo_right);
	else
		OSD_EraserImg1_NOUpdate(&tOsdImgInfo_right);

}
//------------------------------------------------------------------------------
void UI_DrawParkingLine_OSD1_DUAL(uint8_t ubCamNum,OSD_UPDATE_TYP tMode)//防止被画框数据清掉，都用图层一会导致移动检测区域很�?
{
#if 0
	OSD_IMG_INFO tOsdImgInfo_left,tOsdImgInfo_right,tOsdImgInfo_upper,tOsdImgInfo_lower;
	uint8_t DUAL_RIGHT_FLAG,ubIsShow;
	//----------------------left--------------------------------
	if(tUI_CuSetting.tParkingLineEnable[tCamViewSel.tCamViewPool[0]] == TRUE)
	{
		DUAL_RIGHT_FLAG = 0;
		ubCamNum = tCamViewSel.tCamViewPool[0];
		//left
		tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_PARKINGLINE_LEFT_DUAL, 1, &tOsdImgInfo_left);
		//tOsdImgInfo_left.uwHSize = tOsdImgInfo_left.uwHSize/2;
		tOsdImgInfo_left.uwXStart = (tUI_CuSetting.ParkingLine_XY[ubCamNum][0] << 8) + tUI_CuSetting.ParkingLine_XY[ubCamNum][1];
		tOsdImgInfo_left.uwXStart = tOsdImgInfo_left.uwXStart/2 + DUAL_RIGHT_FLAG*512;
		tOsdImgInfo_left.uwYStart = (tUI_CuSetting.ParkingLine_XY[ubCamNum][2] << 8) + tUI_CuSetting.ParkingLine_XY[ubCamNum][3];
		tOSD_Img1(&tOsdImgInfo_left, OSD_QUEUE);
		//upper
		tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_PARKINGLINE_UPPER1 + tUI_CuSetting.ParkingLineSize[ubCamNum], 1, &tOsdImgInfo_upper);
		tOsdImgInfo_upper.uwHSize = tOsdImgInfo_upper.uwHSize/2;
		tOsdImgInfo_upper.uwXStart = tOsdImgInfo_left.uwXStart + tOsdImgInfo_left.uwHSize;
		//tOsdImgInfo_upper.uwXStart = tOsdImgInfo_upper.uwXStart + DUAL_RIGHT_FLAG*512;
		tOsdImgInfo_upper.uwYStart = tOsdImgInfo_left.uwYStart;
		tOSD_Img1(&tOsdImgInfo_upper, OSD_QUEUE);
		//mid upper
		tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_PARKINGLINE_UPPER1 + tUI_CuSetting.ParkingLineSize[ubCamNum], 1, &tOsdImgInfo_upper);
		tOsdImgInfo_upper.uwHSize = tOsdImgInfo_upper.uwHSize/2;
		tOsdImgInfo_upper.uwXStart = tOsdImgInfo_left.uwXStart + tOsdImgInfo_left.uwHSize ;
		//tOsdImgInfo_upper.uwXStart = tOsdImgInfo_upper.uwXStart + DUAL_RIGHT_FLAG*512;
		tOsdImgInfo_upper.uwYStart = tOsdImgInfo_left.uwYStart + tOsdImgInfo_left.uwVSize*1/3;
		tOSD_Img1(&tOsdImgInfo_upper, OSD_QUEUE);
		//mid lower
		tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_PARKINGLINE_UPPER1 + tUI_CuSetting.ParkingLineSize[ubCamNum], 1, &tOsdImgInfo_upper);
		tOsdImgInfo_upper.uwHSize = tOsdImgInfo_upper.uwHSize/2;
		tOsdImgInfo_upper.uwXStart = tOsdImgInfo_left.uwXStart + tOsdImgInfo_left.uwHSize ;
		//tOsdImgInfo_upper.uwXStart = tOsdImgInfo_upper.uwXStart + DUAL_RIGHT_FLAG*512;
		tOsdImgInfo_upper.uwYStart = tOsdImgInfo_left.uwYStart + tOsdImgInfo_left.uwVSize*2/3;
		tOSD_Img1(&tOsdImgInfo_upper, OSD_QUEUE);
		//lower
		tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_PARKINGLINE_LOWER1 + tUI_CuSetting.ParkingLineSize[ubCamNum], 1, &tOsdImgInfo_lower);
		tOsdImgInfo_lower.uwHSize = tOsdImgInfo_lower.uwHSize/2;
		tOsdImgInfo_lower.uwXStart = tOsdImgInfo_left.uwXStart + tOsdImgInfo_left.uwHSize ;
		//tOsdImgInfo_lower.uwXStart = tOsdImgInfo_lower.uwXStart + DUAL_RIGHT_FLAG*512;
		tOsdImgInfo_lower.uwYStart = tOsdImgInfo_left.uwYStart + tOsdImgInfo_left.uwVSize - tOsdImgInfo_lower.uwVSize; 
		tOSD_Img1(&tOsdImgInfo_lower, OSD_QUEUE);
		//right
		tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_PARKINGLINE_RIGHT_DUAL, 1, &tOsdImgInfo_right);
		//tOsdImgInfo_right.uwHSize = tOsdImgInfo_right.uwHSize/2;
		tOsdImgInfo_right.uwXStart = tOsdImgInfo_left.uwXStart + tOsdImgInfo_left.uwHSize + tOsdImgInfo_upper.uwHSize;
		//tOsdImgInfo_right.uwXStart = tOsdImgInfo_right.uwXStart + DUAL_RIGHT_FLAG*512;
		tOsdImgInfo_right.uwYStart = tOsdImgInfo_left.uwYStart;
		tOSD_Img1(&tOsdImgInfo_right, tMode);
		ubIsShow = 1;
	}
	//---------------------right----------------------------------
	if(tUI_CuSetting.tParkingLineEnable[tCamViewSel.tCamViewPool[1]] == TRUE)
	{
		DUAL_RIGHT_FLAG = 1;
		ubCamNum = tCamViewSel.tCamViewPool[1];
		//left
		tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_PARKINGLINE_LEFT_DUAL, 1, &tOsdImgInfo_left);
		tOsdImgInfo_left.uwXStart = (tUI_CuSetting.ParkingLine_XY[ubCamNum][0] << 8) + tUI_CuSetting.ParkingLine_XY[ubCamNum][1];
		tOsdImgInfo_left.uwXStart = tOsdImgInfo_left.uwXStart/2 + DUAL_RIGHT_FLAG*512;
		tOsdImgInfo_left.uwYStart = (tUI_CuSetting.ParkingLine_XY[ubCamNum][2] << 8) + tUI_CuSetting.ParkingLine_XY[ubCamNum][3];
		tOSD_Img1(&tOsdImgInfo_left, OSD_QUEUE);
		//upper
		tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_PARKINGLINE_UPPER1 + tUI_CuSetting.ParkingLineSize[ubCamNum], 1, &tOsdImgInfo_upper);
		tOsdImgInfo_upper.uwHSize = tOsdImgInfo_upper.uwHSize/2;
		tOsdImgInfo_upper.uwXStart = tOsdImgInfo_left.uwXStart + tOsdImgInfo_left.uwHSize;
		//tOsdImgInfo_upper.uwXStart = tOsdImgInfo_upper.uwXStart + DUAL_RIGHT_FLAG*512;
		tOsdImgInfo_upper.uwYStart = tOsdImgInfo_left.uwYStart;
		tOSD_Img1(&tOsdImgInfo_upper, OSD_QUEUE);
		//mid upper
		tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_PARKINGLINE_UPPER1 + tUI_CuSetting.ParkingLineSize[ubCamNum], 1, &tOsdImgInfo_upper);
		tOsdImgInfo_upper.uwHSize = tOsdImgInfo_upper.uwHSize/2;
		tOsdImgInfo_upper.uwXStart = tOsdImgInfo_left.uwXStart + tOsdImgInfo_left.uwHSize ;
		//tOsdImgInfo_upper.uwXStart = tOsdImgInfo_upper.uwXStart + DUAL_RIGHT_FLAG*512;
		tOsdImgInfo_upper.uwYStart = tOsdImgInfo_left.uwYStart + tOsdImgInfo_left.uwVSize*1/3;
		tOSD_Img1(&tOsdImgInfo_upper, OSD_QUEUE);
		//mid lower
		tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_PARKINGLINE_UPPER1 + tUI_CuSetting.ParkingLineSize[ubCamNum], 1, &tOsdImgInfo_upper);
		tOsdImgInfo_upper.uwHSize = tOsdImgInfo_upper.uwHSize/2;
		tOsdImgInfo_upper.uwXStart = tOsdImgInfo_left.uwXStart + tOsdImgInfo_left.uwHSize ;
		//tOsdImgInfo_upper.uwXStart = tOsdImgInfo_upper.uwXStart + DUAL_RIGHT_FLAG*512;
		tOsdImgInfo_upper.uwYStart = tOsdImgInfo_left.uwYStart + tOsdImgInfo_left.uwVSize*2/3;
		tOSD_Img1(&tOsdImgInfo_upper, OSD_QUEUE);
		//lower
		tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_PARKINGLINE_LOWER1 + tUI_CuSetting.ParkingLineSize[ubCamNum], 1, &tOsdImgInfo_lower);
		tOsdImgInfo_lower.uwHSize = tOsdImgInfo_lower.uwHSize/2;
		tOsdImgInfo_lower.uwXStart = tOsdImgInfo_left.uwXStart + tOsdImgInfo_left.uwHSize ;
		//tOsdImgInfo_lower.uwXStart = tOsdImgInfo_lower.uwXStart + DUAL_RIGHT_FLAG*512;
		tOsdImgInfo_lower.uwYStart = tOsdImgInfo_left.uwYStart + tOsdImgInfo_left.uwVSize - tOsdImgInfo_lower.uwVSize; 
		tOSD_Img1(&tOsdImgInfo_lower, OSD_QUEUE);
		//right
		tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_PARKINGLINE_RIGHT_DUAL, 1, &tOsdImgInfo_right);
		tOsdImgInfo_right.uwXStart = tOsdImgInfo_left.uwXStart + tOsdImgInfo_left.uwHSize + tOsdImgInfo_upper.uwHSize;
		//tOsdImgInfo_right.uwXStart = tOsdImgInfo_right.uwXStart + DUAL_RIGHT_FLAG*512;
		tOsdImgInfo_right.uwYStart = tOsdImgInfo_left.uwYStart;
		tOSD_Img1(&tOsdImgInfo_right, tMode);

	}		
#endif
}

//------------------------------------------------------------------------------
void UI_DrawParkingLine_OSD1_QUAL(uint8_t ubCamNum,OSD_UPDATE_TYP tMode)//防止被画框数据清掉，都用图层一会导致移动检测区域很�?
{
#if 0
	OSD_IMG_INFO tOsdImgInfo_left,tOsdImgInfo_right,tOsdImgInfo_upper,tOsdImgInfo_lower;
	uint8_t DUAL_RIGHT_FLAG,DUAL_LOWER_FLAG,ubIsShow;
	//----------------------left--------------------------------
	if(tUI_CuSetting.tParkingLineEnable[tCamViewSel.tCamViewPool[0]] == TRUE)
	{
		DUAL_RIGHT_FLAG = 0;
		DUAL_LOWER_FLAG = 0;
		ubCamNum = tCamViewSel.tCamViewPool[0];
		//left
		tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_PARKINGLINE_LEFT_QUAL, 1, &tOsdImgInfo_left);
		//tOsdImgInfo_left.uwHSize = tOsdImgInfo_left.uwHSize/2;
		tOsdImgInfo_left.uwXStart = (tUI_CuSetting.ParkingLine_XY[ubCamNum][0] << 8) + tUI_CuSetting.ParkingLine_XY[ubCamNum][1];
		tOsdImgInfo_left.uwXStart = tOsdImgInfo_left.uwXStart/2 + DUAL_RIGHT_FLAG*512;
		tOsdImgInfo_left.uwYStart = (tUI_CuSetting.ParkingLine_XY[ubCamNum][2] << 8) + tUI_CuSetting.ParkingLine_XY[ubCamNum][3];
		tOsdImgInfo_left.uwYStart = tOsdImgInfo_left.uwYStart/2 + DUAL_LOWER_FLAG*300;
		tOSD_Img1(&tOsdImgInfo_left, OSD_QUEUE);
		//upper
		tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_PARKINGLINE_UPPER1 + tUI_CuSetting.ParkingLineSize[ubCamNum], 1, &tOsdImgInfo_upper);
		tOsdImgInfo_upper.uwHSize = tOsdImgInfo_upper.uwHSize/2;
		tOsdImgInfo_upper.uwVSize = 3;
		tOsdImgInfo_upper.uwXStart = tOsdImgInfo_left.uwXStart + tOsdImgInfo_left.uwHSize;
		//tOsdImgInfo_upper.uwXStart = tOsdImgInfo_upper.uwXStart + DUAL_RIGHT_FLAG*512;
		tOsdImgInfo_upper.uwYStart = tOsdImgInfo_left.uwYStart;
		tOSD_Img1(&tOsdImgInfo_upper, OSD_QUEUE);
		//mid upper
		tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_PARKINGLINE_UPPER1 + tUI_CuSetting.ParkingLineSize[ubCamNum], 1, &tOsdImgInfo_upper);
		tOsdImgInfo_upper.uwHSize = tOsdImgInfo_upper.uwHSize/2;
		tOsdImgInfo_upper.uwVSize = 3;
		tOsdImgInfo_upper.uwXStart = tOsdImgInfo_left.uwXStart + tOsdImgInfo_left.uwHSize ;
		//tOsdImgInfo_upper.uwXStart = tOsdImgInfo_upper.uwXStart + DUAL_RIGHT_FLAG*512;
		tOsdImgInfo_upper.uwYStart = tOsdImgInfo_left.uwYStart + tOsdImgInfo_left.uwVSize*1/3;
		tOSD_Img1(&tOsdImgInfo_upper, OSD_QUEUE);
		//mid lower
		tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_PARKINGLINE_UPPER1 + tUI_CuSetting.ParkingLineSize[ubCamNum], 1, &tOsdImgInfo_upper);
		tOsdImgInfo_upper.uwHSize = tOsdImgInfo_upper.uwHSize/2;
		tOsdImgInfo_upper.uwVSize = 3;
		tOsdImgInfo_upper.uwXStart = tOsdImgInfo_left.uwXStart + tOsdImgInfo_left.uwHSize ;
		//tOsdImgInfo_upper.uwXStart = tOsdImgInfo_upper.uwXStart + DUAL_RIGHT_FLAG*512;
		tOsdImgInfo_upper.uwYStart = tOsdImgInfo_left.uwYStart + tOsdImgInfo_left.uwVSize*2/3;
		tOSD_Img1(&tOsdImgInfo_upper, OSD_QUEUE);
		//lower
		tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_PARKINGLINE_LOWER1 + tUI_CuSetting.ParkingLineSize[ubCamNum], 1, &tOsdImgInfo_lower);
		tOsdImgInfo_lower.uwHSize = tOsdImgInfo_lower.uwHSize/2;
		tOsdImgInfo_lower.uwVSize = 3;
		tOsdImgInfo_lower.uwXStart = tOsdImgInfo_left.uwXStart + tOsdImgInfo_left.uwHSize ;
		//tOsdImgInfo_lower.uwXStart = tOsdImgInfo_lower.uwXStart + DUAL_RIGHT_FLAG*512;
		tOsdImgInfo_lower.uwYStart = tOsdImgInfo_left.uwYStart + tOsdImgInfo_left.uwVSize - tOsdImgInfo_lower.uwVSize; 
		tOSD_Img1(&tOsdImgInfo_lower, OSD_QUEUE);
		//right
		tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_PARKINGLINE_RIGHT_QUAL, 1, &tOsdImgInfo_right);
		//tOsdImgInfo_right.uwHSize = tOsdImgInfo_right.uwHSize/2;
		tOsdImgInfo_right.uwXStart = tOsdImgInfo_left.uwXStart + tOsdImgInfo_left.uwHSize + tOsdImgInfo_upper.uwHSize;
		//tOsdImgInfo_right.uwXStart = tOsdImgInfo_right.uwXStart + DUAL_RIGHT_FLAG*512;
		tOsdImgInfo_right.uwYStart = tOsdImgInfo_left.uwYStart;
		tOSD_Img1(&tOsdImgInfo_right, tMode);
		ubIsShow = 1;
	}
	//right
	if(tUI_CuSetting.tParkingLineEnable[tCamViewSel.tCamViewPool[1]] == TRUE)
	{
		DUAL_RIGHT_FLAG = 1;
		DUAL_LOWER_FLAG = 0;
		ubCamNum = tCamViewSel.tCamViewPool[1];
		//left
		tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_PARKINGLINE_LEFT_QUAL, 1, &tOsdImgInfo_left);
		//tOsdImgInfo_left.uwHSize = tOsdImgInfo_left.uwHSize/2;
		tOsdImgInfo_left.uwXStart = (tUI_CuSetting.ParkingLine_XY[ubCamNum][0] << 8) + tUI_CuSetting.ParkingLine_XY[ubCamNum][1];
		tOsdImgInfo_left.uwXStart = tOsdImgInfo_left.uwXStart/2 + DUAL_RIGHT_FLAG*512;
		tOsdImgInfo_left.uwYStart = (tUI_CuSetting.ParkingLine_XY[ubCamNum][2] << 8) + tUI_CuSetting.ParkingLine_XY[ubCamNum][3];
		tOsdImgInfo_left.uwYStart = tOsdImgInfo_left.uwYStart/2 + DUAL_LOWER_FLAG*300;
		tOSD_Img1(&tOsdImgInfo_left, OSD_QUEUE);
		//upper
		tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_PARKINGLINE_UPPER1 + tUI_CuSetting.ParkingLineSize[ubCamNum], 1, &tOsdImgInfo_upper);
		tOsdImgInfo_upper.uwHSize = tOsdImgInfo_upper.uwHSize/2;
		tOsdImgInfo_upper.uwVSize = 3;
		tOsdImgInfo_upper.uwXStart = tOsdImgInfo_left.uwXStart + tOsdImgInfo_left.uwHSize;
		//tOsdImgInfo_upper.uwXStart = tOsdImgInfo_upper.uwXStart + DUAL_RIGHT_FLAG*512;
		tOsdImgInfo_upper.uwYStart = tOsdImgInfo_left.uwYStart;
		tOSD_Img1(&tOsdImgInfo_upper, OSD_QUEUE);
		//mid upper
		tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_PARKINGLINE_UPPER1 + tUI_CuSetting.ParkingLineSize[ubCamNum], 1, &tOsdImgInfo_upper);
		tOsdImgInfo_upper.uwHSize = tOsdImgInfo_upper.uwHSize/2;
		tOsdImgInfo_upper.uwVSize = 3;
		tOsdImgInfo_upper.uwXStart = tOsdImgInfo_left.uwXStart + tOsdImgInfo_left.uwHSize ;
		//tOsdImgInfo_upper.uwXStart = tOsdImgInfo_upper.uwXStart + DUAL_RIGHT_FLAG*512;
		tOsdImgInfo_upper.uwYStart = tOsdImgInfo_left.uwYStart + tOsdImgInfo_left.uwVSize*1/3;
		tOSD_Img1(&tOsdImgInfo_upper, OSD_QUEUE);
		//mid lower
		tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_PARKINGLINE_UPPER1 + tUI_CuSetting.ParkingLineSize[ubCamNum], 1, &tOsdImgInfo_upper);
		tOsdImgInfo_upper.uwHSize = tOsdImgInfo_upper.uwHSize/2;
		tOsdImgInfo_upper.uwVSize = 3;
		tOsdImgInfo_upper.uwXStart = tOsdImgInfo_left.uwXStart + tOsdImgInfo_left.uwHSize ;
		//tOsdImgInfo_upper.uwXStart = tOsdImgInfo_upper.uwXStart + DUAL_RIGHT_FLAG*512;
		tOsdImgInfo_upper.uwYStart = tOsdImgInfo_left.uwYStart + tOsdImgInfo_left.uwVSize*2/3;
		tOSD_Img1(&tOsdImgInfo_upper, OSD_QUEUE);
		//lower
		tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_PARKINGLINE_LOWER1 + tUI_CuSetting.ParkingLineSize[ubCamNum], 1, &tOsdImgInfo_lower);
		tOsdImgInfo_lower.uwHSize = tOsdImgInfo_lower.uwHSize/2;
		tOsdImgInfo_lower.uwVSize = 3;
		tOsdImgInfo_lower.uwXStart = tOsdImgInfo_left.uwXStart + tOsdImgInfo_left.uwHSize ;
		//tOsdImgInfo_lower.uwXStart = tOsdImgInfo_lower.uwXStart + DUAL_RIGHT_FLAG*512;
		tOsdImgInfo_lower.uwYStart = tOsdImgInfo_left.uwYStart + tOsdImgInfo_left.uwVSize - tOsdImgInfo_lower.uwVSize; 
		tOSD_Img1(&tOsdImgInfo_lower, OSD_QUEUE);
		//right
		tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_PARKINGLINE_RIGHT_QUAL, 1, &tOsdImgInfo_right);
		//tOsdImgInfo_right.uwHSize = tOsdImgInfo_right.uwHSize/2;
		tOsdImgInfo_right.uwXStart = tOsdImgInfo_left.uwXStart + tOsdImgInfo_left.uwHSize + tOsdImgInfo_upper.uwHSize;
		//tOsdImgInfo_right.uwXStart = tOsdImgInfo_right.uwXStart + DUAL_RIGHT_FLAG*512;
		tOsdImgInfo_right.uwYStart = tOsdImgInfo_left.uwYStart;
		tOSD_Img1(&tOsdImgInfo_right, tMode);
		ubIsShow = 1;
	}
	//left_lower
	if(tUI_CuSetting.tParkingLineEnable[tCamViewSel.tCamViewPool[2]] == TRUE)
	{
		DUAL_RIGHT_FLAG = 0;
		DUAL_LOWER_FLAG = 1;
		ubCamNum = tCamViewSel.tCamViewPool[2];
		//left
		tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_PARKINGLINE_LEFT_QUAL, 1, &tOsdImgInfo_left);
		//tOsdImgInfo_left.uwHSize = tOsdImgInfo_left.uwHSize/2;
		tOsdImgInfo_left.uwXStart = (tUI_CuSetting.ParkingLine_XY[ubCamNum][0] << 8) + tUI_CuSetting.ParkingLine_XY[ubCamNum][1];
		tOsdImgInfo_left.uwXStart = tOsdImgInfo_left.uwXStart/2 + DUAL_RIGHT_FLAG*512;
		tOsdImgInfo_left.uwYStart = (tUI_CuSetting.ParkingLine_XY[ubCamNum][2] << 8) + tUI_CuSetting.ParkingLine_XY[ubCamNum][3];
		tOsdImgInfo_left.uwYStart = tOsdImgInfo_left.uwYStart/2 + DUAL_LOWER_FLAG*300;
		tOSD_Img1(&tOsdImgInfo_left, OSD_QUEUE);
		//upper
		tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_PARKINGLINE_UPPER1 + tUI_CuSetting.ParkingLineSize[ubCamNum], 1, &tOsdImgInfo_upper);
		tOsdImgInfo_upper.uwHSize = tOsdImgInfo_upper.uwHSize/2;
		tOsdImgInfo_upper.uwVSize = 3;
		tOsdImgInfo_upper.uwXStart = tOsdImgInfo_left.uwXStart + tOsdImgInfo_left.uwHSize;
		//tOsdImgInfo_upper.uwXStart = tOsdImgInfo_upper.uwXStart + DUAL_RIGHT_FLAG*512;
		tOsdImgInfo_upper.uwYStart = tOsdImgInfo_left.uwYStart;
		tOSD_Img1(&tOsdImgInfo_upper, OSD_QUEUE);
		//mid upper
		tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_PARKINGLINE_UPPER1 + tUI_CuSetting.ParkingLineSize[ubCamNum], 1, &tOsdImgInfo_upper);
		tOsdImgInfo_upper.uwHSize = tOsdImgInfo_upper.uwHSize/2;
		tOsdImgInfo_upper.uwVSize = 3;
		tOsdImgInfo_upper.uwXStart = tOsdImgInfo_left.uwXStart + tOsdImgInfo_left.uwHSize ;
		//tOsdImgInfo_upper.uwXStart = tOsdImgInfo_upper.uwXStart + DUAL_RIGHT_FLAG*512;
		tOsdImgInfo_upper.uwYStart = tOsdImgInfo_left.uwYStart + tOsdImgInfo_left.uwVSize*1/3;
		tOSD_Img1(&tOsdImgInfo_upper, OSD_QUEUE);
		//mid lower
		tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_PARKINGLINE_UPPER1 + tUI_CuSetting.ParkingLineSize[ubCamNum], 1, &tOsdImgInfo_upper);
		tOsdImgInfo_upper.uwHSize = tOsdImgInfo_upper.uwHSize/2;
		tOsdImgInfo_upper.uwVSize = 3;
		tOsdImgInfo_upper.uwXStart = tOsdImgInfo_left.uwXStart + tOsdImgInfo_left.uwHSize ;
		//tOsdImgInfo_upper.uwXStart = tOsdImgInfo_upper.uwXStart + DUAL_RIGHT_FLAG*512;
		tOsdImgInfo_upper.uwYStart = tOsdImgInfo_left.uwYStart + tOsdImgInfo_left.uwVSize*2/3;
		tOSD_Img1(&tOsdImgInfo_upper, OSD_QUEUE);
		//lower
		tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_PARKINGLINE_LOWER1 + tUI_CuSetting.ParkingLineSize[ubCamNum], 1, &tOsdImgInfo_lower);
		tOsdImgInfo_lower.uwHSize = tOsdImgInfo_lower.uwHSize/2;
		tOsdImgInfo_lower.uwVSize = 3;
		tOsdImgInfo_lower.uwXStart = tOsdImgInfo_left.uwXStart + tOsdImgInfo_left.uwHSize ;
		//tOsdImgInfo_lower.uwXStart = tOsdImgInfo_lower.uwXStart + DUAL_RIGHT_FLAG*512;
		tOsdImgInfo_lower.uwYStart = tOsdImgInfo_left.uwYStart + tOsdImgInfo_left.uwVSize - tOsdImgInfo_lower.uwVSize; 
		tOSD_Img1(&tOsdImgInfo_lower, OSD_QUEUE);
		//right
		tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_PARKINGLINE_RIGHT_QUAL, 1, &tOsdImgInfo_right);
		//tOsdImgInfo_right.uwHSize = tOsdImgInfo_right.uwHSize/2;
		tOsdImgInfo_right.uwXStart = tOsdImgInfo_left.uwXStart + tOsdImgInfo_left.uwHSize + tOsdImgInfo_upper.uwHSize;
		//tOsdImgInfo_right.uwXStart = tOsdImgInfo_right.uwXStart + DUAL_RIGHT_FLAG*512;
		tOsdImgInfo_right.uwYStart = tOsdImgInfo_left.uwYStart;
		tOSD_Img1(&tOsdImgInfo_right, tMode);
		ubIsShow = 1;
	}
	//right_lower
	if(tUI_CuSetting.tParkingLineEnable[tCamViewSel.tCamViewPool[3]] == TRUE)
	{
		DUAL_RIGHT_FLAG = 1;
		DUAL_LOWER_FLAG = 1;
		ubCamNum = tCamViewSel.tCamViewPool[3];
		//left
		tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_PARKINGLINE_LEFT_QUAL, 1, &tOsdImgInfo_left);
		//tOsdImgInfo_left.uwHSize = tOsdImgInfo_left.uwHSize/2;
		tOsdImgInfo_left.uwXStart = (tUI_CuSetting.ParkingLine_XY[ubCamNum][0] << 8) + tUI_CuSetting.ParkingLine_XY[ubCamNum][1];
		tOsdImgInfo_left.uwXStart = tOsdImgInfo_left.uwXStart/2 + DUAL_RIGHT_FLAG*512;
		tOsdImgInfo_left.uwYStart = (tUI_CuSetting.ParkingLine_XY[ubCamNum][2] << 8) + tUI_CuSetting.ParkingLine_XY[ubCamNum][3];
		tOsdImgInfo_left.uwYStart = tOsdImgInfo_left.uwYStart/2 + DUAL_LOWER_FLAG*300;
		tOSD_Img1(&tOsdImgInfo_left, OSD_QUEUE);
		//upper
		tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_PARKINGLINE_UPPER1 + tUI_CuSetting.ParkingLineSize[ubCamNum], 1, &tOsdImgInfo_upper);
		tOsdImgInfo_upper.uwHSize = tOsdImgInfo_upper.uwHSize/2;
		tOsdImgInfo_upper.uwVSize = 3;
		tOsdImgInfo_upper.uwXStart = tOsdImgInfo_left.uwXStart + tOsdImgInfo_left.uwHSize;
		//tOsdImgInfo_upper.uwXStart = tOsdImgInfo_upper.uwXStart + DUAL_RIGHT_FLAG*512;
		tOsdImgInfo_upper.uwYStart = tOsdImgInfo_left.uwYStart;
		tOSD_Img1(&tOsdImgInfo_upper, OSD_QUEUE);
		//mid upper
		tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_PARKINGLINE_UPPER1 + tUI_CuSetting.ParkingLineSize[ubCamNum], 1, &tOsdImgInfo_upper);
		tOsdImgInfo_upper.uwHSize = tOsdImgInfo_upper.uwHSize/2;
		tOsdImgInfo_upper.uwVSize = 3;
		tOsdImgInfo_upper.uwXStart = tOsdImgInfo_left.uwXStart + tOsdImgInfo_left.uwHSize ;
		//tOsdImgInfo_upper.uwXStart = tOsdImgInfo_upper.uwXStart + DUAL_RIGHT_FLAG*512;
		tOsdImgInfo_upper.uwYStart = tOsdImgInfo_left.uwYStart + tOsdImgInfo_left.uwVSize*1/3;
		tOSD_Img1(&tOsdImgInfo_upper, OSD_QUEUE);
		//mid lower
		tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_PARKINGLINE_UPPER1 + tUI_CuSetting.ParkingLineSize[ubCamNum], 1, &tOsdImgInfo_upper);
		tOsdImgInfo_upper.uwHSize = tOsdImgInfo_upper.uwHSize/2;
		tOsdImgInfo_upper.uwVSize = 3;
		tOsdImgInfo_upper.uwXStart = tOsdImgInfo_left.uwXStart + tOsdImgInfo_left.uwHSize ;
		//tOsdImgInfo_upper.uwXStart = tOsdImgInfo_upper.uwXStart + DUAL_RIGHT_FLAG*512;
		tOsdImgInfo_upper.uwYStart = tOsdImgInfo_left.uwYStart + tOsdImgInfo_left.uwVSize*2/3;
		tOSD_Img1(&tOsdImgInfo_upper, OSD_QUEUE);
		//lower
		tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_PARKINGLINE_LOWER1 + tUI_CuSetting.ParkingLineSize[ubCamNum], 1, &tOsdImgInfo_lower);
		tOsdImgInfo_lower.uwHSize = tOsdImgInfo_lower.uwHSize/2;
		tOsdImgInfo_lower.uwVSize = 3;
		tOsdImgInfo_lower.uwXStart = tOsdImgInfo_left.uwXStart + tOsdImgInfo_left.uwHSize ;
		//tOsdImgInfo_lower.uwXStart = tOsdImgInfo_lower.uwXStart + DUAL_RIGHT_FLAG*512;
		tOsdImgInfo_lower.uwYStart = tOsdImgInfo_left.uwYStart + tOsdImgInfo_left.uwVSize - tOsdImgInfo_lower.uwVSize; 
		tOSD_Img1(&tOsdImgInfo_lower, OSD_QUEUE);
		//right
		tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_PARKINGLINE_RIGHT_QUAL, 1, &tOsdImgInfo_right);
		//tOsdImgInfo_right.uwHSize = tOsdImgInfo_right.uwHSize/2;
		tOsdImgInfo_right.uwXStart = tOsdImgInfo_left.uwXStart + tOsdImgInfo_left.uwHSize + tOsdImgInfo_upper.uwHSize;
		//tOsdImgInfo_right.uwXStart = tOsdImgInfo_right.uwXStart + DUAL_RIGHT_FLAG*512;
		tOsdImgInfo_right.uwYStart = tOsdImgInfo_left.uwYStart;
		tOSD_Img1(&tOsdImgInfo_right, tMode);
		ubIsShow = 1;


	}		
#endif
}

//------------------------------------------------------------------------------

void UI_SwitchViewType(UI_CamNum_t tUI_CamSel,uint8_t ubClosePanel)
{
	isDrawNosignal[0] = 0;
	isDrawNosignal[1] = 0;
	isDrawNosignal[2] = 0;
	isDrawNosignal[3] = 0;
	

	if(!ubUI_CuPowerDiscFlag)
	{
		ubUI_CuSemaphorewaitFlag = 1;
		//printf("UI_SwitchViewType osUI_UpdateStatusCtr  wait 111111111111 \n");
		osSemaphoreWait(osUI_UpdateStatusCtr, osWaitForever);
		//printf("UI_SwitchViewType osUI_CuTriggerCtr\n");
		osSemaphoreWait(osUI_CuTriggerCtr, osWaitForever);
	}
	
	//printf("UI_SwitchViewType tUI_CamSel = %d\n", tUI_CamSel);
	ubUI_FinishViewSwitch = FALSE;
	UI_CamViewType_t tCamViewTypeSel;
	if(ubClosePanel)
	{
		ubUI_ReOpenPanel = TRUE;
		UI_PanelOff();
	}
	
	if(UI_REC_START == tUI_RecPlayAct.tRecAct)
	{
		UI_VideoRecordingExec(UI_REC_STOP);
		ubUI_RestartRec = TRUE;
	}

	//H�����λ�����û��ǰ���ԭ�������÷�����ԭ���Ĵ���һ��ʼ�ǲ�֧��dual�����quad�����λ�����ã��Ǻ����Լ��ӵ�?
	if(tUI_CamSel <= CAM4)
	{
		tCamViewSel.tCamViewPool[0] = tUI_CamSel;

	}
	else if(tUI_CamSel == DUALVIEW_ITEM)//dual
	{
		for(int i = 0;i < 4;i++)
		{
			if(tUI_CamStatus[i].tCamDispLocation_Dual == DISP_LEFT)
				tCamViewSel.tCamViewPool[0] = (UI_CamNum_t)i;			//tCamViewSel.tCamViewPool[0]�����dual_left��tx���?
			if(tUI_CamStatus[i].tCamDispLocation_Dual == DISP_RIGHT)
				tCamViewSel.tCamViewPool[1] = (UI_CamNum_t)i;			//tCamViewSel.tCamViewPool[1]�����dual_right��tx���?
		}
	}
	else if(tUI_CamSel == QUALVIEW_ITEM)//quad
	{
		for(int i = 0;i < 4;i++)
		{
			if(tUI_CamStatus[i].tCamDispLocation_Quad == DISP_UPPER_LEFT)
				tCamViewSel.tCamViewPool[0] = (UI_CamNum_t)i;			//tCamViewSel.tCamViewPool[0]�����DISP_UPPER_LEFT��tx���?
			if(tUI_CamStatus[i].tCamDispLocation_Quad == DISP_UPPER_RIGHT)
				tCamViewSel.tCamViewPool[1] = (UI_CamNum_t)i;			//tCamViewSel.tCamViewPool[1]�����DISP_UPPER_RIGHT��tx���?
			if(tUI_CamStatus[i].tCamDispLocation_Quad == DISP_LOWER_LEFT)
				tCamViewSel.tCamViewPool[2] = (UI_CamNum_t)i;			//tCamViewSel.tCamViewPool[1]�����DISP_LOWER_LEFT��tx���?
			if(tUI_CamStatus[i].tCamDispLocation_Quad == DISP_LOWER_RIGHT)
				tCamViewSel.tCamViewPool[3] = (UI_CamNum_t)i;			//tCamViewSel.tCamViewPool[1]�����DISP_LOWER_RIGHT��tx���?
		}
	}
	
	if(tUI_CamSel <= CAM4)
		tCamViewTypeSel = SINGLE_VIEW;
	else if(tUI_CamSel == DUALVIEW_ITEM)
		tCamViewTypeSel = DUAL_VIEW;
	else if(tUI_CamSel == QUALVIEW_ITEM)
		tCamViewTypeSel = QUAD_VIEW;
	
	tCamViewSel.tCamViewType = tCamViewTypeSel;
	
	UI_SwitchCameraSource();
	//�л���Ƶ
	UI_SwitchAudioSource(UI_GetAudioSrc());

	if(!ubUI_CuStandbyFlag || (ubUI_CuStandbyFlag == TRUE && TriggerLock == TRUE))
	{
		//osDelay(100);
		UI_AudioOn();
	}

}


void UI_TriggerEventExec(void *pvTriggerEvent)
{
	//printf("UI_TriggerEventExec\n");
	TRIGGER_EVENT_t * ptTriggerEvent = (TRIGGER_EVENT_t*)pvTriggerEvent;
	UI_CamNum_t tUI_ViewSel = ptTriggerEvent->tUI_ViewSel;
	OSD_IMG_INFO tOsdImgInfo;
	
	DeskTopShowView = tUI_ViewSel;
	//printf("*************Trigger/Disconnect Switch start\n");	
	
	if(!TriggerLock && ubUI_CuStandbyFlag)
		UI_DisconnectTx();
	else
		UI_SwitchViewType(tUI_ViewSel,TRUE);

	//printf("************Trigger/Disconnect Switch over\n");
	UI_ClearOsdImageNoUpdate();
	//ubDrawBSDRange = 0;
	UI_DrawDesktopIcon();
	//ubDrawBSDRange = 1;
	//�������?
	if(TriggerLock && tUI_ViewSel <= CAM4)
	{
		if(tUI_CuSetting.tParkingLineEnable[tUI_ViewSel] == PARKING_LINE_AUTO && tUI_CuSetting.ubIsEnableBSDRANGE[tUI_ViewSel] == 0)
		{
//			tOSD_GetOsdImgInfor(1, OSD_IMG1, OSD1IMG_BSD_RANGE_SINGLE, 1, &tOsdImgInfo);
//			tOSD_Img1(&tOsdImgInfo, OSD_UPDATE);	
			
			UI_DrawParkingLine_OSD1(tUI_ViewSel,OSD_UPDATE);
			if(!tUI_CuSetting.ubHideChannelName)
			{
				tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DESKTOP_CAM1_WORD + tUI_ViewSel, 1, &tOsdImgInfo);
				tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
			}
		}
	}
	
}

//------------------------------------------------------------------------------
void UI_EngModeKey(void)
{
}
//------------------------------------------------------------------------------
void UI_EngModeCtrl(UI_ArrowKey_t tArrowKey)
{
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
			tUI_State = UI_DISPLAY_STATE;
			break;
		}
		default:
			if(UI_EngFuncPrt[tArrowKey])
				UI_EngFuncPrt[tArrowKey]();
			break;
	}
}
//------------------------------------------------------------------------------
void UI_DisplayAppPairingScreen(void)
{
	OSD_IMG_INFO tOsdImgInfo;

	ubUI_StopUpdateStsBarFlag = TRUE;
	UI_ClearOsdImage();
	tUI_State = UI_PAIRING_STATE;
}
//------------------------------------------------------------------------------
void UI_PairingControl(UI_ArrowKey_t tArrowKey)
{
	APP_EventMsg_t tUI_PairMessage = {0};

	tUI_PairMessage.ubAPP_Event = APP_PAIRING_STOP_EVENT;
	UI_SendMessageToAPP(&tUI_PairMessage);
}

void UI_CheckIsPairing(void)
{
	if (APP_PAIRING_STATE == tUI_SyncAppState)
	{
		APP_EventMsg_t tUI_PairMessage = {0};

		tUI_PairMessage.ubAPP_Event = APP_PAIRING_STOP_EVENT;
		UI_SendMessageToAPP(&tUI_PairMessage);

		osDelay(500);
	}
}
//------------------------------------------------------------------------------
void UI_FwUpgViaSdCard(void)
{
	OSD_IMG_INFO tOsdImgInfo[5];

	if(UI_DISPLAY_STATE != tUI_State)
		return;
/*
	if(tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_FWUSDUPGDIAG_ICON, 5, &tOsdImgInfo[0]) != OSD_OK)
	{
		printd(DBG_ErrorLvl, "Load OSD Image FAIL, pls check (%d) !\n", __LINE__);
		return;
	}
	UI_ClearOsdImage();
	OSD_Weight(OSD_WEIGHT_8DIV8);
	tOSD_Img2(&tOsdImgInfo[0], OSD_QUEUE);
	tOSD_Img2(&tOsdImgInfo[1], OSD_QUEUE);
	tOSD_Img2(&tOsdImgInfo[4], OSD_UPDATE);
	tUI_State = UI_SDFWUPG_STATE;
*/

}

void UI_MDTriggerSetting(void)
{
	//#define MD_H_WINDOWSIZE		48
	//#define MD_V_WINDOWSIZE		64
	#define MD_H_WINDOWSIZE		64	// 1024/64=16
	#define MD_V_WINDOWSIZE		48	// 600/40=15

	uint32_t ulLcd_HSize = uwLCD_GetLcdHoSize();
	uint32_t ulLcd_VSize = uwLCD_GetLcdVoSize();
	//uint8_t ubMD_V_WinNum = ulLcd_VSize / MD_V_WINDOWSIZE;
	//uint8_t ubMD_H_WinNum = ulLcd_HSize / MD_H_WINDOWSIZE;
	uint16_t uwMD_StartIdx = 2;
	uint8_t ubMD_RightCnt = 2;
	uint8_t ubMD_DownCnt = 2;
	
	{
		UI_CUReqCmd_t tMdCmd;
		
		printd(DBG_ErrorLvl, "UI_MD_SETTING (%04X) - (%d, %d)\n", uwMD_StartIdx, ubMD_RightCnt, ubMD_DownCnt);

		tMdCmd.tDS_CamNum 				= tCamViewSel.tCamViewPool[0];
		tMdCmd.ubCmd[UI_TWC_TYPE]		= UI_SETTING;
		tMdCmd.ubCmd[UI_SETTING_ITEM]   = UI_MD_SETTING;
		tMdCmd.ubCmd[UI_SETTING_DATA]   = (uwMD_StartIdx & 0xFF);
		tMdCmd.ubCmd[UI_SETTING_DATA+1] = (uwMD_StartIdx >> 8);
		tMdCmd.ubCmd[UI_SETTING_DATA+2] = ubMD_RightCnt;
		tMdCmd.ubCmd[UI_SETTING_DATA+3] = ubMD_DownCnt;
		tMdCmd.ubCmd_Len  				= 6;
		
		if (UI_SendRequestToCAM(osThreadGetId(), &tMdCmd) != rUI_SUCCESS)
		{
			printd(DBG_InfoLvl, ("%s: MD Setting Fail !\n"), __func__);
		}
	}
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

		tOsdImgInfo.uwHSize  = 152;
		tOsdImgInfo.uwVSize  = 100;
		tOsdImgInfo.uwXStart = uwOSD_GetHSize() - tOsdImgInfo.uwHSize;
		tOsdImgInfo.uwYStart = 80;
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

		tOsdImgInfo.uwXStart = 315;
		tOsdImgInfo.uwYStart = 170;
		tOsdImgInfo.uwHSize  = 370;
		tOsdImgInfo.uwVSize  = 108;
		OSD_EraserImg2(&tOsdImgInfo);
		tUI_State = UI_DISPLAY_STATE;
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
				UI_ClearOsdImage();
				osDelay(100);
			}
			tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_PHOTOCAPNOCARD_ICON, 1, &tOsdImgInfo);
			tOsdImgInfo.uwXStart = uwOSD_GetHSize() - tOsdImgInfo.uwHSize;
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
			printf("-------------------------UI_VideoRecordingStsRpt KNL_ErrorFsFmt\n");
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
	osStatus osUI_EventStauts;

	if((TRUE == ubUI_VdoRecChkFlag) &&
	   ((tUI_RecPlayAct.tRecAct == tRecAct) || (UI_PHOTOCAP_MODE == tUI_CuSetting.tVdoMode)))
	{
		return;
	}

	tUI_RecAct.tRecordFunc	   = ((REC_LOOPING == tUI_CuSetting.RecInfo.tREC_Mode) && (UI_REC_START == tRecAct))?KNL_RECORDFUNC_LOOP:KNL_RECORDFUNC_MANU;
	tUI_RecAct.pRecordStsNtyCb = UI_VideoRecordingStsRpt;
	tUI_RecAct.ubRecOnceStopCBFlag = NULL;
	
	osSemaphoreWait(osUI_CuRecCtr, osWaitForever);
	tUI_RecSts = tKNL_ExecRecordFunc(tUI_RecAct);
	
	if(KNL_ERR != tUI_RecSts)
	{
		tUI_RecPlayAct.tRecAct = tRecAct;
		if(UI_REC_STOP == tUI_RecPlayAct.tRecAct)
		{
			uint8_t ubRecRpt;
			osMessageReset(osUI_RecRptQueue);
			osUI_EventStauts = osMessageGet(osUI_RecRptQueue, &ubRecRpt, 10000);
			if(osUI_EventStauts == osEventTimeout)
			{
				printf("Record Stop TimeOut\n");
				SYS_Reboot();
			}
		}
	}
	ubUI_VdoRecChkFlag = TRUE;
	ulStartRecCompleteCount = 500;
	
}

//------------------------------------------------------------------------------
//! Performance Debug
void UI_OsdDisplayFrmErrItem(UI_DisplayLocation_t tDispLoc)
{
#define DBGDISP_ICON_OFFSET 20
	uint16_t uwLcd_HSize  = uwOSD_GetHSize();
	uint16_t uwLcd_VSize  = uwOSD_GetVSize();
	uint16_t uwXOffset[11] = {[DISP_UPPER_LEFT] = 0,               [DISP_UPPER_RIGHT] = (uwLcd_HSize/2),
	                          [DISP_LOWER_LEFT] = 0, 			   [DISP_LOWER_RIGHT] = (uwLcd_HSize/2),
							  [DISP_LEFT] 	    = 0,			   [DISP_RIGHT] 	  = (uwLcd_HSize/2),
							  [DISP_H_L] 	    = 0, 			   [DISP_H_R] 		  = UI_HVIEW_ICON_XOFFSET,
							  [DISP_H_CU] 	    = 300, 		       [DISP_H_CL] 		  = 300};
	uint16_t uwYOffset[11] = {[DISP_UPPER_LEFT] = 0,               [DISP_UPPER_RIGHT] = 0,
	                          [DISP_LOWER_LEFT] = (uwLcd_VSize/2), [DISP_LOWER_RIGHT] = (uwLcd_VSize/2),
							  [DISP_LEFT] 	    = 0, 		       [DISP_RIGHT] 	  = 0,
							  [DISP_H_L] 	    = 0,  			   [DISP_H_R] 		  = 0,
							  [DISP_H_CU] 	    = 0, 		       [DISP_H_CL] 		  = (uwLcd_VSize/2)};
	uint8_t ubDbgItem;

	for(ubDbgItem = 0; ubDbgItem < MAX_DBG_ITEM; ubDbgItem++)
	{
		tUI_PerDbgOsdImgInfo[11+ubDbgItem].uwXStart = uwXOffset[tDispLoc];
		tUI_PerDbgOsdImgInfo[11+ubDbgItem].uwYStart = 150 + (DBGDISP_ICON_OFFSET * ubDbgItem) + uwYOffset[tDispLoc];
		tOSD_Img2(&tUI_PerDbgOsdImgInfo[11+ubDbgItem], OSD_QUEUE);
		tUI_PerDbgOsdImgInfo[11+ubDbgItem].uwXStart = 0;
		tUI_PerDbgOsdImgInfo[11+ubDbgItem].uwYStart = 0;
	}
}
//------------------------------------------------------------------------------
void UI_OsdDisplayFrameSeqFunc(uint8_t ubCamNum, uint16_t uwFrameSeq, uint16_t uwXPos, uint16_t uwYPos)
{
	uint8_t ubTen = 0, ubUnit = 0, ubHun = 0, ubThus = 0;
	uint16_t uwXStart = 0;

	if(UI_DISPLAY_STATE != tUI_State)
		return;
	osMutexWait(osUI_PerDbgMutex, osWaitForever);
	ubThus =  uwFrameSeq / 1000;
	ubHun  = (uwFrameSeq - (ubThus * 1000)) / 100;
	ubTen  = (uwFrameSeq - ((ubThus * 1000) + (ubHun * 100))) / 10;
	ubUnit = (uwFrameSeq - ((ubThus * 1000) + (ubHun * 100) + (ubTen * 10)));
	tUI_PerDbgOsdImgInfo[ubThus].uwXStart  = uwXPos;
	tUI_PerDbgOsdImgInfo[ubThus].uwYStart  = 100 + uwYPos;
	tOSD_Img2(&tUI_PerDbgOsdImgInfo[ubThus], OSD_QUEUE);
	uwXStart = tUI_PerDbgOsdImgInfo[ubThus].uwXStart;
	tUI_PerDbgOsdImgInfo[ubThus].uwXStart  = 0;
	tUI_PerDbgOsdImgInfo[ubThus].uwYStart  = 0;
	tUI_PerDbgOsdImgInfo[ubHun].uwXStart  = uwXStart + tUI_PerDbgOsdImgInfo[ubThus].uwHSize;
	tUI_PerDbgOsdImgInfo[ubHun].uwYStart  = 100 + uwYPos;
	tOSD_Img2(&tUI_PerDbgOsdImgInfo[ubHun], OSD_QUEUE);
	uwXStart = tUI_PerDbgOsdImgInfo[ubHun].uwXStart;
	tUI_PerDbgOsdImgInfo[ubHun].uwXStart   = 0;
	tUI_PerDbgOsdImgInfo[ubHun].uwYStart   = 0;
	tUI_PerDbgOsdImgInfo[ubTen].uwXStart  = uwXStart + tUI_PerDbgOsdImgInfo[ubHun].uwHSize;
	tUI_PerDbgOsdImgInfo[ubTen].uwYStart  = 100 + uwYPos;
	tOSD_Img2(&tUI_PerDbgOsdImgInfo[ubTen], OSD_QUEUE);
	uwXStart = tUI_PerDbgOsdImgInfo[ubTen].uwXStart;
	tUI_PerDbgOsdImgInfo[ubTen].uwXStart   = 0;
	tUI_PerDbgOsdImgInfo[ubTen].uwYStart   = 0;
	tUI_PerDbgOsdImgInfo[ubUnit].uwXStart =  uwXStart + tUI_PerDbgOsdImgInfo[ubTen].uwHSize;
	tUI_PerDbgOsdImgInfo[ubUnit].uwYStart = 100 + uwYPos;
	tOSD_Img2(&tUI_PerDbgOsdImgInfo[ubUnit], OSD_UPDATE);
	tUI_PerDbgOsdImgInfo[ubUnit].uwXStart  = 0;
	tUI_PerDbgOsdImgInfo[ubUnit].uwYStart  = 0;
	osMutexRelease(osUI_PerDbgMutex);
}
//------------------------------------------------------------------------------
void UI_OsdDisplayFpsFunc(uint8_t ubCamNum, uint16_t uwFps, uint16_t uwXPos, uint16_t uwYPos)
{
	uint8_t ubTen = 0, ubUnit = 0;
	uint16_t uwXStart = 0;

	if(UI_DISPLAY_STATE != tUI_State)
		return;
	osMutexWait(osUI_PerDbgMutex, osWaitForever);
	tUI_PerDbgOsdImgInfo[10].uwXStart = 60 + uwXPos;
	tUI_PerDbgOsdImgInfo[10].uwYStart = 100 + uwYPos;
	tOSD_Img2(&tUI_PerDbgOsdImgInfo[10], OSD_QUEUE);
	uwXStart = tUI_PerDbgOsdImgInfo[10].uwXStart;
	tUI_PerDbgOsdImgInfo[10].uwXStart  	  = 0;
	tUI_PerDbgOsdImgInfo[10].uwYStart  	  = 0;
	ubTen  = uwFps / 10;
	ubUnit = uwFps - (ubTen * 10);
	tUI_PerDbgOsdImgInfo[ubTen].uwXStart  = uwXStart + tUI_PerDbgOsdImgInfo[10].uwHSize;
	tUI_PerDbgOsdImgInfo[ubTen].uwYStart  = 100 + uwYPos;
	tOSD_Img2(&tUI_PerDbgOsdImgInfo[ubTen], OSD_QUEUE);
	uwXStart = tUI_PerDbgOsdImgInfo[ubTen].uwXStart;
	tUI_PerDbgOsdImgInfo[ubTen].uwXStart  = 0;
	tUI_PerDbgOsdImgInfo[ubTen].uwYStart  = 0;
	tUI_PerDbgOsdImgInfo[ubUnit].uwXStart = uwXStart + tUI_PerDbgOsdImgInfo[ubTen].uwHSize;
	tUI_PerDbgOsdImgInfo[ubUnit].uwYStart = 100 + uwYPos;
	tOSD_Img2(&tUI_PerDbgOsdImgInfo[ubUnit], OSD_UPDATE);
	tUI_PerDbgOsdImgInfo[ubUnit].uwXStart = 0;
	tUI_PerDbgOsdImgInfo[ubUnit].uwYStart = 0;
	osMutexRelease(osUI_PerDbgMutex);
}
//------------------------------------------------------------------------------
void UI_OsdDisplayRfBwFunc(uint8_t ubCamNum, uint16_t uwBw, uint16_t uwXPos, uint16_t uwYPos)
{
	uint8_t ubHun = 0, ubTen = 0, ubUnit = 0;
	uint16_t uwXStart = 0;

	if(UI_DISPLAY_STATE != tUI_State)
		return;
	osMutexWait(osUI_PerDbgMutex, osWaitForever);
	tUI_PerDbgOsdImgInfo[10].uwXStart = uwXPos;
	tUI_PerDbgOsdImgInfo[10].uwYStart = 125 + uwYPos;
	tOSD_Img2(&tUI_PerDbgOsdImgInfo[10], OSD_QUEUE);
	uwXStart = tUI_PerDbgOsdImgInfo[10].uwXStart;
	tUI_PerDbgOsdImgInfo[10].uwXStart  	  = 0;
	tUI_PerDbgOsdImgInfo[10].uwYStart  	  = 0;
	ubHun  = uwBw / 100;
	ubTen  = (uwBw -  (ubHun* 100)) / 10;
	ubUnit = (uwBw - ((ubHun * 100) + (ubTen * 10)));
	tUI_PerDbgOsdImgInfo[ubHun].uwXStart  = uwXStart + tUI_PerDbgOsdImgInfo[10].uwHSize;
	tUI_PerDbgOsdImgInfo[ubHun].uwYStart  = 125 + uwYPos;
	tOSD_Img2(&tUI_PerDbgOsdImgInfo[ubHun], OSD_QUEUE);
	uwXStart = tUI_PerDbgOsdImgInfo[ubHun].uwXStart;
	tUI_PerDbgOsdImgInfo[ubHun].uwXStart  = 0;
	tUI_PerDbgOsdImgInfo[ubHun].uwYStart  = 0;
	tUI_PerDbgOsdImgInfo[ubTen].uwXStart  = uwXStart + tUI_PerDbgOsdImgInfo[ubHun].uwHSize;
	tUI_PerDbgOsdImgInfo[ubTen].uwYStart  = 125 + uwYPos;
	tOSD_Img2(&tUI_PerDbgOsdImgInfo[ubTen], OSD_QUEUE);
	uwXStart = tUI_PerDbgOsdImgInfo[ubTen].uwXStart;
	tUI_PerDbgOsdImgInfo[ubTen].uwXStart  = 0;
	tUI_PerDbgOsdImgInfo[ubTen].uwYStart  = 0;
	tUI_PerDbgOsdImgInfo[ubUnit].uwXStart = uwXStart + tUI_PerDbgOsdImgInfo[ubTen].uwHSize;
	tUI_PerDbgOsdImgInfo[ubUnit].uwYStart = 125 + uwYPos;
	tOSD_Img2(&tUI_PerDbgOsdImgInfo[ubUnit], OSD_UPDATE);
	tUI_PerDbgOsdImgInfo[ubUnit].uwXStart = 0;
	tUI_PerDbgOsdImgInfo[ubUnit].uwYStart = 0;
	osMutexRelease(osUI_PerDbgMutex);
}
//------------------------------------------------------------------------------
void UI_OsdDisplayFrameErrType(uint8_t ubCamNum, uint16_t uwErrorType, uint16_t uwXPos, uint16_t uwYPos)
{
	uint8_t ubTen = 0, ubUnit = 0, ubHun = 0, ubThus = 0;
	uint16_t uwXStart = 0;

	if(UI_DISPLAY_STATE != tUI_State)
		return;
	osMutexWait(osUI_PerDbgMutex, osWaitForever);
	ubThus = ulUI_FrameErrCnt[ubCamNum][uwErrorType] / 1000;
	ubHun  = (ulUI_FrameErrCnt[ubCamNum][uwErrorType] - (ubThus * 1000)) / 100;
	ubTen  = (ulUI_FrameErrCnt[ubCamNum][uwErrorType] - ((ubThus * 1000) + (ubHun * 100))) / 10;
	ubUnit = (ulUI_FrameErrCnt[ubCamNum][uwErrorType] - ((ubThus * 1000) + (ubHun * 100) + (ubTen * 10)));
	tUI_PerDbgOsdImgInfo[ubThus].uwXStart  = 20 + uwXPos;
	tUI_PerDbgOsdImgInfo[ubThus].uwYStart  = 150 + (DBGDISP_ICON_OFFSET * uwErrorType) + uwYPos;
	tOSD_Img2(&tUI_PerDbgOsdImgInfo[ubThus], OSD_QUEUE);
	uwXStart = tUI_PerDbgOsdImgInfo[ubThus].uwXStart;
	tUI_PerDbgOsdImgInfo[ubThus].uwXStart  = 0;
	tUI_PerDbgOsdImgInfo[ubThus].uwYStart  = 0;
	tUI_PerDbgOsdImgInfo[ubHun].uwXStart   = uwXStart + tUI_PerDbgOsdImgInfo[ubThus].uwHSize;
	tUI_PerDbgOsdImgInfo[ubHun].uwYStart   = 150 + (DBGDISP_ICON_OFFSET * uwErrorType) + uwYPos;
	tOSD_Img2(&tUI_PerDbgOsdImgInfo[ubHun], OSD_QUEUE);
	uwXStart = tUI_PerDbgOsdImgInfo[ubHun].uwXStart;
	tUI_PerDbgOsdImgInfo[ubHun].uwXStart   = 0;
	tUI_PerDbgOsdImgInfo[ubHun].uwYStart   = 0;
	tUI_PerDbgOsdImgInfo[ubTen].uwXStart   = uwXStart + tUI_PerDbgOsdImgInfo[ubHun].uwHSize;
	tUI_PerDbgOsdImgInfo[ubTen].uwYStart   = 150 + (DBGDISP_ICON_OFFSET * uwErrorType) + uwYPos;
	tOSD_Img2(&tUI_PerDbgOsdImgInfo[ubTen], OSD_QUEUE);
	uwXStart = tUI_PerDbgOsdImgInfo[ubTen].uwXStart;
	tUI_PerDbgOsdImgInfo[ubTen].uwXStart   = 0;
	tUI_PerDbgOsdImgInfo[ubTen].uwYStart   = 0;
	tUI_PerDbgOsdImgInfo[ubUnit].uwXStart  = uwXStart + tUI_PerDbgOsdImgInfo[ubTen].uwHSize;
	tUI_PerDbgOsdImgInfo[ubUnit].uwYStart  = 150 + (DBGDISP_ICON_OFFSET * uwErrorType) + uwYPos;
	tOSD_Img2(&tUI_PerDbgOsdImgInfo[ubUnit], OSD_UPDATE);
	tUI_PerDbgOsdImgInfo[ubUnit].uwXStart  = 0;
	tUI_PerDbgOsdImgInfo[ubUnit].uwYStart  = 0;
	osMutexRelease(osUI_PerDbgMutex);
}
//------------------------------------------------------------------------------
void UI_DisplayTrxInfo(UI_FuncExecMsg_t tPerRpt)
{
	static void (*pvUI_PerRptFunc[])(uint8_t, uint16_t, uint16_t, uint16_t) =
	{
		[PER_FRMSTS_RPT] = UI_OsdDisplayFrameErrType,
		[PER_FRMSEQ_RPT] = UI_OsdDisplayFrameSeqFunc,
		[PER_FPS_RPT] 	 = UI_OsdDisplayFpsFunc,
		[PER_RFBW_RPT] 	 = UI_OsdDisplayRfBwFunc,
	};
	uint16_t uwLcd_HSize  = uwOSD_GetHSize();
	uint16_t uwLcd_VSize  = uwOSD_GetVSize();
	uint16_t uwXOffset[11] = {[DISP_UPPER_LEFT] = 0,               [DISP_UPPER_RIGHT] = (uwLcd_HSize/2),
	                          [DISP_LOWER_LEFT] = 0, 			   [DISP_LOWER_RIGHT] = (uwLcd_HSize/2),
							  [DISP_LEFT] 	    = 0,			   [DISP_RIGHT] 	  = (uwLcd_HSize/2),
							  [DISP_H_L] 	    = 0, 			   [DISP_H_R] 		  = UI_HVIEW_ICON_XOFFSET,
							  [DISP_H_CU] 	    = 300, 		       [DISP_H_CL] 		  = 300};
	uint16_t uwYOffset[11] = {[DISP_UPPER_LEFT] = 0,               [DISP_UPPER_RIGHT] = 0,
	                          [DISP_LOWER_LEFT] = (uwLcd_VSize/2), [DISP_LOWER_RIGHT] = (uwLcd_VSize/2),
							  [DISP_LEFT] 	    = 0, 		       [DISP_RIGHT] 	  = 0,
							  [DISP_H_L] 	    = 0,  			   [DISP_H_R] 		  = 0,
							  [DISP_H_CU] 	    = 0, 		       [DISP_H_CL] 		  = (uwLcd_VSize/2)};
	UI_DisplayLocation_t tUI_DispLoc;
	UI_DisplayLocation_t t4TQuadDefuLoc[CAM_4T] = 
	{
		tUI_CamStatus[0].tCamDispLocation_Quad, 
		tUI_CamStatus[1].tCamDispLocation_Quad,
		tUI_CamStatus[2].tCamDispLocation_Quad,
		tUI_CamStatus[3].tCamDispLocation_Quad
	};

	if((SINGLE_VIEW == tCamViewSel.tCamViewType) && (((UI_CamNum_t)tPerRpt.ubCamNum != tCamViewSel.tCamViewPool[0])))
		return;
	if((VDO_DISP_TYPE == KNL_DISP_H) && (QUAD_VIEW == tCamViewSel.tCamViewType))
		tUI_DispLoc = t4TQuadDefuLoc[tPerRpt.ubCamNum];
	else
		tUI_DispLoc = (tCamViewSel.tCamViewType == SINGLE_VIEW)?DISP_UPPER_LEFT:
					  (tCamViewSel.tCamViewType == DUAL_VIEW)?((tPerRpt.ubCamNum == tCamViewSel.tCamViewPool[0])?DISP_LEFT:DISP_RIGHT):tUI_CamStatus[tPerRpt.ubCamNum].tCamDispLocation;
	if(pvUI_PerRptFunc[tPerRpt.ubType])
		pvUI_PerRptFunc[tPerRpt.ubType](tPerRpt.ubCamNum, tPerRpt.uwFuncMsg, uwXOffset[tUI_DispLoc], uwYOffset[tUI_DispLoc]);
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
void UI_ShowButtonValueNormal(uint16_t uwValue,uint16_t uwXStart,uint16_t uwYStart,OSD_UPDATE_TYP tUpdateMod)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint8_t ubUnits = 0, ubTens = 0, ubHunds = 0,ubThous = 0;
	uint8_t *ubVal[4] = {&ubUnits,&ubTens,&ubHunds,&ubThous};
	for(uint8_t i = 0;i < 4;i++)
	{
		*ubVal[i] = uwValue % 10;
		uwValue /= 10;
	}
	if(ubThous)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_BUTTON_NUM0_NOR+ubThous, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = uwXStart;
		tOsdImgInfo.uwYStart = uwYStart;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_BUTTON_NUM0_NOR+ubHunds, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = uwXStart + 20;
		tOsdImgInfo.uwYStart = uwYStart;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_BUTTON_NUM0_NOR+ubTens, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = uwXStart + 40;
		tOsdImgInfo.uwYStart = uwYStart;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_BUTTON_NUM0_NOR+ubUnits, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = uwXStart + 60;
		tOsdImgInfo.uwYStart = uwYStart;
		tOSD_Img2(&tOsdImgInfo, tUpdateMod);

	}
	else if(ubHunds)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_BUTTON_NUM0_NOR+ubHunds, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = uwXStart;
		tOsdImgInfo.uwYStart = uwYStart;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_BUTTON_NUM0_NOR+ubTens, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = uwXStart + 20;
		tOsdImgInfo.uwYStart = uwYStart;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_BUTTON_NUM0_NOR+ubUnits, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = uwXStart + 40;
		tOsdImgInfo.uwYStart = uwYStart;
		tOSD_Img2(&tOsdImgInfo, tUpdateMod);
	}
	else if(ubTens)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_BUTTON_NUM0_NOR+ubTens, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = uwXStart;
		tOsdImgInfo.uwYStart = uwYStart;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_BUTTON_NUM0_NOR+ubUnits, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = uwXStart + 20;
		tOsdImgInfo.uwYStart = uwYStart;
		tOSD_Img2(&tOsdImgInfo, tUpdateMod);
	}
	else
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_BUTTON_NUM0_NOR+ubUnits, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = uwXStart;
		tOsdImgInfo.uwYStart = uwYStart;
		tOSD_Img2(&tOsdImgInfo, tUpdateMod);
	}
}

//------------------------------------------------------------------------------
void UI_ShowButtonValueHighLight(uint16_t uwValue,uint16_t uwXStart,uint16_t uwYStart,OSD_UPDATE_TYP tUpdateMod)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint8_t ubUnits = 0, ubTens = 0, ubHunds = 0,ubThous = 0;
	uint8_t *ubVal[4] = {&ubUnits,&ubTens,&ubHunds,&ubThous};
	for(uint8_t i = 0;i < 4;i++)
	{
		*ubVal[i] = uwValue % 10;
		uwValue /= 10;
	}
	if(ubThous)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_BUTTON_NUM0_HL+ubThous, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = uwXStart;
		tOsdImgInfo.uwYStart = uwYStart;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_BUTTON_NUM0_HL+ubHunds, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = uwXStart + 20;
		tOsdImgInfo.uwYStart = uwYStart;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_BUTTON_NUM0_HL+ubTens, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = uwXStart + 40;
		tOsdImgInfo.uwYStart = uwYStart;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_BUTTON_NUM0_HL+ubUnits, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = uwXStart + 60;
		tOsdImgInfo.uwYStart = uwYStart;
		tOSD_Img2(&tOsdImgInfo, tUpdateMod);

	}
	else if(ubHunds)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_BUTTON_NUM0_HL+ubHunds, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = uwXStart;
		tOsdImgInfo.uwYStart = uwYStart;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_BUTTON_NUM0_HL+ubTens, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = uwXStart + 20;
		tOsdImgInfo.uwYStart = uwYStart;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_BUTTON_NUM0_HL+ubUnits, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = uwXStart + 40;
		tOsdImgInfo.uwYStart = uwYStart;
		tOSD_Img2(&tOsdImgInfo, tUpdateMod);
	}
	else if(ubTens)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_BUTTON_NUM0_HL+ubTens, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = uwXStart;
		tOsdImgInfo.uwYStart = uwYStart;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_BUTTON_NUM0_HL+ubUnits, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = uwXStart + 20;
		tOsdImgInfo.uwYStart = uwYStart;
		tOSD_Img2(&tOsdImgInfo, tUpdateMod);
	}
	else
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_BUTTON_NUM0_HL+ubUnits, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = uwXStart;
		tOsdImgInfo.uwYStart = uwYStart;
		tOSD_Img2(&tOsdImgInfo, tUpdateMod);
	}
}

//------------------------------------------------------------------------------
void UI_ShowKeyBoardValue(uint16_t uwValue,OSD_UPDATE_TYP tUpdateMod)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint8_t ubUnits = 0, ubTens = 0, ubHunds = 0,ubThous = 0;

	uint8_t *ubVal[4] = {&ubUnits,&ubTens,&ubHunds,&ubThous};
	for(uint8_t i = 0;i < 4;i++)
	{
		*ubVal[i] = uwValue % 10;
		uwValue /= 10;
	}
	if(ubThous)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_KEYBOARD_SHOWNUM0+ubThous, 1, &tOsdImgInfo);
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_KEYBOARD_SHOWNUM0+ubHunds, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = tOsdImgInfo.uwXStart + 20;
		tOsdImgInfo.uwYStart = tOsdImgInfo.uwYStart;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_KEYBOARD_SHOWNUM0+ubTens, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = tOsdImgInfo.uwXStart + 40;
		tOsdImgInfo.uwYStart = tOsdImgInfo.uwYStart;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_KEYBOARD_SHOWNUM0+ubUnits , 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = tOsdImgInfo.uwXStart + 60;
		tOsdImgInfo.uwYStart = tOsdImgInfo.uwYStart;
		tOSD_Img2(&tOsdImgInfo, tUpdateMod);

	}
	else if(ubHunds)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_KEYBOARD_SHOWNUM0+ubHunds, 1, &tOsdImgInfo);
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_KEYBOARD_SHOWNUM0+ubTens, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = tOsdImgInfo.uwXStart + 20;
		tOsdImgInfo.uwYStart = tOsdImgInfo.uwYStart;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_KEYBOARD_SHOWNUM0+ubUnits, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = tOsdImgInfo.uwXStart + 40;
		tOsdImgInfo.uwYStart = tOsdImgInfo.uwYStart;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
		
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_KEYBOARD_SHOWNUM_COVERING, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = tOsdImgInfo.uwXStart + 60;
		tOsdImgInfo.uwYStart = tOsdImgInfo.uwYStart;
		tOSD_Img2(&tOsdImgInfo, tUpdateMod);
	}
	else if(ubTens)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_KEYBOARD_SHOWNUM0+ubTens, 1, &tOsdImgInfo);
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_KEYBOARD_SHOWNUM0+ubUnits, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = tOsdImgInfo.uwXStart + 20;
		tOsdImgInfo.uwYStart = tOsdImgInfo.uwYStart;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
		
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_KEYBOARD_SHOWNUM_COVERING, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = tOsdImgInfo.uwXStart + 40;
		tOsdImgInfo.uwYStart = tOsdImgInfo.uwYStart;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
		tOsdImgInfo.uwXStart = tOsdImgInfo.uwXStart + 20;
		tOsdImgInfo.uwYStart = tOsdImgInfo.uwYStart;
		tOSD_Img2(&tOsdImgInfo, tUpdateMod);
	}
	else
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_KEYBOARD_SHOWNUM0+ubUnits, 1, &tOsdImgInfo);
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
		
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_KEYBOARD_SHOWNUM_COVERING, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = tOsdImgInfo.uwXStart + 20;
		tOsdImgInfo.uwYStart = tOsdImgInfo.uwYStart;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
		tOsdImgInfo.uwXStart = tOsdImgInfo.uwXStart + 20;
		tOsdImgInfo.uwYStart = tOsdImgInfo.uwYStart;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
		tOsdImgInfo.uwXStart = tOsdImgInfo.uwXStart + 20;
		tOsdImgInfo.uwYStart = tOsdImgInfo.uwYStart;
		tOSD_Img2(&tOsdImgInfo, tUpdateMod);
	}
}
//------------------------------------------------------------------
void UI_ShowPlaybackPageNum(uint16_t uwValue,uint16_t uwXStart,uint16_t uwYStart,OSD_UPDATE_TYP tUpdateMod)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint8_t ubUnits = 0, ubTens = 0, ubHunds = 0,ubThous = 0;
	uint8_t *ubVal[4] = {&ubUnits,&ubTens,&ubHunds,&ubThous};
	for(uint8_t i = 0;i < 4;i++)
	{
		*ubVal[i] = uwValue % 10;
		uwValue /= 10;
	}
	if(ubThous)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_CHAR_NUM0+ubThous, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = uwXStart;
		tOsdImgInfo.uwYStart = uwYStart;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_CHAR_NUM0+ubHunds, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = uwXStart + tOsdImgInfo.uwHSize;
		tOsdImgInfo.uwYStart = uwYStart;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_CHAR_NUM0+ubTens, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = uwXStart + 2*tOsdImgInfo.uwHSize;
		tOsdImgInfo.uwYStart = uwYStart;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_CHAR_NUM0+ubUnits, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = uwXStart + 3*tOsdImgInfo.uwHSize;
		tOsdImgInfo.uwYStart = uwYStart;
		tOSD_Img2(&tOsdImgInfo, tUpdateMod);

	}
	else if(ubHunds)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_CHAR_NUM0+ubHunds, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = uwXStart;
		tOsdImgInfo.uwYStart = uwYStart;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_CHAR_NUM0+ubTens, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = uwXStart + tOsdImgInfo.uwHSize;
		tOsdImgInfo.uwYStart = uwYStart;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_CHAR_NUM0+ubUnits, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = uwXStart + 2*tOsdImgInfo.uwHSize;
		tOsdImgInfo.uwYStart = uwYStart;
		tOSD_Img2(&tOsdImgInfo, tUpdateMod);
	}
	else if(ubTens)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_CHAR_NUM0+ubTens, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = uwXStart;
		tOsdImgInfo.uwYStart = uwYStart;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_CHAR_NUM0+ubUnits, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = uwXStart + tOsdImgInfo.uwHSize;
		tOsdImgInfo.uwYStart = uwYStart;
		tOSD_Img2(&tOsdImgInfo, tUpdateMod);
	}
	else
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_CHAR_NUM0+ubUnits, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = uwXStart;
		tOsdImgInfo.uwYStart = uwYStart;
		tOSD_Img2(&tOsdImgInfo, tUpdateMod);
	}
}
//------------------------------------------------------------------
void UI_ShowPlayTime(uint16_t uwCurTime,uint16_t uwTotalTime,uint16_t uwXStart,uint16_t uwYStart)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint8_t ubminute,ubsecond;
	uint8_t *ubVal[2] = {&ubsecond,&ubminute};
	for(uint8_t i = 0;i < 2;i ++)
	{
		*ubVal[i] = uwCurTime % 60;
		uwCurTime /= 60;
	}
	//slash
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_TIME_SLASH, 1, &tOsdImgInfo); 
	tOsdImgInfo.uwXStart = uwXStart + 4*tOsdImgInfo.uwHSize + 5;
	tOsdImgInfo.uwYStart = uwYStart;
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	
	/***********CurTime****************/
	//Colon
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_TIME_COLON, 1, &tOsdImgInfo);	
	tOsdImgInfo.uwXStart = uwXStart + 2*tOsdImgInfo.uwHSize - 5;
	tOsdImgInfo.uwYStart = uwYStart;
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	
	//minute
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_TIME_NUM0 + ubminute/10, 1, &tOsdImgInfo);	
	tOsdImgInfo.uwXStart = uwXStart;
	tOsdImgInfo.uwYStart = uwYStart;
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_TIME_NUM0 + ubminute%10, 1, &tOsdImgInfo);	
	tOsdImgInfo.uwXStart = uwXStart + tOsdImgInfo.uwHSize;
	tOsdImgInfo.uwYStart = uwYStart;
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

	//second
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_TIME_NUM0 + ubsecond/10, 1, &tOsdImgInfo);	
	tOsdImgInfo.uwXStart = uwXStart + 3*tOsdImgInfo.uwHSize - 10;
	tOsdImgInfo.uwYStart = uwYStart;
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_TIME_NUM0 + ubsecond%10, 1, &tOsdImgInfo);	
	tOsdImgInfo.uwXStart = uwXStart + 4*tOsdImgInfo.uwHSize - 10;
	tOsdImgInfo.uwYStart = uwYStart;
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	
	/***********TotalTime****************/

	for(uint8_t i = 0;i < 2;i ++)
	{
		*ubVal[i] = uwTotalTime % 60;
		uwTotalTime /= 60;
	}

	//Colon
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_TIME_COLON, 1, &tOsdImgInfo);	
	tOsdImgInfo.uwXStart = uwXStart + 7*tOsdImgInfo.uwHSize - 5;
	tOsdImgInfo.uwYStart = uwYStart;
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	
	//minute
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_TIME_NUM0 + ubminute/10, 1, &tOsdImgInfo);	
	tOsdImgInfo.uwXStart = uwXStart + 5*tOsdImgInfo.uwHSize;
	tOsdImgInfo.uwYStart = uwYStart;
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_TIME_NUM0 + ubminute%10, 1, &tOsdImgInfo);	
	tOsdImgInfo.uwXStart = uwXStart + 6*tOsdImgInfo.uwHSize;
	tOsdImgInfo.uwYStart = uwYStart;
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);

	//second
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_TIME_NUM0 + ubsecond/10, 1, &tOsdImgInfo);	
	tOsdImgInfo.uwXStart = uwXStart + 8*tOsdImgInfo.uwHSize - 10;
	tOsdImgInfo.uwYStart = uwYStart;
	tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
	
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_TIME_NUM0 + ubsecond%10, 1, &tOsdImgInfo);	
	tOsdImgInfo.uwXStart = uwXStart + 9*tOsdImgInfo.uwHSize - 10;
	tOsdImgInfo.uwYStart = uwYStart;
	tOSD_Img2(&tOsdImgInfo, OSD_UPDATE);
	
}
//------------------------------------------------------------------------------
void UI_ShowNumber(uint16_t uwValue,uint16_t uwXStart,uint16_t uwYStart,uint16_t uwNum0_ImageIndex,OSD_UPDATE_TYP tUpdateMod)
{
	OSD_IMG_INFO tOsdImgInfo;
	uint8_t ubUnits = 0, ubTens = 0, ubHunds = 0,ubThous = 0;
	uint8_t *ubVal[4] = {&ubUnits,&ubTens,&ubHunds,&ubThous};
	for(uint8_t i = 0;i < 4;i++)
	{
		*ubVal[i] = uwValue % 10;
		uwValue /= 10;
	}
	if(ubThous)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, uwNum0_ImageIndex+ubThous, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = uwXStart;
		tOsdImgInfo.uwYStart = uwYStart;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
		tOSD_GetOsdImgInfor(1, OSD_IMG2, uwNum0_ImageIndex+ubHunds, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = uwXStart + 20;
		tOsdImgInfo.uwYStart = uwYStart;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
		tOSD_GetOsdImgInfor(1, OSD_IMG2, uwNum0_ImageIndex+ubTens, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = uwXStart + 40;
		tOsdImgInfo.uwYStart = uwYStart;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
		tOSD_GetOsdImgInfor(1, OSD_IMG2, uwNum0_ImageIndex+ubUnits, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = uwXStart + 60;
		tOsdImgInfo.uwYStart = uwYStart;
		tOSD_Img2(&tOsdImgInfo, tUpdateMod);

	}
	else if(ubHunds)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, uwNum0_ImageIndex+ubHunds, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = uwXStart;
		tOsdImgInfo.uwYStart = uwYStart;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
		tOSD_GetOsdImgInfor(1, OSD_IMG2, uwNum0_ImageIndex+ubTens, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = uwXStart + 20;
		tOsdImgInfo.uwYStart = uwYStart;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
		tOSD_GetOsdImgInfor(1, OSD_IMG2, uwNum0_ImageIndex+ubUnits, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = uwXStart + 40;
		tOsdImgInfo.uwYStart = uwYStart;
		tOSD_Img2(&tOsdImgInfo, tUpdateMod);
	}
	else if(ubTens)
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, uwNum0_ImageIndex+ubTens, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = uwXStart;
		tOsdImgInfo.uwYStart = uwYStart;
		tOSD_Img2(&tOsdImgInfo, OSD_QUEUE);
		tOSD_GetOsdImgInfor(1, OSD_IMG2, uwNum0_ImageIndex+ubUnits, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = uwXStart + 20;
		tOsdImgInfo.uwYStart = uwYStart;
		tOSD_Img2(&tOsdImgInfo, tUpdateMod);
	}
	else
	{
		tOSD_GetOsdImgInfor(1, OSD_IMG2, uwNum0_ImageIndex+ubUnits, 1, &tOsdImgInfo);
		tOsdImgInfo.uwXStart = uwXStart;
		tOsdImgInfo.uwYStart = uwYStart;
		tOSD_Img2(&tOsdImgInfo, tUpdateMod);
	}
}


//------------------------------------------------------------------
void UI_InitCharArray(void)
{
	uint8_t i;
	
	for(i = 0; i < 10; i++)
		tCharOsdImgDB.uwUI_NumArray[i] = OSD2IMG_CHAR_NUM0 + i;
																			 
	tUI_CharOsdImgInfo.pNumImgIdxArray = tCharOsdImgDB.uwUI_NumArray;
	for(i = 0; i < 26; i++)
	{
		tCharOsdImgDB.uwUI_UpperLetterArray[i] = OSD2IMG_CHAR_UPA + i;
																				      
		tCharOsdImgDB.uwUI_LowerLetterArray[i] = OSD2IMG_CHAR_UPA + i;
																					
	}
	tUI_CharOsdImgInfo.pUpperLetterImgIdxArray = tCharOsdImgDB.uwUI_UpperLetterArray;
	tUI_CharOsdImgInfo.pLowerLetterImgIdxArray = tCharOsdImgDB.uwUI_LowerLetterArray;
	for(i = 0; i < 4; i++)
		tCharOsdImgDB.uwUI_SymbolArray[i] = OSD2IMG_CHAR_COLON + i;
																				 
	tUI_CharOsdImgInfo.pSymbolImgIdxArray = tCharOsdImgDB.uwUI_SymbolArray;
}	

//------------------------------------------------------------------
long UI_Map(long x, long in_min, long in_max, long out_min, long out_max)
{
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
//------------------------------------------------------------------
uint32_t UI_GetBackLightPwm(void)
{
	if(tUI_CuSetting.ubAutoDimmer)
	{
		if(uwSADC_GetReport(SADC_CH4) < BACKLIGHT_ADC_VALUE)//������
		{
			ubUI_CurDimmerLvl = tUI_CuSetting.ubAutoDimmer_MinVal;
			
		}
		else
		{
			ubUI_CurDimmerLvl = tUI_CuSetting.ubAutoDimmer_MaxVal;
		}
		return (uint16_t)UI_Map(ubUI_CurDimmerLvl,0,99,LCD_PWM_MIN,LCD_PWM_MAX);
	}
	else
		return (uint16_t)UI_Map(tUI_CuSetting.BriLvL.ulBL_UpdateLvL,0,99,LCD_PWM_MIN,LCD_PWM_MAX);
}
//------------------------------------------------------------------
uint32_t UI_GetVolumePwm(void)
{
	if(tUI_CuSetting.ubMuteFlag)
		return 0;
	else
	{
		uint16_t uwValue = UI_Map(UI_GetCurrentVolumeLevel(),0,99,VOLUME_PWM_MIN,VOLUME_PWM_MAX);
		if(uwValue == VOLUME_PWM_MIN)
			return 0;
		else
			return uwValue;
	}
}

//------------------------------------------------------------------
UI_CamSource UI_GetAudioSrc(void)
{
	UI_CamSource tAdoCamNum;
	if(tCamViewSel.tCamViewType == SINGLE_VIEW) {
		if(tUI_CuSetting.tAdoSrcCamNum_Single[tCamViewSel.tCamViewPool[0]] == CamSource_AI) {
			tAdoCamNum = CamSource_AI;
		} else {
			tAdoCamNum = (UI_CamSource)tCamViewSel.tCamViewPool[0];
		}
	} else if(tCamViewSel.tCamViewType == DUAL_VIEW) {
		tAdoCamNum = tUI_CuSetting.tAdoSrcCamNum_Dual;
	} else if(tCamViewSel.tCamViewType == QUAD_VIEW) {
		tAdoCamNum = tUI_CuSetting.tAdoSrcCamNum_Quad;
	}
	
	return tAdoCamNum;
}
void UI_SetAudioSrc(UI_CamSource tAdoSrc)
{
	if(tCamViewSel.tCamViewType == SINGLE_VIEW)
		tUI_CuSetting.tAdoSrcCamNum_Single[tCamViewSel.tCamViewPool[0]] = tAdoSrc;
	if(tCamViewSel.tCamViewType == DUAL_VIEW)
		tUI_CuSetting.tAdoSrcCamNum_Dual = tAdoSrc;
	else if(tCamViewSel.tCamViewType == QUAD_VIEW)
		tUI_CuSetting.tAdoSrcCamNum_Quad = tAdoSrc;
	
}
//------------------------------------------------------------------
void UI_PanelOn(void)
{
	if(tUI_CuSetting.ubAutoDimmer)
	{
		UI_CheckLightSensor(TRUE);
	}
	else
	{
		ubUI_CurDimmerLvl = tUI_CuSetting.BriLvL.ulBL_UpdateLvL;
		uint8_t ubValue = (uint16_t)UI_Map(tUI_CuSetting.BriLvL.ulBL_UpdateLvL ,0,99,LCD_PWM_MIN,LCD_PWM_MAX);
		SET_PANEL_PWM(ubValue);
	}
		
}
//------------------------------------------------------------------
void UI_PanelOff(void)
{
	ubUI_CurDimmerLvl = 0;
	SET_PANEL_PWM(0);
}
//------------------------------------------------------------------
void UI_AudioOn(void)
{
	uint16_t uwValue;
	if(tUI_CuSetting.ubMuteFlag)
	{
		SET_VOLUME_PWM(0);//拉低pwm
		osDelay(50);
		SET_VOLUME_IO = 0;//关闭4875_power使能
		
	}
	else
	{
		uwValue = (uint16_t)UI_Map(UI_GetCurrentVolumeLevel(),0,99,VOLUME_PWM_MIN,VOLUME_PWM_MAX);			
		SET_VOLUME_PWM((uwValue == VOLUME_PWM_MIN)?0:uwValue);//拉高pwm
		osDelay(50);
		if(uwValue == VOLUME_PWM_MIN)
			SET_VOLUME_IO = 0;//断电4875
		else 
			SET_VOLUME_IO = 1;//上电4875
	}
}
//------------------------------------------------------------------
void UI_AudioOff(void)
{
	SET_VOLUME_PWM(0);
	//osDelay(50);
	SET_VOLUME_IO = 0;
}

//------------------------------------------------------------------
//void UI_CheckPwrUartCount(void)
//{
//	if(ubPwrUartFlag == TRUE && ubPwrUartCount == 0)
//	{
//		ubPwrUartFlag = FALSE;
//		//ubPwrUartCount = 5000;
//		GLB->PADIO49 = 2; //UART2_TX
//		GLB->PADIO50 = 2; //UART2_RX
//		KNL_UsbdFwuFg = 0;
//		printf("UI_CheckPwrUartCount run!\n");
//		osDelay(100);
//	}
//	return;
//}

//------------------------------------------------------------------
void UI_CheckMenuOffCount(void)
{
	UI_Event_t tMenuOffEvent;
	osMessageQId *pUI_MenuOffEventQH = NULL;
	static int cnt = 0;
    cnt++;
    if (cnt >= 15)
    {
		cnt = 0;
		//printf("ulMenuAutoOffCount = %d, MenuOnFlag = %d\n", ulMenuAutoOffCount, MenuOnFlag);
    }
	
	if(ulMenuAutoOffCount == 0 && MenuOnFlag == TRUE)
	{
		printf("UI_CheckMenuOffCount !!!!!!!!!!");
		tMenuOffEvent.tEventType = MENUOFF_EVENT;
		tMenuOffEvent.pvEvent 	  = NULL;
		pUI_MenuOffEventQH 	  = pUI_GetEventQueueHandle();
	    osMessagePut(*pUI_MenuOffEventQH, &tMenuOffEvent, 0);
		UI_UpdateDevStatusInfo();
	}

}
//------------------------------------------------------------------
void UI_CheckPanelOffCount(void)
{
	if(ulPanelOffCount == 0 && !MenuOnFlag && !ubUI_CuStandbyFlag)
	{
		UI_EnterStandby();
	}
}
//------------------------------------------------------------------
void UI_CheckTriggerOverCount(void)
{
	osSemaphoreWait(osUI_CuTriggerCtr, osWaitForever);
	if(ulTriggerOverCount == 0 && TriggerLock)
	{
		printf("************Over Tigger!!!\n");
		TriggerLock = FALSE;
		TriggerSource = 0xff;
		Trigger_View.tUI_ViewSel = TriggerBackUp_View;
		
		if(ubUI_CuStandbyFlag)
		{
			UI_PanelOff();
			UI_AudioOff();
			POWER_LED_IO = 1;

			if(UI_REC_START == tUI_RecPlayAct.tRecAct)
			{
				UI_VideoRecordingExec(UI_REC_STOP);
				ubUI_RestartRec = TRUE;
			}
			UI_DisconnectTx();
		}
		
		UI_Event_t Trigger_Event;
		Trigger_Event.pvEvent = (void *)(&Trigger_View);
		Trigger_Event.tEventType = TRIGGER_EVENT;
		printf("*************UI_CheckTriggerOverCount tUI_ViewSel = %d!!\n", Trigger_View.tUI_ViewSel);
		 if(osMessagePut(UI_EventQueue, &Trigger_Event, 0) != osOK)
			printd(DBG_ErrorLvl, "Trigger Q Full\n");
	}
	osSemaphoreRelease(osUI_CuTriggerCtr);
}

//------------------------------------------------------------------
void UI_ShowRecordingStatus(bool is_flip)
{
	OSD_IMG_INFO tOsdImgInfo[3],tEraserOsdImgInfo;
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_RECORDING_BLACK_ICON, 3, &tOsdImgInfo[0]);
	static uint8_t ubImgIdx = 0;
	// 信号图标下方也是保留区，先擦除整段背景，避免小录像图标之外残留框线。
	tEraserOsdImgInfo.uwXStart = 4;
	tEraserOsdImgInfo.uwYStart = 50;
	tEraserOsdImgInfo.uwHSize = 106;
	tEraserOsdImgInfo.uwVSize = 30;
	OSD_EraserImg2_NoUpdate(&tEraserOsdImgInfo);
	if(UI_REC_START == tUI_RecPlayAct.tRecAct)
	{
		if(tUI_SyncAppState == APP_LINK_STATE)
		{
			// 重画图标时保持当前状态，仅录像定时刷新时切换闪烁。
			if(is_flip)
				ubImgIdx = 1 - ubImgIdx;
			tOSD_Img2(&tOsdImgInfo[ubImgIdx], OSD_UPDATE);
		}
		else
		{
			tOSD_Img2(&tOsdImgInfo[2], OSD_UPDATE);
		}
	}
	else
	{	
		ubImgIdx = 0;
		UI_ClearRecordStatusOsdImg();
	}

}

//------------------------------------------------------------------
void UI_BackToDesktop(uint8_t ubSwitchView)
{
	printf("UI_BackToDesktop\n");
	if(ubSwitchView && !TriggerLock)
		UI_SwitchViewType(DeskTopShowView,FALSE);
	
	UI_ClearOsdImageNoUpdate();
	MenuOnFlag = FALSE;
	SendIrCodeFlag = FALSE;
	tUI_State = UI_DISPLAY_STATE;
	ubDrawBSDRange = 1;
	UI_DrawDesktopIcon();
	//ubDrawBSDRange = 0;
//	if(TriggerLock && DeskTopShowView <= CAM4 && tUI_CuSetting.tParkingLineEnable[DeskTopShowView] == PARKING_LINE_AUTO)
//	{
//		//UI_DrawParkingLine(DeskTopShowView,OSD_UPDATE);
//	}

}
//------------------------------------------------------------------
void UI_EnterStandby(void)
{
	printf("---UI_EnterStandby\n");
	osSemaphoreWait(osUI_PowerLightCtr, osWaitForever);
	ubUI_CuStandbyFlag = TRUE;
	UI_PanelOff();
	UI_AudioOff();
	
	if(UI_REC_START == tUI_RecPlayAct.tRecAct)
	{
		UI_VideoRecordingExec(UI_REC_STOP);
		ubUI_RestartRec = TRUE;
	}
	UI_BackToDesktop(FALSE);

	
	POWER_LED_IO = 1;
	
	if(tUI_CuSetting.tPowerOnMode == POWERON_LAST)
		UI_SaveLastMode();
	UI_DisconnectTx();
	osSemaphoreRelease(osUI_PowerLightCtr);
}
//------------------------------------------------------------------
void UI_WakeupStandby(void)
{
	printf("UI_WakeupStandby\n");
	if(ubUI_CuPowerOffFlag)
	{
		SYS_Reboot();
		while(1);
	}
	ubUI_CuStandbyFlag = FALSE;
	printf("start SwitchViewType!!\n");
	UI_SwitchViewType(DeskTopShowView,TRUE);
	printf("end SwitchViewType!!\n");
	UI_ClearOsdImageNoUpdate();
	//ubDrawBSDRange = 0;
	ubDrawBSDRange = 1;
	UI_DrawDesktopIcon();
	BUZZER_DI(150);
	//ubDrawBSDRange = 1;
	POWER_LED_IO = 0;
	if(tUI_CuSetting.tPowerOnMode == POWERON_LAST)
		UI_SaveLastMode();
	ubAutoScanReady = TRUE;
}
//------------------------------------------------------------------
void UI_MenuLock(void)
{
	OSD_IMG_INFO tOsdImgInfo;
	
	osSemaphoreWait(osUI_CuUiCtr, osWaitForever);
	tUI_CuSetting.ubMenuLock = 1 - tUI_CuSetting.ubMenuLock;
	tOSD_GetOsdImgInfor(1, OSD_IMG2, OSD2IMG_DESKTOP_MENUUNLOCK + tUI_CuSetting.ubMenuLock, 1, &tOsdImgInfo);
	tOSD_Img1(&tOsdImgInfo, OSD_UPDATE);
	osDelay(500);
	OSD_EraserImg1(&tOsdImgInfo);
	//ubDrawBSDRange = 0;
	UI_DrawDesktopIcon();
	//ubDrawBSDRange = 1;
	osSemaphoreRelease(osUI_CuUiCtr);
	UI_UpdateDevStatusInfo();
}
//------------------------------------------------------------------
void UI_SetTxImgProc(UI_CamNum_t tCamNum)
{
	uint8_t ubValue,ubTryCount;
	UI_CUReqCmd_t tCamSetCmd;
	tCamSetCmd.tDS_CamNum				= tCamNum;
	tCamSetCmd.ubCmd[UI_TWC_TYPE]		= UI_SETTING;
	tCamSetCmd.ubCmd[UI_SETTING_ITEM]	= UI_IMGPROC_SETTING;
	tCamSetCmd.ubCmd_Len				 = 4;
	ubTryCount = 10;;
	while(-- ubTryCount)
	{
		if(tUI_CamStatus[tCamNum].tCamConnSts == CAM_OFFLINE)
			osDelay(100);			
		else
			break;
	}
	if(ubTryCount == 0)
	{
		printf("tUI_CamStatus[%d].tCamConnSts == CAM_OFFLINE\n",tCamNum);
		return;
	}
		
	ubValue = UI_Map(tUI_CamStatus[tCamNum].tCamColorParam.ubColorBL + tUI_CuSetting.bColorBLGain[tCamNum],0,99,0,127);
	tCamSetCmd.ubCmd[UI_SETTING_DATA]	 = UI_IMGBL_SETTING ;
	tCamSetCmd.ubCmd[UI_SETTING_DATA+1] = ubValue;
	ubTryCount = 5;
	while(-- ubTryCount)
	{
		if(UI_SendRequestToCAM(osThreadGetId(), &tCamSetCmd) != rUI_SUCCESS)
		{
			printf("UI_IMGBL_SETTING Fail !\n");
			osDelay(20);
		}
		else
			break;
	}
	
	ubValue = UI_Map(tUI_CamStatus[tCamNum].tCamColorParam.ubColorContrast + tUI_CuSetting.bColorContrastGain[tCamNum],0,99,0,127);
	tCamSetCmd.ubCmd[UI_SETTING_DATA]	 = UI_IMGCONTRAST_SETTING ;
	tCamSetCmd.ubCmd[UI_SETTING_DATA+1] = ubValue;
	ubTryCount = 5;
	while(-- ubTryCount)
	{
		if(UI_SendRequestToCAM(osThreadGetId(), &tCamSetCmd) != rUI_SUCCESS)
		{
			printf("UI_IMGCONTRAST_SETTING Fail !\n");
			osDelay(20);
		}
		else
			break;
	}
	
	ubValue = UI_Map(tUI_CamStatus[tCamNum].tCamColorParam.ubColorSaturation + tUI_CuSetting.bColorSaturationGain[tCamNum],0,99,0,127);
	tCamSetCmd.ubCmd[UI_SETTING_DATA]	 = UI_IMGSATURATION_SETTING ;
	tCamSetCmd.ubCmd[UI_SETTING_DATA+1] = ubValue;
	ubTryCount = 5;
	while(-- ubTryCount)
	{
		if(UI_SendRequestToCAM(osThreadGetId(), &tCamSetCmd) != rUI_SUCCESS)
		{
			printf("UI_IMGSATURATION_SETTING Fail !\n");
			osDelay(20);
		}
		else
			break;
	}

	ubValue = UI_Map(tUI_CamStatus[tCamNum].tCamColorParam.ubColorHue + tUI_CuSetting.bColorHueGain[tCamNum],0,99,0,127);
	tCamSetCmd.ubCmd[UI_SETTING_DATA]	 = UI_IMGHUE_SETTING ;
	tCamSetCmd.ubCmd[UI_SETTING_DATA+1] = ubValue;
	ubTryCount = 5;
	while(-- ubTryCount)
	{
		if(UI_SendRequestToCAM(osThreadGetId(), &tCamSetCmd) != rUI_SUCCESS)
		{
			printf("UI_IMGHUE_SETTING Fail !\n");
			osDelay(20);
		}
		else
			break;
	}

	tCamSetCmd.ubCmd[UI_SETTING_DATA]	= UI_IMGMIRROR_SETTING;
	tCamSetCmd.ubCmd[UI_SETTING_DATA+1] = tUI_CamStatus[tCamNum].tCamImgMirror;
	ubTryCount = 5;
	while(-- ubTryCount)
	{
		if(UI_SendRequestToCAM(osThreadGetId(), &tCamSetCmd) != rUI_SUCCESS)
		{
			printf("UI_IMGMIRROR_SETTING  Fail !\n");
			osDelay(20);
		}
		else
			break;
	}

	tCamSetCmd.ubCmd[UI_SETTING_DATA]	= UI_IMGFLIP_SETTING;
	tCamSetCmd.ubCmd[UI_SETTING_DATA+1] = tUI_CamStatus[tCamNum].tCamImgFlip;
	ubTryCount = 5;
	while(-- ubTryCount)
	{
		if(UI_SendRequestToCAM(osThreadGetId(), &tCamSetCmd) != rUI_SUCCESS)
		{
			printf("UI_IMGFLIP_SETTING Fail !\n");
			osDelay(20);
		}
		else
			break;

	}
	printf("UI_SetTxImgProc tCamNum = %d success\n",tCamNum);
}
//------------------------------------------------------------------
void UI_SaveLastMode(void)
{
	if(!ubUI_CuStandbyFlag)
		tUI_CuSetting.tLastMode = (UI_LastMode_t)tUI_ViewModeSel;
	else if(ubUI_CuStandbyFlag && tUI_ViewModeSel != SCANVIEW_ITEM)
		tUI_CuSetting.tLastMode = LASTMODE_STANDBY;
	else if(ubUI_CuStandbyFlag && tUI_ViewModeSel == SCANVIEW_ITEM)
		tUI_CuSetting.tLastMode = LASTMODE_SCAN_STANDBY;
	
	tUI_CuSetting.tLastView = DeskTopShowView;
	UI_UpdateDevStatusInfo();
}

//------------------------------------------------------------------
void UI_SyncTimeStamp2TX(void)
{
	APP_EventMsg_t tUI_Message = {0};
	tUI_Message.ubAPP_Event 	 = APP_SYNCTIMESTAMP_EVENT;
	tUI_Message.ubAPP_Message[0] = 1;	
	tUI_Message.ubAPP_Message[1] = tUI_CuSetting.ubTimeStame;
	UI_SendMessageToAPP(&tUI_Message);
}


/********************************************************************TOUCH*********************************************************************************************/
/********************************************************************TOUCH*********************************************************************************************/
/********************************************************************TOUCH*********************************************************************************************/

//------------------------------------------------------------------------------
void UI_DisplayTouchFunc(TOUCH_EVENT_t *Touch_Info)
{
	if(ubUI_CuStandbyFlag && Touch_Info->Gesture != TOUCH_TURNUP)
		return;
	switch(Touch_Info->Gesture)
	{
		case TOUCH_PRESS:
			if(Touch_Info->startY > 510)//����home menu
			{
				UI_MenuKey();
			}
			else//������ת
			{
				if((tUI_ViewModeSel == SCANVIEW_ITEM) || (ubUI_FinishViewSwitch == FALSE) || TriggerLock)
					return;
//				//wav
//				ADO_WavStop();
//				ISPlaying_wav = 0;

				//osDelay(20);
				switch(tCamViewSel.tCamViewType)
				{
					case SINGLE_VIEW:	
						tUI_ViewModeSel = tUI_SaveViewNum;
						break;
					case DUAL_VIEW:	
						tUI_SaveViewNum = tUI_ViewModeSel;
						if(Touch_Info->startX < 512)
						{
							tUI_ViewModeSel = tCamViewSel.tCamViewPool[0];
						}
						else
						{
							tUI_ViewModeSel = tCamViewSel.tCamViewPool[1];
						}
							
						break;
					case QUAD_VIEW:	
						tUI_SaveViewNum = tUI_ViewModeSel;
						if(Touch_Info->startX < 512 && Touch_Info->startY < 300)
						{
							tUI_ViewModeSel = tCamViewSel.tCamViewPool[0];
						}
						else if(Touch_Info->startX >= 512 && Touch_Info->startY < 300)
						{
							tUI_ViewModeSel = tCamViewSel.tCamViewPool[1];
						}
						else if(Touch_Info->startX < 512 && Touch_Info->startY >= 300)
						{
							tUI_ViewModeSel = tCamViewSel.tCamViewPool[2];
						}
						else
						{
							tUI_ViewModeSel = tCamViewSel.tCamViewPool[3];
						}
						break;
					default:
						break;
				}
				DeskTopShowView = tUI_ViewModeSel;
				UI_SwitchViewType(tUI_ViewModeSel,TRUE);
				if(tUI_CuSetting.tPowerOnMode == POWERON_LAST)
					UI_SaveLastMode();
				
				//UI_ClearOsdImageNoUpdate();
				UI_ClearOsdImage();
//				//ubDrawBSDRange = 0;
				UI_DrawDesktopIcon();
				//ubDrawBSDRange = 1;
			}
			
			break;
		case TOUCH_TURNUP:
			if(ubUI_CuStandbyFlag && !ubUI_CuPowerOffFlag && !TriggerLock)
			{
				UI_WakeupStandby();
			}
			break;
		case TOUCH_TURNDOWN:
			if(!ubUI_CuStandbyFlag && !TriggerLock)	
			{
				UI_EnterStandby();
			}
			break;
		case TOUCH_TURNLEFT:
			if((tUI_ViewModeSel == SCANVIEW_ITEM) || (ubUI_FinishViewSwitch == FALSE) || TriggerLock)
					return;

//			//wav
//			ADO_WavStop();
//			ISPlaying_wav = 0;

			
			if(tUI_ViewModeSel > CAM1)
				tUI_ViewModeSel --;
			else
				tUI_ViewModeSel = QUALVIEW_ITEM;
			
			DeskTopShowView = tUI_ViewModeSel;
			UI_SwitchViewType(tUI_ViewModeSel,TRUE);
			if(tUI_CuSetting.tPowerOnMode == POWERON_LAST)
				UI_SaveLastMode();

			
			UI_ClearOsdImage();
			//ubDrawBSDRange = 0;
			UI_DrawDesktopIcon();
			//ubDrawBSDRange = 1;

			break;
			
		case TOUCH_TURNRIGHT:
			if((tUI_ViewModeSel == SCANVIEW_ITEM) || (ubUI_FinishViewSwitch == FALSE) || TriggerLock)
					return;
//			//wav
//			ADO_WavStop();
//			ISPlaying_wav = 0;

			if(tUI_ViewModeSel < QUALVIEW_ITEM)
				tUI_ViewModeSel ++;
			else
				tUI_ViewModeSel = CAM1;
			
			DeskTopShowView = tUI_ViewModeSel;
			UI_SwitchViewType(tUI_ViewModeSel,TRUE);
			if(tUI_CuSetting.tPowerOnMode == POWERON_LAST)
				
				UI_SaveLastMode();
			
			UI_ClearOsdImage();
			//ubDrawBSDRange = 0;
			UI_DrawDesktopIcon();
			//ubDrawBSDRange = 1;
			break;

		default :

			break;
	}	
}

//------------------------------------------------------------------------------
void UI_PairingTouchControl(TOUCH_EVENT_t *Touch_Info)
{
	APP_EventMsg_t tUI_PairMessage;
	switch(Touch_Info->Gesture)
	{
		case TOUCH_PRESS:	
			tUI_PairMessage.ubAPP_Event = APP_PAIRING_STOP_EVENT;
			UI_SendMessageToAPP(&tUI_PairMessage);
			
			break;

		default :

			break;
	}	

}
//---------------------------------------------------------------------------------
//void UI_FWUSD_1126Key(void)
//(
//	KNL_UsbdFwupgrade();
//	return;
//)

#endif		//! End of #ifdef BSP_D_SNCC71_GM8285C_RX_V2


