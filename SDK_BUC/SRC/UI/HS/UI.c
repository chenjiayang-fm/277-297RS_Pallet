/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		UI.c
	\brief		User Interface (for High Speed Mode)
	\author		Hanyi Chiu
	\version	0.13
	\date		2021/12/01
	\copyright	Copyright(C) 2021 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#include "UI.h"
#include "APP_HS.h"
#include "LCD.h"
#include "OSD.h"
#include "BUF.h"
#ifdef BSP_PWRCTRL_FUNC
#include "SADC.h"
#include "PDET.h"
#endif
#if (defined(BSP_SN93711_FHD_REC_RX_V4)||defined(BSP_D_SN93701_SSD2828_RX_V5) || defined(BSP_D_SN93701_TC358778_RX_V6))
#include "UI_BUCCU[HD].h"
#endif
#include "UI_BUCCU[WSVGA].h"
#include "IR.h"
#include "TouchPanel.h"
#include "cmsis_os.h"

osThreadId osUI_ThreadId;
osMessageQId UI_EventQueue;
osMessageQId UI_AIBOXQueue0;
osMessageQId UI_AIBOXQueue1;
osMessageQId UI_AIBOXQueue2;
osMessageQId UI_AIBOXQueue3;

osMessageQId UI_AI_clearBOXQueue;

osMessageQId *pAPP_MessageQH;

//uint8_t ubPwrUartFlag = FALSE; //频繁上断电延时开启串口
//uint32_t ubPwrUartCount = 5000; //

//static void GPIOCheck_Thread(void);
static void PwrCtl_Thread(void);
static void UI_Thread(void const *argument);
static void UI_EventThread(void const *argument);
static void UI_AI_clearBOXThread(void const *argument);
static void UI_AIBOXThread0(void const *argument);
static void UI_AIBOXThread1(void const *argument);
static void UI_AIBOXThread2(void const *argument);
static void UI_AIBOXThread3(void const *argument);
static void UI_TestThread(void const *argument);

//------------------------------------------------------------------------------
void UI_Init(osMessageQId *pvMsgQId)
{
	UI_StateReset();
	pAPP_MessageQH = pvMsgQId;
    osMessageQDef(UI_EventQueue, UI_Q_SIZE, UI_Event_t);
    UI_EventQueue = osMessageCreate(osMessageQ(UI_EventQueue), NULL);
	
	//draw box
	osMessageQDef(UI_AIBOXQueue0, 40, Algo_Result);
	UI_AIBOXQueue0 = osMessageCreate(osMessageQ(UI_AIBOXQueue0), NULL);
	osMessageQDef(UI_AIBOXQueue1, 40, Algo_Result);
	UI_AIBOXQueue1 = osMessageCreate(osMessageQ(UI_AIBOXQueue1), NULL);
	osMessageQDef(UI_AIBOXQueue2, 40, Algo_Result);
	UI_AIBOXQueue2 = osMessageCreate(osMessageQ(UI_AIBOXQueue2), NULL);
	osMessageQDef(UI_AIBOXQueue3, 40, Algo_Result);
	UI_AIBOXQueue3 = osMessageCreate(osMessageQ(UI_AIBOXQueue3), NULL);
	


	//clear box
	osMessageQDef(UI_AI_clearBOXQueue, 5, uint32_t);
   	UI_AI_clearBOXQueue = osMessageCreate(osMessageQ(UI_AI_clearBOXQueue), NULL);
	
	KEY_Init(&UI_EventQueue);
#ifdef BSP_PWRCTRL_FUNC
	PDET_Init();
#endif
	UI_1MSTimerInit();
	IR_init();
	TouchPanel_init();

	osThreadDef(UI_EventThread, UI_EventThread, osPriorityAboveNormal, 1, THREAD_STACK_UIEVENT_HANDLER);
	osThreadCreate(osThread(UI_EventThread), NULL);
	//draw box
	osThreadDef(UI_AIBOXThread0, UI_AIBOXThread0, osPriorityNormal, 1, 2048);
	osThreadCreate(osThread(UI_AIBOXThread0), NULL);
	osThreadDef(UI_AIBOXThread1, UI_AIBOXThread1, osPriorityNormal, 1, 2048);
	osThreadCreate(osThread(UI_AIBOXThread1), NULL);
	osThreadDef(UI_AIBOXThread2, UI_AIBOXThread2, osPriorityNormal, 1, 2048);
	osThreadCreate(osThread(UI_AIBOXThread2), NULL);
	osThreadDef(UI_AIBOXThread3, UI_AIBOXThread3, osPriorityNormal, 1, 2048);
	osThreadCreate(osThread(UI_AIBOXThread3), NULL);
	//clearbox
	osThreadDef(UI_AI_clearBOXThread, UI_AI_clearBOXThread, osPriorityNormal, 1, 2048);
	osThreadCreate(osThread(UI_AI_clearBOXThread), NULL);

	// osThreadDef(UI_TestThread, UI_TestThread, osPriorityRealtime, 1, THREAD_STACK_UI_HANDLER);
	// osThreadCreate(osThread(UI_TestThread), NULL);

	
	KNL_SetBbFrmMonitCbFunc(UI_FrameTRXFinish);
}
//------------------------------------------------------------------------------

static void UI_TestThread(void const *argument)
{
	const uint32_t tickFreq = 100;
	const uint32_t busyWaitTicks = tickFreq / 100;
	uint32_t startTick;
	uint32_t i = 0, num = 0;
	FS_FILE_SUB_HIDN_INFO_t testInfo;
	uint32_t result = 0;
//	APP_EventMsg_t tTestMessage = {0};
//	tTestMessage.ubAPP_Event 	 = APP_WAKEUPTX_EVENT;
//	tTestMessage.ubAPP_Message[0] = 2;		//! Message Length
//	tTestMessage.ubAPP_Message[1] = 0;
//	tTestMessage.ubAPP_Message[2] = 0;
//	tTestMessage.ubAPP_Message[3] = FALSE;
	FS_KNL_CRE_PROCESS_t KnlCreProc;
	char name[10];
	
	
	while(1)
	{
		startTick = osKernelSysTick();
		while ((osKernelSysTick() - startTick) < busyWaitTicks) {
			// Busy wait
			//UI_SendMessageToAPP(&tTestMessage);
			if(UI_REC_START == tUI_RecPlayAct.tRecAct)
			{
				for(num = 1; num <= 10; num++)
				{
					for(i = 0; i <= 100; i++)
					{
						if(i == 0)
						{
							memset(&KnlCreProc, 0, sizeof(KnlCreProc));
							KnlCreProc.MediaSel = FS_MEDIA_0;
							KnlCreProc.SrcNum	= 0;
							KnlCreProc.ubRecGroupFileNum = 10;
							snprintf(name, 8, "test%03d", num);
							KnlCreProc.FileName.ubLen = 7;
							memcpy(KnlCreProc.FileName.chName, name, 7);
							memcpy(KnlCreProc.FileName.chExt, "BIN", 3);
							KnlCreProc.FileAttr = FILE_ATTR_ARCHIVE;
							KnlCreProc.FilePath = FILE_PATH2;
							result = FS_CreateFile(&KnlCreProc);
							printf("FS_CreateFile num = %d test result = %d\n", num, result);
						}
						else if(i < 100)
						{
							result = FS_WriteFile(FS_MEDIA_0, 0, (uint32_t)0x151b60, 2048, NULL);
							printf("FS_WriteFile test %d result = %d\n", i, result);
							
						}
						else if(i == 100)
						{
							
							testInfo.ubEvent = 0;
							testInfo.ubSrcLocate[0] = 0;
							testInfo.ubSrcLocate[1] = 0;
							testInfo.ubSrcLocate[2] = 0;
							testInfo.ubSrcLocate[3] = 0;	
							result = FS_CloseFile(FS_MEDIA_0, 0, &testInfo);
							printf("FS_CloseFile test result = %d\n", result);
						}
					}
					osDelay(3);
				}
				osThreadSuspend(NULL);
			}
        }
		osDelay(3);
	}
}



//{
//	const uint32_t tickFreq = 100;
//	const uint32_t busyWaitTicks = 2;
//	uint32_t startTick;
//	APP_EventMsg_t tUI_PsMessage = {0};
//	while(1)
//	{
//		startTick = osKernelSysTick();
//		while ((osKernelSysTick() - startTick) < 1) {
//			tUI_PsMessage.ubAPP_Event	   = APP_SYNCTIMESTAMP_EVENT;
//			tUI_PsMessage.ubAPP_Message[0] = 1; 	//! Message Length
//			tUI_PsMessage.ubAPP_Message[1] = 0;
//			UI_SendMessageToAPP(&tUI_PsMessage);		
//
//			// Busy wait
//        }
//		osDelay(2);
//	}
//}

#define OSD_WINSHIFT_PIXEL	0
uint32_t ulUI_BufSetup(uint32_t ulBUF_StartAddr)
{
	uint32_t ulUI_BufSize = 0;
#ifdef BUC_CU
	#if (defined(BSP_SN93711_FHD_REC_RX_V4)||defined(BSP_D_SN93701_SSD2828_RX_V5) || defined(BSP_D_SN93701_TC358778_RX_V6))
	ulUI_BufSize = ulOSD_CalBufSize(HD_WIDTH, HD_HEIGHT);
	#endif
	#if (defined(BSP_D_SNCC71_GM8285C_RX_V2))
	ulUI_BufSize = ulOSD_CalBufSize((WSVGA_WIDTH - (OSD_WINSHIFT_PIXEL * 2)), WSVGA_HEIGHT);
	#elif (defined(BSP_D_SNCC71_TP2915_AHD_RX_V3) || defined(BSP_D_SNCC71_NVP6021_AHD_RX_V4))
	ulUI_BufSize = ulOSD_CalBufSize(uwOSD_GetOsdImgHsize(), uwOSD_GetOsdImgVsize());
	#endif
	OSD_SetOsdBufAddr(ulBUF_StartAddr);
	printd(DBG_Debug2Lvl, "OSD Buffer Addr 0x%X[0x%X]\n", ulBUF_StartAddr, ulUI_BufSize);
#endif
	return ulUI_BufSize;
}
//------------------------------------------------------------------------------
void UI_PlugIn(void)
{
#ifdef BUC_CU
	LCD_Init(LCD_LCD_PANEL);
	LCD_SetLcdBufAddr(ulBUF_GetBlkBufAddr(0, BUF_LCD_IP));	
    #if (defined(BSP_SN93711_FHD_REC_RX_V4)||defined(BSP_D_SN93701_SSD2828_RX_V5))
	tOSD_Init(OSD_WEIGHT_8DIV8, uwLCD_GetLcdHoSize(), uwLCD_GetLcdVoSize(), 0, 0, OSD_SCALE_1X, OSD_SCALE_1X);
	#endif
	#if (defined(BSP_D_SNCC71_GM8285C_RX_V2))
	tOSD_Init(OSD_WEIGHT_8DIV8, (uwLCD_GetLcdHoSize() - (OSD_WINSHIFT_PIXEL * 2)), uwLCD_GetLcdVoSize(), OSD_WINSHIFT_PIXEL, 0, OSD_SCALE_1X, OSD_SCALE_1X);
	#elif (defined(BSP_D_SNCC71_TP2915_AHD_RX_V3))
	{
		uint8_t ubX, ubY;
		UI_GetOsdSft(&ubX, &ubY);
		tOSD_Init(OSD_WEIGHT_8DIV8, uwOSD_GetOsdImgHsize(), uwOSD_GetOsdImgVsize(),
				  uwLCD_GetLcdHoSize() - uwOSD_GetOsdImgHsize() - ubX, ubY, OSD_SCALE_1X, OSD_SCALE_1X);
	}
	#endif
	UI_OnInitDialog();
#endif	//! End of #ifdef BUC_CU
	osThreadDef(UI_Thread, UI_Thread, osPriorityBelowNormal, 1, THREAD_STACK_UI_HANDLER);
	osUI_ThreadId = osThreadCreate(osThread(UI_Thread), NULL);

//	osThreadDef(GPIOCheck_Thread, GPIOCheck_Thread, osPriorityHigh, 1, THREAD_STACK_UI_PWRCTRL_HANDLER);
//	osUI_ThreadId = osThreadCreate(osThread(GPIOCheck_Thread), NULL);


	osThreadDef(PwrCtl_Thread, PwrCtl_Thread, osPriorityAboveNormal, 1, THREAD_STACK_UI_PWRCTRL_HANDLER);
	osUI_ThreadId = osThreadCreate(osThread(PwrCtl_Thread), NULL);
	
#ifdef BSP_PWRCTRL_FUNC
	osThreadDef(UI_PwrCtrlThread, UI_PwrCtrlThread, osPriorityNormal, 1, THREAD_STACK_UI_PWRCTRL_HANDLER);
	osThreadCreate(osThread(UI_PwrCtrlThread), NULL);
#endif
}
//------------------------------------------------------------------------------
//static void GPIOCheck_Thread(void)
//{
//	//检测断电后将串口给设置为gpio拉低，然后上电后延时5s改为串口，防止串口漏电让1126无法启动
//	static uint8_t power_on_once = 0, power_down_once = 0;
//	while(1)
//	{
//		if(power_on_once == 0)
//		{
//			if(GET_PWR_REPORT)
//			{
//				power_on_once = 1;
//				GLB->PADIO49 = 0; //UART2_TX
//				GLB->PADIO50 = 0; //UART2_RX
//				GPIO->GPIO_OE7 = 1;
//				GPIO->GPIO_OE8 = 1;
//				GPIO->GPIO_O7 = 0;
//				GPIO->GPIO_O8 = 0;
//				KNL_UsbdFwuFg = 1;
//				printf("*****UART2_TX/RX DOWN GPIO!!!!!!!\n");

//			}
//		}
//		else
//		{
//			if(!GET_PWR_REPORT)
//			{
//				power_on_once = 0;
//				ubPwrUartFlag = TRUE;
//				//5s开启串口
//				ubPwrUartCount = 5000;
//				KNL_UsbdFwuFg = 0;
//				printf("*****time reset 5000 pwr!!!!!!!\n");
//			}
//		}
//		osDelay(20);
//	}
//}

//------------------------------------------------------------------------------
static void PwrCtl_Thread(void)
{
	while(1)
	{
		UI_PwrCtrl();
		osDelay(50);
	}
}

//------------------------------------------------------------------------------
static void UI_Thread(void const *argument)
{
	static uint16_t uwUI_TaskCnt = 0;
	while(1)
	{
		UI_UpdateStatus(&uwUI_TaskCnt);
		osDelay(UI_TASK_PERIOD);
	}
}
//------------------------------------------------------------------------------
static void UI_EventThread(void const *argument)
{
	UI_Event_t tUI_Event;
	while(1)
	{
        osMessageGet(UI_EventQueue, &tUI_Event, osWaitForever);
		UI_EventHandles(&tUI_Event);
	}
}
//------------------------------------------------------------------------------
static void UI_AIBOXThread0(void const *argument)
{
	//osPoolDef(mpool, 16, Algo_Result);
	//osPoolId  mpool;
	//UI_Event_t tUI_Event;
	//mpool = osPoolCreate(osPool(mpool));
	Algo_Result curRet111;
	while(1)
	{	
		osMessageGet(UI_AIBOXQueue0,&curRet111,osWaitForever);
		UI_EventDrawBox(curRet111);
		//printf("curRet.chn is %d, cnt is %d",curRet111.chn,curRet111.cnt);
	}
}
static void UI_AIBOXThread1(void const *argument)
{
	//osPoolDef(mpool, 16, Algo_Result);
	//osPoolId  mpool;
	//UI_Event_t tUI_Event;
	//mpool = osPoolCreate(osPool(mpool));
	Algo_Result curRet111;
	while(1)
	{	
		osMessageGet(UI_AIBOXQueue1,&curRet111,osWaitForever);
		UI_EventDrawBox(curRet111);
		//printf("curRet.chn is %d, cnt is %d",curRet111.chn,curRet111.cnt);
	}
}
static void UI_AIBOXThread2(void const *argument)
{
	//osPoolDef(mpool, 16, Algo_Result);
	//osPoolId  mpool;
	//UI_Event_t tUI_Event;
	//mpool = osPoolCreate(osPool(mpool));
	Algo_Result curRet111;
	while(1)
	{	
		osMessageGet(UI_AIBOXQueue2,&curRet111,osWaitForever);
		UI_EventDrawBox(curRet111);
		//printf("curRet.chn is %d, cnt is %d",curRet111.chn,curRet111.cnt);
	}
}
static void UI_AIBOXThread3(void const *argument)
{
	//osPoolDef(mpool, 16, Algo_Result);
	//osPoolId  mpool;
	//UI_Event_t tUI_Event;
	//mpool = osPoolCreate(osPool(mpool));
	Algo_Result curRet111;
	while(1)
	{	
		osMessageGet(UI_AIBOXQueue3,&curRet111,osWaitForever);
		UI_EventDrawBox(curRet111);
		//printf("curRet.chn is %d, cnt is %d",curRet111.chn,curRet111.cnt);
	}
}

//------------------------------------------------------------------------------
static void UI_AI_clearBOXThread(void const *argument)
{
	uint32_t ClearBOX_chn;
	while(1)
	{
        osMessageGet(UI_AI_clearBOXQueue,&ClearBOX_chn, osWaitForever);		
		UI_EventClearBox(ClearBOX_chn);
	}
}

//------------------------------------------------------------------------------
#ifdef BSP_PWRCTRL_FUNC
#define ACTIVE_CD_TH			30	//Active Count-Down Threshold, 30*50ms = 1.5 Sec
void UI_ReGetPwrActionNoRec(void)
{
	printf("Re-Get Power(No Rec)\r\n");	
	SYS_KEEP_PWR;

#ifdef OP_AP
	LCD_BL_CTRL(UI_ENABLE);
#endif
}

void UI_ReGetPwrActionWithRec(void)
{
	printf("Re-Get Power(With Rec)\r\n");	
	SYS_KEEP_PWR;

#ifdef OP_AP
	LCD_BL_CTRL(UI_ENABLE);
#endif	
	UI_VideoRecordingExec(UI_REC_START);
}
//------------------------------------------------------------------------------
void UI_LostPwrAction(void)
{
	UI_RecPlayAct_t tRecPlayAct;	
	uint8_t ubMaxWaitTh = 70;	//70*50ms = 3.5 Sec
	uint8_t ubMaxWaitCnt = 0;	

	printf("Lost Power\r\n");
	

	RTC_WriteUserRam(RTC_RECORD_PWRSTS_ADDR, RTC_PWRSTS_KEEP_TAG);
	
	tRecPlayAct = tUI_GetRecPlayState();
	if(UI_REC_START == tRecPlayAct.tRecAct)
	{
		printf("Stop Rec\r\n");
		UI_VideoRecordingExec(UI_REC_STOP);
		
		while(1)
		{			
			ubMaxWaitCnt++;
			
			if(REC_UpdateChkFinish())
			{
				printf("Release Pwr\r\n");								
				SYS_RELEASE_PWR;
				break;					
			}
			else
			{
				if(ubMaxWaitCnt >= ubMaxWaitTh)
				{
					printf("Over Max.Wait Time\r\n");
					SYS_RELEASE_PWR;
					break;
				}
			}
			osDelay(50);						
		}				
	}
	else	
	{
		printf("Release Pwr\r\n");		
		SYS_RELEASE_PWR;
	}	
}
//------------------------------------------------------------------------------
static void UI_PwrCtrlThread(void const *argument)
{
	UI_RecPlayAct_t tRecPlayAct;	
	uint8_t ubActiveCnt = 0;
	uint8_t ubPreRecFlg = 0;	

	while(1)
	{		
		if((ubActiveCnt == 0) && (GET_PWR_REPORT <= LOW_PWR_TH))		
		{			
			ubActiveCnt = ACTIVE_CD_TH;				
			
			tRecPlayAct = tUI_GetRecPlayState();
			if(UI_REC_START == tRecPlayAct.tRecAct)
				ubPreRecFlg = 1;
			else
				ubPreRecFlg = 0;	
		
			UI_LostPwrAction();			
		}	
		else if(ubActiveCnt)
		{			
			ubActiveCnt--;
			if((ubActiveCnt == 0) && (GET_PWR_REPORT > LOW_PWR_TH))									//Count down is finish
			{				
				if(ubPreRecFlg)
				{					
					UI_ReGetPwrActionWithRec();					
				}
				else
				{					
					UI_ReGetPwrActionNoRec();					
				}				
				ubPreRecFlg = 0;	//Reset								
			}					
			else if((ubActiveCnt >= 1) && (GET_PWR_REPORT > LOW_PWR_TH) && REC_UpdateChkFinish())	//Count down is not finish. Need to Check "Close file is done"
			{				
				ubActiveCnt = 0;
				
				if(ubPreRecFlg)
				{					
					UI_ReGetPwrActionWithRec();				
				}
				else
				{					
					UI_ReGetPwrActionNoRec();					
				}
				
				ubPreRecFlg = 0;	//Reset									
			}			
		}
		osDelay(UI_PWRCTRL_TASK_PERIOD);
	}
}
#endif
//------------------------------------------------------------------------------
osMessageQId *pUI_GetEventQueueHandle(void)
{
	return &UI_EventQueue;
}
osMessageQId *pUI_GetAIBOXQueueHandle(uint32_t chn)
{
	switch(chn)
	{
		case 0: 
			//printf("put chn 000\n;");
			return &UI_AIBOXQueue0;
			break;
		case 1: 
			//printf("put chn 111\n;");
			return &UI_AIBOXQueue1;
			break;
		case 2:
			//printf("put chn 222\n;");
			return &UI_AIBOXQueue2;
			break;
		case 3: 
			//printf("put chn 333\n;");
			return &UI_AIBOXQueue3;
			break;
		default:
			return 0;
			break;
	}
	printf("error Queue choose!!!!");
}
osMessageQId *pUI_GetAIBOX_clearQueueHandle(void)
{
	return &UI_AI_clearBOXQueue;
}

//------------------------------------------------------------------------------
void UI_SendMessageToAPP(void *pvMessage)
{
	if(osMessagePut(*pAPP_MessageQH, pvMessage, 0) != osOK)
	{
		printd(DBG_ErrorLvl, "APP Q Full\n");
#if KNL_DEBUG_APP_Q_FULL_EN
		KNL_AutoEnterFileListStop();
#endif
#if KNL_DEBUG_INFO_EN
		KNL_PrintDbgInfo();
#endif
	}
}
//------------------------------------------------------------------------------
void UI_StopUpdateThread(void)
{
	if((NULL != osUI_ThreadId) && (osOK != osThreadIsSuspended(osUI_ThreadId)))
		osThreadSuspend(osUI_ThreadId);
}
//------------------------------------------------------------------------------
void UI_StartUpdateThread(void)
{
	if((NULL != osUI_ThreadId) && (osOK == osThreadIsSuspended(osUI_ThreadId)))
		osThreadResume(osUI_ThreadId);
}
//------------------------------------------------------------------------------
void UI_FrameTRXFinish(uint8_t ubFrmRpt)
{
	switch(ubFrmRpt)
	{
	#ifdef BUC_CU
		case 0x0:
		case 0x1:
		case 0x2:
		case 0x3:
	#endif
	#ifdef BUC_CAM
		case 0xF:
	#endif
	#if (!defined(BSP_D_SN93714_TX_V1) || !APP_SD_FUNC_ENABLE)
			SIGNAL_LED_IO(!GET_SIGNAL_LED_IO);
	#endif
			break;
		default:
			SIGNAL_LED_IO(0);
			break;
	}
}
//------------------------------------------------------------------------------
#if (defined(BUC_CU)&&(APP_DOORPHONE_ENABLE==1))
void UI_DP_Init(uint8_t *st)
{
    UI_DP_SycAPPStatus(st);
}
#endif

