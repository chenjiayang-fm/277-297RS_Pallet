/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		VDO.h
	\brief		Video Process Header file for BUC
	\author		Hanyi Chiu
	\version	0.10
	\date		2022/03/01
	\copyright	Copyright(C) 2022 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _VDO_H_
#define _VDO_H_

#include "BSP.h"
#include "APP_HS.h"
#include "KNL.h"
#include "BUF.h"
#include "SEN.h"

// Default video mode setting

#define IS_KNL_DISP3T_VIEW(disptype)	((((KNL_DISP_3T_2L1R == disptype) || (KNL_DISP_3T_1L2R == disptype)) ||		\
										  ((KNL_DISP_3T_2T1B == disptype) || (KNL_DISP_3T_1T2B == disptype)) ||		\
										   (KNL_DISP_3T_3COL == disptype))?1:0)

#if APP_DUAL_HOST_ENABLE
//	#define VDO_DISP_TYPE					KNL_DISP_QUAD
	#define VDO_DISP_TYPE					KNL_DISP_3T_3C
#else
	#if (defined(BSP_SN93711_FHD_REC_RX_V4) || defined(BSP_D_SN93701_SSD2828_RX_V5)||defined(BSP_D_SN93701_TC358778_RX_V6)) 
#define VDO_DISP_TYPE					((DISPLAY_MODE == DISPLAY_4T1R)?KNL_DISP_H:(DISPLAY_MODE == DISPLAY_2T1R)?KNL_DISP_DUAL_C:KNL_DISP_SINGLE)
	#else
	#if (defined(APP_USBD_UVCVDO_MODE)&&((APP_USBD_UVCVDO_MODE==1 && APP_UVC_SNXYUV_ONLY == 4)||(APP_USBD_UVCVDO_MODE==2)))
#define VDO_DISP_TYPE					((DISPLAY_MODE == DISPLAY_4T1R)?KNL_DISP_QUAD:(DISPLAY_MODE == DISPLAY_2T1R)?KNL_DISP_DUAL_U:KNL_DISP_SINGLE)
	#else
#define VDO_DISP_TYPE					((DISPLAY_MODE == DISPLAY_4T1R)?KNL_DISP_H:(DISPLAY_MODE == DISPLAY_2T1R)?KNL_DISP_DUAL_U:KNL_DISP_SINGLE)		//Original BUC
	#endif
	#endif
#endif

#ifdef S2019A
	#define VDO_MAIN_H_SIZE(DispType)		(DispType == KNL_DISP_SINGLE)?HD_WIDTH:VGA_WIDTH
	#define VDO_MAIN_V_SIZE(DispType)		(DispType == KNL_DISP_SINGLE)?HD_HEIGHT:VGA_HEIGHT
	#define VDO_DISP_SCAN					FALSE
#else
	#if APP_DUAL_HOST_ENABLE
		#define VDO_MAIN_H_SIZE(DispType)		HD_WIDTH
		#define VDO_MAIN_V_SIZE(DispType)		HD_HEIGHT
	#elif (defined(RTC676x) &&	(	defined(BSP_D_SNCC71_TP2915_AHD_RX_V3) ||\
									(!MIN_DRAM_ENABLE && (SEN_USE == SEN_TP9950 || SEN_USE == SEN_RN6752)) )	)
		#define VDO_MAIN_H_SIZE(DispType)		(((DispType == KNL_DISP_H) || (DispType == KNL_DISP_QUAD) || (IS_KNL_DISP3T_VIEW(DispType)))?VGA_WIDTH:FHD_WIDTH)
		#define VDO_MAIN_V_SIZE(DispType)		(((DispType == KNL_DISP_H) || (DispType == KNL_DISP_QUAD) || (IS_KNL_DISP3T_VIEW(DispType)))?VGA_HEIGHT:FHD_HEIGHT)
	#elif ( defined(RTC676x) && defined(BSP_D_SNCC71_NVP6021_AHD_RX_V4) )
		#define VDO_MAIN_H_SIZE(DispType)		(((DispType == KNL_DISP_H) || (DispType == KNL_DISP_QUAD) || (IS_KNL_DISP3T_VIEW(DispType)))?VGA_WIDTH:HD_WIDTH)
		#define VDO_MAIN_V_SIZE(DispType)		(((DispType == KNL_DISP_H) || (DispType == KNL_DISP_QUAD) || (IS_KNL_DISP3T_VIEW(DispType)))?VGA_HEIGHT:HD_HEIGHT)
	#else
		#if (defined(APP_USBD_UVCVDO_MODE)&&((APP_USBD_UVCVDO_MODE==1 && APP_UVC_SNXYUV_ONLY == 4)||(APP_USBD_UVCVDO_MODE==2)))
		//#define VDO_MAIN_H_SIZE(DispType)		(((DispType == KNL_DISP_DUAL_U) ||(DispType == KNL_DISP_H) || (DispType == KNL_DISP_QUAD) || (IS_KNL_DISP3T_VIEW(DispType)))?VGA_WIDTH:HD_WIDTH)
		//#define VDO_MAIN_V_SIZE(DispType)		(((DispType == KNL_DISP_DUAL_U) ||(DispType == KNL_DISP_H) || (DispType == KNL_DISP_QUAD) || (IS_KNL_DISP3T_VIEW(DispType)))?VGA_HEIGHT:HD_HEIGHT)
		#define VDO_MAIN_H_SIZE(DispType)		(((DispType == KNL_DISP_H) || (DispType == KNL_DISP_QUAD) || (IS_KNL_DISP3T_VIEW(DispType)))?VGA_WIDTH:HD_WIDTH)
		#define VDO_MAIN_V_SIZE(DispType)		(((DispType == KNL_DISP_H) || (DispType == KNL_DISP_QUAD) || (IS_KNL_DISP3T_VIEW(DispType)))?VGA_HEIGHT:HD_HEIGHT)		
		#else
		#define VDO_MAIN_H_SIZE(DispType)		(((DispType == KNL_DISP_H) || (DispType == KNL_DISP_QUAD) || (IS_KNL_DISP3T_VIEW(DispType)))?VGA_WIDTH:HD_WIDTH)
		#define VDO_MAIN_V_SIZE(DispType)		(((DispType == KNL_DISP_H) || (DispType == KNL_DISP_QUAD) || (IS_KNL_DISP3T_VIEW(DispType)))?VGA_HEIGHT:HD_HEIGHT)
		#endif
	#endif
	#define VDO_DISP_SCAN					FALSE
#endif	//! End of #ifdef S2019A

#define VDO_SUB_H_SIZE					VGA_WIDTH
#define VDO_SUB_V_SIZE					VGA_HEIGHT

#define VDO_PATH_KEEP_ACTIVITYCAM		1
#define VDO_PATH_ONLY_LIVECAM			2
#define VDO_DATAPATH_MODE				VDO_PATH_ONLY_LIVECAM

#ifdef S2019A
	#define VGA_RES_FPS						25
	#define HD_RES1T_FPS					25
	#define HD_RES2T_FPS					20
	#define VDO_FRAME_RATE(H, V)			((H == VGA_WIDTH) && (V == VGA_HEIGHT))?VGA_RES_FPS:((DISPLAY_MODE == DISPLAY_1T1R)?HD_RES1T_FPS:HD_RES2T_FPS)
#else
	#ifdef RTC676x
		#define VGA_RES_FPS						25
		#define HD_RES_FPS						20
	#else
		#define VGA_RES_FPS						20
		#define HD_RES_FPS						15
	#endif	//! End of #ifdef RTC676x
	#define VDO_FRAME_RATE(H, V)			((H == VGA_WIDTH) && (V == VGA_HEIGHT))?VGA_RES_FPS:HD_RES_FPS
#endif	//! End of #ifdef S2019A


#ifdef BUC_CAM

#define VDO_SUBPATH_ENABLE				0
#define KNL_SenorSetup(KNL_MainSrcNum, KNL_SubSrcNum)																							\
										{																										\
											SEN_SetPathSrc(KNL_MainSrcNum, KNL_SRC_NONE, KNL_SubSrcNum);										\
											SEN_SetOutResolution(SENSOR_PATH1, VDO_MAIN_H_SIZE(VDO_DISP_TYPE), VDO_MAIN_V_SIZE(VDO_DISP_TYPE));	\
											SEN_SetOutResolution(SENSOR_PATH3, VDO_SUB_H_SIZE,  VDO_SUB_V_SIZE);								\
										}
#define KNL_SensorStartProcess()		ubSEN_InitProcess();
#define KNL_VDO_SCALE					KNL_SCALE_X1

#endif	//! End #ifdef BUC_CAM
#define KNL_CamIsCvbsCheck()			ubKNL_CheckCvbs();

#ifdef BUC_CU

#if (defined(BSP_SN93711_FHD_REC_RX_V4)||defined(BSP_D_SN93701_SSD2828_RX_V5)||defined(BSP_D_SN93701_TC358778_RX_V6)||defined(BSP_D_SNCC70_TX_V2)||defined(BSP_D_SN93714_TX_V1))
#define LCD_H_SIZE						HD_WIDTH
#define LCD_V_SIZE						HD_HEIGHT
#endif

#ifdef BSP_D_SNCC71_TP2915_AHD_RX_V3
#define LCD_H_SIZE						FHD_WIDTH
#define LCD_V_SIZE						FHD_HEIGHT
#endif

#ifdef BSP_D_SNCC71_NVP6021_AHD_RX_V4
#define LCD_H_SIZE						FHD_WIDTH
#define LCD_V_SIZE						FHD_HEIGHT
#endif				

#ifdef BSP_D_SNCC71_GM8285C_RX_V2
#define LCD_H_SIZE						WSVGA_WIDTH
#define LCD_V_SIZE						WSVGA_HEIGHT
#endif

#if (defined(BSP_SN93711_FHD_REC_RX_V4)||defined(BSP_D_SN93701_SSD2828_RX_V5)||defined(BSP_D_SN93701_TC358778_RX_V6)||defined(BSP_D_SNCC70_TX_V2)||defined(BSP_D_SN93714_TX_V1))
#define	KNL_DISP_ROTATE_SETTING			(KNL_DISP_ROTATE_90)
#define KNL_VDO_SCALE					((KNL_DISP_H == VDO_DISP_TYPE)||(KNL_DISP_3T_3C == VDO_DISP_TYPE))?(VDO_MAIN_H_SIZE(VDO_DISP_TYPE) > 1024)?KNL_SCALE_X0P5:KNL_SCALE_X1:KNL_SCALE_X1
#endif
#if (	defined(BSP_D_SNCC71_GM8285C_RX_V2) ||\
		defined(BSP_D_SNCC71_TP2915_AHD_RX_V3) ||\
		defined(BSP_D_SNCC71_NVP6021_AHD_RX_V4)		)
#define KNL_DISP_ROTATE_SETTING			(KNL_DISP_ROTATE_0)
#define KNL_VDO_SCALE					(KNL_DISP_H == VDO_DISP_TYPE)?(VDO_MAIN_H_SIZE(VDO_DISP_TYPE) > 1024)?KNL_SCALE_X0P25:KNL_SCALE_X0P5:KNL_SCALE_X1
#endif

#define KNL_VdoDisplaySetting()																				\
										{																	\
											KNL_SetDispType(VDO_DISP_TYPE);									\
											KNL_SetDispHV(LCD_H_SIZE, LCD_V_SIZE);							\
											KNL_SetDispRotate(KNL_DISP_ROTATE_SETTING);						\
										}
#define KNL_VdoDisplayParamUpdate()		ubKNL_SetDispCropScaleParam();

#endif	//! End #ifdef BUC_CU

#define KNL_SetVdoResolution(KNL_SrcNum, H_SIZE, V_SIZE)													\
										{																	\
											KNL_SetVdoH(KNL_SrcNum, H_SIZE);								\
											KNL_SetVdoV(KNL_SrcNum, V_SIZE);								\
										}
#define KNL_BufSetup()																						\
										{																	\
											BUF_ResetFreeAddr();											\
											printf("%s %s %d call KNL_BufInit====\n", __FILE__, __func__, __LINE__);\
											KNL_BufInit();													\
										}

typedef enum
{
	VDO_STOP,
	VDO_START,
}VDO_PlayState_t;

typedef enum
{
	VDO_MAIN_SRC,
	VDO_SUB_SRC,
	VDO_AUX_SRC,
	VDO_SRC_MAX,
}VDO_SrcType_t;

typedef enum
{
	VDO_1TSLOT = 1,
	VDO_2TSLOT = 2,
	VDO_4TSLOT = 4,
}VDO_TrxSlotNum_t;

typedef enum
{
	VDO_HD_MODE,
	VDO_SD_MODE,
}VDO_DisplayMode_t;

typedef struct
{
	KNL_SRC		tKNL_SrcNum;
	uint8_t		ubVDO_CodecIdx;
}VDO_KNLRoleParam_t;

typedef struct
{
	VDO_KNLRoleParam_t tVDO_KNLParam[VDO_SRC_MAX];
}VDO_KNLRoleInfo_t;

typedef struct
{
	VDO_PlayState_t   tVdoPlaySte[6];
	KNL_DISP_TYPE	  tVdoDispType;
}VDO_Status_t;

typedef struct
{
	void (*VDO_tPsFunPtr)(void);
}VDO_PsFuncPtr_t;

void VDO_Init(void);
void VDO_Setup(void);
void VDO_Start(void);
void VDO_Stop(void);

#ifdef BUC_CAM
void VDO_KNLSysInfoSetup(KNL_ROLE tVDO_KNLRole);
#endif

#ifdef BUC_CU
void VDO_UpdateDisplayParameter(void);
void VDO_DisplayLocationSetup(KNL_ROLE tVDO_CamRole, KNL_DISP_LOCATION tVDO_DispLocation);
void VDO_SwitchDisplayType(KNL_DISP_TYPE tVDO_DisplayType, KNL_ROLE *pVDO_CamRole);
void VDO_RestartPreview(KNL_DISP_TYPE tVDO_DisplayType, KNL_ROLE *pVDO_CamRole);
void VDO_RemoveDataPath(KNL_ROLE tVDO_CamRole);
void VDO_SetStreamSlotNum(uint8_t ubSlotNum);
void VDO_SetFrameColor(uint8_t ubRed, uint8_t ubGreen, uint8_t ubBlue);
void VDO_FillFrameBuf(uint8_t ubCamNum);
#endif

void VDO_DataPathSetup(KNL_ROLE tVDO_KNLRole, VDO_SrcType_t tVDO_SrcType);
void VDO_ChangePlayState(KNL_ROLE tVDO_KNLRole, VDO_PlayState_t tVdoPlySte);
KNL_SRC VDO_GetSourceNumber(KNL_VA_DATAPATH tVDO_Path, KNL_ROLE tVDO_KNLRole);
KNL_ROLE VDO_KNLSrcNumMap2KNLRoleNum(KNL_SRC tVDO_SrcNum);
void VDO_SwitchAhdType (KNL_DISP_TYPE tVDO_DisplayType, KNL_ROLE *pVDO_CamRole,
						uint16_t uwHsize, uint16_t uwVsize);
void VDO_KNLVdoRes (KNL_ROLE tVDO_KNLRole);
KNL_DISP_TYPE VDO_GetDispType(void);


#endif //! #ifndef _VDO_H_
