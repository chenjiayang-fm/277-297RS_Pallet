/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		APP_HS.h
	\brief		Application header file (for High Speed Mode)
	\author		Hanyi Chiu
	\version	0.4
	\date		2020/04/23
	\copyright	Copyright(C) 2020 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _APP_HS_H_
#define _APP_HS_H_
//------------------------------------------------------------------------------
#include "_510PF.h"
#include "UI.h"
#include "KNL.h"
#include "PAIR.h"
#include "APP_CFG.h"
#include "BSP.h"
#include "USBD_API.h"
//------------------------------------------------------------------------------
#define APP_EVENTQUEUE_SZ			30
#define APP_PAIRING_TIMEOUT			20	//!< Unit: seconds

//! APP Display Resolution
typedef enum
{
	FHD_WIDTH 	 = 1920,
	FHD_HEIGHT	 = 1088,
	HD_WIDTH 	 = 1280,
	HD_HEIGHT	 = 720,
	WSVGA_WIDTH  = 1024,
	WSVGA_HEIGHT = 600,
	WVGA_WIDTH   = 576,
	WVGA_HEIGHT  = 320,
	VGA_WIDTH    = 576,
	VGA_HEIGHT   = 320,
	//VGA_HEIGHT   = 360,
}APP_DISPLAY_RESOLUTION;

typedef enum
{
	LCD_PM_SUSPEND,
	LCD_PWR_OFF
}LCD_PM_OPT;
#define LCD_PM							LCD_PM_SUSPEND

#define LCD_BYPASS						0
#define LCD_REAL						1
#define LCD_SEL							LCD_REAL

#define RTC_RECORD_PWRSTS_ADDR			0
#define RTC_PWRSTS_KEEP_TAG				0x03
#define RTC_WATCHDOG_CHK_TAG			0x0C
#define RTC_PS_WOR_TAG					0x30

#define RTC_RECORD_VIEW_MODE_ADDR		1
#define RTC_RECORD_VIEW_CAM_ADDR		2

typedef enum
{
	APP_STATE_NULL = 0,       		  	//!< System reserved state. Don't change this line
	APP_POWER_OFF_STATE,
	APP_IDLE_STATE,
	APP_LINK_STATE,
	APP_LOSTLINK_STATE,
	APP_PAIRING_STATE,
}APP_State_t;

typedef enum
{
	rFAIL = 0,
	rSUCCESS,
	rLOSTLINK = 0,
	rLINK,
}APP_Result_t;

typedef enum
{
	APP_LINKSTS_RPT = 1,
	APP_PAIRSTS_RPT,
	APP_PAIRUDCAM_PRT,
	APP_VWMODESTS_RPT,
	APP_VOXMODESTS_RPT,
	APP_DISPPAIRICON_RPT,
	APP_TRXBWSWRET_RPT,
	APP_RPT_NONE = 0xFF,
}APP_ReportType_t;

typedef struct
{
	APP_State_t  	 tAPP_State;
	APP_ReportType_t tAPP_ReportType;
	uint8_t 	 	 ubAPP_Report[18];
	uint32_t 	 	 ulAPP_Report[8];
}APP_StatusReport_t;

typedef enum
{
	APP_TUNINGMODE_OFF,
	APP_TUNINGMODE_ON,
}APP_TuningMode_t;

typedef enum
{
	APP_RF_TEST_OFF,
	APP_RF_TEST_ON,
}APP_RfTestMode_t;
typedef struct
{
	void(*pvAPP_TuningFunc)(void);
}APP_TuningFuncPtr_t;

typedef void(*pvAPP_StateCtrl)(APP_EventMsg_t *);

typedef struct
{
	void (*pvFuncPtr)(APP_EventMsg_t *ptAPP_Message);
}APP_StateFunc_t;

typedef struct
{
	KNL_ROLE	tKNL_StaNum;
	PAIR_TAG	tPAIR_StaNum;
	TWC_TAG		tTWC_StaNum;
}APP_StaNumMap_t;

typedef struct
{
	void (*APP_tActFunPtr)(void);
}APP_ActFuncPtr_t;

#ifdef BUC_CU
typedef struct
{
	KNL_DISP_LOCATION tKNL_DispLocation;
}APP_DispLocMap_t;

typedef struct
{
	KNL_DISP_LOCATION tKNL_DispLoc;
}APP_KNLRoleInfo_t;

typedef struct
{
	KNL_ROLE 			 tPairCamRole;
	KNL_DISP_LOCATION 	 tPairCamDispLoc;
	uint8_t 			 ubAppUpdUiStsFlag;
	UI_DisplayLocation_t tAppDispLoc;
}APP_PairRoleInfo_t;
#endif

typedef struct
{
	char 		   	  cbKNL_InfoTag[12];
	char 		      cbKNL_FwVersion[11];
	KNL_ROLE		  tKNL_Role;
	KNL_OPMODE		  tKNL_OpMode;
#ifdef BUC_CU
	APP_KNLRoleInfo_t tCamRoleInfo[4];		//! [0] = KNL_STA1, [1] = KNL_STA2, [2] = KNL_STA3, [3] = KNL_STA4
#endif
	KNL_ROLE		  tAdoSrcRole;
	USBD_ClassMode_t  tUsbdClassMode;
	APP_TuningMode_t  tTuningMode;

	APP_RfTestMode_t  tRfTestMode;
	uint32_t 		  ulRFVCOGroup[4];
}APP_KNLInfo_t;

#define APP_KNLRoleMap2CamNum(RoleNum, CamNum)							\
{																		\
	for(CamNum = CAM1; CamNum <= CAM4; CamNum++)						\
	{																	\
		if(APP_GetSTANumMappingTable(CamNum)->tKNL_StaNum == RoleNum)	\
			break;														\
	}																	\
}

#define APP_PairTagMap2CamNum(StaTag, CamNum)							\
{																		\
	for(CamNum = CAM1; CamNum <= CAM4; CamNum++)						\
	{																	\
		if(APP_GetSTANumMappingTable(CamNum)->tPAIR_StaNum == StaTag)	\
			break;														\
	}																	\
}

#define APP_TwcTagMap2CamNum(StaTag, CamNum)							\
{																		\
	for(CamNum = CAM1; CamNum <= CAM4; CamNum++)						\
	{																	\
		if(APP_GetSTANumMappingTable(CamNum)->tTWC_StaNum == StaTag)	\
			break;														\
	}																	\
}
//------------------------------------------------------------------------------
void APP_Disconnect(void);
void APP_RefreshBandwidth(APP_EventMsg_t *ptEventMsg);

void APP_StateFlowCtrl(APP_EventMsg_t *ptEventMsg);
void APP_PowerCtrlFunc(APP_EventMsg_t *ptEventMsg);
void APP_IdleStateFunc(APP_EventMsg_t *ptEventMsg);
void APP_LinkStateFunc(APP_EventMsg_t *ptEventMsg);
void APP_LostLinkStateFunc(APP_EventMsg_t *ptEventMsg);
void APP_PairingStateFunc(APP_EventMsg_t *ptEventMsg);
void APP_doPairingStart(void *pvPairInfo);
#ifdef BUC_CU
void APP_doUnbindCAM(APP_EventMsg_t *ptEventMsg);
#endif
uint8_t APP_UpdateLinkStatus(void);
APP_StaNumMap_t *APP_GetSTANumMappingTable(UI_CamNum_t tCamNum);
void APP_LoadKNLSetupInfo(void);
void APP_UpdateKNLSetupInfo(void);
void APP_KNLParamSetup(void);
void APP_FWUgradeSetup(void);
#ifdef BUC_CU
void APP_SwitchTrxBwExec(APP_EventMsg_t *ptEventMsg);
void APP_SwitchViewTypeExec(APP_EventMsg_t *ptEventMsg);
#endif
void APP_PowerSaveExec(APP_EventMsg_t *ptEventMsg);
void APP_SetTuningToolMode(APP_TuningMode_t tTuningMode);
APP_TuningMode_t APP_GetTuningToolMode(void);
void APP_SetRfTestMode(APP_RfTestMode_t tRfTestMode);
APP_RfTestMode_t APP_GetRfTestMode(void);
uint32_t APP_GetRFVCOGroup(int index);
void APP_SetRFVCOGroup(int index,uint32_t group);
void APP_Start(void);
#if (APP_DOORPHONE_ENABLE==1)  
typedef enum
{
    APP_DP_CMD_IDLE = 0,    // 0
	APP_DP_CMD_ACK,         // 1
	APP_DP_CMD_PAIRING,     // 2
	APP_DP_CMD_CALL,        // 3
	APP_DP_CMD_LISTEN,      // 4
	APP_DP_CMD_MONITORREQ,  // 5
	APP_DP_CMD_MONITOR,     // 6
	APP_DP_CMD_TALK,        // 7
	APP_DP_CMD_HANDUP,      // 8
	APP_DP_CMD_SWITCH,      // 9
	APP_DP_CMD_MISMATCH,    // 10
	APP_DP_CMD_TIMEOUT,     // 11
}APP_BBFAST_CMD_t;

typedef enum
{
    APP_DP_POWERON = 0,
    APP_DP_IDLE,
    APP_DP_LISTEN,    
	APP_DP_CALL,
	APP_DP_WAITMONITOR,
	APP_DP_MONITOR,
	APP_DP_TALK,
	APP_DP_PAIRING,
	APP_DP_MAX,
}APP_DP_STATUS_t;
    
typedef struct
{
	uint8_t  	    ubAPP_DPCmd;
    uint8_t  	    ubAPP_DPID;
	uint8_t 	 	ubAPP_DPData[2];
}APP_DP_CmdPacket_t;

// Definition time count
#define APP_DP_TM_POWERON       6   // 600ms
#define APP_DP_TM_IDLE          2   // 200ms
#define APP_DP_TM_LISTEN_STA    6   // 600ms
#define APP_DP_TM_ANSWER        1   // 100ms
#define APP_DP_TM_WAITMONITOR   80  // 8s
#define APP_DP_TM_CALL          120 // 12s
#define APP_DP_TM_HANDUP        2   // 200ms
#define APP_DP_TM_PAIRING       1   // 100ms
#define APP_DP_TM_LOOP          1   // 100ms
// Definition command
#define APP_DP_CMD_RETRY_CNT    5
#define APP_DP_CMD_RETRY_LOOP   2    

// Definition Listen
#define APP_DP_LOOP_LISTEN      4

#ifdef BUC_CU
void APP_DP_APHandler(uint8_t ubCamNum);
void APP_DP_APCmdRecvProc(uint8_t ubCamNum);
void APP_DP_Exec(APP_EventMsg_t *ptEventMsg);
void APP_DP_APSetStatus(uint8_t ubCamNum,uint8_t ubValue, uint8_t ubTargetCnt);
uint8_t APP_DP_APGetStatus(uint8_t ubCamNum);
#endif

#ifdef BUC_CAM
void APP_DP_STACmdRecvProc(uint8_t ubNb);
void APP_DP_STAHandler(void);
void APP_DP_STAKeyExec(APP_EventMsg_t *ptEventMsg);
void APP_DP_STAPowerSaveExec(void);
void APP_DP_STASetStatus(uint8_t ubValue,uint8_t ubTargetCnt);
uint8_t APP_DP_STAGetStatus(void);
#endif
uint8_t APP_DP_SetCmd(uint8_t ubCmd, uint8_t ubKNL_ROLE, uint8_t ubData0, uint8_t ubData1);
#endif	
#endif																			//!< End of _APP_HS_H_ definition

