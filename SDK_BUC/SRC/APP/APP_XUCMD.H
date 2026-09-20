/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		APP_XUCMD.h
	\brief		XU Command Application header file
	\author		Hanyi Chiu
	\version	1.0
	\date		2018/08/29
	\copyright	Copyright(C) 2018 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _APP_XUCMD_H_
#define _APP_XUCMD_H_

#include "_510PF.h"

#define XU_CMD_ACC_TIME			100	//ms
#define XU_CMD_READ				0x70

#define XU_CMD_BUF_NUM			10

#define APPXU_EXT_BUF_NUM		10

#define APPXU_XU_MAX_DATA_LEN		64
#define APPXU_XU_MAX_EXT_DATA_LEN	60	//64-2(OpCode)-2(DataLen) = 60

typedef enum
{	
	USB_UVC_DEVICE1	= 0,
	USB_UVC_DEVICE2	= 5,
	USB_HOST		= 0xFF,
}APPXU_USB_TARGET;
typedef enum
{
	SNX_MSG_TYPE_SYNCTIME = 0x10,
	SNX_MSG_TYPE_GET_TX_FW_VERSION,
	SNX_MSG_TYPE_GET_RESOLUTION,
	SNX_MSG_TYPE_GET_CAM_ONLINE_STATUS,
	SNX_MSG_TYPE_SET_DEVICE_ENABLE,
	SNX_MSG_TYPE_GET_LIB_VERSION,
	SNX_MSG_TYPE_REMOTE_WAKEUP,
	SNX_MSG_TYPE_GET_REMOT_WAKEUP_PREVIEW_INTERVAL,
	SNX_MSG_TYPE_SET_REMOT_WAKEUP_PREVIEW_INTERVAL,
	SNX_MSG_TYPE_GET_TX_BATTERY_LEVEL,
	SNX_MSG_TYPE_GET_REMOTE_WAKEUP_STATUS,
	SNX_MSG_TYPE_GET_USBD_MODE = 0x1B,
	SNX_MSG_TYPE_RESEND_IFRAME,
	SNX_MSG_TYPE_GET_RX_INFOR,
	SNX_MSG_TYPE_SET_RX_INFOR,
	SNX_MSG_TYPE_GET_TX_BATTERY_STATUS = 0x20,	
	SNX_MSG_TYPE_GET_INCREASE_PREVIEW_INTERVAL,
	SNX_MSG_TYPE_SET_INCREASE_PREVIEW_INTERVAL,
	SNX_MSG_TYPE_GET_MAIN_VSTREAM_RES,
	SNX_MSG_TYPE_SET_MAIN_VSTREAM_RES,
	SNX_MSG_TYPE_GET_AUDIO_MUTE,
	SNX_MSG_TYPE_SET_AUDIO_MUTE,
	SNX_MSG_TYPE_SET_WIFI_STATUS,
	SNX_MSG_TYPE_SET_VIEW_TYPE = 0x2F,
    SNX_MSG_TYPE_SET_CAPABILITY	= 0x30,
	SNX_MSG_TYPE_GET_CAPABILITY,
	SNX_MSG_TYPE_CAPABILITY_SET_DEFAULT,
    SNX_MSG_TYPE_SET_NTSC_PAL = 0x38,
    SNX_MSG_TYPE_GET_NTSC_PAL,
	SNX_MSG_TYPE_SET_FCC_CE,
	SNX_MSG_TYPE_GET_PIR_SETTING = 0x40,
	SNX_MSG_TYPE_SET_PIR_SETTING,
	SNX_MSG_TYPE_GET_PIR_WEEK_SCHEDULE,
	SNX_MSG_TYPE_SET_PIR_WEEK_SCHEDULE,
    SNX_MSG_TYPE_SET_PIR_ADJUST = 0x48,
    SNX_MSG_TYPE_GET_PIR_ADJUST,
	SNX_MSG_TYPE_UPDATE_FIRMWARE = 0x51,
	SNX_MSG_TYPE_QUERY_UPDATE_STATUS,
	SNX_MSG_TYPE_CHECK_DEVICE_READY,
	SNX_MSG_TYPE_CAM_SET_PAIRING = 0x5A,
	SNX_MSG_TYPE_CAM_GET_PAIRING_STATUS,
	SNX_MSG_TYPE_GET_CAM_PAIRED_STATUS,
	SNX_MSG_TYPE_CAM_UNPAIR,
	SNX_MSG_TYPE_PUSH_TALK = 0x60,
	SNX_MSG_TYPE_ALERT_MELODY,
//	SNX_MSG_TYPE_ENGMODE_SETTING,
	SNX_MSG_TYPE_ENGMODE_START = 0x73,
	SNX_MSG_TYPE_ENGMODE_STOP,
	SNX_MSG_TYPE_ENGMODE_GET_TEST_RESULT,
	SNX_MSG_TYPE_SET_TEST_MODE = 0x80,
	SNX_MSG_TYPE_GET_TEST_RESULT,
	SNX_MSG_TYPE_USER_DEFINED_SET_CMD_SET_DAT = 0x90,
	SNX_MSG_TYPE_USER_DEFINED_SET_CMD_GET_DAT,

	SNX_MSG_TYPE_SET_HOST_TO_DEVICE = 0xA0,
	SNX_MSG_TYPE_GET_DEVICE_TO_HOST	= 0xA1,
	SNX_MSG_TYPE_GET_ACTIVE 		= 0xA2,	//Active Message from Device to Host
}APP_XuOpcode_t;

typedef enum
{
	APP_UNPAIR,
	APP_PAIRING,
	APP_PAIR_OK,
	APP_PAIR_FAIL,
}APP_PairSts_t;

void APPXU_Init(void);
void APPXU_RecvCmd(uint8_t ubMode, uint32_t *pCmd);

typedef void (*pvAPPXU_GetDataFromUsbCb)(uint8_t *pBuf,uint8_t ubLen);
void APPXU_GetDataFromUsbCbFunc(APPXU_USB_TARGET tTarget,pvAPPXU_GetDataFromUsbCb pFunc);
void APPXU_GetDataFromUsbHost(uint8_t *pBuf,uint8_t ubLen);
void APPXU_GetDataFromUsbDevice1(uint8_t *pBuf,uint8_t ubLen);
void APPXU_GetDataFromUsbDevice2(uint8_t *pBuf,uint8_t ubLen);
#if OP_AP
static void APPXU_UsbCmdMonitAtHostThread(void const *argument);
#endif
uint8_t ubAPPXU_SendData(APPXU_USB_TARGET tTarget,uint8_t *pBuf,uint8_t ubLen);

#endif
