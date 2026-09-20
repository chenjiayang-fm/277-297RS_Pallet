/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		UI_BUCCU.h
	\brief		User Interface Header file (for High Speed Mode)
	\author		Hanyi Chiu
	\version	1.0
	\date		2019/01/23
	\copyright	Copyright (C) 2019 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _UI_BUCCU_H_
#define _UI_BUCCU_H_

#include "UI.h"
#include "APP_HS.h"
#include "FS_API.h"
#include "OSD.h"
#include "LCD.h"
#include "RTC_API.h"
#include <string.h>
#include "VDO.h"

#if (BSP_RTC_TIMER_SEL == RTC_TIMER_EXTERNAL)
#include "RTC_PCF85063A.h"
#endif

#include "TouchPanel.h"
#include "Trigger.h"
#include "IR.h"
#define	INVALID_ID 					0xFFFFFFFF
#define INVAILD_ACT					0xFFFFFFFE
#define UI_UPDATESTS_PERIOD			(200 / UI_TASK_PERIOD)
#define UI_SHOWLOSTLOGO_PERIOD		(1000 / UI_TASK_PERIOD)
#define UI_UPDATEBRILVL_PERIOD		(1000 / UI_TASK_PERIOD)
#define UI_UPDATEVOLLVL_PERIOD		(1000 / UI_TASK_PERIOD)
#define UI_WARNINGNOTE_PERIOD		(1000 / UI_TASK_PERIOD)
#define UI_PHOTOGRAPHSTS_PERIOD		(1000 / UI_TASK_PERIOD)
#define UI_RECGRAPHSTS_PERIOD		(1000 / UI_TASK_PERIOD)
#define UI_RECORDINGSTS_PERIOD		(600 / UI_TASK_PERIOD)
#define UI_RF433CTR_PERIOD			(1000 / UI_TASK_PERIOD)
#define UI_SYNCTIMESTAMP_PERIOD		(600 / UI_TASK_PERIOD)

#define UI_SCAN_DUTY_MIN			5
#define UI_SCAN_DUTY_MAX			45

#define LCD_PWM_MAX                100
#define LCD_PWM_MIN                10
#define VOLUME_PWM_MAX              100
#define VOLUME_PWM_MIN              40
#define OSD_HEIGHT             600
#define OSD_WIDTH              1024
#define REMOTE_SELECT_ITEM 0
#define REMOTE_SET_VALUE 1
#define MENUAUTOOFFCOUNT  30000 
#define PANELAUTOOFFCOUNT  60000 
//#define POINT_CHOOSE_MODE  0
//#define POINT_MOVE_MODE    1
//#define POINT_INDEX_MAX    8
#define AI_Config_Xpos     285
#define AI_Config_Ypos     175
#define AI_Config_Xstep    147
#define AI_Config_Ystep	   (tUI_CuSetting.tLanguage == LANGUAGE_CHINESE)?66:62
#define VERSION_LEN 22

// Reused OSD images: shared placement for drawing and touch.
// Pairing / delete camera buttons (X and size come from PAIRCAM images).
#define PAIRING_Y                    215
#define PAIRING_Y_STEP               100

// Camera sliders / highlight frames (X and size come from brightness images).
#define CAMERA_SLIDER_Y              180
#define CAMERA_HL_Y                  176
#define CAMERA_Y_STEP                70

// Date / time buttons (origin and size come from DATE_TIME_YEAR images).
#define DATE_TIME_YEAR_DIST_X        130
#define DATE_TIME_YEAR_DIST_Y        118

// Trigger delay / priority numeric frames.
#define TRIGGER_X                    220
#define TRIGGER_Y                    200
#define TRIGGER_X_STEP               140
#define TRIGGER_Y_STEP               80

// Auto scan switches / delay numeric frames.
#define AUTOSCAN_SWITCH_X            240
#define AUTOSCAN_SWITCH_Y            195
#define AUTOSCAN_DELAY_X             340
#define AUTOSCAN_DELAY_Y             185
#define AUTOSCAN_X_STEP              420
#define AUTOSCAN_Y_STEP              95

// Parking line / guide line switches.
#define PARKINGLINE_SWITCH_X         250
#define PARKINGLINE_SWITCH_Y         215
#define PARKINGLINE_X_STEP           400
#define PARKINGLINE_Y_STEP           140

#define COMMAND_LENGTH 7

#if HDW277RS_000000 //Firmware Version for 277
#define SOFEWARE_VERSION            "V1.1-FCC-AI-00-250408"
#elif HDW297RS_000000 //Firmware Version for 297
#define SOFEWARE_VERSION            "V1.4-FCC-AI-00-260623"
#endif



typedef void (*pvUiFuncPtr)(void);

typedef enum
{
	UI_ICON_NORMAL = 0,
	UI_ICON_HIGHLIGHT,
	UI_ICON_READY
}UI_IconType_t;

typedef enum
{
	UP_ARROW,
	DOWN_ARROW,
	LEFT_ARROW,
	RIGHT_ARROW,
	ENTER_ARROW,
	EXIT_ARROW,
	PAIRING_ARROW,
	SELECT_ARROW,
	MIRROR_ARROW,
	MUTE_ARROW,
	VERSION_ARROW,
}UI_ArrowKey_t;

typedef enum
{
	UI_DISPLAY_STATE = 0x20,			//! Display Image
	UI_MAINMENU_STATE,					//! Menu
	UI_SUBMENU_STATE,					//! Sub menu
	UI_SUBSUBMENU_STATE,				//! Sub sub menu
	UI_SUBSUBSUBMENU_STATE,				//! Sub sub sub menu
	UI_SUBSUBSUBSUBMENU_STATE,				//! Sub sub sub sub menu
	UI_SET_VOLUME_STATE,				//! Selection Audio source
	UI_SET_CAMECOMODE_STATE,			//! Setting ECO mode of BUC CAM
	UI_SET_CUPSMODE_STATE,				//! Setting VOX/WOR mode of BUC CU
	UI_CAMSETTINGMENU_STATE,
	UI_SET_CAMCOLOR_STATE,
	UI_DPTZ_CONTROL_STATE,
	UI_MD_WINDOW_STATE,
	UI_PAIRING_STATE,
	UI_SDFWUPG_STATE,
	UI_VOXPS_STATE,
	UI_ADOONLYPS_STATE,
	UI_RECFOLDER_SEL_STATE,
	UI_RECFILES_SEL_STATE,
	UI_RECPLAYLIST_STATE,
	UI_RECPLAYADOSRC_SEL_STATE,
	UI_RECPLAYDISPTYPE_SEL_STATE,
	UI_PHOTOPLAYNRDY_STATE,
	UI_PHOTOPLAYLIST_STATE,
	UI_SDCARDFMT_STATE,
	UI_SHOWSTSICON_STATE,
	UI_ENGMODE_STATE,
	UI_FWUPG_STATE,
	UI_VOLUMESET_STATE,
	UI_SHOW_SDCARD_INFO_STATE,//sd?������3�ꨬ����?
	UI_SHOW_TX_VERSION_STATE,
	UI_STATE_MAX,
}UI_State_t;

typedef enum
{
	ADO_MODE_ITEM,
	VDO_MODE_ITEM,
	PAIRING_ITEM,
	RECORD_ITEM,
	PLAYBACK_ITEM,
	SETTING_ITEM,
	POWER_ITEM,
	MENUITEM_MAX,
}UI_MenuItemList_t;

typedef enum
{
	NOT_ACTION,
	DRAW_MENUPAGE,
	DRAW_HIGHLIGHT_MENUICON,
	EXECUTE_MENUFUNC,
	EXIT_MENUFUNC,
}UI_MenuAct_t;


typedef enum
{
	CAMSET_OFF,
	CAMSET_ON,
}UI_CamsSetMode_t;

typedef enum
{
	CAMFLICKER_50HZ,
	CAMFLICKER_60HZ,
}UI_CamFlicker_t;

typedef enum
{
	CAMIMGFLIP_DISABLE,
	CAMIMGFLIP_ENABLE,
}UI_CamImgFlip_t;

typedef enum
{
	CAMIMGMIRROR_DISABLE,
	CAMIMGMIRROR_ENABLE,
}UI_CamImgMirror_t;

typedef enum
{
	CAMLASER_DISABLE,
	CAMLASER_ENABLE,
}UI_CamLaser_t;

typedef enum
{
	CAMLED_DISABLE,
	CAMLED_ENABLE,
}UI_CamLed_t;


typedef enum
{
	CAM1VIEW_ITEM,
	CAM2VIEW_ITEM,
	CAM3VIEW_ITEM,
	CAM4VIEW_ITEM,	
	DUALVIEW_ITEM,
	QUALVIEW_ITEM,
	SCANVIEW_ITEM,
	VDO_VIEW_MAX,
}UI_VDO_MODE_SubMenuItemList_t;

typedef enum
{
	PAIRINGCAM1_ITEM,
	PAIRINGCAM2_ITEM,
	PAIRINGCAM3_ITEM,
	PAIRINGCAM4_ITEM,
	DELETECAM1_ITEM,
	DELETECAM2_ITEM,
	DELETECAM3_ITEM,
	DELETECAM4_ITEM,
	PAIRING_ITEM_MAX
}UI_PairSubMenuItemList_t;

typedef enum
{
	POWER_SCREENOFF,
	POWER_SLEEP,
	POWER_ITEM_MAX
}UI_PowerSubMenuItemList_t;
//	RECMODE_ITEM,
typedef enum
{
	RECTIME_1MIN_ITEM,
	RECTIME_3MIN_ITEM,
	RECTIME_5MIN_ITEM,
	SDCARD_ITEM,
	POWERONREC_ITEM,
	TIMESTAMP_ITEM,
	RECITEM_MAX
}UI_RecordSubSubMenuItemList_t;

typedef enum
{
	DIMMER_ITEM,
	AUTO_ONOFF_ITEM,
	AUTO_SET_ITEM,
	MENULOCK_ITEM,
	UPGRADE_ITEM,
	LANGUAGE_ITEM,
	RESET_ITEM,
	SD_SWITCH_ITEM,
	HIDE_CHANNEL_NAME_ITEM,
	BUZZER_ITEM,
	DATETIME_ITEM,
	SYSTEMITEM_MAX
}UI_SystemSubSubMenuItemList_t;

typedef enum
{
	DAY_SLIDER,
	NIGHT_SLIDER,
	DIMMER_SLIDER_MAX
}UI_SubSubSub_SystemMode_t;
typedef enum
{
	DUAL_LEFT_CAM1,
	DUAL_LEFT_CAM2,
	DUAL_LEFT_CAM3,
	DUAL_LEFT_CAM4,
	DUAL_RIGHT_CAM1,
	DUAL_RIGHT_CAM2,
	DUAL_RIGHT_CAM3,
	DUAL_RIGHT_CAM4,
	DUAL_MAX
}UI_DualSubSubMenuItemList_t;
typedef enum
{
	QUAD_UPPER_LEFT_CAM1,
	QUAD_UPPER_LEFT_CAM2,
	QUAD_UPPER_LEFT_CAM3,
	QUAD_UPPER_LEFT_CAM4,
	QUAD_UPPER_RIGHT_CAM1,
	QUAD_UPPER_RIGHT_CAM2,
	QUAD_UPPER_RIGHT_CAM3,
	QUAD_UPPER_RIGHT_CAM4,
	QUAD_LOWER_LEFT_CAM1,
	QUAD_LOWER_LEFT_CAM2,
	QUAD_LOWER_LEFT_CAM3,
	QUAD_LOWER_LEFT_CAM4,
	QUAD_LOWER_RIGHT_CAM1,
	QUAD_LOWER_RIGHT_CAM2,
	QUAD_LOWER_RIGHT_CAM3,
	QUAD_LOWER_RIGHT_CAM4,
	QUAD_MAX
}UI_QuadSubSubMenuItemList_t;
typedef enum
{
	REC_LOOPING,
	REC_MANUAL,
	REC_OFF,
	REC_RECMODE_MAX,
}UI_RecordMode_t;

typedef enum
{
	RECRES_FHD,
	RECRES_HD,
	RECRES_WVGA,
	RECRES_MAX,
}UI_RecordResolution_t;

typedef enum
{
	RECTIME_1MIN,
	RECTIME_3MIN,
	RECTIME_5MIN,
	RECTIME_MAX,
}UI_RecordTime_t;

typedef enum
{
	PHOTOSELCAM_ITEM,
	PHOTOFUNC_ITEM,
	PHOTORES_ITEM,
	PHOTOITEM_MAX
}UI_PhotoSubMenuItemList_t;

typedef enum
{
	PHOTOFUNC_OFF,
	PHOTOFUNC_ON,
	PHOTOFUNC_MAX,
}UI_PhotoFunction_t;

typedef enum
{
	PHOTORES_3M,
	PHOTORES_5M,
	PHOTORES_12M,
	PHOTORES_MAX,
}UI_PhotoResolution_t;

typedef enum
{
	CAMERASET_ITEM,
	RECORDSET_ITEM,
	SYSTEMSET_ITEM,
	DUALSET_ITEM,
	QUADSET_ITEM,
	TRIGGERSET_ITEM,
	AUTOSCANSET_ITEM,
	POWERONSET_ITEM,
	PARKINGLINE_ITEM,
	GUIDELINE_ITEM,
	AI_ITEM,
	SETTINGITEM_MAX
}UI_SettingSubMenuItemList_t;

typedef enum
{
	AECFUNC_OFF,
	AECFUNC_ON,
	AECFUNC_MAX,
}UI_AecFunction_t;

typedef enum
{
	CCAFUNC_OFF,
	CCAFUNC_ON,
	CCAMODE_MAX,
}UI_CCAMode_t;

typedef enum
{
	YEAR_ITEM,
	MONTH_ITEM,
	DATE_ITEM,
	HOUR_ITEM,
	MIN_ITEM,
	SEC_ITEM,
	ALLCALE_ITEM,
}UI_CalendarItem_t;

typedef enum
{
	TRIGGER_DELAY_CAM1,
	TRIGGER_DELAY_CAM2,
	TRIGGER_DELAY_CAM3,
	TRIGGER_DELAY_CAM4,
	TRIGGER_DELAY_CAMSPLIT,
	TRIGGER_PRIORITY_CAM1,
	TRIGGER_PRIORITY_CAM2,
	TRIGGER_PRIORITY_CAM3,
	TRIGGER_PRIORITY_CAM4,
	TRIGGER_PRIORITY_CAMSPLIT,
	TRIGGER_DISPLAY_SETUP,
	TRIGGER_MAX
}UI_SubSub_TriggerMode_t;
typedef enum
{
	TRIGGER_DELAY,
	TRIGGER_PRIORITY,
	TRIGGER_SETUP,
	TRIGGER_SUBSUBSUB_MAX
}UI_SubSubSub_TriggerMode_t;
typedef enum
{
	AUTOSCAN_ONOFF_CAM1,
	AUTOSCAN_DELAY_CAM1,
	AUTOSCAN_ONOFF_CAM2,
	AUTOSCAN_DELAY_CAM2,
	AUTOSCAN_ONOFF_CAM3,
	AUTOSCAN_DELAY_CAM3,
	AUTOSCAN_ONOFF_CAM4,
	AUTOSCAN_DELAY_CAM4,
	AUTOSCAN_ONOFF_DUAL,
	AUTOSCAN_DELAY_DUAL,
	AUTOSCAN_ONOFF_QUAD,
	AUTOSCAN_DELAY_QUAD,
	AUTOSCAN_MAX
}UI_SubSub_AutoScanMode_t;
typedef enum
{
	AUTOSCAN_SELECT,
	AUTOSCAN_DELAY,
	AUTOSCAN_SUBSUBSUB_MAX
}UI_SubSubSub_AutoScanMode_t;
typedef enum
{
	POWERON_CAM1,
	POWERON_CAM2,
	POWERON_CAM3,
	POWERON_CAM4,
	POWERON_DUAL,
	POWERON_QUAD,
	POWERON_SCAN,
	POWERON_STANDBY,
	POWERON_LAST,
	POWERON_MAX
}UI_PowerOnMode_t;
typedef enum
{
	LASTMODE_CAM1,
	LASTMODE_CAM2,
	LASTMODE_CAM3,
	LASTMODE_CAM4,
	LASTMODE_DUAL,
	LASTMODE_QUAD,
	LASTMODE_SCAN,
	LASTMODE_STANDBY,
	LASTMODE_SCAN_STANDBY,
	LASTMODE_MAX
}UI_LastMode_t;
typedef enum
{
	CAMERA_BRIGHTNESS,
	CAMERA_CONTRAST,
	CAMERA_SATURATION,
	CAMERA_HUE,
	CAMERA_VOLUME,
	CAMERA_H_MIRROR,
	CAMERA_V_MIRROR,
	CAMERA_LASER,
	CAMERA_LED,
	CAMERA_MAX
}UI_CamsSubSubSubMenuItemList_t;
typedef enum
{
	PARKINGLINE_CAM1_ONOFF,
	PARKINGLINE_CAM1_CAL,
	PARKINGLINE_CAM2_ONOFF,
	PARKINGLINE_CAM2_CAL,
	PARKINGLINE_CAM3_ONOFF,
	PARKINGLINE_CAM3_CAL,
	PARKINGLINE_CAM4_ONOFF,
	PARKINGLINE_CAM4_CAL,
//	GUIDE_BSD_BOX_SHOW_SWITCH,
	PARKINGLINE_MAX,
}UI_SubSub_ParkingLineMode_t;

typedef enum
{
	GUIDELINE_CAM1_ONOFF,
	GUIDELINE_CAM1_CAL,
	GUIDELINE_CAM2_ONOFF,
	GUIDELINE_CAM2_CAL,
	GUIDELINE_CAM3_ONOFF,
	GUIDELINE_CAM3_CAL,
	GUIDELINE_CAM4_ONOFF,
	GUIDELINE_CAM4_CAL,
	GUIDELINE_MAX,
}UI_SubSub_GuideLineMode_t;

typedef enum
{
//	AI_BSD_CAM1,
//	AI_BSD_CAM2,	
//	AI_BSD_CAM3,	
//	AI_BSD_CAM4,
	AI_PD_CAM1,
	AI_PD_CAM2,	
	AI_PD_CAM3,	
	AI_PD_CAM4,
	AI_CD_CAM1,
	AI_CD_CAM2,	
	AI_CD_CAM3,	
	AI_CD_CAM4,
	AI_ALARM_CAM1,
	AI_ALARM_CAM2,	
	AI_ALARM_CAM3,	
	AI_ALARM_CAM4,
	AI_TRIGGER_CAM1,
	AI_TRIGGER_CAM2,	
	AI_TRIGGER_CAM3,	
	AI_TRIGGER_CAM4,
	AI_BSDRANGE_CAM1,
	AI_BSDRANGE_CAM2,	
	AI_BSDRANGE_CAM3,	
	AI_BSDRANGE_CAM4,	
	AI_MAX,
}UI_SubSub_AIMode_t;
typedef enum
{
	PonitUpLeft=0, 
	PonitUpRight, 
	PonitDownLeft,
	PonitDownRight,
	SecondLine,
	ThirdLine,
	WholeArea,
	POINT_INDEX_MAX,
}UI_Select_Point;
typedef enum
{
	UnSelect,
	Selected,
}UI_Select_Mode;


#define ZoomH_Pixels 12   //bsd区域水平缩放时，一次移动12个像素
#define ZoomV_Pixels 12    //bsd区域竖向缩放时，一次移动12个像素

typedef enum
{
//	BSDRANGE_TYPE1_ITEM,//报警框类型一
//	BSDRANGE_TYPE2_ITEM,//报警框类型二
//	PERSON_DET_ITEM,//人检测
	BSDRANGE_ENABLE_ITEM,//检测范围
//	VEHICLES_DET_ITEM,//车检测
//	BSDALARM_ENABLE_ITEM,//报警音
//	BSDTRIGGER_OUT_ITEM,//触发输出	
	//以下是方向控件
	BSDRANGE_UP_ITEM,
	BSDRANGE_LEFT_ITEM,
	BSDRANGE_OK_ITEM,
	BSDRANGE_RIGHT_ITEM,
	BSDRANGE_DOWN_ITEM,
	//复位检测区域
	BSDRANGE_RESET_ITEM,
	AIITEM_MAX,
}UI_AISubSubMenuItemList_t;

typedef enum
{
	DATE_TIME_YEAR,
	DATE_TIME_MONTH,
	DATE_TIME_DAY,
	DATE_TIME_HOUR,
	DATE_TIME_MINUTE,
	DATE_TIME_SECOND,    
	DATE_TIME_UTC_SELECT,
    DATE_TIME_UTC_COMFIRM,
	DATE_TIME_MAX,
}UI_SubSub_DateTimeMode_t;
typedef enum
{
	PARKINGLINE_SELECT,
	PARKINGLINE_U_D,
	PARKINGLINE_L_R,
	PARKINGLINE_SIZE,
	PARKINGLINE_SUBSUBSUB_MAX
}UI_SubSubSub_ParkingLineMode_t;
typedef enum
{
	SDCARD_FORMAT_YES,
	SDCARD_FORMAT_NO,
	SDCARD_FORMAT_MAX
}UI_SubSubSub_FormatSdCardMode_t;
typedef enum
{
	KEYBOARD_KEY1,
	KEYBOARD_KEY2,
	KEYBOARD_KEY3,
	KEYBOARD_KEY4,
	KEYBOARD_KEY5,
	KEYBOARD_KEY6,
	KEYBOARD_KEY7,
	KEYBOARD_KEY8,
	KEYBOARD_KEY9,
	KEYBOARD_KEY0,
	KEYBOARD_KEY_ENTER,
	KEYBOARD_KEY_DELETE,
	KEYBOARD_KEY_MAX,
}UI_KeyBoard_t;

typedef enum
{
	SYSTEM_UPGRADE_YES,
	SYSTEM_UPGRADE_NO,
	SYSTEM_UPGRADE_MAX,
}UI_SystemUpgrade_t;


typedef enum
{
	LANGUAGE_ENGLISH,
	LANGUAGE_GERMAN,
	LANGUAGE_FRENCH,
	LANGUAGE_CHINESE,
	LANGUAGE_MAX,
}UI_Language_t;

typedef enum
{
	PARKING_LINE_OFF,
	PARKING_LINE_ON,
	PARKING_LINE_AUTO,
	PARKING_LINE_MAX
}UI_ParkingLine_t;

typedef struct
{
	void (*pvFuncPtr)(UI_ArrowKey_t);
}UI_MenuFuncPtr_t;
typedef struct
{
	void (*pvFuncPtr)(TOUCH_EVENT_t *TouchInf);
}UI_MenuTouchFuncPtr_t;

typedef struct
{
	void (*pvFuncPtr)(void);
}UI_DrawSubMenuFuncPtr_t;

typedef struct
{
	uint8_t 	 ubItemIdx;
	uint8_t 	 ubItemPreIdx;
}UI_MenuItem_t;

typedef struct
{
	uint8_t 	  ubFirstItem;
	uint8_t 	  ubItemCount;
	UI_MenuItem_t tSubMenuInfo;
}UI_SubMenuItem_t;

typedef struct
{
	UI_CamNum_t tCamNum4CamSetSub;
	UI_CamNum_t tCamNum4CamView;	
	UI_CamNum_t tCamNum4RecSub;
	UI_CamNum_t tCamNum4PhotoSub;
}UI_SubMenuCamNum_t;

typedef struct
{
	UI_SubMenuItem_t tCameraViewPage;
}UI_CamsSubSubMenuItem_t;


typedef struct
{


	UI_SubMenuItem_t tSettingS[SETTINGITEM_MAX];
}UI_SettingSubSubMenuItem_t;


typedef enum
{
	UI_OsdUpdate,
	UI_OsdErase,
}UI_OsdImgFnType_t;

typedef struct
{
	UI_CamViewType_t tCamViewType;
	UI_CamNum_t 	 tCamViewPool[CAM_4T];
}UI_CamViewSelect_t;

typedef struct
{
	uint32_t ulCardSize;
	uint32_t ulRemainSize;
}UI_SdInfo_t;

typedef enum
{
	UI_SD_CFM,
	UI_SD_NRDY,
	UI_SD_RDY,
}UI_SdSts_t;

typedef enum
{
	RECLOOP_MODE,
	RECMANU_MODE,
	PHOTO_MODE,
	RECPHOTO_MODE,
	VDOMODE_MAX
}UI_CamVdoMode_t;

typedef enum
{
	ANT_NOSIGNAL,
	ANT_SIGNALLVL1,
	ANT_SIGNALLVL2,
	ANT_SIGNALLVL3,
	ANT_SIGNALLVL4,
	ANT_SIGNALLVL5,
}UI_AntLvl_t;

typedef enum
{
	BAT_NO,
	BAT_LVL0,
	BAT_LVL1,
	BAT_LVL2,
	BAT_LVL3,
	BAT_LVL4,
	BAT_FULL,
	BAT_CHARGE,
}UI_BatLvl_t;

typedef enum
{
	BL_LVL0,
	BL_LVL1,
	BL_LVL2,
	BL_LVL3,
	BL_LVL4,
	BL_LVL5,
}UI_BrightnessLvl_t;

typedef struct
{
	UI_CamNum_t 			tPairSelCam;
	UI_DisplayLocation_t 	tDispLocation;
	uint8_t					ubDrawFlag;
}UI_PairingInfo_t;

typedef struct
{
	UI_AntLvl_t tAntLvl;
	uint8_t 	ubRssiValue;
}UI_RssiMap2AntLvl_t;

typedef struct
{
	UI_AntLvl_t tAntLvl;
	uint8_t 	ubPerValue;
}UI_PerMap2AntLvl_t;
typedef struct
{
	UI_BatLvl_t tBatLvl;
	uint8_t 	ubValue;
}UI_Map2BatLvl_t;

typedef enum
{
	UI_SCALEUP_2X = 2,
	UI_SCALEUP_4X,
}UI_ScaleUp_t;

typedef enum
{
	DUAL_CAM1_CAM2,
	DUAL_CAM1_CAM3,
	DUAL_CAM1_CAM4,
	DUAL_CAM2_CAM1,
	DUAL_CAM2_CAM3,
	DUAL_CAM2_CAM4,
	DUAL_CAM3_CAM1,
	DUAL_CAM3_CAM2,
	DUAL_CAM3_CAM4,
	DUAL_CAM4_CAM1,
	DUAL_CAM4_CAM2,
	DUAL_CAM4_CAM3,
	DUAL_VIEWCAMSEL_MAX,
}UI_DualViewCamSel_t;
	
typedef enum
{
	UI_CAMISP_SETUP,
	UI_CAMFUNC_SETUP,
}UI_CameraSettingMenu_t;

typedef enum
{
	UI_COLOR_ITEM,
	UI_DPTZ_ITEM,
	UI_MD_ITEM,
}UI_CameraSettingItem_t;

typedef struct
{
	LCD_DYN_INFOR_TYP tUI_LcdCropParam;
	UI_ScaleUp_t	  tScaleParam;
}UI_DPTZParam_t;

typedef struct
{
	uint8_t ubColorBL;
	uint8_t ubColorContrast;
	uint8_t ubColorSaturation;
	uint8_t ubColorHue;
}UI_ColorParam_t;

typedef enum
{
	UI_OSDLDDISP_OFF,
	UI_OSDLDDISP_ON,
}UI_OsdLdDispSts_t;

typedef enum
{
	UI_SEARCH_DCIMFOLDER,
	UI_SEARCH_RECFILES,
	UI_SDCARD_FORMAT,
	UI_LD_DEFU,
}UI_LoadingState_t;

typedef struct
{
	uint16_t uwFunc;
	uint8_t  ubType;
	uint8_t  ubCamNum;
	uint16_t uwFuncMsg;
}UI_FuncExecMsg_t;
	
typedef enum
{
	UI_SDCARDFMT_ACT,
	UI_PERDBGRPT_ACT,
}UI_FuncWorkAct_t;

typedef enum
{
	UI_RECORDING_MODE,
	UI_PHOTOCAP_MODE,
	UI_VDOMODE_MAX
}UI_VdoMode_t;

typedef enum
{
	UI_VDORECLOOP_MODE,
	UI_VDORECMANU_MODE,
	UI_VDORECTRIG_MODE,
	UI_VDOPHOTO_MODE,
	UI_VDOMODELIST_MAX
}UI_VdoModeList_t;

typedef enum
{
	UI_RECIMG_COLOR1,
	UI_RECIMG_COLOR2,
	UI_RECIMG_COLOR3,
	UI_RECIMG_COLOR4,
	UI_RECIMG_COLOR5,
	UI_RECIMG_COLOR6,
	UI_RECIMG_DEFU = 0xFF,
}UI_RecImgColor_t;

typedef enum
{
	UI_REC_STOP,
	UI_REC_START,
}UI_RecordingAct_t;

typedef enum
{
	UI_RECFILE_PAUSE,
	UI_RECFILE_PLAY,
	UI_RECFILE_STOP,
}UI_RecPlayStatus_t;

typedef enum
{
	UI_RECADOSRCSEL_ITEM,
	UI_RECSKIPBKFWD_ITEM,
	UI_RECPLAYPAUSE_ITEM,
	UI_RECSKIPFRFWD_ITEM,
	UI_RECRETURN_ITEM,
	UI_RECPLAYLISTITEM_MAX
}UI_RecPlayListItem_t;


typedef enum
{
	UI_PLAYADOSRC1_ITEM,

	UI_PLAYADOSRC2_ITEM,

	UI_PLAYADOSRC3_ITEM,
	UI_PLAYADOSRC4_ITEM,
	UI_PLAYADOSRC_MAX
}UI_PlayAdoSrcListItem_t;

#if 1

typedef struct
{
	uint16_t uwUI_RecNumArray[10];
	uint16_t uwUI_RecUpperLetterArray[26];
	uint16_t uwUI_RecLowerLetterArray[26];
	uint16_t uwUI_RecSymbolArray[4];
}UI_RecOsdImgDb_t;
#endif

#if 1
typedef struct
{
		uint8_t uwTotalRecFolderNum;
		uint8_t uwRecFolderSelIdx;
	KNL_FOLDERSINFO_t tRecFolderInfo[KNL_TOTAL_SUB_FLD_NUM];
}UI_RecFoldersInfo_t;
#endif

#if 1
typedef struct
{
	uint16_t uwTotalRecFileNum;
	uint16_t uwRecFileSelIdx;
	KNL_FILESINFO_t tRecFilesInfo[KNL_MAX_FILE_NUM];
}UI_RecFilesInfo_t;
#endif

#if 0
typedef struct
{
	UI_RecordingAct_t tRecAct;
	uint8_t ubPlayMode;
	UI_RecPlayStatus_t tPlaySts;
	UI_CamNum_t tVdoPlayCam[4];
	UI_CamNum_t tAdoPlayCam;
}UI_RecPlayAct_t;
#endif
#if 0
typedef enum
{
	UI_RECPLAY_MULTIVIEW,
	UI_RECPLAY_SINGLEVIEW,
}UI_RecPlayDispType_t;
#endif
#if 0
typedef enum
{
	UI_WARN_RECMODE,
}UI_WarnNote_t;
#endif

#if 0
typedef struct
{
	uint16_t uwYear;
	uint8_t  ubMonth;
	uint8_t  ubDate;
	uint8_t  ubHour;
	uint8_t  ubMin;
	uint8_t  ubSec;
}UI_Calendar_t;
#endif
typedef struct 
{
	int xPoint;
	int yPoint;
}UI_LinePoint_t;

typedef struct 
{
	UI_LinePoint_t tupLeft;
	UI_LinePoint_t tupRight;
	//中点坐标预留不使用
	UI_LinePoint_t tmid_upLeft;
	UI_LinePoint_t tmid_upRight;
	UI_LinePoint_t tmid_downLeft;
	UI_LinePoint_t tmid_downRight;
	UI_LinePoint_t tdownLeft;
	UI_LinePoint_t tdownRight;
	/* data */
}UI_ParkinglinePoint_t;

#define REC_FOLDER_LIST_MAXNUM	10
#define REC_FILE_LIST_MAXNUM	8
#pragma pack(push) /* push current alignment to stack */
#pragma pack(1) /* set alignment to 1 byte boundary */
#if 0
	typedef struct
	{
		uint16_t uwUI_RecNumArray[10];
		uint16_t uwUI_RecUpperLetterArray[26];
		uint16_t uwUI_RecLowerLetterArray[26];
		uint16_t uwUI_RecSymbolArray[4];
	}UI_RecOsdImgDb_t;
#endif
#if 0
	typedef struct
	{
		uint8_t uwTotalRecFileNum;
		uint8_t uwRecFolderSelIdx;
		KNL_FOLDERSINFO_t tRecFolderInfo[KNL_TOTAL_SUB_FLD_NUM];
	}UI_RecFoldersInfo_t;
#endif
#if 0
	typedef struct
	{
		uint16_t uwTotalRecFileNum;
		uint16_t uwRecFileSelIdx;
		KNL_FILESINFO_t tRecFilesInfo[KNL_MAX_FILE_NUM];
	}UI_RecFilesInfo_t;
#endif
#if 1
	typedef struct
	{
		UI_RecordingAct_t tRecAct;
		uint8_t ubPlayMode;
		UI_RecPlayStatus_t tPlaySts;
		UI_CamNum_t tVdoPlayCam[4];
		UI_CamNum_t tAdoPlayCam;
	}UI_RecPlayAct_t;
#endif
#if 1
typedef enum
{
		UI_RECPLAY_MULTIVIEW,
		UI_RECPLAY_SINGLEVIEW,
	}UI_RecPlayDispType_t;
#endif
#if 1
	typedef enum
	{
		UI_WARN_RECMODE,
	}UI_WarnNote_t;
#endif

#if 1
	typedef struct
	{
		uint16_t uwYear;
		uint8_t  ubMonth;
		uint8_t  ubDate;
		uint8_t  ubHour;
		uint8_t  ubMin;
		uint8_t  ubSec;
	}UI_Calendar_t;
#endif
typedef struct
{
	uint32_t 			  ulCAM_ID;
	UI_DisplayLocation_t  tCamDispLocation;
	UI_CamConnectStatus_t tCamConnSts;
	UI_CamVdoMode_t		  tCamVdoMode;
	UI_AntLvl_t			  tCamAntLvl;
	UI_BatLvl_t 		  tCamBatLvl;
	UI_SdInfo_t 		  tSdInfo;
	UI_RecordMode_t		  tREC_Mode;
	UI_RecordResolution_t tREC_Resolution;
	UI_PhotoFunction_t	  tPHOTO_Func;
	UI_PhotoResolution_t  tPHOTO_Resolution;
	UI_CamsSetMode_t	  tCamAnrMode;
	UI_CamsSetMode_t	  tCam3DNRMode;
	UI_CamsSetMode_t	  tCamvLDCMode;
	UI_CamsSetMode_t	  tCamAecMode;
	UI_CamsSetMode_t	  tCamDisMode;
	UI_CamFlicker_t		  tCamFlicker;
	UI_CamsSetMode_t	  tCamCbrMode;
	UI_CamsSetMode_t	  tCamCondenseMode;
	UI_ColorParam_t		  tCamColorParam;
	struct
	{
		uint8_t			  ubMD_Mode;
		uint8_t 		  ubMD_Param[4];
	}MdParam;
	UI_PowerSaveMode_t	  tCamPsMode;
	UI_CamsSetMode_t	  tCamScanMode;
	UI_CamImgFlip_t		  tCamImgFlip;
	UI_CamImgMirror_t	  tCamImgMirror;
	UI_CamLaser_t         tCamLaser;   //Alan
	UI_CamLed_t           tCamLed; 
	UI_DisplayLocation_t  tCamDispLocation_Dual;
	UI_DisplayLocation_t  tCamDispLocation_Quad;
	uint8_t ubVolumeLvl;
	char 				  cCamVersion[30];
	uint32_t              ulFrameCount;
	uint32_t			  ulImageStuckCount;
	uint32_t			  ulFps;
	uint8_t				  ubReserved[163];

}UI_CamStatus_t;
#pragma pack (1)/*指定按1字节对齐*/
typedef struct
{
	uint8_t 				ubTotalCamNum;
	uint8_t 				ubPairedCamNum;
	UI_Calendar_t 			tSysCalendar;
	uint8_t 				ubAEC_Mode;
	uint8_t 				ubCCA_Mode;
	uint8_t 				ubSTORAGE_Mode;
	struct
	{
		uint8_t				ubDrawStsIconFlag;
		uint8_t				ubRdPairIconFlag;
		uint8_t				ubClearThdCntFlag;
		uint8_t 			ubShowLostLogoFlag;
		uint8_t				ubDrawNoCardIconFlag;
		uint8_t 			ubDrawMdTrigFlag;
	}IconSts;
	struct
	{
		uint32_t			ulBL_UpdateLvL;
		uint8_t				ubBL_UpdateCnt;
	}BriLvL;
	struct
	{
		uint32_t			ulVOL_UpdateLvL;
		uint8_t				ubVOL_UpdateCnt;
	}VolLvL;
	
	UI_PowerSaveMode_t		tPsMode;
	struct
	{
		UI_RecordMode_t		tREC_Mode;
		UI_RecordTime_t		tREC_Time;
	}RecInfo;
	UI_VdoMode_t			tVdoMode;
	uint8_t					ubVdoRecStsCnt;
	struct
	{
		UI_WarnNote_t 		tWarnNote;
		uint8_t				ubWarnUpdateCnt;
	}WarnIcon;
	UI_CamsBwMode_t			tCamsBwMode;
	struct
	{
		UI_CamNum_t			tAutoScanCamPool[4];
		uint8_t 			ubScanDuty;
	}AutoScan;
	uint8_t					ubRFArgData[2];
	UI_PowerOnMode_t  tPowerOnMode; //209
	uint8_t TriggerPriority[5];//204  
	uint8_t TriggerDelay[5];//199;
	UI_CamNum_t TriggerSplitView;//195
	uint8_t AutoScanDuty[7];//188;
	uint8_t AutoScanEnable[7];//181;  cam1  cam2  cam3 cam4 dual h quad
	uint8_t ParkingLineEnable[4];//177  0: dis  1: en 
	uint8_t ParkingLine_XY[4][4];//161 cam0 x = ParkingLine_XY[0][0]<< 8 + ParkingLine_XY[0][1]  y = ParkingLine_XY[0][2]<< 8 + ParkingLine_XY[0][3]  
	uint8_t ParkingLineSize[4];//157   0~9
	uint8_t ubMenuLock;//156   
	uint8_t ubBuzzer;//155	  
	uint8_t ubPowerOnRecord;//154
	UI_CamSource tAdoSrcCamNum_Dual;//150
	UI_CamSource tAdoSrcCamNum_Single[4];//146 
	UI_CamSource tAdoSrcCamNum_Quad;// s  
	uint8_t ubAutoDimmer;	//141
	uint8_t ubAutoDimmer_MinVal; //140
	uint8_t ubAutoDimmer_MaxVal; //139
	uint8_t ubMuteFlag;//138
	UI_LastMode_t tLastMode;//134
	UI_CamNum_t tLastView;//130
	UI_PowerOnMode_t  tOldPowerOnMode; //126
	
	uint8_t        ubUpdateTxParam[4];
	int8_t         bColorBLGain[4];
	int8_t         bColorContrastGain[4];
	int8_t         bColorSaturationGain[4];
	int8_t         bColorHueGain[4];
	
	uint8_t GuideLineEnable[4];
	uint8_t GuideLine_XY[4][4];
	uint8_t GuideLineSize[4];

	UI_Language_t tLanguage;//78
	UI_ParkingLine_t tParkingLineEnable[4]; //62
	uint8_t        ubHideChannelName;//61
 	uint8_t        ubSD_Switch;//60
 	uint8_t ubtimezone;//59
 	uint8_t ubTimeStame;//58
 	
	//uint8_t ubIsEnableBSD[4];//54	
	uint8_t ubIsEnableBSDALARM[4];//54
	uint8_t ubDetectPeopleFlag[4];//50
	uint8_t ubDetectCarFlag[4];//46
	uint8_t ubIsEnableBSDRANGE[4];//42
	uint8_t ubBSDTriggerOut[4];//38
	//uint8_t ubBSDBoxType;//37
	UI_ParkinglinePoint_t tUI_AIDetectlinePoint[4];//4*8*2*2
	uint8_t tLocation1[4];
	uint8_t tLocation2[4];
	uint8_t AI_ubVolumeLvl;
	uint8_t	ubReserved[4];
}UI_CUSetting_t;
#pragma pack () /*取消指定对齐，恢复缺省对齐*/

typedef struct
{
	char 		   cbUI_DevStsTag[11];
	char 		   cbUI_FwVersion[11];
	UI_CUSetting_t tCU_SettingInfo;
	UI_CamStatus_t tCAM_StatusInfo[CAM_4T];
}UI_DeviceStatusInfo_t;

#pragma pack(pop)
typedef struct
{
	uint16_t uwUI_NumArray[10];
	uint16_t uwUI_UpperLetterArray[26];
	uint16_t uwUI_LowerLetterArray[26];
	uint16_t uwUI_SymbolArray[4];
}UI_CharOsdImgDb_t;

typedef enum
{
	UI_ADO_ADCSAMPLERATE,
	UI_ADO_ADCVOLL,
	UI_ADO_ADCVOLR,
	UI_ADO_ADCMUTE,
	UI_ADO_DACSAMPLERATE,
	UI_ADO_DACVOL,
	UI_ADO_DACMUTE,
	UI_ADO_UNKNOW,
}UI_AdoSetItem_t;

#define UI_CLEAR_THREADCNT(Flag, Count)				do { if(Flag == TRUE) { (Count) = 0; Flag = FALSE; } } while(0)
#define UI_CLEAR_CAMSETTINGTODEFU(xFUNC, mDEFU)		do { xFUNC = mDEFU; } while(0)
#define UI_CLEAR_CALENDAR_TODEFU(xFUNC, mDEFU)		do { xFUNC = mDEFU; } while(0)

#define UI_CHK_CAMSFUNCS(Mode, Status)				do { if(Mode > CAMSET_ON) { Mode = Status; } } while(0)
#define UI_CHK_CAMFLICKER(HZ)						do { if(HZ > CAMFLICKER_60HZ) { HZ = CAMFLICKER_50HZ; } } while(0)
#define UI_CHK_CAMPARAM(Param, Value)				do { if(Param >= 128) { Param = Value; } } while(0)
#define UI_CHK_CAMSYS(SysParam, Limit, Target)		do { if(SysParam >= Limit) { SysParam = Target; } } while(0)
#define UI_CHK_CAMIMGSET(Mode, Limit, Target)		do { if(Mode > Limit) { Mode = Target; } } while(0)
#define UI_CHK_CUSYS(SysParam, Limit, Target)		do { if(SysParam >= Limit) { SysParam = Target; } } while(0)
#define UI_CHK_MYSYS(SysParam, Limit, Target)		do { if(SysParam >= Limit) { SysParam = Target; return rUI_FAIL;} } while(0)

//! Two way command timeout
#define UI_TWC_TIMEOUT		(3 * 1000)				//! Unit: ms
//! Two way command format of UI
#define UI_TWC_TYPE			0
#define UI_REPORT_ITEM		1
#define UI_REPORT_DATA		2
#define UI_SETTING_ITEM		1
#define UI_SETTING_DATA		2

typedef enum
{
	UI_REPORT,
	UI_SETTING,
}UI_TwcDataType_t;

typedef enum
{
	UI_UPDATE_CAMSTS = 0x20,
	UI_VOX_TRIG,
	UI_MD_TRIG,
	UI_VOICE_TRIG,
}UI_CamReqCmdID_t;

typedef enum
{
	UI_PTZ_SETTING = 1,
	UI_RECMODE_SETTING,
	UI_RECRES_SETTING,
	UI_SDCARD_SETTING,
	UI_PHOTOMODE_SETTING,
	UI_PHOTORES_SETTING,
	UI_SYSINFO_SETTING,
	UI_VOXMODE_SETTING,
	UI_ECOMODE_SETTING,
	UI_WORMODE_SETTING,
	UI_ADOANR_SETTING,
	UI_ADOAEC_SETTING,
	UI_IMGPROC_SETTING,
	UI_MD_SETTING,
	UI_VOICETRIG_SETTING,
	UI_PERDBGMODE_SETTING,
	UI_SAVEPARAMETERS,
}UI_CUReqCmdID_t;

typedef enum
{
	UI_IMG3DNR_SETTING,
	UI_IMGvLDC_SETTING,
	UI_IMGWDR_SETTING,
	UI_IMGDIS_SETTING,
	UI_IMGCBR_SETTING,
	UI_IMGCONDENSE_SETTING,
	UI_FLICKER_SETTING,
	UI_IMGBL_SETTING,
	UI_IMGCONTRAST_SETTING,
	UI_IMGSATURATION_SETTING,
	UI_IMGHUE_SETTING,
	UI_IMGFLIP_SETTING,
	UI_IMGMIRROR_SETTING,
	UI_IMGSETTING_MAX = 20,
}UI_ImgProcSettingItem_t;

#pragma pack(push) /* push current alignment to stack */
#pragma pack(1) /* set alignment to 1 byte boundary */
typedef struct
{
	UI_CamNum_t 	tDS_CamNum;
	uint8_t 		ubCmd_Len;
	uint8_t			ubCmd[8];
}UI_CUReqCmd_t;

typedef struct
{
	osThreadId	thread_id;
	int32_t		iSignals;
	UI_Result_t	tReportSts;
}UI_ThreadNotify_t;
#pragma pack(pop)

typedef struct
{
	void (*pvAction)(UI_CamNum_t, void *);
}UI_ReportFuncPtr_t;

typedef struct
{
	void (*pvAction)(UI_CamNum_t, void *);
}UI_SettingFuncPtr_t;
#define DATA_LENGTH_MAX 30
typedef struct
{
	uint16_t x1; //左上角x坐标��?
	uint16_t y1; //左上角y坐标��?
	uint16_t x2; //右下角x坐标��?
	uint16_t y2; //右下角y坐标��?
	uint8_t alarm_type;//画框颜色 1：green 2：yellow 3：red
} Algo_Pos;
 
typedef struct
{
	uint32_t chn;
	uint32_t P_OR_C;//Person or Car
	uint32_t cnt;
	Algo_Pos pos[DATA_LENGTH_MAX];
} Algo_Result;


void UI_PowerKey(void);
void UI_PowerLongKey(void);
void UI_MenuKey(void);
void UI_UpArrowKey(void);
void UI_DownArrowKey(void);
void UI_LeftArrowKey(void);
void UI_RightArrowKey(void);
void UI_EnterKey(void);
void UI_SelKey(void);
void UI_PairingKey(void);
void UI_MirrorKey(void);
void UI_MuteKey(void);
void UI_VersionKey(void);
//void UI_FWUSD_1126Key(void);

void UI_DrawBSDRange(void);
UI_Result_t UI_DPTZ_KeyPress(uint8_t ubKeyID, uint8_t ubKeyMapIdx);
void UI_DPTZ_KeyRelease(uint8_t ubKeyID);
void UI_CameraSelection4DualView(UI_ArrowKey_t tArrowKey);
void UI_DisplayArrowKeyFunc(UI_ArrowKey_t tArrowKey);
void UI_DrawMenuPage(void);
void UI_DrawSubMenuPage(UI_MenuItemList_t MenuItem);
void UI_HomeMenu_Key(UI_ArrowKey_t tArrowKey);
void UI_SubKeyMenu(UI_ArrowKey_t tArrowKey);
void UI_SubSubKeyMenu(UI_ArrowKey_t tArrowKey);
void UI_SubSubSubKeyMenu(UI_ArrowKey_t tArrowKey);
void UI_CameraSettingSubSubMenuPage(UI_ArrowKey_t tArrowKey);
void UI_PairingSubMenuPage(UI_ArrowKey_t tArrowKey);
void UI_PairingSubSubSubMenuPage(UI_ArrowKey_t tArrowKey);
void UI_DrawPairingStatusIcon(void);
void UI_ReportPairingResult(UI_Result_t tResult);
void UI_ReportAppPairingResult(UI_Result_t tResult);
void UI_DrawDCIMFolderMenu(void);
void UI_DrawRecordFileMenu(void);
void UI_DCIMFolderSelection(UI_ArrowKey_t tArrowKey);
void UI_PlayDispTypeSelection(UI_ArrowKey_t tArrowKey);
void UI_PlaybackSubMenuPage(UI_ArrowKey_t tArrowKey);
void UI_CamSelSubMenuPage(UI_ArrowKey_t tArrowKey);
void UI_AdoSelSubMenuPage(UI_ArrowKey_t tArrowKey);
void UI_PowerSubMenuPage(UI_ArrowKey_t tArrowKey);
void UI_SettingSysDateTimeSubSubMenuPage(UI_ArrowKey_t tArrowKey);
void UI_DrawHLandNormalIcon(uint16_t uwNormalOsdImgIdx, uint16_t uwHighLigthOsdImgIdx);
UI_MenuAct_t UI_KeyEventMap2SubMenuInfo(UI_ArrowKey_t *ptArrowKey, UI_SubMenuItem_t *ptSubMenu);
UI_MenuAct_t UI_KeyEventMap2SubSubMenuInfo(UI_ArrowKey_t *ptArrowKey, UI_SubMenuItem_t *ptSubSubMenuItem);
void UI_ReportCamConnectionStatus(void *pvConnectionSts);
void UI_UpdateCamStatus(UI_CamNum_t tCamNum, void *pvStatus);
void UI_UnBindCam(UI_CamNum_t tUI_DelCam);
void UI_VoxTrigger(UI_CamNum_t tCamNum, void *pvTrig);
void UI_EnableVox(void);
void UI_DisableVox(void);
void UI_DisableCuAdoOnlyMode(void);
void UI_MDTrigger(UI_CamNum_t tCamNum, void *pvTrig);
void UI_VoiceTrigger(UI_CamNum_t tCamNum, void *pvTrig);
UI_Result_t UI_SendLaserorLedToCAM(osThreadId thread_id, UI_CUReqCmd_t *ptReqCmd,TWC_OPC Opc);//Alan
UI_Result_t UI_SendRequestToCAM(osThreadId thread_id, UI_CUReqCmd_t *ptReqCmd);
void UI_RecvCamResponse(TWC_TAG tRecv_StaNum, TWC_STATUS tStatus);
void UI_RecvCamRequest(TWC_TAG tRecv_StaNum, uint8_t *pTwc_Data);
void UI_LoadDevStatusInfo(void);
void UI_UpdateDevStatusInfo(void);
void UI_UpdateTxDevStatusInfo(UI_CamNum_t tCamNum);

void UI_CheckScanModeCount(void);
void UI_SwitchCameraSource(void);
void UI_SwitchAudioSource(UI_CamSource tCamNum);
void UI_EngModeKey(void);
void UI_EngModeCtrl(UI_ArrowKey_t tArrowKey);
void UI_DisplayAppPairingScreen(void);
void UI_PairingControl(UI_ArrowKey_t tArrowKey);
void UI_FwUpgViaSdCard(void);
void UI_UpdateRecStsIcon(void);
void UI_UpdateWarningNoteIcon(void);
void UI_SdCardFormatFunc(UI_ArrowKey_t tArrowKey);
void UI_VideoRecordingExec(UI_RecordingAct_t tRecAct);
void UI_OsdDisplayFrmErrItem(UI_DisplayLocation_t tDispLoc);
void UI_DisplayTrxInfo(UI_FuncExecMsg_t tPerRpt);
//! SPRF
void UI_MDTriggerSetting(void);
void UI_1MSTimerInit(void);
void UI_CheckScanModeCount(void);
void UI_ScanModeExec(void);
void UI_CheckPanelOffCount(void);
void UI_CheckTriggerOverCount(void);
void UI_DisplayTouchFunc(TOUCH_EVENT_t *Touch_Info);
void UI_DrawVolumeMenuPage(void);
void UI_HomeMenuTouchFunc(TOUCH_EVENT_t *Touch_Info);
void UI_VolumeMenuPage(UI_ArrowKey_t tArrowKey);
void UI_VolumeMenuTouchFunc(TOUCH_EVENT_t *Touch_Info);
void UI_ShowSdCardInfo(UI_ArrowKey_t tArrowKey);
void UI_ShowSdCardInfoTouchFunc(TOUCH_EVENT_t *Touch_Info);
void UI_ShowTxVersion(UI_ArrowKey_t tArrowKey);
void UI_ShowTxVersionTouchFunc(TOUCH_EVENT_t *Touch_Info);
void UI_SubTouchMenu(TOUCH_EVENT_t *Touch_Info);
void UI_SubSubTouchMenu(TOUCH_EVENT_t *Touch_Info);
void UI_PairingSubSubTouchMenuPage(TOUCH_EVENT_t *Touch_Info);
void UI_SubSubSubTouchMenu(TOUCH_EVENT_t *Touch_Info);
void UI_PairingSubSubSubTouchMenu(TOUCH_EVENT_t *Touch_Info);
void UI_PairingTouchControl(TOUCH_EVENT_t *Touch_Info);
void UI_SetRecImgColor(UI_RecImgColor_t tColorNum);
void UI_ClearOsdImage(void);
void UI_SwitchViewType(UI_CamNum_t tUI_CamSel,uint8_t ubClosePanel);
void UI_SettingSubSubSubMenuPage(UI_ArrowKey_t tArrowKey);
void UI_SettingSubSubSubTouchMenu(TOUCH_EVENT_t *Touch_Info);
void UI_PlaybackSubSubMenuPage(UI_ArrowKey_t tArrowKey);
void UI_PlaybackSubSubTouchMenuPage(TOUCH_EVENT_t *Touch_Info);
void UI_PlayDispTypeTouchSelection(TOUCH_EVENT_t *Touch_Info);
void UI_PlaybackSubSubSubTouchMenu(TOUCH_EVENT_t *Touch_Info);
void UI_PlaybackSubSubSubMenuPage(UI_ArrowKey_t tArrowKey);
void UI_SetLdDispStatus(UI_OsdLdDispSts_t tDispSts);
void UI_SettingSubSubSubSubMenuPage(UI_ArrowKey_t tArrowKey);
void UI_SettingSubSubSubSubTouchMenu(TOUCH_EVENT_t *Touch_Info);
void UI_SubSubSubSubTouchMenu(TOUCH_EVENT_t *Touch_Info);
void UI_SubSubSubSubKeyMenu(UI_ArrowKey_t tArrowKey);
void UI_ShowKeyBoardValue(uint16_t uwValue,OSD_UPDATE_TYP tUpdateMod);
void UI_ShowButtonValueNormal(uint16_t uwValue,uint16_t uwXStart,uint16_t uwYStart,OSD_UPDATE_TYP tUpdateMod);
void UI_ShowButtonValueHighLight(uint16_t uwValue,uint16_t uwXStart,uint16_t uwYStart,OSD_UPDATE_TYP tUpdateMod);
void UI_ShowNumber(uint16_t uwValue,uint16_t uwXStart,uint16_t uwYStart,uint16_t uwNum0_ImageIndex,OSD_UPDATE_TYP tUpdateMod);
long UI_Map(long x, long in_min, long in_max, long out_min, long out_max);
void UI_TriggerEventExec(void *pvTouchEvent);
void UI_DrawParkingLine(uint8_t ubCamNum,OSD_UPDATE_TYP tMode);
void UI_EraseParkingLine(uint8_t ubCamNum,OSD_UPDATE_TYP tMode);
void UI_DrawParkingLine_OSD1(uint8_t ubCamNum,OSD_UPDATE_TYP tMode);
void UI_EraseParkingLine_OSD1(uint8_t ubCamNum,OSD_UPDATE_TYP tMode);
void UI_DrawParkingLine_OSD1_DUAL(uint8_t ubCamNum,OSD_UPDATE_TYP tMode);
void UI_EraseParkingLine_OSD1_DUAL(uint8_t ubCamNum,OSD_UPDATE_TYP tMode);
void UI_DrawParkingLine_OSD1_QUAL(uint8_t ubCamNum,OSD_UPDATE_TYP tMode);
void UI_EraseParkingLine_OSD1_QUAL(uint8_t ubCamNum,OSD_UPDATE_TYP tMode);


void UI_DrawDesktopIcon(void);
void UI_ResetUIParameter(void);
void UI_PanelOn(void);
void UI_PanelOff(void);
void UI_AudioOn(void);
void UI_AudioOff(void);
UI_CamSource UI_GetAudioSrc(void);
void UI_SetAudioSrc(UI_CamSource tAdoSrc);
void UI_CheckMenuOffCount(void);
void UI_CheckPwrUartCount(void);
void UI_ShowRecordingStatus(bool is_flip);
void UI_BackToDesktop(uint8_t ubSwitchView);
void UI_ClearOsdImageNoUpdate(void);
void UI_WakeUpTx(void);
void UI_SleepTx(void);
void UI_DisconnectTx(void);
void UI_RefreshBandwidth(void);
void UI_WakeupStandby(void);
void UI_EnterStandby(void);
void UI_MenuLock(void);
void UI_CheckLightSensor(uint8_t ubFirstPanelOn);
void UI_InitCharArray(void);
void UI_ShowPlaybackPageNum(uint16_t uwValue,uint16_t uwXStart,uint16_t uwYStart,OSD_UPDATE_TYP tUpdateMod);
void UI_ShowPlayTime(uint16_t uwCurTime,uint16_t uwTotalTime,uint16_t uwXStart,uint16_t uwYStart);
void UI_SetTxImgProc(UI_CamNum_t tCamNum);
void UI_HandlePreviewStatus(void *ptAppStsReport);
void UI_SaveLastMode(void);
void UI_RecvTxType (TWC_TAG tRecv_StaNum, uint8_t *pData);
void UI_RecvTxVersion (TWC_TAG tRecv_StaNum, uint8_t *pData);
uint16_t uwUI_GetTxType(UI_CamNum_t tCamNum);
void UI_SetTxType(UI_CamNum_t tCamNum, uint16_t uwVal);
void UI_SyncTimeStamp2TX(void);
void UI_UART2_PutChar(char ch);
void UI_SendBSDRangeTo1126(UI_ParkinglinePoint_t tParkinglinePoint,uint8_t chn);
void UI_DRAW_BSDRANGE(UI_ParkinglinePoint_t tParkinglinePoint ,uint8_t location1 ,uint8_t location2,OSD_UPDATE_TYP OSD_UPDATE,OSD_LAYER_TYP OSD_TYP);
void UI_CONNECT_POINT3(UI_LinePoint_t tParkinglinePoint1 , UI_LinePoint_t tParkinglinePoint2 , UI_LinePoint_t tParkinglinePoint3, UI_LinePoint_t tParkinglinePoint4,uint8_t Location1,uint8_t Location2, OSD_LAYER_TYP OSD_TYP);
uint8_t UI_CheckTxVersion(char *pTxVersion,uint8_t ubTxVersionLen,char *pTxDst,uint8_t ubTxDst);


extern 	UI_MenuItem_t tUI_MenuItem;
extern UI_SubMenuItem_t tUI_SubMenuItem[MENUITEM_MAX];
extern UI_SubMenuItem_t tSystemSubSubMenuItem;
extern UI_SubMenuItem_t tDualSubSubMenuItem;
extern UI_SubMenuItem_t tQuadSubSubMenuItem;
extern UI_SubMenuItem_t tPowerOnSubSubMenuItem;

extern uint8_t ubUI_PowerOnSubSubStsUpdateFlag;
extern uint8_t ubUI_SubSubStsUpdateFlag;
extern UI_SettingSubSubMenuItem_t tSettingSubSubMenuItem;
extern uint8_t CameraSubSubSubRemoteMode;
extern UI_SubMenuItem_t *tCameraSubSubSubMenuItem[];
extern uint8_t SystemSubSubSubRemoteMode;
extern uint8_t AISubSubSubRemoteMode;

extern uint8_t SettingSubSubSubRemoteMode;
extern UI_SubMenuItem_t tSystemSubSubSubMenuItem[]; 
extern UI_SubMenuItem_t tDateTimeSubSubSubSubMenuItem;
extern UI_SubMenuItem_t tAISubSubSubMenuItem;


extern UI_MenuItem_t tUI_RecPlayListItem;
extern UI_MenuItem_t tUI_PlyDispTypeSelItem;
extern UI_MenuItem_t tUI_RecPlayAdoSrcItem;
extern UI_State_t tUI_State;
extern UI_CUSetting_t tUI_CuSetting;
extern UI_RecPlayAct_t tUI_RecPlayAct;
extern void UI_MenuKey(void);
extern uint16_t crc16_Gen(char *pu8Data, int u32Len);

extern UI_CamStatus_t tUI_CamStatus[CAM_4T];
extern UI_CamViewSelect_t tCamViewSel;
extern UI_CamNum_t tUI_ViewModeSel;
extern UI_CamNum_t tUI_SaveViewNum;
extern UI_CamNum_t tUI_AutoScanNumSel;
extern UI_RecFoldersInfo_t tUI_RecFoldersInfo;
extern OSD_IMGIDXARRARY_t tUI_RecOsdImgInfo;
extern APP_State_t tUI_SyncAppState;
extern UI_SdSts_t tUI_SdCardSts;
extern uint8_t ubUI_FinishViewSwitch;
extern uint8_t ubUI_RestartRec;
extern uint8_t ubUI_RestartRec;
extern uint8_t ubUI_CuStandbyFlag;
extern uint8_t ubUI_CuPowerOffFlag;
extern uint8_t ubUI_CuPowerDiscFlag;
extern uint16_t uwUI_ViewSwitchTime;
extern UI_RecFilesInfo_t tUI_RecFilesInfo;
extern osMessageQId osUI_OsdLdDispQueue;
extern uint8_t ubUI_CurDimmerLvl;
extern UI_PairingInfo_t tPairInfo;
extern ADO_R2R_VOL tUI_VOLTable[];
extern uint8_t MenuOnFlag;
extern uint8_t TriggerLock;
extern uint8_t TriggerSource;
extern UI_CamNum_t DeskTopShowView;
extern osMessageQId osUI_FuncsExecQue;
extern osMessageQId osUI_FuncsFinExecQue;
extern OSD_IMGIDXARRARY_t tUI_CharOsdImgInfo;
extern uint32_t ulMenuAutoOffCount;//��?3?2?�̣���1??����
extern uint32_t ulAutoScanCount;//��??������?��?-??��1??����
extern uint32_t ulPanelOffCount;//?��?����1??����
extern uint32_t ulTriggerOverCount;
extern uint32_t ulStartRecCompleteCount;
extern uint16_t uwCurVideoPlayIdx[4];
extern uint8_t ubUI_SubSubSubStsUpdateFlag;
extern uint8_t ubUI_DateTimeSubSubSubSubStsUpdateFlag;

extern uint8_t ParkingLineRemoteMode;
extern uint8_t ubAutoScanReady;
extern UI_CamNum_t TriggerBackUp_View;
extern osSemaphoreId osUI_CuTriggerCtr;
extern osSemaphoreId osUI_PowerLightCtr;

extern osSemaphoreId osUI_CuUiCtr;

extern uint8_t ubUI_TouchPanelSts;
extern osMessageQId UI_AI_clearBOXQueue;
extern uint8_t AILineRemoteMode;
extern uint8_t AILinePointIndex;
//extern UI_ParkinglinePoint_t UI_AIDetectlinePoint[4];
extern uint8_t KNL_UsbdFwuFg;
//extern uint8_t Location1[4],Location2[4];
extern uint16_t LINE2_Mid_XPoint,LINE2_Mid_YPoint,LINE3_Mid_XPoint,LINE3_Mid_YPoint;

extern uint8_t receivedVersion[VERSION_LEN];
extern uint8_t systemAIVersion[VERSION_LEN];
extern  uint8_t CheckSensorStatus[COMMAND_LENGTH];


extern char zoomchar[4];
extern char Laserchar[4];

/*PELCO code*/
extern uint8_t pelco_out[COMMAND_LENGTH];
extern uint8_t pelco_in[COMMAND_LENGTH];
extern uint8_t pelco_stop[COMMAND_LENGTH];

extern uint8_t pelco_near[COMMAND_LENGTH];  
extern uint8_t pelco_far[COMMAND_LENGTH];
extern uint8_t pelco_auto[COMMAND_LENGTH]; 
extern uint8_t pelco_restart[COMMAND_LENGTH];

extern uint8_t pelco_digital_zoom[COMMAND_LENGTH];

extern uint8_t pelco_BSD[14];   //= {0xFF, 0xA0, 0x00, 0x00, 0x00, 0x00, 0xA0, \
                         0xFF, 0xA0, 0x00, 0x01, 0x00, 0x00, 0xA1};
extern uint8_t pelco_Trace[14]; //= {0xFF, 0xB0, 0x00, 0x00, 0x00, 0x00, 0xB0, \
	                     0xFF, 0xB0, 0x00, 0x01, 0x00, 0x00, 0xB1};
extern  uint8_t CheckSensorStatus[COMMAND_LENGTH];

extern uint8_t CheckAlarmRoi[COMMAND_LENGTH];
extern uint8_t Picture_Mirror_Flip[COMMAND_LENGTH];

extern osMutexId osEnterVolumeFlag;


#endif
