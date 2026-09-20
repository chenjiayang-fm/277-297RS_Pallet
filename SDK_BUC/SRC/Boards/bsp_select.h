/*!
	The information contained herein is the exclusive property of SONiX and
	shall not be distributed, or disclosed in whole or in part without prior
	permission of SONiX.
	SONiX reserves the right to make changes without further notice to the
	product to improve reliability, function or design. SONiX does not assume
	any liability arising out of the application or use of any product or
	circuits described herein. All application information is advisor and does
	not from part of the specification.

	\file		bsp_select.h
	\brief		BSP Select header file
	\author		futao_jiang
	\version	0.6
	\date		2022/03/01
	\copyright	Copyright(C) 2022 SONiX Technology Co., Ltd. All rights reserved.
*/
//------------------------------------------------------------------------------
#ifndef _BSP_SELECT_H_
#define _BSP_SELECT_H_

//------------------------------------------------------------------------------
//! Board Supported List
//------------------------------------------------------------------------------
//! BUC_CU
#define D_SNCC71_GM8285C_RX_V2          1               //!< CU Board #1 BUC_CU_RTC676x,BUC_CU_A7130_UVC,BUC_CU_S2019A, WSVGA LCD Panel
#define SN93711_FHD_REC_RX_V4           2               //!< CU Board #2 BUC_CU_A7130,BUC_CU_A7130_UVC, HD LCD Panel
#define D_SN93701_SSD2828_RX_V5         3               //!< CU Board #3 BUC_CU_A7130,BUC_CU_A7130_UVC, HD LCD Panel
#define D_SNCC71_TP2915_AHD_RX_V3       4               //!< CU Board #4 BUC_CU_RTC676x_AHD
#define D_SN93701_TC358778_RX_V6        5               //!< CU Board #5 BUC_CU_A7130,BUC_CU_A7130_UVC, HD LCD Panel
#define D_SNCC71_NVP6021_AHD_RX_V4		6				//!< CU Board #6 BUC_CU_RTC676x_AHD

//! BUC_CAM
#define D_SNCC70_TX_V2                  11				//!< CAM  Board #11 BUC_CAM_A7130, BUC_CAM_RTC676x, BUC_CAM_S2019A
#define SN93710_FHD_REC_TX_V4	        12				//!< CAM  Board #12 BUC_CAM_A7130
#define SNCC72_AHD_RN6752M_TX_V1        13				//!< CAM  Board #13 BUC_CAM_A7130_AHD, BUC_CAM_RTC676x_AHD    
#define SNCC72_AHD_TP9950_TX_V2         14				//!< CAM  Board #14 BUC_CAM_A7130_AHD, BUC_CAM_RTC676x_AHD    
#define SNCC70_AHD_TP9950_TX_V1         15				//!< CAM  Board #15 BUC_CAM_RTC676x_AHD
#define SNCC70_AHD_RN6752M_TX_V1        16				//!< CAM  Board #16 BUC_CAM_RTC676x_AHD
#define D_SN93712_VBM_TX_V2             17				//!< CAM  Board #17 BUC_CAM_A7130_UVC
#define D_SNCC72_TX_V1                  18				//!< CAM  Board #18
#define D_SN93714_TX_V1		        	19				//!< CAM  Board #19
#define D_SNCC74_TX_V1		        	20				//!< CAM  Board #20
#define D_SN93716_TX_V1		        	21				//!< CAM  Board #21

//! <<< Use Configuration Wizard in Context Menu >>>
//! <h> Board Part Number
//------------------------------------------------------------------------------
//! Select Board
//------------------------------------------------------------------------------
//! Select CU Board
//! <ol> CU Board Select			< 1=> D_SNCC71_GM8285C_RX_V2  (WSVGA LCD Panel)
//!    								< 2=> SN93711_FHD_REC_RX_V4   (HD LCD Panel)
//!    								< 3=> D_SN93701_SSD2828_RX_V5 (HD LCD Panel)
//!    								< 4=> D_SNCC71_TP2915_AHD_RX_V3 (AHD)
//!    								< 5=> D_SN93701_TC358778_RX_V6(HD LCD Panel)
//! 								< 6=> D_SNCC71_NVP6021_AHD_RX_V4 (AHD)
//!                                 <11=> D_SNCC70_TX_V2 (Wireless USB Camera)
//!    								<19=> D_SN93714_TX_V1 (Wireless USB Camera)
#define CU_BOARD_SELECT			1

//! Select CAM Board
//! <ol> CAM Board Select			<11=> D_SNCC70_TX_V2     
//!    								<12=> SN93710_FHD_REC_TX_V4 
//!    								<13=> SNCC72_AHD_RN6752M_TX_V1(AHD)
//!    								<14=> SNCC72_AHD_TP9950_TX_V2 (AHD)
//!    								<15=> SNCC70_AHD_TP9950_TX_V1 (AHD)
//!    								<16=> SNCC70_AHD_RN6752M_TX_V1(AHD)
//!    								<17=> D_SN93712_VBM_TX_V2
//!    								<18=> D_SNCC72_TX_V1
//!    								<19=> D_SN93714_TX_V1 
//!    								<20=> D_SNCC74_TX_V1
//!    								<21=> D_SN93716_TX_V1
#define CAM_BOARD_SELECT		11
//!	</h>

//------------------------------------------------------------------------------
//! IC Supported List
//------------------------------------------------------------------------------
#define SN93701                0xC0            			//!< CU IC 	(32M)
#define SNCC71				   0xC1            			//!< CU IC  (64M)
//#define SN93511              0xC2            			//!  CU IC 	(64M)
#define SN93712                0x10            			//!< CAM IC (16M)
#define SNCC72				   0x11            			//!< CAM IC (16M)
#define SN93700                0x20            			//!< CAM IC (32M)
#define SNCC70				   0x21            			//!< CAM IC (32M)
#define SN93714				   0x22            			//!< CAM IC (32M)
#define SNCC74				   0x23            			//!< CAM IC (32M)
#define SN93716				   0x24            			//!< CAM IC (32M)

//! <h> IC Part Number
//------------------------------------------------------------------------------
//! Select IC
//------------------------------------------------------------------------------
//! <ol> CU IC Select				<0xC0=> SN93701
//!    								<0xC1=> SNCC71
//!    								<0x21=> SNCC70
//!    								<0x22=> SN93714
#define CU_IC_SELECT		   0xC1

//! <ol> CAM IC Select				<0x10=> SN93712
//!    								<0x11=> SNCC72
//!    								<0x20=> SN93700
//!    								<0x21=> SNCC70
//!    								<0x22=> SN93714
//!    								<0x23=> SNCC74
//!    								<0x24=> SN93716
#define CAM_IC_SELECT		   0x21
//! </h
//! <<< end of configuration section >>>

//------------------------------------------------------------------------------
//! CU Board Config
#ifdef BUC_CU
	#if (CU_BOARD_SELECT == SN93711_FHD_REC_RX_V4)
		#define BSP_SN93711_FHD_REC_RX_V4				        //!< BUC_CU Board(2) : MIPI HD LCD Panel @Demo Board
	#endif
	
	#if (CU_BOARD_SELECT == D_SN93701_SSD2828_RX_V5)
		#define BSP_D_SN93701_SSD2828_RX_V5
	#endif
	
	#if (CU_BOARD_SELECT == D_SNCC71_GM8285C_RX_V2)
		#define BSP_D_SNCC71_GM8285C_RX_V2
	#endif
	
	#if (CU_BOARD_SELECT == D_SNCC71_TP2915_AHD_RX_V3)
		#define BSP_D_SNCC71_TP2915_AHD_RX_V3
	#endif
	
	#if (CU_BOARD_SELECT == D_SNCC71_NVP6021_AHD_RX_V4)
		#define BSP_D_SNCC71_NVP6021_AHD_RX_V4
	#endif
	
	#if (CU_BOARD_SELECT == D_SN93701_TC358778_RX_V6)
		#define BSP_D_SN93701_TC358778_RX_V6
	#endif
	
	#if (CU_BOARD_SELECT == D_SNCC70_TX_V2)
		#define BSP_D_SNCC70_TX_V2
	#endif
	
	#if (CU_BOARD_SELECT == D_SN93714_TX_V1)
		#define BSP_D_SN93714_TX_V1
	#endif
	
	
	#if (CU_IC_SELECT == SN93701)
		#define BSP_DDRSIZE	(32)
	#endif
	
	#if (CU_IC_SELECT == SNCC71)
		#define BSP_DDRSIZE	(64)
	#endif
	
	#if (CU_IC_SELECT == SNCC70)
		#define BSP_DDRSIZE	(32)
	#endif
	
	#if (CU_IC_SELECT == SN93714)
		#define BSP_DDRSIZE	(32)
	#endif
	
	
	#ifndef BSP_DDRSIZE
		#define BSP_DDRSIZE	(32)
	#endif
	
#endif	//!< End #ifdef BUC_CU

//------------------------------------------------------------------------------
//! CAM Board Config 
#ifdef BUC_CAM
	#if (CAM_BOARD_SELECT == SN93710_FHD_REC_TX_V4)
		#define BSP_SN93710_FHD_REC_TX_V4
	#endif
	
	#if (CAM_BOARD_SELECT == D_SNCC70_TX_V2)
		#define BSP_D_SNCC70_TX_V2
	#endif

	#if (CAM_BOARD_SELECT == SNCC70_AHD_RN6752M_TX_V1)
		#define BSP_SNCC70_AHD_RN6752M_TX_V1
	#endif
	
	#if (CAM_BOARD_SELECT == SNCC70_AHD_TP9950_TX_V1)
		#define BSP_SNCC70_AHD_TP9950_TX_V1
	#endif
	
	#if (CAM_BOARD_SELECT == D_SN93712_VBM_TX_V2)
		#define BSP_D_SN93712_VBM_TX_V2
	#endif
	
	#if (CAM_BOARD_SELECT == SNCC72_AHD_RN6752M_TX_V1)
		#define BSP_SNCC72_AHD_RN6752M_TX_V1
	#endif

	#if (CAM_BOARD_SELECT == SNCC72_AHD_TP9950_TX_V2)
		#define BSP_SNCC72_AHD_TP9950_TX_V2
	#endif
	
	#if (CAM_BOARD_SELECT == D_SNCC72_TX_V1)
		#define BSP_D_SNCC72_TX_V1
	#endif
	
	#if (CAM_BOARD_SELECT == D_SN93714_TX_V1)
		#define BSP_D_SN93714_TX_V1 
	#endif
	
	#if (CAM_BOARD_SELECT == D_SNCC74_TX_V1)
		#define BSP_D_SN93714_TX_V1 
	#endif
	
	#if (CAM_BOARD_SELECT == D_SN93716_TX_V1)
		#define BSP_D_SN93716_TX_V1 
	#endif
	
	
	#if (CAM_IC_SELECT == SN93700)
		#define BSP_DDRSIZE	(32)
	#endif
	
	#if (CAM_IC_SELECT == SNCC70)
		#define BSP_DDRSIZE	(32)
	#endif
	
	#if (CAM_IC_SELECT == SN93712)
		#define BSP_DDRSIZE	(16)
	#endif
	
	#if (CAM_IC_SELECT == SNCC72)
		#define BSP_DDRSIZE	(16)
	#endif
	
	#if (CAM_IC_SELECT == SN93714)
		#define BSP_DDRSIZE	(32)
	#endif
	
	#if (CAM_IC_SELECT == SNCC74)
		#define BSP_DDRSIZE	(32)
	#endif
	
	#if (CAM_IC_SELECT == SN93716)
		#define BSP_DDRSIZE	(32)
	#endif
	
	
	#ifndef BSP_DDRSIZE
		#define BSP_DDRSIZE	(32)
	#endif
	
#endif	//!< End of #ifdef BUC_CAM


#endif	//!< End of #ifndef _BSP_SELECT_H_
