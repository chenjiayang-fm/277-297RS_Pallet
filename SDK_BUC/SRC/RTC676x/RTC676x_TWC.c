/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		TWCR.c
	\brief		Two way Command funcations Interface for RTC676x
	\author		Justin
	\version	0.91
	\date		2019/05/05
	\copyright	Copyright(C) 2019 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------

#ifdef RTC676x

#include <stdio.h>
#include <MATH.H>
#include <STDARG.H>
#include <ABSACC.H>
#include <STRING.H>
#include <CTYPE.H>
#include <STDLIB.H>
#include "RTC676x_TWC.h"
#include "APP_CFG.h"
#include "KNL.h"
#include "CHECKSUM_API.h"
#include "BUF.h"

#include "RTC676x_SPI.h"
#include "RTC676x_CTRL.h"
#include "rwerror.h"
#include "rwrf.h"

uint8_t ubDataBufRollingIdx = TWCR_MAX_FUNCTION_OBJ-1;
uint8_t ubTWCR_DataBuf[TWCR_MAX_FUNCTION_OBJ][TWCR_DATA_MAX_SIZE];
pvRecvDataFunc 	pvTwcrDataFunc[TWCR_MAX_FUNCTION_OBJ];
pvRptTxStsFunc 	pvTwcrStatusFunc[TWCR_MAX_FUNCTION_OBJ];

osMessageQId tTWCR_Message;
osSemaphoreId tTWCR_RxAccessSem;
osSemaphoreId tTWCR_TxAccessSem;

#define TWCR_MAJORVER	0
#define TWCR_MINORVER	91

//------------------------------------------------------------------------------
void TWCR_Init(void)
{
	uint8_t i,j;

	for(i=0;i<TWCR_MAX_FUNCTION_OBJ;i++)
	{
		pvTwcrDataFunc[i]	= NULL;
		pvTwcrStatusFunc[i]	=  NULL;
		for(j=0;j<TWCR_DATA_MAX_SIZE;j++)
		{
			ubTWCR_DataBuf[i][j] = 0;
		}
	}
	osMessageQDef(TWCR_MessageQue, TWCR_QUEUE_SIZE, sizeof(TWCR_Que_t));
    tTWCR_Message = osMessageCreate(osMessageQ(TWCR_MessageQue), NULL);
	
	osSemaphoreDef(tTWCR_RxAccessSem);
	tTWCR_RxAccessSem	= osSemaphoreCreate(osSemaphore(tTWCR_RxAccessSem), 1);
	
	osSemaphoreDef(tTWCR_TxAccessSem);
	tTWCR_TxAccessSem	= osSemaphoreCreate(osSemaphore(tTWCR_TxAccessSem), 1);
}
//------------------------------------------------------------------------------
static void TWCR_Thread(void const *argument)
{
	TWCR_Que_t tTWCR_QueInfo;	

	while(1)
	{
		osMessageGet(tTWCR_Message, &tTWCR_QueInfo, osWaitForever);
		if(pvTwcrDataFunc[tTWCR_QueInfo.ubFuncIdx] != NULL)
		{
			pvTwcrDataFunc[tTWCR_QueInfo.ubFuncIdx]((TWC_TAG)tTWCR_QueInfo.ubSN,&ubTWCR_DataBuf[tTWCR_QueInfo.ubDataIdx][0]);
		}
	}
}
//------------------------------------------------------------------------------
void TWCR_RcvData(uint8_t ubSN, TWC_OPC Opc, uint8_t *pBuf)
{
	uint8_t i;
	TWCR_Que_t tInfo;		

	osSemaphoreWait(tTWCR_RxAccessSem, osWaitForever);
	
	tInfo.ubFuncIdx = Opc;
	tInfo.ubSN = ubSN;

	//Update Rolling Index
	ubDataBufRollingIdx++;
	if(ubDataBufRollingIdx > (TWCR_MAX_FUNCTION_OBJ-1))
	{
		ubDataBufRollingIdx = 0;
	}
	tInfo.ubDataIdx = ubDataBufRollingIdx;

	//Copy Data to TwcIf's Data Buffer
	for(i=0;i<TWCR_DATA_MAX_SIZE;i++)
		ubTWCR_DataBuf[ubDataBufRollingIdx][i] = *pBuf++;	

	osMessagePut(tTWCR_Message, &tInfo, 0);
	
	osSemaphoreRelease(tTWCR_RxAccessSem);
}
//------------------------------------------------------------------------------
void TWCR_Start(void)
{
    osThreadDef(TWCR_Thread, TWCR_Thread, THREAD_PRIO_TWC_HANDLER, 1, THREAD_STACK_TWC_HANDLER);
    osThreadCreate(osThread(TWCR_Thread), NULL);
}
//------------------------------------------------------------------------------
TWC_STATUS tTWCR_RegTransCbFunc(TWC_OPC Opc, pvRptTxStsFunc RptSts_cb, pvRecvDataFunc RptData_cb)
{
	if((Opc > TWC_USE63) || (Opc == TWC_RESEV))
	{
		return TWC_FAIL;
	}
	pvTwcrStatusFunc[Opc] 	= RptSts_cb;
	pvTwcrDataFunc[Opc] 	= RptData_cb;

	return TWC_SUCCESS;
}
//------------------------------------------------------------------------------
TWC_STATUS tTWCR_Send(TWC_TAG Tag, TWC_OPC Opc, uint8_t *Data, uint8_t ubLen, uint8_t ubRetry)
{
	uint8_t ubTimeOutCnt = 0;
	//uint8_t ubTimeOutMaxTh = 5;
	uint8_t ubTimeOutMaxTh = 2;
	
	int result;	
	int free_buf_count = 0;
    size_t free_buf_count_len = sizeof(free_buf_count);

	uint8_t ubWaitDlyCnt = 0;
	//uint8_t ubWaitMaxDlyTh = 50;
	uint8_t ubWaitMaxDlyTh = 2;
	uint8_t ubLinkStatus = 0;
	int Id;
	uint64_t udlAck;
	//uint8_t ubPkt[MAX_CMD_PACKET_LEN];	//(36) = Type(1)+OPC(1)+Data(32)+CheckSum(2)
	uint8_t ubPkt[MAX_CMD_PACKET_LEN];	//(38) = Mode(1)+Rsv(1)+Type(1)+OPC(1)+Data(32)+CheckSum(2)
	
	uint8_t i;
	uint8_t ubPktLen = 0;
	TWC_STATUS tStatus;
	uint16_t uwCheckSum = 0;		
	uint32_t ulTemp;
	
	osSemaphoreWait(tTWCR_TxAccessSem, osWaitForever);
	
	result = result;
	
#ifdef RTC676x
	if(ubKNL_GetStopTrxFlg())
	{		
		tStatus = TWC_FAIL;
		if(pvTwcrStatusFunc[Opc] != NULL)
		{
			pvTwcrStatusFunc[Opc](Tag,tStatus);
		}
		osSemaphoreRelease(tTWCR_TxAccessSem);
		return tStatus;
	}
#endif

#if OP_AP	//AP->STA TWC
	Tag = (TWC_TAG)ubKNL_GetStaInfo(KNL_TWCTAG, Tag);
	if(Tag > TWC_STA4)
	{
		osSemaphoreRelease(tTWCR_TxAccessSem);
		return TWC_FAIL;
	}
	ubLinkStatus = rf_get_link_status(rf_get_remote_id(0+Tag));
#else		//STA->AP TWC
	ubLinkStatus = rf_get_link_status(rf_get_remote_id(0));
#endif
	if(ubLinkStatus)
	{
		//Initial Buffer
		memset(ubPkt,0,MAX_CMD_PACKET_LEN);
		
		ubPkt[PKT_INFO_OFS_MODE] 	= KNL_GetCurRwBbRateMode();	//Modulation Mode
		ubPkt[PKT_INFO_OFS_RSV]		= 0;
		
		//(Part1)Type
		ubPkt[PKT_INFO_OFS_TYPE] 	= PKT_CMD;	//Packet Type
		//(Part2)OPC
		ubPkt[PKT_INFO_OFS_TYPE+1] 	= Opc;		//Opc for TWC
		//(Part3)Data
		for(i=0;i<ubLen;i++)					//Data for TWC
		{
			ubPkt[PKT_INFO_OFS_TYPE+2+i] = *Data++;
		}		
		//(Part4)CheckSum
		ulTemp = ulBUF_GetBlkBufAddr(0,BUF_RW_FRAME)+BUF_SZ_MAX_VDO+BUF_SZ_MAX_ADO;		
		for(i=0;i<MAX_CMD_PACKET_LEN;i++)
		{
			*((uint8_t *)(ulTemp+i)) = 0;
		}
		for(i=0;i<MAX_CMD_PACKET_LEN-2;i++)
		{
			*((uint8_t *)(ulTemp+i)) = ubPkt[i];
		}
		
		uwCheckSum = (uint16_t)uwCHECKSUM_Calc(ulTemp,MAX_CMD_PACKET_LEN-2);
		
		ubPkt[MAX_CMD_PACKET_LEN-2] = (uwCheckSum & 0x00FF);
		ubPkt[MAX_CMD_PACKET_LEN-1] = (uwCheckSum & 0xFF00) >> 8;		
		ubPktLen = MAX_CMD_PACKET_LEN;

#if OP_AP	//AP->STA TWC 
		Id 	= rf_get_remote_id(0+Tag);
		
#else		//STA->AP TWC
		Id 	= rf_get_remote_id(0);			
#endif	

		//ubTwcOkFlg = 0;
		tStatus = TWC_FAIL;
		for(i=0;i<ubRetry;i++)
		{
			#if OP_AP	//AP->STA TWC
			ubLinkStatus = rf_get_link_status(rf_get_remote_id(0+Tag));
			#else		//STA->AP TWC
			ubLinkStatus = rf_get_link_status(rf_get_remote_id(0));
			#endif
			if(ubLinkStatus)
			{
				udlAck = 0;				

				//Free Packet Buffer Check
				ubWaitDlyCnt = 0;
				
				rf_getopt(RF_DRV_OPT_TX_FREE_BUF_CNT, &free_buf_count, &free_buf_count_len);	
				
				while(free_buf_count < 3)
				{					
				#ifdef RTC676x		
					if(ubKNL_GetStopTrxFlg())
					{						
						tStatus = TWC_FAIL;
						if(pvTwcrStatusFunc[Opc] != NULL)
						{
							pvTwcrStatusFunc[Opc](Tag,tStatus);
						}
						osSemaphoreRelease(tTWCR_TxAccessSem);
						return tStatus;
					}
				#endif
	
					osDelay(20);						
					
					ubWaitDlyCnt++;					
					if(ubWaitDlyCnt > ubWaitMaxDlyTh)
					{
						printf("CT\r\n");
						break;
					}
					rf_getopt(RF_DRV_OPT_TX_FREE_BUF_CNT, &free_buf_count, &free_buf_count_len);					
				}				
				
				result = rf_send_smsg_packet(ubPkt,ubPktLen,MSG_REQACK,Id,&udlAck);				
				//result = iKNL_SendPacket(COMM_DATA_TYPE_CMD,ubPkt,ubPktLen,MSG_REQACK,Id,&udlAck);				
				
				if(result != MAX_CMD_PACKET_LEN)
				{
					if(result != COMM_NACK)
						printf("CF1:%d\r\n",result);
				
					ubTimeOutCnt = 0;
					while((result == COMM_TRY_AGAIN_1) || (result == COMM_TRY_AGAIN_2)|| (result == COMM_TIMEOUT_1)|| (result == COMM_TIMEOUT_2))
					{
					#ifdef RTC676x
						if(ubKNL_GetStopTrxFlg())
						{							
							tStatus = TWC_FAIL;
							if(pvTwcrStatusFunc[Opc] != NULL)
							{
								pvTwcrStatusFunc[Opc](Tag,tStatus);
							}
							osSemaphoreRelease(tTWCR_TxAccessSem);
							return tStatus;
						}
					#endif
	
						osDelay(20);
						
						ubTimeOutCnt++;
						if(ubTimeOutCnt > ubTimeOutMaxTh)
						{
							printf("CF1-TimeOut:%d\r\n",result);
							break;
						}					
						
						result = rf_send_smsg_packet(ubPkt,ubPktLen,MSG_REQACK,Id,&udlAck);						
						//result = iKNL_SendPacket(COMM_DATA_TYPE_CMD,ubPkt,ubPktLen,MSG_REQACK,Id,&udlAck);					
					}					
				}				
				
				if(udlAck & 0x01)
				{
					tStatus = TWC_SUCCESS;
					break;
				}
			}
		}
		//printf("TwcTo%d:%d->%d\r\n",Tag,Opc,tStatus);
		if(pvTwcrStatusFunc[Opc] != NULL)
		{
			pvTwcrStatusFunc[Opc](Tag,tStatus);
		}
		osSemaphoreRelease(tTWCR_TxAccessSem);
		return tStatus;
	}
	else
	{
		if(pvTwcrStatusFunc[Opc] != NULL)
		{
			pvTwcrStatusFunc[Opc](Tag,TWC_FAIL);
		}
		osSemaphoreRelease(tTWCR_TxAccessSem);
		return TWC_FAIL;
	}
}
//------------------------------------------------------------------------------
TWC_STATUS tTWCR_StopTwcSend(TWC_TAG Tag, TWC_OPC Opc)
{
//#if OP_STA
//	uint8_t ubXloop;
//	uint8_t ubFlg;
//	ubFlg = 0;
//	for(ubXloop=0; ubXloop<TWC_QUEUE_SIZE; ubXloop++)
//	{
//		if((TwcSend.BUF[ubXloop][((Tag == TWC_AP_MASTER)?0:1)].ubTag == Tag) &&
//		   (TwcSend.BUF[ubXloop][((Tag == TWC_AP_MASTER)?0:1)].ubOPC == Opc))
//		{
//			TwcSend.BUF[ubXloop][((Tag == TWC_AP_MASTER)?0:1)].ubOPC = TWC_RESEV;
//			ubFlg = 1;
//		}
//	}
//#endif
//#if OP_AP
//	uint8_t ubXloop;
//	uint8_t ubFlg;
//	uint8_t ubTag;
//	ubFlg = 0;
//	if((Tag == TWC_AP_SLAVE) || (Tag == TWC_AP_MASTER))
//	{
//		ubTag = 4;
//	}
//	else
//	{
//		ubTag = Tag;
//	}
//	for(ubXloop=0; ubXloop<TWC_QUEUE_SIZE; ubXloop++)
//	{
//		if((TwcSend.BUF[ubXloop][ubTag].ubTag == Tag) &&
//		   (TwcSend.BUF[ubXloop][ubTag].ubOPC == Opc))
//		{
//			TwcSend.BUF[ubXloop][ubTag].ubOPC = TWC_RESEV;
//			ubFlg = 1;
//		}
//	}
//#endif
//	if((ubTWC_NowSendTag == Tag) && (ubTWC_NowSendOpc == Opc))
//	{
//		ubTWC_NowSendTag = 0;
//		ubTWC_NowSendOpc = 0;
//		ubFlg = 1;
//	}
//	if(ubFlg)
//	{
//		return TWC_SUCCESS;
//	}
//	else
//	{
//		return TWC_FAIL;
//	}
	return TWC_FAIL;
}

//------------------------------------------------------------------------------
uint16_t uwTWCR_GetVersion(void)
{
    return ((TWCR_MAJORVER << 8) + TWCR_MINORVER);
}

#endif
