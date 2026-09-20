/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		USBD_API.h
	\brief		USB Device Config header file
	\author		Hanyi Chiu
	\version	1.18
	\date		2022/07/28
	\copyright	Copyright(C) 2022 SONiX Technology Co.,Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _USBD_API_H_
#define _USBD_API_H_

#include "_510PF.h"

#define USBD_ENABLE				TRUE

#define USBD_FuncEnable			(GLB->UD_FUNC_DIS = 0)
#define USBD_FuncDisable		(GLB->UD_FUNC_DIS = 1)

#define USBD_UVC_HEADER_LEN		12
#define USBD_UVC_DUMMY_SZ		30

typedef enum
{
	USBD_UVC_MODE = 1,
	USBD_MSC_MODE,
	USBD_UAC_MODE,
	USBD_COMPOSITE_MODE,
	USBD_FWU_MODE,
	USBD_UNKNOWN_MODE,
}USBD_ClassMode_t;

#define USBD_UVC_MJ_FMT			1
#define USBD_UVC_H264_FMT		2
#define USBD_UVC_H265_FMT		3

#ifdef OP_AP
#define USBD_DEFU_CLASS			 USBD_COMPOSITE_MODE//USBD_COMPOSITE_MODE(uvc enable) USBD_MSC_MODE
	#if ((USBD_DEFU_CLASS == USBD_UVC_MODE) || (USBD_DEFU_CLASS == USBD_COMPOSITE_MODE))
		#define USBD_UVC_FMT	USBD_UVC_MJ_FMT
	#endif
#endif
#ifdef OP_STA
#define USBD_DEFU_CLASS			USBD_MSC_MODE
	#if ((USBD_DEFU_CLASS == USBD_UVC_MODE) || (USBD_DEFU_CLASS == USBD_COMPOSITE_MODE))
		#define USBD_UVC_FMT	USBD_UVC_H264_FMT
	#endif
#endif

typedef enum
{
	USB_UVC_VS_FORMAT_UNDEFINED  	= 0x00,
    USB_UVC_VS_FORMAT_UNCOMPRESSED  = 0x04,
	USB_UVC_VS_FORMAT_MJPEG         = 0x06,
    USB_UVC_VS_FORMAT_FRAME_BASED   = 0x10,
}USBD_ClassType_t;

#define USBD_MULTI_MSC  	0x1
#define USBD_MULTI_UVC  	0x2
#define USBD_MULTI_HID  	0x4
#define USBD_MULTI_UAC  	0x8
#define USBD_MULTI_MSCSPEC  0x10
typedef enum
{
	USBD_MULTI_UVCHID 	  = (USBD_MULTI_UVC | USBD_MULTI_HID),
	USBD_MULTI_UVCMSC 	  = (USBD_MULTI_UVC | USBD_MULTI_MSC),
	USBD_MULTI_UVCMSCSPEC = (USBD_MULTI_UVC | USBD_MULTI_MSCSPEC),
	USBD_MULTI_UVCUAC 	  = (USBD_MULTI_UVC | USBD_MULTI_UAC),
}USBD_MultiMd_t;

typedef enum
{
	USBD_SUCCESS	= 0,
	USBD_FAIL,
}USBD_STATUS;

typedef enum
{
    USB_UVC_FHD_WIDTH       = 1920,
	USB_UVC_HD_WIDTH        = 1280,
    USB_UVC_VGA_WIDTH       = 640,
}USBD_ResolutionType_t;

typedef enum
{
	USBD_OPC_TUNNING = 0,
	USBD_OPC_NVR 	 = 5,
	USBD_OPC_MAX
}USBD_XuOpc_t;

typedef enum
{
	USBDM_OPC_ADO = 0,
    USBDM_OPC_FWUW,
    USBDM_OPC_FWUR,
	USBDM_OPC_MSCW,
    USBDM_OPC_MSCR,
	USBDM_OPC_MAX,
}USBD_MscOpc_t;

#define USBD_IMG_INF_MAX			25					// Defin by USBD Libuary UVC_MAX_FPS_NUM
struct uvc_image_info {
	uint8_t                         type;
	uint16_t                        width;
	uint16_t                        height;
	uint32_t                        imagesize;
	uint8_t                         fps_num;
	uint8_t                         fps[6];				// Defin by USBD Libuary UVC_MAX_FPS_NUM
	uint32_t                        payload_size[6];	// Defin by USBD Libuary UVC_MAX_FPS_NUM

}__attribute__ ((packed));


typedef void(*pvUsbdXuCbFunc)(uint8_t, uint32_t *);
typedef void(*pvUsbdMscCbFunc)(uint8_t *, uint32_t, uint32_t);
typedef uint8_t(*pvUsbdMscCbFunc_MediaDetect)(void);
typedef uint32_t(*pvUsbdMscCbFunc_MediaCapaticy)(void);
typedef void(*pvUvcReleaseBufCbFunc)(uint32_t);

typedef void (*pvUsbdReusmeCbFunc)(void);
typedef void (*pvUsbdSuspendCbFunc)(void);
typedef void(*pvUvcSetImgInfCbFunc)(uint16_t, uint16_t);
typedef void(*pvUvcSetStillImgInfCbFunc)(uint16_t, uint16_t);
typedef void(*pvUvcCtProcessCbFunc)(uint8_t, uint8_t, uint8_t *);
typedef void(*pvUvcPuProcessCbFunc)(uint8_t, uint8_t, uint8_t *);
typedef void (*pvUvcImgInfoCbFunc)(uint8_t *);
typedef void (*pvUsbdSetConfigurationCbFunc)(void);

typedef int (*usbd_drv_uac_start_record_cb_t)(uint32_t SamplesPerSec);
typedef void (*usbd_drv_uac_stop_record_cb_t)(void);
typedef int (*usbd_drv_uac_ctrl_volume_cb_t)(unsigned char channel, unsigned char ctrl, unsigned short *volume);
typedef int (*usbd_drv_uac_ctrl_mute_cb_t)(unsigned char channel, unsigned char ctrl, unsigned char *enable);
//------------------------------------------------------------------------------
/*!
\brief USB Device initial
\param tClassMode 		USB device class mode
\return(no)
*/
void USBD_Init(USBD_ClassMode_t tClassMode);
//------------------------------------------------------------------------------
/*!
\brief USB Device uninitial
\return(no)
*/
void USBD_UnInit(void);
//------------------------------------------------------------------------------
/*!
\brief USB Device Start
\return(no)
*/
void USBD_Start(void);
//------------------------------------------------------------------------------
/*!
\brief Get USB device config status
\return Config result
\par Note:
		USBD_SUCCESS: USB device enumeration success.
		USBD_FAIL   : USB device enumeration fail.
*/
USBD_STATUS tUSBD_GetConfigStatus(void);
//------------------------------------------------------------------------------
/*!
\brief Get USB Class Mode
\return Class mode
*/
USBD_ClassMode_t tUSBD_GetClassMode(void);
//------------------------------------------------------------------------------
/*!
\brief Buffer setup for USB Device
\param ulBUF_StartAddr 	Available memory address of DDR
\return USBD Buffer Size
*/
uint32_t USBD_BufSetup(uint32_t ulBUF_StartAddr);
//------------------------------------------------------------------------------
/*!
\brief Callback function for USBD Reusm
\param pvCb		Callback function
\return (no)
*/
void tUSBD_RegReusmeCbFunc(pvUsbdReusmeCbFunc pvCb);
//------------------------------------------------------------------------------
/*!
\brief Callback function for USBD Suspend
\param pvCb		Callback function
\return (no)
*/
void tUSBD_RegSuspendCbFunc(pvUsbdSuspendCbFunc pvCb);
//------------------------------------------------------------------------------
/*!
\brief Callback function for USBD SetConfiguration
\param pvCb		Callback function
\return (no)
*/
void tUSBD_RegSetConfigurationCbFunc(pvUsbdSetConfigurationCbFunc pvCb);
//------------------------------------------------------------------------------
/*!
\brief Callback function for USBD MSC
\param tOpc 	Operation code
\param pvCb		Callback function
\return status
*/
USBD_STATUS tUSBD_RegMscCbFunc(USBD_MscOpc_t tOpc, pvUsbdMscCbFunc pvCb);
//------------------------------------------------------------------------------
/*!
\brief Callback function for USBD MSC
\param pvCb		Callback function
\return status
*/
USBD_STATUS tUSBD_RegMscCbFunc_MediaDetect(pvUsbdMscCbFunc_MediaDetect pvCb);
//------------------------------------------------------------------------------
/*!
\brief Callback function for USBD MSC
\param pvCb		Callback function
\return status
*/
USBD_STATUS tUSBD_RegMscCbFunc_MediaCapacity(pvUsbdMscCbFunc_MediaCapaticy pvCb);
//------------------------------------------------------------------------------
/*!
\brief Set MSC Capacities Inf
\param ulSz		Capacities 512Byte Unit
\return (no)
*/
void USBD_SetMscCapacities(uint32_t ulSz);
//------------------------------------------------------------------------------
/*!
\brief Callback function for USBD UVC XU
\param tOpc 	Operation code
\param pvCb		Callback function
\return status
*/
USBD_STATUS tUSBD_RegXuCbFunc(USBD_XuOpc_t tOpc, pvUsbdXuCbFunc pvCb);
//------------------------------------------------------------------------------
/*!
\brief Register release buffer callback function for UVC Bulk
\param pvReleaseCb 		Call back function
\return(no)
*/
USBD_STATUS tUSBD_RegUvcReleaseBuffCbFunc(pvUvcReleaseBufCbFunc pvReleaseCb);
//------------------------------------------------------------------------------
/*!
\brief Register Set Image infromation callback function for UVC Bulk
\param pvSetImgInf_cb 		Call back function
\return status
*/
USBD_STATUS tUSBD_UvcSetImgInfCbFunc(pvUvcSetImgInfCbFunc pvSetImgInf_cb);
//------------------------------------------------------------------------------
/*!
\brief Register Set Still Image infromation callback function for UVC Bulk
\param pvSetStillImgInf_cb 		Call back function
\return status
*/
USBD_STATUS tUSBD_UvcSetStillImgInfCbFunc(pvUvcSetStillImgInfCbFunc pvSetStillImgInf_cb);
/*!
\brief Get video format of UVC
\return Video format
*/
uint8_t UVC_GetVdoFormat(void);
//------------------------------------------------------------------------------
/*!
\brief Get video width of UVC
\return Video width
*/
uint16_t UVC_GetVdoWidth(void);
//------------------------------------------------------------------------------
/*!
\brief Get video Height of UVC
\return Video height
*/
uint16_t UVC_GetVdoHeight(void);
//------------------------------------------------------------------------------
/*!
\brief Get video start flag of UVC,read clear
\return start flag 
*/
uint8_t UVC_GetVdoStart(void);
//------------------------------------------------------------------------------
/*!
\brief Check UVC resolution
\return 1: Check OK
        0: Check Fail
*/
uint8_t ubUVC_CheckResolution(uint16_t uwH_Size, uint16_t uwV_Size);

//------------------------------------------------------------------------------
/*!
\brief Check UVC in preview mode
\return 1: in preview
        0: in standby
*/
uint8_t ubUVC_CheckInPreview(void);
//------------------------------------------------------------------------------
/*!
\brief Update video image through UVC path
\param pImg_Buf 		Video Image buffer
\param ulImg_Size 		Video Image Size
\return result
*/
uint8_t uvc_update_image(uint32_t *pImg_Buf, uint32_t ulImg_Size);
//------------------------------------------------------------------------------
/*!
\brief Set UVC format for user define info @ LCD side
\param ubUvcFmt 		UVC format
\return(no)
*/
void USBD_SetUserDefUvcFmt4LcdSide(uint8_t ubUvcFmt);
//------------------------------------------------------------------------------
/*!
\brief Set UVC format for user define info @ Camera side
\param ubUvcFmt 		UVC format
\return(no)
*/
void USBD_SetUserDefUvcFmt4CamSide(uint8_t ubUvcFmt);
//------------------------------------------------------------------------------
/*!
\brief Set UVC compressed format of frame.
\param ubUvcCompressFmt 		Compressed format
\return(no)
*/
void USBD_SetUvcFrmCompressFmt(uint8_t ubUvcCompressFmt);
//------------------------------------------------------------------------------
/*!
\brief Setup UVC still image capture info
\param ubConfig 		Config UVC still image capture function.
\param uwImgH			Image H size for still image capture.
\param uwImgV			Image V size for still image capture.
\return(no)
*/
void USBD_SetUvcStillImageCaptureInfo(uint8_t ubConfig, uint16_t uwImgH, uint16_t uwImgV);
//------------------------------------------------------------------------------
/*!
\brief Snapshot image use still image capture defined in the UVC specification
\return(no)
*/
void USBD_doSnapShotUseStillImageCapture(void);
//------------------------------------------------------------------------------
/*!
\brief Set PID and product name for UVC class.
\param uwPID 		Product id.
\param pPrdLabel 	Product string.
\return(no)
*/
void USBD_SetUvcDevInfo(uint16_t uwPID, char *pPrdLabel);
//------------------------------------------------------------------------------
/*!
\brief Set PID and string for UAC class.
\param uwPID 			Product id.
\param pFunctionLabel 	Function string.
\param pControlLabel 	Interface string.
\param pIsoInAlt0 		IsoInAlt0 string.
\param pIsoInAlt1 		IsoInAlt1 string.
\param pIsoOutAlt0 		IsoOutAlt0 string.
\param pIsoOutAlt1 		IsoOutAlt1 string.
\return(no)
*/
void USBD_SetUacDevInfo(uint16_t uwPID, char *pFunctionLabel, char *pControlLabel, char *pIsoInAlt0,char *pIsoInAlt1,char *pIsoOutAlt0,char *pIsoOutAlt1);
//------------------------------------------------------------------------------
/*!
\brief Set Device inf for Multi class.
\param uwVID 		Vendor id.
\param uwPID 		Product id.
\param pVendLabel 	Manufacturer string.
\param pPrdLabel 	Product string.
\param pSenbLabel 	SerialNumber string.
\return(no)
*/
void USBD_SetMultiDevInfo(uint16_t uwVID,uint16_t uwPID, char *pVendLabel, char *pPrdLabel, char *pSenbLabel);
//------------------------------------------------------------------------------
/*!
\brief Set multi driver mode for composite class.
\param tMultiMd 		Set multi mode for composite class.
\return(no)
*/
void USBD_SetMultiDrvMode(USBD_MultiMd_t tMultiMd);
//------------------------------------------------------------------------------
/*!
\brief Get multi driver mode of composite class.
\return multi mode
*/
uint8_t ubUSBD_GetMultiDrvMode(void);
uint8_t ubUSBD_ChkCompUvcClass(void);
uint8_t ubUSBD_ChkCompMscClass(void);
uint8_t ubUSBD_ChkCompUvcUacClass(void);
//------------------------------------------------------------------------------
/*!
\brief USBD HID report
\param pData 			Report data buffer point.
\param ubDataLen		Report data length, MAX: 64Bytes.
\return ESUCCESS:report success, EBUSY/EIO:report fail
*/
int iUSBD_HidData(uint8_t *pData, uint8_t ubDataLen);
//------------------------------------------------------------------------------
/*!
\brief 	Get USBD Version	
\return	Version
*/
uint16_t uwUSBD_GetVersion(void);
//------------------------------------------------------------------------------
/*!
\brief Callback function for USBD UAC start record
\param cb		Callback function
\return (no)
*/
void usbd_drv_uac_start_record_reg_cb(usbd_drv_uac_start_record_cb_t cb);
//------------------------------------------------------------------------------
/*!
\brief Callback function for USBD UAC stop record
\param cb		Callback function
\return (no)
*/
void usbd_drv_uac_stop_record_reg_cb(usbd_drv_uac_stop_record_cb_t cb);
//------------------------------------------------------------------------------
/*!
\brief Callback function for USBD UAC volume control
\param cb		Callback function
\return (no)
*/
void usbd_drv_uac_ctrl_volume_reg_cb(usbd_drv_uac_ctrl_volume_cb_t cb);
//------------------------------------------------------------------------------
/*!
\brief Callback function for USBD UAC volume mute control
\param cb		Callback function
\return (no)
*/
void usbd_drv_uac_ctrl_mute_reg_cb(usbd_drv_uac_ctrl_mute_cb_t cb);
//------------------------------------------------------------------------------
/*!
\brief Callback function for USBD UAC do record
\param audio_buf 		Audio Image buffer
\param audio_len 		Audio Image Size
\return (no)
*/
void usbd_uac_drv_do_record(unsigned char *audio_buf, unsigned int audio_len);
//------------------------------------------------------------------------------
/*!
\brief Callback function for USBD UAC do play
\param audio_buf 		Audio Image buffer
\param audio_len 		Audio Image Size
\return (no)
*/
int usbd_uac_drv_do_play(unsigned char *audio_buf, unsigned int audio_len);
//------------------------------------------------------------------------------
/*!
\brief Get IQ File Size
\return IQ File Size
*/
int32_t slUSBD_GetIQFileSize(void);
//------------------------------------------------------------------------------
/*!
\brief Set IQ File Size
\param slSize 		IQ File Size
\return (no)
*/
void USBD_SetIQFileSize(int32_t slSize);

int16_t swUSBD_GetRemainTransferLength(void);

void USBD_SetRemainTransferLength(int16_t swLength);
//------------------------------------------------------------------------------
/*!
\brief Callback function for USBD UVC Ct Process
\param cb		Callback function
\return (no)
*/
USBD_STATUS tUSBD_UvcCtProcessCbFunc(pvUvcCtProcessCbFunc pv_cb);
//------------------------------------------------------------------------------
/*!
\brief Callback function for USBD UVC Pu Process
\param cb		Callback function
\return (no)
*/
USBD_STATUS tUSBD_UvcPuProcessCbFunc(pvUvcPuProcessCbFunc pv_cb);
//------------------------------------------------------------------------------
/*!
\brief Callback function for USBD UVC Set Img Info list
\param cb		Callback function
\return USBD_SUCCESS/USBD_FAIL
*/
USBD_STATUS tUSBD_UvcImgInfolistCbFunc(pvUvcImgInfoCbFunc pv_cb);
#endif
