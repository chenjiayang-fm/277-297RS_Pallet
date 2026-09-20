/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		APP_CFG.h
	\brief		APP Configuration header file
	\author		Hanyi Chiu
	\version	0.8
	\date		2020/05/27
	\copyright	Copyright(C) 2020 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _APP_CFG_H_
#define _APP_CFG_H_
#include <stdint.h>
#include "bsp_config.h"

//------------------------------------------------------------------------------
//!Thread Priority Setting
#ifdef RTC676x
#define THREAD_PRIO_APP_HANDLER			osPriorityAboveNormal
#define THREAD_PRIO_UI_HANDLER			osPriorityNormal
#define THREAD_PRIO_UIEVENT_HANDLER		osPriorityNormal
#define THREAD_PRIO_KEY_HANDLER			osPriorityAboveNormal
#else
#define THREAD_PRIO_APP_HANDLER			osPriorityAboveNormal
#define THREAD_PRIO_UI_HANDLER			osPriorityAboveNormal

#define THREAD_PRIO_UI_PWRCTRL_HANDLER	osPriorityAboveNormal
#define THREAD_PRIO_UIEVENT_HANDLER		osPriorityAboveNormal
#define THREAD_PRIO_KEY_HANDLER			osPriorityAboveNormal
#endif
#define THREAD_PRIO_BB_HANDLER			osPriorityRealtime
#define THREAD_PRIO_TWC_HANDLER			osPriorityAboveNormal
#define THREAD_PRIO_RW_RX_HANDLER		osPriorityHigh
#define THREAD_PRIO_RW_PACKET_HANDLER	osPriorityHigh

#define THREAD_PRIO_RF_TEST_HANDLER		osPriorityNormal

#define THREAD_PRIO_KNL_AVG_PLY			osPriorityAboveNormal
#define THREAD_PRIO_KNL_TWC_MONIT		osPriorityNormal
#define THREAD_PRIO_KNL_VDOIN_PROC		osPriorityAboveNormal
#define THREAD_PRIO_KNL_RESSW_PROC		osPriorityAboveNormal

#define THREAD_PRIO_ADO_ENC_PROC		osPriorityAboveNormal
#define THREAD_PRIO_ADO_DEC_PROC		osPriorityNormal		//! osPriorityHigh
#ifdef S2019A
#define THREAD_PRIO_KNL_PROC			osPriorityHigh
#define THREAD_PRIO_COMM_TX_VDO			osPriorityHigh
#define THREAD_PRIO_COMM_RX_VDO			osPriorityHigh
#else
#define THREAD_PRIO_KNL_PROC			osPriorityHigh
#define THREAD_PRIO_COMM_TX_VDO			osPriorityHigh
#define THREAD_PRIO_COMM_RX_VDO			osPriorityNormal
#endif
#define THREAD_PRIO_COMM_RX_ADO			osPriorityNormal
#define THREAD_PRIO_IMG_MONIT			osPriorityNormal
#define THREAD_PRIO_PAIRING_HANDLER		osPriorityNormal
#define THREAD_PRIO_EN_HANDLER			osPriorityBelowNormal
#define THREAD_PRIO_KNLRECORD_HANDLER	osPriorityBelowNormal
#define THREAD_PRIO_JPEG_MONIT			osPriorityBelowNormal
#define THREAD_PRIO_LINK_MONIT			osPriorityBelowNormal
#define THREAD_PRIO_RC_MONIT			osPriorityBelowNormal
#define THREAD_PRIO_RC_SYS_MONIT		osPriorityBelowNormal
#define THREAD_PRIO_LINK_UPDATE			osPriorityBelowNormal
#define THREAD_PRIO_SEC_MONIT			osPriorityBelowNormal

//#define THREAD_PRIO_USB_MONIT			osPriorityHigh
#define THREAD_PRIO_USB_MONIT			osPriorityNormal
#define THREAD_PRIO_USB_CMD_MONIT		osPriorityBelowNormal
#define THREAD_PRIO_USB_RESET			osPriorityNormal

#define THREAD_PRIO_TIME_STAMP          osPriorityNormal
#define THREAD_PRIO_TX_REC              osPriorityNormal
#define THREAD_PRIO_GOPSYNC          	osPriorityNormal
#define THREAD_PRIO_SYS_MONIT			osPriorityBelowNormal
#define THREAD_PRIO_KNL_SD_HANDLER      osPriorityNormal
#define THREAD_PRIO_KNL_EMERGENCY_HANDLER      osPriorityNormal
#define THREAD_PRIO_KNL_TEST_HANDLER    osPriorityNormal
#define THREAD_PRIO_KNL_RTC_MONIT_HANDLER    osPriorityNormal
#define THREAD_PRIO_WDT_HANDLER			osPriorityLow				// other thread can not set osPriorityLow!!!
#define THREAD_PRIO_UI_SIGNAL_HANDLER	osPriorityAboveNormal
#define THREAD_PRIO_DP_HANDLER			osPriorityNormal

//------------------------------------------------------------------------------
//! Thread Stack Size Setting
#define THREAD_STACK_UI_HANDLER			3072
#define THREAD_STACK_UI_PWRCTRL_HANDLER	512

#define THREAD_STACK_UIEVENT_HANDLER	8192
#define THREAD_STACK_KEY_HANDLER		1024
#define THREAD_STACK_BB_HANDLER			8192
#define THREAD_STACK_TWC_HANDLER	    4096
#define THREAD_STACK_RW_RX_HANDLER		8192
#define THREAD_STACK_RW_PACKET_HANDLER	10240

#define THREAD_STACK_RF_TEST_HANDLER	1024

#define THREAD_STACK_EN_HANDLER			8192
#define THREAD_STACK_KNL_PROC			10240
#define THREAD_STACK_KNL_AVG_PLY		2048
#define THREAD_STACK_KNL_TWC_MONIT		2048
#define THREAD_STACK_KNL_VDOIN_PROC		5120
#define THREAD_STACK_KNL_RESSW_PROC		2048

#define THREAD_STACK_ADO_PROC			8192
#define THREAD_STACK_COMM_TX_VDO	    2176
#define THREAD_STACK_COMM_RX_VDO	    1536
#define THREAD_STACK_COMM_RX_ADO	    1536
#define THREAD_STACK_IMG_MONIT		    1024
#define THREAD_STACK_APP_HANDLER		16384
#define THREAD_STACK_PAIRING_HANDLER	512
#define THREAD_STACK_KNLRECORD_HANDLER	1024
#define THREAD_STACK_JPEG_MONIT			2048
#define THREAD_STACK_LINK_MONIT			1024
#define THREAD_STACK_RC_MONIT			2048
#define THREAD_STACK_RC_SYS_MONIT		2048
#define THREAD_STACK_LINK_UPDATE		1024

#define THREAD_STACK_SEC_MONIT			2048
#define THREAD_STACK_USB_MONIT			2048
#define THREAD_STACK_USB_CMD_MONIT		4096
#define THREAD_STACK_USB_RESET			2048

#define THREAD_STACK_TIME_STAMP         1024
#define THREAD_STACK_TX_REC             2048
#define THREAD_STACK_SYNCGOP            256
#define THREAD_STACK_SYS_MONIT			1024
#define THREAD_STACK_KNL_SD_HANDLER     2048
#define THREAD_STACK_KNL_EMERGENCY_HANDLER     2048
#define THREAD_STACK_TEST_HANDLER	    8192
#define THREAD_STACK_UI_SIGNAL_HANDLER	512
#define THREAD_STACK_RTC_MONIT			1024

//! APP Event Definition
#define APP_REFRESH_EVENT				0
#define APP_PWRCTRL_EVENT				1
#define APP_LINK_EVENT					2
#define APP_LOSTLINK_EVENT				3
#define APP_PAIRING_START_EVENT			4
#define APP_PAIRING_STOP_EVENT			5
#define APP_PAIRING_SUCCESS_EVENT		6
#define APP_PAIRING_FAIL_EVENT			7
#define APP_LINKSTATUS_REPORT_EVENT		8
#define APP_UNBIND_CAM_EVENT			9
#define APP_VIEWTYPECHG_EVENT			10
#define APP_ADOSRCSEL_EVENT				11
#define APP_PTT_EVENT					12
#define APP_POWERSAVE_EVENT				13
#define APP_TRXBWSW_EVENT				14
#define APP_DP_KEY_EVENT			    15
#define APP_DP_EVENT			        16
//20210122
#define APP_H_VIEW_POSIZION_EVENT		17
#define APP_BACKLIGHT_EVENT		        18
#define APP_VOLUME_EVENT		        19
#define APP_WAKEUPTX_EVENT		        20
#define APP_SLEEPTX_EVENT		        21
#define APP_SAVE_PARAM_EVENT		    22
//20211207
#define APP_DISCONNECT_EVENT		    23
//20220510
#define APP_SETBANDWIDTH_EVENT		    24
//20230613
#define APP_SYNCTIMESTAMP_EVENT		    25



//------------------------------------------------------------------------------
//! APP Queue message type
typedef struct
{
	uint8_t ubAPP_Event;
	uint8_t ubAPP_Message[7];
}APP_EventMsg_t;

//------------------------------------------------------------------------------
//! APP Serial Flash Start Sector
#define PAIR_SF_START_SECTOR			2
#define UI_SF_START_SECTOR				6		//! Greater than to 2
#define KNL_SF_START_SECTOR				7		//! Greater than to 2

#define	SF_AP_UI_SECTOR_TAG 			"SNCC71_UI"
#define	SF_STA_UI_SECTOR_TAG 			"SNCC70_UI"

#define	SF_AP_KNL_SECTOR_TAG 			"SNCC71_KNL"
#define	SF_STA_KNL_SECTOR_TAG 			"SNCC70_KNL"

//------------------------------------------------------------------------------
//! APP FWU Volume label
#ifdef BUC_CAM
#define SNCCXX_VOLUME_LABLE				"SNCC70CAM"
#endif
#ifdef BUC_CU
#define SNCCXX_VOLUME_LABLE				"SNCC71CU"
#endif

//------------------------------------------------------------------------------
#include "APP_CFG_SEL.h"
//------------------------------------------------------------------------------
//#if (defined(S2019A) && (DISPLAY_4T1R == DISPLAY_MODE))
//#error S2019A: 4T1R mode not yet supported!
//#endif	
#if (	( DISPLAY_4T1R == DISPLAY_MODE ) &&\
											(	defined(BSP_D_SNCC71_TP2915_AHD_RX_V3) ||\
												defined(BSP_D_SNCC71_NVP6021_AHD_RX_V4)	)	)
	#error AHD Out: 4T1R mode not yet supported!	
#endif
#if (	( DISPLAY_2T1R == DISPLAY_MODE ) &&\
											(	defined(BSP_D_SNCC71_TP2915_AHD_RX_V3) ||\
												defined(BSP_D_SNCC71_NVP6021_AHD_RX_V4)	)	)
	#error AHD Out: 2T1R mode not yet supported!	
#endif
#if (!APP_SD_FUNC_ENABLE && APP_PHOTOGRAPH_FUNC_ENABLE && APP_PHOTO_STORE_SEL == APP_FS_MEDIA_TYPE_SD)
	#error No SD Card: Photo cannot be stored in SD Card!	
#endif
#if (APP_DUAL_HOST_ENABLE && APP_SD_FUNC_ENABLE)
	#error The dual UVC host case no supports recording!	
#endif
#if (defined(BSP_SN93711_FHD_REC_RX_V4) && (BSP_DDRSIZE == 32) && APP_REC_FUNC_ENABLE)
	#error Record function no support.
#endif


#endif
