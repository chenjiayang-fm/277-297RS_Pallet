/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		FWU_API.h
	\brief		Firmware upgrade function header file
	\author		Hanyi Chiu
	\version	1.37
	\date		2023/04/13
	\copyright	Copyright (C) 2020 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _FWU_API_H_
#define _FWU_API_H_

#include <stdint.h>

//! SN937XX Firmware Version
//#define SN937XX_FW_VERSION				"01.00.01.056"
#if HDW277RS_000000
#define SN937XX_FW_VERSION				"25.04.08.277"
#elif HDW297RS_000000
#define SN937XX_FW_VERSION				"26.06.23.297"
#endif

typedef enum
{
	FWU_USBDMSC,
	FWU_SDCARD,
	FWU_DISABLE,
}FWU_MODE_t;

typedef enum
{
	FWU_UPG_FAIL 		= 120,
	FWU_UPG_DEVTAG_FAIL,
	FWU_UPG_SUCCESS,
	FWU_UPG_INPROGRESS,
}FWU_UpgResult_t;

typedef enum
{
	FWUV_R,
	FWUV_W,
}FWU_VendorCmdAct_t;

typedef enum
{
	SFRW_CMD   = 0x20,
	REGRW_CMD,
	MEMRW_CMD,
	RFENG_CMD,
}FWU_VendorCmd_t;


typedef enum {
	FW_OTA_TYPE_IMG_HS		= 0x0001,
	FW_OTA_TYPE_PROFILE		= 0x0002,
	FW_OTA_TYPE_IQ			= 0x0004,
	//FW_OTA_UPDT_LOGO 		= 0x0008,	
	//FW_OTA_UPDT_OSD_FONT 	= 0x0010,	
	//FW_OTA_UPDT_OSD_IMG 	= 0x0020,
	
	FW_OTA_TYPE_MASK		= (FW_OTA_TYPE_IMG_HS|FW_OTA_TYPE_PROFILE|FW_OTA_TYPE_IQ)
} E_FW_OTA_UPDT_TYPE;

typedef enum {
	FW_OTA_UPDT_SUCCESS = 0,
	FW_OTA_UPDT_FAIL_CRC_ERR,
	FW_OTA_UPDT_FAIL_FS_ERR,
	FW_OTA_UPDT_FAIL_IMG_ERR,
	FW_OTA_UPDT_FAIL_TAG_ERR,	
	FW_OTA_UPDT_FAIL_TYPE_ERR,	
	FW_OTA_UPDT_FAIL_PROFILE_ERR,	
	FW_OTA_UPDT_FAIL_IQ_ERR,	
	FW_OTA_UPDT_FAIL_SIZE_IMG_HS_ERR,	
	FW_OTA_UPDT_FAIL_SIZE_PROFILE_ERR,	
	FW_OTA_UPDT_FAIL_SIZE_IQ_ERR,	
	FW_OTA_UPDT_FAIL_TOTAL_SIZE_ERR,	
	FW_OTA_UPDT_FAIL_WIFI_LOST_ERR,
	FW_OTA_UPDT_FAIL_SAMEVER_NOUPDATE,		
	FW_OTA_UPDT_FAIL_BLE_ERR,	

	FW_OTA_UPDT_FAIL_APP_ERR = 20,	
	
} E_FW_OTA_UPDT_RSLT;

typedef void (*pvFWU_StsRptFunc)(uint8_t);
typedef void (*pvFWU_VendorCmdFunc)(uint8_t ubAct, uint8_t *pCDB, uint8_t *pBuf);
typedef void(*pvFWU_ProcCtrl)(void);

typedef struct
{
	char cVolumeLable[11];
	char cFileName[8];					//! Length must be less than or equal to 8.(File system only support 8.3 filename)
	char cFileNameExt[3];				//! Length must be less than or equal to 3.(File system only support 8.3 filename)
	pvFWU_StsRptFunc pStsRptCbFunc;
	pvFWU_VendorCmdFunc pVendorCmdCbFunc;
}FWU_MSCParam_t;

typedef struct
{
	char cTargetFileName[32];
	uint8_t ubTargetFileNameLen;
	pvFWU_StsRptFunc pStsRptCbFunc;
	uint8_t ubIncrementProgressBy;		//! Progress bar scale, Unit: percent
}FWU_SDParam_t;

typedef void (*pvFWU_CbFunc)(void);
//------------------------------------------------------------------------------
/*!
\brief Firmware upgrade initialization
\param pvCB	Callback Function
\return (no)
*/
void FWU_Init(void);
//------------------------------------------------------------------------------
/*!
\brief Firmware upgrade register callback 
\param pvCB	Callback Function
\return (no)
*/
void FWU_RegCb(pvFWU_CbFunc pvCB);
//------------------------------------------------------------------------------
/*!
\brief Setup Firmware upgrade parameter.
\param FWU_MODE_t		Setup upgrade mode.
\param pvModeParam		Setup upgrade mode parameter setup.
\return (no)
*/
void FWU_Setup(FWU_MODE_t tModeSel, void *pvModeParam);
//------------------------------------------------------------------------------
/*!
\brief Enable firmware upgrade function
\return (no)
*/
void FWU_Enable(void);
//------------------------------------------------------------------------------
/*!
\brief Disable firmware upgrade function
\return (no)
*/
void FWU_Disable(void);
//------------------------------------------------------------------------
/*!
\brief Start upgrade function use SD card
\param ulBUF_StartAddr	Buffer start address
\return upgrade result
*/
FWU_UpgResult_t FWU_SdUpgradeStart(uint32_t ulBUF_StartAddr);
//------------------------------------------------------------------------
/*!
\brief Enable check firmware content for model name or device tag.
\param ubEnChkModelName	Enable or disable check fucntion of model name.
\param ubEnChkDevTag	Enable or disable check fucntion of device tag.
\return (no)
*/
void FWU_EnChkUpgFwFunc(uint8_t ubEnChkModelName, uint8_t ubEnChkDevTag);
void FWU_SetProcCtrlCbFunc(pvFWU_ProcCtrl tProcCtrl);
//------------------------------------------------------------------------
/*!
\brief 	Get FWU Version	
\return	Version
*/
uint16_t uwFWU_GetVersion(void);
//------------------------------------------------------------------------
/*!
\brief Firmware upgrade mass storage write 
\param pBf          source address    
\param ulLBA        MSC LBA(base on 512bytes)
\param ulLBA_Cnt    MSC size(base on 512bytes)
\return (no)
*/
void FWU_MscSectorWrite(uint8_t *pBf, uint32_t ulLBA, uint32_t ulLBA_Cnt);
//------------------------------------------------------------------------
/*!
\brief Firmware upgrade mass storage read  
\param pBf          destination address    
\param ulLBA        MSC LBA(base on 512bytes)
\param ulLBA_Cnt    MSC size(base on 512bytes)
\return (no)
*/
void FWU_MscSectorRead(uint8_t *pBf, uint32_t ulLBA, uint32_t ulLBA_Cnt);
//------------------------------------------------------------------------
/*!
\brief Start upgrade function use SD card
\param ulBUF_StartAddr	Buffer start address
\return upgrade status
*/
E_FW_OTA_UPDT_RSLT ulFOTA_UpdateFW(uint8_t ubType, uint8_t *fwImgBuf);

#endif
