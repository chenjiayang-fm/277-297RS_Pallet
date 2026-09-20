/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		ETH_API.h
	\brief		Ethernet API header file
	\author		Chinwei Hsu
	\version    0.2
	\date       2020/3/27
	\copyright	Copyright(C) 2017 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _ETH_API_H_
#define _ETH_API_H_

#include "_510PF.h"

typedef void(*ETH_IsrHook)(void);

//-----------------------------------------------------------------------------
#pragma pack(push)
#pragma pack(1)
//-----------------------------------------------------------------------------
typedef struct MAC_INITIAL_PARAMETER
{
	uint32_t ulBufStartAddr;
	ETH_IsrHook TxIsrHook;
	ETH_IsrHook RxIsrHook;
}ETH_INIT_t;
//-----------------------------------------------------------------------------
#pragma pack(pop)
//-----------------------------------------------------------------------------

//==============================================================================
// Ethernet external API
//==============================================================================
uint16_t uwEth_GetVersion(void);
uint32_t ulEth_GetTotalBufferSize(void);
uint8_t ubEth_Init(ETH_INIT_t *info);
uint8_t ubEth_SendOnePkt(uint32_t ulSendDataAddr,uint32_t ulSize);
uint8_t ubEth_SendMultiPkt(uint32_t *pSendDataAddr,uint32_t ulTotalSize,uint32_t ulMaxPktSize);
uint8_t ubEth_RcvOnePkt(uint32_t ulRcvDataAddr,uint32_t ulBufSize);
void Eth_RcvStop(void);
uint32_t ulEth_GetRcvPktSz(void);
void Eth_ResetRcvPktSz(void);
void Eth_WaitTxFinish(void);
void Eth_WaitRxFinish(void);
uint8_t ubEth_GetTxStatus(void);
uint8_t ubEth_GetRxStatus(void);
uint8_t ubEth_CheckTxDesc(void);
uint8_t ubEth_CheckRxDesc(void);
void Eth_SetFilterMode(uint8_t ubGroup);
void Eth_SetMacAddr(uint8_t * pMacAddr);
void Eth_TxIsrHook(void);
void Eth_RxIsrHook(void);
uint8_t ubEth_GetLinkStatus(void);

//For Testing
void Eth_Test_Init(uint8_t ubMode);	//0 is for Tx; 1 is for Rx
#endif


