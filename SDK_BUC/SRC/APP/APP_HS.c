/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		APP_HS.c
	\brief		Application function (for High Speed Mode)
	\author		Hanyi Chiu
	\version	0.13
	\date		2021/12/01
	\copyright	Copyright(C) 2021 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include "APP_HS.h"
#include "APP_CFG.h"
#include "FWU_API.h"
#include "PROFILE_API.h"
#include "RTC_API.h"
#include "SF_API.h"
#include "MMU_API.h"
#include "DMAC_API.h"
#include "BB_API.h"
#include "SD_API.h"
#include "VDO.h"
#include "ADO.h"
#include "RC.h"
#include "CLI.h"
#include "CIPHER_API.h"
#include "LCD.h"
#include "WDT.h"
#include "APP_XUCMD.h"
#include "APP_UVCUACCMD.h"
#include "sPRF_API.h"
#ifdef CFG_UART1_ENABLE
#include "UI_UART1.h"
#endif
#if (BSP_RTC_TIMER_SEL == RTC_TIMER_EXTERNAL)
#include "RTC_PCF85063A.h"
#endif
#ifdef RTC676x
#include "RTC676x_CTRL.h"
#include "rwrf.h"
#endif
#include "UART4AI.h"
#include "SADC.h"
#include "UI_BUCCU[WSVGA].h"

//------------------------------------------------------------------------------
bool ado_src_ai = 0;
#if (defined(BUC_CAM))
const char cAPP_ModelName[] __attribute__((section(".ARM.__at_0x00005FE0"))) = "70BUCCAM";
#elif (defined(BUC_CU))
const char cAPP_ModelName[] __attribute__((section(".ARM.__at_0x00005FE0"))) = "71BUCCU";
#endif
const uint8_t ubAPP_SfWpGpioPin __attribute__((section(".ARM.__at_0x00005FF0"))) = SF_WP_GPIN;
osMessageQId APP_EventQueue;
pvAPP_StateCtrl APP_StateCtrlFunc;
static APP_StatusReport_t tAPP_StsReport;
uint32_t ulAPP_WaitTickTime;
const static APP_StaNumMap_t tAPP_STANumTable[CAM_4T] =
{
	[CAM1] = {KNL_STA1, PAIR_STA1, TWC_STA1},
	[CAM2] = {KNL_STA2, PAIR_STA2, TWC_STA2},
	[CAM3] = {KNL_STA3, PAIR_STA3, TWC_STA3},
	[CAM4] = {KNL_STA4, PAIR_STA4, TWC_STA4},
};
static APP_KNLInfo_t tAPP_KNLInfo;
#ifdef BUC_CU
const static APP_DispLocMap_t tAPP_DispLocMap[] =
{
#if (defined(BSP_SN93711_FHD_REC_RX_V4)|| defined(BSP_D_SN93701_SSD2828_RX_V5) || defined(BSP_D_SN93701_TC358778_RX_V6))
	//! Qual View
	[DISP_UPPER_LEFT]  = {KNL_DISP_LOCATION3},
	[DISP_UPPER_RIGHT] = {KNL_DISP_LOCATION1},
	[DISP_LOWER_LEFT]  = {KNL_DISP_LOCATION4},
	[DISP_LOWER_RIGHT] = {KNL_DISP_LOCATION2},
	//! Dual View
	[DISP_LEFT]  	   = {KNL_DISP_LOCATION2},
	[DISP_RIGHT]  	   = {KNL_DISP_LOCATION1},
#elif (defined(BSP_D_SNCC71_GM8285C_RX_V2) || defined(BSP_D_SNCC71_TP2915_AHD_RX_V3) || defined(BSP_D_SNCC71_NVP6021_AHD_RX_V4))
	//! Qual View
	[DISP_UPPER_LEFT]  = {KNL_DISP_LOCATION1},
	[DISP_UPPER_RIGHT] = {KNL_DISP_LOCATION2},
	[DISP_LOWER_LEFT]  = {KNL_DISP_LOCATION3},
	[DISP_LOWER_RIGHT] = {KNL_DISP_LOCATION4},
	//! Dual View
	[DISP_LEFT]  	   = {KNL_DISP_LOCATION1},
	[DISP_RIGHT]  	   = {KNL_DISP_LOCATION2},
#endif
	//! Single View
	[DISP_1T]		   = {KNL_DISP_LOCATION1},
	//! H View
	[DISP_H_L]		   = {KNL_DISP_LOCATION4},
	[DISP_H_R]		   = {KNL_DISP_LOCATION1},
	[DISP_H_CU]		   = {KNL_DISP_LOCATION2},
	[DISP_H_CL]		   = {KNL_DISP_LOCATION3},
};
APP_PairRoleInfo_t tAPP_PairRoleInfo;
#endif
#if (defined(BUC_CAM) && APP_AUTO_PAIR)
static uint8_t APP_LostLinkCnt;
#endif
#if (APP_DOORPHONE_ENABLE==1)   
#if defined(BUC_CU)
uint8_t ubAPP_DP_APPreCmd[4]={0};
uint8_t ubAPP_DP_APCmdRetryTmOut[4]={0};
uint8_t ubAPP_DP_APCmdRetryLoop[4]={0};
uint8_t ubAPP_DP_APHandlerCnt[4]={0};
uint8_t ubAPP_DP_APHandlerTargetCnt[4]={APP_DP_TM_POWERON,APP_DP_TM_POWERON,APP_DP_TM_POWERON,APP_DP_TM_POWERON};
uint8_t ubAPP_DP_APStatus[4]={APP_DP_POWERON,APP_DP_POWERON,APP_DP_POWERON,APP_DP_POWERON};
uint8_t ubAPP_DP_APPreStatus[4]={APP_DP_POWERON,APP_DP_POWERON,APP_DP_POWERON,APP_DP_POWERON};
uint8_t ubApp_DP_APLostLinkCnt[4] = {0};
uint8_t ubAPP_DP_APPairCam=CAM1;
#endif
#if defined(BUC_CAM)
uint8_t ubAPP_DP_STAGPIdx=0;
uint8_t ubAPP_DP_STAPreCmd[4]={0};
uint8_t ubAPP_DP_STACmdRetryTmOut[4]={0};
uint8_t ubAPP_DP_STACmdRetryLoop[4]={0};
uint8_t ubAPP_DP_STAHandlerCnt=0;
uint8_t ubAPP_DP_STAHandlerTargetCnt=APP_DP_TM_POWERON;
uint8_t ubAPP_DP_STAStatus=APP_DP_POWERON;
uint8_t ubAPP_DP_STAPreStatus=APP_DP_POWERON;
uint8_t ubApp_DP_STALostLinkCnt= 0;
uint8_t ubAPP_ListenLoop=0;
uint8_t ubAPP_DP_STAGPChange=0;
uint8_t ubAPP_DP_STA_Monitorfg=0;
#endif
static void APP_DP_Thread(void const *argument);
#endif
static void APP_StartThread(void const *argument);
static void APP_WatchDogThread(void const *argument);
//------------------------------------------------------------------------------
void APP_Init(void)
{
    osStatus APP_OsStatus;
		
#if (defined(BUC_CAM) && APP_AUTO_PAIR)
	APP_LostLinkCnt = UI_AUTO_PAIR_START_TIME / UI_TASK_PERIOD;
#endif	
		
	APP_OsStatus = osKernelInitialize((uint8_t*)ulMMU_GetCacheHeapStartAddr(), 
												osHeapSize, 
									  (uint8_t*)ulMMU_GetUnCacheHeapStartAddr(), 
												ulMMU_GetUnCacheHeapSize());
    if(APP_OsStatus != osOK)
    {
        printd(DBG_ErrorLvl, "RTOS initial fail\n");
    }
	
	RTC_Init((BSP_RTC_TIMER_SEL == RTC_TIMER_INTERNAL)?RTC_TimerEnable:RTC_TimerDisable);
	BSP_Init();
#if (BSP_RTC_TIMER_SEL == RTC_TIMER_EXTERNAL)
	RTC_PCF85063A_Init(RTC_PCF85063A_24hr);
#endif
	if(ubAPP_SfWpGpioPin <= 13)
	{
		printd(DBG_InfoLvl, "SF_WP=GPIO%d\n", ubAPP_SfWpGpioPin);
	}
	SF_SetWpPin(ubAPP_SfWpGpioPin);
	SF_Init();
	PROF_Init();
	MMU_Init();
	TWC_Init();
	CLI_Init();
	CIPHER_Init();

#ifdef CFG_UART1_ENABLE
	UART1_RecvInit();
#endif

	FWU_Init();
	UI_Init(&APP_EventQueue);
	if (BSP_DDRSIZE > (ulDDR_GetCapacity() >> 20))
	{	
		printd(DBG_ErrorLvl, "Select IC part number fail!\n");
		while(1);
	}
	
#if (defined(BUC_CU) && APP_DOORPHONE_ENABLE==1) 
    UI_DP_Init(&ubAPP_DP_APStatus[0]);
#endif
	tAPP_StsReport.tAPP_State  	= APP_POWER_OFF_STATE;
	ulAPP_WaitTickTime      	= 0;	//!< osWaitForever;
	APP_StateCtrlFunc 			= APP_StateFlowCtrl;
	osMessageQDef(APP_EventQueue, APP_EVENTQUEUE_SZ, APP_EventMsg_t);
	APP_EventQueue = osMessageCreate(osMessageQ(APP_EventQueue), NULL);
    osThreadDef(APP_StartThread, APP_StartThread, THREAD_PRIO_APP_HANDLER, 1, THREAD_STACK_APP_HANDLER);
    if(osThreadCreate(osThread(APP_StartThread), NULL) == NULL)
	{
		printd(DBG_ErrorLvl, "Create APP_StartThread fail!\n");
		while(1);
	}
    osThreadDef(APP_WatchDogThread, APP_WatchDogThread, THREAD_PRIO_WDT_HANDLER, 1, 256);
    if(osThreadCreate(osThread(APP_WatchDogThread), NULL) == NULL)
	{
		printd(DBG_ErrorLvl, "Create APP_WatchDogThread fail!\n");
		while(1);
	}
#if (APP_DOORPHONE_ENABLE==1) 
    osThreadDef(APP_DP_Thread, APP_DP_Thread, THREAD_PRIO_DP_HANDLER, 1, 256);
    if(osThreadCreate(osThread(APP_DP_Thread), NULL) == NULL)
	{
		printd(DBG_ErrorLvl, "Create APP_DP_Thread fail!\n");
		while(1);
	}    
#endif
	/*! Start the kernel.  From here on, only tasks and interrupts will run. */
	osKernelStart();

	/*! If all is well, the scheduler will now be running, and the following
	line will never be reached. */
	for( ;; );
}
//------------------------------------------------------------------------------
void APP_StartThread(void const *argument)
{
	osStatus osAPP_EventStauts;
	APP_EventMsg_t tAPP_EventMsg;

	while(1)
	{
		osAPP_EventStauts = osMessageGet(APP_EventQueue, &tAPP_EventMsg, ulAPP_WaitTickTime);
		if(osAPP_EventStauts == osEventTimeout)
			tAPP_EventMsg.ubAPP_Event = APP_REFRESH_EVENT;
		if(APP_StateCtrlFunc)
			APP_StateCtrlFunc(&tAPP_EventMsg);
	}
}
//------------------------------------------------------------------------------
void APP_WatchDogThread(void const *argument)
{
	while(1)
	{
		WDT_TimerClr(WDT_RST);
		osDelay(500);
	}
}
//------------------------------------------------------------------------------
void APP_PowerOnFunc(void)
{
	uint32_t ulBUF_StartAddr = 0;
	
//! BUC CU A7130
//! BUC CU RTC676x	
#if (defined(OP_AP) && APP_DUAL_HOST_ENABLE)
	KNL_SetAdjBufEnable(1);
	KNL_SetSenThenEncEnable(0);		
	KNL_SetFixDisplayLocateEnable(1);		
	APPXU_Init();	
#endif		

#if defined(OP_STA)	
#if APP_UVC_CAM_ENABLE
	APPXU_Init();
#endif
#endif		
	
	//Sensor Buffer
#if (BSP_DDRSIZE == 16)
	#if (defined(RTC676x))
		KNL_SetYuvBufNub(1);
		KNL_SetSenThenEncEnable(1);
	#else
		KNL_SetYuvBufNub(2);
		KNL_SetSenThenEncEnable(0);
	#endif
#else
        KNL_SetYuvBufNub(2);
		KNL_SetSenThenEncEnable(0);
#endif
	
		
//! BUC CAM A7130
//! BUC CAM RTC676x		
#if (defined(OP_STA) && (!defined(S2019A)))
#if APP_UVC_CAM_ENABLE	
	KNL_SetAdjBufEnable(0);	
	KNL_SetSenThenEncEnable(0);
	KNL_SetFixDisplayLocateEnable(0);	
#else
	if (16 == (ulDDR_GetCapacity() >> 20))
	{
		KNL_SetAdjBufEnable(1);
	#ifdef RTC676x
		KNL_SetSenThenEncEnable(1);
	#endif	
		KNL_SetFixDisplayLocateEnable(0);		
	}
#endif
#endif	
	
	APP_LoadKNLSetupInfo();
#if USBD_ENABLE
	//! USB Device initialization
	if(APP_USBD_COMPOSITE_MODE)
		USBD_SetMultiDrvMode((USBD_MultiMd_t)USBD_MULTI_UVCUAC);
	USBD_Init(tAPP_KNLInfo.tUsbdClassMode);
	if(USBD_COMPOSITE_MODE == tAPP_KNLInfo.tUsbdClassMode)
		tUSBD_RegXuCbFunc(USBD_OPC_NVR, APPXU_RecvCmd);
#ifdef BUC_CU	
	tUSBD_UvcCtProcessCbFunc(APP_UvcCtCmd);
	tUSBD_UvcPuProcessCbFunc(APP_UvcPuCmd);
#endif		
#endif

	//! Firmware Upgrade Setup
	APP_FWUgradeSetup();

	//! System initialization
	DMAC_Init();
	PAIR_Init(&APP_EventQueue);

	ulBUF_StartAddr  = ulMMU_GetBufStartAddr();
	//! UI Buffer Setup
	ulBUF_StartAddr += ulUI_BufSetup(ulBUF_StartAddr);

	//! Kernel / Buffer initialization
	KNL_Init();
	BUF_Init(ulBUF_StartAddr);

	//! Kernel Parameter Setup
	APP_KNLParamSetup();

	//! Rate Control Preset Setup
#if (defined(S2019A))
	RC_PresetSetup(RC_QTY_AND_DYNAFPS);
#elif (defined(RTC676x))
	RC_PresetSetup(RC_DYNAMIC_FPS);
#elif (defined(A7130))	
	RC_PresetSetup(RC_QTY_AND_BW);
#endif

	//! Video / Audio initialization
	VDO_Init();
	ADO_Init();
	//BSP_ReInitAfterADOInit();

	//! Kernel Buffer Calculate
	KNL_BufInit();

	//! UI Plug-in
	UI_PlugIn();

	//! Application Start
	APP_Start();
	BUZ_Init();

	UART4AI_Init();
	Trigger_init();
	TouchPanel_init();
	IR_init();
}
//------------------------------------------------------------------------------
void APP_StateFlowCtrl(APP_EventMsg_t *ptEventMsg)
{
	const static APP_StateFunc_t tAppStateFunc[] =
	{
		[APP_POWER_OFF_STATE] 	= APP_PowerCtrlFunc,
		[APP_IDLE_STATE] 		= APP_IdleStateFunc,
		[APP_LINK_STATE] 		= APP_LinkStateFunc,
		[APP_LOSTLINK_STATE] 	= APP_LostLinkStateFunc,
		[APP_PAIRING_STATE] 	= APP_PairingStateFunc,
	};
	if(tAppStateFunc[tAPP_StsReport.tAPP_State].pvFuncPtr)
		tAppStateFunc[tAPP_StsReport.tAPP_State].pvFuncPtr(ptEventMsg);
}
//------------------------------------------------------------------------------
void APP_PowerCtrlFunc(APP_EventMsg_t *ptEventMsg)
{
	ulAPP_WaitTickTime = osWaitForever;
	switch(tAPP_StsReport.tAPP_State)
	{
		case APP_POWER_OFF_STATE:
			APP_PowerOnFunc();
			break;
		default:
			tAPP_StsReport.tAPP_State = APP_POWER_OFF_STATE;
			break;
	}
}
//------------------------------------------------------------------------------
void APP_IdleStateFunc(APP_EventMsg_t *ptEventMsg)
{
	switch(ptEventMsg->ubAPP_Event)
	{
		case APP_LINKSTATUS_REPORT_EVENT:
			tAPP_StsReport.tAPP_ReportType = APP_LINKSTS_RPT;
			tAPP_StsReport.tAPP_State = (APP_UpdateLinkStatus() == APP_LINK_EVENT)?APP_LINK_STATE:APP_LOSTLINK_STATE;
			UI_UpdateAppStatus(&tAPP_StsReport);
		#if (defined(BUC_CAM) && APP_AUTO_PAIR && APP_AUTO_PAIR_MODE)
			APP_LostLinkCnt = UI_AUTO_PAIR_START_TIME / UI_TASK_PERIOD;
		#endif
			break;
		case APP_PAIRING_START_EVENT:
			APP_doPairingStart(ptEventMsg->ubAPP_Message);
			tAPP_StsReport.tAPP_State = APP_PAIRING_STATE;
			UI_UpdateAppStatus(&tAPP_StsReport);
			break;
	#ifdef BUC_CU
		case APP_UNBIND_CAM_EVENT:
			APP_doUnbindCAM(ptEventMsg);
			break;
		case APP_VIEWTYPECHG_EVENT:
			APP_SwitchViewTypeExec(ptEventMsg);
			break;
		case APP_TRXBWSW_EVENT:
			APP_SwitchTrxBwExec(ptEventMsg);
			break;
        #if(APP_DOORPHONE_ENABLE==1 && APP_DP_TXPS_MODE==0)
        case APP_DP_EVENT:
			APP_DP_Exec(ptEventMsg);
			break;
        #endif
	#endif
		case APP_POWERSAVE_EVENT:
			APP_PowerSaveExec(ptEventMsg);
			break;
    #if (defined(BUC_CAM) && APP_DOORPHONE_ENABLE==1)             
        case APP_DP_KEY_EVENT:
            APP_DP_STAKeyExec(ptEventMsg);
            break;
    #endif
		case APP_ADOSRCSEL_EVENT:
		{
			UI_CamSource ado_src = (UI_CamSource)ptEventMsg->ubAPP_Message[1];
			uint8_t ubUpdFlag  = ptEventMsg->ubAPP_Message[2];
			
			if(ado_src >= CamSource_AI) {
				ado_src_ai = 1;
				// tAPP_KNLInfo.tAdoSrcRole = KNL_NONE;
				// ADO_Stop();
				if(TRUE == ubUpdFlag)
					APP_UpdateKNLSetupInfo();
				break;
			}
			ado_src_ai = 0;
			
			KNL_ROLE tKNL_Role = tAPP_STANumTable[ado_src].tKNL_StaNum;
			tAPP_KNLInfo.tAdoSrcRole = tKNL_Role;
			if(tAPP_KNLInfo.tAdoSrcRole<=KNL_STA4)
			{
				ADO_RstRcvTimeLatency(tAPP_KNLInfo.tAdoSrcRole);
			}
			ADO_Start(tAPP_KNLInfo.tAdoSrcRole);
#if (APP_DOORPHONE_ENABLE==1)			   
			if(ubAPP_DP_APStatus[ptEventMsg->ubAPP_Message[1]] == APP_DP_TALK)
				ADO_PTTStart();
#endif
			if(TRUE == ubUpdFlag)
				APP_UpdateKNLSetupInfo();
			break;
		}

		case APP_DISCONNECT_EVENT:
			APP_Disconnect();
			break;	
		case APP_SETBANDWIDTH_EVENT:
			APP_RefreshBandwidth(ptEventMsg);
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void APP_LinkStateFunc(APP_EventMsg_t *ptEventMsg)
{
	switch(ptEventMsg->ubAPP_Event)
	{
		case APP_LINKSTATUS_REPORT_EVENT:
			tAPP_StsReport.tAPP_ReportType = APP_LINKSTS_RPT;
			tAPP_StsReport.tAPP_State = (APP_UpdateLinkStatus() == APP_LINK_EVENT)?APP_LINK_STATE:APP_LOSTLINK_STATE;
			UI_UpdateAppStatus(&tAPP_StsReport);
		#if (defined(BUC_CAM) && APP_AUTO_PAIR && APP_AUTO_PAIR_MODE)
			APP_LostLinkCnt = UI_AUTO_PAIR_START_TIME / UI_TASK_PERIOD;
		#endif
			break;
		case APP_PAIRING_START_EVENT:
			APP_doPairingStart(ptEventMsg->ubAPP_Message);
			tAPP_StsReport.tAPP_State = APP_PAIRING_STATE;
			UI_UpdateAppStatus(&tAPP_StsReport);
			break;
	#ifdef BUC_CU
		case APP_UNBIND_CAM_EVENT:
			APP_doUnbindCAM(ptEventMsg);
			break;
		case APP_VIEWTYPECHG_EVENT:
			APP_SwitchViewTypeExec(ptEventMsg);
			break;
		case APP_TRXBWSW_EVENT:
			APP_SwitchTrxBwExec(ptEventMsg);
			break;
		case APP_ADOSRCSEL_EVENT:
		{			
			UI_CamSource ado_src = (UI_CamSource)ptEventMsg->ubAPP_Message[1];
			uint8_t ubUpdFlag  = ptEventMsg->ubAPP_Message[2];
			
			if(ado_src >= CamSource_AI) {
				ado_src_ai = 1;
				// tAPP_KNLInfo.tAdoSrcRole = KNL_NONE;
				// ADO_Stop();
				if(TRUE == ubUpdFlag)
					APP_UpdateKNLSetupInfo();
				break;
			}
			ado_src_ai = 0;

			tAPP_KNLInfo.tAdoSrcRole = tAPP_STANumTable[ado_src].tKNL_StaNum;
			if(tAPP_KNLInfo.tAdoSrcRole<=KNL_STA4)
			{
				ADO_RstRcvTimeLatency(tAPP_KNLInfo.tAdoSrcRole);
			}
			ADO_Start(tAPP_KNLInfo.tAdoSrcRole);
    #if (APP_DOORPHONE_ENABLE==1)              
            if(ubAPP_DP_APStatus[ptEventMsg->ubAPP_Message[1]] == APP_DP_TALK)
                ADO_PTTStart();
    #endif
			if(TRUE == ubUpdFlag)
				APP_UpdateKNLSetupInfo();
			break;
		}
		case APP_PTT_EVENT:
		{
		#if APP_ADO_FUNC_ENABLE
			uint8_t ubAPP_PttFlag = ptEventMsg->ubAPP_Message[1];
			ADO_PttFuncPtr_t tAPP_PttFunc[] = {ADO_PTTStop, ADO_PTTStart};

			if(tAPP_PttFunc[ubAPP_PttFlag].ADO_tPttFunPtr)
				tAPP_PttFunc[ubAPP_PttFlag].ADO_tPttFunPtr();
		#endif
			break;
		}
    #if (APP_DOORPHONE_ENABLE==1)
		case APP_DP_EVENT:
            APP_DP_Exec(ptEventMsg);
			break;
    #endif
	#endif
    
    #if (defined(BUC_CAM)&&(APP_DOORPHONE_ENABLE==1))
        case APP_DP_KEY_EVENT:
            APP_DP_STAKeyExec(ptEventMsg);
            break;
    #endif    
		case APP_POWERSAVE_EVENT:
			APP_PowerSaveExec(ptEventMsg);
			break;
		case APP_DISCONNECT_EVENT:
			APP_Disconnect();
			break;	
		case APP_SETBANDWIDTH_EVENT:
			APP_RefreshBandwidth(ptEventMsg);
			break;
		case APP_SYNCTIMESTAMP_EVENT:
			KNL_SyncTimeStamp2TX(ptEventMsg->ubAPP_Message[1]);
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void APP_LostLinkStateFunc(APP_EventMsg_t *ptEventMsg)
{
	switch(ptEventMsg->ubAPP_Event)
	{
		case APP_LINKSTATUS_REPORT_EVENT:
			tAPP_StsReport.tAPP_ReportType = APP_LINKSTS_RPT;
			tAPP_StsReport.tAPP_State = (APP_UpdateLinkStatus() == APP_LINK_EVENT)?APP_LINK_STATE:APP_LOSTLINK_STATE;
			UI_UpdateAppStatus(&tAPP_StsReport);
		#if (defined(BUC_CAM) && APP_AUTO_PAIR)
			if (APP_LOSTLINK_STATE == tAPP_StsReport.tAPP_State && 
				APP_LostLinkCnt)
			{
				--APP_LostLinkCnt;
				if (!APP_LostLinkCnt)
				{
					if (APP_AUTO_PAIR_MODE)
						APP_LostLinkCnt = UI_AUTO_PAIR_START_TIME / UI_TASK_PERIOD;
					if (AUTO_PAIR_PIN)
						UI_PairingKey();
				}
			}
		#endif
			break;
		case APP_PAIRING_START_EVENT:
			APP_doPairingStart(ptEventMsg->ubAPP_Message);
			tAPP_StsReport.tAPP_State = APP_PAIRING_STATE;
			tAPP_StsReport.tAPP_ReportType = APP_RPT_NONE;
			UI_UpdateAppStatus(&tAPP_StsReport);
			break;
    #if (defined(BUC_CAM)&&(APP_DOORPHONE_ENABLE==1))
        case APP_DP_KEY_EVENT:
            APP_DP_STAKeyExec(ptEventMsg);
            break;
    #endif
	#ifdef BUC_CU
		case APP_UNBIND_CAM_EVENT:
			APP_doUnbindCAM(ptEventMsg);
			break;
		case APP_VIEWTYPECHG_EVENT:
			APP_SwitchViewTypeExec(ptEventMsg);
			break;
		case APP_TRXBWSW_EVENT:
			APP_SwitchTrxBwExec(ptEventMsg);
			break;
        #if(APP_DOORPHONE_ENABLE==1 && APP_DP_TXPS_MODE==0)
        case APP_DP_EVENT:
			APP_DP_Exec(ptEventMsg);
			break;
        #endif
	#endif
		case APP_ADOSRCSEL_EVENT:
		{
			UI_CamSource ado_src = (UI_CamSource)ptEventMsg->ubAPP_Message[1];
			uint8_t ubUpdFlag  = ptEventMsg->ubAPP_Message[2];
			
			if(ado_src >= CamSource_AI) {
				ado_src_ai = 1;
				// tAPP_KNLInfo.tAdoSrcRole = KNL_NONE;
				// ADO_Stop();
				if(TRUE == ubUpdFlag)
					APP_UpdateKNLSetupInfo();
				break;
			}
			
			ado_src_ai = 0;

			tAPP_KNLInfo.tAdoSrcRole = tAPP_STANumTable[ado_src].tKNL_StaNum;
			if(tAPP_KNLInfo.tAdoSrcRole<=KNL_STA4)
			{
				ADO_RstRcvTimeLatency(tAPP_KNLInfo.tAdoSrcRole);
			}
			ADO_Start(tAPP_KNLInfo.tAdoSrcRole);
#if (APP_DOORPHONE_ENABLE==1)			   
			if(ubAPP_DP_APStatus[ptEventMsg->ubAPP_Message[1]] == APP_DP_TALK)
				ADO_PTTStart();
#endif
			if(TRUE == ubUpdFlag)
				APP_UpdateKNLSetupInfo();
			break;
		}

		case APP_POWERSAVE_EVENT:
			APP_PowerSaveExec(ptEventMsg);
			break;
		case APP_DISCONNECT_EVENT:
			APP_Disconnect();
			break;	
		case APP_SETBANDWIDTH_EVENT:
			APP_RefreshBandwidth(ptEventMsg);
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void APP_PairingStateFunc(APP_EventMsg_t *ptEventMsg)
{
	switch(ptEventMsg->ubAPP_Event)
	{
		case APP_PAIRING_STOP_EVENT:
			PAIR_Stop();
		case APP_PAIRING_FAIL_EVENT:
			tAPP_StsReport.tAPP_ReportType = APP_PAIRSTS_RPT;
			tAPP_StsReport.tAPP_State 	   = APP_IDLE_STATE;
			tAPP_StsReport.ubAPP_Report[0] = rFAIL;
		#ifdef BUC_CU
			tAPP_StsReport.ubAPP_Report[2] = tAPP_PairRoleInfo.ubAppUpdUiStsFlag;
            #if (APP_DOORPHONE_ENABLE==1)
            KNL_DP_SetVideoOnOff(tAPP_STANumTable[ubAPP_DP_APPairCam].tKNL_StaNum,0);
            APP_DP_APSetStatus(ubAPP_DP_APPairCam,APP_DP_IDLE,APP_DP_TM_IDLE);
            #endif
        #endif            
        #if (defined(BUC_CAM)&&(APP_DOORPHONE_ENABLE==1))
            APP_DP_STASetStatus(APP_DP_IDLE,APP_DP_TM_IDLE);
		#endif
			UI_UpdateAppStatus(&tAPP_StsReport);
			tAPP_StsReport.tAPP_ReportType = APP_RPT_NONE;
			printd(DBG_Debug1Lvl, (APP_PAIRING_STOP_EVENT == ptEventMsg->ubAPP_Event)?"Pairing_stop\r\n":"Pairing_fail\r\n");
			break;
		case APP_PAIRING_SUCCESS_EVENT:
		{
		#ifdef BUC_CU
			UI_CamNum_t tAdoSrcCamNum;
            #if (APP_DOORPHONE_ENABLE==1)
            KNL_DP_SetVideoOnOff(tAPP_STANumTable[ubAPP_DP_APPairCam].tKNL_StaNum,1);
            APP_DP_APSetStatus(ubAPP_DP_APPairCam,APP_DP_MONITOR,APP_DP_TM_LOOP);
            #endif
        #endif 
        #if (defined(BUC_CAM)&&(APP_DOORPHONE_ENABLE==1))
            APP_DP_STASetStatus(APP_DP_MONITOR,APP_DP_TM_LOOP);
		#endif
			tAPP_StsReport.tAPP_ReportType = APP_PAIRSTS_RPT;
			tAPP_StsReport.tAPP_State 	   = APP_IDLE_STATE;
			tAPP_StsReport.ubAPP_Report[0] = rSUCCESS;
		#if (defined(BUC_CU))
			KNL_ResetLcdChannel();
			APP_KNLRoleMap2CamNum(tAPP_KNLInfo.tAdoSrcRole, tAdoSrcCamNum);
			tAPP_StsReport.ubAPP_Report[1] = tAdoSrcCamNum;
			tAPP_StsReport.ubAPP_Report[2] = tAPP_PairRoleInfo.ubAppUpdUiStsFlag;
			if(TRUE == tAPP_PairRoleInfo.ubAppUpdUiStsFlag)
			{
				tAPP_StsReport.ubAPP_Report[3] = tAPP_PairRoleInfo.tPairCamRole;
				tAPP_StsReport.ubAPP_Report[4] = tAPP_PairRoleInfo.tAppDispLoc;
			}

			tAPP_StsReport.ubAPP_Report[3] = tAPP_PairRoleInfo.tPairCamRole;
			UI_UpdateAppStatus(&tAPP_StsReport);
			tAPP_StsReport.tAPP_ReportType = APP_RPT_NONE;
			if((UI_CamNum_t)tAPP_StsReport.ubAPP_Report[1] != tAdoSrcCamNum)
			{
				tAdoSrcCamNum = (UI_CamNum_t)tAPP_StsReport.ubAPP_Report[1];
				tAPP_KNLInfo.tAdoSrcRole = APP_GetSTANumMappingTable(tAdoSrcCamNum)->tKNL_StaNum;
			}
			tAPP_KNLInfo.tCamRoleInfo[tAPP_PairRoleInfo.tPairCamRole].tKNL_DispLoc = tAPP_PairRoleInfo.tPairCamDispLoc;
           
			if(!ubKNL_GetFixDisplayLocateEnable())
			{
				VDO_DisplayLocationSetup(tAPP_PairRoleInfo.tPairCamRole, tAPP_PairRoleInfo.tPairCamDispLoc);
				VDO_UpdateDisplayParameter();
			}		
			
			if(APP_UNBIND_CAM_EVENT == ptEventMsg->ubAPP_Message[2])
			{
				UI_CamNum_t tDelCam;

				tAPP_StsReport.tAPP_ReportType = APP_PAIRUDCAM_PRT;
				tAPP_StsReport.tAPP_State 	   = APP_IDLE_STATE;
				APP_PairTagMap2CamNum((PAIR_TAG)ptEventMsg->ubAPP_Message[1], tDelCam);
				tAPP_StsReport.ubAPP_Report[0] = tDelCam;
				tAPP_StsReport.ubAPP_Report[1] = FALSE;
				UI_UpdateAppStatus(&tAPP_StsReport);
				tAPP_StsReport.tAPP_ReportType = APP_RPT_NONE;
			}
		#elif (defined(BUC_CAM))
			VDO_KNLVdoRes(tAPP_KNLInfo.tKNL_Role);
			UI_UpdateAppStatus(&tAPP_StsReport);
			tAPP_StsReport.tAPP_ReportType = APP_RPT_NONE;
			tAPP_KNLInfo.tKNL_Role   = tAPP_STANumTable[PAIR_GetStaNumber()].tKNL_StaNum;
			tAPP_KNLInfo.tAdoSrcRole = tAPP_KNLInfo.tKNL_Role;			
			VDO_KNLSysInfoSetup(tAPP_KNLInfo.tKNL_Role);
			ADO_KNLSysInfoSetup(tAPP_KNLInfo.tKNL_Role);
		#endif
			APP_UpdateKNLSetupInfo();

			KNL_SetBatteryInfoBwRpt(tAPP_PairRoleInfo.tPairCamRole,0);//清除电池信息
			break;
		}
		default:
			return;
	}
	VDO_Start();
	ADO_Start(tAPP_KNLInfo.tAdoSrcRole);
}
//------------------------------------------------------------------------------
void APP_doPairingStart(void *pvPairInfo)
{
#if (defined(BUC_CU))
	uint8_t *pAPP_PairInfo 				= (uint8_t *)pvPairInfo;
	PAIR_TAG tPair_Tag 					= tAPP_STANumTable[pAPP_PairInfo[1]].tPAIR_StaNum;
	tAPP_PairRoleInfo.tPairCamRole		= tAPP_STANumTable[pAPP_PairInfo[1]].tKNL_StaNum;
	tAPP_PairRoleInfo.tPairCamDispLoc	= tAPP_DispLocMap[pAPP_PairInfo[2]].tKNL_DispLocation;
	tAPP_PairRoleInfo.ubAppUpdUiStsFlag = pAPP_PairInfo[3];
	tAPP_PairRoleInfo.tAppDispLoc 		= (UI_DisplayLocation_t)pAPP_PairInfo[2];

	if(TRUE == tAPP_PairRoleInfo.ubAppUpdUiStsFlag)
	{
		tAPP_StsReport.tAPP_ReportType = APP_DISPPAIRICON_RPT;
		UI_UpdateAppStatus(&tAPP_StsReport);
		tAPP_StsReport.tAPP_ReportType = APP_RPT_NONE;
	}
    #if (APP_DOORPHONE_ENABLE==1)
    ubAPP_DP_APPairCam = pAPP_PairInfo[1];
    KNL_DP_SetVideoOnOff(tAPP_STANumTable[pAPP_PairInfo[1]].tKNL_StaNum,0);
    APP_DP_APSetStatus(pAPP_PairInfo[1],APP_DP_PAIRING,APP_DP_TM_PAIRING);
    #endif
#elif (defined(BUC_CAM))
	PAIR_TAG tPair_Tag = PAIR_AP_ASSIGN;
    #if (APP_DOORPHONE_ENABLE==1)
    uint8_t *pAPP_PairInfo              = (uint8_t *)pvPairInfo;    
    if(pAPP_PairInfo[0]==0)
        ubPAIR_ChangeStaIDGroup(0);
    if(pAPP_PairInfo[0]==1)
        ubPAIR_ChangeStaIDGroup(1);  
    #if(APP_DP_TXPS_MODE==0)
    BB_SetRfStopTx(0);
    #endif
    APP_DP_STASetStatus(APP_DP_PAIRING,APP_DP_TM_PAIRING);
    #endif
#endif
	ADO_Stop();
	VDO_Stop();
	PAIR_Start(tPair_Tag, APP_PAIRING_TIMEOUT, 0);
}
//------------------------------------------------------------------------------
#ifdef BUC_CU
void APP_doUnbindCAM(APP_EventMsg_t *ptEventMsg)
{
	PAIR_TAG tPair_Tag = tAPP_STANumTable[ptEventMsg->ubAPP_Message[1]].tPAIR_StaNum;
	KNL_ROLE tKNL_Role = tAPP_STANumTable[ptEventMsg->ubAPP_Message[1]].tKNL_StaNum;

	if((tPair_Tag > PAIR_STA4) || (tKNL_Role > KNL_STA4))
		return;
	PAIR_DeleteTxId(tPair_Tag);
	VDO_RemoveDataPath(tKNL_Role);
	ADO_RemoveDataPath(tKNL_Role);
	tAPP_KNLInfo.tCamRoleInfo[tKNL_Role].tKNL_DispLoc = KNL_DISP_LOCATION_ERR;
	tAPP_KNLInfo.tAdoSrcRole = (tAPP_KNLInfo.tAdoSrcRole == tKNL_Role)?KNL_NONE:tAPP_KNLInfo.tAdoSrcRole;
	APP_UpdateKNLSetupInfo();
}
#endif
//------------------------------------------------------------------------------
uint8_t APP_UpdateLinkStatus(void)
{
#if (defined(BUC_CU))
		uint8_t ubAPP_Event = APP_LOSTLINK_EVENT;
		KNL_ROLE ubKNL_RoleNum;
	
		for(ubKNL_RoleNum = KNL_STA1; ubKNL_RoleNum <= KNL_STA4; ubKNL_RoleNum++)
		{
			tAPP_StsReport.ubAPP_Report[ubKNL_RoleNum]	 = rLOSTLINK;
			tAPP_StsReport.ubAPP_Report[ubKNL_RoleNum+4] = 0;
			tAPP_StsReport.ubAPP_Report[ubKNL_RoleNum+8] = 0;
			tAPP_StsReport.ubAPP_Report[ubKNL_RoleNum+12] = 0;
			if(ubKNL_GetCommLinkStatus(ubKNL_RoleNum) == BB_LINK)
			{
				tAPP_StsReport.ubAPP_Report[ubKNL_RoleNum]	 = rLINK;
				tAPP_StsReport.ubAPP_Report[ubKNL_RoleNum+4] = KNL_GetPerValue(ubKNL_RoleNum);
				tAPP_StsReport.ubAPP_Report[ubKNL_RoleNum+8] = KNL_GetRssiValue(ubKNL_RoleNum);
				tAPP_StsReport.ubAPP_Report[ubKNL_RoleNum+12] = ulKNL_GetBatteryInfoBwRpt(ubKNL_RoleNum);
				ubAPP_Event = APP_LINK_EVENT;
			}
			tAPP_StsReport.ulAPP_Report[ubKNL_RoleNum] = KNL_GetFrameCount(ubKNL_RoleNum);
			tAPP_StsReport.ulAPP_Report[ubKNL_RoleNum + 4] = KNL_GetFps(ubKNL_RoleNum);
		}
		return ubAPP_Event;
#elif (defined(BUC_CAM))
		return (ubKNL_GetCommLinkStatus(KNL_MASTER_AP) == BB_LINK)?APP_LINK_EVENT:APP_LOSTLINK_EVENT;
#endif

}
//------------------------------------------------------------------------------
APP_StaNumMap_t *APP_GetSTANumMappingTable(UI_CamNum_t tCamNum)
{
	return (APP_StaNumMap_t*)&tAPP_STANumTable[tCamNum];
}
//------------------------------------------------------------------------------
void APP_LoadKNLSetupInfo(void)
{
	uint32_t ulAPP_KNLInfoSFAddr = pSF_Info->ulSize - (KNL_SF_START_SECTOR * pSF_Info->ulSecSize);

	tAPP_KNLInfo.tAdoSrcRole = KNL_NONE;
	SF_Read(ulAPP_KNLInfoSFAddr, sizeof(APP_KNLInfo_t), (uint8_t *)&tAPP_KNLInfo);
	printd(DBG_InfoLvl, "KNL TAG:%s\n",tAPP_KNLInfo.cbKNL_InfoTag);
	printd(DBG_InfoLvl, "KNL VER:%s\n",tAPP_KNLInfo.cbKNL_FwVersion);	
	
	if (tAPP_KNLInfo.tRfTestMode > APP_RF_TEST_ON)
		tAPP_KNLInfo.tRfTestMode = APP_RF_TEST_OFF;
	
#if (defined(BUC_CAM))
	if ((strncmp(tAPP_KNLInfo.cbKNL_InfoTag, SF_STA_KNL_SECTOR_TAG, sizeof(tAPP_KNLInfo.cbKNL_InfoTag) - 1) != 0)
	|| (strncmp(tAPP_KNLInfo.cbKNL_FwVersion, SN937XX_FW_VERSION, sizeof(tAPP_KNLInfo.cbKNL_FwVersion) - 1) != 0)) {
		printd(DBG_ErrorLvl, "TAG no match, Reset KNL\n");

		tAPP_KNLInfo.tUsbdClassMode = USBD_DEFU_CLASS;
		tAPP_KNLInfo.tTuningMode = APP_TUNINGMODE_OFF;
		return;
	}
	
	if(tAPP_KNLInfo.tUsbdClassMode >= USBD_UNKNOWN_MODE)
		tAPP_KNLInfo.tUsbdClassMode = USBD_MSC_MODE;
	if(tAPP_KNLInfo.tTuningMode > APP_TUNINGMODE_ON)
		tAPP_KNLInfo.tTuningMode = APP_TUNINGMODE_OFF;
	tAPP_KNLInfo.tUsbdClassMode = (!USBD_ENABLE)?USBD_MSC_MODE:USBD_DEFU_CLASS;
	if(APP_TUNINGMODE_ON == tAPP_KNLInfo.tTuningMode)
		tAPP_KNLInfo.tUsbdClassMode = USBD_UVC_MODE;
#elif (defined(BUC_CU))
	tAPP_KNLInfo.tTuningMode    = APP_TUNINGMODE_OFF;
	tAPP_KNLInfo.tUsbdClassMode = (!USBD_ENABLE)?USBD_MSC_MODE:USBD_DEFU_CLASS;
#endif
	

}
//------------------------------------------------------------------------------
void APP_UpdateKNLSetupInfo(void)
{
	uint32_t ulAPP_KNLInfoSFAddr;

	ulAPP_KNLInfoSFAddr = pSF_Info->ulSize - (KNL_SF_START_SECTOR * pSF_Info->ulSecSize);
#ifdef OP_AP
	strncpy(tAPP_KNLInfo.cbKNL_InfoTag, SF_AP_KNL_SECTOR_TAG, sizeof(tAPP_KNLInfo.cbKNL_InfoTag));
#else
	strncpy(tAPP_KNLInfo.cbKNL_InfoTag, SF_STA_KNL_SECTOR_TAG, sizeof(tAPP_KNLInfo.cbKNL_InfoTag));
#endif
	strncpy(tAPP_KNLInfo.cbKNL_FwVersion, SN937XX_FW_VERSION, sizeof(tAPP_KNLInfo.cbKNL_FwVersion));
	SF_DisableWrProtect();
	SF_Erase(SF_SE, ulAPP_KNLInfoSFAddr, pSF_Info->ulSecSize, 1);
	SF_Write(ulAPP_KNLInfoSFAddr, sizeof(APP_KNLInfo_t), (uint8_t *)&tAPP_KNLInfo);
	SF_EnableWrProtect();
}
//------------------------------------------------------------------------------
void APP_KNLParamSetup(void)
{
#if (defined(BUC_CU))
	KNL_ROLE tKNL_CamRole;
	UI_CamNum_t tCamNum;

	KNL_SetRole((tAPP_KNLInfo.tKNL_Role = KNL_MASTER_AP));
	if ((strncmp(tAPP_KNLInfo.cbKNL_InfoTag,   SF_AP_KNL_SECTOR_TAG, sizeof(tAPP_KNLInfo.cbKNL_InfoTag) - 1) == 0) &&
		(strncmp(tAPP_KNLInfo.cbKNL_FwVersion, SN937XX_FW_VERSION, sizeof(tAPP_KNLInfo.cbKNL_FwVersion) - 1) == 0))
	{	

	}
	printf("location setup start!!!!!!!\n");
	for(tCamNum = CAM1; tCamNum < DISPLAY_MODE; tCamNum++)
	{
		tKNL_CamRole = tAPP_STANumTable[tCamNum].tKNL_StaNum;
		VDO_DisplayLocationSetup(tKNL_CamRole, tAPP_KNLInfo.tCamRoleInfo[tKNL_CamRole].tKNL_DispLoc);
		
	}
	printf("location setup ok!!!!!!!\n");	
	if(tAPP_KNLInfo.tAdoSrcRole > KNL_STA4)
		tAPP_KNLInfo.tAdoSrcRole = KNL_STA1;
#endif
	tAPP_KNLInfo.tKNL_OpMode = (DISPLAY_MODE ==	DISPLAY_4T1R)?KNL_OPMODE_BUC_4T:
	                           (DISPLAY_MODE ==	DISPLAY_2T1R)?KNL_OPMODE_BUC_2T_1L1R:
							   (DISPLAY_MODE ==	DISPLAY_1T1R)?KNL_OPMODE_BUC_1T:KNL_OPMODE_BUC_4T;
	KNL_SetOpMode(tAPP_KNLInfo.tKNL_OpMode);
}
//------------------------------------------------------------------------------
void APP_FWUgradeStatusReport(uint8_t ubStsReport)
{
#define PROGRESS_BAR_SCALE	25
	APP_StatusReport_t tAPP_UpgStsReport;

	tAPP_UpgStsReport.ubAPP_Report[0] = ubStsReport;
	tAPP_UpgStsReport.ubAPP_Report[1] = PROGRESS_BAR_SCALE;
	UI_UpdateFwUpgStatus(&tAPP_UpgStsReport);
	switch(ubStsReport)
	{
		case FWU_UPG_INPROGRESS:
		{
		#if (defined(BUC_CAM))
			KNL_WakeupDevice(KNL_MASTER_AP, FALSE);
		#elif (defined(BUC_CU))
			UI_CamNum_t tCamNum;
			KNL_ROLE tKnlRole;

			for(tCamNum = CAM1; tCamNum < DISPLAY_MODE; tCamNum++)
			{
				tKnlRole = tAPP_STANumTable[tCamNum].tKNL_StaNum;
				KNL_WakeupDevice(tKnlRole, FALSE);
			}
		#endif
		//#if RTC676x	
		//	KNL_EnterFwUpgradeMode();
		//#endif			
			UI_StopUpdateThread();
			ADO_Stop();
			VDO_Stop();
			break;
		}
		case FWU_UPG_SUCCESS:
#if APP_PC_CONNECT_EN
			if(ubKNL_GetPcConnSdFwuStatus()==0)
#endif
			{
				printd(DBG_ErrorLvl, "APP FWU->system reboot\n");
				SYS_Reboot();
			}
			break;
		case FWU_UPG_DEVTAG_FAIL:
		case FWU_UPG_FAIL:
		//#if RTC676x		
		//	KNL_QuitFwUpgradeMode();
		//#endif		
			VDO_Start();
			ADO_Start(tAPP_KNLInfo.tAdoSrcRole);
			UI_StartUpdateThread();
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void APP_UsbVendorCmdProc(uint8_t ubAct, uint8_t *pCDB, uint8_t *pBuf)
{
	uint8_t ubVendorCmd = 0xFF;

	ubVendorCmd = (FWUV_W == ubAct)?*pCDB:(FWUV_R == ubAct)?pCDB[4]:0xFF;
	if(FWUV_R == ubAct)
		pBuf[0] = pCDB[3];	
	switch(ubVendorCmd)
	{
	#ifdef S2019A
		case RFENG_CMD:
			sPRF_EngTstProc(pBuf);
			break;
	#endif
		default:
			break;
	}
}
//------------------------------------------------------------------------------
void APP_FWUgradeSetup(void)
{
	FWU_MODE_t tAPP_FwuMode = FWU_USBDMSC;
	char *pFW_Ver = SN937XX_FW_VERSION, *p, *q;
	FWU_MSCParam_t tAPP_FWUParam = {{0},{0},{0}, NULL, NULL};
	FWU_SDParam_t  tAPP_FWUSdParam;

	strncpy(tAPP_FWUParam.cVolumeLable, SNCCXX_VOLUME_LABLE, sizeof(tAPP_FWUParam.cVolumeLable));
	for(p=pFW_Ver; (q=strchr(p, '.'))!=NULL; p=q+1)
		p = q;
	strncpy(tAPP_FWUParam.cFileName, pFW_Ver, (p - pFW_Ver - 1));
	strncpy(tAPP_FWUParam.cFileNameExt, ((strlen(pFW_Ver) - (p - pFW_Ver)) > 3)?(p + 1):p, (strlen(pFW_Ver) - (p - pFW_Ver)));
	printf("copy down !!!!!!!!!!!\n");
#if !USBD_ENABLE
	if(FWU_USBDMSC == tAPP_FwuMode)
		tAPP_FwuMode = FWU_DISABLE;
#endif
	//! Setup check function for Model name or device tag.
	FWU_EnChkUpgFwFunc(FALSE, FALSE);
	//! USBD UPG Parameter Setup
	tAPP_FWUParam.pStsRptCbFunc 	= APP_FWUgradeStatusReport;
	tAPP_FWUParam.pVendorCmdCbFunc 	= APP_UsbVendorCmdProc;
	FWU_Setup(tAPP_FwuMode, &tAPP_FWUParam);
	//! SD UPG Parameter Setup
	tAPP_FWUSdParam.ubTargetFileNameLen = 8;
	strncpy(tAPP_FWUSdParam.cTargetFileName, "SNCC7XFW", tAPP_FWUSdParam.ubTargetFileNameLen);
	tAPP_FWUSdParam.pStsRptCbFunc = APP_FWUgradeStatusReport;
	tAPP_FWUSdParam.ubIncrementProgressBy = PROGRESS_BAR_SCALE;
	FWU_Setup(FWU_SDCARD, &tAPP_FWUSdParam);
	FWU_Enable();
}
//------------------------------------------------------------------------------
#ifdef BUC_CU
void APP_SwitchTrxBwExec(APP_EventMsg_t *ptEventMsg)
{
#ifdef RTC676x
	UI_CamsBwMode_t tAPP_TrxBwMode;
	uint8_t ubSlotNum[] = {
							[CAMS_1T30_BWMODE] = 1,
							[CAMS_1T60_BWMODE] = 1,
							[CAMS_2T_BWMODE]   = 2,
							[CAMS_4T_BWMODE]   = 4,
						  };
	int ulSlaveId[4], iRet = 0;

	tAPP_TrxBwMode = (UI_CamsBwMode_t)ptEventMsg->ubAPP_Message[1];
	if(tAPP_TrxBwMode >= CAMS_BWMODE_MAX)
		return;
	ulSlaveId[0] = ulKNL_GetSlaveId(0);
	ulSlaveId[1] = ulKNL_GetSlaveId(1);
	ulSlaveId[2] = ulKNL_GetSlaveId(2);
	ulSlaveId[3] = ulKNL_GetSlaveId(3);
	iRet = KNL_SetRwRemoteId(ulSlaveId, ubSlotNum[tAPP_TrxBwMode]);
	if(iRet != -1)
	{
		KNL_SetTXRSlotNum(ubSlotNum[tAPP_TrxBwMode]);
		VDO_SetStreamSlotNum(ubSlotNum[tAPP_TrxBwMode]);
	}
	tAPP_StsReport.tAPP_ReportType = APP_TRXBWSWRET_RPT;
	tAPP_StsReport.tAPP_State 	   = APP_IDLE_STATE;
	tAPP_StsReport.ubAPP_Report[0] = (-1 == iRet)?rFAIL:rSUCCESS;
	tAPP_StsReport.ubAPP_Report[1] = tAPP_TrxBwMode;
	UI_UpdateAppStatus(&tAPP_StsReport);
	tAPP_StsReport.tAPP_ReportType = APP_RPT_NONE;
#endif
}
//------------------------------------------------------------------------------
void APP_ViewTypeSetup(void)
{
	/*APP_EventMsg_t *ptViewTypeParam;	
	KNL_DISP_TYPE tAPP_KnlDispType;
	KNL_ROLE tAPP_KnlRole[4];

	ptViewTypeParam = tUI_ViewTypeSetup(&tAPP_ViewType);
	if(NULL == ptViewTypeParam)
		return;
	tAPP_KnlRole[0] = (ptViewTypeParam->ubAPP_Message[0] <= CAM4)?tAPP_STANumTable[ptViewTypeParam->ubAPP_Message[0]].tKNL_StaNum:KNL_NONE;
	tAPP_KnlRole[1] = (ptViewTypeParam->ubAPP_Message[1] <= CAM4)?tAPP_STANumTable[ptViewTypeParam->ubAPP_Message[1]].tKNL_StaNum:KNL_NONE;
	tAPP_KnlRole[2] = (ptViewTypeParam->ubAPP_Message[2] <= CAM4)?tAPP_STANumTable[ptViewTypeParam->ubAPP_Message[2]].tKNL_StaNum:KNL_NONE;
	tAPP_KnlRole[3] = (ptViewTypeParam->ubAPP_Message[3] <= CAM4)?tAPP_STANumTable[ptViewTypeParam->ubAPP_Message[3]].tKNL_StaNum:KNL_NONE;

	tAPP_KNLInfo.tAdoSrcRole = (ptViewTypeParam->ubAPP_Message[4] <= CAM4)?tAPP_STANumTable[ptViewTypeParam->ubAPP_Message[4]].tKNL_StaNum:KNL_NONE;

	tAPP_KnlDispType = (tAPP_ViewType == SINGLE_VIEW)?KNL_DISP_SINGLE:
					   (tAPP_ViewType == DUAL_VIEW)?KNL_DISP_DUAL_C:
					   (tAPP_ViewType == V3_3T_VIEW)?KNL_DISP_3T_3C:
					   (tAPP_ViewType == TRIPLE_2L1R_VIEW)?KNL_DISP_3T_2L1R:(tAPP_ViewType == TRIPLE_1L2R_VIEW)?KNL_DISP_3T_1L2R:
	                   (tAPP_ViewType == TRIPLE_2T1B_VIEW)?KNL_DISP_3T_2T1B:(tAPP_ViewType == TRIPLE_1T2B_VIEW)?KNL_DISP_3T_1T2B:
	                   (tAPP_ViewType == TRIPLE_3COL_VIEW)?KNL_DISP_3T_3COL:(tAPP_ViewType == QUAD_VIEW)?KNL_DISP_QUAD:KNL_DISP_H;
#if (defined(BSP_D_SNCC71_GM8285C_RX_V2) || defined(BSP_D_SNCC71_TP2915_AHD_RX_V3) || defined(BSP_D_SNCC71_NVP6021_AHD_RX_V4))
	tAPP_KnlDispType = (tAPP_ViewType == DUAL_VIEW)?KNL_DISP_DUAL_U:tAPP_KnlDispType;
#endif
	VDO_SwitchDisplayType(tAPP_KnlDispType, tAPP_KnlRole);*/

	APP_EventMsg_t *ptViewTypeParam;	
	KNL_DISP_TYPE tAPP_KnlDispType;
	KNL_ROLE tAPP_KnlRole[4];
	UI_CamViewType_t tAPP_ViewType1;

	ptViewTypeParam = tUI_ViewTypeSetup(&tAPP_ViewType1);
	if(NULL == ptViewTypeParam)
		return;
	tAPP_KnlRole[0] = (ptViewTypeParam->ubAPP_Message[0] <= CAM4)?tAPP_STANumTable[ptViewTypeParam->ubAPP_Message[0]].tKNL_StaNum:KNL_NONE;
	tAPP_KnlRole[1] = (ptViewTypeParam->ubAPP_Message[1] <= CAM4)?tAPP_STANumTable[ptViewTypeParam->ubAPP_Message[1]].tKNL_StaNum:KNL_NONE;
	tAPP_KnlRole[2] = (ptViewTypeParam->ubAPP_Message[2] <= CAM4)?tAPP_STANumTable[ptViewTypeParam->ubAPP_Message[2]].tKNL_StaNum:KNL_NONE;
	tAPP_KnlRole[3] = (ptViewTypeParam->ubAPP_Message[3] <= CAM4)?tAPP_STANumTable[ptViewTypeParam->ubAPP_Message[3]].tKNL_StaNum:KNL_NONE;
	
	tAPP_KNLInfo.tAdoSrcRole = (ptViewTypeParam->ubAPP_Message[4] <= CAM4)?tAPP_STANumTable[ptViewTypeParam->ubAPP_Message[4]].tKNL_StaNum:KNL_NONE;
	
	tAPP_KnlDispType = (tAPP_ViewType1 == SINGLE_VIEW)?KNL_DISP_SINGLE:
					   (tAPP_ViewType1 == DUAL_VIEW)?KNL_DISP_DUAL_U:
					   (tAPP_ViewType1 == QUAD_VIEW)?KNL_DISP_QUAD:KNL_DISP_SINGLE;
	VDO_DisplayMode_t tAPP_VdoDispMode = (VDO_DisplayMode_t)ptViewTypeParam->ubAPP_Message[6];

	printf("tAPP_KnlRole[0] = %d\n tAPP_KnlRole[1] = %d\n tAPP_KnlRole[2] = %d\n tAPP_KnlRole[3] = %d\n",
	tAPP_KnlRole[0],tAPP_KnlRole[1],tAPP_KnlRole[2],tAPP_KnlRole[3]);

	VDO_SetDisplayMode(tAPP_VdoDispMode);
	VDO_SwitchDisplayType(tAPP_KnlDispType, tAPP_KnlRole);
}
//------------------------------------------------------------------------------
void APP_SwitchViewTypeExec(APP_EventMsg_t *ptEventMsg)
{
	printf("APP_SwitchViewTypeExec\n");
	KNL_ROLE tKNL_Role[5];
	KNL_DISP_TYPE tKNL_DispType;
	UI_CamViewType_t tAPP_CamView;
	VDO_DisplayMode_t tAPP_VdoDispMode;
	uint8_t i;
	tAPP_CamView  = (UI_CamViewType_t)ptEventMsg->ubAPP_Message[1];
	tKNL_DispType = ((tAPP_CamView == SINGLE_VIEW) || (tAPP_CamView == SCAN_VIEW))?KNL_DISP_SINGLE:
	                (tAPP_CamView == DUAL_VIEW)?KNL_DISP_DUAL_U:
					(tAPP_CamView == H_VIEW)?KNL_DISP_H:
					(tAPP_CamView == V3_3T_VIEW)?KNL_DISP_3T_3C:KNL_DISP_QUAD;
					

	for(i = 0; i < 4; i++)
		tKNL_Role[i] = (ptEventMsg->ubAPP_Message[2+i] <= CAM4)?tAPP_STANumTable[ptEventMsg->ubAPP_Message[2+i]].tKNL_StaNum:KNL_NONE;
	
	tAPP_VdoDispMode = (VDO_DisplayMode_t)ptEventMsg->ubAPP_Message[6];
	VDO_SetDisplayMode(tAPP_VdoDispMode);
	VDO_SwitchDisplayType(tKNL_DispType, tKNL_Role);
	tAPP_StsReport.tAPP_ReportType = APP_VWMODESTS_RPT;
	tAPP_StsReport.ubAPP_Report[0] = tAPP_CamView;
	UI_UpdateAppStatus(&tAPP_StsReport);
	tAPP_StsReport.tAPP_ReportType = APP_RPT_NONE;
}
//------------------------------------------------------------------------------
void APP_LcdDisplayOff(void)
{
#if (LCD_PANEL != LCD_NO_PANEL)
	switch(LCD_PM)
	{
		case LCD_PWR_OFF:
			LCD_UnInit();
			LCD->LCD_MODE = LCD_GPIO;
			GLB->LCD_FUNC_DIS  = 1;
			SSP->SSP_GPIO_MODE = 1;
			//LCD_PWR_DISABLE;
			break;
		case LCD_PM_SUSPEND:
			LCD_Suspend();
			break;
		default:
			break;
	}
#endif   
}
//------------------------------------------------------------------------------
void APP_LcdDisplayOn(void)
{
#if (LCD_PANEL != LCD_NO_PANEL)
	switch(LCD_PM)
	{
		case LCD_PWR_OFF:
			//LCD_PWR_ENABLE;
			osDelay(200);
#if (defined(BSP_D_SN93701_SSD2828_RX_V5) || defined(BSP_D_SN93701_TC358778_RX_V6))
            LCD_RESET(10);
#endif
			GLB->LCD_FUNC_DIS = 0;
			LCD_Init(LCD_LCD_PANEL);
			VDO_UpdateDisplayParameter();
			LCD_Start();
			break;
		case LCD_PM_SUSPEND:
			LCD_Resume();
			break;
		default:
			return;
	}
	LCDBL_ENABLE(UI_ENABLE);
#endif    
}
#endif
//------------------------------------------------------------------------------
void APP_PowerSaveExec(APP_EventMsg_t *ptEventMsg)
{
#ifdef A7130
	UI_PowerSaveMode_t tAPP_PsMode = (UI_PowerSaveMode_t)ptEventMsg->ubAPP_Message[1];
#ifdef BUC_CU
	UI_CamNum_t tPsCamNum          = (UI_CamNum_t)ptEventMsg->ubAPP_Message[3];
	KNL_ROLE tKNL_Role;
#endif
	switch(tAPP_PsMode)
	{
		case PS_VOX_MODE:
		{
			VDO_PsFuncPtr_t tAPP_VoxFunc[] = {VDO_Start, VDO_Stop};
		#ifdef BUC_CU
			APP_ActFuncPtr_t tAPP_LcdFunc[] = {APP_LcdDisplayOn, APP_LcdDisplayOff};
			SYS_PowerState_t tAPP_PsState[]	= {SYS_PS0, SYS_PS1};
		#endif
			uint8_t ubAPP_PsFlag = ptEventMsg->ubAPP_Message[2];

			if(tAPP_VoxFunc[ubAPP_PsFlag].VDO_tPsFunPtr)
				tAPP_VoxFunc[ubAPP_PsFlag].VDO_tPsFunPtr();
		#ifdef BUC_CU
			SYS_SetPowerStates(tAPP_PsState[ubAPP_PsFlag]);
			SIGNAL_LED_IO_ENABLE((!ubAPP_PsFlag)?TRUE:FALSE);
			tAPP_StsReport.tAPP_ReportType = APP_VOXMODESTS_RPT;
			tAPP_StsReport.ubAPP_Report[0] = ubAPP_PsFlag;
			UI_UpdateAppStatus(&tAPP_StsReport);
			tAPP_StsReport.tAPP_ReportType = APP_RPT_NONE;
			if(tAPP_LcdFunc[ubAPP_PsFlag].APP_tActFunPtr)
				tAPP_LcdFunc[ubAPP_PsFlag].APP_tActFunPtr();
		#endif
			break;
		}
		case PS_ECO_MODE:
		#if (defined(BUC_CAM))
			VDO_ChangePlayState(tAPP_KNLInfo.tKNL_Role, VDO_STOP);
			KNL_EnableWORFunc();
			break;
		#elif (defined(BUC_CU))
		{
	    #if (APP_DOORPHONE_ENABLE==0)
			VDO_PlayState_t tAPP_VdoPlySte = (TRUE == ptEventMsg->ubAPP_Message[4])?VDO_START:VDO_STOP;

			tKNL_Role = tAPP_STANumTable[tPsCamNum].tKNL_StaNum;
			VDO_ChangePlayState(tKNL_Role, tAPP_VdoPlySte);
			if((BB_ENABLE_ALL_STA_WAKEUP == tKNL_GetWORMode()) && (FALSE == ptEventMsg->ubAPP_Message[2]))
				break;
			KNL_WakeupDevice(tKNL_Role, ptEventMsg->ubAPP_Message[2]);
        #else
			VDO_PlayState_t tAPP_VdoPlySte = (TRUE == ptEventMsg->ubAPP_Message[4])?VDO_START:VDO_STOP;
			tKNL_Role = tAPP_STANumTable[tPsCamNum].tKNL_StaNum;
			VDO_ChangePlayState(tKNL_Role, tAPP_VdoPlySte);
			KNL_WakeupDevice(tKNL_Role, ptEventMsg->ubAPP_Message[2]);            
            APP_DP_APSetStatus(tKNL_Role,APP_DP_WAITMONITOR,APP_DP_TM_WAITMONITOR);
            KNL_DP_SetVideoOnOff(tKNL_Role,1);
            APP_DP_SetCmd(APP_DP_CMD_MONITORREQ,tKNL_Role,NULL,NULL);
        #endif
			break;
		}
		#endif
		case PS_WOR_MODE:
		#if (defined(BUC_CAM))
		{
			VDO_PsFuncPtr_t tAPP_WorFunc[] = {VDO_Stop, VDO_Start};
			uint8_t ubAPP_PsFlag = ptEventMsg->ubAPP_Message[2];
			uint8_t ubAPP_VdoActFlag = ptEventMsg->ubAPP_Message[4];

			if((TRUE == ubAPP_VdoActFlag) &&
			   (tAPP_WorFunc[ubAPP_PsFlag].VDO_tPsFunPtr))
				tAPP_WorFunc[ubAPP_PsFlag].VDO_tPsFunPtr();
			KNL_WakeupDevice(KNL_MASTER_AP, ptEventMsg->ubAPP_Message[3]);
			break;
		}
		#elif (defined(BUC_CU))
			KNL_EnableWORFunc();
			break;
		case PS_ADOONLY_MODE:
		{
			VDO_PsFuncPtr_t tAPP_AdoOnFunc[] = {VDO_Start, VDO_Stop};
			APP_ActFuncPtr_t tAPP_LcdFunc[]  = {LCD_Resume, LCD_Suspend};
			SYS_PowerState_t tAPP_PsState[]	 = {SYS_PS0, SYS_PS1};
			uint8_t ubAPP_PsFlag 			 = ptEventMsg->ubAPP_Message[2];

			KNL_SetTRXPathActivity();
			if(tAPP_AdoOnFunc[ubAPP_PsFlag].VDO_tPsFunPtr)
				tAPP_AdoOnFunc[ubAPP_PsFlag].VDO_tPsFunPtr();
			SYS_SetPowerStates(tAPP_PsState[ubAPP_PsFlag]);
			SIGNAL_LED_IO_ENABLE ((!ubAPP_PsFlag)?TRUE:FALSE);
			if(tAPP_LcdFunc[ubAPP_PsFlag].APP_tActFunPtr)
				tAPP_LcdFunc[ubAPP_PsFlag].APP_tActFunPtr();
			#if (LCD_PANEL != LCD_NO_PANEL)
			if(FALSE == ubAPP_PsFlag)
				LCDBL_ENABLE(UI_ENABLE);
			#endif
			break;
		}
		case POWER_NORMAL_MODE:
			tKNL_Role = tAPP_STANumTable[tPsCamNum].tKNL_StaNum;
			KNL_WakeupDevice(tKNL_Role, FALSE);
			break;
		#endif
		default:
			break;
	}
#endif
}
//------------------------------------------------------------------------------
void APP_SetTuningToolMode(APP_TuningMode_t tTuningMode)
{
	tAPP_KNLInfo.tTuningMode = tTuningMode;
	switch(tTuningMode)
	{
		case APP_TUNINGMODE_ON:
			tAPP_KNLInfo.tUsbdClassMode = USBD_UVC_MODE;
			KNL_TurnOnTuningTool();
			break;
		case APP_TUNINGMODE_OFF:
			tAPP_KNLInfo.tUsbdClassMode = USBD_DEFU_CLASS;
			KNL_TurnOffTuningTool();
			break;
		default:
			return;
	}
	APP_UpdateKNLSetupInfo();
}
//------------------------------------------------------------------------------
APP_TuningMode_t APP_GetTuningToolMode(void)
{
	return tAPP_KNLInfo.tTuningMode;
}
//------------------------------------------------------------------------------

void APP_SetRfTestMode(APP_RfTestMode_t tRfTestMode)
{
	tAPP_KNLInfo.tRfTestMode = tRfTestMode;
	APP_UpdateKNLSetupInfo();
}
//------------------------------------------------------------------------------
APP_RfTestMode_t APP_GetRfTestMode(void)
{
	return tAPP_KNLInfo.tRfTestMode;
}

uint32_t APP_GetRFVCOGroup(int index)
{
	return tAPP_KNLInfo.ulRFVCOGroup[index];
}

void APP_SetRFVCOGroup(int index,uint32_t group)
{
	tAPP_KNLInfo.ulRFVCOGroup[index] = group;
	APP_UpdateKNLSetupInfo();
}
//------------------------------------------------------------------------------

void UART2_SendOneByte(char ch)
{
	while(!UART2->TX_RDY);
	UART2->RS_DATA = ch;
	ch = ch;
}

void APP_TxEnterSleepMode(KNL_ROLE tKNL_Role)
{
	uint8_t ubTemp[8],i,j;
	char num;
	uint32_t ulSlaveId = ulKNL_GetSlaveId(tKNL_Role);
	
	for(i = 0;i < 8;i++)
	{
		ubTemp[i] = ulSlaveId&0xf;
		ulSlaveId >>= 4;
	}
	UART2_SendOneByte('R');
	UART2_SendOneByte('Q');
	UART2_SendOneByte(':');
	for(j = 0;j < 2;j++)
	{
		sprintf(&num,"%x",ubTemp[1]);
		UART2_SendOneByte(num);

		sprintf(&num,"%x",ubTemp[0]);
		UART2_SendOneByte(num);
		
		sprintf(&num,"%x",ubTemp[3]);
		UART2_SendOneByte(num);

		sprintf(&num,"%x",ubTemp[2]);
		UART2_SendOneByte(num);

		sprintf(&num,"%x",ubTemp[5]);
		UART2_SendOneByte(num);
		
		sprintf(&num,"%x",ubTemp[4]);
		UART2_SendOneByte(num);

		sprintf(&num,"%x",ubTemp[7]);
		UART2_SendOneByte(num);
		
		sprintf(&num,"%x",ubTemp[6]);
		UART2_SendOneByte(num);
	}

}
//------------------------------------------------------------------------------
void APP_WakeUpTx(KNL_ROLE tKNL_Role)
{
	uint8_t ubTemp[8],i,j;
	char num;
	uint32_t ulSlaveId = ulKNL_GetSlaveId(tKNL_Role);
	for(i = 0;i < 8;i++)
	{
		ubTemp[i] = ulSlaveId&0xf;
		ulSlaveId >>= 4;
	}
	UART2_SendOneByte('I');
	UART2_SendOneByte('D');
	UART2_SendOneByte(':');
	for(j = 0;j < 2;j++)
	{
		sprintf(&num,"%x",ubTemp[1]);
		UART2_SendOneByte(num);

		sprintf(&num,"%x",ubTemp[0]);
		UART2_SendOneByte(num);
		
		sprintf(&num,"%x",ubTemp[3]);
		UART2_SendOneByte(num);

		sprintf(&num,"%x",ubTemp[2]);
		UART2_SendOneByte(num);

		sprintf(&num,"%x",ubTemp[5]);
		UART2_SendOneByte(num);
		
		sprintf(&num,"%x",ubTemp[4]);
		UART2_SendOneByte(num);

		sprintf(&num,"%x",ubTemp[7]);
		UART2_SendOneByte(num);
		
		sprintf(&num,"%x",ubTemp[6]);
		UART2_SendOneByte(num);
	}

}
//------------------------------------------------------------------------------
void APP_Disconnect(void)
{
	printf("APP_Disconnect\n");
	uint32_t ulSlaveId[4] = {0,0,0,0};
	KNL_SetRwRemoteId(ulSlaveId, 4);
}

//------------------------------------------------------------------------------
void APP_RefreshBandwidth(APP_EventMsg_t *ptEventMsg)
{
	uint8_t ubIdx;
	KNL_ROLE tKNL_Role[4];
	uint32_t ulSlaveId[4] = {0,0,0,0};
	uint8_t ubCamNum  = ptEventMsg->ubAPP_Message[1];
	for(ubIdx = 0; ubIdx < 4; ubIdx++)
		tKNL_Role[ubIdx] = (ptEventMsg->ubAPP_Message[2 + ubIdx] <= CAM4)?tAPP_STANumTable[ptEventMsg->ubAPP_Message[2 + ubIdx]].tKNL_StaNum:KNL_NONE;
	//����֮���Է����������Ϊ�˺��л���������̶�Ӧ
	if(ubCamNum <= 2)
	{
		for(ubIdx = 0; ubIdx < ubCamNum; ubIdx++)
			ulSlaveId[ubIdx] = ulKNL_GetSlaveId(tKNL_Role[ubIdx]);
	}
	else
	{
		for(ubIdx = 0; ubIdx < 4; ubIdx++)
			ulSlaveId[ubIdx] = ulKNL_GetSlaveId(ubIdx);
	}
	KNL_SetRwRemoteId(ulSlaveId, ubCamNum);
}

//------------------------------------------------------------------------------
void APP_Start(void)
{
#ifdef A7130
	//! WOR Mode Setup
    #if (APP_DOORPHONE_ENABLE==0)    
	KNL_SetWORMode(BB_ENABLE_ALL_STA_WAKEUP);
    #else
	KNL_SetWORMode(BB_ENABLE_T2R_WAKEUP);
    #endif
#endif
	SADC_SetPinMode(SADC_CH4, SADC_PIN_ANALOG_INPUT_MODE);//����
	SADC_Enable();
	OSD_LogoJpeg(OSDLOGO_BOOT);
//	ADO_WavRepeat(0);
//	osDelay(100);
//	SET_PANEL_PWM((UI_GetBackLightPwm() < LCD_PWM_MIN)?LCD_PWM_MIN : UI_GetBackLightPwm());
//	osDelay(2500);
//	ADO_WavStop();

	//! Kernel Setup
	KNL_BlockInit();
	KNL_ResetLcdChannel();

	APP_ViewTypeSetup();
	SET_VOLUME_PWM(UI_GetVolumePwm());
//4875控制电源引脚
	//osDelay(50);
	if(UI_GetVolumePwm() == 0)
	{
		SET_VOLUME_IO = 0;
	}
	else
	{
		SET_VOLUME_IO = 1;
	}

#ifdef RTC676x
	if (tAPP_KNLInfo.tRfTestMode == APP_RF_TEST_OFF)
#endif
	{
		//! Video Start
		VDO_Start();

		//! Audio Start
		ADO_Start(tAPP_KNLInfo.tAdoSrcRole);

		//! Two way command Start
		TWC_Start();
	}
	tAPP_StsReport.tAPP_State = APP_IDLE_STATE;
	UI_UpdateAppStatus(&tAPP_StsReport);

#ifdef RTC676x
    if (tAPP_KNLInfo.tRfTestMode == APP_RF_TEST_ON)
        RTC676x_TestInit();
#endif

}
#if (APP_DOORPHONE_ENABLE==1)
#define DP_LOGLVL    DBG_CriticalLvl
const char* DPStatusTab[APP_DP_MAX] = {
    "POWERON",
    "IDLE",
    "LISTEN",
    "CALL",
    "WAITMONITOR",
    "MONITOR",
    "TALK",
    "PAIRING",
};

void APP_DP_Thread(void const *argument)
{
    static uint8_t ubDPTimes = 1;
#ifdef BUC_CU
    //KNL_ROLE ubKNL_RoleNum;
    UI_CamNum_t ubCamNum;
#endif

	while(1)
	{
        if(ubDPTimes == 10)
        {
            ubDPTimes = 1;
#ifdef BUC_CU
            for(ubCamNum = CAM1; ubCamNum <= CAM4; ubCamNum++)
                APP_DP_APHandler(ubCamNum);
#endif
#ifdef BUC_CAM
            APP_DP_STAHandler();
#endif
        }
        else    ubDPTimes++;

#ifdef BUC_CU
        for(ubCamNum = CAM1; ubCamNum <= CAM4; ubCamNum++)
            APP_DP_APCmdRecvProc(ubCamNum);
#endif
#ifdef BUC_CAM
        APP_DP_STACmdRecvProc(ubPAIR_GetNowGroup());
#endif
        osDelay(10);
	}    
}

uint8_t APP_DP_SetCmd(uint8_t ubCmd, uint8_t ubKNL_ROLE, uint8_t ubData0, uint8_t ubData1)
{
    APP_DP_CmdPacket_t tAPP_DPCmdPacket;
    
    tAPP_DPCmdPacket.ubAPP_DPCmd = ubCmd;
    tAPP_DPCmdPacket.ubAPP_DPID = ubKNL_ROLE;
    tAPP_DPCmdPacket.ubAPP_DPID = ubKNL_ROLE;
    tAPP_DPCmdPacket.ubAPP_DPData[0] = ubData0;
    tAPP_DPCmdPacket.ubAPP_DPData[1] = ubData1;
    BB_SetFastCmd((uint8_t*)&tAPP_DPCmdPacket);
    return 1;
}

#if defined(BUC_CU)
void APP_DP_APHandler(uint8_t ubCamNum)
{
    if(ubAPP_DP_APHandlerCnt[ubCamNum] <= ubAPP_DP_APHandlerTargetCnt[ubCamNum])    ubAPP_DP_APHandlerCnt[ubCamNum]++;
    else    return;

    if(ubAPP_DP_APHandlerTargetCnt[ubCamNum]>1)
    printd(DP_LOGLVL,"DpApSt(%d,%s) CT(%d,%d)\n", ubCamNum, DPStatusTab[ubAPP_DP_APStatus[ubCamNum]],
                                                    ubAPP_DP_APHandlerCnt[ubCamNum],
                                                    ubAPP_DP_APHandlerTargetCnt[ubCamNum]);

    if(ubAPP_DP_APHandlerCnt[ubCamNum] != ubAPP_DP_APHandlerTargetCnt[ubCamNum])    return;

    switch(ubAPP_DP_APStatus[ubCamNum])
    {
        case APP_DP_POWERON:
            KNL_DP_SetVideoOnOff(tAPP_STANumTable[ubCamNum].tKNL_StaNum,0);
            APP_DP_APSetStatus(ubCamNum,APP_DP_IDLE,APP_DP_TM_IDLE);
            break;
            
        case APP_DP_IDLE:
            KNL_DP_SetVideoOnOff(tAPP_STANumTable[ubCamNum].tKNL_StaNum,0);
            break;
            
        case APP_DP_CALL:
            // All CALL Timeout,TurnOff All Wave
            ADO_WavStop();
            if(ubAPP_DP_APPreStatus[ubCamNum] == APP_DP_MONITOR)
            {
                printd(DP_LOGLVL,"RetMonitor\n");
                APP_DP_APSetStatus(ubCamNum,APP_DP_MONITOR,APP_DP_TM_LOOP);
            }
            else
            {
                printd(DP_LOGLVL,"RetIdle\n");
                APP_DP_APSetStatus(ubCamNum,APP_DP_IDLE,APP_DP_TM_IDLE);
            }
            break;

        case APP_DP_WAITMONITOR:
            APP_DP_APSetStatus(ubCamNum,APP_DP_IDLE,APP_DP_TM_IDLE);
            APP_DP_SetCmd(APP_DP_CMD_IDLE,tAPP_STANumTable[ubCamNum].tKNL_StaNum,NULL,NULL);
            BB_SetWakeUp(FALSE, 0xFF);
            break;

        case APP_DP_MONITOR:
        case APP_DP_TALK:
            if(ubKNL_GetCommLinkStatus(tAPP_STANumTable[ubCamNum].tKNL_StaNum) == BB_LOST_LINK)  ubApp_DP_APLostLinkCnt[ubCamNum]++;
            else    ubApp_DP_APLostLinkCnt[ubCamNum] = 0;

            if(ubApp_DP_APLostLinkCnt[ubCamNum] > 30)
            {
                ubApp_DP_APLostLinkCnt[ubCamNum] = 0;
                APP_DP_APSetStatus(ubCamNum,APP_DP_IDLE,APP_DP_TM_IDLE);
                APP_DP_SetCmd(APP_DP_CMD_IDLE,tAPP_STANumTable[ubCamNum].tKNL_StaNum,NULL,NULL);
            }
            else
            {
                ubAPP_DP_APHandlerCnt[ubCamNum]=0;
                ubAPP_DP_APHandlerTargetCnt[ubCamNum]=APP_DP_TM_LOOP;
            }
            break;
            
        case APP_DP_PAIRING:
            ubAPP_DP_APHandlerCnt[ubCamNum]=0;
            ubAPP_DP_APHandlerTargetCnt[ubCamNum]=APP_DP_TM_LOOP;
            break;
    }
}

void APP_DP_APCmdRecvProc(uint8_t ubCamNum)
{
    uint8_t ulRecv[4];
    BB_GetRxFastCmd(ubCamNum,(uint8_t*)&ulRecv);

    if(ubAPP_DP_APPreCmd[ubCamNum] == ulRecv[0])    
    {
        if( ulRecv[0] == APP_DP_CMD_CALL || ulRecv[0] == APP_DP_CMD_SWITCH || ulRecv[0] == APP_DP_CMD_MONITOR || ulRecv[0] == APP_DP_CMD_ACK )
        {
            // Check Receive command flow compelet.
            if(ubAPP_DP_APCmdRetryTmOut[ubCamNum] != APP_DP_CMD_RETRY_CNT)
                ubAPP_DP_APCmdRetryTmOut[ubCamNum]++;
            else
            {
                // Over loop Number, Reset bus, otherwise retry send response command  
                if(ubAPP_DP_APCmdRetryLoop[ubCamNum] == APP_DP_CMD_RETRY_LOOP)
                    APP_DP_SetCmd(APP_DP_CMD_TIMEOUT,tAPP_STANumTable[ubCamNum].tKNL_StaNum,NULL,NULL);
                else
                {
                    if(ulRecv[0]==APP_DP_CMD_ACK)
                        APP_DP_SetCmd(APP_DP_CMD_IDLE,tAPP_STANumTable[ubCamNum].tKNL_StaNum,NULL,NULL);
                    else
                        APP_DP_SetCmd(APP_DP_CMD_ACK,tAPP_STANumTable[ubCamNum].tKNL_StaNum,NULL,NULL);
                    
                    ubAPP_DP_APCmdRetryTmOut[ubCamNum] = 0;
                    ubAPP_DP_APCmdRetryLoop[ubCamNum]++;
                    ubAPP_DP_APPreCmd[ubCamNum] = ulRecv[0];
                }
            }
        }
        else if((ulRecv[0] == APP_DP_CMD_TIMEOUT) && (++ubAPP_DP_APCmdRetryTmOut[ubCamNum] == APP_DP_CMD_RETRY_CNT))
        {
            ubAPP_DP_APCmdRetryTmOut[ubCamNum] = 0;
            ubAPP_DP_APCmdRetryLoop[ubCamNum] = 0;
            ubAPP_DP_APPreCmd[ubCamNum] = ulRecv[0];
            APP_DP_SetCmd(APP_DP_CMD_IDLE,tAPP_STANumTable[ubCamNum].tKNL_StaNum,NULL,NULL);
        }
        return;
    }
    printd(DP_LOGLVL,"APRev[%d]=[%d %d %d %d][%d]\n",ubCamNum,ulRecv[0],ulRecv[1],ulRecv[2],ulRecv[3],ubAPP_DP_APPreCmd[ubCamNum]);    
    switch(ulRecv[0])
    {
        case APP_DP_CMD_CALL:
            APP_DP_SetCmd(APP_DP_CMD_ACK,tAPP_STANumTable[ubCamNum].tKNL_StaNum,NULL,NULL);
            break;
        
        case APP_DP_CMD_SWITCH:
            APP_DP_SetCmd(APP_DP_CMD_ACK,tAPP_STANumTable[ubCamNum].tKNL_StaNum,NULL,NULL);
            break;

        case APP_DP_CMD_ACK:
            printd(DP_LOGLVL,"ApDpAck\n");
            APP_DP_SetCmd(APP_DP_CMD_IDLE,tAPP_STANumTable[ubCamNum].tKNL_StaNum,NULL,NULL);
            break;
            
        case APP_DP_CMD_TIMEOUT:
            printd(DP_LOGLVL,"ApDpTimeOut\n");
            APP_DP_SetCmd(APP_DP_CMD_IDLE,tAPP_STANumTable[ubCamNum].tKNL_StaNum,NULL,NULL);
            break;
            
        case APP_DP_CMD_MISMATCH:
            printd(DP_LOGLVL,"ApDpMitchMatch\n");
            if(ubAPP_DP_APStatus[ubCamNum] != APP_DP_WAITMONITOR)
                APP_DP_SetCmd(APP_DP_CMD_IDLE,tAPP_STANumTable[ubCamNum].tKNL_StaNum,NULL,NULL);
            break;    

        case APP_DP_CMD_IDLE:
            printd(DP_LOGLVL,"ApDpIdle\n");
            APP_DP_SetCmd(APP_DP_CMD_IDLE,tAPP_STANumTable[ubCamNum].tKNL_StaNum,NULL,NULL);
            if(ubAPP_DP_APPreCmd[ubCamNum] == APP_DP_CMD_CALL)
            {
                ADO_WavStop();
                printd(DP_LOGLVL,"ApDpRingIn %s\n",DPStatusTab[ubAPP_DP_APStatus[ubCamNum]]);
                if(ubAPP_DP_APStatus[ubCamNum] == APP_DP_TALK || ubAPP_DP_APStatus[ubCamNum] == APP_DP_PAIRING )
                    break;
                else if(ubAPP_DP_APStatus[ubCamNum] == APP_DP_MONITOR)
                {
                    APP_DP_APSetStatus(ubCamNum,APP_DP_CALL,APP_DP_TM_CALL);
                }
                else if(ubAPP_DP_APStatus[ubCamNum] == APP_DP_TM_CALL && ubAPP_DP_APPreStatus[ubCamNum] == APP_DP_MONITOR)
                {
                    ubAPP_DP_APHandlerCnt[ubCamNum]=0;
                    ubAPP_DP_APHandlerTargetCnt[ubCamNum]=APP_DP_TM_CALL;                    
                }
                else
                {
                    KNL_DP_SetVideoOnOff(tAPP_STANumTable[ubCamNum].tKNL_StaNum,1);
                    APP_DP_APSetStatus(ubCamNum,APP_DP_CALL,APP_DP_TM_CALL);
                }
                ADO_WavPlay(0);
            }
            else if(ubAPP_DP_APPreCmd[ubCamNum] == APP_DP_CMD_MONITOR)
            {
                printd(DP_LOGLVL,"ApDpMonitor\n");
                APP_DP_APSetStatus(ubCamNum,APP_DP_MONITOR,APP_DP_TM_LOOP);
            }
            else if(ubAPP_DP_APPreCmd[ubCamNum] == APP_DP_CMD_SWITCH)
            {
                printd(DP_LOGLVL,"ApDpSwitch\n");
                ADO_WavStop();
                APP_DP_APSetStatus(ubCamNum,APP_DP_IDLE,APP_DP_TM_IDLE);
            }
            break;
    }
    ubAPP_DP_APCmdRetryTmOut[ubCamNum] = 0;
    ubAPP_DP_APCmdRetryLoop[ubCamNum] = 0;
    ubAPP_DP_APPreCmd[ubCamNum] = ulRecv[0];
}
void APP_DP_Exec(APP_EventMsg_t *ptEventMsg)
{
    KNL_ROLE tKNL_Role = tAPP_STANumTable[ptEventMsg->ubAPP_Message[2]].tKNL_StaNum;
    switch(ptEventMsg->ubAPP_Message[1])
    {
        #if(APP_DP_TXPS_MODE==0)
        case APP_DP_CMD_MONITORREQ:
            APP_DP_APSetStatus(tKNL_Role,APP_DP_WAITMONITOR,APP_DP_TM_WAITMONITOR);
            KNL_DP_SetVideoOnOff(tKNL_Role,1);
            APP_DP_SetCmd(APP_DP_CMD_MONITORREQ,tKNL_Role,NULL,NULL);            
        break;
        #endif
        case APP_DP_CMD_TALK:
		#if (APP_ADO_FUNC_ENABLE == 1)
        {      
            if(ptEventMsg->ubAPP_Message[3])
            {
                // accept Ringing on mic speak
               tAPP_KNLInfo.tAdoSrcRole = tKNL_Role;
               ADO_Start(tAPP_KNLInfo.tAdoSrcRole);
               APP_UpdateKNLSetupInfo();
               ADO_WavStop();
               ADO_PTTStart();
               APP_DP_APSetStatus(tKNL_Role,APP_DP_TALK,APP_DP_TM_LOOP);
               APP_DP_SetCmd(APP_DP_CMD_TALK,tKNL_Role,NULL,NULL);
            }
        }
		#endif            
        break;
        
        case APP_DP_CMD_HANDUP:
            {
                ADO_PTTStop();
                if(ptEventMsg->ubAPP_Message[3])
                {
                    tAPP_KNLInfo.tAdoSrcRole = tAPP_STANumTable[ptEventMsg->ubAPP_Message[4]].tKNL_StaNum;
                    ADO_Start(tAPP_KNLInfo.tAdoSrcRole);
                    APP_UpdateKNLSetupInfo();
                    ADO_PTTStart();
                }
                APP_DP_APSetStatus(tKNL_Role,APP_DP_IDLE,APP_DP_TM_IDLE);
                APP_DP_SetCmd(APP_DP_CMD_HANDUP,tKNL_Role,NULL,NULL);
            }            
        break;            
    }
}

void APP_DP_APSetStatus(uint8_t ubCamNum,uint8_t ubValue, uint8_t ubTargetCnt)
{
    ubAPP_DP_APPreStatus[ubCamNum] = ubAPP_DP_APStatus[ubCamNum];
    ubAPP_DP_APStatus[ubCamNum] = ubValue;

    ubAPP_DP_APHandlerCnt[ubCamNum]=0;
    ubAPP_DP_APHandlerTargetCnt[ubCamNum]=ubTargetCnt;    
}
uint8_t APP_DP_APGetStatus(uint8_t ubCamNum)
{
    return ubAPP_DP_APStatus[ubCamNum];
}
#endif
#ifdef BUC_CAM
void APP_DP_STAHandler(void)
{
    if(ubAPP_DP_STAHandlerCnt <= ubAPP_DP_STAHandlerTargetCnt)    ubAPP_DP_STAHandlerCnt++;
    else    return;

    if(ubAPP_DP_STAHandlerTargetCnt>1)
    printd(DP_LOGLVL,"DpStaSt(%d,%s) CT(%d,%d)\n",ubKNL_GetRole(),DPStatusTab[ubAPP_DP_STAStatus],ubAPP_DP_STAHandlerCnt,ubAPP_DP_STAHandlerTargetCnt);

    if(ubAPP_DP_STAHandlerCnt != ubAPP_DP_STAHandlerTargetCnt)    return;    
    
    switch (ubAPP_DP_STAStatus)
    {
        case APP_DP_POWERON:
            APP_DP_STASetStatus(APP_DP_LISTEN,APP_DP_TM_LISTEN_STA);
        break;
        
        case APP_DP_IDLE:
                BB_SetRfStopTx(1);
                #if(APP_DP_TXPS_MODE==1)
                APP_DP_STAPowerSaveExec();
                #else
                ubAPP_ListenLoop=0;
                APP_DP_STASetStatus(APP_DP_LISTEN,APP_DP_TM_LISTEN_STA);
                #endif
        break;

        case APP_DP_LISTEN:
            if(ubAPP_ListenLoop==APP_DP_LOOP_LISTEN)
            {
                APP_DP_STASetStatus(APP_DP_IDLE,APP_DP_TM_IDLE);
                break;
            }
            if( ubAPP_DP_STAHandlerCnt == ubAPP_DP_STAHandlerTargetCnt)
            {
                ubAPP_DP_STAHandlerCnt=0;
                ubAPP_ListenLoop++;
                if(ubPAIR_GetNowGroup())
                    ubPAIR_ChangeStaIDGroup(0);
                else
                    ubPAIR_ChangeStaIDGroup(1);
            }
            break;

        case APP_DP_CALL:
            ADO_WavStop();
            if(ubAPP_DP_STAPreStatus == APP_DP_MONITOR && ubAPP_DP_STAGPChange == 0)
            {
                printd(DP_LOGLVL,"RetMonitor\n");
                APP_DP_STASetStatus(APP_DP_MONITOR,APP_DP_TM_LOOP);
            }
            else
            {
                printd(DP_LOGLVL,"RetIdle\n");
                APP_DP_STASetStatus(APP_DP_IDLE,APP_DP_TM_IDLE);
                APP_DP_SetCmd(APP_DP_CMD_IDLE,ubKNL_GetRole(),NULL,NULL);
            }
            ubAPP_DP_STAGPChange=0;
        break;

        case APP_DP_MONITOR:
            if(ubAPP_DP_STA_Monitorfg)
            {
                ubAPP_DP_STA_Monitorfg = 0;
                APP_DP_SetCmd(APP_DP_CMD_MONITOR,ubKNL_GetRole(),NULL,NULL);
            }
        case APP_DP_TALK:
            if(ubKNL_GetCommLinkStatus(KNL_MASTER_AP) == BB_LOST_LINK)  ubApp_DP_STALostLinkCnt++;
            else    ubApp_DP_STALostLinkCnt = 0;

            if(ubApp_DP_STALostLinkCnt > 30)
            {
                ubApp_DP_STALostLinkCnt = 0;
                APP_DP_STASetStatus(APP_DP_IDLE,APP_DP_TM_IDLE);
            }
            else
            {
                ubAPP_DP_STAHandlerCnt=0;
                ubAPP_DP_STAHandlerTargetCnt=APP_DP_TM_LOOP;
            }
            break;
        case APP_DP_PAIRING:
            ubAPP_DP_STAHandlerCnt=0;
            ubAPP_DP_STAHandlerTargetCnt=APP_DP_TM_LOOP;
            break;
    }
}

void APP_DP_STACmdRecvProc(uint8_t ubNb)
{
    uint8_t ulRecv[4];
    BB_GetRxFastCmd(ubNb,(uint8_t*)&ulRecv);

    if(ubAPP_DP_STAPreCmd[ubNb] == ulRecv[0])    
    {
        // Check Receive command flow compelet.
        if( ulRecv[0] == APP_DP_CMD_TALK || ulRecv[0] == APP_DP_CMD_HANDUP || ulRecv[0] == APP_DP_CMD_MONITORREQ || ulRecv[0] == APP_DP_CMD_ACK )
        {
            if(ubAPP_DP_STACmdRetryTmOut[ubNb]!=APP_DP_CMD_RETRY_CNT )
                ubAPP_DP_STACmdRetryTmOut[ubNb]++;
            else
            {
                // Over loop Number, Reset bus, otherwise retry send response command  
                if(ubAPP_DP_STACmdRetryLoop[ubNb] == APP_DP_CMD_RETRY_LOOP)
                    APP_DP_SetCmd(APP_DP_CMD_TIMEOUT,ubKNL_GetRole(),NULL,NULL);
                else
                {
                    if(ulRecv[0]==APP_DP_CMD_ACK)
                        APP_DP_SetCmd(APP_DP_CMD_IDLE,ubKNL_GetRole(),NULL,NULL);
                    else
                        APP_DP_SetCmd(APP_DP_CMD_ACK,ubKNL_GetRole(),NULL,NULL);
                    
                    ubAPP_DP_STACmdRetryTmOut[ubNb] = 0;
                    ubAPP_DP_STACmdRetryLoop[ubNb]++;
                    ubAPP_DP_STAPreCmd[ubNb] = ulRecv[0];
                }
            }
        }
        else if(ulRecv[0] == APP_DP_CMD_TIMEOUT && (++ubAPP_DP_STACmdRetryTmOut[ubNb] == APP_DP_CMD_RETRY_CNT))
        {
            ubAPP_DP_STACmdRetryTmOut[ubNb] = 0;
            ubAPP_DP_STAPreCmd[ubNb] = ulRecv[0];
            APP_DP_SetCmd(APP_DP_CMD_IDLE,tAPP_STANumTable[ubNb].tKNL_StaNum,NULL,NULL);
        }        
        return;
    }
    
    if(ubKNL_GetRole() != ulRecv[1])
    {
       // printf("STA No. Mismatch %d %d %d\n",ubKNL_GetRole(),ulRecv[0],ulRecv[1]);
        ubAPP_DP_STACmdRetryTmOut[ubNb] = 0;
        ubAPP_DP_STACmdRetryLoop[ubNb]=0;
        return;
    }
    
    printd(DP_LOGLVL,"DPRev[%d]=[%d %d %d %d][%d]\n",ubNb,ulRecv[0],ulRecv[1],ulRecv[2],ulRecv[3],ubAPP_DP_STAPreCmd[ubNb]); 
    switch(ulRecv[0])
    {
        case APP_DP_CMD_TALK:
            APP_DP_SetCmd(APP_DP_CMD_ACK,ubKNL_GetRole(),NULL,NULL);
            break;

        case APP_DP_CMD_HANDUP:
            APP_DP_SetCmd(APP_DP_CMD_ACK,ubKNL_GetRole(),NULL,NULL);
            break;

        case APP_DP_CMD_MONITORREQ:
            #if(APP_DP_TXPS_MODE==0)
            BB_SetRfStopTx(0);
            #endif
            APP_DP_SetCmd(APP_DP_CMD_ACK,ubKNL_GetRole(),NULL,NULL);
            break;
            
        case APP_DP_CMD_ACK:
            printd(DP_LOGLVL,"StaDpAck\n");
            APP_DP_SetCmd(APP_DP_CMD_IDLE,ubKNL_GetRole(),NULL,NULL);
            break;
            
        case APP_DP_CMD_TIMEOUT:
            printd(DP_LOGLVL,"StaDpTimeOut\n");
            APP_DP_SetCmd(APP_DP_CMD_IDLE,ubKNL_GetRole(),NULL,NULL);
            break;            
            
        case APP_DP_CMD_IDLE:
            printd(DP_LOGLVL,"StaDpIdle\n");
            APP_DP_SetCmd(APP_DP_CMD_IDLE,ubKNL_GetRole(),NULL,NULL);
            if(ubAPP_DP_STAPreCmd[ubNb] == APP_DP_CMD_HANDUP)
            {
                printd(DP_LOGLVL,"StaDpHandup\n");
                APP_DP_STASetStatus(APP_DP_IDLE,APP_DP_TM_HANDUP);
            }
            else if(ubAPP_DP_STAPreCmd[ubNb] == APP_DP_CMD_TALK)
            {
                printd(DP_LOGLVL,"StaDpAnswer\n");
                ADO_WavStop();
                APP_DP_STASetStatus(APP_DP_TALK,APP_DP_TM_LOOP);
            }
            else if(ubAPP_DP_STAPreCmd[ubNb] == APP_DP_CMD_MONITORREQ)
            {
                printd(DP_LOGLVL,"StaDpMonitor\n");
                ubAPP_DP_STA_Monitorfg=1;
                APP_DP_STASetStatus(APP_DP_MONITOR,APP_DP_TM_LOOP);
            }
            break;    
    }
    ubAPP_DP_STACmdRetryTmOut[ubNb] = 0;
    ubAPP_DP_STACmdRetryLoop[ubNb] = 0;
    ubAPP_DP_STAPreCmd[ubNb] = ulRecv[0];
}

void APP_DP_STAKeyExec(APP_EventMsg_t *ptEventMsg)
{
    ADO_WavStop();
    switch(ubAPP_DP_STAStatus)
    {
        case APP_DP_POWERON:
        case APP_DP_IDLE:
        case APP_DP_LISTEN:
            #if(APP_DP_TXPS_MODE==0)
            BB_SetRfStopTx(0);
            #endif
            ADO_WavPlay(0);
            APP_DP_SetCmd(APP_DP_CMD_SWITCH,ubKNL_GetRole(),NULL,NULL);
            osDelay(200);
            ubPAIR_ChangeStaIDGroup(ptEventMsg->ubAPP_Message[0]);
            osDelay(100);
            APP_DP_STASetStatus(APP_DP_CALL,APP_DP_TM_CALL);
            APP_DP_SetCmd(APP_DP_CMD_CALL,ubKNL_GetRole(),NULL,NULL);
        break;

        case APP_DP_MONITOR:
        case APP_DP_TALK:
        case APP_DP_CALL:
            if(ubPAIR_GetNowGroup() != ptEventMsg->ubAPP_Message[0])
            {
                ADO_WavPlay(1);
                ubAPP_DP_STAGPChange = 1;
                APP_DP_SetCmd(APP_DP_CMD_SWITCH,ubKNL_GetRole(),NULL,NULL);
                osDelay(200);
                ubPAIR_ChangeStaIDGroup(ptEventMsg->ubAPP_Message[0]);
                osDelay(100);
                APP_DP_STASetStatus(APP_DP_CALL,APP_DP_TM_CALL);
                APP_DP_SetCmd(APP_DP_CMD_CALL,ubKNL_GetRole(),NULL,NULL);
            }
            else
            {
                if(ubAPP_DP_STAStatus == APP_DP_TALK)
                    break;
                    
                if(ubAPP_DP_STAStatus == APP_DP_MONITOR)
                    ADO_WavPlay(1);
                else if(ubAPP_DP_STAStatus == APP_DP_CALL)    
                    // DinDon
					printd(DP_LOGLVL,"DinDon\n");
                APP_DP_STASetStatus(APP_DP_CALL,APP_DP_TM_CALL);
                APP_DP_SetCmd(APP_DP_CMD_CALL,ubKNL_GetRole(),NULL,NULL);
            }
        break;
    }
}
void APP_DP_STAPowerSaveExec(void)
{
#ifdef A7130
    VDO_ChangePlayState(tAPP_KNLInfo.tKNL_Role, VDO_STOP);
    KNL_EnableWORFunc();
#endif
}
void APP_DP_STASetStatus(uint8_t ubValue,uint8_t ubTargetCnt)
{
    ubAPP_DP_STAPreStatus = ubAPP_DP_STAStatus;
    ubAPP_DP_STAStatus = ubValue;

    ubAPP_DP_STAHandlerCnt=0;
    ubAPP_DP_STAHandlerTargetCnt=ubTargetCnt;    
}
uint8_t APP_DP_STAGetStatus(void)
{
    return ubAPP_DP_STAStatus;
}
#endif
#endif

