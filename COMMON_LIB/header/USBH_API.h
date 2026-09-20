#ifndef _USBH_API_H_
#define _USBH_API_H_

#include "_510PF.h"

enum {
	SLICE_PIC	= 1,
	SLICE_PA,
	SLICE_PB,
	SLICE_PC,
	SLICE_IDR_PIC,
	SEI,
	SPS,
	PPS,
};

enum {
	TYPE_ERROR = -2,
	NOT_SLICE,
	TYPE_P,
	TYPE_B,
	TYPE_I,
	TYPE_SP,
	TYPE_SI
};

typedef struct bits_handler {
	const char *ptr, *base;
	unsigned length;
	int index;
} bits_handler;

typedef struct
{
	uint32_t	stream_id;
	uint32_t 	*ptr;
	uint32_t 	size;
	uint32_t 	framecnt;
	uint32_t	errcnt;
	uint32_t	babblecnt;
	uint32_t	underflowcnt;
	uint32_t	discardcnt;
	char		fmt[8];
	char		width[8];
	char		height[8];	
	char		fps[8];
	uint32_t	debug_msg:1;
	uint32_t	sd_record:1;
	uint32_t	usb_preview:1;
	uint32_t	lcm_preview:1;
	uint32_t	isH264:1;
	uint32_t	isIFrame:1;
	uint32_t	isOpen:1;
	uint32_t	reserve:25;
	
}USBH_UVC_APP_STREAM_STRUCTURE;

typedef struct
{  
	//USBH_UVC_APP_STREAM_STRUCTURE	stream[max_stream_count];
	USBH_UVC_APP_STREAM_STRUCTURE	stream[2];
	
	//Wayne 2020.03.26
	uint8_t     isUVCDevReady;
}USBH_UVC_APP_DEV_STRUCTURE;

typedef struct
{
	uint32_t 			sd_record_enable;	
	uint32_t 			usb_preview_enable;		
	uint32_t 			lcm_preview_enable;	
	uint32_t 			debug_enable;
	//uint32_t 			err_debug_enable;
#if(_SUPPORT_USBD_WIFI_PREVIEW)
	int 					socket_fd;
#endif	
	//USBH_UVC_APP_DEV_STRUCTURE	dev[USBH_MAX_PORT*2];
	USBH_UVC_APP_DEV_STRUCTURE	dev[5*2];
	uint32_t *uvc_buf_addr[5*2];
	
}USBH_UVC_APP_STRUCTURE;

extern USBH_UVC_APP_STRUCTURE usbh_uvc_app;

#define USBH_UVC1_DEVICE_ID			0
#define USBH_UVC2_DEVICE_ID			5
#define USBH_FuncEnable									\
						{								\
							USB_PHY1->PHY_PWR = 1;		\
							USB_PHY2->PHY_PWR = 1;		\
							GLB->UH1_FUNC_DIS = 0;		\
							GLB->UH2_FUNC_DIS = 0;		\
						}
#define USBH_FuncDisable								\
						{								\
							USB_PHY1->PHY_PWR = 0;		\
							USB_PHY2->PHY_PWR = 0;		\
							GLB->UH1_FUNC_DIS = 1;		\
							GLB->UH2_FUNC_DIS = 1;		\
						}
typedef	enum
{
	USBH_UVC_NONE = 0,
	USBH_UVC_YUV,
	USBH_UVC_MJPEG,
	USBH_UVC_H264
}USBH_UVC_FRAME_FMT;

typedef	enum
{
	USBH_UVC_1920x1080 = 0,
	USBH_UVC_1280x720
}USBH_UVC_FRAME_RESOLUTION;

typedef	enum
{
	USBH_UVC_FPS30 = 0
}USBH_UVC_FRAME_FPS;

typedef enum _USBH_PLUG_STATUS
{
    USBH_PLUG_OUT,
    USBH_PLUG_IN
}USBH_PLUG_STATUS;

typedef struct _USBH_UVC_FRAME_INFO
{
    USBH_UVC_FRAME_FMT fmt[8];
    uint16_t ulWidth[8];
    uint16_t ulHeight[8];
}USBH_UVC_FRAME_INFO;

typedef struct _USBH_UVC_QUEUE_DATA_INFO
{
    uint32_t ulStartAddr;
    uint32_t ulSize;
}USBH_UVC_QUEUE_DATA;

extern osMessageQId USBH_Q_Ext;

uint32_t ulUSBH_GetBufferSize(uint8_t bIsEnableWIFI, uint8_t bIsEnableDualHost, uint8_t stream_num, USBH_UVC_FRAME_INFO uvc_frame_info);
void USBH_Init(uint32_t ulBufferStrAddr);
uint8_t ubUSBH_UVC_GetInfo(void);
USBH_PLUG_STATUS USB_GetPlugStatus(void);

uint8_t ubUSBH_UVC_Start(uint8_t DeviceId, USBH_UVC_FRAME_FMT Fmt, USBH_UVC_FRAME_RESOLUTION Res, USBH_UVC_FRAME_FPS Fps);

void ulUSBH_MscInit(uint8_t DeviceId);
uint32_t ulUSBH_GetMscCapacity(void);
uint8_t ubUSBH_Msc_Write(uint32_t ulSrcAddr, uint32_t ulLba, uint32_t ulSize);
uint8_t ubUSBH_Msc_Read(uint32_t ulDesAddr, uint32_t ulLba, uint32_t ulSize);


typedef void (*pvUSBH_GetFramInfoCbFunc)(uint8_t ubStreamId,uint8_t ubFrameType,uint32_t ulAddr,uint32_t ulSize); 
void USBH_SetFrameInfoCbFunc(pvUSBH_GetFramInfoCbFunc pFunc);
typedef void (*pvUSBH_UvcRdyInfoCbFunc)(uint8_t ubStreamId); 
typedef void (*pvUSBH_UvcErrInfoCbFunc)(uint8_t ubStreamId);
typedef void (*pvUSBH_CxToInfoCbFunc)(void);

void USBH_SetUvcRdyInfoCbFunc(pvUSBH_UvcRdyInfoCbFunc pFunc);
void USBH_SetUvcErrInfoCbFunc(pvUSBH_UvcErrInfoCbFunc pFunc);
void USBH_SetCxToInfoCbFunc(pvUSBH_CxToInfoCbFunc pFunc);

typedef void (*pvUSBH_SetUvcChkFlgCbFunc)(uint8_t ubIdx,uint8_t ubFlg); 
void USBH_SetUvcChkFlgCbFunc(pvUSBH_SetUvcChkFlgCbFunc pFunc);

void UVC_CleanBkBuf(void);

extern pvUSBH_SetUvcChkFlgCbFunc pUSBH_SetUvcChkFlgCbFunc;

int32_t Cmd_uvc_init(int argc, char* argv[]);
int32_t Cmd_uvc_start(int argc, char* argv[]);
int32_t Cmd_uvc_get_info(int argc, char* argv[]);		
int32_t Cmd_uvc_start(int argc, char* argv[]);
int32_t Cmd_uvc_stop(int argc, char* argv[]);

uint8_t uvc_XU_write(uint8_t device_id, uint8_t Len, uint8_t *buf);
uint8_t uvc_XU_read(uint8_t device_id, uint8_t ubWriteLen , uint8_t ubReadLen, uint8_t *buf);

int8_t USBH_ChkDeviceRdy(uint32_t device_id);
#endif
