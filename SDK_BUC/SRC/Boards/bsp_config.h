/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		bsp_config.h
	\brief		BSP Config header file
	\author		Wales Wang
	\version	1.6
	\date		2022/02/23
	\copyright	Copyright(C) 2022 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _BSP_CONFIG_H_
#define _BSP_CONFIG_H_

//------------------------------------------------------------------------------
#include "bsp_select.h"

//! BSP RTC Time Definition
#define RTC_TIMER_INTERNAL				0
#define RTC_TIMER_EXTERNAL				1
#define RTC_TIMER_NULL					2
//! BSP Audio aec/nr process by hw/sw define
#define AEC_NR_SW						0
#define AEC_NR_HW						1
//------------------------------------------------------------------------------
//! Include CU Board Config File 
#ifdef BUC_CU
	#if (CU_BOARD_SELECT == SN93711_FHD_REC_RX_V4)			//!< BUC_CU Board(2) : MIPI HD LCD Panel @Demo Board
		#include "bsp_SN93711_FHD_REC_RX.h"
	#endif

	#if (CU_BOARD_SELECT == D_SN93701_SSD2828_RX_V5)
		#include "bsp_SN93711_FHD_REC_RX.h"
	#endif
	
	#if (CU_BOARD_SELECT == D_SNCC71_GM8285C_RX_V2)
		#include "bsp_SNCC71_CARCAM_RX.h"						//!< BUC_CU Board(1) : WSVGA LVDS Panel	@Demo Board
	#endif

	#if (CU_BOARD_SELECT == D_SNCC71_TP2915_AHD_RX_V3)
		#include "bsp_SNCC71_TP2915_AHD_RX.h"
	#endif
	
	#if (CU_BOARD_SELECT == D_SNCC71_NVP6021_AHD_RX_V4)
		#include "bsp_SNCC71_NVP6021_AHD_RX.h"
	#endif

	#if (CU_BOARD_SELECT == D_SN93701_TC358778_RX_V6)
		#include "bsp_SN93711_FHD_REC_RX.h"
	#endif

	#if (CU_BOARD_SELECT == D_SNCC70_TX_V2)
		#include "bsp_SNCC70_CARCAM_TX.h"
	#endif

	#if (CU_BOARD_SELECT == D_SN93714_TX_V1)
		#include "bsp_SN93714_TX.h"
	#endif
#endif	//!< End #ifdef BUC_CU

//------------------------------------------------------------------------------
//! Include CAM Board Config file 
#ifdef BUC_CAM
	#if (CAM_BOARD_SELECT == SN93710_FHD_REC_TX_V4)
		#include "bsp_SN93710_FHD_REC_TX.h"
	#endif
	
	#if (CAM_BOARD_SELECT == D_SNCC70_TX_V2)
		#include "bsp_SNCC70_CARCAM_TX.h"
	#endif
	
	#if (CAM_BOARD_SELECT == SNCC70_AHD_RN6752M_TX_V1)
		#include "bsp_SNCC70_AHD_RN6752M_TX.h"
	#endif
	
	#if (CAM_BOARD_SELECT == SNCC70_AHD_TP9950_TX_V1)
		#include "bsp_SNCC70_AHD_TP9950_TX.h"
	#endif
	
	#if (CAM_BOARD_SELECT == SNCC72_AHD_RN6752M_TX_V1)
		#include "bsp_SNCC72_AHD_RN6752M_TX.h"
	#endif
	
	#if (CAM_BOARD_SELECT == SNCC72_AHD_TP9950_TX_V2)
		#include "bsp_SNCC72_AHD_TP9950_TX.h"
	#endif
	
	#if (CAM_BOARD_SELECT == D_SN93712_VBM_TX_V2)
		#include "bsp_SN93712_VBM_TX.h"
	#endif

	#if (CAM_BOARD_SELECT == D_SNCC72_TX_V1)
		#include "bsp_SNCC72_TX.h"
	#endif

	#if (CAM_BOARD_SELECT == D_SN93714_TX_V1)
		#include "bsp_SN93714_TX.h"
	#endif

	#if (CAM_BOARD_SELECT == D_SNCC74_TX_V1)
		#include "bsp_SN93714_TX.h"
	#endif

	#if (CAM_BOARD_SELECT == D_SN93716_TX_V1)
		#include "bsp_SN93716_TX.h"
	#endif
#endif	//!< End of #ifdef BUC_CAM


#endif	//!< End of #ifndef _BSP_CONFIG_H_
