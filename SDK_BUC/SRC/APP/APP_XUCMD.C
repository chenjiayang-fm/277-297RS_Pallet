/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		APP_XUCMD.c
	\brief		XU Command Application function
	\author		Hanyi Chiu
	\version	1.1
	\date		2020/05/04
	\copyright	Copyright(C) 2020 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include "APP_XUCMD.h"
#include "APP_HS.h"
#include "APP_CFG.h"
#include "UI.h"
#include "PAIR.h"
#include "BUF.h"
#if OP_AP
#if (APP_DUAL_HOST_ENABLE == 1)
#include "USBH_API.h"
#endif
#endif

osSemaphoreId tAPPXU_AccSem;
osSemaphoreId tAPPXU_AccExtraDataSem;
uint8_t ubAPPXU_ExtraWrDataIdx = APPXU_EXT_BUF_NUM-1;
uint8_t ubAPPXU_ExtraRdDataIdx = APPXU_EXT_BUF_NUM-1;
uint8_t ubAPPXU_ExtraDataFlg[APPXU_EXT_BUF_NUM];
uint8_t ubAPPXU_ExtraData[APPXU_EXT_BUF_NUM][64];
pvAPPXU_GetDataFromUsbCb pAPPXU_GetDataFromUsbHostCb 	= NULL;
pvAPPXU_GetDataFromUsbCb pAPPXU_GetDataFromUsbDevice1Cb = NULL;
pvAPPXU_GetDataFromUsbCb pAPPXU_GetDataFromUsbDevice2Cb = NULL;

#ifdef OP_AP
const static UI_DisplayLocation_t tAPPXU_DispLocMap[] =
{
	DISP_UPPER_LEFT, DISP_UPPER_RIGHT, DISP_LOWER_LEFT, DISP_LOWER_RIGHT
};
static UI_CamNum_t tAPPXU_PairCamNum = NO_CAM;
#endif

void APPXU_Init(void)
{		
#if OP_STA
#if APP_UVC_CAM_ENABLE	
	uint8_t i;	
	for(i=0;i<KNL_EXT_BUF_NUM;i++)
		ubAPPXU_ExtraDataFlg[i] = 0;	
	osSemaphoreDef(tAPPXU_AccExtraDataSem);
	tAPPXU_AccExtraDataSem	= osSemaphoreCreate(osSemaphore(tAPPXU_AccExtraDataSem), 1);	
#endif
#endif	
	
#if OP_AP	
	osSemaphoreDef(tAPPXU_AccSem);
	tAPPXU_AccSem	= osSemaphoreCreate(osSemaphore(tAPPXU_AccSem), 1);
	osThreadDef(APPXUUsbCmdMonitAtHostThread, APPXU_UsbCmdMonitAtHostThread, THREAD_PRIO_USB_CMD_MONIT, 1, THREAD_STACK_USB_CMD_MONIT);	
	osThreadCreate(osThread(APPXUUsbCmdMonitAtHostThread), NULL);
#endif
}

void APPXU_GetDataFromUsbCbFunc(APPXU_USB_TARGET tTarget,pvAPPXU_GetDataFromUsbCb pFunc)
{
	if(tTarget == USB_HOST)
		pAPPXU_GetDataFromUsbHostCb = pFunc;
	else if(tTarget == USB_UVC_DEVICE1)
		pAPPXU_GetDataFromUsbDevice1Cb = pFunc;
	else if(tTarget == USB_UVC_DEVICE2)
		pAPPXU_GetDataFromUsbDevice2Cb = pFunc;
}

//void APPXU_GetDataFromUsbHost(uint8_t *pBuf,uint8_t ubLen)
//{
//	uint8_t i;	
//	
//	printf("RH:%d\r\n",ubLen);
//	for(i=0;i<ubLen;i++)
//		printf("D[%d]:0x%x\r\n",i,*pBuf++);
//}

//void APPXU_GetDataFromUsbDevice1(uint8_t *pBuf,uint8_t ubLen)
//{
//	uint8_t i;	
//	
//	printf("R1:%d\r\n",ubLen);
//	for(i=0;i<ubLen;i++)
//		printf("D[%d]:0x%x\r\n",i,*pBuf++);
//}

//void APPXU_GetDataFromUsbDevice2(uint8_t *pBuf,uint8_t ubLen)
//{
//	uint8_t i;	
//	
//	printf("R2:%d\r\n",ubLen);
//	for(i=0;i<ubLen;i++)
//		printf("D[%d]:0x%x\r\n",i,*pBuf++);
//}

void APPXU_RecvCmd(uint8_t ubMode, uint32_t *pCmd)
{	
#ifdef OP_STA
	uint8_t ubTemp[2];
	uint32_t ulExtraLen;
	uint32_t i;
	uint8_t *pBuf = (uint8_t *)pCmd;			
	
	uint16_t *pXuCmd, *pCmdLen;
	uint8_t *pCmdBuf = (uint8_t *)pCmd;		
	pXuCmd  = (uint16_t *)&pCmdBuf[0];
	pCmdLen = (uint16_t *)&pCmdBuf[2];
	//printd(DBG_CriticalLvl, "	>>CMD:%X,Len:%d\n", pXuCmd[0],*pCmdLen);		
	
	switch(pXuCmd[0])
	{
		case SNX_MSG_TYPE_SET_HOST_TO_DEVICE:		//Host -> Device
				if(pAPPXU_GetDataFromUsbHostCb != NULL)
				{
					pBuf = (uint8_t *)pCmd;											
					pAPPXU_GetDataFromUsbHostCb(pBuf+4,*pCmdLen);
				}
			break;	

		case XU_CMD_READ:					

			
			if(osSemaphoreWait(tAPPXU_AccExtraDataSem, 0) == osOK)
			{			
				ubAPPXU_ExtraRdDataIdx++;
				if(ubAPPXU_ExtraRdDataIdx >= APPXU_EXT_BUF_NUM)
					ubAPPXU_ExtraRdDataIdx = 0;
				
				if(ubAPPXU_ExtraDataFlg[ubAPPXU_ExtraRdDataIdx] == 1)
				{		
					ubAPPXU_ExtraDataFlg[ubAPPXU_ExtraRdDataIdx] = 0;
					
					//Get Length
					ubTemp[0] = ubAPPXU_ExtraData[ubAPPXU_ExtraRdDataIdx][1];	//Len[0]
					ubTemp[1] = ubAPPXU_ExtraData[ubAPPXU_ExtraRdDataIdx][2];	//Len[1]
					ulExtraLen = ((uint32_t)ubTemp[0]) + (((uint32_t)ubTemp[1])<<8);
					
					//Add Extra Data
					//Cmd & Length
					pCmdBuf[0] = (uint8_t)SNX_MSG_TYPE_GET_ACTIVE;
					pCmdBuf[1] = 0;
					pCmdBuf[2] = ubTemp[0] & 0x000000FFL;
					pCmdBuf[3] = ubTemp[1];
					
					//Data					
					for(i=0;i<ulExtraLen;i++)					
						pCmdBuf[4+i] = ubAPPXU_ExtraData[ubAPPXU_ExtraRdDataIdx][4+i];					
				}
				else
				{
					//Restore
					if(ubAPPXU_ExtraRdDataIdx)
						ubAPPXU_ExtraRdDataIdx--;
					else
						ubAPPXU_ExtraRdDataIdx = APPXU_EXT_BUF_NUM-1;							
					
					osSemaphoreRelease(tAPPXU_AccExtraDataSem);					
				}		
				
				osSemaphoreRelease(tAPPXU_AccExtraDataSem);					
			}
			else
			{
				pCmdBuf[0] = 0;	//Cmd[0]
				pCmdBuf[1] = 0;	//Cmd[1]
				pCmdBuf[2] = 0;	//Len[0]
				pCmdBuf[3] = 0;	//Len[1]
			}				
			break;
	}
#endif
	
#ifdef OP_AP
	uint16_t *pXuCmd, *pCmdLen;
	uint8_t *pCmdBuf = (uint8_t *)pCmd;
	UI_CamNum_t tCamNum;
	APP_EventMsg_t tAPPXU_Message = {0};

	pXuCmd  = (uint16_t *)&pCmdBuf[0];
	pCmdLen = (uint16_t *)&pCmdBuf[2];
	printd(DBG_CriticalLvl, "	>>CMD: %X\n", pXuCmd[0]);
	switch(pXuCmd[0])
	{
		case SNX_MSG_TYPE_SYNCTIME:
			memset(pCmdBuf, 0, (pCmdLen[0] + 4));
			break;
		case SNX_MSG_TYPE_CAM_SET_PAIRING:
		{
			if(NO_CAM == tAPPXU_PairCamNum)
			{
				tAPPXU_PairCamNum = (UI_CamNum_t)(pCmdBuf[4] - 1);
				if(tAPPXU_PairCamNum < DISPLAY_4T1R)
				{
					tAPPXU_Message.ubAPP_Event 	 = APP_PAIRING_START_EVENT;
					tAPPXU_Message.ubAPP_Message[0] = 3;
					tAPPXU_Message.ubAPP_Message[1] = tAPPXU_PairCamNum;
					tAPPXU_Message.ubAPP_Message[2] = (DISPLAY_1T1R == DISPLAY_MODE)?DISP_UPPER_LEFT:tAPPXU_DispLocMap[tAPPXU_PairCamNum];
					tAPPXU_Message.ubAPP_Message[3] = TRUE;
					UI_SendMessageToAPP(&tAPPXU_Message);
				}
				else
					tAPPXU_PairCamNum = NO_CAM;
			}
			memset(pCmdBuf, 0, (pCmdLen[0] + 4));
			break;
		}
		case SNX_MSG_TYPE_CAM_GET_PAIRING_STATUS:
		{
			APP_PairSts_t tAppPairSts;
			PAIR_STATE tPairSte;

			tCamNum    	= (UI_CamNum_t)(pCmdBuf[4] - 1);
			tPairSte  	= tPAIR_GetPairState();
			tAppPairSts = ((NO_CAM == tAPPXU_PairCamNum) || (tCamNum != tAPPXU_PairCamNum) || (PAIR_TIMEOUT == tPairSte))?APP_PAIR_FAIL:
						  (PAIR_SUCCESS == tPairSte)?APP_PAIR_OK:APP_PAIRING;
			if((APP_PAIR_FAIL == tAppPairSts) || (APP_PAIR_OK == tAppPairSts))
			{
				PAIR_SetPairState(PAIR_NULL);
				tAPPXU_PairCamNum = NO_CAM;
				printd(DBG_Debug3Lvl, "	>>Pair %s\n", (APP_PAIR_OK == tAppPairSts)?"OK":"FAIL");
			}
			pCmdLen[0] = 1;
			pCmdBuf[5] = tAppPairSts - 1;
			break;
		}
		case SNX_MSG_TYPE_GET_CAM_PAIRED_STATUS:
		{
			PAIR_TAG tPair_Tag;
			uint32_t *pTxId;

			pCmdLen[0] = 6;
			for(tCamNum = CAM1; tCamNum < DISPLAY_4T1R; tCamNum++)
			{
				tPair_Tag = APP_GetSTANumMappingTable(tCamNum)->tPAIR_StaNum;
				pTxId = (uint32_t *)PAIR_GetId(tPair_Tag);
				pCmdBuf[4 + tCamNum] = ((PAIR_INVALID_ID == *pTxId) || (0xFFFFFFFF == *pTxId))?FALSE:TRUE;
			}                              
			break;
		}
		case SNX_MSG_TYPE_CAM_UNPAIR:
		{
			APP_StatusReport_t tAPP_StsReport;

			tCamNum = (UI_CamNum_t)(pCmdBuf[4] - 1);
			tAPP_StsReport.tAPP_ReportType = APP_PAIRUDCAM_PRT;
			tAPP_StsReport.tAPP_State 	   = APP_IDLE_STATE;
			tAPP_StsReport.ubAPP_Report[0] = tCamNum;
			tAPP_StsReport.ubAPP_Report[1] = TRUE;
			UI_UpdateAppStatus(&tAPP_StsReport);
			tAPP_StsReport.tAPP_ReportType = APP_RPT_NONE;
			memset(pCmdBuf, 0, (pCmdLen[0] + 4));
			break;
		}
		case SNX_MSG_TYPE_REMOTE_WAKEUP:
			tCamNum = (UI_CamNum_t)(pCmdBuf[4] - 1);
			printd(DBG_CriticalLvl, "	>>[%d]WakeUp: %d\n", tCamNum, pCmdBuf[5]);
			memset(pCmdBuf, 0, (pCmdLen[0] + 4));
			break;
		case SNX_MSG_TYPE_RESEND_IFRAME:
			tCamNum = (UI_CamNum_t)(pCmdBuf[4] - 1);
			KNL_ResendIframeFunc(APP_GetSTANumMappingTable(tCamNum)->tKNL_StaNum);
			break;
		case SNX_MSG_TYPE_PUSH_TALK:
			KNL_ActiveUsbdAdoEncFlag(pCmdBuf[5]);
			break;
		case SNX_MSG_TYPE_SET_VIEW_TYPE:
			if(pCmdBuf[4])
				KNL_ModifyUsbdViewType((pCmdBuf[4] - 1));
			break;
		case SNX_MSG_TYPE_GET_REMOT_WAKEUP_PREVIEW_INTERVAL:
			pCmdLen[0] = 2;
			pCmdBuf[5] = 0;
			break;
		case SNX_MSG_TYPE_GET_CAM_ONLINE_STATUS:
			pCmdLen[0] = 2;
			pCmdBuf[5] = 1;
			break;
		case SNX_MSG_TYPE_GET_TX_BATTERY_LEVEL:
			pCmdLen[0] = 2;
			pCmdBuf[5] = 5;
            break;
	}
#endif
}//------------------------------------------------------------------------------

uint8_t ubAPPXU_GetDataFromUvcDev(UVC_DEVICE tDevice,uint8_t *pBuf)
{
#if OP_AP
#if (APP_DUAL_HOST_ENABLE == 1)
	uint16_t uwCmd;
	uint8_t ubActiveLen;
	uint8_t *pTempBuf,*pTemp2Buf;
	uint32_t ulReadBufAddr;	
	
	osSemaphoreWait(tAPPXU_AccSem,osWaitForever);
	
	ulReadBufAddr = ulBUF_GetBlkBufAddr(0,BUF_USB_TRX) + 512;	
	pTempBuf = (uint8_t *)ulReadBufAddr;
	pTemp2Buf = (uint8_t *)ulReadBufAddr;
	*pTemp2Buf++ = XU_CMD_READ;			//Cmd[0]
	*pTemp2Buf++ = 0;					//Cmd[1]

	if(tDevice == UVC_DEVICE1)
	{		
		if(1 == uvc_XU_read(USBH_UVC1_DEVICE_ID, 32 , 64, pTempBuf))	//1 -> Fail
		{
			osSemaphoreRelease(tAPPXU_AccSem);
			return 1;
		}
	}
	else if(tDevice == UVC_DEVICE2)
	{
		if(1 == uvc_XU_read(USBH_UVC2_DEVICE_ID, 32 , 64, pTempBuf))
		{
			osSemaphoreRelease(tAPPXU_AccSem);
			return 1;
		}
	}
	
	//Parsing Received Data
	//===========================================================
	uwCmd = 0;
	uwCmd = uwCmd + *((uint8_t *)(pTempBuf+0)); 
	
	if(uwCmd == SNX_MSG_TYPE_GET_ACTIVE)	//Active
	{		
		ubActiveLen = 0;
		ubActiveLen = ubActiveLen + *((uint8_t *)(pTempBuf+2));
		
		if(tDevice == UVC_DEVICE1)
		{
			if(pAPPXU_GetDataFromUsbDevice1Cb != NULL)
			{
				pTempBuf = (uint8_t *)ulReadBufAddr;				
				pAPPXU_GetDataFromUsbDevice1Cb(pTempBuf+4,ubActiveLen);
			}
		}
		else if(tDevice == UVC_DEVICE2)
		{
			if(pAPPXU_GetDataFromUsbDevice2Cb != NULL)
			{
				pTempBuf = (uint8_t *)ulReadBufAddr;				
				pAPPXU_GetDataFromUsbDevice2Cb(pTempBuf+4,ubActiveLen);
			}
		}
	}
	osSemaphoreRelease(tAPPXU_AccSem);
#endif	
#endif	
	return 0;	//1->Fail, 0->Success
}

#if OP_AP
static void APPXU_UsbCmdMonitAtHostThread(void const *argument)
{
#if (APP_DUAL_HOST_ENABLE == 1)
	uint8_t i;
	uint8_t ubBuf[KNL_XU_MAX_DATA_LEN];
	uint8_t ubIsDevice1 = 1;
	
	uint16_t uwDelayMs = XU_CMD_ACC_TIME;
	
	while(1)
	{
		for(i=0;i<KNL_XU_MAX_DATA_LEN;i++)
			ubBuf[i] = 0;
		
		if(ubIsDevice1)
		{
			ubIsDevice1 = 0;
			
			if(TRUE == USBH_ChkDeviceRdy(USBH_UVC1_DEVICE_ID))	
			{				
				if(1 == ubAPPXU_GetDataFromUvcDev(UVC_DEVICE1,&ubBuf[0]))//Fail
				{
					uwDelayMs = 500;
				}
				else
				{
					uwDelayMs = XU_CMD_ACC_TIME;
				}				
			}
			else
			{
				uwDelayMs = XU_CMD_ACC_TIME;
			}
		}
		else
		{
			ubIsDevice1 = 1;
			
			if(TRUE == USBH_ChkDeviceRdy(USBH_UVC2_DEVICE_ID))
			{				
				if(1 == ubAPPXU_GetDataFromUvcDev(UVC_DEVICE2,&ubBuf[0]))//Fail
				{
					uwDelayMs = 500;
				}
				else
				{
					uwDelayMs = XU_CMD_ACC_TIME;
				}
			}
			else
			{
				uwDelayMs = XU_CMD_ACC_TIME;
			}
		}		
		osDelay(uwDelayMs);		
	}
#endif
}

#endif

uint8_t ubAPPXU_SendData(APPXU_USB_TARGET tTarget,uint8_t *pBuf,uint8_t ubLen)
{
#if OP_AP
	uint8_t ubTemp;
#if (APP_DUAL_HOST_ENABLE == 1)	
	uint8_t i;
	uint8_t ubBuf[KNL_XU_MAX_DATA_LEN];		
	uint8_t ubSendDataLen = 4;	//cmd(2)+len(2)	
	uint32_t ulUnCacheBufAddr;		
	
	osSemaphoreWait(tAPPXU_AccSem,osWaitForever);
	
	ulUnCacheBufAddr = ulBUF_GetBlkBufAddr(0,BUF_USB_TRX);
		
	if(ubLen > KNL_XU_MAX_EXT_DATA_LEN)	
	{
		osSemaphoreRelease(tAPPXU_AccSem);		
		return 0;	//0->Fail, 1->Success
	}

	//OP Code
	ubBuf[0] = SNX_MSG_TYPE_SET_HOST_TO_DEVICE & 0x00FF;
	ubBuf[1] = (SNX_MSG_TYPE_SET_HOST_TO_DEVICE & 0xFF00) >> 8;
	
	//Length of Data
	ubBuf[2] = ubLen;
	ubBuf[3] = 0;
	
	//Byte[4]~...
	for(i=0;i<ubLen;i++)
		ubBuf[4+i] = *pBuf++;	
	
	ubSendDataLen += ubLen;	
	
	for(i=0;i<ubSendDataLen;i++)
		*((uint8_t *)(ulUnCacheBufAddr+i)) = ubBuf[i];			
	
	ubTemp = uvc_XU_write(tTarget,ubSendDataLen, (uint8_t *)ulUnCacheBufAddr);
	osSemaphoreRelease(tAPPXU_AccSem);
#endif
	return  (!ubTemp);
#else
	osSemaphoreWait(tAPPXU_AccExtraDataSem, osWaitForever);	
	
	if(ubLen > APPXU_XU_MAX_EXT_DATA_LEN)
	{			
		osSemaphoreRelease(tAPPXU_AccExtraDataSem);
		return 0;	
	}	
	ubAPPXU_ExtraWrDataIdx++;
	if(ubAPPXU_ExtraWrDataIdx >= APPXU_EXT_BUF_NUM)
		ubAPPXU_ExtraWrDataIdx = 0;
	
	if(ubAPPXU_ExtraDataFlg[ubAPPXU_ExtraWrDataIdx] == 0)
	{		
		ubAPPXU_ExtraDataFlg[ubAPPXU_ExtraWrDataIdx] = 1;
		
		ubAPPXU_ExtraData[ubAPPXU_ExtraWrDataIdx][0] = 0;					//Source, Don't care
		ubAPPXU_ExtraData[ubAPPXU_ExtraWrDataIdx][1] = ubLen & 0x00FFL;		//Len[0]
		ubAPPXU_ExtraData[ubAPPXU_ExtraWrDataIdx][2] = (ubLen & 0xFF00L)>>8;//Len[1]
		ubAPPXU_ExtraData[ubAPPXU_ExtraWrDataIdx][3] = 0;					//Reserved
		memcpy((uint8_t*)&ubAPPXU_ExtraData[ubAPPXU_ExtraWrDataIdx][4],pBuf,ubLen);
	}
	else
	{		
		//Restore
		if(ubAPPXU_ExtraWrDataIdx)
			ubAPPXU_ExtraWrDataIdx--;
		else
			ubAPPXU_ExtraWrDataIdx = APPXU_EXT_BUF_NUM-1;
		
		osSemaphoreRelease(tAPPXU_AccExtraDataSem);
		return 0;		
	}			
	osSemaphoreRelease(tAPPXU_AccExtraDataSem);	
	return 1;	
#endif
}

