/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		VDO.c
	\brief		Video Process for BUC
	\author		Hanyi Chiu
	\version	0.10
	\date		2022/03/01
	\copyright	Copyright(C) 2022 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------

#if defined(BUC_CU) || defined(BUC_CAM)

#include "VDO.h"
#include "LCD.h"

const static VDO_KNLRoleInfo_t tVDO_KNLRoleInfo[] = 
{
#ifdef BUC_CAM
	[KNL_STA1] = { { { KNL_SRC_1_MAIN, ENCODE_0 }, { KNL_SRC_1_AUX, ENCODE_1 } } },
	[KNL_STA2] = { { { KNL_SRC_2_MAIN, ENCODE_0 }, { KNL_SRC_2_AUX, ENCODE_1 } } },
	[KNL_STA3] = { { { KNL_SRC_3_MAIN, ENCODE_0 }, { KNL_SRC_3_AUX, ENCODE_1 } } },
	[KNL_STA4] = { { { KNL_SRC_4_MAIN, ENCODE_0 }, { KNL_SRC_4_AUX, ENCODE_1 } } },
	[KNL_NONE] = { { { KNL_SRC_NONE,   ENCODE_0 }, { KNL_SRC_NONE,  ENCODE_1 } } },
#endif
#ifdef BUC_CU
	[KNL_STA1] = { { { KNL_SRC_1_MAIN, DECODE_0 }, { KNL_SRC_1_AUX, DECODE_0 } } },
	[KNL_STA2] = { { { KNL_SRC_2_MAIN, DECODE_1 }, { KNL_SRC_2_AUX, DECODE_1 } } },
	[KNL_STA3] = { { { KNL_SRC_3_MAIN, DECODE_2 }, { KNL_SRC_3_AUX, DECODE_2 } } },
	[KNL_STA4] = { { { KNL_SRC_4_MAIN, DECODE_3 }, { KNL_SRC_4_AUX, DECODE_3 } } },
	[KNL_NONE] = { { { KNL_SRC_NONE,   DECODE_0 }, { KNL_SRC_NONE,  DECODE_0 } } },
#endif
};
#ifdef BUC_CU
static VDO_Status_t tVDO_Status;
static KNL_ROLE tVDO_SvPlayRole;
static KNL_ROLE tKNL_DualCamRole[3];
static KNL_ROLE tKNL_QuadCamRole[4];
static KNL_SCALE tVDO_DecScaleParam;
static VDO_TrxSlotNum_t tVDO_TrxSlotNum;
static uint8_t ubVDO_TrxSlotChgFlag;
static uint8_t ubVDO_BufRstFlag;
static uint8_t ubVDO_AhdFlag;
#endif
#ifdef BUC_CAM
static uint8_t ubVDO_SysSetupFlag;
#endif
static uint16_t uwVDO_HSIZE;
static uint16_t uwVDO_VSIZE;
VDO_DisplayMode_t tVDO_DisplayMode = VDO_HD_MODE;
//------------------------------------------------------------------------------
void VDO_Init(void)
{
	int i = 0;
	KNL_ROLE tVDO_KNLRole;
	KNL_SRC tVDO_KNLMainSrcNum;
#ifdef BUC_CAM
	KNL_SRC tVDO_KNLSubSrcNum;
#endif
#if (defined(BUC_CU)&&defined(APP_USBD_UVCVDO_MODE) && APP_USBD_UVCVDO_MODE==1 && (APP_UVC_SNXYUV_ONLY == 2))
	    KNL_SRC tVDO_KNLSubSrcNum;
#endif

	KNL_SetVdoCodec(KNL_VDO_CODEC_H264);
#ifdef BUC_CAM
	ubVDO_SysSetupFlag = FALSE;
	tVDO_KNLRole = (KNL_ROLE)ubKNL_GetRole();
	if(KNL_NONE == tVDO_KNLRole)
	{
		tVDO_KNLRole = KNL_STA1;
		KNL_SetRole(tVDO_KNLRole);
	}
#endif
#ifdef BUC_CU
	ubVDO_AhdFlag		 = 0;
	tVDO_TrxSlotNum      = (DISPLAY_MODE == DISPLAY_1T1R)?VDO_1TSLOT:(DISPLAY_MODE == DISPLAY_2T1R)?VDO_2TSLOT:VDO_4TSLOT;
	ubVDO_TrxSlotChgFlag = FALSE;	
	tVDO_SvPlayRole      = (VDO_DISP_TYPE == KNL_DISP_SINGLE)?KNL_STA1:KNL_NONE;
	tKNL_DualCamRole[0]  = KNL_NONE;
	tKNL_DualCamRole[1]  = KNL_NONE;
	tKNL_DualCamRole[2]  = KNL_NONE;
	ubVDO_BufRstFlag	 = FALSE;
	//! Display Setting
	tVDO_Status.tVdoDispType = VDO_DISP_TYPE;
	KNL_VdoDisplaySetting();
	KNL_SetPlyMode(KNL_NORMAL_PLY);				//!< Normal Play
	VDO_SetFrameColor(0, 0, 0);				//!< Background color
#endif
	
	uwVDO_HSIZE = VDO_MAIN_H_SIZE(VDO_DISP_TYPE);
	uwVDO_VSIZE = VDO_MAIN_V_SIZE(VDO_DISP_TYPE);
	KNL_SetVdoFps(VDO_FRAME_RATE(uwVDO_HSIZE, uwVDO_VSIZE));
	KNL_SetSysMaxFps(VDO_FRAME_RATE(uwVDO_HSIZE, uwVDO_VSIZE));

	//! Data Path Setting
	KNL_VdoPathReset();
	for(i = 0; i < 256; i++)
		KNL_SetMultiOutNode(i, 0, 0, 0, 0);		//!< Disable Multi Out Feature
	KNL_SetVdoRoleInfoCbFunc(VDO_GetSourceNumber);
#ifdef BUC_CAM
	tVDO_KNLMainSrcNum = tVDO_KNLRoleInfo[tVDO_KNLRole].tVDO_KNLParam[VDO_MAIN_SRC].tKNL_SrcNum;
	tVDO_KNLSubSrcNum  = tVDO_KNLRoleInfo[tVDO_KNLRole].tVDO_KNLParam[VDO_SUB_SRC].tKNL_SrcNum;
	KNL_SetVdoResolution(tVDO_KNLMainSrcNum, uwVDO_HSIZE, uwVDO_VSIZE);
	KNL_SetVdoResolution(tVDO_KNLSubSrcNum, VDO_SUB_H_SIZE, VDO_SUB_V_SIZE);
	VDO_DataPathSetup(tVDO_KNLRole, VDO_MAIN_SRC);
	VDO_DataPathSetup(tVDO_KNLRole, VDO_SUB_SRC);
	#if !VDO_SUBPATH_ENABLE
	tVDO_KNLSubSrcNum = KNL_SRC_NONE;
	#endif
	KNL_SenorSetup(tVDO_KNLMainSrcNum, tVDO_KNLSubSrcNum);
#endif
#ifdef BUC_CU
	for(tVDO_KNLRole = KNL_STA1; tVDO_KNLRole < tVDO_TrxSlotNum; tVDO_KNLRole++)
	{
		tVDO_KNLMainSrcNum = tVDO_KNLRoleInfo[tVDO_KNLRole].tVDO_KNLParam[VDO_MAIN_SRC].tKNL_SrcNum;
		KNL_SetVdoResolution(tVDO_KNLMainSrcNum, uwVDO_HSIZE, uwVDO_VSIZE);
		tVDO_DecScaleParam = KNL_VDO_SCALE;
		VDO_DataPathSetup(tVDO_KNLRole, VDO_MAIN_SRC);
		#if (defined(APP_USBD_UVCVDO_MODE) && APP_USBD_UVCVDO_MODE==1 && (APP_UVC_SNXYUV_ONLY == 2))
		tVDO_KNLSubSrcNum = tVDO_KNLRoleInfo[tVDO_KNLRole].tVDO_KNLParam[VDO_SUB_SRC].tKNL_SrcNum;
		KNL_SetVdoResolution(tVDO_KNLSubSrcNum, uwVDO_HSIZE, uwVDO_VSIZE);
		VDO_DataPathSetup(tVDO_KNLRole, VDO_SUB_SRC);
		#endif
		tVDO_Status.tVdoPlaySte[tVDO_KNLRole] = VDO_STOP;
	}
#endif
}
//------------------------------------------------------------------------------
#ifdef BUC_CAM
static uint16_t uwVDO_CurH, uwVDO_CurV;
void VDO_KNLVdoRes (KNL_ROLE tVDO_KNLRole)
{
#if (SEN_USE == SEN_TP9950 || SEN_USE == SEN_RN6752)
	uwVDO_CurH = uwKNL_GetVdoH(tVDO_KNLRoleInfo[tVDO_KNLRole].tVDO_KNLParam[VDO_MAIN_SRC].tKNL_SrcNum);
	uwVDO_CurV = uwKNL_GetVdoV(tVDO_KNLRoleInfo[tVDO_KNLRole].tVDO_KNLParam[VDO_MAIN_SRC].tKNL_SrcNum);
#else
	uwVDO_CurH = VDO_MAIN_H_SIZE(VDO_DISP_TYPE);
	uwVDO_CurV = VDO_MAIN_V_SIZE(VDO_DISP_TYPE);
#endif	
}
//------------------------------------------------------------------------------
void VDO_KNLSysInfoSetup(KNL_ROLE tVDO_KNLRole)
{
	KNL_SRC tVDO_KNLMainSrcNum, tVDO_KNLSubSrcNum;

	KNL_SetRole(tVDO_KNLRole);
	KNL_VdoPathReset();
	tVDO_KNLMainSrcNum = tVDO_KNLRoleInfo[tVDO_KNLRole].tVDO_KNLParam[VDO_MAIN_SRC].tKNL_SrcNum;
	tVDO_KNLSubSrcNum  = tVDO_KNLRoleInfo[tVDO_KNLRole].tVDO_KNLParam[VDO_SUB_SRC].tKNL_SrcNum;
	KNL_SetVdoResolution(tVDO_KNLMainSrcNum, uwVDO_CurH, uwVDO_CurV);
	KNL_SetVdoResolution(tVDO_KNLSubSrcNum, VDO_SUB_H_SIZE, VDO_SUB_V_SIZE);
	VDO_DataPathSetup(tVDO_KNLRole, VDO_MAIN_SRC);
	VDO_DataPathSetup(tVDO_KNLRole, VDO_SUB_SRC);
	KNL_ImageEncodeSetup(tVDO_KNLMainSrcNum);
	#if VDO_SUBPATH_ENABLE
	KNL_ImageEncodeSetup(tVDO_KNLSubSrcNum);
	#else
	tVDO_KNLSubSrcNum = KNL_SRC_NONE;
	#endif
	SEN_SetPathSrc(tVDO_KNLMainSrcNum, KNL_SRC_NONE, tVDO_KNLSubSrcNum);										\
	SEN_SetOutResolution(SENSOR_PATH1, uwVDO_CurH, uwVDO_CurV);	\
	SEN_SetOutResolution(SENSOR_PATH3, VDO_SUB_H_SIZE,  VDO_SUB_V_SIZE);
	//KNL_SenorSetup(tVDO_KNLMainSrcNum, tVDO_KNLSubSrcNum);
	ubVDO_SysSetupFlag = TRUE;
}
#endif
//------------------------------------------------------------------------------
#ifdef BUC_CU
void VDO_UpdateDisplayParameter(void)
{
#if	(DISPLAY_MODE == DISPLAY_2T1R)
	KNL_ROLE tKNL_Role = tVDO_SvPlayRole;
	KNL_DISP_TYPE tVDO_DisplayType = tVDO_Status.tVdoDispType;

	tVDO_SvPlayRole = KNL_NONE;
	tVDO_Status.tVdoDispType = KNL_DISP_NONSUP;
	if(KNL_NONE == tKNL_Role)
		return;
	VDO_SwitchDisplayType(tVDO_DisplayType, &tKNL_Role);
#endif
#if	(DISPLAY_MODE == DISPLAY_4T1R)
	KNL_ROLE tKNL_Role[3] = {tKNL_DualCamRole[0], tKNL_DualCamRole[1], tKNL_DualCamRole[2]};
	KNL_DISP_TYPE tVDO_DisplayType = tVDO_Status.tVdoDispType;

	if(KNL_DISP_SINGLE == tVDO_DisplayType)
	{
		tKNL_Role[0] = (KNL_NONE == tVDO_SvPlayRole)?KNL_STA1:tVDO_SvPlayRole;
		tKNL_Role[1] = KNL_STA2;
		tVDO_SvPlayRole = KNL_NONE;
	}
	tKNL_DualCamRole[0] = tKNL_DualCamRole[1] = tKNL_DualCamRole[2] = KNL_NONE;
	tVDO_Status.tVdoDispType = KNL_DISP_NONSUP;
	if((KNL_DISP_H != tVDO_DisplayType) &&
	   ((KNL_NONE == tKNL_Role[0]) || (KNL_NONE == tKNL_Role[1])))
		return;
	VDO_SwitchDisplayType(tVDO_DisplayType, tKNL_Role);
#endif
}
//------------------------------------------------------------------------------
void VDO_DisplayLocationSetup(KNL_ROLE tVDO_CamRole, KNL_DISP_LOCATION tVDO_DispLocation)
{
	KNL_DISP_LOCATION tVDO_DispLoc;

	tVDO_DispLoc = tVDO_DispLocation;
#if	(DISPLAY_MODE == DISPLAY_1T1R)
	if(tKNL_GetDispType() == KNL_DISP_SINGLE)
		tVDO_DispLoc = KNL_DISP_LOCATION1;
#endif
	KNL_SetDispSrc(tVDO_DispLoc, tVDO_KNLRoleInfo[tVDO_CamRole].tVDO_KNLParam[VDO_MAIN_SRC].tKNL_SrcNum);
	if((KNL_DISP_SINGLE == tVDO_Status.tVdoDispType) && (KNL_NONE == tVDO_SvPlayRole))
		tVDO_SvPlayRole = tVDO_CamRole;
}
//------------------------------------------------------------------------------
void VDO_HviewSw2QuadView(KNL_DISP_TYPE tVDO_DisplayType, KNL_SrcLocateMap_t *pLocInfo)
{
#if (defined(BSP_SN93711_FHD_REC_RX_V4)||defined(BSP_D_SN93701_SSD2828_RX_V5)|| defined(BSP_D_SN93701_TC358778_RX_V6) || defined(BSP_D_SNCC70_TX_V2)||defined(BSP_D_SN93714_TX_V1))
	KNL_DISP_LOCATION t4TQuadDefuLoc[] = {KNL_DISP_LOCATION3, KNL_DISP_LOCATION1, KNL_DISP_LOCATION4, KNL_DISP_LOCATION2};
#endif

#if (	defined(BSP_D_SNCC71_GM8285C_RX_V2)		||\
		defined(BSP_D_SNCC71_TP2915_AHD_RX_V3)	||\
		defined(BSP_D_SNCC71_NVP6021_AHD_RX_V4)		)
	KNL_DISP_LOCATION t4TQuadDefuLoc[] = {KNL_DISP_LOCATION1, KNL_DISP_LOCATION2, KNL_DISP_LOCATION3, KNL_DISP_LOCATION4};
#endif
	KNL_ROLE tKNL_Role;

	if((KNL_DISP_QUAD == tVDO_DisplayType))
	{
		pLocInfo->ubSetupFlag = TRUE;
		for(tKNL_Role = KNL_STA1; tKNL_Role < tVDO_TrxSlotNum; tKNL_Role++)
		{
			pLocInfo->tSrcNum[tKNL_Role] = tVDO_KNLRoleInfo[tKNL_Role].tVDO_KNLParam[VDO_MAIN_SRC].tKNL_SrcNum;
			pLocInfo->tSrcLocate[tKNL_Role] = t4TQuadDefuLoc[tKNL_Role];
		}
	}
}
//------------------------------------------------------------------------------
#if (defined(RTC676x) && (VDO_PATH_ONLY_LIVECAM == VDO_DATAPATH_MODE))
void VDO_SetupTrxBwByViewType(KNL_DISP_TYPE tVDO_DisplayType, KNL_ROLE *pVDO_CamRole)
{
	static uint8_t ubVDO_Rtc67xSlotNum = VDO_4TSLOT;
	UI_CamsBwMode_t tVdo_TrxBwMode;
	uint8_t ubIdx = 0;
	uint8_t ubSlotNum[] = {
							[CAMS_1T30_BWMODE] = 1,
							[CAMS_1T60_BWMODE] = 1,
							[CAMS_2T_BWMODE]   = 2,
						    [CAMS_4T_BWMODE]   = 4,
						  };
	int ulSlaveId[4], iRet = 0;

	switch(tVDO_DisplayType)
	{
		case KNL_DISP_SINGLE:
		case KNL_DISP_DUAL_C:
		case KNL_DISP_DUAL_U:
			VDO_Stop();			
			tVdo_TrxBwMode = ((tVDO_DisplayType == KNL_DISP_DUAL_U) || (tVDO_DisplayType == KNL_DISP_DUAL_C))?CAMS_2T_BWMODE:CAMS_1T30_BWMODE;	
			ubVDO_Rtc67xSlotNum = ubSlotNum[tVdo_TrxBwMode];
			KNL_ResetLcdChannel();
			for(ubIdx = 0; ubIdx < ubVDO_Rtc67xSlotNum; ubIdx++)
				ulSlaveId[ubIdx] = ulKNL_GetSlaveId(pVDO_CamRole[ubIdx]);
			iRet = KNL_SetRwRemoteId(ulSlaveId, ubVDO_Rtc67xSlotNum);
			if(iRet != -1)
			{
				for(ubIdx = 0; ubIdx < ubVDO_Rtc67xSlotNum; ubIdx++)
					KNL_UpdateStaInfo(pVDO_CamRole[ubIdx], (KNL_STA1+ubIdx));
			}
			else
				return;
			break;
		case KNL_DISP_H:
		case KNL_DISP_QUAD:
		case KNL_DISP_3T_2L1R:
		case KNL_DISP_3T_1L2R:
		case KNL_DISP_3T_2T1B:
		case KNL_DISP_3T_1T2B:
		case KNL_DISP_3T_3COL:
			VDO_Stop();
			tVdo_TrxBwMode = CAMS_4T_BWMODE;
			for(ubIdx = 0; ubIdx < 4; ubIdx++)
				ulSlaveId[ubIdx] = ulKNL_GetSlaveId(ubIdx);

				KNL_ResetLcdChannel();
				
				iRet = KNL_SetRwRemoteId(ulSlaveId, 4);
				if(iRet != -1)
				{
					for(ubIdx = 0; ubIdx < 4; ubIdx++)
						KNL_UpdateStaInfo((KNL_STA1+ubIdx), (KNL_STA1+ubIdx));
				}
				else
					return;
			break;
	
		default:
			return;
	}
	KNL_SetTXRSlotNum(ubSlotNum[tVdo_TrxBwMode]);
	VDO_SetStreamSlotNum(ubSlotNum[tVdo_TrxBwMode]);
}
#endif
//------------------------------------------------------------------------------
void VDO_DataPathFixedMode(KNL_DISP_TYPE tVDO_DisplayType, KNL_ROLE *pVDO_CamRole)
{
	KNL_SrcLocateMap_t tVDO_KNLSrcLocate;

	switch(tVDO_DisplayType)
	{
		case KNL_DISP_SINGLE:
		{
			KNL_ROLE tVDO_KNLRole = KNL_NONE;

			KNL_SetTRXPathActivity();
			tVDO_KNLRole = *pVDO_CamRole;
			if(tVDO_SvPlayRole == tVDO_KNLRole)
				break;
			tVDO_KNLSrcLocate.ubSetupFlag 	  	= FALSE;
			tVDO_KNLSrcLocate.ubDispBufChgFlag 	= ubVDO_BufRstFlag;
			tVDO_KNLSrcLocate.tSrcNum[0]  = tVDO_KNLRoleInfo[tVDO_KNLRole].tVDO_KNLParam[VDO_MAIN_SRC].tKNL_SrcNum;
			KNL_ModifyDispType(KNL_DISP_SINGLE, tVDO_KNLSrcLocate);
			tVDO_SvPlayRole = tVDO_KNLRole;
			break;
		}
		case KNL_DISP_DUAL_C:
		case KNL_DISP_DUAL_U:
		case KNL_DISP_3T_2L1R:
		case KNL_DISP_3T_1L2R:
		case KNL_DISP_3T_2T1B:
		case KNL_DISP_3T_1T2B:
		case KNL_DISP_3T_3COL:
		{
			uint8_t ubTolRoleNum = 2, ubRoleIdx;

			KNL_SetTRXPathActivity();
			ubTolRoleNum = (IS_KNL_DISP3T_VIEW(tVDO_DisplayType))?3:2;
			tKNL_DualCamRole[2] = KNL_NONE;
			tVDO_KNLSrcLocate.ubSetupFlag 		= TRUE;
			tVDO_KNLSrcLocate.ubDispBufChgFlag 	= ubVDO_BufRstFlag;
			for(ubRoleIdx = 0; ubRoleIdx < ubTolRoleNum; ubRoleIdx++)
			{
				tKNL_DualCamRole[ubRoleIdx] = *(pVDO_CamRole + ubRoleIdx);
				tVDO_KNLSrcLocate.tSrcNum[ubRoleIdx] = tVDO_KNLRoleInfo[tKNL_DualCamRole[ubRoleIdx]].tVDO_KNLParam[VDO_MAIN_SRC].tKNL_SrcNum;
			}
			tVDO_KNLSrcLocate.tSrcLocate[0] = (KNL_DISP_DUAL_U == tVDO_DisplayType)?KNL_DISP_LOCATION1:KNL_DISP_LOCATION2;
			tVDO_KNLSrcLocate.tSrcLocate[1] = (KNL_DISP_DUAL_U == tVDO_DisplayType)?KNL_DISP_LOCATION2:KNL_DISP_LOCATION1;
			if(IS_KNL_DISP3T_VIEW(tVDO_DisplayType))
			{
				if(KNL_DISP_3T_3COL == tVDO_DisplayType)
				{
					tVDO_KNLSrcLocate.tSrcLocate[0] = KNL_DISP_LOCATION1;
					tVDO_KNLSrcLocate.tSrcLocate[1] = KNL_DISP_LOCATION2;
					tVDO_KNLSrcLocate.tSrcLocate[2] = KNL_DISP_LOCATION3;
				}
				else if((((KNL_DISP_3T_2L1R == tVDO_DisplayType) || (KNL_DISP_3T_1L2R == tVDO_DisplayType)) && (KNL_DISP_ROTATE_90 == KNL_DISP_ROTATE_SETTING)) ||
		                (((KNL_DISP_3T_2T1B == tVDO_DisplayType) || (KNL_DISP_3T_1T2B == tVDO_DisplayType)) && ( KNL_DISP_ROTATE_0 == KNL_DISP_ROTATE_SETTING)))
				{
					tVDO_KNLSrcLocate.tSrcLocate[0] = (KNL_DISP_ROTATE_90 == KNL_DISP_ROTATE_SETTING)?KNL_DISP_LOCATION1:KNL_DISP_LOCATION2;
					tVDO_KNLSrcLocate.tSrcLocate[1] = (KNL_DISP_ROTATE_90 == KNL_DISP_ROTATE_SETTING)?KNL_DISP_LOCATION2:KNL_DISP_LOCATION3;
					tVDO_KNLSrcLocate.tSrcLocate[2] = (KNL_DISP_ROTATE_90 == KNL_DISP_ROTATE_SETTING)?KNL_DISP_LOCATION3:KNL_DISP_LOCATION1;
				}
				else
				{
					tVDO_KNLSrcLocate.tSrcLocate[0] = (KNL_DISP_ROTATE_90 == KNL_DISP_ROTATE_SETTING)?KNL_DISP_LOCATION2:KNL_DISP_LOCATION1;
					tVDO_KNLSrcLocate.tSrcLocate[1] = (KNL_DISP_ROTATE_90 == KNL_DISP_ROTATE_SETTING)?KNL_DISP_LOCATION1:KNL_DISP_LOCATION2;
					tVDO_KNLSrcLocate.tSrcLocate[2] = KNL_DISP_LOCATION3;
				}
			}
			KNL_ModifyDispType(tVDO_DisplayType, tVDO_KNLSrcLocate);
			tVDO_SvPlayRole = KNL_NONE;
			break;
		}
		default:
			if(tVDO_DisplayType == tVDO_Status.tVdoDispType)
				break;
			KNL_DisManuCtrlTRXPath();
			tVDO_KNLSrcLocate.ubSetupFlag 		= FALSE;
			tVDO_KNLSrcLocate.ubDispBufChgFlag 	= ubVDO_BufRstFlag;
			if(tVDO_DisplayType == KNL_DISP_QUAD)
			{
				tVDO_KNLSrcLocate.ubSetupFlag = TRUE;
				for(uint8_t ubRoleIdx = 0; ubRoleIdx < 4; ubRoleIdx++)
				{
					tKNL_QuadCamRole[ubRoleIdx] = *(pVDO_CamRole + ubRoleIdx);
					tVDO_KNLSrcLocate.tSrcNum[ubRoleIdx] = tVDO_KNLRoleInfo[tKNL_QuadCamRole[ubRoleIdx]].tVDO_KNLParam[VDO_MAIN_SRC].tKNL_SrcNum;
				}
				tVDO_KNLSrcLocate.tSrcLocate[0] = KNL_DISP_LOCATION1;//upper left
				tVDO_KNLSrcLocate.tSrcLocate[1] = KNL_DISP_LOCATION2;//upper right
				tVDO_KNLSrcLocate.tSrcLocate[2] = KNL_DISP_LOCATION3;//lower left
				tVDO_KNLSrcLocate.tSrcLocate[3] = KNL_DISP_LOCATION4;//lower right
			}
			KNL_ModifyDispType(tVDO_DisplayType, tVDO_KNLSrcLocate);
			tVDO_SvPlayRole = KNL_NONE;
			break;
	}
}
//------------------------------------------------------------------------------
extern uint8_t ISPlaying_wav;
void VDO_DataPathResetMode(KNL_DISP_TYPE tVDO_DisplayType, KNL_ROLE *pVDO_CamRole)
{
	KNL_ROLE tKNL_Role;
	KNL_SRC tVDO_KNLSrcNum;
#if (defined(APP_USBD_UVCVDO_MODE) && APP_USBD_UVCVDO_MODE==1 && (APP_UVC_SNXYUV_ONLY == 2))
	KNL_SRC tVDO_KNLSubSrcNum;
#endif
	KNL_SrcLocateMap_t tVDO_KNLSrcLocate;
	static uint8_t ubVDO_DualPathFlag = FALSE;

	ubVDO_DualPathFlag = ubVDO_DualPathFlag;
	ubVDO_AhdFlag = ubVDO_AhdFlag;
	printf("tVDO_DisplayType = %d\n", tVDO_DisplayType);
//	//wav
//	ADO_WavStop();
//	ISPlaying_wav = 0;
	KNL_DisManuCtrlTRXPath();
	switch(tVDO_DisplayType)
	{
		case KNL_DISP_SINGLE:
		{
			KNL_ROLE tVDO_KNLRole = KNL_NONE;

			tVDO_KNLRole = *pVDO_CamRole;			
			tVDO_KNLSrcLocate.ubSetupFlag = FALSE;
			tVDO_KNLSrcLocate.ubDispBufChgFlag = TRUE;
			tVDO_KNLSrcLocate.ubFixDispChFlag = TRUE;
			tVDO_KNLSrcLocate.tSrcNum[0]  = tVDO_KNLSrcNum = tVDO_KNLRoleInfo[tVDO_KNLRole].tVDO_KNLParam[VDO_MAIN_SRC].tKNL_SrcNum;
			#if (defined(APP_USBD_UVCVDO_MODE) && APP_USBD_UVCVDO_MODE==1 && (APP_UVC_SNXYUV_ONLY == 2))
			tVDO_KNLSubSrcNum = tVDO_KNLRoleInfo[tVDO_KNLRole].tVDO_KNLParam[VDO_SUB_SRC].tKNL_SrcNum;
			#endif
			VDO_Stop();
		#if (defined(RTC676x) && (VDO_DATAPATH_MODE == VDO_PATH_ONLY_LIVECAM))
			VDO_SetupTrxBwByViewType(tVDO_DisplayType, pVDO_CamRole);
		#endif
			KNL_VdoPathReset();
			KNL_SetVdoResolution(tVDO_KNLSrcNum, uwVDO_HSIZE, uwVDO_VSIZE);
			#if (defined(APP_USBD_UVCVDO_MODE) && APP_USBD_UVCVDO_MODE==1 && (APP_UVC_SNXYUV_ONLY == 2))
			KNL_SetVdoResolution(tVDO_KNLSubSrcNum, uwVDO_HSIZE, uwVDO_VSIZE);
			#endif
			tVDO_DecScaleParam = KNL_SCALE_X1;
			VDO_DataPathSetup(tVDO_KNLRole, VDO_MAIN_SRC);
			#if (defined(APP_USBD_UVCVDO_MODE) && APP_USBD_UVCVDO_MODE==1 && (APP_UVC_SNXYUV_ONLY == 2))
			VDO_DataPathSetup(tVDO_KNLRole, VDO_SUB_SRC);
			#endif
			KNL_ModifyDispType(KNL_DISP_SINGLE, tVDO_KNLSrcLocate);
			KNL_ImageDecodeSetup(tVDO_KNLSrcNum);
			#if (defined(APP_USBD_UVCVDO_MODE) && APP_USBD_UVCVDO_MODE==1 && (APP_UVC_SNXYUV_ONLY == 2))
			KNL_ImageDecodeSetup(tVDO_KNLSubSrcNum);
			#endif
			ubVDO_DualPathFlag = FALSE;
			tKNL_DualCamRole[0] = tKNL_DualCamRole[1] = tKNL_DualCamRole[2] = KNL_NONE;
			if(VDO_STOP == tVDO_Status.tVdoPlaySte[tVDO_KNLRole])
			{
				tVDO_KNLSrcNum = tVDO_KNLRoleInfo[tVDO_KNLRole].tVDO_KNLParam[VDO_MAIN_SRC].tKNL_SrcNum;
				KNL_VdoStart(tVDO_KNLSrcNum);
				#if (defined(APP_USBD_UVCVDO_MODE) && APP_USBD_UVCVDO_MODE==1 && (APP_UVC_SNXYUV_ONLY == 2))
				tVDO_KNLSubSrcNum = tVDO_KNLRoleInfo[tVDO_KNLRole].tVDO_KNLParam[VDO_SUB_SRC].tKNL_SrcNum;
				KNL_VdoStart(tVDO_KNLSubSrcNum);
				#endif
				tVDO_Status.tVdoPlaySte[tVDO_KNLRole] = VDO_START;
			}
			tVDO_SvPlayRole = tVDO_KNLRole;
			break;
		}
#if (DISPLAY_MODE == DISPLAY_4T1R)
		case KNL_DISP_DUAL_C:
		case KNL_DISP_DUAL_U:
/*		case KNL_DISP_3T_2L1R:
		case KNL_DISP_3T_1L2R:
		case KNL_DISP_3T_2T1B:
		case KNL_DISP_3T_1T2B:
		case KNL_DISP_3T_3COL:*/
		{
			uint8_t ubTolRoleNum = 2, ubRoleIdx;

			ubTolRoleNum = (IS_KNL_DISP3T_VIEW(tVDO_DisplayType))?3:2;
			tKNL_DualCamRole[2] = KNL_NONE;
			tVDO_KNLSrcLocate.ubSetupFlag = TRUE;
			tVDO_KNLSrcLocate.ubDispBufChgFlag = TRUE;
			tVDO_KNLSrcLocate.ubFixDispChFlag = TRUE;
			for(ubRoleIdx = 0; ubRoleIdx < ubTolRoleNum; ubRoleIdx++)
			{
				tKNL_DualCamRole[ubRoleIdx] = *(pVDO_CamRole + ubRoleIdx);
				tVDO_KNLSrcLocate.tSrcNum[ubRoleIdx] = tVDO_KNLRoleInfo[tKNL_DualCamRole[ubRoleIdx]].tVDO_KNLParam[VDO_MAIN_SRC].tKNL_SrcNum;
			}
			tVDO_KNLSrcLocate.tSrcLocate[0] = (KNL_DISP_DUAL_U == tVDO_DisplayType)?KNL_DISP_LOCATION1:KNL_DISP_LOCATION2;
			tVDO_KNLSrcLocate.tSrcLocate[1] = (KNL_DISP_DUAL_U == tVDO_DisplayType)?KNL_DISP_LOCATION2:KNL_DISP_LOCATION1;
			/*if(IS_KNL_DISP3T_VIEW(tVDO_DisplayType))
			{
				if(KNL_DISP_3T_3COL == tVDO_DisplayType)
				{
					tVDO_KNLSrcLocate.tSrcLocate[0] = KNL_DISP_LOCATION1;
					tVDO_KNLSrcLocate.tSrcLocate[1] = KNL_DISP_LOCATION2;
					tVDO_KNLSrcLocate.tSrcLocate[2] = KNL_DISP_LOCATION3;
				}
				else if((((KNL_DISP_3T_2L1R == tVDO_DisplayType) || (KNL_DISP_3T_1L2R == tVDO_DisplayType)) && (KNL_DISP_ROTATE_90 == KNL_DISP_ROTATE_SETTING)) ||
		                (((KNL_DISP_3T_2T1B == tVDO_DisplayType) || (KNL_DISP_3T_1T2B == tVDO_DisplayType)) && ( KNL_DISP_ROTATE_0 == KNL_DISP_ROTATE_SETTING)))
				{
					tVDO_KNLSrcLocate.tSrcLocate[0] = (KNL_DISP_ROTATE_90 == KNL_DISP_ROTATE_SETTING)?KNL_DISP_LOCATION1:KNL_DISP_LOCATION2;
					tVDO_KNLSrcLocate.tSrcLocate[1] = (KNL_DISP_ROTATE_90 == KNL_DISP_ROTATE_SETTING)?KNL_DISP_LOCATION2:KNL_DISP_LOCATION3;
					tVDO_KNLSrcLocate.tSrcLocate[2] = (KNL_DISP_ROTATE_90 == KNL_DISP_ROTATE_SETTING)?KNL_DISP_LOCATION3:KNL_DISP_LOCATION1;
				}
				else
				{
					tVDO_KNLSrcLocate.tSrcLocate[0] = (KNL_DISP_ROTATE_90 == KNL_DISP_ROTATE_SETTING)?KNL_DISP_LOCATION2:KNL_DISP_LOCATION3;
					tVDO_KNLSrcLocate.tSrcLocate[1] = (KNL_DISP_ROTATE_90 == KNL_DISP_ROTATE_SETTING)?KNL_DISP_LOCATION1:KNL_DISP_LOCATION1;
					tVDO_KNLSrcLocate.tSrcLocate[2] = (KNL_DISP_ROTATE_90 == KNL_DISP_ROTATE_SETTING)?KNL_DISP_LOCATION3:KNL_DISP_LOCATION2;
				}
			}*/
			VDO_Stop();
		#if (defined(RTC676x) && (VDO_DATAPATH_MODE == VDO_PATH_ONLY_LIVECAM))
			VDO_SetupTrxBwByViewType(tVDO_DisplayType, pVDO_CamRole);
		#endif
			KNL_VdoPathReset();
			for(ubRoleIdx = 0; ubRoleIdx < ubTolRoleNum; ubRoleIdx++)
			{
				tVDO_KNLSrcNum = tVDO_KNLRoleInfo[tKNL_DualCamRole[ubRoleIdx]].tVDO_KNLParam[VDO_MAIN_SRC].tKNL_SrcNum;
				KNL_SetVdoResolution(tVDO_KNLSrcNum, uwVDO_HSIZE, uwVDO_VSIZE);
				tVDO_DecScaleParam = KNL_SCALE_X1;
				VDO_DataPathSetup(tKNL_DualCamRole[ubRoleIdx], VDO_MAIN_SRC);
			}
			KNL_ModifyDispType(tVDO_DisplayType, tVDO_KNLSrcLocate);
			for(ubRoleIdx = 0; ubRoleIdx < ubTolRoleNum; ubRoleIdx++)
				KNL_ImageDecodeSetup(tVDO_KNLRoleInfo[tKNL_DualCamRole[ubRoleIdx]].tVDO_KNLParam[VDO_MAIN_SRC].tKNL_SrcNum);
			for(ubRoleIdx = 0; ubRoleIdx < ubTolRoleNum; ubRoleIdx++)
			{
				if(VDO_STOP == tVDO_Status.tVdoPlaySte[tKNL_DualCamRole[ubRoleIdx]])
				{
					tVDO_KNLSrcNum = tVDO_KNLRoleInfo[tKNL_DualCamRole[ubRoleIdx]].tVDO_KNLParam[VDO_MAIN_SRC].tKNL_SrcNum;
					KNL_VdoStart(tVDO_KNLSrcNum);
					tVDO_Status.tVdoPlaySte[tKNL_DualCamRole[ubRoleIdx]] = VDO_START;
				}
			}
			ubVDO_DualPathFlag = TRUE;
			tVDO_SvPlayRole    = KNL_NONE;
			break;
		}
#endif
		default:
			//if(tVDO_DisplayType == tVDO_Status.tVdoDispType)
			//	break;
			tVDO_KNLSrcLocate.ubSetupFlag = FALSE;
			tVDO_KNLSrcLocate.ubDispBufChgFlag = TRUE;
			tVDO_KNLSrcLocate.ubFixDispChFlag = TRUE;
			if(tVDO_DisplayType == KNL_DISP_QUAD)
			{
				tVDO_KNLSrcLocate.ubSetupFlag = TRUE;
				for(uint8_t ubRoleIdx = 0; ubRoleIdx < 4; ubRoleIdx++)
				{
					tKNL_QuadCamRole[ubRoleIdx] = *(pVDO_CamRole + ubRoleIdx);
					tVDO_KNLSrcLocate.tSrcNum[ubRoleIdx] = tVDO_KNLRoleInfo[tKNL_QuadCamRole[ubRoleIdx]].tVDO_KNLParam[VDO_MAIN_SRC].tKNL_SrcNum;
				}
				tVDO_KNLSrcLocate.tSrcLocate[0] = KNL_DISP_LOCATION1;//upper left
				tVDO_KNLSrcLocate.tSrcLocate[1] = KNL_DISP_LOCATION2;//upper right
				tVDO_KNLSrcLocate.tSrcLocate[2] = KNL_DISP_LOCATION3;//lower left
				tVDO_KNLSrcLocate.tSrcLocate[3] = KNL_DISP_LOCATION4;//lower right
			}
			VDO_Stop();
		#if (defined(RTC676x) && (VDO_DATAPATH_MODE == VDO_PATH_ONLY_LIVECAM))
			VDO_SetupTrxBwByViewType(tVDO_DisplayType, pVDO_CamRole);
		#endif
			//VDO_HviewSw2QuadView(tVDO_DisplayType, &tVDO_KNLSrcLocate);
			KNL_VdoPathReset();
			for(tKNL_Role = KNL_STA1; tKNL_Role < tVDO_TrxSlotNum; tKNL_Role++)
			{
				tVDO_KNLSrcNum = tVDO_KNLRoleInfo[tKNL_Role].tVDO_KNLParam[VDO_MAIN_SRC].tKNL_SrcNum;
				KNL_SetVdoResolution(tVDO_KNLSrcNum, uwVDO_HSIZE, uwVDO_VSIZE);
				tVDO_DecScaleParam = (KNL_DISP_H == tVDO_DisplayType)?KNL_VDO_SCALE:KNL_SCALE_X1;
				VDO_DataPathSetup(tKNL_Role, VDO_MAIN_SRC);
				#if (defined(APP_USBD_UVCVDO_MODE) && APP_USBD_UVCVDO_MODE==1 && (APP_UVC_SNXYUV_ONLY == 2))
				tVDO_KNLSubSrcNum = tVDO_KNLRoleInfo[tKNL_Role].tVDO_KNLParam[VDO_SUB_SRC].tKNL_SrcNum;
				KNL_SetVdoResolution(tVDO_KNLSubSrcNum, uwVDO_HSIZE, uwVDO_VSIZE);
				VDO_DataPathSetup(tKNL_Role, VDO_SUB_SRC);
				#endif
			}

			KNL_ModifyDispType(tVDO_DisplayType, tVDO_KNLSrcLocate);

			for(tKNL_Role = KNL_STA1; tKNL_Role < tVDO_TrxSlotNum; tKNL_Role++)
			{
				KNL_ImageDecodeSetup(tVDO_KNLRoleInfo[tKNL_Role].tVDO_KNLParam[VDO_MAIN_SRC].tKNL_SrcNum);
				#if (defined(APP_USBD_UVCVDO_MODE) && APP_USBD_UVCVDO_MODE==1 && (APP_UVC_SNXYUV_ONLY == 2))
				KNL_ImageDecodeSetup(tVDO_KNLRoleInfo[tKNL_Role].tVDO_KNLParam[VDO_SUB_SRC].tKNL_SrcNum);
				#endif
			}

			for(tKNL_Role = KNL_STA1; tKNL_Role < tVDO_TrxSlotNum; tKNL_Role++)
			{
				if(VDO_STOP == tVDO_Status.tVdoPlaySte[tKNL_Role])
				{
					tVDO_KNLSrcNum = tVDO_KNLRoleInfo[tKNL_Role].tVDO_KNLParam[VDO_MAIN_SRC].tKNL_SrcNum;
					KNL_VdoStart(tVDO_KNLSrcNum);
					#if (defined(APP_USBD_UVCVDO_MODE) && APP_USBD_UVCVDO_MODE==1 && (APP_UVC_SNXYUV_ONLY == 2))
					tVDO_KNLSubSrcNum = tVDO_KNLRoleInfo[tKNL_Role].tVDO_KNLParam[VDO_SUB_SRC].tKNL_SrcNum;
					KNL_VdoStart(tVDO_KNLSubSrcNum);
					#endif
					tVDO_Status.tVdoPlaySte[tKNL_Role] = VDO_START;
				}
			}
			tVDO_SvPlayRole     = KNL_NONE;
			tKNL_DualCamRole[0] = tKNL_DualCamRole[1] = tKNL_DualCamRole[2] = KNL_NONE;
			ubVDO_DualPathFlag = FALSE;
			break;
	}
}
//------------------------------------------------------------------------------
void VDO_SwitchAhdType (KNL_DISP_TYPE tVDO_DisplayType, KNL_ROLE *pVDO_CamRole,
						uint16_t uwHsize, uint16_t uwVsize)
{
	uwVDO_HSIZE 		 = uwHsize;
	uwVDO_VSIZE 		 = uwVsize;
	ubVDO_AhdFlag = 1;
	printf("%s %s %d call VDO_DataPathResetMode=========\n", __FILE__, __func__, __LINE__);
	VDO_DataPathResetMode(tVDO_DisplayType, pVDO_CamRole);
	ubVDO_AhdFlag = 0;
	tVDO_Status.tVdoDispType = tVDO_DisplayType;
	
}
//------------------------------------------------------------------------------
void VDO_SwitchDisplayType(KNL_DISP_TYPE tVDO_DisplayType, KNL_ROLE *pVDO_CamRole)
{
	uint16_t uwHSize = 0, uwVSize = 0;
	uint8_t ubVDO_ResChgFlag = FALSE;
	uint8_t ubStatus;

	void (*pVDO_DataPathMode[])(KNL_DISP_TYPE, KNL_ROLE *) = {VDO_DataPathFixedMode, VDO_DataPathResetMode};

#if (VDO_DATAPATH_MODE == VDO_PATH_ONLY_LIVECAM)
	uwHSize = VDO_MAIN_H_SIZE(tVDO_DisplayType);
	uwVSize = VDO_MAIN_V_SIZE(tVDO_DisplayType);
	ubVDO_ResChgFlag = TRUE;
	
	if(tVDO_DisplayMode == VDO_SD_MODE && (uwHSize > WVGA_WIDTH || uwVSize > WVGA_HEIGHT))//㊣那???㏒那???℅?∩車﹞?㊣??那?米?a800x448
	{
		uwHSize = WVGA_WIDTH;
		uwVSize = WVGA_HEIGHT;
	}
	for(uint8_t i = 0;i < 4;i ++)
	{
		if(tVDO_DisplayMode == VDO_SD_MODE)
			KNL_SetFlag4SkipFirstFrame(i,TRUE);
		else
			KNL_SetFlag4SkipFirstFrame(i,FALSE);
	}
#else	
	switch(tVDO_TrxSlotNum)
	{
		case VDO_1TSLOT:
			uwHSize = VDO_MAIN_H_SIZE(KNL_DISP_SINGLE);
			uwVSize = VDO_MAIN_V_SIZE(KNL_DISP_SINGLE);
			break;
		case VDO_2TSLOT:
			uwHSize = VDO_MAIN_H_SIZE(KNL_DISP_DUAL_U);
			uwVSize = VDO_MAIN_V_SIZE(KNL_DISP_DUAL_U);
			break;
		case VDO_4TSLOT:
			uwHSize = VDO_MAIN_H_SIZE(VDO_DISP_TYPE);
			uwVSize = VDO_MAIN_V_SIZE(VDO_DISP_TYPE);
			break;
		default:
			return;
	}
#endif
	ubStatus = KNL_CamIsCvbsCheck();
	if(ubStatus)
	{
		uwHSize = 640;
		uwVSize = 480;
	}
	if((uwVDO_HSIZE != uwHSize) || (uwVDO_VSIZE != uwVSize) ||
	   (TRUE == ubVDO_TrxSlotChgFlag))
	{
		uwVDO_HSIZE 		 = uwHSize;
		uwVDO_VSIZE 		 = uwVSize;
		ubVDO_ResChgFlag 	 = TRUE;
		ubVDO_TrxSlotChgFlag = FALSE;
	}
	if(pVDO_DataPathMode[ubVDO_ResChgFlag])
	{
		printf("%s %s %d call pVDO_DataPathMode=========\n", __FILE__, __func__, __LINE__);
		printf("ubVDO_ResChgFlag = %d\n", ubVDO_ResChgFlag);
		pVDO_DataPathMode[ubVDO_ResChgFlag](tVDO_DisplayType, pVDO_CamRole);
	}
	tVDO_Status.tVdoDispType = tVDO_DisplayType;
}
//------------------------------------------------------------------------------
KNL_DISP_TYPE VDO_GetDispType(void)
{
	return tVDO_Status.tVdoDispType;
}
//------------------------------------------------------------------------------
void VDO_RestartPreview(KNL_DISP_TYPE tVDO_DisplayType, KNL_ROLE *pVDO_CamRole)
{
	KNL_ROLE tKNL_Role;
	KNL_SRC tVDO_KNLSrcNum;

	VDO_Stop();
	KNL_VdoPathReset();
#if APP_FS_FILE_LIST_STYLE
	uwVDO_HSIZE = KNL_ThmShowInfo.uwHSize_Display;
	uwVDO_VSIZE = KNL_ThmShowInfo.uwVSize_Display;
//	printf("VDO RestartPreV->%d_%d\n",uwVDO_HSIZE,uwVDO_VSIZE);
#endif
	tVDO_SvPlayRole = KNL_NONE;
	tVDO_Status.tVdoDispType = KNL_DISP_NONSUP;
	for(tKNL_Role = KNL_STA1; tKNL_Role < tVDO_TrxSlotNum; tKNL_Role++)
	{
		tVDO_KNLSrcNum = tVDO_KNLRoleInfo[tKNL_Role].tVDO_KNLParam[VDO_MAIN_SRC].tKNL_SrcNum;
		if(((uwKNL_GetVdoH(tVDO_KNLSrcNum) != uwVDO_HSIZE) ||
		    (uwKNL_GetVdoV(tVDO_KNLSrcNum) != uwVDO_VSIZE)) && (FALSE == ubVDO_BufRstFlag))
		{
			ubVDO_BufRstFlag = TRUE;
//			printf("VDO RestartPreV->%d_%d, %d_%d, BufRstFlg=1\n",
//				uwKNL_GetVdoH(tVDO_KNLSrcNum),uwKNL_GetVdoV(tVDO_KNLSrcNum),
//				uwVDO_HSIZE,uwVDO_VSIZE);
		}
		KNL_SetVdoResolution(tVDO_KNLSrcNum, uwVDO_HSIZE, uwVDO_VSIZE);
		tVDO_DecScaleParam = KNL_VDO_SCALE;
		VDO_DataPathSetup(tKNL_Role, VDO_MAIN_SRC);
	}
	VDO_SwitchDisplayType(tVDO_DisplayType, pVDO_CamRole);
	for(tKNL_Role = KNL_STA1; tKNL_Role < tVDO_TrxSlotNum; tKNL_Role++)
	{
		tVDO_KNLSrcNum = tVDO_KNLRoleInfo[tKNL_Role].tVDO_KNLParam[VDO_MAIN_SRC].tKNL_SrcNum;
	    KNL_ImageDecodeSetup(tVDO_KNLSrcNum);
	}
	ubVDO_BufRstFlag = FALSE;
	VDO_Start();
}
//------------------------------------------------------------------------------
void VDO_RemoveDataPath(KNL_ROLE tVDO_CamRole)
{
	KNL_VdoStop(tVDO_KNLRoleInfo[tVDO_CamRole].tVDO_KNLParam[VDO_MAIN_SRC].tKNL_SrcNum);
	KNL_VdoStop(tVDO_KNLRoleInfo[tVDO_CamRole].tVDO_KNLParam[VDO_SUB_SRC].tKNL_SrcNum);
	KNL_VdoPathNodeReset(tVDO_KNLRoleInfo[tVDO_CamRole].tVDO_KNLParam[VDO_MAIN_SRC].tKNL_SrcNum);
	tVDO_Status.tVdoPlaySte[tVDO_CamRole] = VDO_STOP;
	if(tVDO_SvPlayRole == tVDO_CamRole)
		tVDO_SvPlayRole = KNL_NONE;
}
//------------------------------------------------------------------------------
void VDO_SetFrameColor(uint8_t ubRed, uint8_t ubGreen, uint8_t ubBlue)
{
	LCD_ChDisableColor(ubRed, ubGreen, ubBlue);
	KNL_SetDmyPatColor(ubRed, ubGreen, ubBlue);
}
//------------------------------------------------------------------------------
void VDO_FillFrameBuf(uint8_t ubCamNum)
{
	UI_CamNum_t tDisCamNum;
	KNL_ROLE tVdoRole;	

	tDisCamNum = (UI_CamNum_t)ubCamNum;
	tVdoRole   = APP_GetSTANumMappingTable(tDisCamNum)->tKNL_StaNum;
	KNL_VdoPathNodeReset(tVDO_KNLRoleInfo[tVdoRole].tVDO_KNLParam[VDO_MAIN_SRC].tKNL_SrcNum);
}
//------------------------------------------------------------------------------
void VDO_SetStreamSlotNum(uint8_t ubSlotNum)
{
	uint16_t uwHSize = 0, uwVSize = 0;
	KNL_DISP_TYPE tDispType;
	
	tDispType = tDispType;//Avoid Warning

	tVDO_TrxSlotNum = (VDO_TrxSlotNum_t)ubSlotNum;
	tVDO_SvPlayRole = KNL_NONE;
	tVDO_Status.tVdoDispType = KNL_DISP_NONSUP;
	ubVDO_TrxSlotChgFlag = TRUE;
	tDispType = (VDO_4TSLOT == tVDO_TrxSlotNum)?KNL_DISP_H:KNL_DISP_SINGLE;
	uwHSize   = VDO_MAIN_H_SIZE(tDispType);
	uwVSize   = VDO_MAIN_V_SIZE(tDispType);
	KNL_SetVdoFps(VDO_FRAME_RATE(uwHSize, uwVSize));
	KNL_SetTargetFps(ubKNL_GetVdoFps());
}
#endif
//------------------------------------------------------------------------------
void VDO_DataPathSetup(KNL_ROLE tVDO_KNLRole, VDO_SrcType_t tVDO_SrcType)
{
#ifdef BUC_CAM
#if (APP_TRX_FUNC_ENABLE == 1)
	KNL_NODE tVDO_KNLMainPath[] = {KNL_NODE_SEN, KNL_NODE_SEN_YUV_BUF, KNL_NODE_H264_ENC, KNL_NODE_VDO_BS_BUF1, KNL_NODE_COMM_TX_VDO, KNL_NODE_END};
#else
	KNL_NODE tVDO_KNLMainPath[] = {KNL_NODE_SEN, KNL_NODE_SEN_YUV_BUF, KNL_NODE_H264_ENC, KNL_NODE_VDO_BS_BUF1, KNL_NODE_END};
#endif
	KNL_NODE tVDO_KNLSubPath[]  = {KNL_NODE_SEN, KNL_NODE_SEN_YUV_BUF, KNL_NODE_END};
#endif
#ifdef BUC_CU
    #if ((LCD_PANEL != LCD_NO_PANEL) && (((USBD_COMPOSITE_MODE == USBD_DEFU_CLASS) || (USBD_UVC_MODE == USBD_DEFU_CLASS)) && (USBD_UVC_H265_FMT == USBD_UVC_FMT)))
	KNL_NODE tVDO_KNLMainPath[] = {KNL_NODE_COMM_RX_VDO, KNL_NODE_VDO_BS_BUF1, KNL_NODE_LCD, KNL_NODE_END};
	KNL_NODE tVDO_KNLSubPath[]  = {KNL_NODE_COMM_RX_VDO, KNL_NODE_VDO_BS_BUF1, KNL_NODE_LCD, KNL_NODE_END};
	#elif ((LCD_PANEL == LCD_NO_PANEL) && (((USBD_COMPOSITE_MODE == USBD_DEFU_CLASS) || (USBD_UVC_MODE == USBD_DEFU_CLASS)) && (USBD_UVC_H265_FMT == USBD_UVC_FMT)))
	KNL_NODE tVDO_KNLMainPath[] = {KNL_NODE_COMM_RX_VDO, KNL_NODE_VDO_BS_BUF1, KNL_NODE_END};
	KNL_NODE tVDO_KNLSubPath[]  = {KNL_NODE_COMM_RX_VDO, KNL_NODE_VDO_BS_BUF1, KNL_NODE_END};
	#elif (LCD_PANEL != LCD_NO_PANEL)
	KNL_NODE tVDO_KNLMainPath[] = {KNL_NODE_COMM_RX_VDO, KNL_NODE_VDO_BS_BUF1, KNL_NODE_H264_DEC, KNL_NODE_LCD, KNL_NODE_END};	
	#if (defined(APP_USBD_UVCVDO_MODE) && APP_USBD_UVCVDO_MODE==1 && (APP_UVC_SNXYUV_ONLY == 2))
	KNL_NODE tVDO_KNLSubPath[]  = {KNL_NODE_COMM_RX_VDO, KNL_NODE_VDO_BS_BUF1, KNL_NODE_H264_DEC, KNL_NODE_UVC_MJPG, KNL_NODE_END};
	#else
	KNL_NODE tVDO_KNLSubPath[]  = {KNL_NODE_COMM_RX_VDO, KNL_NODE_VDO_BS_BUF1, KNL_NODE_H264_DEC, KNL_NODE_LCD, KNL_NODE_END};
	#endif
    #else
	KNL_NODE tVDO_KNLMainPath[] = {KNL_NODE_COMM_RX_VDO, KNL_NODE_VDO_BS_BUF1, KNL_NODE_H264_DEC, KNL_NODE_UVC_MJPG , KNL_NODE_END};
	KNL_NODE tVDO_KNLSubPath[]  = {KNL_NODE_COMM_RX_VDO, KNL_NODE_VDO_BS_BUF1, KNL_NODE_H264_DEC, KNL_NODE_END};
	#endif
#if APP_FS_FILE_LIST_STYLE
	KNL_NODE tVDO_KNLMainPath_Thm[] = {KNL_NODE_VDO_BS_BUF1, KNL_NODE_H264_DEC, KNL_NODE_LCD, KNL_NODE_END};
#endif
#endif
	KNL_NODE_INFO tVDO_KNLNodeInfo = {0};
	KNL_NODE *pVDO_KNLPath[VDO_SRC_MAX] = {[VDO_MAIN_SRC] = tVDO_KNLMainPath,
										   [VDO_SUB_SRC]  = tVDO_KNLSubPath};
	KNL_SRC tVDO_KNLSrcNum;
	uint16_t uwVDO_NodeNum, i;
	uint8_t ubVDO_CodecIdx;

	tVDO_KNLSrcNum = tVDO_KNLRoleInfo[tVDO_KNLRole].tVDO_KNLParam[tVDO_SrcType].tKNL_SrcNum;
	ubVDO_CodecIdx = tVDO_KNLRoleInfo[tVDO_KNLRole].tVDO_KNLParam[tVDO_SrcType].ubVDO_CodecIdx;
	uwVDO_NodeNum  = ((tVDO_SrcType == VDO_MAIN_SRC)?(sizeof tVDO_KNLMainPath):(sizeof tVDO_KNLSubPath)) / sizeof(KNL_NODE);
#if APP_FS_FILE_LIST_STYLE
	if(KNL_ThmShowInfo.ubEnFlg==1)
	{
		pVDO_KNLPath[VDO_MAIN_SRC] = tVDO_KNLMainPath_Thm;
		uwVDO_NodeNum = (sizeof tVDO_KNLMainPath_Thm) / sizeof(KNL_NODE);
	}
#endif
	for(i = 0; i < uwVDO_NodeNum; i++)
	{
		tVDO_KNLNodeInfo.ubPreNode	= (0 == i)?KNL_NODE_NONE:pVDO_KNLPath[tVDO_SrcType][i-1];
		tVDO_KNLNodeInfo.ubCurNode 	= pVDO_KNLPath[tVDO_SrcType][i];
		tVDO_KNLNodeInfo.ubNextNode = (uwVDO_NodeNum == (i+1))?KNL_NODE_NONE:pVDO_KNLPath[tVDO_SrcType][i+1];
		tVDO_KNLNodeInfo.ubCodecIdx = ubVDO_CodecIdx;
		tVDO_KNLNodeInfo.uwVdoH		= (tVDO_KNLNodeInfo.ubCurNode == KNL_NODE_END)?0:uwKNL_GetVdoH(tVDO_KNLSrcNum);
		tVDO_KNLNodeInfo.uwVdoV		= (tVDO_KNLNodeInfo.ubCurNode == KNL_NODE_END)?0:uwKNL_GetVdoV(tVDO_KNLSrcNum);
		tVDO_KNLNodeInfo.ubHMirror	= 0;
		tVDO_KNLNodeInfo.ubVMirror	= 0;		
	
//	#if defined(BUC_CU)				
//		if((tVDO_KNLNodeInfo.ubCurNode == KNL_NODE_H264_DEC) && (VDO_DISP_TYPE == KNL_DISP_3T_3C))
//		{			
//			tVDO_KNLNodeInfo.ubVMirror	= 1;
//		}
//	#endif		
		
#ifdef BUC_CU
		tVDO_KNLNodeInfo.ubRotate	= (tVDO_KNLNodeInfo.ubCurNode == KNL_NODE_H264_DEC)?KNL_DISP_ROTATE_SETTING:0;
#if (defined(APP_USBD_UVCVDO_MODE) && APP_USBD_UVCVDO_MODE==1 && (APP_UVC_SNXYUV_ONLY == 2))
        if( (tVDO_KNLNodeInfo.ubCurNode == KNL_NODE_H264_DEC) && (tVDO_SrcType == VDO_SUB_SRC))
        {
            tVDO_KNLNodeInfo.ubHScale = KNL_SCALE_X0P5;
            tVDO_KNLNodeInfo.ubVScale = KNL_SCALE_X0P5;
        }
		else
#endif
		{
    		tVDO_KNLNodeInfo.ubHScale	= (tVDO_KNLNodeInfo.ubCurNode == KNL_NODE_END)?0:tVDO_DecScaleParam;
    		tVDO_KNLNodeInfo.ubVScale	= (tVDO_KNLNodeInfo.ubCurNode == KNL_NODE_END)?0:tVDO_DecScaleParam;
		}
#endif
		ubKNL_SetVdoPathNode(tVDO_KNLSrcNum, i, tVDO_KNLNodeInfo);
	}
}
//------------------------------------------------------------------------------
void VDO_ChangePlayState(KNL_ROLE tVDO_KNLRole, VDO_PlayState_t tVdoPlySte)
{
	switch(tVdoPlySte)
	{
		#ifdef BUC_CU
		case VDO_START:
			KNL_VdoResume(tVDO_KNLRoleInfo[tVDO_KNLRole].tVDO_KNLParam[VDO_MAIN_SRC].tKNL_SrcNum);
			break;
		#endif
		case VDO_STOP:
		#ifdef BUC_CAM
			KNL_VdoStop(tVDO_KNLRoleInfo[tVDO_KNLRole].tVDO_KNLParam[VDO_MAIN_SRC].tKNL_SrcNum);
			ubKNL_WaitNodeFinish(tVDO_KNLRoleInfo[tVDO_KNLRole].tVDO_KNLParam[VDO_MAIN_SRC].tKNL_SrcNum);
		#endif
		#ifdef BUC_CU
			KNL_VdoSuspend(tVDO_KNLRoleInfo[tVDO_KNLRole].tVDO_KNLParam[VDO_MAIN_SRC].tKNL_SrcNum);
		#endif
			break;
		default:
			break;
	}
}
//------------------------------------------------------------------------------
KNL_SRC VDO_GetSourceNumber(KNL_VA_DATAPATH tVDO_Path, KNL_ROLE tVDO_KNLRole)
{
	VDO_SrcType_t tVDO_SrcPath[] = {[KNL_MAIN_PATH] = VDO_MAIN_SRC,
									[KNL_SUB_PATH]  = VDO_SUB_SRC,
		                            [KNL_AUX_PATH]  = VDO_AUX_SRC};
	return tVDO_KNLRoleInfo[tVDO_KNLRole].tVDO_KNLParam[tVDO_SrcPath[tVDO_Path]].tKNL_SrcNum;
}
//------------------------------------------------------------------------------
KNL_ROLE VDO_KNLSrcNumMap2KNLRoleNum(KNL_SRC tVDO_SrcNum)
{
	KNL_ROLE tVDO_KNLRole;

	for(tVDO_KNLRole = KNL_STA1; tVDO_KNLRole <= KNL_STA4; tVDO_KNLRole++)
	{
		if((tVDO_KNLRoleInfo[tVDO_KNLRole].tVDO_KNLParam[VDO_MAIN_SRC].tKNL_SrcNum == tVDO_SrcNum) ||
		   (tVDO_KNLRoleInfo[tVDO_KNLRole].tVDO_KNLParam[VDO_SUB_SRC].tKNL_SrcNum == tVDO_SrcNum))
			return tVDO_KNLRole;
	}
	return KNL_NONE;
}
//------------------------------------------------------------------------------
void VDO_Start(void)
{
#ifdef BUC_CAM
	KNL_ROLE tVDO_KNLRole = (KNL_ROLE)ubKNL_GetRole();

	if(TRUE == ubVDO_SysSetupFlag)
	{
		KNL_BufSetup();
		ubVDO_SysSetupFlag = FALSE;
	}
	if(tVDO_KNLRole <= KNL_STA4)
	{
		KNL_VdoStart(tVDO_KNLRoleInfo[tVDO_KNLRole].tVDO_KNLParam[VDO_MAIN_SRC].tKNL_SrcNum);
	#if VDO_SUBPATH_ENABLE
		KNL_VdoStart(tVDO_KNLRoleInfo[tVDO_KNLRole].tVDO_KNLParam[VDO_SUB_SRC].tKNL_SrcNum);
	#endif
	}
#endif
#ifdef BUC_CU
	#if (VDO_DATAPATH_MODE == VDO_PATH_ONLY_LIVECAM)
	KNL_ROLE tVDO_KNLRole, tVDO_KNLSRole = (KNL_DISP_SINGLE == tVDO_Status.tVdoDispType)?tVDO_SvPlayRole:KNL_STA1;
	uint8_t ubKNL_RoleNum = (KNL_DISP_SINGLE == tVDO_Status.tVdoDispType)?tVDO_SvPlayRole:DISPLAY_MODE;
	#else
	KNL_ROLE tVDO_KNLRole, tVDO_KNLSRole = KNL_STA1;
	uint8_t ubKNL_RoleNum = (tVDO_TrxSlotNum)?(tVDO_TrxSlotNum-1):tVDO_TrxSlotNum;
	#endif
	for(tVDO_KNLRole = tVDO_KNLSRole; tVDO_KNLRole <= ubKNL_RoleNum; tVDO_KNLRole++)
	{
		if(VDO_STOP == tVDO_Status.tVdoPlaySte[tVDO_KNLRole])
		{
			KNL_VdoStart(tVDO_KNLRoleInfo[tVDO_KNLRole].tVDO_KNLParam[VDO_MAIN_SRC].tKNL_SrcNum);
			#if (defined(APP_USBD_UVCVDO_MODE) && APP_USBD_UVCVDO_MODE==1 && (APP_UVC_SNXYUV_ONLY == 2))
			KNL_VdoStart(tVDO_KNLRoleInfo[tVDO_KNLRole].tVDO_KNLParam[VDO_SUB_SRC].tKNL_SrcNum);
			#endif
			tVDO_Status.tVdoPlaySte[tVDO_KNLRole] = VDO_START;
		}
	}
	#if (VDO_DATAPATH_MODE == VDO_PATH_KEEP_ACTIVITYCAM)
	if((KNL_DISP_SINGLE == tVDO_Status.tVdoDispType) ||
	   (KNL_DISP_DUAL_C == tVDO_Status.tVdoDispType) ||
	   (KNL_DISP_DUAL_U == tVDO_Status.tVdoDispType))
		KNL_SetTRXPathActivity();
	#endif	
#endif
}
//------------------------------------------------------------------------------
void VDO_Stop(void)
{
#ifdef BUC_CAM
	KNL_ROLE tVDO_KNLRole = (KNL_ROLE)ubKNL_GetRole();

	if(tVDO_KNLRole <= KNL_STA4)
	{
		KNL_VdoStop(tVDO_KNLRoleInfo[tVDO_KNLRole].tVDO_KNLParam[VDO_MAIN_SRC].tKNL_SrcNum);
		KNL_VdoStop(tVDO_KNLRoleInfo[tVDO_KNLRole].tVDO_KNLParam[VDO_SUB_SRC].tKNL_SrcNum);
		KNL_ResetVdoProc();
		ubKNL_WaitNodeFinish(tVDO_KNLRoleInfo[tVDO_KNLRole].tVDO_KNLParam[VDO_MAIN_SRC].tKNL_SrcNum);		
		ubKNL_WaitNodeFinish(tVDO_KNLRoleInfo[tVDO_KNLRole].tVDO_KNLParam[VDO_SUB_SRC].tKNL_SrcNum);
	}
#endif
#ifdef BUC_CU
	KNL_ROLE tVDO_KNLRole, tStopRole = KNL_STA1;

	for(tVDO_KNLRole = KNL_STA1; tVDO_KNLRole < DISPLAY_MODE; tVDO_KNLRole++)
	{
		if(VDO_START == tVDO_Status.tVdoPlaySte[tVDO_KNLRole])
		{
			KNL_VdoStop(tVDO_KNLRoleInfo[tVDO_KNLRole].tVDO_KNLParam[VDO_MAIN_SRC].tKNL_SrcNum);
			#if (defined(APP_USBD_UVCVDO_MODE) && APP_USBD_UVCVDO_MODE==1 && (APP_UVC_SNXYUV_ONLY == 2))
			KNL_VdoStop(tVDO_KNLRoleInfo[tVDO_KNLRole].tVDO_KNLParam[VDO_SUB_SRC].tKNL_SrcNum);
			#endif
			tVDO_Status.tVdoPlaySte[tVDO_KNLRole] = VDO_STOP;
			tStopRole = tVDO_KNLRole;
		}
	}
	KNL_ResetVdoProc();
	ubKNL_WaitNodeFinish(tVDO_KNLRoleInfo[tStopRole].tVDO_KNLParam[VDO_MAIN_SRC].tKNL_SrcNum);
#endif
}
//------------------------------------------------------------------------------
void VDO_SetDisplayMode(VDO_DisplayMode_t tMode)
{
	tVDO_DisplayMode = tMode;
}
//------------------------------------------------------------------------------
#endif //! End #if defined BUC_CU || defined BUC_CAM
