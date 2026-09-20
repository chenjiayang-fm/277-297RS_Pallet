/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		sPRF_API.h
	\brief		SPRF header
	\author		Hanyi Chiu
	\version	1.39
	\date		2023/06/16
	\copyright	Copyright(C) 2023 SONiX Technology Co.,Ltd. All rights reserved.
*/
//-----------------------------------------------------------------------------
#ifndef __SPRF_API_H__
#define __SPRF_API_H__

#include "_510PF.h"

#ifdef S2019A

#define sPRF_Init(...)					S2019x_Init(__VA_ARGS__)
#define ulsPRF_GetInitSts				ulS2019x_GetInitStatus
#define ulsPRF_BufSetup(...)			ulS2019x_BufSetup(__VA_ARGS__)
#define sPRF_BufferReset				S2019x_BufferReset
#define sPRF_Send(...)					iS2019x_Send(__VA_ARGS__)
#define sPRF_SetDevId(...)				S2019x_SetDevId(__VA_ARGS__)
#define tsPRF_GetDevId					tS2019x_GetDevId
#define sPRF_GetIdInfo(...)				S2019x_GetIdInfo(__VA_ARGS__)
#define sPRF_GetTxIdInfo(...)			ubS2019x_GetTxIdInfo(__VA_ARGS__)
#define sPRF_GetRxIdInfo(...)			ubS2019x_GetRxIdInfo(__VA_ARGS__)
#define sPRF_UpdateIdInfo(...)			S2019x_UpdateIdInfo(__VA_ARGS__)
#define sPRF_StartPairing				S2019x_StartPairing
#define sPRF_StopPairing				S2019x_StopPairing
#define sPRF_DrvOpen(...) 				S2019x_DrvOpen(__VA_ARGS__)
#define sPRF_DrvClose(...) 				S2019x_DrvClose(__VA_ARGS__)
#define tsPRF_GetDrvSts(...) 			tS2019x_GetDrvSts(__VA_ARGS__)
#define sPRF_RegCbFunc(...)				S2019x_RegCbFunc(__VA_ARGS__)
#define sPRF_SetVdoGop(...)				S2019x_SetVdoGop(__VA_ARGS__)
#define sPRF_EnLowLatencyMode(...)		S2019x_EnLowLatencyMode(__VA_ARGS__)
#define sPRF_FrameRptFunc(...) 			S2019x_FrameRptFunc(__VA_ARGS__)
#define sPRF_DelTxDevId(...)			S2019x_DelTxDevId(__VA_ARGS__)
#define ubsPRF_GetAttachedDevNums 		ubS2019x_GetAttachedDevNums
#define tsPRF_GetRateLvl  				tS2019x_GetRateLvl
#define tsPRF_GetRssi 					ubS2019x_GetRssi
#define tsPRF_GetPER(...)				ubS2019x_GetPER(__VA_ARGS__)
#define tsPRF_GetTxBufNum(...)			ubS2019x_GetTxBufNum(__VA_ARGS__)
#define sPRF_EnAutoSwCh(...)			S2019x_EnAutoSwCh(__VA_ARGS__)
#define sPRF_SetWorkCh(...)				S2019x_SetWorkCh(__VA_ARGS__)
#define ulsPRF_GetIntTmCnt				ulS2019x_GetIntTmCnt
#define ubsPRF_GetWorkCh				ubS2019x_GetWorkCh
#define uwsPRF_GetVersion 				uwS2019x_GetVersion

#define SUCCESS 						0
#define UNSUCCESS   					1
#define UNASSIGNED						2

typedef enum
{
	sPRF_TRX_MODE = 0,
	sPRF_APDIRECT_MODE,
	sPRF_STADIRECT_MODE = 0x10,
	sPRF_BRIDGE_MODE 	= 0x30,
}sPRF_DrvMode_t;

typedef struct
{
#define SPRF_VSNTM_BUF	3
#define SPRF_FRMTM_BUF	4
	uint8_t  ubStaNum;
	float    fFrmLtcy;
	float 	 fMinLtcy;
	float 	 fAvgLtcy;
	uint32_t ulFrmIdx;
	uint8_t  ubvsTmIdx;
	uint32_t ulTxVsyncTm[SPRF_VSNTM_BUF];
	uint32_t ulRxVsyncTm;
	uint32_t ulsFrmTm;
	uint8_t  ubsTmIdx;
	uint32_t uldFrmTm[SPRF_FRMTM_BUF];
	uint8_t  ubUpdFlag;
	uint8_t  ubDispFlag;
	uint8_t  ubRefhFlag;
}SPRF_LatyInfo_t;

typedef struct
{
	uint8_t 		ubEn;
	uint8_t 		ubDbg;
	SPRF_LatyInfo_t tLatency[4];
}SPRF_VdoLatency_t;
#define sPRF_LTYMEAS_EN					1
#define sPRF_DBG_PROC(io, x)			(GPIO->GPIO_O##io = x)

typedef enum
{
	sPRF_TX_IF,
	sPRF_RX_IF
}sPRF_Interface_t;

typedef enum
{
	sPRF_STA1 = 0,
	sPRF_STA2,
	sPRF_STA3,
	sPRF_STA4,
	sPRF_AP = 0xF,
	sPRF_ID_MAX,
	sPRF_ID_INVAILD = 0xFF
}sPRF_DevId_t;

typedef enum
{
	sPRF_LOST_LINK,
	sPRF_LINK,
}sPRF_LinkSts_t;

typedef enum
{
	sPRF_DEVDRV_DIS,
	sPRF_DEVDRV_EN,
}sPRF_DevDrvSts_t;

typedef enum
{
	sPRF_HDR_PKT  = 0x30,
	sPRF_CMD_PKT  = 0x50,
	sPRF_VDO_PKT,
	sPRF_ADO_PKT,
	sPRF_ACK_PKT  = 0x60,
	sPRF_SYNC_PKT,
	sPRF_PREP_PKT = 0x70
}sPRF_PktType_t;

typedef struct
{
	sPRF_DevId_t 	tRptId;
	sPRF_PktType_t 	tPacketType;
	uint32_t		ulDataAddr;
	uint32_t 		ulDataSize;
}sPRF_RxRpt_t;

typedef enum
{
	sPRF_LOWMD_RATE = 0,
	sPRF_NORMALMD_RATE,
	sPRF_RATEMD_MAX = 0xF,
}sPRF_RateMd_t;

typedef enum
{
	sPRF_AI_NORMAL,		//! Default
	sPRF_AI_LVL1,
	sPRF_AI_LVL2,
	sPRF_AI_LVL3,
	sPRF_AI_LVL4,
}sPRF_AdjChAiLvl_t;

typedef enum
{
	sPRF_PWR_OFF,
	sPRF_PWR_ON,
}sPRF_PwrCtrl_t;

typedef void(*pvSPRF_PwrCtrlFunc)(sPRF_PwrCtrl_t);
typedef void(*pvSPRF_RstGopFunc)(sPRF_DevId_t, uint32_t);
typedef void(*pvSPRF_LinkRptFunc)(sPRF_DevId_t, sPRF_LinkSts_t);
typedef uint32_t(*pvSPRF_RxBuf)(sPRF_DevId_t, sPRF_PktType_t, uint8_t);
typedef uint8_t(*pvSPRF_RxProcFunc)(sPRF_RxRpt_t);
typedef uint8_t(*pvSPRF_RptTrxBufNum)(sPRF_DevId_t);
typedef void(*pvSPRF_StpTxProc)(uint8_t);
typedef void(*pvSPRF_DrvMdStp)(void);
typedef void(*pvsPRF_SwDrvProc)(sPRF_DrvMode_t);
typedef struct
{
	pvSPRF_PwrCtrlFunc	pvPwrCtrl;
	pvSPRF_RstGopFunc	pvRstGop;
	pvSPRF_LinkRptFunc	pvLinkRpt;
	pvSPRF_RxBuf		pvGetRxBuf;
	pvSPRF_RxProcFunc	pvRxProc;
	pvSPRF_RptTrxBufNum	pvGetTrxBufNum;
	pvSPRF_StpTxProc	pvStpTxProc;
	pvSPRF_DrvMdStp		pvDrvMdStp;
	pvsPRF_SwDrvProc    pvSwDrvProc;
}sPRF_CbFunc_t;

//------------------------------------------------------------------------
/*!
\brief SONiX RF initialize
\param tRfIF 	RF interface
\par [Example]
\code
	 sPRF_Init(sPRF_TX_IF);
\return(no)
*/
void S2019x_Init(sPRF_Interface_t tRfIF);
//------------------------------------------------------------------------
/*!
\brief Setup driver mode
\param sPRF_DrvMode_t 	Driver mode
\return(no)
*/
void sPRF_SetDrvMode(sPRF_DrvMode_t tDrvMode);
//------------------------------------------------------------------------
/*!
\brief Get driver mode
\return Driver mode
*/
sPRF_DrvMode_t tsPRF_GetDrvMode(void);
//------------------------------------------------------------------------
/*!
\brief Check default driver mode, only for TX mode.
*/
void sPRF_ChkDefuDrvMode(void);
//------------------------------------------------------------------------
/*!
\brief S2019x Buffer Setup
\param ulBUF_StartAddr 	Avaliable DDR address
\par [Example]
\code
	ulS2019x_BufSetup(0xA0000);
\return Buffer Size
*/
uint32_t ulS2019x_BufSetup(uint32_t ulBUF_StartAddr);
//------------------------------------------------------------------------
/*!
\brief S2019x Device ID Setup
\param tDevId 		Device ID
\return (no)
*/
void S2019x_SetDevId(sPRF_DevId_t tDevId);
//------------------------------------------------------------------------
/*!
\brief Get S2019x Device ID
\return Device ID
*/
sPRF_DevId_t tS2019x_GetDevId(void);
//------------------------------------------------------------------------
/*!
\brief S2019x TX Function
\param data 		data
\param data_len		data length
\return result
*/
int iS2019x_Send(sPRF_DevId_t tDstId, sPRF_PktType_t tPktType, void *data, uint32_t data_len, uint8_t ubBufType);
//------------------------------------------------------------------------
/*!
\brief Register callback function for S2019x.
\param tCbFunc 		Callback function for S2019x.
\return (no)
*/
void S2019x_RegCbFunc(sPRF_CbFunc_t tCbFunc);
//------------------------------------------------------------------------
/*!
\brief Set video GOP.
\param uwGop 		GOP
\return (no)
*/
void S2019x_SetVdoGop(uint16_t uwGop);
//------------------------------------------------------------------------
/*!
\brief Set low latency mode
\param ubEn 		Enable?
\return (no)
*/
void S2019x_EnLowLatencyMode(uint8_t ubEn);
//------------------------------------------------------------------------
/*!
\brief Obtain the status of S2019x initialize
\return status
*/
uint32_t ulS2019x_GetInitStatus(void);
//------------------------------------------------------------------------
/*!
\brief Obtain the ID info of S2019x
\param pId 		ID
\return result
*/
void S2019x_GetIdInfo(void *pId);
//------------------------------------------------------------------------
/*!
\brief Obtain the TX ID info of S2019x
\param tTxDev 		TX number
\param pTxId 		TX ID
\return 0:Invalid, 1:Invalid
*/
uint8_t ubS2019x_GetTxIdInfo(sPRF_DevId_t tTxDev, void *pTxId);
//------------------------------------------------------------------------
/*!
\brief Obtain the RX ID info of S2019x
\param pRxId 		RX ID
\return 0:Invalid, 1:Invalid
*/
uint8_t ubS2019x_GetRxIdInfo(void *pRxId);
//------------------------------------------------------------------------
/*!
\brief Update the ID info.
\param tUpdateId 	Device ID
\param pId 			ID info
\return result
*/
void S2019x_UpdateIdInfo(sPRF_DevId_t tUpdateId, uint8_t *pId);
//------------------------------------------------------------------------
/*!
\brief Start pairing function
\return (no)
*/
void S2019x_StartPairing(void);
//------------------------------------------------------------------------
/*!
\brief Stop pairing function
\return (no)
*/
void S2019x_StopPairing(void);
//------------------------------------------------------------------------
/*!
\brief Enable adjust power on pairing mode.
\return (no)
*/
void sPRF_EnAdjPairPwr(void);
//------------------------------------------------------------------------
/*!
\brief S2019x Buffer reset
\return (no)
*/
void S2019x_BufferReset(void);
//------------------------------------------------------------------------
/*!
\brief Open S2019x driver
\return (no)
*/
void S2019x_DrvOpen(sPRF_DevId_t tDevId);
//------------------------------------------------------------------------
/*!
\brief Close S2019x driver
\return (no)
*/
void S2019x_DrvClose(sPRF_DevId_t tDevId);
sPRF_DevDrvSts_t tS2019x_GetDrvSts(sPRF_DevId_t tDevId);
//------------------------------------------------------------------------
/*!
\brief Frame rate report
\param tDevId 	Device ID
\return (no)
*/
void S2019x_FrameRptFunc(sPRF_DevId_t tDevId);
//------------------------------------------------------------------------
/*!
\brief Buffer status report
\param pBufNum Buffer number
\return (no)
*/
void S2019x_BufNumRptFunc(uint8_t *pBufNum);
//------------------------------------------------------------------------
/*!
\brief Delete device id
C
\return (no)
*/
void S2019x_DelTxDevId(sPRF_DevId_t tDevId);
//------------------------------------------------------------------------
/*!
\brief Get attached device number
\return (no)
*/
uint8_t ubS2019x_GetAttachedDevNums(void);
//------------------------------------------------------------------------
/*!
\brief TX rate level
\return rate level
*/
sPRF_RateMd_t tS2019x_GetRateLvl(void);
//------------------------------------------------------------------------
/*!
\brief Get RSSI value
\return RSSI
*/
uint8_t ubS2019x_GetRssi(void);
//------------------------------------------------------------------------
/*!
\brief Get packet error rate
\param tDevId 	Device ID
\return PER
*/
uint8_t ubS2019x_GetPER(sPRF_DevId_t tDevId);
//------------------------------------------------------------------------
/*!
\brief Set adjacent channel interference level
\param tAdjLvl 	Interference level
\return (no)
*/
void sPRF_SetAdjChAiLevel(sPRF_AdjChAiLvl_t tAdjLvl);
//------------------------------------------------------------------------
/*!
\brief Enbale keep alive function.
\param ubEn 	Enable or disable
\return (no)
*/
void sPRF_KeepAlive(uint8_t ubEn);
typedef enum
{
	sPRF_ACCP_ALL,
	sPRF_BLK_AV,
	sPRF_BLK_V,
	sPRF_BLK_A,
}sPRF_TxBlockLvl_t;
void sPRF_SetTxBlockLvl(sPRF_TxBlockLvl_t tLvl);
//------------------------------------------------------------------------
/*!
\brief Get buffer number of TX bit stream.
\param ubEn 	Enable or disable
\return (no)
*/
uint8_t ubS2019x_GetTxBufNum(sPRF_DevId_t tTxId);
//------------------------------------------------------------------------
/*!
\brief Get timer count
\return (no)
*/
uint32_t ulS2019x_GetIntTmCnt(void);
//! RF Channel
void S2019x_EnAutoSwCh(uint8_t ubEn);
void S2019x_SetWorkCh(uint8_t ubCh);
uint8_t ubS2019x_GetWorkCh(void);

//------------------------------------------------------------------------
/*!
\brief Set test command for engineer mode.
\param ubRate 	Data rate
\return (no)
*/
void sPRF_EngTstProc(uint8_t *pData);
//------------------------------------------------------------------------------
/*!
\brief 	Obtain S2019X Version
\return	Version
*/
uint16_t uwS2019x_GetVersion(void);
//------------------------------------------------------------------------------
/*!
\brief 	Set debug level.
\return (no)
*/
void sPRF_SetDbgMsgLvl(int32_t tLvl);
//! Direct mode
typedef enum
{
	QID_HW_MAC_ADDR = 0,
	QID_HW_CHANNEL,
	SET_HW_MAC_ADDR,
	QID_HW_ESSID_ADDR,
	SET_HW_CHANNEL,
	SET_BEACON_ON,
	SET_BEACON_OFF,
	SET_BEACON_SSID,
	SET_AUTOFB_ON,
	SET_AUTOFB_OFF,
	SET_TXRTY_VAL,
	GET_TXRTY_VAL,
	SET_BEACON_RATE,
	SET_SECURITY_WEP128,
	SET_SECURITY_WEP_DISABLED,
	SET_SECURITY_WEP64,
	SET_DIS_ACT,
	SET_STA_DIS_ACT,
	SET_MUCAST_FILTER,
	QID_WEP_EN,
	QID_LINK_STATE,
	SET_FN_CIPHER_KEY,
	QID_REMOVE_KEY,
	QID_WPA_PASSWORD,
	SET_WPA_PASSWORD,
	SET_SECURITY_WPA2,
	SET_AP_DIS_ACT,	
}DT_QUEY_ID;
typedef void(*pvsPRF_DtRxData)(uint8_t *, uint8_t);
void sPRF_SetApHandleRxCbFunc(pvsPRF_DtRxData pCb);
typedef void(*pvsPRF_DtDisConnect)(uint8_t *);
void sPRF_SetApHandleDisConnectCbFunc(pvsPRF_DtDisConnect pCb);
typedef uint8_t(*psPRF_DtConnected)(void);
void sPRF_SetApHandleStaContedCbFunc(psPRF_DtConnected pCb);
typedef void(*pvsPRF_DtWpaRx)(void *, uint8_t *, uint8_t);
void sPRF_SetWpaHandleRxCbFunc(pvsPRF_DtWpaRx pCb);
//------------------------------------------------------------------------
/*!
\brief Query and setup DT device
\param item 	Query item
\param pOutBuf 	Data
\param pLen 	Length
\return result
*/
uint8_t sPRF_QueryAndSet(uint8_t item, uint8_t *pOutBuf, uint16_t *pLen);
//------------------------------------------------------------------------
/*!
\brief Bulk out handle
\param ep 	Endpoint
\param add 	Address
\param len 	Bulk out length
\return result
*/
uint8_t ubsPRF_DtmOUThndl(uint8_t ep, uint8_t *add, uint32_t len);
//------------------------------------------------------------------------
/*!
\brief Check driver mode of TX
\param ubDrvMd  Driver mode
\return (no)
*/
#define sPRF_ChkTxDrvMode(...)	S2019x_ChkTxDrvMode(__VA_ARGS__)
void S2019x_ChkTxDrvMode(uint8_t ubDrvMd);
//------------------------------------------------------------------------
/*!
\brief Set default work channel of DT device.
\param ubCh  Channel
\return (no)
*/
void sPRF_SetDefuDtWorkCh(uint8_t ubCh);
//------------------------------------------------------------------------
/*!
\brief Obtain the DT device status
\return (0:Not ready, 1:ready)
*/
uint8_t ubsPRF_GetDtDevRdy(void);
//------------------------------------------------------------------------
/*!
\brief Obtain the DT device link status
\return (0:lost-link, 1:link)
*/
uint8_t ubsPRF_GetDtDevLinkSts(void);
//------------------------------------------------------------------------
/*!
\brief Enable concurrent mode.
\param ubEn  		Enable concurrent mode if TX device is lost-link state.
\param pStaDtProc  	DT mode process.
\return (no)
*/
typedef void(*pvsPRF_StaDtProc)(void);
void sPRF_EnStaDtMode(uint8_t ubEn, pvsPRF_StaDtProc pStaDtProc);
//------------------------------------------------------------------------
/*!
\brief Search station, only for concurrent mode.
\return (no)
*/
void sPRF_SearchStation(void);
//------------------------------------------------------------------------
/*!
\brief Power Down function
\return (no)
*/
void sPRF_DevPwrDown(void);
//------------------------------------------------------------------------
/*!
\brief Enable bridge mode, only for RX.
\param ubEn  		Enable bridge mode.
\param pStaDtProc  	Bridge mode process.
\return (no)
*/
void sPRF_EnApBridgeMode(uint8_t ubEn, pvsPRF_StaDtProc pApBdgProc);
//------------------------------------------------------------------------
/*!
\brief Turn on or off Ap bridge mode.
\param ubOn  		TRUE:turn on, FASLE: turn off.
\return Setup result.
*/
uint8_t ubsPRF_TurnOnApBridgeMd(uint8_t ubOn);
//------------------------------------------------------------------------
/*!
\brief Get bridge mode status
\return Ap bridge mode status, enable or disable.
*/
typedef enum
{
	sPRF_APBDG_IS_OFF = 0,
	sPRF_APBDG_IS_ON,
}sPRF_ApBdgMd_Sts;
sPRF_ApBdgMd_Sts tsPRF_GetApBridgeMdSts(void);
//------------------------------------------------------------------------
/*!
\brief Update RX bridge status.
\param ubSts  		Bridge mode status.
\return (no)
*/
void sPRF_UpdRxBdgSts(uint8_t ubSts);
//------------------------------------------------------------------------
/*!
\brief Production test
\param pvsPRF_PerTst  Callback function for report.
\return (no)
*/
typedef void(*pvsPRF_PerTst)(sPRF_DevId_t, uint8_t);
void sPRF_RegPerTstCbFunc(pvsPRF_PerTst pCb);
//------------------------------------------------------------------------
/*!
\brief Force leave by device.
\return (no)
*/
void sPRF_ForceLeavByDev(void);
#else
#define sPRF_SetDbgMsgLvl(...)		((void)0)
#endif	//! End of S2019A

#endif	//! #ifndef __SPRF_API_H__
