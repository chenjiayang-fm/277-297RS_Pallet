/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		WiFi_API.h
	\brief		Wi-Fi header
	\author		Chris
	\version	1.6
	\date		2020/12/04
	\copyright	Copyright(C) 2020 SONiX Technology Co.,Ltd. All rights reserved.
*/
//-----------------------------------------------------------------------------
#ifndef __WIFI_API_H__
#define __WIFI_API_H__

#include "_510PF.h"
#include "TWC_API.h"

#define PF_MAX_STA_NUM					4
#define E_FW_VER_LEN					30 
#define E_PF_SSID_LEN					30           
#define E_PB_MAX_LAN					32
#if 1
#define E_BUC_APPS_SYNC_MIN_TIMEOUT	    300	// 4 sec
#define E_BUC_APPS_SYNC_MAX_TIMEOUT	    650	// 6.5 sec
#else
#define E_BUC_APPS_SYNC_MIN_TIMEOUT	    800	// 8 sec
#define E_BUC_APPS_SYNC_MAX_TIMEOUT	    2500	// 25 sec
#endif

#define E_WOR_ADVERT_INTERVAL_TIME		50	// (500/50ms)
#define E_WOR_ADVERT_NORMAL_TIME			500	// (500/50ms)
#define E_WOR_ADVERT_EVENT_TIME			50	// (50/50ms)
#define E_WOR_ADVERT_TIMEOUT				2000   	// 10 sec
#define E_WOR_MAX_DATA_LEN				250         // bytes
#define E_WOR_APPS_ALIVE_TIME			3000			// 30 sec
#define E_WOR_APPS_CHK_TIME				2200	 	// 22 sec
#define E_BLE_FOTA_CUST_DEFINED			1   		// for customer defined

//------------------------------------------------------------------------------
#define AP_MAX_NUMBER					1
#define REPEATER_MAX_NUMBER				3
#define AP_WORKING_CHANNEL				11
#define WIFI_CH_MAX_NUM					0x0D
#define PF_ETH_BUF_CNT					10

#define E_PF_PWD_WEP128_MAX_LEN			13
#define E_PF_PWD_WEP64_MAX_LEN			5
#define E_PF_PWD_MAX_LEN				E_PF_PWD_WEP128_MAX_LEN
#define E_DT_WPA_PWD_LEN				8


#define MANUAL_DT_ENCRY_TYPE			0		//! 0:OPEN 1:WPA2
#define MANUAL_DT_PWD_USER_SET			1

#define MANUAL_DT_SSID					"SONiX_" 
#define MANUAL_DT_PWD					"12345678"	// "bbbbbbbb" 
#define MANUAL_VDO_ST_PWD				"12345678"

//#define E_VDO_DT_RES					1
#define VDO_FRC_FPS						25  		
#define VDO_MIN_FPS						8
#define VDO_QC_DROP_TIME				3
#define E_RVCS_ALL_I_FRM				0
#define VDO_RESO_AUTO_SWITCH			1

#define VDO_RESO_SWCH_DWN_FPS			10
#define VDO_RESO_SWCH_UP_FPS			20
#define VDO_RESO_AUTO_SWCH_OBSV_SEC		7

#define PLYBK_DWNLD_MODE_EVENT			0xB0

#define E_DT_DR_LEVEL					E_DT_LEVEL_NO_CHANGE

#define UI_CODE_VERSION					"21.09.12.01"       //! YY.MM.DD.XX   XX: build version in day.

#define FLIPW_word( x ) ( (((uint16_t) (x)[1])<<8) + (x)[0] ) 
#define FLIPW_long( x ) ( (((uint32_t) (x)[3])<<24) + (((uint32_t) (x)[2])<<16) + (((uint32_t) (x)[1])<<8) + (x)[0] ) 

enum {
	VDO_SEN_NOCHANGE = 0,
	VDO_SEN_MIRROR,
	VDO_SEN_FLIP,
	VDO_SEN_MIRROR_FLIP,
};

enum {
	VDO_SEN_POWER_FREQ_50HZ = 1,
	VDO_SEN_POWER_FREQ_60HZ = 2,		
};

enum {
	E_OPCODE_SET = 0,
	E_OPCODE_GET,
};

// {G_54M, G_48M, G_24M, B_11M, B_55M};
enum {
	E_DT_LEVEL_1 = 0,
	E_DT_LEVEL_2,
	E_DT_LEVEL_3,
	E_DT_LEVEL_4,
	E_DT_LEVEL_5,

	E_DT_LEVEL_CNT,	
	E_DT_LEVEL_NO_CHANGE = 0xFF,	
}; // E_DT_LEVEL;

enum {
	E_DT_OPEN = 0, 
	E_DT_WPA2,
};

enum {		
	TARGET_STA0 = 0,
	TARGET_STA1,
	TARGET_STA2,
	TARGET_STA3,	
	TARGET_STA4,
	TARGET_STA5,
	TARGET_STA6,
	TARGET_STA7,
	TARGET_STA8,

	TARGET_AP = 0xF,

	TARGET_RPT0 = 0x10,
	TARGET_RPT1 = 0x20,
	TARGET_RPT2 = 0x40,
	TARGET_RPT3 = 0x80,
	TARGET_ESSID = 0x90,
	TARGET_ERROR = 0xEE,
	TARGET_BROADCAST = 0xFF,
};

//#define E_FW_VER_LEN		30  // TX(4)+BT(?)
#define E_FW_DATA_TX		0x01
#define E_FW_DATA_BT		0x08
#define E_FW_DATA_MASK	0x09

#define APP_VDO_OFFSET_PWD_ENABLE		8
#define APP_VDO_OFFSET_SECURITY_KEY		9
#define APP_VDO_OFFSET_PWD_LEN			10
#define APP_VDO_OFFSET_PWD_DATA			11
#define APP_PWDCHG_OFFSET_PWD_LEN		0
#define APP_PWDCHG_OFFSET_PWD_DATA		1

enum {  
	E_TWC_CMD_OK = 0,
	E_TWC_STOP_NO_RESOURCE_CONNECT,
	E_TWC_STOP_CERTIFICATE_FAIL,
	E_TWC_STOP_CERTIFICATE_FAIL_RECNN,
	E_TWC_CMD_RECONNECT,
};

enum {
	E_STATUS_FW_STATUS = 0,
	E_STATUS_FW_BEGIN_UPDATE,	
	E_STATUS_FW_TX_UPDATING,
	E_STATUS_FW_TX_UPDATE_OK,	
	E_STATUS_FW_TX_UPDATE_FAIL,	
	E_STATUS_FW_BT_UPDATING,
	E_STATUS_FW_BT_UPDATE_OK,
	E_STATUS_FW_BT_UPDATE_FAIL,	

	E_STATUS_APP_PREVIEW_TIMEOUT = 20,
	E_STATUS_WIFI_INITIAL_TIMEOUT
};

enum {
	E_BT_FOTA_FAIL = 0,
	E_BT_FOTA_OK,	
	E_BT_FOTA_PROCESSING,
};

//! Wireless Data(WD) OPCode --> (Internal: 0~127)
typedef enum 
{
	UI_APPTWC_VDO_START         = 0,
	UI_APPTWC_VDO_STOP          = 1,
	UI_APPTWC_VDO_RESTART       = 2,
	UI_APPTWC_VDO_I_FRAME       = 3,
	
	UI_APPTWC_ADO_START         = 4,
	UI_APPTWC_ADO_STOP          = 5,
	UI_APPTWC_ADO_RESTART       = 6,
	
	UI_APPTWC_FRM_OK            = 7,
	UI_APPTWC_FRM_LOST          = 8, 	
	
	UI_APPTWC_PV_MODE_SW        = 9,
	UI_APPTWC_SET_CWMAX         = 10,
	
	UI_APPTWC_STA_ALIVE         = 11,

	UI_APPTWC_SET_CWMIN         = 12,
	UI_APPTWC_SET_AIFSN         = 13,
	UI_APPTWC_SET_TXRTY         = 14,

	UI_APPTWC_REQ_FW_VERSION    = 15,
	UI_APPTWC_GET_FW_VERSION    = 16,
		
	UI_APPTWC_SEND_TE 		    = 17,
	UI_APPTWC_RECE_TE		    = 18,
	UI_APPTWC_CLEAR_TE	        = 19,
	
	UI_APPTWC_NOTIFY_FW_UPDATE	    = 20, 	// 0x14, 
	UI_APPTWC_SEND_FWUPDATE_OK	    = 21,
	UI_APPTWC_SEND_FWUPDATE_FAIL	= 22,
	
	UI_APPTWC_STA_ALIVE_AGAIN	    = 23,

	UI_APPTWC_WEM_SEND_TEST 		= 24,
	UI_APPTWC_WEM_RECEIVE_TEST		= 25,
	UI_APPTWC_WEM_CLEAR_TEST_REPORT	= 26,
	UI_APPTWC_VDO_PARAMETER     	= 27,
	UI_APPTWC_SET_SYNC_TIME_TEST  	= 28,

	UI_APPTWC_UPDATA_FW_INFO        = 0x25,  // 37

	UI_APPTWC_SNPSHT_PLYBK_FUNC		= 0x26,

	UI_APPTWC_FRM_ACK_REQ 			= 0x27,

	UI_APPTWC_RESO_SWITCH_REQ		= 0x28,

	UI_APPTWC_PLYBK_FILE_READ		= 0x29,

	UI_APPTWC_VDO_PARM_MIRROR_FLIP  = 0x30,

	UI_APPTWC_VDO_SENSOR_POWER_FREQ = 0x40,	

	UI_APPTWC_PROP_BEACON			= 0x7F,
	
	/* UI OP CODE start from 128th. The front 128 opcode reserve in EXT*/
	UI_APPTWC_VDO_IQ_BRIGHTNESS 	= 0x80,	
	UI_TWC_VDO_IQ_CHROMA,       			
	UI_TWC_VDO_IQ_CONTRACT,					
	UI_TWC_VDO_IQ_SATURATION,				

	UI_TWC_ADO_VOL_CTL	 			= 0x8C,
	UI_TWC_PUSHTALK_FUNC 			= 0x8D,
	UI_TWC_CHANGE_RF_MODE 			= 0x8E,
	UI_TWC_SET_PF_PASSWORD 			= 0x90,
	UI_TWC_PWIFI_PAIRING_MODE 		= 0x91,
	UI_APPTWC_SET_PRVSRC			= 0x92,
	
	UI_APPTWC_MAX_NUM,
} CMD_Type;

typedef enum {
	APP_OTA_UPDT_SUCCESS = 0,
	APP_OTA_UPDT_SF_TRAN_OK,
	APP_OTA_UPDT_HDR_TAG_OK,
	APP_OTA_UPDT_CRC_OK,
	APP_OTA_UPDT_BURN_SF,
	
} E_APP_OTA_UPDT_RSLT;

enum {
	E_BLE_ADVERT_NORMAL = 0,
	E_BLE_ADVERT_EVENT,	
};

enum {
	E_510_PWN_ON = 1,	
	E_510_PWN_SAVING,	
};

enum {
	E_EVENT_RST			= 1,
	E_EVENT_AKEY_TRIG,
	E_EVENT_VOICE_TRIG,
	E_EVENT_TRIG_3,
	E_EVENT_TRIG_4,
	E_EVENT_TRIG_5,
	E_EVENT_TRIG_6,
	E_EVENT_TRIG_7,
	E_EVENT_TRIG_8,
};

enum
{
	E_STREAM_MODE_ALL_I   = 0,		/* ALL I frames of H.264 */
	E_STREAM_MODE_I_P     = 1,		/* I/P stream of H.264 */

	E_STREAM_MODE_NUM
};

#pragma pack(push) /* push current alignment to stack */
#pragma pack(1) /* set alignment to 1 byte boundary */
//==========================
typedef struct
{
	uint8_t			PktSrcAddr;			// packet source address
	uint8_t			PktDestAddr;		// packet destination address
	uint8_t			PktIndex[2];		// packet index
	uint8_t			PktSize[2];			// packet size
	uint8_t			PktType;			// packet type
} PACKET_IDEX;

typedef struct
{
// 	uint8_t			Dummy1;
	uint8_t			TwcOpc;				// two way cmd op-code
	uint8_t			TwcData[4];			// two way cmd data field
} PKT_DP_Hdr, *pPKT_DP_Hdr;

typedef struct
{
	PACKET_IDEX     PacketIdex;
	PKT_DP_Hdr		DP_Hdr;
} DP;

typedef struct
{
	uint8_t			AdoSmpRate[2];		// audio sampling rate
	uint8_t			AdoTS[4];			// audio timestamp
	uint8_t			TwcOpc;				// two way cmd op-code
	uint8_t			TwcData[2];			// two way cmd data field
	uint8_t			DataPayload[4];		// Audio payload	
} PKT_ADOP_Hdr, *pPKT_ADOP_Hdr;

typedef struct
{
	PACKET_IDEX		PacketIdex;
	PKT_ADOP_Hdr	ADOP_Hdr;
} ADOP;
#pragma pack(pop)

//! Callback function definition.
typedef void(*pvUIApp_TwcProcess)(DP*, uint8_t *, uint8_t *);
typedef uint8_t(*pvUIApp_BTProcess)(uint8_t *, uint32_t, uint8_t);
typedef uint8_t(*pvUIApp_FWStatusProc)(uint8_t ubStatus, uint8_t *ubData);
typedef void(*pvUIApp_AdoProcess)(ADOP*);
typedef void(*pvDT_RstRc)(uint8_t);

// Callback of BLE Image
typedef uint8_t (*BLEFillHdrcallback) (uint32_t ulSrcAddr, uint32_t ulDstAddr, uint32_t ulPktCnt, uint8_t ubFrmIdx, uint32_t ulFrmSize, uint32_t ulShortPktLen);


typedef struct
{
	pvUIApp_TwcProcess		pvTwcProc;
	pvUIApp_AdoProcess		pvAdoProc;
	pvUIApp_BTProcess		pvBTProc;
	pvUIApp_FWStatusProc	pvFwStsProc;
	pvDT_RstRc				pvRcRstFunc;
}WiFiDt_CbFunc_t;
void WiFiDt_RegCbFunc(WiFiDt_CbFunc_t tCbFunc);
uint32_t ulWiFiDt_BufSetup(uint32_t ulBUF_StartAddr);
void WiFiDt_Open(void);
void WiFiDt_Close(void);
void WiFiDt_Setup(uint8_t ubFps, uint16_t uwVdo_H, uint16_t uwVdo_V, uint32_t ulStream_type);
void WiFiDt_Setup_Lite(uint8_t ubFps, uint16_t uwVdo_H, uint16_t uwVdo_V, uint32_t ulStream_type);
uint16_t uwWiFiDt_GetBufUsedLvl(void);
uint16_t uwWiFiDt_GetTxVdoQues(uint8_t ubFreeCnt);
uint16_t uwWiFiDt_GetRxCmdQues(void);
void WiFiDt_SetDispFps(uint8_t ubFps);
uint8_t ubWiFiDt_GetDispFps(void);
uint8_t ubWiFiDt_GetRateLvl(uint8_t ubUpdate);
uint8_t ubWiFiDt_GetLinkStatus(void);
void WiFiDt_SetSsid(uint8_t ubMode, char *SSIDH, uint16_t uwSSIDHLen, uint8_t ubUpdtBeacon);
uint8_t ubWiFiDt_GetSsid(uint8_t *pSsid);
void WiFiDt_SetWepPassword(uint8_t ubSecuMode, uint8_t ubMode, uint8_t *ubPasswaord);
void WiFiDt_GetMacAddr(uint8_t *ubMACAddr);
void WiFiDt_SetDataRate(uint8_t ubRate);
void WiFiDt_SetPushTalkFunc(uint8_t ubEnable);
uint8_t ubWiFiDt_GetTransFrm(void);
uint8_t ubWiFiDt_GetRssi(uint8_t ubSta);
void WiFiDt_ResetVdoAdoQue(uint8_t ubTimeoutStop);
uint8_t ubWiFiDt_TwcProc(DP* pPtr, uint8_t *SrcIP, uint8_t *SrcMac, uint8_t ubFlag);
void WiFiDt_SendTwc(uint8_t ubOpCode, uint8_t ubRx_Id, uint8_t *DstIP, uint8_t *pbPtr, uint32_t ulDataLen, uint8_t ubCount);
uint8_t ubWiFiDt_GetVdoTransSts(void);
uint8_t ubWiFiDt_TxMQReq(uint32_t ulBufInAddr, uint32_t ulReqSize, uint32_t ubFrmType, uint8_t ubVdoGop, uint8_t ubEvent, uint32_t ulTime);
void WiFiDt_SendAdoData(uint8_t ubRxId, uint8_t *pbPtr, uint32_t ulDataLen, uint32_t ulTime);
void WiFiDt_SetupStaEth(void);
void WiFiDt_EnStaEth(void);
uint16_t uwWiFiDt_GetVersion(void);
uint8_t ubWiFiDt_QueryAppConnectStatus(void);
void WiFiDt_UpdateAppConnectStatus(uint8_t ubStatus);
void WiFiDt_StopBeacon(void);
void WiFiDt_StopSend(void);
uint8_t WiFiDt_ChkVdo_TxTrnsmt(void);
void WiFiDt_Set_PlyBk_Dwnld_mode(uint8_t ubDwnld_mode);

#define SUCCESS 		0
#define UNSUCCESS   	1
#define UNASSIGNED		2

#ifdef WIFI_TYPE_SDIO
#define WIFI_LINK_OVERHEAD_BELOW_ETHERNET_FRAME_MAX 	( 8 + 12 + 4 + 2 )
#else
#define WIFI_LINK_OVERHEAD_BELOW_ETHERNET_FRAME_MAX 	( 8 + 12 + 4 + 2 + 2 )
#endif
#define WIFI_LINK_TAIL_AFTER_ETHERNET_FRAME     		( 0 )
#define WIFI_ETHERNET_SIZE         						(14)
#define WIFI_PHYSICAL_HEADER       						(WIFI_LINK_OVERHEAD_BELOW_ETHERNET_FRAME_MAX + WIFI_ETHERNET_SIZE)
#define WIFI_PHYSICAL_TRAILER      						(WIFI_LINK_TAIL_AFTER_ETHERNET_FRAME)
#define WIFI_PAYLOAD_MTU           						(1500)
#define WIFI_LINK_MTU              						(WIFI_PAYLOAD_MTU + WIFI_PHYSICAL_HEADER + WIFI_PHYSICAL_TRAILER)

typedef enum
{
	WIFI_POWER_OFF,
	WIFI_POWER_ON
}WiFi_PowerCtrl_t;

typedef enum
{
	WIFI_MT7601,
	WIFI_AP6181
}WiFi_SupportType_t;

typedef enum
{
	WIFI_STA_INTERFACE     = 0, /**< STA Interface  */
    WIFI_AP_INTERFACE      = 1, /**< SoftAP Interface         */
}WIFI_Interface_t;

typedef enum
{
	RECV_DIRECT,
	RECV_MEMREALLOC,
}WIFI_RecvMemType_t;

typedef enum
{
    WIFI_SECURITY_OPEN           = 0,                 /**< Open security                           */
    WIFI_SECURITY_WPA_TKIP_PSK   = 3,                 /**< WPA Security with TKIP                  */
    WIFI_SECURITY_WPA_AES_PSK    = 4,                 /**< WPA Security with AES                   */
    WIFI_SECURITY_WPA2_AES_PSK   = 5,                 /**< WPA2 Security with AES                  */
    WIFI_SECURITY_WPA2_TKIP_PSK  = 6,                 /**< WPA2 Security with TKIP                 */
    WIFI_SECURITY_WPA2_MIXED_PSK = 7,  				  /**< WPA2 Security with AES & TKIP           */
}WIFI_Security_t;

typedef struct
{
	struct
	{
		uint8_t len;
		uint8_t val[32];
	}eSSID;
	WIFI_Security_t security;
	uint8_t security_key_length;
    char    security_key[64];
	uint8_t channel;
}WIFI_ConfigDct_t;

typedef void(*pvWiFi_PwrCtrl)(WiFi_PowerCtrl_t);
//------------------------------------------------------------------------
/*!
\brief Wi-Fi Function initialize
\param tWiFi_IF 	Wi-Fi interface
\param PwrCtrl_cb 	Wi-Fi dongle power control callback function
\par [Example]
\code    
	 WiFi_Init(WIFI_STA_INTERFACE, APP_WiFiPowerControl);
\return(no)
*/
void WiFi_Init(WIFI_Interface_t tWiFi_IF, pvWiFi_PwrCtrl PwrCtrl_cb);
//------------------------------------------------------------------------
/*!
\brief Wi-Fi dongle initialize
\return(no)
*/
void WiFi_Start(void);
//------------------------------------------------------------------------
/*!
\brief Wi-Fi Buffer Setup
\param ulBUF_StartAddr 	Avaliable DDR address
\param PwrCtrl_cb 	Wi-Fi dongle power control callback function
\par [Example]
\code
	ulWiFi_BufSetup(0xA0000);
\return Wi-Fi Buffer Size
*/
uint32_t ulWiFi_BufSetup(uint32_t ulBUF_StartAddr);
//------------------------------------------------------------------------
/*!
\brief Wi-Fi Config
\param tDct_Info 	Router information
\return(no)
*/
void WiFi_ConfigDct(WIFI_ConfigDct_t tDct_Info);
//------------------------------------------------------------------------
/*!
\brief Get interface of Wi-Fi dongle
\return Interface
*/
WIFI_Interface_t WiFi_GetInterface(void);
//------------------------------------------------------------------------
/*!
\brief Wi-Fi TX Function
\param data 		data
\param data_len		data length
\return result
*/
int WiFi_SendFunc(void *data, uint32_t data_len);
//------------------------------------------------------------------------
/*!
\brief Set callback function of Wi-Fi RX
\param recv_cb 		Callback function of RX
\return Function pointer
*/
typedef void(*pvWiFi_RX)(void *data, uint32_t data_len, void *type);
void WiFi_SetRecv_cbFunc(pvWiFi_RX recv_cb);
//------------------------------------------------------------------------
/*!
\brief Set callback function of Wi-Fi connected.(Link to Router)
\param connected_cb 		Callback function of connected
\return Function pointer
*/
typedef void(*pvWiFi_Connected)(void);
void WiFi_SetConnected_cbFunc(pvWiFi_Connected connected_cb);
//------------------------------------------------------------------------
/*!
\brief Set callback function of Wi-Fi disconnect
\param disconnect_cb 		Callback function of disconnect
\return Function pointer
*/
typedef void(*pvWiFi_DisConnect)(void);
void WiFi_SetDisConnect_cbFunc(pvWiFi_DisConnect disconnect_cb);
//------------------------------------------------------------------------
/*!
\brief Obtain the status of Wi-Fi dongle initialize
\return status
*/
uint32_t WiFi_GetInitStatus(void);
//------------------------------------------------------------------------
/*!
\brief Obtain the MAC address of Wi-Fi dongle
\param mac 		MAC address
\return result
*/
int WiFi_Get_MACAddr(void *mac);
//------------------------------------------------------------------------
/*!
\brief Obtain the Wi-Fi dongle type
\param mac 		MAC address
\return result
*/
uint8_t WiFi_GetWiFiDongleType(void);
//------------------------------------------------------------------------------
/*!
\brief 	Obtain Wi-Fi Version	
\return	Version
*/
uint16_t uwWiFi_GetVersion(void);
//------------------------------------------------------------------------
/*!
\brief query Wcid function
\return Wcid
*/
int iWiFi_WcidInquiry(uint8_t * pMac, uint8_t * wcid, uint8_t * wpaState);
//------------------------------------------------------------------------
/*!
\brief update wpa state function
\return (no)
*/
int iWiFi_SetStaWpaState(uint8_t * pMac, uint8_t wpaState);
//------------------------------------------------------------------------
/*!
\brief query wpa state function
\return wpa state
*/
int iWiFi_GetStaWpaState(uint8_t * pMac, uint8_t* wpaState);
//------------------------------------------------------------------------
int Hostapd_onInit(void* param);
void Hostapd_onUninit(void);
int Hostapd_check_lauched(void);
int Hostapd_setupParam(char* ssid, uint8_t ssidLen, uint8_t* pwd, uint8_t pwdLen, uint8_t* paramSet);
void WiFiDt_StartUpdateFw(uint8_t* ubData);
uint8_t WiFiDt_UpAppConnInfo(uint8_t ubRxId, uint8_t *SrcMac, uint8_t *PortData);

void WiFiDt_StartPlyBk_DownloadMode(void); 

uint16_t uwS2019App_GetBufUsedLvl(uint8_t ubCh);
uint16_t uwS2019App_BufGetPFVdoFrmUsedCnt(void);
uint16_t uwS2019App_BufGetPFVdoFrmMaxCnt(void);

TWC_STATUS tTWC_WiFiDt_RegTransCbFunc(TWC_OPC Opc, pvRptTxStsFunc ReportSts_cb, pvRecvDataFunc RecvData_cb);
uint8_t ubTWC_WiFiDt_GetData(TWC_TAG tTWCW_RcvTag, TWC_OPC tTWCW_RcvOpc, uint8_t *SrcIP, uint8_t *pData);
TWC_STATUS WiFiDt_PlyBkSend(TWC_TAG Tag, TWC_OPC Opc, uint8_t *Data, uint8_t ubLen, uint8_t ubRetry);
uint16_t uwWiFiDt_HwCalcCrc(uint32_t ulAddr, uint32_t ulSize);

uint8_t ubWiFiDt_getPwrSvStatus(void);
uint32_t ubWiFiDt_getPwrSvTime(void);
uint16_t uwWiFiDt_getRcvTwcCnt(void);
uint16_t uwWiFiDt_getRcvFrmAckCnt(uint8_t ubFrmOk);
uint32_t uwWiFiDt_getFrmAckWaitTime(void);
uint16_t uwWiFiDt_getRcvPwrSvCtrlPktCnt(uint8_t ubHrtBt);
uint16_t uwWiFiDt_getVdoSentSz(void);

// BLE Image functions
uint8_t ubWiFiDt_BleUse_Init(void);
uint8_t ubWiFiDt_BleUse_Uninit(void);
void ubWiFiDt_BleUse_RegisterFillHdr_callback(BLEFillHdrcallback callback);
uint8_t ubWiFiDt_BleUse_VideoBufRequest(uint32_t ulBufInAddr, 
			uint32_t ulReqSize, 
			uint32_t ubFrmType, 
			uint32_t ulTime, 
			uint32_t ulATCmdLen,
			uint32_t ulBleVdoHdrLen, 
			uint32_t ulPaddingLen,
			uint32_t ulMaxPktLen);
uint8_t ubWiFiDt_BleUse_VideoBufRelease(uint8_t ubFrmIdx, uint32_t ulFrmLen);
void ubWiFiDt_BleUse_ResetVdoQueue(void);
uint16_t uwWiFiDt_BleUse_GetTxVdoQues(uint8_t ubFreeCnt);
uint32_t ubWiFiDt_BleUse_GetFrameFromQueue(uint8_t *ubFrmIdx, uint32_t *ulFrmLen, uint32_t *ulPktTrgtCnt);
uint8_t ubWiFiDt_BleUse_GetVdoQueueCount(void);
uint8_t ubWiFiDt_BleUse_GetVdoTransSts(void);
void WiFiDt_BleUse_SetVdoMaxPktSize(uint16_t uwSize);

#endif
